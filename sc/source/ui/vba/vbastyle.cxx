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

#include "vbastyle.hxx"
#include <basic/sberrors.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const OUStringLiteral DISPLAYNAME = u"DisplayName";

uno::Reference< container::XNameAccess >
ScVbaStyle::getStylesNameContainer( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName("CellStyles"), uno::UNO_QUERY_THROW );
    return xStylesAccess;
}

/// @throws script::BasicErrorException
/// @throws uno::RuntimeException
static uno::Reference< beans::XPropertySet >
lcl_getStyleProps( const OUString& sStyleName, const uno::Reference< frame::XModel >& xModel )
{

    uno::Reference< beans::XPropertySet > xStyleProps( ScVbaStyle::getStylesNameContainer( xModel )->getByName( sStyleName ), uno::UNO_QUERY_THROW );
    return xStyleProps;
}

void ScVbaStyle::initialise()
{
    if (!mxModel.is() )
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, u"XModel Interface could not be retrieved" );
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxPropertySet, uno::UNO_QUERY_THROW );
    if ( !xServiceInfo->supportsService("com.sun.star.style.CellStyle") )
    {
            DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
    mxStyle.set( mxPropertySet, uno::UNO_QUERY_THROW );

    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxModel, uno::UNO_QUERY_THROW );
    mxStyleFamilyNameContainer.set(  ScVbaStyle::getStylesNameContainer( mxModel ), uno::UNO_QUERY_THROW );

}

ScVbaStyle::ScVbaStyle( const uno::Reference< ov::XHelperInterface >& xParent,
                        const uno::Reference< uno::XComponentContext > & xContext,
                        const OUString& sStyleName, const uno::Reference< frame::XModel >& _xModel )
    :  ScVbaStyle_BASE( xParent, xContext, lcl_getStyleProps( sStyleName, _xModel ), _xModel, false )
{
    try
    {
        initialise();
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

ScVbaStyle::ScVbaStyle( const uno::Reference< XHelperInterface >& xParent,
                        const uno::Reference< uno::XComponentContext > & xContext,
                        const uno::Reference< beans::XPropertySet >& _xPropertySet,
                        const uno::Reference< frame::XModel >& _xModel )
    : ScVbaStyle_BASE( xParent, xContext, _xPropertySet, _xModel, false )
{
    try
    {
        initialise();
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

sal_Bool SAL_CALL
ScVbaStyle::BuiltIn()
{
    return !mxStyle->isUserDefined();

}
void SAL_CALL
ScVbaStyle::setName( const OUString& Name )
{
    mxStyle->setName(Name);
}

OUString SAL_CALL
ScVbaStyle::getName()
{
    return mxStyle->getName();
}

void SAL_CALL
ScVbaStyle::setNameLocal( const OUString& NameLocal )
{
    try
    {
        mxPropertySet->setPropertyValue(DISPLAYNAME, uno::makeAny( NameLocal ) );
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
}

OUString SAL_CALL
ScVbaStyle::getNameLocal()
{
    OUString sName;
    try
    {
        mxPropertySet->getPropertyValue(DISPLAYNAME) >>= sName;
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
    return sName;
}

void SAL_CALL
ScVbaStyle::Delete()
{
    try
    {
        mxStyleFamilyNameContainer->removeByName(mxStyle->getName());
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

void SAL_CALL
ScVbaStyle::setMergeCells( const uno::Any& /*MergeCells*/ )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
}

uno::Any SAL_CALL
ScVbaStyle::getMergeCells(  )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
    return uno::Any();
}

OUString
ScVbaStyle::getServiceImplName()
{
    return "ScVbaStyle";
}

uno::Sequence< OUString >
ScVbaStyle::getServiceNames()
{
        static uno::Sequence< OUString > const aServiceNames
        {
            "ooo.vba.excel.XStyle"
        };
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
