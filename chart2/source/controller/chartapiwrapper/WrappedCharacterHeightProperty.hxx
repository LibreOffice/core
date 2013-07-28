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
#ifndef CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX
#define CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX

#include "WrappedProperty.hxx"

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
    virtual ~WrappedCharacterHeightProperty_Base();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState getPropertyState( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const;

protected:
    ReferenceSizePropertyProvider*  m_pRefSizePropProvider;
};

class WrappedCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedCharacterHeightProperty();

    static void addWrappedProperties( std::vector< WrappedProperty* >& rList, ReferenceSizePropertyProvider* pRefSizePropProvider );
};

class WrappedAsianCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedAsianCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedAsianCharacterHeightProperty();
};

class WrappedComplexCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedComplexCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedComplexCharacterHeightProperty();
};

} //namespace wrapper
} //namespace chart

// CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
