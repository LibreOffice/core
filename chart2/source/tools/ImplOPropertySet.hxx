/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef CHART_IMPLOPROPERTYSET_HXX
#define CHART_IMPLOPROPERTYSET_HXX

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/XStyle.hpp>

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
    explicit ImplOPropertySet( const ImplOPropertySet & rOther );

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
    void cloneInterfaceProperties();

    tPropertyMap    m_aProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
        m_xStyle;
};

} //  namespace impl
} //  namespace chart

// CHART_IMPLOPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
