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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDCHARACTERHEIGHTPROPERTY_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDCHARACTERHEIGHTPROPERTY_HXX

#include <WrappedProperty.hxx>

#include <vector>

namespace chart
{
namespace wrapper
{

class ReferenceSizePropertyProvider;

class WrappedCharacterHeightProperty_Base : public WrappedProperty
{
public:
    WrappedCharacterHeightProperty_Base( const OUString& rOuterEqualsInnerName, ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedCharacterHeightProperty_Base() override;

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

    virtual css::beans::PropertyState getPropertyState( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected:
    virtual css::uno::Any convertInnerToOuterValue( const css::uno::Any& rInnerValue ) const override;
    virtual css::uno::Any convertOuterToInnerValue( const css::uno::Any& rOuterValue ) const override;

protected:
    ReferenceSizePropertyProvider*  m_pRefSizePropProvider;
};

class WrappedCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    explicit WrappedCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedCharacterHeightProperty() override;

    static void addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList, ReferenceSizePropertyProvider* pRefSizePropProvider );
};

class WrappedAsianCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    explicit WrappedAsianCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedAsianCharacterHeightProperty() override;
};

class WrappedComplexCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    explicit WrappedComplexCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedComplexCharacterHeightProperty() override;
};

} //namespace wrapper
} //namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDCHARACTERHEIGHTPROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
