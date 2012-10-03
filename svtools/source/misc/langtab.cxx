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

#include <boost/static_assert.hpp>

#include <tools/shl.hxx>

#include <com/sun/star/i18n/DirectionProperty.hpp>

#include <i18npool/lang.h>
#include <i18npool/mslangid.hxx>
#include <i18npool/languagetag.hxx>

#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/langtab.hxx>
#include <unotools/syslocale.hxx>


using namespace ::com::sun::star;

//------------------------------------------------------------------------

SVT_DLLPUBLIC const String ApplyLreOrRleEmbedding( const String &rText )
{
    const sal_uInt16 nLen = rText.Len();
    if (nLen == 0)
        return String();

    const sal_Unicode cLRE_Embedding  = 0x202A;      // the start char of an LRE embedding
    const sal_Unicode cRLE_Embedding  = 0x202B;      // the start char of an RLE embedding
    const sal_Unicode cPopDirectionalFormat = 0x202C;   // the unicode PDF (POP_DIRECTIONAL_FORMAT) char that terminates an LRE/RLE embedding

    // check if there are alreay embedding characters at the strings start
    // if so change nothing
    const sal_Unicode cChar = rText.GetBuffer()[0];
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
    String aRes( rText );
    if (bFound)
    {
        aRes.Insert( cStart, 0 );
        aRes.Insert( cPopDirectionalFormat );
    }

    return aRes;
}

//------------------------------------------------------------------------

namespace {
    BOOST_STATIC_ASSERT(
        16642 == STR_ARR_SVT_LANGUAGE_TABLE);
//        "The value of STR_ARR_SVT_LANGUAGE_TABLE has changed. wizards/com/sun/star/wizards/letter/LocaleCodes.java has this value hard coded, please adapt it to your change."
}

SvtLanguageTable::SvtLanguageTable() :
    ResStringArray( SvtResId( STR_ARR_SVT_LANGUAGE_TABLE ) )
{
}

//------------------------------------------------------------------------

SvtLanguageTable::~SvtLanguageTable()
{
}

//------------------------------------------------------------------------

const rtl::OUString SvtLanguageTable::GetString( const LanguageType eType, bool bUserInterfaceSelection ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType, bUserInterfaceSelection);
    sal_uInt32 nPos = FindIndex( eLang );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );

    //Rather than return a fairly useless "Unknown" name, return a geeky but usable-in-a-pinch lang-tag
    OUString sLangTag(LanguageTag(eType).getBcp47());
    SAL_WARN("svtools", "Language: 0x"
        << std::hex << eType
        << " with unknown name, so returning lang-tag of: "
        << sLangTag);
    return sLangTag;
}

String SvtLanguageTable::GetLanguageString( const LanguageType eType )
{
    static const SvtLanguageTable aLangTable;
    return aLangTable.GetString( eType );
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetType( const String& rStr ) const
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

//------------------------------------------------------------------------

sal_uInt32 SvtLanguageTable::GetEntryCount() const
{
    return Count();
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetTypeAtIndex( sal_uInt32 nIndex ) const
{
    LanguageType nType = LANGUAGE_DONTKNOW;
    if (nIndex < Count())
        nType = LanguageType( GetValue( nIndex ) );
    return nType;
}

//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
