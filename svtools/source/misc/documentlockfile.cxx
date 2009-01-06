/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentlockfile.cxx,v $
 *
 * $Revision: 1.3.82.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <stdio.h>

#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>

#include <osl/time.h>
#include <osl/security.hxx>
#include <osl/socket.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>

#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>

#include <ucbhelper/content.hxx>

#include <svtools/useroptions.hxx>

#include <svtools/documentlockfile.hxx>

using namespace ::com::sun::star;

namespace svt {

sal_Bool DocumentLockFile::m_bAllowInteraction = sal_True;

// ----------------------------------------------------------------------
DocumentLockFile::DocumentLockFile( const ::rtl::OUString& aOrigURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
{
    if ( !m_xFactory.is() )
        m_xFactory = ::comphelper::getProcessServiceFactory();

    INetURLObject aDocURL( aOrigURL );
    if ( aDocURL.HasError() )
        throw lang::IllegalArgumentException();

    ::rtl::OUString aShareURLString = aDocURL.GetPartBeforeLastName();
    aShareURLString += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".~lock." ) );
    aShareURLString += aDocURL.GetName();
    aShareURLString += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "#" ) );
    m_aURL = INetURLObject( aShareURLString ).GetMainURL( INetURLObject::NO_DECODE );
}

// ----------------------------------------------------------------------
DocumentLockFile::~DocumentLockFile()
{
}

// ----------------------------------------------------------------------
void DocumentLockFile::WriteEntryToStream( uno::Sequence< ::rtl::OUString > aEntry, uno::Reference< io::XOutputStream > xOutput )
{
    ::rtl::OUStringBuffer aBuffer;

    for ( sal_Int32 nEntryInd = 0; nEntryInd < aEntry.getLength(); nEntryInd++ )
    {
        aBuffer.append( EscapeCharacters( aEntry[nEntryInd] ) );
        if ( nEntryInd < aEntry.getLength() - 1 )
            aBuffer.append( (sal_Unicode)',' );
        else
            aBuffer.append( (sal_Unicode)';' );
    }

    ::rtl::OString aStringData( ::rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
    uno::Sequence< sal_Int8 > aData( (sal_Int8*)aStringData.getStr(), aStringData.getLength() );
    xOutput->writeBytes( aData );
}

// ----------------------------------------------------------------------
sal_Bool DocumentLockFile::CreateOwnLockFile()
{
    try
    {
        uno::Reference< io::XStream > xTempFile(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY_THROW );
        uno::Reference< io::XSeekable > xSeekable( xTempFile, uno::UNO_QUERY_THROW );

        uno::Reference< io::XInputStream > xInput = xTempFile->getInputStream();
        uno::Reference< io::XOutputStream > xOutput = xTempFile->getOutputStream();

        if ( !xInput.is() || !xOutput.is() )
            throw uno::RuntimeException();

        uno::Sequence< ::rtl::OUString > aNewEntry = GenerateOwnEntry();
        WriteEntryToStream( aNewEntry, xOutput );
        xOutput->closeOutput();

        xSeekable->seek( 0 );

        uno::Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content aTargetContent( m_aURL, xEnv );

        ucb::InsertCommandArgument aInsertArg;
        aInsertArg.Data = xInput;
        aInsertArg.ReplaceExisting = sal_False;
        uno::Any aCmdArg;
        aCmdArg <<= aInsertArg;
        aTargetContent.executeCommand( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ), aCmdArg );

        // try to let the file be hidden if possible
        try {
            aTargetContent.setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsHidden" ) ), uno::makeAny( sal_True ) );
        } catch( uno::Exception& ) {}
    }
    catch( ucb::NameClashException& )
    {
        return sal_False;
    }

    return sal_True;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DocumentLockFile::ParseEntry( const uno::Sequence< sal_Int8 >& aBuffer )
{
    sal_Int32 nCurPos = 0;

    uno::Sequence< ::rtl::OUString > aResult( LOCKFILE_ENTRYSIZE );

    for ( int nInd = 0; nInd < LOCKFILE_ENTRYSIZE; nInd++ )
    {
        aResult[nInd] = ParseName( aBuffer, nCurPos );
        if ( nCurPos >= aBuffer.getLength()
          || ( nInd < LOCKFILE_ENTRYSIZE - 1 && aBuffer[nCurPos++] != ',' )
          || ( nInd == LOCKFILE_ENTRYSIZE - 1 && aBuffer[nCurPos++] != ';' ) )
            throw io::WrongFormatException();
    }

    return aResult;
}

// ----------------------------------------------------------------------
::rtl::OUString DocumentLockFile::ParseName( const uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos )
{
    ::rtl::OStringBuffer aResult;
    sal_Bool bHaveName = sal_False;
    sal_Bool bEscape = sal_False;

    while( !bHaveName )
    {
        if ( o_nCurPos >= aBuffer.getLength() )
            throw io::WrongFormatException();

        if ( bEscape )
        {
            if ( aBuffer[o_nCurPos] == ',' || aBuffer[o_nCurPos] == ';' || aBuffer[o_nCurPos] == '\\' )
                aResult.append( (sal_Char)aBuffer[o_nCurPos] );
            else
                throw io::WrongFormatException();

            bEscape = sal_False;
            o_nCurPos++;
        }
        else if ( aBuffer[o_nCurPos] == ',' || aBuffer[o_nCurPos] == ';' )
            bHaveName = sal_True;
        else
        {
            if ( aBuffer[o_nCurPos] == '\\' )
                bEscape = sal_True;
            else
                aResult.append( (sal_Char)aBuffer[o_nCurPos] );

            o_nCurPos++;
        }
    }

    return ::rtl::OStringToOUString( aResult.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

// ----------------------------------------------------------------------
::rtl::OUString DocumentLockFile::EscapeCharacters( const ::rtl::OUString& aSource )
{
    ::rtl::OUStringBuffer aBuffer;
    const sal_Unicode* pStr = aSource.getStr();
    for ( sal_Int32 nInd = 0; nInd < aSource.getLength() && pStr[nInd] != 0; nInd++ )
    {
        if ( pStr[nInd] == '\\' || pStr[nInd] == ';' || pStr[nInd] == ',' )
            aBuffer.append( (sal_Unicode)'\\' );
        aBuffer.append( pStr[nInd] );
    }

    return aBuffer.makeStringAndClear();
}

// ----------------------------------------------------------------------
::rtl::OUString DocumentLockFile::GetOOOUserName()
{
    SvtUserOptions aUserOpt;
    ::rtl::OUString aName = aUserOpt.GetFirstName();
    if ( aName.getLength() )
        aName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
    aName += aUserOpt.GetLastName();

    return aName;
}

// ----------------------------------------------------------------------
::rtl::OUString DocumentLockFile::GetCurrentLocalTime()
{
    ::rtl::OUString aTime;

    TimeValue aSysTime;
    if ( osl_getSystemTime( &aSysTime ) )
    {
        TimeValue aLocTime;
        if ( osl_getLocalTimeFromSystemTime( &aSysTime, &aLocTime ) )
        {
            oslDateTime aDateTime;
            if ( osl_getDateTimeFromTimeValue( &aLocTime, &aDateTime ) )
            {
                char pDateTime[20];
                sprintf( pDateTime, "%02d.%02d.%4d %02d:%02d", aDateTime.Day, aDateTime.Month, aDateTime.Year, aDateTime.Hours, aDateTime.Minutes );
                aTime = ::rtl::OUString::createFromAscii( pDateTime );
            }
        }
    }

    return aTime;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DocumentLockFile::GenerateOwnEntry()
{
    uno::Sequence< ::rtl::OUString > aResult( LOCKFILE_ENTRYSIZE );

    aResult[LOCKFILE_OOOUSERNAME_ID] = GetOOOUserName();

    ::osl::Security aSecurity;
    aSecurity.getUserName( aResult[LOCKFILE_SYSUSERNAME_ID] );

    aResult[LOCKFILE_LOCALHOST_ID] = ::osl::SocketAddr::getLocalHostname();

    aResult[LOCKFILE_EDITTIME_ID] = GetCurrentLocalTime();

    ::utl::Bootstrap::locateUserInstallation( aResult[LOCKFILE_USERURL_ID] );


    return aResult;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DocumentLockFile::GetLockData()
{
    uno::Reference< io::XInputStream > xInput = OpenStream();
    if ( !xInput.is() )
        throw uno::RuntimeException();

    const sal_Int32 nBufLen = 32000;
    uno::Sequence< sal_Int8 > aBuffer( nBufLen );

    sal_Int32 nRead = 0;

    nRead = xInput->readBytes( aBuffer, nBufLen );
    xInput->closeInput();

    if ( nRead == nBufLen )
        throw io::WrongFormatException();

    return ParseEntry( aBuffer );
}

// ----------------------------------------------------------------------
uno::Reference< io::XInputStream > DocumentLockFile::OpenStream()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
            xFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
            uno::UNO_QUERY_THROW );

    // the file can be opened readonly, no locking will be done
    return xSimpleFileAccess->openFileRead( m_aURL );
}

// ----------------------------------------------------------------------
sal_Bool DocumentLockFile::OverwriteOwnLockFile()
{
    // allows to overwrite the lock file with the current data
    try
    {
        uno::Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content aTargetContent( m_aURL, xEnv );

        uno::Sequence< ::rtl::OUString > aNewEntry = GenerateOwnEntry();

        uno::Reference< io::XStream > xStream = aTargetContent.openWriteableStreamNoLock();
        uno::Reference< io::XOutputStream > xOutput = xStream->getOutputStream();
        uno::Reference< io::XTruncate > xTruncate( xOutput, uno::UNO_QUERY_THROW );

        xTruncate->truncate();
        WriteEntryToStream( aNewEntry, xOutput );
        xOutput->closeOutput();
    }
    catch( ucb::NameClashException& )
    {
        return sal_False;
    }

    return sal_True;
}

// ----------------------------------------------------------------------
void DocumentLockFile::RemoveFile()
{
    // TODO/LATER: the removing is not atomar, is it possible in general to make it atomar?
    uno::Sequence< ::rtl::OUString > aNewEntry = GenerateOwnEntry();
    uno::Sequence< ::rtl::OUString > aFileData = GetLockData();

    if ( aFileData.getLength() < LOCKFILE_ENTRYSIZE )
        throw io::WrongFormatException();

    if ( !aFileData[LOCKFILE_SYSUSERNAME_ID].equals( aNewEntry[LOCKFILE_SYSUSERNAME_ID] )
      || !aFileData[LOCKFILE_LOCALHOST_ID].equals( aNewEntry[LOCKFILE_LOCALHOST_ID] )
      || !aFileData[LOCKFILE_USERURL_ID].equals( aNewEntry[LOCKFILE_USERURL_ID] ) )
        throw io::IOException(); // not the owner, access denied

    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
        xFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
        uno::UNO_QUERY_THROW );
    xSimpleFileAccess->kill( m_aURL );
}

} // namespace svt

