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
#include "vbatemplate.hxx"
#include <vbahelper/vbahelper.hxx>
#include "wordvbahelper.hxx"
#include "vbaautotextentry.hxx"
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <rtl/character.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static OUString lcl_CheckGroupName( const OUString& rGroupName )
{
    OUString sRet;
    //group name should contain only A-Z and a-z and spaces
    for( sal_Int32 i = 0; i < rGroupName.getLength(); i++ )
    {
        sal_Unicode cChar = rGroupName[i];
        if (rtl::isAsciiAlphanumeric(cChar) ||
            cChar == '_' || cChar == 0x20)
        {
            sRet += OUStringLiteral1(cChar);
        }
    }
    return comphelper::string::strip(sRet, ' ');
}

SwVbaTemplate::SwVbaTemplate( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const OUString& rFullUrl )
    : SwVbaTemplate_BASE( rParent, rContext ), msFullUrl( rFullUrl )
{
}

SwVbaTemplate::~SwVbaTemplate()
{
}

OUString
SwVbaTemplate::getName()
{
    OUString sName;
    if( !msFullUrl.isEmpty() )
    {
        INetURLObject aURL( msFullUrl );
        ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    }
    return sName;
}

OUString
SwVbaTemplate::getPath()
{
    OUString sPath;
    if( !msFullUrl.isEmpty() )
    {
        INetURLObject aURL( msFullUrl );
        OUString sURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
        sURL = sURL.copy( 0, sURL.getLength() - aURL.GetLastName().getLength() - 1 );
        ::osl::File::getSystemPathFromFileURL( sURL, sPath );
    }
    return sPath;
}

uno::Any SAL_CALL
SwVbaTemplate::AutoTextEntries( const uno::Any& index )
{
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    uno::Reference< text::XAutoTextContainer2 > xAutoTextContainer = text::AutoTextContainer::create( xContext );

    // the default template is "Normal.dot" in Word.
    OUString sGroup("Normal");
    OUString sName = getName();
    sal_Int32 nIndex = sName.lastIndexOf( '.' );
    if( nIndex > 0 )
    {
        sGroup = sName.copy( 0, sName.lastIndexOf( '.' ) );
    }
    OUString sNewGroup = lcl_CheckGroupName( sGroup );

    uno::Reference< container::XIndexAccess > xGroup;
    if( !xAutoTextContainer->hasByName( sNewGroup ) )
    {
        throw uno::RuntimeException("Auto Text Entry doesn't exist" );
    }

    xGroup.set( xAutoTextContainer->getByName( sNewGroup ), uno::UNO_QUERY_THROW );

    uno::Reference< XCollection > xCol( new SwVbaAutoTextEntries( this, mxContext, xGroup ) );
    if( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

OUString
SwVbaTemplate::getServiceImplName()
{
    return OUString("SwVbaTemplate");
}

uno::Sequence< OUString >
SwVbaTemplate::getServiceNames()
{
        static uno::Sequence< OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = "ooo.vba.word.Template";
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
