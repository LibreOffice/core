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

#include <sal/config.h>

#include <sal/log.hxx>
#include <tools/stream.hxx>

#include <svl/macitem.hxx>
#include <stringio.hxx>

TYPEINIT1_FACTORY(SvxMacroItem, SfxPoolItem, new SvxMacroItem(0));
SfxPoolItem* SvxMacroItem::CreateDefault() { return new SvxMacroItem(0); }


SvxMacro::SvxMacro( const OUString &rMacName, const OUString &rLanguage)
    : aMacName( rMacName ), aLibName( rLanguage),
      eType( EXTENDED_STYPE)
{
    if ( rLanguage == SVX_MACRO_LANGUAGE_STARBASIC )
        eType=STARBASIC;
    else if ( rLanguage == SVX_MACRO_LANGUAGE_JAVASCRIPT )
        eType=JAVASCRIPT;
}

OUString SvxMacro::GetLanguage()const
{
    if(eType==STARBASIC)
    {
        return OUString(SVX_MACRO_LANGUAGE_STARBASIC);
    }
    else if(eType==JAVASCRIPT)
    {
        return OUString(SVX_MACRO_LANGUAGE_JAVASCRIPT);
    }
    else if(eType==EXTENDED_STYPE)
    {
        return OUString(SVX_MACRO_LANGUAGE_SF);

    }
    return aLibName;
}

SvxMacro& SvxMacro::operator=( const SvxMacro& rBase )
{
    if( this != &rBase )
    {
        aMacName = rBase.aMacName;
        aLibName = rBase.aLibName;
        eType = rBase.eType;
    }
    return *this;
}



SvxMacroTableDtor& SvxMacroTableDtor::operator=( const SvxMacroTableDtor& rTbl )
{
    aSvxMacroTable.clear();
    aSvxMacroTable.insert(rTbl.aSvxMacroTable.begin(), rTbl.aSvxMacroTable.end());
    return *this;
}

bool SvxMacroTableDtor::operator==( const SvxMacroTableDtor& rOther ) const
{
    // Count different => odd in any case
    if ( aSvxMacroTable.size() != rOther.aSvxMacroTable.size() )
        return false;

    // Compare single ones; the sequence matters due to performance reasons
    SvxMacroTable::const_iterator it1 = aSvxMacroTable.begin();
    SvxMacroTable::const_iterator it2 = rOther.aSvxMacroTable.begin();
    for ( ; it1 != aSvxMacroTable.end(); ++it1, ++it2 )
    {
        const SvxMacro& rOwnMac = it1->second;
        const SvxMacro& rOtherMac = it2->second;
        if (    it1->first != it2->first ||
                rOwnMac.GetLibName() != rOtherMac.GetLibName() ||
                rOwnMac.GetMacName() != rOtherMac.GetMacName() )
            return false;
    }

    return true;
}

SvStream& SvxMacroTableDtor::Read( SvStream& rStrm, sal_uInt16 nVersion )
{
    if( SVX_MACROTBL_VERSION40 <= nVersion )
        rStrm.ReadUInt16( nVersion );

    short nMacro(0);
    rStrm.ReadInt16(nMacro);
    if (nMacro < 0)
    {
        SAL_WARN("editeng", "Parsing error: negative value " << nMacro);
        return rStrm;
    }

    const size_t nMinStringSize = rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE ? 4 : 2;
    size_t nMinRecordSize = 2 + 2*nMinStringSize;
    if( SVX_MACROTBL_VERSION40 <= nVersion )
        nMinRecordSize+=2;

    const size_t nMaxRecords = rStrm.remainingSize() / nMinRecordSize;
    if (static_cast<size_t>(nMacro) > nMaxRecords)
    {
        SAL_WARN("editeng", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nMacro<< " claimed, truncating");
        nMacro = nMaxRecords;
    }

    for (short i = 0; i < nMacro; ++i)
    {
        sal_uInt16 nCurKey, eType = STARBASIC;
        OUString aLibName, aMacName;
        rStrm.ReadUInt16( nCurKey );
        aLibName = readByteString(rStrm);
        aMacName = readByteString(rStrm);

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStrm.ReadUInt16( eType );

        aSvxMacroTable.insert( SvxMacroTable::value_type(nCurKey, SvxMacro( aMacName, aLibName, (ScriptType)eType ) ));
    }
    return rStrm;
}


SvStream& SvxMacroTableDtor::Write( SvStream& rStream ) const
{
    sal_uInt16 nVersion = SOFFICE_FILEFORMAT_31 == rStream.GetVersion()
                                    ? SVX_MACROTBL_VERSION31
                                    : SVX_MACROTBL_AKTVERSION;

    if( SVX_MACROTBL_VERSION40 <= nVersion )
        rStream.WriteUInt16( nVersion );

    rStream.WriteUInt16( aSvxMacroTable.size() );

    SvxMacroTable::const_iterator it = aSvxMacroTable.begin();
    while( it != aSvxMacroTable.end() && rStream.GetError() == SVSTREAM_OK )
    {
        const SvxMacro& rMac = it->second;
        rStream.WriteUInt16( it->first );
        writeByteString(rStream, rMac.GetLibName());
        writeByteString(rStream, rMac.GetMacName());

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStream.WriteUInt16( rMac.GetScriptType() );
        ++it;
    }
    return rStream;
}

// returns NULL if no entry exists, or a pointer to the internal value
const SvxMacro* SvxMacroTableDtor::Get(sal_uInt16 nEvent) const
{
    SvxMacroTable::const_iterator it = aSvxMacroTable.find(nEvent);
    return it == aSvxMacroTable.end() ? NULL : &(it->second);
}

// returns NULL if no entry exists, or a pointer to the internal value
SvxMacro* SvxMacroTableDtor::Get(sal_uInt16 nEvent)
{
    SvxMacroTable::iterator it = aSvxMacroTable.find(nEvent);
    return it == aSvxMacroTable.end() ? NULL : &(it->second);
}

// return true if the key exists
bool SvxMacroTableDtor::IsKeyValid(sal_uInt16 nEvent) const
{
    SvxMacroTable::const_iterator it = aSvxMacroTable.find(nEvent);
    return it != aSvxMacroTable.end();
}

// This stores a copy of the rMacro parameter
SvxMacro& SvxMacroTableDtor::Insert(sal_uInt16 nEvent, const SvxMacro& rMacro)
{
    return aSvxMacroTable.insert( SvxMacroTable::value_type( nEvent, rMacro ) ).first->second;
}

// If the entry exists, remove it from the map and release it's storage
bool SvxMacroTableDtor::Erase(sal_uInt16 nEvent)
{
    SvxMacroTable::iterator it = aSvxMacroTable.find(nEvent);
    if ( it != aSvxMacroTable.end())
    {
        aSvxMacroTable.erase(it);
        return true;
    }
    return false;
}



bool SvxMacroItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxMacroTableDtor& rOwn = aMacroTable;
    const SvxMacroTableDtor& rOther = static_cast<const SvxMacroItem&>(rAttr).aMacroTable;

    return rOwn == rOther;
}



SfxPoolItem* SvxMacroItem::Clone( SfxItemPool* ) const
{
    return new SvxMacroItem( *this );
}


bool SvxMacroItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper *
)   const
{
/*!!!
    SvxMacroTableDtor& rTbl = (SvxMacroTableDtor&)GetMacroTable();
    SvxMacro* pMac = rTbl.First();

    while ( pMac )
    {
        rText += pMac->GetLibName();
        rText += cpDelim;
        rText += pMac->GetMacName();
        pMac = rTbl.Next();
        if ( pMac )
            rText += cpDelim;
    }
*/
    rText.clear();
    return false;
}



SvStream& SvxMacroItem::Store( SvStream& rStrm , sal_uInt16 ) const
{
    return aMacroTable.Write( rStrm );
}



SfxPoolItem* SvxMacroItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    SvxMacroItem* pAttr = new SvxMacroItem( Which() );
    pAttr->aMacroTable.Read( rStrm, nVersion );
    return pAttr;
}



void SvxMacroItem::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    aMacroTable.Insert( nEvent, rMacro);
}



sal_uInt16 SvxMacroItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    return SOFFICE_FILEFORMAT_31 == nFileFormatVersion
                ? 0 : SvxMacroTableDtor::GetVersion();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
