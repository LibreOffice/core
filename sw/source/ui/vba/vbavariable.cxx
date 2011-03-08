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
#include "vbavariable.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaVariable::SwVbaVariable( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const uno::Reference< beans::XPropertyAccess >& rUserDefined, const rtl::OUString& rName ) throw ( uno::RuntimeException ) :
    SwVbaVariable_BASE( rParent, rContext ), mxUserDefined( rUserDefined ), maName( rName )
{
}

SwVbaVariable::~SwVbaVariable()
{
}

rtl::OUString SAL_CALL
SwVbaVariable::getName() throw ( css::uno::RuntimeException )
{
    return maName;
}

void SAL_CALL
SwVbaVariable::setName( const rtl::OUString& ) throw ( css::uno::RuntimeException )
{
    throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(" Fail to set name")), uno::Reference< uno::XInterface >() );
}

uno::Any SAL_CALL
SwVbaVariable::getValue() throw ( css::uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xProp( mxUserDefined, uno::UNO_QUERY_THROW );
    return xProp->getPropertyValue( maName );
}

void SAL_CALL
SwVbaVariable::setValue( const uno::Any& rValue ) throw ( css::uno::RuntimeException )
{
    // FIXME: fail to set the value if the new type of vaue is differenct from the original one.
    uno::Reference< beans::XPropertySet > xProp( mxUserDefined, uno::UNO_QUERY_THROW );
    xProp->setPropertyValue( maName, rValue );
}

sal_Int32 SAL_CALL
SwVbaVariable::getIndex() throw ( css::uno::RuntimeException )
{
    const uno::Sequence< beans::PropertyValue > props = mxUserDefined->getPropertyValues();
    for (sal_Int32 i = 0; i < props.getLength(); ++i)
    {
        if( maName.equals( props[i].Name ) )
            return i+1;
    }

    return 0;
}

rtl::OUString&
SwVbaVariable::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaVariable") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaVariable::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Variable" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
