/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Red Hat, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): David Tardon <dtardon@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    static yes check_sig(U*, test<U* (U::*)() const, &U::clone>* = 0);
    template<typename U>
    static no check_sig(...);

    static bool const value = sizeof(has_clone<T>::template check_sig<T>(0)) == sizeof(yes);
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

// vim: set sts=4 sw=4 et:
