/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OOX_HELPER_REFVECTOR_HXX
#define INCLUDED_OOX_HELPER_REFVECTOR_HXX

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include <sal/types.h>

namespace oox {


/** Template for a vector of ref-counted objects with additional accessor functions.

    An instance of the class RefVector< Type > stores elements of the type
    std::shared_ptr< Type >. The new accessor functions has() and get()
    work correctly for indexes out of the current range, there is no need to
    check the passed index before.
 */
template< typename ObjType >
class RefVector : public ::std::vector< std::shared_ptr< ObjType > >
{
public:
    typedef ::std::vector< std::shared_ptr< ObjType > > container_type;
    typedef typename container_type::value_type             value_type;
    typedef typename container_type::size_type              size_type;

public:

    /** Returns a reference to the object with the passed index, or 0 on error. */
    value_type          get( sal_Int32 nIndex ) const
                        {
                            if( const value_type* pxRef = getRef( nIndex ) ) return *pxRef;
                            return value_type();
                        }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template <typename FunctorType> void forEach(FunctorType aFunctor) const
    {
        forEachWithIndex([&aFunctor](size_type, ObjType& rObj) { aFunctor(rObj); });
    }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template <typename FuncType, typename... T> void forEachMem(FuncType pFunc, T&&... args) const
    {
        forEach([pFunc, &args...](ObjType& rObj) { (rObj.*pFunc)(args...); });
    }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index as first argument to the member function. */
    template <typename FuncType, typename... T>
    void forEachMemWithIndex(FuncType pFunc, T&&... args) const
    {
        forEachWithIndex([pFunc, &args...](size_type i, ObjType& rObj)
                         { (rObj.*pFunc)(i, args...); });
    }

    /** Searches for an element by using the passed functor that takes a
        constant reference of the object type (const ObjType&). */
    template <typename FunctorType> value_type findIf(const FunctorType& rFunctor) const
    {
        auto aIt = ::std::find_if(this->begin(), this->end(), [&rFunctor](const value_type& rValue)
                                  { return rValue && rFunctor(*rValue); });
        return (aIt == this->end()) ? value_type() : *aIt;
    }

private:
    template <typename F> void forEachWithIndex(const F& f) const
    {
        for (size_type i = 0; i < this->size(); ++i)
            if (const value_type& rValue = (*this)[i])
                f(i, *rValue);
    }

    const value_type*   getRef( sal_Int32 nIndex ) const
                        {
                            return ((0 <= nIndex) && (static_cast< size_type >( nIndex ) < this->size())) ?
                                &(*this)[ static_cast< size_type >( nIndex ) ] : nullptr;
                        }
};


} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
