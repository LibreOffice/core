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

#ifndef INCLUDED_BASEBMP_INC_COLORBLENDACCESSORADAPTER_HXX
#define INCLUDED_BASEBMP_INC_COLORBLENDACCESSORADAPTER_HXX

#include <colortraits.hxx>

namespace basebmp
{

/** Accessor adapter that blends input value against fixed color value

    Used to blend an alpha mask 'through' a fixed color value into the
    destination.

    The getter functors return a constant value (usually the zero of
    the value type, this preserves the original destination content
    when blitting through a mask) - there really isn't no other
    sensible default behaviour for these methods.
 */
template< class    WrappedAccessor,
          typename AlphaType,
          bool     polarity > class ConstantColorBlendSetterAccessorAdapter
{
public:
    typedef AlphaType                            alpha_type;
    typedef AlphaType                            value_type;
    typedef typename WrappedAccessor::value_type color_type;

private:
    typename ColorTraits< color_type >::
             template blend_functor<alpha_type,polarity>::type   maFunctor;
    WrappedAccessor                                              maWrappee;
    color_type                                                   maBlendColor;
    value_type                                                   maGetterValue;

public:
    ConstantColorBlendSetterAccessorAdapter() :
        maFunctor(),
        maWrappee(),
        maBlendColor(),
        maGetterValue()
    {}

    template< class T > explicit ConstantColorBlendSetterAccessorAdapter( T acc ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(),
        maGetterValue()
    {}

    template< class T > ConstantColorBlendSetterAccessorAdapter( T          acc,
                                                                 color_type col ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(col),
        maGetterValue()
    {}

    template< class T > ConstantColorBlendSetterAccessorAdapter( T          acc,
                                                                 color_type col,
                                                                 value_type val ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(col),
        maGetterValue(val)
    {}



    void        setColor( color_type col ) { maBlendColor=col; }



    /// @return constant value, regardless of iterator content
    template< typename IteratorType > value_type operator()(SAL_UNUSED_PARAMETER IteratorType const& ) const
    {
        return maGetterValue;
    }
    /// @return constant value, regardless of iterator content
    template< typename IteratorType, class Difference >
    value_type operator()(SAL_UNUSED_PARAMETER IteratorType const& , SAL_UNUSED_PARAMETER Difference const& ) const
    {
        return maGetterValue;
    }



    template< typename V, typename IteratorType >
    void set(V const& value, IteratorType const& i) const
    {
        maWrappee.set(
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i),
                maBlendColor),
            i );
    }

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_COLORBLENDACCESSORADAPTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
