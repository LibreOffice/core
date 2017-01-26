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

#ifndef INCLUDED_SVX_UNOMASTER_HXX
#define INCLUDED_SVX_UNOMASTER_HXX

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

/** this abstract class is the interface for an instance that likes to enhance
    the functionality of components implemented with derivations from SvxShape

    @see SvxShape::setMaster
*/
class SvxShapeMaster
{
public:
    virtual bool queryAggregation( const css::uno::Type & rType, css::uno::Any& aAny ) = 0;

    /// @throws css::uno::RuntimeException
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) = 0;
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) = 0;
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) = 0;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) = 0;
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) = 0;
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) = 0;

    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) = 0;

    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() = 0;

    virtual void dispose() = 0;

    virtual void modelChanged( SdrModel* pNewModel ) = 0;

protected:
    ~SvxShapeMaster() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
