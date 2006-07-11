/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colortraits.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 11:38:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_COLORTRAITS_HXX
#define INCLUDED_BASEBMP_COLORTRAITS_HXX

#include <basebmp/accessoradapters.hxx>
#include <basebmp/colortraits.hxx>

#include <vigra/mathutil.hxx>

namespace basebmp
{

template< typename ValueType, typename AlphaType > struct BlendFunctorBase
{
    typedef AlphaType first_argument_type;
    typedef ValueType second_argument_type;
    typedef ValueType third_argument_type;
    typedef ValueType result_type;
};

/// Functor template, to calculate alpha blending between two values. Float case.
template< typename ValueType, typename AlphaType > struct BlendFunctor :
        public BlendFunctorBase<ValueType,AlphaType>
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

/// Functor template, to calculate alpha blending between two values. Integer case.
template< typename ValueType, typename AlphaType > struct IntegerBlendFunctor
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

//-----------------------------------------------------------------------------

template< typename ColorType > struct ColorTraits
{
    /// Metafunction to select blend functor from color and alpha type
    template< typename AlphaType > struct blend_functor : public
        ifScalarIntegral< AlphaType,
                          IntegerBlendFunctor< ColorType, AlphaType >,
                          BlendFunctor< ColorType, AlphaType > > {};

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
