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

#include <dp_misc.h>
#include <dp_persmap.h>
#include <rtl/byteseq.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

using namespace ::rtl;

// the persistent map is used to manage a handful of key-value string pairs
// this implementation replaces a rather heavy-weight berkeleydb integration

// the file backing up a persistent map consists of line pairs with
// - a key string   (encoded with chars 0x00..0x0F being escaped)
// - a value string (encoded with chars 0x00..0x0F being escaped)

namespace dp_misc
{

const char PmapMagic[4] = {'P','m','p','1'};

PersistentMap::PersistentMap( OUString const & url_ )
:    m_MapFile( expandUnoRcUrl(url_) )
,    m_bIsOpen( false )
,    m_bToBeCreated( true )
,    m_bIsDirty( false )
{
    open();
}

PersistentMap::PersistentMap()
:    m_MapFile( OUString() )
,    m_bIsOpen( false )
,    m_bToBeCreated( false )
,    m_bIsDirty( false )
{}

PersistentMap::~PersistentMap()
{
    if( m_bIsDirty )
        flush();
    if( m_bIsOpen )
        m_MapFile.close();
}


// replace 0x00..0x0F with "%0".."%F"
// replace "%" with "%%"
static OString encodeString( const OString& rStr)
{
    const char* pChar = rStr.getStr();
    const sal_Int32 nLen = rStr.getLength();
    sal_Int32 i = nLen;
    // short circuit for the simple non-encoded case
    while( --i >= 0)
    {
        const unsigned char c = static_cast<unsigned char>(*(pChar++));
        if( c <= 0x0F )
            break;
        if( c == '%')
            break;
    }
    if( i < 0)
        return rStr;

    // escape chars 0x00..0x0F with "%0".."%F"
    OStringBuffer aEncStr( nLen + 32);
    aEncStr.append( pChar - (nLen-i), nLen - i);
    while( --i >= 0)
    {
        unsigned char c = static_cast<unsigned char>(*(pChar++));
        if( c <= 0x0F )
        {
            aEncStr.append( '%');
            c += (c <= 0x09) ? '0' : 'A'-10;
        } else if( c == '%')
            aEncStr.append( '%');
        aEncStr.append( char(c) );
    }

    return aEncStr.makeStringAndClear();
}

// replace "%0".."%F" with 0x00..0x0F
// replace "%%" with "%"
static OString decodeString( const char* pEncChars, int nLen)
{
    const char* pChar = pEncChars;
    sal_Int32 i = nLen;
    // short circuit for the simple non-encoded case
    while( --i >= 0)
        if( *(pChar++) == '%')
            break;
    if( i < 0)
        return OString( pEncChars, nLen);

    // replace escaped chars with their decoded counterparts
    OStringBuffer aDecStr( nLen);
    pChar = pEncChars;
    for( i = nLen; --i >= 0;)
    {
        char c = *(pChar++);
        // handle escaped character
        if( c == '%')
        {
            --i;
            OSL_ASSERT( i >= 0);
            c = *(pChar++);
            if( ('0' <= c) && (c <= '9'))
                c -= '0';
            else
            {
                OSL_ASSERT( ('A' <= c) && (c <= 'F'));
                c -= ('A'-10);
            }
        }
        aDecStr.append( c);
    }

    return aDecStr.makeStringAndClear();
}

void PersistentMap::open()
{
    // open the existing file
    sal_uInt32 const nOpenFlags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;

    const osl::File::RC rcOpen = m_MapFile.open( nOpenFlags);
    m_bIsOpen = (rcOpen == osl::File::E_None);

    // or create later if needed
    m_bToBeCreated &= (rcOpen == osl::File::E_NOENT) && !m_bIsOpen;

    if( !m_bIsOpen)
        return;

    readAll();
}


void PersistentMap::readAll()
{
    // prepare for re-reading the map-file
    m_entries.clear();
    const osl::FileBase::RC nRes = m_MapFile.setPos( osl_Pos_Absolute, 0);
    if (nRes != osl::FileBase::E_None)
    {
        SAL_WARN("desktop.deployment", "setPos failed with " << +nRes);
        return;
    }

    // read header and check magic
    char aHeaderBytes[ sizeof(PmapMagic)];
    sal_uInt64 nBytesRead = 0;
    m_MapFile.read( aHeaderBytes, sizeof(aHeaderBytes), nBytesRead);
    OSL_ASSERT( nBytesRead == sizeof(aHeaderBytes));
    if( nBytesRead != sizeof(aHeaderBytes))
        return;
    // check header magic
    for( int i = 0; i < int(sizeof(PmapMagic)); ++i)
        if( aHeaderBytes[i] != PmapMagic[i])
            return;

    // read key value pairs and add them to the map
    ByteSequence aKeyLine;
    ByteSequence aValLine;
    for(;;)
    {
        // read key-value line pair
        // an empty key name indicates the end of the line pairs
        if( m_MapFile.readLine( aKeyLine) != osl::File::E_None)
            return;
        if( !aKeyLine.getLength())
            break;
        if( m_MapFile.readLine( aValLine) != osl::File::E_None)
            return;
        // decode key and value strings
        const OString aKeyName = decodeString( reinterpret_cast<char const *>(aKeyLine.getConstArray()), aKeyLine.getLength());
        const OString aValName = decodeString( reinterpret_cast<char const *>(aValLine.getConstArray()), aValLine.getLength());
        // insert key-value pair into map
        add( aKeyName, aValName );
        // check end-of-file status
        sal_Bool bIsEOF = true;
        if( m_MapFile.isEndOfFile( &bIsEOF) != osl::File::E_None )
            return;
        if( bIsEOF )
            break;
    }

    m_bIsDirty = false;
}

void PersistentMap::flush()
{
    if( !m_bIsDirty)
        return;
    if( m_bToBeCreated && !m_entries.empty())
    {
        const sal_uInt32 nOpenFlags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create;
        const osl::File::RC rcOpen = m_MapFile.open( nOpenFlags);
        m_bIsOpen = (rcOpen == osl::File::E_None);
        m_bToBeCreated = !m_bIsOpen;
    }
    if( !m_bIsOpen)
        return;

    // write header magic
    const osl::FileBase::RC nRes = m_MapFile.setPos( osl_Pos_Absolute, 0);
    if (nRes != osl::FileBase::E_None)
    {
        SAL_WARN("desktop.deployment", "setPos failed with " << +nRes);
        return;
    }
    sal_uInt64 nBytesWritten = 0;
    m_MapFile.write( PmapMagic, sizeof(PmapMagic), nBytesWritten);

    // write key value pairs
    for (auto const& entry : m_entries)
    {
        // write line for key
        const OString aKeyString = encodeString( entry.first);
        const sal_Int32 nKeyLen = aKeyString.getLength();
        m_MapFile.write( aKeyString.getStr(), nKeyLen, nBytesWritten);
        OSL_ASSERT( nKeyLen == static_cast<sal_Int32>(nBytesWritten));
        m_MapFile.write( "\n", 1, nBytesWritten);
        // write line for value
        const OString& rValString = encodeString( entry.second);
        const sal_Int32 nValLen = rValString.getLength();
        m_MapFile.write( rValString.getStr(), nValLen, nBytesWritten);
        OSL_ASSERT( nValLen == static_cast<sal_Int32>(nBytesWritten));
        m_MapFile.write( "\n", 1, nBytesWritten);
    }

    // write a file delimiter (an empty key-string)
    m_MapFile.write( "\n", 1, nBytesWritten);
    // truncate file here
    sal_uInt64 nNewFileSize;
    if( m_MapFile.getPos( nNewFileSize) == osl::File::E_None)
        m_MapFile.setSize( nNewFileSize);
    // flush to disk
    m_MapFile.sync();
    // the in-memory map now matches to the file on disk
    m_bIsDirty = false;
}

bool PersistentMap::has( OString const & key ) const
{
    return get( nullptr, key );
}

bool PersistentMap::get( OString * value, OString const & key ) const
{
    t_string2string_map::const_iterator it = m_entries.find( key);
    if( it == m_entries.end())
        return false;
    if( value)
        *value = it->second;
    return true;
}

void PersistentMap::add( OString const & key, OString const & value )
{
    auto r = m_entries.emplace(key,value);
    m_bIsDirty = r.second;
}


void PersistentMap::put( OString const & key, OString const & value )
{
    add( key, value);
    // HACK: flush now as the extension manager does not seem
    //       to properly destruct this object in some situations
    if(m_bIsDirty)
        flush();
}

bool PersistentMap::erase( OString const & key )
{
    size_t nCount = m_entries.erase( key);
    if( !nCount)
        return false;
    m_bIsDirty = true;
    flush();
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
