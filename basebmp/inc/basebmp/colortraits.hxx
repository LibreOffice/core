/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colortraits.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_COLORTRAITS_HXX
#define INCLUDED_BASEBMP_COLORTRAITS_HXX

#include <basebmp/accessoradapters.hxx>
#include <basebmp/metafunctions.hxx>

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

//-----------------------------------------------------------------------------

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

    /// Calculate normalized distance between color c1 and c2
    static inline vigra::NormTraits<ColorType> distance( ColorType c1,
                                                         ColorType c2 )
    {
        return vigra::norm(c1 - c2);
    }

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

#endif /* INCLUDED_BASEBMP_COLORTRAITS_HXX */
