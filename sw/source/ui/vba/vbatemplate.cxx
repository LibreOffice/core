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
#include "vbatemplate.hxx"
#include <vbahelper/vbahelper.hxx>
#include "wordvbahelper.hxx"
#include "vbaautotextentry.hxx"
#include <com/sun/star/text/XAutoTextContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static String lcl_CheckGroupName( const String& rGroupName )
{
    String sRet;
    //group name should contain only A-Z and a-z and spaces
    for( xub_StrLen i = 0; i < rGroupName.Len(); i++ )
    {
        sal_Unicode cChar = rGroupName.GetChar(i);
        if (comphelper::string::isalnumAscii(cChar) ||
            cChar == '_' || cChar == 0x20)
        {
            sRet += cChar;
        }
    }
    return comphelper::string::strip(sRet, ' ');
}


SwVbaTemplate::SwVbaTemplate( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel, const rtl::OUString& rFullUrl )
    : SwVbaTemplate_BASE( rParent, rContext ), mxModel( rModel ), msFullUrl( rFullUrl )
{
}


SwVbaTemplate::~SwVbaTemplate()
{
}

rtl::OUString
SwVbaTemplate::getName() throw ( css::uno::RuntimeException )
{
    rtl::OUString sName;
    if( !msFullUrl.isEmpty() )
    {
        INetURLObject aURL( msFullUrl );
        ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    }
    return sName;
}

rtl::OUString
SwVbaTemplate::getPath() throw ( css::uno::RuntimeException )
{
    rtl::OUString sPath;
    if( !msFullUrl.isEmpty() )
    {
        INetURLObject aURL( msFullUrl );
        rtl::OUString sURL( aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        sURL = sURL.copy( 0, sURL.getLength() - aURL.GetLastName().getLength() - 1 );
        ::osl::File::getSystemPathFromFileURL( sURL, sPath );
    }
    return sPath;
}

uno::Any SAL_CALL
SwVbaTemplate::AutoTextEntries( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< text::XAutoTextContainer > xAutoTextContainer( xMgr->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.AutoTextContainer") ) ), uno::UNO_QUERY_THROW );

    // the default template is "Normal.dot" in Word.
    rtl::OUString sGroup( RTL_CONSTASCII_USTRINGPARAM("Normal") );
    rtl::OUString sName = getName();
    sal_Int32 nIndex = sName.lastIndexOf( sal_Unicode('.') );
    if( nIndex > 0 )
    {
        sGroup = sName.copy( 0, sName.lastIndexOf( sal_Unicode('.') ) );
    }
    String sNewGroup = lcl_CheckGroupName( sGroup );

    uno::Reference< container::XIndexAccess > xGroup;
    if( xAutoTextContainer->hasByName( sNewGroup ) )
    {
        xGroup.set( xAutoTextContainer->getByName( sNewGroup ), uno::UNO_QUERY_THROW );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Auto Text Entry doesn't exist") ), uno::Reference< uno::XInterface >() );
    }

    uno::Reference< XCollection > xCol( new SwVbaAutoTextEntries( this, mxContext, xGroup ) );
    if( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

rtl::OUString
SwVbaTemplate::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaTemplate"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
