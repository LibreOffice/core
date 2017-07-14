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
#include "vbavariable.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaVariable::SwVbaVariable( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const uno::Reference< beans::XPropertyAccess >& rUserDefined, const OUString& rVariableName ) :
    SwVbaVariable_BASE( rParent, rContext ), mxUserDefined( rUserDefined ), maVariableName( rVariableName )
{
}

SwVbaVariable::~SwVbaVariable()
{
}

OUString SAL_CALL
SwVbaVariable::getName()
{
    return maVariableName;
}

void SAL_CALL
SwVbaVariable::setName( const OUString& )
{
    throw uno::RuntimeException(" Fail to set name" );
}

uno::Any SAL_CALL
SwVbaVariable::getValue()
{
    uno::Reference< beans::XPropertySet > xProp( mxUserDefined, uno::UNO_QUERY_THROW );
    return xProp->getPropertyValue( maVariableName );
}

void SAL_CALL
SwVbaVariable::setValue( const uno::Any& rValue )
{
    // FIXME: fail to set the value if the new type of value is different from the original one.
    uno::Reference< beans::XPropertySet > xProp( mxUserDefined, uno::UNO_QUERY_THROW );
    xProp->setPropertyValue( maVariableName, rValue );
}

sal_Int32 SAL_CALL
SwVbaVariable::getIndex()
{
    const uno::Sequence< beans::PropertyValue > props = mxUserDefined->getPropertyValues();
    for (sal_Int32 i = 0; i < props.getLength(); ++i)
    {
        if( maVariableName == props[i].Name )
            return i+1;
    }

    return 0;
}

OUString
SwVbaVariable::getServiceImplName()
{
    return OUString("SwVbaVariable");
}

uno::Sequence< OUString >
SwVbaVariable::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Variable";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
