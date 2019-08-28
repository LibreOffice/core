/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iterator>
#include <vector>

struct S
{
    bool empty() const;
};

template <typename> struct ST
{
    bool empty() const;
};

namespace std
{
bool empty( //expected-error {{Do not declare a function in namespace 'std' [loplugin:stdfunction]}}
    S const&);

template <>
bool empty( //expected-error {{Do not declare a function template specialization in namespace 'std' [loplugin:stdfunction]}}
    S const&);

template <typename T>
bool empty( //expected-error {{Do not declare a function template in namespace 'std' [loplugin:stdfunction]}}
    ST<T> const&);

template <> class vector<S>
{
public:
    [[nodiscard]] constexpr bool empty() const noexcept;
};

[[nodiscard]] constexpr bool vector<S>::empty() const noexcept { return true; }
}

namespace std::inner
{
bool empty( //expected-error {{Do not declare a function in namespace 'std' [loplugin:stdfunction]}}
    S const&);
}

namespace outer::std
{
bool empty(S const&);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
