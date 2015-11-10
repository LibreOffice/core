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
#ifndef INCLUDED_CHART2_SOURCE_TOOLS_IMPLOPROPERTYSET_HXX
#define INCLUDED_CHART2_SOURCE_TOOLS_IMPLOPROPERTYSET_HXX

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
                                   ::com::sun::star::uno::Any * pOldValue = nullptr );

    bool SetStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > & xStyle );
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
        GetStyle() const { return m_xStyle;}

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

// INCLUDED_CHART2_SOURCE_TOOLS_IMPLOPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
