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

#include "dp_misc.h"
#include "dp_persmap.h"
#include "rtl/strbuf.hxx"

#ifndef DISABLE_BDB2PMAP
#  include <vector>
#endif

using namespace ::rtl;

// the persistent map is used to manage a handful of key-value string pairs
// this implementation replaces a rather heavy-weight berkeleydb integration

// the file backing up a persistent map consists of line pairs with
// - a key string   (encoded with chars 0x00..0x0F being escaped)
// - a value string (encoded with chars 0x00..0x0F being escaped)

namespace dp_misc
{

static const char PmapMagic[4] = {'P','m','p','1'};

PersistentMap::PersistentMap( OUString const & url_, bool readOnly )
:    m_MapFile( expandUnoRcUrl(url_) )
,    m_bReadOnly( readOnly )
,    m_bIsOpen( false )
,    m_bToBeCreated( !readOnly )
,    m_bIsDirty( false )
{
#ifndef DISABLE_BDB2PMAP
    m_MapFileName = expandUnoRcUrl( url_ );
#endif
    open();
}

PersistentMap::PersistentMap()
:    m_MapFile( OUString() )
,    m_bReadOnly( false )
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
    const sal_Char* pChar = rStr.getStr();
    const sal_Int32 nLen = rStr.getLength();
    sal_Int32 i = nLen;
    // short circuit for the simple non-encoded case
    while( --i >= 0)
    {
        const unsigned char c = (unsigned char) *(pChar++);
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
        unsigned char c = (unsigned char) *(pChar++);
        if( c <= 0x0F )
        {
            aEncStr.append( '%');
            c += (c <= 0x09) ? '0' : 'A'-10;
        } else if( c == '%')
            aEncStr.append( '%');
        aEncStr.append( c);
    }

    return aEncStr.makeStringAndClear();
}

// replace "%0".."%F" with 0x00..0x0F
// replace "%%" with "%"
static OString decodeString( const sal_Char* pEncChars, int nLen)
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
        sal_Char c = *(pChar++);
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

bool PersistentMap::open()
{
    // open the existing file
    sal_uInt32 nOpenFlags = osl_File_OpenFlag_Read;
    if( !m_bReadOnly)
        nOpenFlags |= osl_File_OpenFlag_Write;

    const osl::File::RC rcOpen = m_MapFile.open( nOpenFlags);
    m_bIsOpen = (rcOpen == osl::File::E_None);

    // or create later if needed
    m_bToBeCreated &= (rcOpen == osl::File::E_NOENT) && !m_bIsOpen;

#ifndef DISABLE_BDB2PMAP
    if( m_bToBeCreated )
        importFromBDB();
#endif

    if( !m_bIsOpen)
        return m_bToBeCreated;

    return readAll();
}

//______________________________________________________________________________
bool PersistentMap::readAll()
{
    // prepare for re-reading the map-file
    const osl::FileBase::RC nRes = m_MapFile.setPos( osl_Pos_Absolut, 0);
    (void)nRes;
    m_entries.clear();

    // read header and check magic
    char aHeaderBytes[ sizeof(PmapMagic)];
    sal_uInt64 nBytesRead = 0;
    m_MapFile.read( aHeaderBytes, sizeof(aHeaderBytes), nBytesRead);
    OSL_ASSERT( nBytesRead == sizeof(aHeaderBytes));
    if( nBytesRead != sizeof(aHeaderBytes))
        return false;
    // check header magic
    for( int i = 0; i < (int)sizeof(PmapMagic); ++i)
        if( aHeaderBytes[i] != PmapMagic[i])
            return false;

    // read key value pairs and add them to the map
    ByteSequence aKeyLine;
    ByteSequence aValLine;
    for(;;)
    {
        // read key-value line pair
        // an empty key name indicates the end of the line pairs
        if( m_MapFile.readLine( aKeyLine) != osl::File::E_None)
            return false;
        if( !aKeyLine.getLength())
            break;
        if( m_MapFile.readLine( aValLine) != osl::File::E_None)
            return false;
        // decode key and value strings
        const OString aKeyName = decodeString( (sal_Char*)aKeyLine.getConstArray(), aKeyLine.getLength());
        const OString aValName = decodeString( (sal_Char*)aValLine.getConstArray(), aValLine.getLength());
        // insert key-value pair into map
        add( aKeyName, aValName );
        // check end-of-file status
        sal_Bool bIsEOF = true;
        if( m_MapFile.isEndOfFile( &bIsEOF) != osl::File::E_None )
            return false;
        if( bIsEOF )
            break;
    }

    m_bIsDirty = false;
    return true;
}

void PersistentMap::flush()
{
    if( !m_bIsDirty)
        return;
    OSL_ASSERT( !m_bReadOnly);
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
    const osl::FileBase::RC nRes = m_MapFile.setPos( osl_Pos_Absolut, 0);
    (void)nRes;
    sal_uInt64 nBytesWritten = 0;
    m_MapFile.write( PmapMagic, sizeof(PmapMagic), nBytesWritten);

    // write key value pairs
    t_string2string_map::const_iterator it = m_entries.begin();
    for(; it != m_entries.end(); ++it) {
        // write line for key
        const OString aKeyString = encodeString( (*it).first);
        const sal_Int32 nKeyLen = aKeyString.getLength();
        m_MapFile.write( aKeyString.getStr(), nKeyLen, nBytesWritten);
        OSL_ASSERT( nKeyLen == (sal_Int32)nBytesWritten);
        m_MapFile.write( "\n", 1, nBytesWritten);
        // write line for value
        const OString& rValString = encodeString( (*it).second);
        const sal_Int32 nValLen = rValString.getLength();
        m_MapFile.write( rValString.getStr(), nValLen, nBytesWritten);
        OSL_ASSERT( nValLen == (sal_Int32)nBytesWritten);
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
    return get( NULL, key );
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
    if( m_bReadOnly)
        return;
    typedef std::pair<t_string2string_map::iterator,bool> InsertRC;
    InsertRC r = m_entries.insert( t_string2string_map::value_type(key,value));
    m_bIsDirty = r.second;
}

//______________________________________________________________________________
void PersistentMap::put( OString const & key, OString const & value )
{
    add( key, value);
    // HACK: flush now as the extension manager does not seem
    //       to properly destruct this object in some situations
    if(m_bIsDirty)
        flush();
}

bool PersistentMap::erase( OString const & key, bool flush_immediately )
{
    if( m_bReadOnly)
        return false;
    size_t nCount = m_entries.erase( key);
    if( !nCount)
        return false;
    m_bIsDirty = true;
    if( flush_immediately)
        flush();
    return true;
}

t_string2string_map PersistentMap::getEntries() const
{
    // TODO: return by const reference instead?
    return m_entries;
}

#ifndef DISABLE_BDB2PMAP
bool PersistentMap::importFromBDB()
{
    if( m_bReadOnly)
        return false;

    // get the name of its BDB counterpart
    rtl::OUString aDBName = m_MapFileName;
    if( !aDBName.endsWith( ".pmap" ))
        return false;
    aDBName = aDBName.replaceAt( aDBName.getLength()-5, 5, ".db");

    // open the corresponding BDB file for reading
    osl::File aDBFile( aDBName);
    osl::File::RC rc = aDBFile.open( osl_File_OpenFlag_Read);
    if( rc != osl::File::E_None)
        return false;
    sal_uInt64 nFileSize = 0;
    if( aDBFile.getSize( nFileSize) != osl::File::E_None)
        return false;

    // read the BDB file
    std::vector<sal_uInt8> aRawBDB( nFileSize);
    for( sal_uInt64 nOfs = 0; nOfs < nFileSize;) {
        sal_uInt64 nBytesRead = 0;
        rc = aDBFile.read( (void*)&aRawBDB[nOfs], nFileSize - nOfs, nBytesRead);
        if( (rc != osl::File::E_None) || !nBytesRead)
            return false;
        nOfs += nBytesRead;
    }

    // check BDB file header for non_encrypted Hash format v4..9
    if( nFileSize < 0x0100)
        return false;
    if( aRawBDB[24] != 0) // only not-encrypted migration
        return false;
    if( aRawBDB[25] != 8) // we expect a P_HASHMETA page
        return false;
    const bool bLE = (aRawBDB[12]==0x61 && aRawBDB[13]==0x15 && aRawBDB[14]==0x06);
    const bool bBE = (aRawBDB[15]==0x61 && aRawBDB[14]==0x15 && aRawBDB[13]==0x06);
    if( bBE == bLE)
        return false;
    if( (aRawBDB[16] < 4) || (9 < aRawBDB[16])) // version
        return false;
    const sal_uInt64 nPgSize = bLE
    ?    (aRawBDB[20] + (aRawBDB[21]<<8) + (aRawBDB[22]<<16) + (aRawBDB[23]<<24))
    :    (aRawBDB[23] + (aRawBDB[22]<<8) + (aRawBDB[21]<<16) + (aRawBDB[20]<<24));
    const int nPgCount = nFileSize / nPgSize;
    if( nPgCount * nPgSize != nFileSize)
        return false;

    // find PackageManager's new_style entries
    // using a simple heuristic for BDB_Hash pages
    int nEntryCount = 0;
    for( int nPgNo = 1; nPgNo < nPgCount; ++nPgNo) {
        // parse the next _db_page
        const sal_uInt8* const pPage = &aRawBDB[ nPgNo * nPgSize];
        const sal_uInt8* const pEnd = pPage + nPgSize;
        const int nHfOffset = bLE ? (pPage[22] + (pPage[23]<<8)) : (pPage[23] + (pPage[22]<<8));
        if( nHfOffset <= 0)
            continue;
        const sal_uInt8* pCur = pPage + nHfOffset;
        // iterate through the entries
        for(; pCur < pEnd; ++pCur) {
            if( pCur[0] != 0x01)
                continue;
            // get the value-candidate
            const sal_uInt8* pVal = pCur + 1;
            while( ++pCur < pEnd)
                if( (*pCur < ' ') || ((*pCur > 0x7F) && (*pCur != 0xFF)))
                    break;
            if( pCur >= pEnd)
                break;
            if( (pCur[0] != 0x01) || (pCur[1] != 0xFF))
                continue;
            const OString aVal( (sal_Char*)pVal, pCur - pVal);
            // get the key-candidate
            const sal_uInt8* pKey = pCur + 1;
            while( ++pCur < pEnd)
                if( (*pCur < ' ') || ((*pCur > 0x7F) && (*pCur != 0xFF)))
                    break;
            if( (pCur < pEnd) && (*pCur > 0x01))
                continue;
            const OString aKey( (sal_Char*)pKey, pCur - pKey);
            --pCur; // prepare for next round by rewinding to end of key-string

            // add the key/value pair
            add( aKey, aVal);
            ++nEntryCount;
        }
    }

    return (nEntryCount > 0);
}
#endif // DISABLE_BDB2PMAP

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
