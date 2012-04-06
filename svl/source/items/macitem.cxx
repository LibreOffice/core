/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#include <tools/stream.hxx>

#include <svl/macitem.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SvxMacroItem);

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxMacroItem, SfxPoolItem, new SvxMacroItem(0));

// -----------------------------------------------------------------------


SjJSbxObjectBase::~SjJSbxObjectBase()
{
}

SjJSbxObjectBase* SjJSbxObjectBase::Clone( void )
{
    return NULL;
}

SvxMacro::SvxMacro( const ::rtl::OUString &rMacName, const ::rtl::OUString &rLanguage)
    : aMacName( rMacName ), aLibName( rLanguage),
      pFunctionObject(NULL), eType( EXTENDED_STYPE)
{
    if ( rLanguage == SVX_MACRO_LANGUAGE_STARBASIC )
        eType=STARBASIC;
    else if ( rLanguage == SVX_MACRO_LANGUAGE_JAVASCRIPT )
        eType=JAVASCRIPT;
}


SvxMacro::~SvxMacro()
{
    delete pFunctionObject;
}

::rtl::OUString SvxMacro::GetLanguage()const
{
    if(eType==STARBASIC)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVX_MACRO_LANGUAGE_STARBASIC));
    }
    else if(eType==JAVASCRIPT)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVX_MACRO_LANGUAGE_JAVASCRIPT));
    }
    else if(eType==EXTENDED_STYPE)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVX_MACRO_LANGUAGE_SF));

    }
    return aLibName;
}



SvxMacro& SvxMacro::operator=( const SvxMacro& rBase )
{
    if( this != &rBase )
    {
        aMacName = rBase.aMacName;
        aLibName = rBase.aLibName;
        delete pFunctionObject;
        pFunctionObject = rBase.pFunctionObject ? rBase.pFunctionObject->Clone() : NULL;
        eType = rBase.eType;
    }
    return *this;
}

// -----------------------------------------------------------------------

SvxMacroTableDtor& SvxMacroTableDtor::operator=( const SvxMacroTableDtor& rTbl )
{
    aSvxMacroTable.clear();
    aSvxMacroTable.insert(rTbl.aSvxMacroTable.begin(), rTbl.aSvxMacroTable.end());
    return *this;
}

int SvxMacroTableDtor::operator==( const SvxMacroTableDtor& rOther ) const
{
    // Anzahl unterschiedlich => auf jeden Fall ungleich
    if ( aSvxMacroTable.size() != rOther.aSvxMacroTable.size() )
        return sal_False;

    // einzeln verleichen; wegen Performance ist die Reihenfolge wichtig
    SvxMacroTable::const_iterator it1 = aSvxMacroTable.begin();
    SvxMacroTable::const_iterator it2 = rOther.aSvxMacroTable.begin();
    for ( ; it1 != aSvxMacroTable.end(); ++it1, ++it2 )
    {
        const SvxMacro& rOwnMac = it1->second;
        const SvxMacro& rOtherMac = it2->second;
        if (    it1->first != it2->first ||
                rOwnMac.GetLibName() != rOtherMac.GetLibName() ||
                rOwnMac.GetMacName() != rOtherMac.GetMacName() )
            return sal_False;
    }

    return sal_True;
}

SvStream& SvxMacroTableDtor::Read( SvStream& rStrm, sal_uInt16 nVersion )
{
    if( SVX_MACROTBL_VERSION40 <= nVersion )
        rStrm >> nVersion;
    short nMacro;
    rStrm >> nMacro;

    for( short i = 0; i < nMacro; ++i )
    {
        sal_uInt16 nCurKey, eType = STARBASIC;
        String aLibName, aMacName;
        rStrm >> nCurKey;
        aLibName = SfxPoolItem::readByteString(rStrm);
        aMacName = SfxPoolItem::readByteString(rStrm);

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStrm >> eType;

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
        rStream << nVersion;

    rStream << (sal_uInt16)aSvxMacroTable.size();

    SvxMacroTable::const_iterator it = aSvxMacroTable.begin();
    while( it != aSvxMacroTable.end() && rStream.GetError() == SVSTREAM_OK )
    {
        const SvxMacro& rMac = it->second;
        rStream << it->first;
        SfxPoolItem::writeByteString(rStream, rMac.GetLibName());
        SfxPoolItem::writeByteString(rStream, rMac.GetMacName());

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStream << (sal_uInt16)rMac.GetScriptType();
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
sal_Bool SvxMacroTableDtor::Erase(sal_uInt16 nEvent)
{
    SvxMacroTable::iterator it = aSvxMacroTable.find(nEvent);
    if ( it != aSvxMacroTable.end())
    {
        aSvxMacroTable.erase(it);
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------

int SvxMacroItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxMacroTableDtor& rOwn = aMacroTable;
    const SvxMacroTableDtor& rOther = ( (SvxMacroItem&) rAttr ).aMacroTable;

    return rOwn == rOther;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxMacroItem::Clone( SfxItemPool* ) const
{
    return new SvxMacroItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxMacroItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText,
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
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxMacroItem::Store( SvStream& rStrm , sal_uInt16 ) const
{
    return aMacroTable.Write( rStrm );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxMacroItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    SvxMacroItem* pAttr = new SvxMacroItem( Which() );
    pAttr->aMacroTable.Read( rStrm, nVersion );
    return pAttr;
}

// -----------------------------------------------------------------------

void SvxMacroItem::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    aMacroTable.Insert( nEvent, rMacro);
}

// -----------------------------------------------------------------------

sal_uInt16 SvxMacroItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    return SOFFICE_FILEFORMAT_31 == nFileFormatVersion
                ? 0 : aMacroTable.GetVersion();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
