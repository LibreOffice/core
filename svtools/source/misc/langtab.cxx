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

#include <string_view>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/i18n/DirectionProperty.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>

#include <i18nlangtag/lang.h>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>

#include <i18nutil/unicode.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/langtab.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/charclass.hxx>
#include <tools/resary.hxx>
#include <officecfg/VCL.hxx>
#include <langtab.hrc>

using namespace ::com::sun::star;

namespace {

class SvtLanguageTableImpl
{
private:
    std::vector<std::pair<OUString, LanguageType>> m_aStrings;
    void            AddItem(const OUString& rLanguage, const LanguageType eType)
    {
        m_aStrings.emplace_back(rLanguage, eType);
    }

public:

    SvtLanguageTableImpl();

    bool            HasType( const LanguageType eType ) const;
    OUString        GetString( const LanguageType eType ) const;
    LanguageType    GetType( std::u16string_view rStr ) const;
    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
    LanguageType    GetValue(sal_uInt32 nIndex) const
    {
        return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].second : LANGUAGE_DONTKNOW;
    }
    sal_uInt32      FindIndex(LanguageType nValue) const
    {
        const size_t nItems = m_aStrings.size();
        for (size_t i = 0; i < nItems; ++i)
        {
            if (m_aStrings[i].second == nValue)
                return i;
        }
        return RESARRAY_INDEX_NOTFOUND;
    }
    void            AddEntry( const OUString& rString, const LanguageType eType);
};

SvtLanguageTableImpl& theLanguageTable()
{
    static SvtLanguageTableImpl aTable;
    return aTable;
}
}

OUString ApplyLreOrRleEmbedding( const OUString &rText )
{
    const sal_Int32 nLen = rText.getLength();
    if (nLen == 0)
        return OUString();

    constexpr sal_Unicode cLRE_Embedding  = 0x202A;      // the start char of an LRE embedding
    constexpr sal_Unicode cRLE_Embedding  = 0x202B;      // the start char of an RLE embedding
    constexpr sal_Unicode cPopDirectionalFormat = 0x202C;   // the unicode PDF (POP_DIRECTIONAL_FORMAT) char that terminates an LRE/RLE embedding

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
    for (sal_Int32 i = 0;  i < nLen && !bFound;  ++i)
    {
        i18n::DirectionProperty nDirection = rCharClass.getCharacterDirection( rText, i );
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
        aRes = OUStringChar(cStart) + aRes
            + OUStringChar(cPopDirectionalFormat);
    }

    return aRes;
}

static OUString lcl_getDescription( const LanguageTag& rTag )
{
    OUString aStr( LanguageTagIcu::getDisplayName( rTag, Application::GetSettings().GetUILanguageTag()));
    if (aStr.isEmpty() || aStr == rTag.getBcp47())
    {
        // Place in curly brackets, so all on-the-fly tags without display name
        // are grouped together at the top of a listbox (but behind the
        // "[None]" entry), and not sprinkled all over, which alphabetically
        // might make sense in an English UI only anyway. Also a visual
        // indicator that it is a programmatical name, IMHO.
        return OUString::Concat("{") + aStr + "}";
    }
    else
    {
        // The ICU display name might be identical to a predefined name or even
        // to another tag's ICU name; clarify that this is a generated name and
        // append the language tag in curly brackets to distinguish.
        return aStr + " {" + rTag.getBcp47() + "}";
    }
}

SvtLanguageTableImpl::SvtLanguageTableImpl()
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(STR_ARR_SVT_LANGUAGE_TABLE); ++i)
    {
        m_aStrings.emplace_back(SvtResId(STR_ARR_SVT_LANGUAGE_TABLE[i].first), STR_ARR_SVT_LANGUAGE_TABLE[i].second);
    }

    auto xNA = officecfg::VCL::ExtraLanguages::get();
    const uno::Sequence <OUString> rElementNames = xNA->getElementNames();
    for (const OUString& rBcp47 : rElementNames)
    {
        OUString aName;
        sal_Int32 nType = 0;
        uno::Reference <container::XNameAccess> xNB;
        xNA->getByName(rBcp47) >>= xNB;
        bool bSuccess = (xNB->getByName(u"Name"_ustr) >>= aName) &&
                        (xNB->getByName(u"ScriptType"_ustr) >>= nType);
        if (bSuccess)
        {
            LanguageTag aLang(rBcp47);
            LanguageType nLangType = aLang.getLanguageType();
            if (nType <= sal_Int32(LanguageTag::ScriptType::RTL) && nType > sal_Int32(LanguageTag::ScriptType::UNKNOWN))
                aLang.setScriptType(LanguageTag::ScriptType(nType));
            sal_uInt32 nPos = FindIndex(nLangType);
            if (nPos == RESARRAY_INDEX_NOTFOUND)
                AddEntry( (aName.isEmpty() ? lcl_getDescription(aLang) : aName), nLangType);
        }
    }
}

bool SvtLanguageTableImpl::HasType( const LanguageType eType ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType );
    sal_uInt32 nPos = FindIndex(eLang);

    return RESARRAY_INDEX_NOTFOUND != nPos && nPos < GetEntryCount();
}

bool SvtLanguageTable::HasLanguageType( const LanguageType eType )
{
    return theLanguageTable().HasType( eType );
}

OUString SvtLanguageTableImpl::GetString( const LanguageType eType ) const
{
    const LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eType);
    const sal_uInt32 nPos = (eType == LANGUAGE_PROCESS_OR_USER_DEFAULT ?
            FindIndex(LANGUAGE_SYSTEM) : FindIndex( nLang));

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < GetEntryCount() )
        return m_aStrings[nPos].first;

    // Obtain from ICU, or a geeky but usable-in-a-pinch lang-tag.
    OUString sLangTag( lcl_getDescription( LanguageTag(nLang)));
    SAL_WARN("svtools.misc", "Language: 0x"
        << std::hex << nLang
        << " with unknown name, so returning generated: "
        << sLangTag);

    // And add it to the table, so it is available in all subsequent language boxes.
    const_cast<SvtLanguageTableImpl*>(this)->AddEntry( sLangTag, nLang);

    return sLangTag;
}

OUString SvtLanguageTable::GetLanguageString( const LanguageType eType )
{
    return theLanguageTable().GetString( eType );
}

LanguageType SvtLanguageTableImpl::GetType( std::u16string_view rStr ) const
{
    LanguageType eType = LANGUAGE_DONTKNOW;
    sal_uInt32 nCount = GetEntryCount();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if (m_aStrings[i].first == rStr)
        {
            eType = GetValue(i);
            break;
        }
    }
    return eType;
}

LanguageType SvtLanguageTable::GetLanguageType( std::u16string_view rStr )
{
    return theLanguageTable().GetType( rStr );
}

sal_uInt32 SvtLanguageTableImpl::GetEntryCount() const
{
    return m_aStrings.size();
}

sal_uInt32 SvtLanguageTable::GetLanguageEntryCount()
{
    return theLanguageTable().GetEntryCount();
}


LanguageType SvtLanguageTableImpl::GetTypeAtIndex( sal_uInt32 nIndex ) const
{
    LanguageType nType = LANGUAGE_DONTKNOW;
    if (nIndex < GetEntryCount())
        nType = GetValue(nIndex);
    return nType;
}

LanguageType SvtLanguageTable::GetLanguageTypeAtIndex( sal_uInt32 nIndex )
{
    return theLanguageTable().GetTypeAtIndex( nIndex);
}

void SvtLanguageTableImpl::AddEntry( const OUString& rString, const LanguageType eType )
{
    if (LanguageTag::isOnTheFlyID(eType)
            && LanguageTag::getOnTheFlyScriptType(eType) == LanguageTag::ScriptType::UNKNOWN)
    {
        // Classify the script type to distribute the entry into the proper
        // language list later.
        LanguageTag aLanguageTag(eType);
        const sal_Int16 nScriptClass = unicode::getScriptClassFromLanguageTag( aLanguageTag);
        LanguageTag::ScriptType eScriptType;
        switch (nScriptClass)
        {
            default:
                eScriptType = LanguageTag::ScriptType::WESTERN;
                assert(!"unexpected ScriptType");
            break;
            case css::i18n::ScriptType::WEAK:
            case css::i18n::ScriptType::LATIN:
                eScriptType = LanguageTag::ScriptType::WESTERN;
            break;
            case css::i18n::ScriptType::ASIAN:
                eScriptType = LanguageTag::ScriptType::CJK;
            break;
            case css::i18n::ScriptType::COMPLEX:
                /* TODO: determine if it would be LanguageTag::ScriptType::RTL
                 * instead; could that be done by
                 * getScriptClassFromLanguageTag() as well by asking Unicode
                 * properties? */
                eScriptType = LanguageTag::ScriptType::CTL;
            break;
        }
        aLanguageTag.setScriptType( eScriptType);
    }
    AddItem( rString, eType);
}

void SvtLanguageTable::AddLanguageTag( const LanguageTag& rLanguageTag )
{
    theLanguageTable().AddEntry( lcl_getDescription(rLanguageTag), rLanguageTag.getLanguageType());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
