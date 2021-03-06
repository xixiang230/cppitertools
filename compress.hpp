#ifndef ITER_COMPRESS_H_
#define ITER_COMPRESS_H_

#include "internal/iterbase.hpp"

#include <utility>
#include <iterator>

namespace iter {
  namespace impl {
    template <typename Container, typename Selector>
    class Compressed;
  }

  template <typename Container, typename Selector>
  impl::Compressed<Container, Selector> compress(Container&&, Selector&&);

}

template <typename Container, typename Selector>
class iter::impl::Compressed {
 private:
  Container container;
  Selector selectors;

  friend Compressed iter::compress<Container, Selector>(
      Container&&, Selector&&);

  // Selector::Iterator type
  using selector_iter_type = decltype(std::begin(selectors));

  Compressed(Container&& in_container, Selector&& in_selectors)
      : container(std::forward<Container>(in_container)),
        selectors(std::forward<Selector>(in_selectors)) {}

 public:
  Compressed(Compressed&&) = default;
  class Iterator : public std::iterator<std::input_iterator_tag,
                       iterator_traits_deref<Container>> {
   private:
    iterator_type<Container> sub_iter;
    iterator_type<Container> sub_end;

    selector_iter_type selector_iter;
    selector_iter_type selector_end;

    void increment_iterators() {
      ++this->sub_iter;
      ++this->selector_iter;
    }

    void skip_failures() {
      while (this->sub_iter != this->sub_end
             && this->selector_iter != this->selector_end
             && !*this->selector_iter) {
        this->increment_iterators();
      }
    }

   public:
    Iterator(iterator_type<Container>&& cont_iter,
        iterator_type<Container>&& cont_end, selector_iter_type&& sel_iter,
        selector_iter_type&& sel_end)
        : sub_iter{std::move(cont_iter)},
          sub_end{std::move(cont_end)},
          selector_iter{std::move(sel_iter)},
          selector_end{std::move(sel_end)} {
      this->skip_failures();
    }

    iterator_deref<Container> operator*() {
      return *this->sub_iter;
    }

    iterator_arrow<Container> operator->() {
      return apply_arrow(this->sub_iter);
    }

    Iterator& operator++() {
      this->increment_iterators();
      this->skip_failures();
      return *this;
    }

    Iterator operator++(int) {
      auto ret = *this;
      ++*this;
      return ret;
    }

    bool operator!=(const Iterator& other) const {
      return this->sub_iter != other.sub_iter
             && this->selector_iter != other.selector_iter;
    }

    bool operator==(const Iterator& other) const {
      return !(*this != other);
    }
  };

  Iterator begin() {
    return {std::begin(this->container), std::end(this->container),
        std::begin(this->selectors), std::end(this->selectors)};
  }

  Iterator end() {
    return {std::end(this->container), std::end(this->container),
        std::end(this->selectors), std::end(this->selectors)};
  }
};

template <typename Container, typename Selector>
iter::impl::Compressed<Container, Selector> iter::compress(
    Container&& container, Selector&& selectors) {
  return {
      std::forward<Container>(container), std::forward<Selector>(selectors)};
}

#endif
