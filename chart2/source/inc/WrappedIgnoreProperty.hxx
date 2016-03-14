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
#ifndef INCLUDED_CHART2_SOURCE_INC_WRAPPEDIGNOREPROPERTY_HXX
#define INCLUDED_CHART2_SOURCE_INC_WRAPPEDIGNOREPROPERTY_HXX

#include "WrappedProperty.hxx"
#include "charttoolsdllapi.hxx"

#include <vector>

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS WrappedIgnoreProperty : public WrappedProperty
{
public:
    WrappedIgnoreProperty( const OUString& rOuterName, const css::uno::Any& rDefaultValue );
    virtual ~WrappedIgnoreProperty();

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;

SAL_DLLPRIVATE virtual void setPropertyToDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (css::beans::UnknownPropertyException, css::uno::RuntimeException) override;

SAL_DLLPRIVATE virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;

SAL_DLLPRIVATE virtual css::beans::PropertyState getPropertyState( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (css::beans::UnknownPropertyException, css::uno::RuntimeException) override;

protected:
    css::uno::Any          m_aDefaultValue;
    mutable css::uno::Any  m_aCurrentValue;
};

class OOO_DLLPUBLIC_CHARTTOOLS WrappedIgnoreProperties
{
public:
    static void addIgnoreLineProperties( std::vector< WrappedProperty* >& rList );

    static void addIgnoreFillProperties( std::vector< WrappedProperty* >& rList );
    SAL_DLLPRIVATE static void addIgnoreFillProperties_without_BitmapProperties( std::vector< WrappedProperty* >& rList );
    SAL_DLLPRIVATE static void addIgnoreFillProperties_only_BitmapProperties( std::vector< WrappedProperty* >& rList );
};

} //namespace chart

// INCLUDED_CHART2_SOURCE_INC_WRAPPEDIGNOREPROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
