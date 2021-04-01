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

#ifndef INCLUDED_COMPHELPER_PROPERTYSETHELPER_HXX
#define INCLUDED_COMPHELPER_PROPERTYSETHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ref.hxx>

namespace comphelper
{
class PropertySetInfo;
struct PropertyMapEntry;

class COMPHELPER_DLLPUBLIC PropertySetHelper : public css::beans::XPropertySet,
                          public css::beans::XPropertyState,
                          public css::beans::XMultiPropertySet
{
private:
    rtl::Reference<PropertySetInfo> mxInfo;

protected:
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) = 0;
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue ) = 0;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual void _getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, css::beans::PropertyState* pStates );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual void _setPropertyToDefault( const comphelper::PropertyMapEntry* pEntry );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any _getPropertyDefault( const comphelper::PropertyMapEntry* pEntry );

public:
    PropertySetHelper( rtl::Reference<comphelper::PropertySetInfo> const & xInfo ) throw();
    virtual ~PropertySetHelper() throw();

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
//  virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;
};

}

#endif // _UTL_PROPERTYSETHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
