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

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>

#include <osl/time.h>
#include <osl/security.hxx>
#include <osl/socket.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include <tools/stream.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/streamwrap.hxx>

#include <unotools/useroptions.hxx>

#include <svl/sharecontrolfile.hxx>

using namespace ::com::sun::star;

namespace svt {

// ----------------------------------------------------------------------
ShareControlFile::ShareControlFile( const ::rtl::OUString& aOrigURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: LockFileCommon( aOrigURL, xFactory, ::rtl::OUString( ".~sharing."  ) )
{
    OpenStream();

    if ( !IsValid() )
        throw io::NotConnectedException();
}

// ----------------------------------------------------------------------
ShareControlFile::~ShareControlFile()
{
    try
    {
        Close();
    }
    catch( uno::Exception& )
    {}
}

// ----------------------------------------------------------------------
void ShareControlFile::OpenStream()
{
    // if it is called outside of constructor the mutex must be locked already

    if ( !m_xStream.is() && !m_aURL.isEmpty() )
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
        ::ucbhelper::Content aContent = ::ucbhelper::Content( m_aURL, xDummyEnv, comphelper::getProcessComponentContext() );

        uno::Reference< ucb::XContentIdentifier > xContId( aContent.get().is() ? aContent.get()->getIdentifier() : 0 );
        if ( !xContId.is() || xContId->getContentProviderScheme() != "file" )
            throw io::IOException(); // the implementation supports only local files for now

        uno::Reference< io::XStream > xStream;

        // Currently the locking of the original document is intended to be used.
        // That means that the shared file should be accessed only when the original document is locked and only by user who has locked the document.
        // TODO/LATER: should the own file locking be used?

        try
        {
            xStream = aContent.openWriteableStreamNoLock();
        }
        catch ( ucb::InteractiveIOException const & e )
        {
            if ( e.Code == ucb::IOErrorCode_NOT_EXISTING )
            {
                // Create file...
                SvMemoryStream aStream(0,0);
                uno::Reference< io::XInputStream > xInput( new ::utl::OInputStreamWrapper( aStream ) );
                ucb::InsertCommandArgument aInsertArg;
                aInsertArg.Data = xInput;
                aInsertArg.ReplaceExisting = sal_False;
                aContent.executeCommand( rtl::OUString("insert"), uno::makeAny( aInsertArg ) );

                // try to let the file be hidden if possible
                try {
                    aContent.setPropertyValue( ::rtl::OUString( "IsHidden"  ), uno::makeAny( sal_True ) );
                } catch( uno::Exception& ) {}

                // Try to open one more time
                xStream = aContent.openWriteableStreamNoLock();
            }
            else
                throw;
        }

        m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
        m_xInputStream.set( xStream->getInputStream(), uno::UNO_QUERY_THROW );
        m_xOutputStream.set( xStream->getOutputStream(), uno::UNO_QUERY_THROW );
        m_xTruncate.set( m_xOutputStream, uno::UNO_QUERY_THROW );
        m_xStream = xStream;
    }
}

// ----------------------------------------------------------------------
void ShareControlFile::Close()
{
    // if it is called outside of destructor the mutex must be locked

    if ( m_xStream.is() )
    {
        try
        {
            if ( m_xInputStream.is() )
                m_xInputStream->closeInput();
            if ( m_xOutputStream.is() )
                m_xOutputStream->closeOutput();
        }
        catch( uno::Exception& )
        {}

        m_xStream = uno::Reference< io::XStream >();
        m_xInputStream = uno::Reference< io::XInputStream >();
        m_xOutputStream = uno::Reference< io::XOutputStream >();
        m_xSeekable = uno::Reference< io::XSeekable >();
        m_xTruncate = uno::Reference< io::XTruncate >();
        m_aUsersData.realloc( 0 );
    }
}

// ----------------------------------------------------------------------
uno::Sequence< uno::Sequence< ::rtl::OUString > > ShareControlFile::GetUsersData()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( !m_aUsersData.getLength() )
    {
        sal_Int64 nLength = m_xSeekable->getLength();
        if ( nLength > SAL_MAX_INT32 )
            throw uno::RuntimeException();

        uno::Sequence< sal_Int8 > aBuffer( (sal_Int32)nLength );
        m_xSeekable->seek( 0 );

        sal_Int32 nRead = m_xInputStream->readBytes( aBuffer, (sal_Int32)nLength );
        nLength -= nRead;
        while ( nLength > 0 )
        {
            uno::Sequence< sal_Int8 > aTmpBuf( (sal_Int32)nLength );
            nRead = m_xInputStream->readBytes( aTmpBuf, (sal_Int32)nLength );
            if ( nRead > nLength )
                throw uno::RuntimeException();

            for ( sal_Int32 nInd = 0; nInd < nRead; nInd++ )
                aBuffer[aBuffer.getLength() - (sal_Int32)nLength + nInd] = aTmpBuf[nInd];
            nLength -= nRead;
        }

        m_aUsersData = ParseList( aBuffer );
    }

    return m_aUsersData;
}

// ----------------------------------------------------------------------
void ShareControlFile::SetUsersDataAndStore( const uno::Sequence< uno::Sequence< ::rtl::OUString > >& aUsersData )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( !m_xTruncate.is() || !m_xOutputStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xTruncate->truncate();
    m_xSeekable->seek( 0 );

    ::rtl::OUStringBuffer aBuffer;
    for ( sal_Int32 nInd = 0; nInd < aUsersData.getLength(); nInd++ )
    {
        if ( aUsersData[nInd].getLength() != SHARED_ENTRYSIZE )
            throw lang::IllegalArgumentException();

        for ( sal_Int32 nEntryInd = 0; nEntryInd < SHARED_ENTRYSIZE; nEntryInd++ )
        {
            aBuffer.append( EscapeCharacters( aUsersData[nInd][nEntryInd] ) );
            if ( nEntryInd < SHARED_ENTRYSIZE - 1 )
                aBuffer.append( (sal_Unicode)',' );
            else
                aBuffer.append( (sal_Unicode)';' );
        }
    }

    ::rtl::OString aStringData( ::rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
    uno::Sequence< sal_Int8 > aData( (sal_Int8*)aStringData.getStr(), aStringData.getLength() );
    m_xOutputStream->writeBytes( aData );
    m_aUsersData = aUsersData;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ShareControlFile::InsertOwnEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersData();
    uno::Sequence< ::uno::Sequence< ::rtl::OUString > > aNewData( m_aUsersData.getLength() + 1 );
    uno::Sequence< ::rtl::OUString > aNewEntry = GenerateOwnEntry();

    sal_Bool bExists = sal_False;
    sal_Int32 nNewInd = 0;
    for ( sal_Int32 nInd = 0; nInd < m_aUsersData.getLength(); nInd++ )
    {
        if ( m_aUsersData[nInd].getLength() == SHARED_ENTRYSIZE )
        {
            if ( m_aUsersData[nInd][SHARED_LOCALHOST_ID] == aNewEntry[SHARED_LOCALHOST_ID]
              && m_aUsersData[nInd][SHARED_SYSUSERNAME_ID] == aNewEntry[SHARED_SYSUSERNAME_ID]
              && m_aUsersData[nInd][SHARED_USERURL_ID] == aNewEntry[SHARED_USERURL_ID] )
            {
                if ( !bExists )
                {
                    aNewData[nNewInd] = aNewEntry;
                    bExists = sal_True;
                }
            }
            else
            {
                aNewData[nNewInd] = m_aUsersData[nInd];
            }

            nNewInd++;
        }
    }

    if ( !bExists )
        aNewData[nNewInd++] = aNewEntry;

    aNewData.realloc( nNewInd );
    SetUsersDataAndStore( aNewData );

    return aNewEntry;
}

// ----------------------------------------------------------------------
bool ShareControlFile::HasOwnEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
    {
        throw io::NotConnectedException();
    }

    GetUsersData();
    uno::Sequence< ::rtl::OUString > aEntry = GenerateOwnEntry();

    for ( sal_Int32 nInd = 0; nInd < m_aUsersData.getLength(); ++nInd )
    {
        if ( m_aUsersData[nInd].getLength() == SHARED_ENTRYSIZE &&
             m_aUsersData[nInd][SHARED_LOCALHOST_ID] == aEntry[SHARED_LOCALHOST_ID] &&
             m_aUsersData[nInd][SHARED_SYSUSERNAME_ID] == aEntry[SHARED_SYSUSERNAME_ID] &&
             m_aUsersData[nInd][SHARED_USERURL_ID] == aEntry[SHARED_USERURL_ID] )
        {
            return true;
        }
    }

    return false;
}

// ----------------------------------------------------------------------
void ShareControlFile::RemoveEntry( const uno::Sequence< ::rtl::OUString >& aArgEntry )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersData();

    uno::Sequence< ::rtl::OUString > aEntry = aArgEntry;
    if ( aEntry.getLength() != SHARED_ENTRYSIZE )
        aEntry = GenerateOwnEntry();

    uno::Sequence< ::uno::Sequence< ::rtl::OUString > > aNewData( m_aUsersData.getLength() + 1 );

    sal_Int32 nNewInd = 0;
    for ( sal_Int32 nInd = 0; nInd < m_aUsersData.getLength(); nInd++ )
    {
        if ( m_aUsersData[nInd].getLength() == SHARED_ENTRYSIZE )
        {
            if ( m_aUsersData[nInd][SHARED_LOCALHOST_ID] != aEntry[SHARED_LOCALHOST_ID]
              || m_aUsersData[nInd][SHARED_SYSUSERNAME_ID] != aEntry[SHARED_SYSUSERNAME_ID]
              || m_aUsersData[nInd][SHARED_USERURL_ID] != aEntry[SHARED_USERURL_ID] )
            {
                aNewData[nNewInd] = m_aUsersData[nInd];
                nNewInd++;
            }
        }
    }

    aNewData.realloc( nNewInd );
    SetUsersDataAndStore( aNewData );

    if ( !nNewInd )
    {
        // try to remove the file if it is empty
        RemoveFile();
    }
}

// ----------------------------------------------------------------------
void ShareControlFile::RemoveFile()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    Close();

    uno::Reference<ucb::XSimpleFileAccess2> xSimpleFileAccess(ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext()));
    xSimpleFileAccess->kill( m_aURL );
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
