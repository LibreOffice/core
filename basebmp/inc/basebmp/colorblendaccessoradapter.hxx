/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colorblendaccessoradapter.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:02:07 $
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

#ifndef INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX
#define INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX

#include <basebmp/colortraits.hxx>

namespace basebmp
{

/** Accessor adapter that blends input value against fixed color value

    Used to blend an alpha mask 'through' a fixed color value into the
    destination.

    The getter functors return a constant value (usually the zero of
    the value type, this preserves the original destination content
    when blitting through a mask) - there really isn't no sensible
    default behaviour for these methods.
 */
template< class WrappedAccessor,
          typename AlphaType > class ConstantColorBlendSetterAccessorAdapter
{
public:
    typedef AlphaType                            alpha_type;
    typedef AlphaType                            value_type;
    typedef typename WrappedAccessor::value_type color_type;

private:
    typename ColorTraits< color_type >::
             template blend_functor<alpha_type>::type   maFunctor;
    WrappedAccessor                                     maWrappee;
    color_type                                          maBlendColor;
    value_type                                          maGetterValue;

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

    // -------------------------------------------------------

    void        setColor( color_type col ) { maBlendColor=col; }
    color_type  getColor() { return maBlendColor; }
    void        setGetterValue( value_type val ) { maGetterValue=val; }
    value_type  getGetterValue() { return maGetterValue; }

    // -------------------------------------------------------

    WrappedAccessor const& getWrappedAccessor() const { return maWrappee; }
    WrappedAccessor&       getWrappedAccessor() { return maWrappee; }

    // -------------------------------------------------------

    /// @return constant value, regardless of iterator content
    template< typename IteratorType > value_type operator()(IteratorType const& ) const
    {
        return maGetterValue;
    }
    /// @return constant value, regardless of iterator content
    template< typename IteratorType, class Difference >
    value_type operator()(IteratorType const& , Difference const& ) const
    {
        return maGetterValue;
    }

    // -------------------------------------------------------

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

    template< typename V, typename IteratorType, class Difference >
    void set(V const& value, IteratorType const& i, Difference const& diff) const
    {
        maWrappee.set(
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i,diff),
                maBlendColor),
            i,
            diff );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX */
