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

#include <com/sun/star/i18n/DirectionProperty.hpp>

#include <i18nlangtag/lang.h>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/langtab.hxx>
#include <unotools/syslocale.hxx>
#include <tools/resary.hxx>


using namespace ::com::sun::star;

class SvtLanguageTableImpl : public ResStringArray
{
public:

    SvtLanguageTableImpl();
    virtual ~SvtLanguageTableImpl();

    bool            HasType( const LanguageType eType ) const;
    const OUString  GetString( const LanguageType eType, bool bUserInterfaceSelection ) const;
    LanguageType    GetType( const OUString& rStr ) const;
    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
};

namespace {
struct theLanguageTable : public rtl::Static< SvtLanguageTableImpl, theLanguageTable > {};
}

const OUString ApplyLreOrRleEmbedding( const OUString &rText )
{
    const sal_Int32 nLen = rText.getLength();
    if (nLen == 0)
        return OUString();

    const sal_Unicode cLRE_Embedding  = 0x202A;      // the start char of an LRE embedding
    const sal_Unicode cRLE_Embedding  = 0x202B;      // the start char of an RLE embedding
    const sal_Unicode cPopDirectionalFormat = 0x202C;   // the unicode PDF (POP_DIRECTIONAL_FORMAT) char that terminates an LRE/RLE embedding

    // check if there are already embedding characters at the strings start
    // if so change nothing
    const sal_Unicode cChar = rText[0];
    if (cChar == cLRE_Embedding || cChar == cRLE_Embedding)
        return rText;

    // since we only call the function getCharacterDirection
    // it does not matter which locale the CharClass is for.
    // Thus we can readily make use of SvtSysLocale::GetCharClass()
    // which should come at no cost...
    SvtSysLocale aSysLocale;
    const CharClass &rCharClass = aSysLocale.GetCharClass();

    // we should look for the first non-neutral LTR or RTL character
    // and use that to determine the embedding of the whole text...
    // Thus we can avoid to check every character of the text.
    bool bFound     = false;
    bool bIsRtlText = false;
    for (sal_uInt16 i = 0;  i < nLen && !bFound;  ++i)
    {
        sal_Int16 nDirection = rCharClass.getCharacterDirection( rText, i );
        switch (nDirection)
        {
            case i18n::DirectionProperty_LEFT_TO_RIGHT :
            case i18n::DirectionProperty_LEFT_TO_RIGHT_EMBEDDING :
            case i18n::DirectionProperty_LEFT_TO_RIGHT_OVERRIDE :
            case i18n::DirectionProperty_EUROPEAN_NUMBER :
            case i18n::DirectionProperty_ARABIC_NUMBER :        // yes! arabic numbers are written from left to right
            {
                bIsRtlText  = false;
                bFound      = true;
                break;
            }

            case i18n::DirectionProperty_RIGHT_TO_LEFT :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE :
            {
                bIsRtlText  = true;
                bFound      = true;
                break;
            }

            default:
            {
                // nothing to be done, character is considered to be neutral we need to look further ...
            }
        }
    }

    sal_Unicode cStart  = cLRE_Embedding;   // default is to use LRE embedding characters
    if (bIsRtlText)
        cStart = cRLE_Embedding;            // then use RLE embedding

    // add embedding start and end chars to the text if the direction could be determined
    OUString aRes( rText );
    if (bFound)
    {
        aRes = OUString(cStart) + aRes + OUString(cPopDirectionalFormat);
    }

    return aRes;
}

SvtLanguageTableImpl::SvtLanguageTableImpl() :
    ResStringArray( SvtResId( STR_ARR_SVT_LANGUAGE_TABLE ) )
{
}

SvtLanguageTableImpl::~SvtLanguageTableImpl()
{
}


bool SvtLanguageTableImpl::HasType( const LanguageType eType ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType );
    sal_uInt32 nPos = FindIndex( eLang );

    return RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count();
}

bool SvtLanguageTable::HasLanguageType( const LanguageType eType )
{
    return theLanguageTable::get().HasType( eType );
}


const OUString SvtLanguageTableImpl::GetString( const LanguageType eType, bool bUserInterfaceSelection ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType, bUserInterfaceSelection);
    sal_uInt32 nPos = FindIndex( eLang );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );

    //Rather than return a fairly useless "Unknown" name, return a geeky but usable-in-a-pinch lang-tag
    OUString sLangTag(LanguageTag::convertToBcp47(eType));
    SAL_WARN("svtools.misc", "Language: 0x"
        << std::hex << eType
        << " with unknown name, so returning lang-tag of: "
        << sLangTag);

    // And add it to the table if it is an on-the-fly-id, which it usually is,
    // so it is available in all subsequent language boxes.
    if (LanguageTag::isOnTheFlyID( eType))
        const_cast<SvtLanguageTableImpl*>(this)->AddItem( sLangTag, eType);

    return sLangTag;
}

OUString SvtLanguageTable::GetLanguageString( const LanguageType eType )
{
    return theLanguageTable::get().GetString( eType, false );
}

OUString SvtLanguageTable::GetLanguageString( const LanguageType eType, bool bUserInterfaceSelection )
{
    return theLanguageTable::get().GetString( eType, bUserInterfaceSelection );
}


LanguageType SvtLanguageTableImpl::GetType( const OUString& rStr ) const
{
    LanguageType eType = LANGUAGE_DONTKNOW;
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if (ResStringArray::GetString( i ).equals(rStr))
        {
            eType = LanguageType( GetValue( i ) );
            break;
        }
    }
    return eType;
}

LanguageType SvtLanguageTable::GetLanguageType( const OUString& rStr )
{
    return theLanguageTable::get().GetType( rStr );
}


sal_uInt32 SvtLanguageTableImpl::GetEntryCount() const
{
    return Count();
}

sal_uInt32 SvtLanguageTable::GetLanguageEntryCount()
{
    return theLanguageTable::get().GetEntryCount();
}


LanguageType SvtLanguageTableImpl::GetTypeAtIndex( sal_uInt32 nIndex ) const
{
    LanguageType nType = LANGUAGE_DONTKNOW;
    if (nIndex < Count())
        nType = LanguageType( GetValue( nIndex ) );
    return nType;
}

LanguageType SvtLanguageTable::GetLanguageTypeAtIndex( sal_uInt32 nIndex )
{
    return theLanguageTable::get().GetTypeAtIndex( nIndex);
}


sal_uInt32 SvtLanguageTable::AddLanguageTag( const LanguageTag& rLanguageTag, const OUString& rString )
{
    return theLanguageTable::get().AddItem( (rString.isEmpty() ? rLanguageTag.getBcp47() : rString),
            rLanguageTag.getLanguageType());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
