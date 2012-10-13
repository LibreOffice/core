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

#include "vbastyle.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static rtl::OUString DISPLAYNAME( RTL_CONSTASCII_USTRINGPARAM("DisplayName") );



uno::Reference< container::XNameAccess >
ScVbaStyle::getStylesNameContainer( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CellStyles" ) ) ), uno::UNO_QUERY_THROW );
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
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XModel Interface could not be retrieved")) );
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxPropertySet, uno::UNO_QUERY_THROW );
    if ( !xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.CellStyle" ) ) ) )
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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaStyle"));
}

uno::Sequence< rtl::OUString >
ScVbaStyle::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.XStyle" ) );
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
