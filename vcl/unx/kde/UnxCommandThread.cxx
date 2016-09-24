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

#include <UnxCommandThread.hxx>
#include <UnxNotifyThread.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

namespace {

::std::list< OUString > tokenize( const OUString &rCommand )
{
    ::std::list< OUString > aList;
    OUStringBuffer aBuffer( 1024 );

    const sal_Unicode *pUnicode = rCommand.getStr();
    const sal_Unicode *pEnd     = pUnicode + rCommand.getLength();
    bool bQuoted            = false;

    for ( ; pUnicode != pEnd; ++pUnicode )
    {
        if ( *pUnicode == '\\' )
        {
            ++pUnicode;
            if ( pUnicode != pEnd )
            {
                if ( *pUnicode == 'n' )
                    aBuffer.append( "\n" );
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

}

// UnxFilePickerCommandThread

UnxFilePickerCommandThread::UnxFilePickerCommandThread( UnxFilePickerNotifyThread *pNotifyThread, int nReadFD )
    : m_pNotifyThread( pNotifyThread ),
      m_nReadFD( nReadFD )
{
}

UnxFilePickerCommandThread::~UnxFilePickerCommandThread()
{
}

bool SAL_CALL UnxFilePickerCommandThread::result()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aResult;
}

OUString SAL_CALL UnxFilePickerCommandThread::getCurrentFilter()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetCurrentFilter;
}

OUString SAL_CALL UnxFilePickerCommandThread::getDirectory()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetDirectory;
}

uno::Sequence< OUString > SAL_CALL UnxFilePickerCommandThread::getSelectedFiles()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nSize = m_aGetFiles.size();
    uno::Sequence< OUString > aFiles( nSize );

    size_t nIdx = 0;
    for ( ::std::list< OUString >::const_iterator it = m_aGetFiles.begin();
       it != m_aGetFiles.end(); ++it, ++nIdx )
    {
        aFiles[nIdx] = *it;
    }

    return aFiles;
}
uno::Sequence< OUString > SAL_CALL UnxFilePickerCommandThread::getFiles()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< OUString > aFiles = getSelectedFiles();
    if (aFiles.getLength() > 1)
        aFiles.realloc(1); // we just want the first entry here

    return aFiles;
}

uno::Any SAL_CALL UnxFilePickerCommandThread::getValue()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_aGetValue;
}

void SAL_CALL UnxFilePickerCommandThread::run()
{
    osl_setThreadName("UnxFilePickerCommandThread");

    if ( m_nReadFD < 0 )
        return;

    sal_Int32 nBufferSize = 1024; // 1 is for testing, 1024 for real use
    sal_Char *pBuffer = new sal_Char[nBufferSize];
    sal_Char *pBufferEnd = pBuffer + nBufferSize;

    sal_Char *pWhereToRead = pBuffer;
    sal_Char *pEntryBegin = pBuffer;
    sal_Int32 nBytesRead = 0;
    bool  bShouldExit = false;
    while ( !bShouldExit && ( nBytesRead = read( m_nReadFD, pWhereToRead, pBufferEnd - pWhereToRead ) ) > 0 )
    {
        bool bFoundNL = false;
        sal_Char *pWhereToReadEnd = pWhereToRead + nBytesRead;
        sal_Char *pEntryEnd = pWhereToRead;
        do {
            for ( ; pEntryEnd < pWhereToReadEnd && *pEntryEnd != '\n'; ++pEntryEnd )
                ;

            if ( pEntryEnd < pWhereToReadEnd )
            {
                bFoundNL = true;
                *pEntryEnd = 0;

                if ( strcmp( pEntryBegin, "exited" ) == 0 )
                    bShouldExit = true;
                else
                    handleCommand( OUString( pEntryBegin, pEntryEnd - pEntryBegin, RTL_TEXTENCODING_UTF8 )/*, bQuit*/ );

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
                memcpy( pNewBuffer, pEntryBegin, pWhereToReadEnd - pEntryBegin );

            delete[] pBuffer;
            pBuffer = pNewBuffer;
            pBufferEnd = pBuffer + nBufferSize;
        }

        pWhereToRead = pBuffer + ( pWhereToReadEnd - pEntryBegin );
        pEntryBegin = pBuffer;
    }
}

void SAL_CALL UnxFilePickerCommandThread::handleCommand( const OUString &rCommand )
{
    ::osl::MutexGuard aGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 0
    ::std::cerr << "UnxFilePicker received: \"" <<
        OUStringToOString( rCommand, RTL_TEXTENCODING_ASCII_US ).getStr() << "\"" << ::std::endl;
#endif

    ::std::list< OUString > aList = tokenize( rCommand );

    if ( aList.empty() )
        return;

    OUString aCommandName = aList.front();
    aList.pop_front();

    if ( aCommandName == "accept" )
    {
        m_aResult = true;
        m_aExecCondition.set();
    }
    else if ( aCommandName == "reject" )
    {
        m_aResult = false;
        m_aExecCondition.set();
    }
    else if ( aCommandName == "fileSelectionChanged" )
    {
        if ( m_pNotifyThread )
            m_pNotifyThread->fileSelectionChanged();
    }
    else if ( aCommandName == "files" )
    {
        m_aGetFiles = aList;
        m_aGetFilesCondition.set();
    }
    else if ( aCommandName == "value" )
    {
        OUString aType;
        if ( !aList.empty() )
        {
            aType = aList.front();
            aList.pop_front();
        }

        if ( aType == "bool" )
        {
            bool bValue = !aList.empty() && aList.front().equalsIgnoreAsciiCase("true");

            m_aGetValue <<= bValue;
            m_aGetValueCondition.set();
        }
        else if ( aType == "int" )
        {
            sal_Int32 nValue = 0;
            if ( !aList.empty() )
                nValue = aList.front().toInt32();

            m_aGetValue <<= nValue;
            m_aGetValueCondition.set();
        }
        else if ( aType == "string" )
        {
            OUString aValue;
            if ( !aList.empty() )
                aValue = aList.front();

            m_aGetValue <<= aValue;
            m_aGetValueCondition.set();
        }
        else if ( aType == "stringList" )
        {
            m_aGetValue <<= comphelper::containerToSequence<OUString>(aList);
            m_aGetValueCondition.set();
        }
        else
        {
            m_aGetValue = uno::Any();
            m_aGetValueCondition.set();
        }
    }
    else if ( aCommandName == "currentFilter" )
    {
        m_aGetCurrentFilter = aList.empty()? OUString(): aList.front();
        m_aGetCurrentFilterCondition.set();
    }
    else if ( aCommandName == "currentDirectory" )
    {
        m_aGetDirectory = aList.empty()? OUString(): aList.front();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
