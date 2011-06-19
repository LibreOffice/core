/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#include <UnxCommandThread.hxx>
#include <UnxNotifyThread.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////
// UnxFilePickerCommandThread
//////////////////////////////////////////////////////////////////////////

UnxFilePickerCommandThread::UnxFilePickerCommandThread( UnxFilePickerNotifyThread *pNotifyThread, int nReadFD )
    : m_pNotifyThread( pNotifyThread ),
      m_nReadFD( nReadFD )
{
}

UnxFilePickerCommandThread::~UnxFilePickerCommandThread()
{
}

sal_Bool SAL_CALL UnxFilePickerCommandThread::result()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aResult;
}

::rtl::OUString SAL_CALL UnxFilePickerCommandThread::getCurrentFilter()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetCurrentFilter;
}

::rtl::OUString SAL_CALL UnxFilePickerCommandThread::getDirectory()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetDirectory;
}

uno::Sequence< ::rtl::OUString > SAL_CALL UnxFilePickerCommandThread::getFiles()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nSize = m_aGetFiles.size();
    uno::Sequence< ::rtl::OUString > aFiles( ( nSize > 1 )? nSize + 1: nSize );

    if ( nSize == 1 )
        aFiles[0] = m_aGetFiles.front();
    else if ( nSize > 1 )
    {
        // First entry in the sequence must be the dirname, the others are the
        // filenames, so we have to rearrange the list...

        ::rtl::OUString aFront = m_aGetFiles.front();
        sal_Int32 nLastSlash = aFront.lastIndexOf( '/' );

        aFiles[0] = ( nLastSlash >= 0 )? aFront.copy( 0, nLastSlash ): ::rtl::OUString();
        ++nLastSlash;

        sal_Int32 nIdx = 1;
        for ( ::std::list< ::rtl::OUString >::const_iterator it = m_aGetFiles.begin();
                it != m_aGetFiles.end(); ++it, ++nIdx )
        {
            sal_Int32 nLength = (*it).getLength() - nLastSlash;
            aFiles[nIdx] = ( nLength >= 0 )? (*it).copy( nLastSlash, nLength ): ::rtl::OUString();
        }
    }

    return aFiles;
}

uno::Any SAL_CALL UnxFilePickerCommandThread::getValue()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetValue;
}

void SAL_CALL UnxFilePickerCommandThread::run()
{
    if ( m_nReadFD < 0 )
        return;

    sal_Int32 nBufferSize = 1024; // 1 is for testing, 1024 for real use
    sal_Char *pBuffer = new sal_Char[nBufferSize];
    sal_Char *pBufferEnd = pBuffer + nBufferSize;

    sal_Char *pWhereToRead = pBuffer;
    sal_Char *pEntryBegin = pBuffer;
    sal_Int32 nBytesRead = 0;
    sal_Bool  bShouldExit = sal_False;
    while ( !bShouldExit && ( nBytesRead = read( m_nReadFD, pWhereToRead, pBufferEnd - pWhereToRead ) ) > 0 )
    {
        sal_Bool bFoundNL = sal_False;
        sal_Char *pWhereToReadEnd = pWhereToRead + nBytesRead;
        sal_Char *pEntryEnd = pWhereToRead;
        do {
            for ( ; pEntryEnd < pWhereToReadEnd && *pEntryEnd != '\n'; ++pEntryEnd )
                ;

            if ( pEntryEnd < pWhereToReadEnd )
            {
                bFoundNL = sal_True;
                *pEntryEnd = 0;

                if ( strcmp( pEntryBegin, "exited" ) == 0 )
                    bShouldExit = sal_True;
                else
                    handleCommand( ::rtl::OUString( pEntryBegin, pEntryEnd - pEntryBegin, RTL_TEXTENCODING_UTF8 )/*, bQuit*/ );

                pEntryBegin = pEntryEnd + 1;
            }
        } while ( pEntryEnd < pWhereToReadEnd );

        if ( bFoundNL )
        {
            if ( pEntryBegin < pBufferEnd )
                memmove( pBuffer, pEntryBegin, pWhereToReadEnd - pEntryBegin );
        }
        else
        {
            // enlarge the buffer size
            nBufferSize *= 2;
            sal_Char *pNewBuffer = new sal_Char[nBufferSize];
            if ( pEntryBegin < pBufferEnd )
                memmove( pNewBuffer, pEntryBegin, pWhereToReadEnd - pEntryBegin );

            delete[] pBuffer;
            pBuffer = pNewBuffer;
            pBufferEnd = pBuffer + nBufferSize;
        }

        pWhereToRead = pBuffer + ( pWhereToReadEnd - pEntryBegin );
        pEntryBegin = pBuffer;
    }
}

void SAL_CALL UnxFilePickerCommandThread::handleCommand( const ::rtl::OUString &rCommand )
{
    ::osl::MutexGuard aGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 0
    ::std::cerr << "UnxFilePicker received: \"" <<
        OUStringToOString( rCommand, RTL_TEXTENCODING_ASCII_US ).getStr() << "\"" << ::std::endl;
#endif

    ::std::list< ::rtl::OUString > aList = tokenize( rCommand );

    if ( aList.size() == 0 )
        return;

    ::rtl::OUString aCommandName = aList.front();
    aList.pop_front();

    if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "accept" ) ) )
    {
        m_aResult = sal_True;
        m_aExecCondition.set();
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "reject" ) ) )
    {
        m_aResult = sal_False;
        m_aExecCondition.set();
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "fileSelectionChanged" ) ) )
    {
        if ( m_pNotifyThread )
            m_pNotifyThread->fileSelectionChanged();
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "files" ) ) )
    {
        m_aGetFiles = aList;
        m_aGetFilesCondition.set();
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "value" ) ) )
    {
        ::rtl::OUString aType;
        if ( !aList.empty() )
        {
            aType = aList.front();
            aList.pop_front();
        }

        if ( aType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "bool" ) ) )
        {
            sal_Bool bValue = !aList.empty() && aList.front().equalsIgnoreAsciiCaseAscii( "true" );

            m_aGetValue <<= bValue;
            m_aGetValueCondition.set();
        }
        else if ( aType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "int" ) ) )
        {
            sal_Int32 nValue = 0;
            if ( !aList.empty() )
                nValue = aList.front().toInt32();

            m_aGetValue <<= nValue;
            m_aGetValueCondition.set();
        }
        else if ( aType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "string" ) ) )
        {
            ::rtl::OUString aValue;
            if ( !aList.empty() )
                aValue = aList.front();

            m_aGetValue <<= aValue;
            m_aGetValueCondition.set();
        }
        else if ( aType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "stringList" ) ) )
        {
            uno::Sequence< ::rtl::OUString > aSequence( aList.size() );
            sal_Int32 nIdx = 0;
            for ( ::std::list< ::rtl::OUString >::const_iterator it = aList.begin(); it != aList.end(); ++it, ++nIdx )
                aSequence[nIdx] = (*it);

            m_aGetValue <<= aSequence;
            m_aGetValueCondition.set();
        }
        else
        {
            m_aGetValue = uno::Any();
            m_aGetValueCondition.set();
        }
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "currentFilter" ) ) )
    {
        m_aGetCurrentFilter = aList.empty()? ::rtl::OUString(): aList.front();
        m_aGetCurrentFilterCondition.set();
    }
    else if ( aCommandName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "currentDirectory" ) ) )
    {
        m_aGetDirectory = aList.empty()? ::rtl::OUString(): aList.front();
        m_aGetDirectoryCondition.set();
    }
    else
    {
#if OSL_DEBUG_LEVEL > 0
        ::std::cerr << "Unrecognized command: "
            << OUStringToOString( aCommandName, RTL_TEXTENCODING_ASCII_US ).getStr() << "\"" << ::std::endl;
#endif
    }
}

::std::list< ::rtl::OUString > SAL_CALL UnxFilePickerCommandThread::tokenize( const ::rtl::OUString &rCommand )
{
    ::std::list< ::rtl::OUString > aList;
    ::rtl::OUStringBuffer aBuffer( 1024 );

    const sal_Unicode *pUnicode = rCommand.getStr();
    const sal_Unicode *pEnd     = pUnicode + rCommand.getLength();
    sal_Bool bQuoted            = sal_False;

    for ( ; pUnicode != pEnd; ++pUnicode )
    {
        if ( *pUnicode == '\\' )
        {
            ++pUnicode;
            if ( pUnicode != pEnd )
            {
                if ( *pUnicode == 'n' )
                    aBuffer.appendAscii( "\n", 1 );
                else
                    aBuffer.append( *pUnicode );
            }
        }
        else if ( *pUnicode == '"' )
            bQuoted = !bQuoted;
        else if ( *pUnicode == ' ' && !bQuoted )
            aList.push_back( aBuffer.makeStringAndClear() );
        else
            aBuffer.append( *pUnicode );
    }
    aList.push_back( aBuffer.makeStringAndClear() );

    return aList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
