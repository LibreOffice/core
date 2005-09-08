/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplOPropertySet.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:29:30 $
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
#ifndef CHART_IMPLOPROPERTYSET_HXX
#define CHART_IMPLOPROPERTYSET_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#include <map>
#include <vector>

namespace property
{
namespace impl
{

class ImplOPropertySet
{
public:
    ImplOPropertySet();

    /** supports states DIRECT_VALUE and DEFAULT_VALUE
     */
    ::com::sun::star::beans::PropertyState
        GetPropertyStateByHandle( sal_Int32 nHandle ) const;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState >
        GetPropertyStatesByHandle( const ::std::vector< sal_Int32 > & aHandles ) const;

    void SetPropertyToDefault( sal_Int32 nHandle );
    void SetPropertiesToDefault( const ::std::vector< sal_Int32 > & aHandles );
    void SetAllPropertiesToDefault();

    /** @param rValue is set to the value for the property given in nHandle.  If
               the property is not set, the style chain is searched for any
               instance set there.  If there was no value found either in the
               property set itself or any of its styles, rValue remains
               unchanged and false is returned.

        @return false if the property is default, true otherwise.
     */
    bool GetPropertyValueByHandle(
        ::com::sun::star::uno::Any & rValue,
        sal_Int32 nHandle ) const;

    void SetPropertyValueByHandle( sal_Int32 nHandle,
                                   const ::com::sun::star::uno::Any & rValue,
                                   ::com::sun::star::uno::Any * pOldValue = NULL );

    bool SetStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > & xStyle );
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
        GetStyle() const;

    typedef
        ::std::map< sal_Int32, ::com::sun::star::uno::Any >
        tPropertyMap;

private:
    tPropertyMap    m_aProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
        m_xStyle;
};

} //  namespace impl
} //  namespace chart

// CHART_IMPLOPROPERTYSET_HXX
#endif
