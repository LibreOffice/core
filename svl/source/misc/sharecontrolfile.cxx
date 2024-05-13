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


#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>

#include <o3tl/enumrange.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>

#include <svl/sharecontrolfile.hxx>

using namespace ::com::sun::star;

namespace svt {


ShareControlFile::ShareControlFile( std::u16string_view aOrigURL )
    : LockFileCommon(GenerateOwnLockFileURL(aOrigURL, u".~sharing."))
{
    if ( !m_xStream.is() && !GetURL().isEmpty() )
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
        ::ucbhelper::Content aContent( GetURL(), xDummyEnv, comphelper::getProcessComponentContext() );

        uno::Reference< ucb::XContentIdentifier > xContId( aContent.get().is() ? aContent.get()->getIdentifier() : nullptr );
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
                ucb::InsertCommandArgument aInsertArg;
                SvMemoryStream aStream(0,0);
                aInsertArg.Data.set(new ::utl::OInputStreamWrapper(aStream));
                aInsertArg.ReplaceExisting = false;
                aContent.executeCommand( u"insert"_ustr, uno::Any( aInsertArg ) );

                // try to let the file be hidden if possible
                try {
                    aContent.setPropertyValue(u"IsHidden"_ustr, uno::Any( true ) );
                } catch( uno::Exception& ) {}

                // Try to open one more time
                xStream = aContent.openWriteableStreamNoLock();
            }
            else
                throw;
        }

        m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
        m_xInputStream.set( xStream->getInputStream(), uno::UNO_SET_THROW );
        m_xOutputStream.set( xStream->getOutputStream(), uno::UNO_SET_THROW );
        m_xTruncate.set( m_xOutputStream, uno::UNO_QUERY_THROW );
        m_xStream = xStream;
    }

    if ( !IsValid() )
        throw io::NotConnectedException();
}

ShareControlFile::~ShareControlFile()
{
    try
    {
        Close();
    }
    catch( uno::Exception& )
    {}
}

void ShareControlFile::Close()
{
    // if it is called outside of destructor the mutex must be locked

    if ( !m_xStream.is() )
        return;

    try
    {
        if ( m_xInputStream.is() )
            m_xInputStream->closeInput();
        if ( m_xOutputStream.is() )
            m_xOutputStream->closeOutput();
    }
    catch( uno::Exception& )
    {}

    m_xStream.clear();
    m_xInputStream.clear();
    m_xOutputStream.clear();
    m_xSeekable.clear();
    m_xTruncate.clear();
    m_aUsersData.clear();
}


std::vector< o3tl::enumarray< LockFileComponent, OUString > > ShareControlFile::GetUsersData()
{
    std::unique_lock aGuard(m_aMutex);
    return GetUsersDataImpl(aGuard);
}

std::vector< o3tl::enumarray< LockFileComponent, OUString > > ShareControlFile::GetUsersDataImpl(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( m_aUsersData.empty() )
    {
        sal_Int64 nLength = m_xSeekable->getLength();
        if ( nLength > SAL_MAX_INT32 )
            throw uno::RuntimeException();

        uno::Sequence< sal_Int8 > aBuffer( static_cast<sal_Int32>(nLength) );
        m_xSeekable->seek( 0 );

        sal_Int32 nRead = m_xInputStream->readBytes( aBuffer, static_cast<sal_Int32>(nLength) );
        auto aBufferRange = asNonConstRange(aBuffer);
        nLength -= nRead;
        while ( nLength > 0 )
        {
            uno::Sequence< sal_Int8 > aTmpBuf( static_cast<sal_Int32>(nLength) );
            nRead = m_xInputStream->readBytes( aTmpBuf, static_cast<sal_Int32>(nLength) );
            if ( nRead > nLength )
                throw uno::RuntimeException();

            for ( sal_Int32 nInd = 0; nInd < nRead; nInd++ )
                aBufferRange[aBuffer.getLength() - static_cast<sal_Int32>(nLength) + nInd] = aTmpBuf[nInd];
            nLength -= nRead;
        }

        ParseList( aBuffer, m_aUsersData );
    }

    return m_aUsersData;
}


void ShareControlFile::SetUsersDataAndStore( std::unique_lock<std::mutex>& /*rGuard*/, std::vector< LockFileEntry >&& aUsersData )
{
    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( !m_xTruncate.is() || !m_xOutputStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xTruncate->truncate();
    m_xSeekable->seek( 0 );

    OUStringBuffer aBuffer;
    for (const auto & rData : aUsersData)
    {
        for ( LockFileComponent nEntryInd : o3tl::enumrange<LockFileComponent>() )
        {
            aBuffer.append( EscapeCharacters( rData[nEntryInd] ) );
            if ( nEntryInd < LockFileComponent::LAST )
                aBuffer.append( ',' );
            else
                aBuffer.append( ';' );
        }
    }

    OString aStringData( OUStringToOString( aBuffer, RTL_TEXTENCODING_UTF8 ) );
    uno::Sequence< sal_Int8 > aData( reinterpret_cast<sal_Int8 const *>(aStringData.getStr()), aStringData.getLength() );
    m_xOutputStream->writeBytes( aData );
    m_aUsersData = aUsersData;
}


LockFileEntry ShareControlFile::InsertOwnEntry()
{
    std::unique_lock aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersDataImpl(aGuard);
    std::vector< LockFileEntry > aNewData( m_aUsersData );
    LockFileEntry aNewEntry = GenerateOwnEntry();

    bool bExists = false;
    sal_Int32 nNewInd = 0;
    for (LockFileEntry & rEntry : m_aUsersData)
    {
        if ( rEntry[LockFileComponent::LOCALHOST] == aNewEntry[LockFileComponent::LOCALHOST]
             && rEntry[LockFileComponent::SYSUSERNAME] == aNewEntry[LockFileComponent::SYSUSERNAME]
             && rEntry[LockFileComponent::USERURL] == aNewEntry[LockFileComponent::USERURL] )
        {
            if ( !bExists )
            {
                aNewData[nNewInd] = aNewEntry;
                bExists = true;
            }
        }
        else
        {
            aNewData[nNewInd] = rEntry;
        }

        nNewInd++;
    }

    if ( !bExists )
        aNewData.push_back( aNewEntry );

    SetUsersDataAndStore( aGuard, std::move(aNewData) );

    return aNewEntry;
}


bool ShareControlFile::HasOwnEntry()
{
    std::unique_lock aGuard( m_aMutex );

    if ( !IsValid() )
    {
        throw io::NotConnectedException();
    }

    GetUsersDataImpl(aGuard);
    LockFileEntry aEntry = GenerateOwnEntry();

    for (LockFileEntry & rEntry : m_aUsersData)
    {
        if ( rEntry[LockFileComponent::LOCALHOST] == aEntry[LockFileComponent::LOCALHOST] &&
             rEntry[LockFileComponent::SYSUSERNAME] == aEntry[LockFileComponent::SYSUSERNAME] &&
             rEntry[LockFileComponent::USERURL] == aEntry[LockFileComponent::USERURL] )
        {
            return true;
        }
    }

    return false;
}


void ShareControlFile::RemoveEntry()
{
    RemoveEntry(GenerateOwnEntry());
}

void ShareControlFile::RemoveEntry( const LockFileEntry& aEntry )
{
    std::unique_lock aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersDataImpl(aGuard);

    std::vector< LockFileEntry > aNewData;

    for (LockFileEntry & rEntry : m_aUsersData)
    {
        if ( rEntry[LockFileComponent::LOCALHOST] != aEntry[LockFileComponent::LOCALHOST]
             || rEntry[LockFileComponent::SYSUSERNAME] != aEntry[LockFileComponent::SYSUSERNAME]
             || rEntry[LockFileComponent::USERURL] != aEntry[LockFileComponent::USERURL] )
        {
            aNewData.push_back( rEntry );
        }
    }

    const bool bNewDataEmpty = aNewData.empty();
    SetUsersDataAndStore( aGuard, std::move(aNewData) );

    if ( bNewDataEmpty )
    {
        // try to remove the file if it is empty
        RemoveFileImpl(aGuard);
    }
}


void ShareControlFile::RemoveFile()
{
    std::unique_lock aGuard(m_aMutex);
    return RemoveFileImpl(aGuard);
}

void ShareControlFile::RemoveFileImpl(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if ( !IsValid() )
        throw io::NotConnectedException();

    Close();

    uno::Reference<ucb::XSimpleFileAccess3> xSimpleFileAccess(ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext()));
    xSimpleFileAccess->kill( GetURL() );
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
