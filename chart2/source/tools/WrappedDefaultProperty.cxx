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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedDefaultProperty.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

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
    catch( beans::UnknownPropertyException& ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aState;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
