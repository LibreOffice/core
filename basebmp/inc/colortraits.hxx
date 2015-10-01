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

#ifndef INCLUDED_BASEBMP_INC_COLORTRAITS_HXX
#define INCLUDED_BASEBMP_INC_COLORTRAITS_HXX

#include <accessoradapters.hxx>
#include <metafunctions.hxx>

#include <vigra/mathutil.hxx>

namespace basebmp
{

/** Functor template, to calculate alpha blending between two
    values. Float case.

    @tpl polarity
    When true, 0 means fully transparent, and 1 fully opaque. And vice
    versa.
 */
template< typename ValueType,
          typename AlphaType,
          bool     polarity > struct BlendFunctor;
template< typename ValueType,
          typename AlphaType > struct BlendFunctor<ValueType,AlphaType,true>
  : public TernaryFunctorBase<AlphaType,ValueType,ValueType,ValueType>
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        const typename vigra::NumericTraits<AlphaType>::RealPromote fAlpha(
            vigra::NumericTraits<AlphaType>::toRealPromote(alpha));
        return (vigra::NumericTraits<AlphaType>::one()-fAlpha)*v1 + fAlpha*v2;
    }
};
template< typename ValueType,
          typename AlphaType > struct BlendFunctor<ValueType,AlphaType,false>
  : public TernaryFunctorBase<AlphaType,ValueType,ValueType,ValueType>
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        const typename vigra::NumericTraits<AlphaType>::RealPromote fAlpha(
            vigra::NumericTraits<AlphaType>::toRealPromote(alpha));
        return fAlpha*v1 + (vigra::NumericTraits<AlphaType>::one()-fAlpha)*v2;
    }
};

/** Functor template, to calculate alpha blending between two
    values. Integer case.

    @tpl polarity
    When true, 0 means fully transparent, and 1 fully opaque. And vice
    versa.
 */
template< typename ValueType,
          typename AlphaType,
          bool     polarity > struct IntegerBlendFunctor;
template< typename ValueType,
          typename AlphaType > struct IntegerBlendFunctor<ValueType,AlphaType,true>
  : public TernaryFunctorBase<AlphaType,ValueType,ValueType,ValueType>
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        return (vigra::NumericTraits<AlphaType>::toPromote(
                    vigra::NumericTraits<AlphaType>::max()-alpha)*v1 + alpha*v2) /
            vigra::NumericTraits<AlphaType>::max();
    }
};
template< typename ValueType,
          typename AlphaType > struct IntegerBlendFunctor<ValueType,AlphaType,false>
  : public TernaryFunctorBase<AlphaType,ValueType,ValueType,ValueType>
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        return (alpha*v1 +
                vigra::NumericTraits<AlphaType>::toPromote(
                    vigra::NumericTraits<AlphaType>::max()-alpha)*v2) /
            vigra::NumericTraits<AlphaType>::max();
    }
};



template< typename ColorType > struct ColorTraits
{
    /// Metafunction to select blend functor from color and alpha type
    template< typename AlphaType, bool polarity > struct blend_functor : public
        ifScalarIntegral< AlphaType,
                          IntegerBlendFunctor< ColorType, AlphaType, polarity >,
                          BlendFunctor< ColorType, AlphaType, polarity > > {};

    /// @return number of color channels
    static int numChannels() { return 1; }

    /// Type of a color component (i.e. the type of an individual channel)
    typedef ColorType component_type;

    static inline component_type toGreyscale( ColorType c )
    {
        return c;
    }

    static inline ColorType fromGreyscale( component_type c )
    {
        return c;
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_COLORTRAITS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
