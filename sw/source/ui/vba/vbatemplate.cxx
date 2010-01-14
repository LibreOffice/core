/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbatemplate.hxx"
#include <vbahelper/vbahelper.hxx>
#include "wordvbahelper.hxx"
#include "vbaautotextentry.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XAutoTextContainer.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaTemplate::SwVbaTemplate( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel, const rtl::OUString& rName )
    : SwVbaTemplate_BASE( rParent, rContext ), mxModel( rModel ), msName( rName )
{
}


SwVbaTemplate::~SwVbaTemplate()
{
}

rtl::OUString
SwVbaTemplate::getName() throw ( css::uno::RuntimeException )
{
    return msName;
}

uno::Any SAL_CALL
SwVbaTemplate::AutoTextEntries( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< text::XAutoTextContainer > xAutoTextContainer( xMgr->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.AutoTextContainer") ) ), uno::UNO_QUERY_THROW );

    // the default template is "Normal.dot" in Word.
    rtl::OUString sGroup( RTL_CONSTASCII_USTRINGPARAM("Normal") );
    sal_Int32 nIndex = msName.lastIndexOf( sal_Unicode('.') );
    if( nIndex > 0 )
    {
        sGroup = msName.copy( 0, msName.lastIndexOf( sal_Unicode('.') ) );
       // OSL_TRACE("SwVbaTemplate::AutoTextEntries: %s", rtl::OUStringToOString( sGroup, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    uno::Reference< container::XIndexAccess > xGroup;
    if( xAutoTextContainer->hasByName( sGroup ) )
    {
        xGroup.set( xAutoTextContainer->getByName( sGroup ), uno::UNO_QUERY_THROW );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Auto Text Entry doesn't exist") ), uno::Reference< uno::XInterface >() );
        //xGroup.set( xAutoTextContainer->insertNewByName( sGroup ), uno::UNO_QUERY_THROW );
    }

    uno::Reference< XCollection > xCol( new SwVbaAutoTextEntries( this, mxContext, xGroup ) );
    if( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

rtl::OUString&
SwVbaTemplate::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaTemplate") );
        return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaTemplate::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Template" ) );
        }
        return aServiceNames;
}

