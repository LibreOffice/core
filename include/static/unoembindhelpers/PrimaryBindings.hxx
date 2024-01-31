/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <stdexcept>

#include <emscripten/bind.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>

template <typename T> struct emscripten::smart_ptr_trait<css::uno::Reference<T>>
{
    using PointerType = css::uno::Reference<T>;
    using element_type = T;
    static T* get(css::uno::Reference<T> const& ptr) { return ptr.get(); }
    static sharing_policy get_sharing_policy() { return sharing_policy::INTRUSIVE; }
    static css::uno::Reference<T>* share(T* v) { return new css::uno::Reference<T>(v); }
    static css::uno::Reference<T>* construct_null() { return new css::uno::Reference<T>(); }
};

namespace unoembindhelpers
{
enum class uno_Reference
{
    FromAny
};

template <typename T> struct UnoInOutParam
{
    UnoInOutParam() {}

    UnoInOutParam(T the_value)
        : value(the_value)
    {
    }

    T get() const { return value; }

    void set(T the_value) { value = the_value; }

    T value;
};

template <typename T>
void checkSequenceAccess(css::uno::Sequence<T> const& sequence, sal_Int32 index)
{
    if (index < 0 || index >= sequence.getLength())
    {
        throw std::out_of_range("index out of bounds");
    }
}

template <typename T> void registerSequence(char const* name)
{
    emscripten::class_<css::uno::Sequence<T>>(name)
        .template constructor<sal_Int32>()
        .function("resize",
                  +[](css::uno::Sequence<T>& self, sal_Int32 size) {
                      if (size < 0)
                      {
                          throw std::invalid_argument("negative size");
                      }
                      self.realloc(size);
                  })
        .function("size", &css::uno::Sequence<T>::getLength)
        .function("get",
                  +[](css::uno::Sequence<T> const& self, sal_Int32 index) {
                      checkSequenceAccess(self, index);
                      return self[index];
                  })
        .function("set", +[](css::uno::Sequence<T>& self, sal_Int32 index, T const& value) {
            checkSequenceAccess(self, index);
            self.getArray()[index] = value;
        });
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
