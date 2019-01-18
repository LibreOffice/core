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


#include <stdio.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>

#include <osl/time.h>
#include <osl/security.hxx>
#include <osl/socket.hxx>
#include <osl/file.hxx>
#include <o3tl/enumrange.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>

#include <ucbhelper/content.hxx>

#include <unotools/useroptions.hxx>

#include <salhelper/linkhelper.hxx>

#include <svl/lockfilecommon.hxx>

using namespace ::com::sun::star;

namespace svt {


LockFileCommon::LockFileCommon( const OUString& aOrigURL, const OUString& aPrefix )
{
    INetURLObject aDocURL = ResolveLinks( INetURLObject( aOrigURL ) );

    OUString aShareURLString = aDocURL.GetPartBeforeLastName();
    aShareURLString += aPrefix;
    aShareURLString += aDocURL.GetName();
    aShareURLString += "%23"; // '#'
    m_aURL = INetURLObject( aShareURLString ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
}


LockFileCommon::~LockFileCommon()
{
}


INetURLObject LockFileCommon::ResolveLinks( const INetURLObject& aDocURL ) const
{
    if ( aDocURL.HasError() )
        throw lang::IllegalArgumentException();

    OUString aURLToCheck = aDocURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    // there is currently no UCB functionality to resolve the symbolic links;
    // since the lock files are used only for local file systems the osl
    // functionality is used directly
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_FileName);
    osl::FileBase::RC eStatus = aResolver.fetchFileStatus(aURLToCheck);
    if (eStatus == osl::FileBase::E_None)
        aURLToCheck = aResolver.m_aStatus.getFileURL();
    else if (eStatus == osl::FileBase::E_MULTIHOP)
    {
        // do not allow too deep links
        throw io::IOException();
    }

    return INetURLObject( aURLToCheck );
}


void LockFileCommon::ParseList( const uno::Sequence< sal_Int8 >& aBuffer, std::vector< LockFileEntry > & aResult )
{
    sal_Int32 nCurPos = 0;
    while ( nCurPos < aBuffer.getLength() )
    {
        aResult.push_back( ParseEntry( aBuffer, nCurPos ) );
    }
}


LockFileEntry LockFileCommon::ParseEntry( const uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& io_nCurPos )
{
    LockFileEntry aResult;

    for ( LockFileComponent nInd : o3tl::enumrange<LockFileComponent>() )
    {
        aResult[nInd] = ParseName( aBuffer, io_nCurPos );
        if ( io_nCurPos >= aBuffer.getLength()
          || ( nInd < LockFileComponent::LAST && aBuffer[io_nCurPos++] != ',' )
          || ( nInd == LockFileComponent::LAST && aBuffer[io_nCurPos++] != ';' ) )
            throw io::WrongFormatException();
    }

    return aResult;
}


OUString LockFileCommon::ParseName( const uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& io_nCurPos )
{
    OStringBuffer aResult;
    bool bHaveName = false;
    bool bEscape = false;

    while( !bHaveName )
    {
        if ( io_nCurPos >= aBuffer.getLength() )
            throw io::WrongFormatException();

        if ( bEscape )
        {
            if ( aBuffer[io_nCurPos] != ',' && aBuffer[io_nCurPos] != ';' && aBuffer[io_nCurPos] != '\\' )
                throw io::WrongFormatException();

            aResult.append( static_cast<sal_Char>(aBuffer[io_nCurPos]) );

            bEscape = false;
            io_nCurPos++;
        }
        else if ( aBuffer[io_nCurPos] == ',' || aBuffer[io_nCurPos] == ';' )
            bHaveName = true;
        else
        {
            if ( aBuffer[io_nCurPos] == '\\' )
                bEscape = true;
            else
                aResult.append( static_cast<sal_Char>(aBuffer[io_nCurPos]) );

            io_nCurPos++;
        }
    }

    return OStringToOUString( aResult.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}


OUString LockFileCommon::EscapeCharacters( const OUString& aSource )
{
    OUStringBuffer aBuffer;
    const sal_Unicode* pStr = aSource.getStr();
    for ( sal_Int32 nInd = 0; nInd < aSource.getLength() && pStr[nInd] != 0; nInd++ )
    {
        if ( pStr[nInd] == '\\' || pStr[nInd] == ';' || pStr[nInd] == ',' )
            aBuffer.append( '\\' );
        aBuffer.append( pStr[nInd] );
    }

    return aBuffer.makeStringAndClear();
}


OUString LockFileCommon::GetOOOUserName()
{
    SvtUserOptions aUserOpt;
    OUString aName = aUserOpt.GetFirstName();
    if ( !aName.isEmpty() )
        aName += " ";
    aName += aUserOpt.GetLastName();

    return aName;
}


OUString LockFileCommon::GetCurrentLocalTime()
{
    OUString aTime;

    TimeValue aSysTime;
    if ( osl_getSystemTime( &aSysTime ) )
    {
        TimeValue aLocTime;
        if ( osl_getLocalTimeFromSystemTime( &aSysTime, &aLocTime ) )
        {
            oslDateTime aDateTime;
            if ( osl_getDateTimeFromTimeValue( &aLocTime, &aDateTime ) )
            {
                char pDateTime[sizeof("65535.65535.-32768 65535:65535")];
                    // reserve enough space for hypothetical max length
                sprintf( pDateTime, "%02" SAL_PRIuUINT32 ".%02" SAL_PRIuUINT32 ".%4" SAL_PRIdINT32 " %02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32, sal_uInt32(aDateTime.Day), sal_uInt32(aDateTime.Month), sal_Int32(aDateTime.Year), sal_uInt32(aDateTime.Hours), sal_uInt32(aDateTime.Minutes) );
                aTime = OUString::createFromAscii( pDateTime );
            }
        }
    }

    return aTime;
}


LockFileEntry LockFileCommon::GenerateOwnEntry()
{
    LockFileEntry aResult;

    aResult[LockFileComponent::OOOUSERNAME] = GetOOOUserName();

    ::osl::Security aSecurity;
    aSecurity.getUserName( aResult[LockFileComponent::SYSUSERNAME] );

    aResult[LockFileComponent::LOCALHOST] = ::osl::SocketAddr::getLocalHostname();

    aResult[LockFileComponent::EDITTIME] = GetCurrentLocalTime();

    ::utl::Bootstrap::locateUserInstallation( aResult[LockFileComponent::USERURL] );


    return aResult;
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
