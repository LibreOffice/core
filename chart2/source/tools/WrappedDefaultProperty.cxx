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

#include "WrappedDefaultProperty.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

WrappedDefaultProperty::WrappedDefaultProperty(
    const OUString& rOuterName, const OUString& rInnerName,
    const uno::Any& rNewOuterDefault ) :
        WrappedProperty( rOuterName, rInnerName ),
        m_aOuterDefaultValue( rNewOuterDefault )
{}

WrappedDefaultProperty::~WrappedDefaultProperty()
{}

void WrappedDefaultProperty::setPropertyToDefault(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropSet( xInnerPropertyState, uno::UNO_QUERY );
    if( xInnerPropSet.is())
        this->setPropertyValue( m_aOuterDefaultValue, xInnerPropSet );
}

uno::Any WrappedDefaultProperty::getPropertyDefault(
    const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    return m_aOuterDefaultValue;
}

beans::PropertyState WrappedDefaultProperty::getPropertyState(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    beans::PropertyState aState = beans::PropertyState_DIRECT_VALUE;
    try
    {
        Reference< beans::XPropertySet > xInnerProp( xInnerPropertyState, uno::UNO_QUERY_THROW );
        uno::Any aValue = this->getPropertyValue( xInnerProp );
        if( m_aOuterDefaultValue == this->convertInnerToOuterValue( aValue ))
            aState = beans::PropertyState_DEFAULT_VALUE;
    }
    catch( const beans::UnknownPropertyException& ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aState;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
