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
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static rtl::OUString DISPLAYNAME("DisplayName");



uno::Reference< container::XNameAccess >
ScVbaStyle::getStylesNameContainer( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( rtl::OUString( "CellStyles" ) ), uno::UNO_QUERY_THROW );
    return xStylesAccess;
}

static uno::Reference< beans::XPropertySet >
lcl_getStyleProps( const rtl::OUString& sStyleName, const uno::Reference< frame::XModel >& xModel ) throw ( script::BasicErrorException, uno::RuntimeException )
{

    uno::Reference< beans::XPropertySet > xStyleProps( ScVbaStyle::getStylesNameContainer( xModel )->getByName( sStyleName ), uno::UNO_QUERY_THROW );
    return xStyleProps;
}


void ScVbaStyle::initialise() throw ( uno::RuntimeException )
{
    if (!mxModel.is() )
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString( "XModel Interface could not be retrieved") );
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxPropertySet, uno::UNO_QUERY_THROW );
    if ( !xServiceInfo->supportsService( rtl::OUString( "com.sun.star.style.CellStyle" ) ) )
    {
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    mxStyle.set( mxPropertySet, uno::UNO_QUERY_THROW );

    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxModel, uno::UNO_QUERY_THROW );
    mxStyleFamilyNameContainer.set(  ScVbaStyle::getStylesNameContainer( mxModel ), uno::UNO_QUERY_THROW );

}

ScVbaStyle::ScVbaStyle( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const rtl::OUString& sStyleName, const uno::Reference< frame::XModel >& _xModel ) throw ( script::BasicErrorException, uno::RuntimeException ) :  ScVbaStyle_BASE( xParent, xContext, lcl_getStyleProps( sStyleName, _xModel ), _xModel, false ), mxModel( _xModel )
{
    try
    {
        initialise();
    }
    catch (const uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

ScVbaStyle::ScVbaStyle( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& _xPropertySet, const uno::Reference< frame::XModel >& _xModel ) throw ( script::BasicErrorException, uno::RuntimeException ) : ScVbaStyle_BASE( xParent, xContext, _xPropertySet, _xModel, false ),  mxModel( _xModel )
{
    try
    {
        initialise();
    }
    catch (const uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}


::sal_Bool SAL_CALL
ScVbaStyle::BuiltIn() throw (script::BasicErrorException, uno::RuntimeException)
{
    return !mxStyle->isUserDefined();

}
void SAL_CALL
ScVbaStyle::setName( const ::rtl::OUString& Name ) throw (script::BasicErrorException, uno::RuntimeException)
{
    mxStyle->setName(Name);
}

::rtl::OUString SAL_CALL
ScVbaStyle::getName() throw (script::BasicErrorException, uno::RuntimeException)
{
    return mxStyle->getName();
}

void SAL_CALL
ScVbaStyle::setNameLocal( const ::rtl::OUString& NameLocal ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxPropertySet->setPropertyValue(DISPLAYNAME, uno::makeAny( NameLocal ) );
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
}

::rtl::OUString SAL_CALL
ScVbaStyle::getNameLocal() throw (script::BasicErrorException, uno::RuntimeException)
{
    rtl::OUString sName;
    try
    {
        mxPropertySet->getPropertyValue(DISPLAYNAME) >>= sName;
    }
    catch (const uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return sName;
}

void SAL_CALL
ScVbaStyle::Delete() throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxStyleFamilyNameContainer->removeByName(mxStyle->getName());
    }
    catch (const uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

void SAL_CALL
ScVbaStyle::setMergeCells( const uno::Any& /*MergeCells*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
}

uno::Any SAL_CALL
ScVbaStyle::getMergeCells(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
    return uno::Any();
}

rtl::OUString
ScVbaStyle::getServiceImplName()
{
    return rtl::OUString("ScVbaStyle");
}

uno::Sequence< rtl::OUString >
ScVbaStyle::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.XStyle" );
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
