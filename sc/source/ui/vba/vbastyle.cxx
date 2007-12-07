/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbastyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:03:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "vbastyle.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

using namespace ::org::openoffice;
using namespace ::com::sun::star;

static rtl::OUString DISPLAYNAME( RTL_CONSTASCII_USTRINGPARAM("DisplayName") );



uno::Reference< container::XNameAccess >
ScVbaStyle::getStylesNameContainer( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CellStyles" ) ) ), uno::UNO_QUERY_THROW );
    return xStylesAccess;
}

uno::Reference< beans::XPropertySet >
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

ScVbaStyle::ScVbaStyle( const uno::Reference< oo::vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const rtl::OUString& sStyleName, const uno::Reference< frame::XModel >& _xModel ) throw ( script::BasicErrorException, uno::RuntimeException ) :  ScVbaStyle_BASE( xParent, xContext, lcl_getStyleProps( sStyleName, _xModel ), _xModel, false )
{
    try
    {
        initialise();
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

ScVbaStyle::ScVbaStyle( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& _xPropertySet, const uno::Reference< frame::XModel >& _xModel ) throw ( script::BasicErrorException, uno::RuntimeException ) : ScVbaStyle_BASE( xParent, xContext, _xPropertySet, _xModel, false )
{
    try
    {
        initialise();
    }
    catch (uno::Exception& )
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
    catch (uno::Exception& e)
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
    catch (uno::Exception e)
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
    catch (uno::Exception& )
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


rtl::OUString&
ScVbaStyle::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaStyle") );
        return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaStyle::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.XStyle" ) );
        }
        return aServiceNames;
}
