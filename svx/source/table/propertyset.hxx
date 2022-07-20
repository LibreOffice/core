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

#ifndef INCLUDED_SVX_SOURCE_TABLE_PROPERTYSET_HXX
#define INCLUDED_SVX_SOURCE_TABLE_PROPERTYSET_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <rtl/ref.hxx>
#include <unordered_map>
#include <vector>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

namespace sdr::table {

typedef std::vector< css::beans::Property > PropertyVector;
typedef std::unordered_map< OUString, ::sal_uInt32 > PropertyMap;

class FastPropertySetInfo : public ::cppu::WeakAggImplHelper1< css::beans::XPropertySetInfo >
{
public:
    explicit FastPropertySetInfo( const PropertyVector& rProps );
    virtual ~FastPropertySetInfo() override;

    void addProperties( const PropertyVector& rProps );

    /// @throws css::beans::UnknownPropertyException
    const css::beans::Property& getProperty( const OUString& aName );
    const css::beans::Property* hasProperty( const OUString& aName );

    // XPropertySetInfo
    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties(  ) override;
    virtual css::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

private:
    PropertyVector maProperties;
    PropertyMap maMap;
};


class FastPropertySet : public ::cppu::WeakAggImplHelper3<  css::beans::XPropertySet, css::beans::XMultiPropertySet, css::beans::XFastPropertySet >
{
public:
    explicit FastPropertySet( rtl::Reference< FastPropertySetInfo > xInfo );
    virtual ~FastPropertySet() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
//    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const css::uno::Any& aValue ) override = 0;
    virtual css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) override = 0;

private:
    rtl::Reference< FastPropertySetInfo > mxInfo;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
