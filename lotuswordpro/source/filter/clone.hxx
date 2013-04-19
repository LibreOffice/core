/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace detail
{

template<typename T>
struct has_clone
{
    template<typename U, U x>
    struct test;

    typedef char yes;
    typedef struct { char a[2]; } no;

    template<typename U>
    static yes& check_sig(U*, test<U* (U::*)() const, &U::clone>* = 0);
    template<typename U>
    static no& check_sig(...);

    enum
    {
        value = sizeof(check_sig<T>(0)) == sizeof(yes)
    };
};

template<typename T, bool HasClone>
struct cloner
{
    static T* clone(T* const other)
    {
        return new T(*other);
    }
};

template<typename T>
struct cloner<T, true>
{
    static T* clone(T* const other)
    {
        return other->clone();
    }
};

}

/** Creates a new copy of the passed object.
    If other is 0, just returns 0. Otherwise, if other has function
    named clone with signature T* (T::*)() const, the function is called.
    Otherwise, copy constructor is used.

    @returns 0 or newly allocated object
 */
template<typename T>
T* clone(T* const other)
{
    return other ? ::detail::cloner<T, ::detail::has_clone<T>::value>::clone(other) : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
