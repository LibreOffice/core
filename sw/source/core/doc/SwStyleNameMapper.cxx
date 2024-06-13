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

#include <SwStyleNameMapper.hxx>
#include <poolfmt.hxx>
#include <strings.hrc>
#include <swtypes.hxx>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/string_view.hxx>
#include <map>

#ifdef _NEED_TO_DEBUG_MAPPING
#include <stdlib.h>
#endif

namespace
{

const OUString &
lcl_GetSpecialExtraName(const OUString& rExtraName, const bool bIsUIName )
{
    const std::vector<OUString>& rExtraArr = bIsUIName
        ? SwStyleNameMapper::GetExtraUINameArray()
        : SwStyleNameMapper::GetExtraProgNameArray();
    static const sal_uInt16 nIds[] =
    {
        RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_FIGURE-RES_POOLCOLL_EXTRA_BEGIN,
        0
    };
    const sal_uInt16 * pIds;
    for ( pIds = nIds; *pIds; ++pIds)
    {
        if (rExtraName == rExtraArr[ *pIds ])
        {
            return bIsUIName
                ? SwStyleNameMapper::GetExtraProgNameArray()[*pIds]
                : SwStyleNameMapper::GetExtraUINameArray()[*pIds];
        }
    }
    return rExtraName;
}

bool lcl_SuffixIsUser(std::u16string_view rString)
{
    // Interesting, why the rest must be longer than 1 character? It is so
    // since commit 4fbc9dd48b7cebb304010e7337b1bbc3936c7923 (2001-08-16)
    return rString.size() > 8 && o3tl::ends_with(rString, u" (user)");
}

void lcl_CheckSuffixAndDelete(OUString & rString)
{
    if (lcl_SuffixIsUser(rString))
    {
        rString = rString.copy(0, rString.getLength() - 7);
    }
}

NameToIdHash HashFromRange(sal_uInt16 nAcc) { return NameToIdHash(nAcc); }
template <typename... Rest>
NameToIdHash HashFromRange(sal_uInt16 nAcc, sal_uInt16 nBegin, sal_uInt16 nEnd,
                           const std::vector<OUString>& (*pFunc)(), Rest... rest)
{
    NameToIdHash hash(HashFromRange(nAcc + nEnd - nBegin, rest...));
    sal_uInt16 nIndex, nId;
    const std::vector<OUString>& rStrings = pFunc();
    for (nIndex = 0, nId = nBegin; nId < nEnd; nId++, nIndex++)
        hash[rStrings[nIndex]] = nId;
    return hash;
}

template <auto initFunc> struct TablePair
{
    static const NameToIdHash& getMap(bool bProgName)
    {
        if (bProgName)
        {
            static const NameToIdHash s_aProgMap(initFunc(true));
            return s_aProgMap;
        }

        SvtSysLocale aSysLocale;
        const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
        static std::map<LanguageTag, NameToIdHash> s_aUIMap;

        auto it = s_aUIMap.find(rCurrentLanguage);
        if (it == s_aUIMap.end())
            it = s_aUIMap.emplace(rCurrentLanguage, initFunc(false)).first;

        return it->second;
    }
};

NameToIdHash GetParaMap (bool bProgName)
{
    return HashFromRange(0,
        RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END, bProgName ? &SwStyleNameMapper::GetTextProgNameArray : &SwStyleNameMapper::GetTextUINameArray,
        RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END, bProgName ? &SwStyleNameMapper::GetListsProgNameArray : &SwStyleNameMapper::GetListsUINameArray,
        RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END, bProgName ? &SwStyleNameMapper::GetExtraProgNameArray : &SwStyleNameMapper::GetExtraUINameArray,
        RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END, bProgName ? &SwStyleNameMapper::GetRegisterProgNameArray : &SwStyleNameMapper::GetRegisterUINameArray,
        RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END, bProgName ? &SwStyleNameMapper::GetDocProgNameArray : &SwStyleNameMapper::GetDocUINameArray,
        RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END, bProgName ? &SwStyleNameMapper::GetHTMLProgNameArray : &SwStyleNameMapper::GetHTMLUINameArray
    );
};

NameToIdHash GetCharMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END, bProgName ? &SwStyleNameMapper::GetChrFormatProgNameArray : &SwStyleNameMapper::GetChrFormatUINameArray,
        RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END, bProgName ? &SwStyleNameMapper::GetHTMLChrFormatProgNameArray : &SwStyleNameMapper::GetHTMLChrFormatUINameArray
    );
};

NameToIdHash GetFrameMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLFRM_BEGIN, RES_POOLFRM_END, bProgName ? &SwStyleNameMapper::GetFrameFormatProgNameArray : &SwStyleNameMapper::GetFrameFormatUINameArray
    );
};

NameToIdHash GetPageMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLPAGE_BEGIN, RES_POOLPAGE_END, bProgName ? &SwStyleNameMapper::GetPageDescProgNameArray : &SwStyleNameMapper::GetPageDescUINameArray
    );
};

NameToIdHash GetNumRuleMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END, bProgName ? &SwStyleNameMapper::GetNumRuleProgNameArray : &SwStyleNameMapper::GetNumRuleUINameArray
    );
};

NameToIdHash GetTableStyleMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLTABLESTYLE_BEGIN, RES_POOLTABLESTYLE_END, bProgName ? &SwStyleNameMapper::GetTableStyleProgNameArray : &SwStyleNameMapper::GetTableStyleUINameArray
    );
};

NameToIdHash GetCellStyleMap(bool bProgName)
{
    return HashFromRange(0,
        RES_POOLCELLSTYLE_BEGIN, RES_POOLCELLSTYLE_END, bProgName ? &SwStyleNameMapper::GetCellStyleProgNameArray : &SwStyleNameMapper::GetCellStyleUINameArray
    );
};

} // namespace

#ifdef _NEED_TO_DEBUG_MAPPING
void SwStyleNameMapper::testNameTable( SwGetPoolIdFromName const nFamily, sal_uInt16 const nStartIndex, sal_uInt16 const nEndIndex )
{
    sal_uInt16 nIndex;
    sal_uInt16 nId;

    for ( nIndex = 0, nId = nStartIndex ; nId < nEndIndex ; nId++,nIndex++ )
    {
        OUString aString, bString;
        FillUIName ( nId, aString );
        bString = GetProgName ( nFamily, aString );
        sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nFamily );
        FillProgName ( nNewId, aString );
        bString = GetUIName ( aString, nFamily );
        nNewId = GetPoolIdFromUIName ( aString, nFamily );
        if ( nNewId != nId )
            abort();
    }
}
#endif

const NameToIdHash & SwStyleNameMapper::getHashTable ( SwGetPoolIdFromName eFlags, bool bProgName )
{
#ifdef _NEED_TO_DEBUG_MAPPING
    static bool bTested = false;
    if ( !bTested )
    {
        bTested = true;

        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END );
        testNameTable( SwGetPoolIdFromName::FrmFmt, RES_POOLFRM_BEGIN, RES_POOLFRM_END );
        testNameTable( SwGetPoolIdFromName::PageDesc, RES_POOLPAGE_BEGIN, RES_POOLPAGE_END );
        testNameTable( SwGetPoolIdFromName::NumRule, RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END );
    }
#endif

    switch ( eFlags )
    {
        case SwGetPoolIdFromName::TxtColl:
            return TablePair<GetParaMap>::getMap(bProgName);
        case SwGetPoolIdFromName::ChrFmt:
            return TablePair<GetCharMap>::getMap(bProgName);
        case SwGetPoolIdFromName::FrmFmt:
            return TablePair<GetFrameMap>::getMap(bProgName);
        case SwGetPoolIdFromName::PageDesc:
            return TablePair<GetPageMap>::getMap(bProgName);
        case SwGetPoolIdFromName::NumRule:
            return TablePair<GetNumRuleMap>::getMap(bProgName);
        case SwGetPoolIdFromName::TabStyle:
            return TablePair<GetTableStyleMap>::getMap(bProgName);
        case SwGetPoolIdFromName::CellStyle:
            return TablePair<GetCellStyleMap>::getMap(bProgName);
    }

    assert(false); // must not reach here
    abort();
}

// This gets the UI name from the programmatic name
const OUString& SwStyleNameMapper::GetUIName(const OUString& rName,
                                             SwGetPoolIdFromName const eFlags)
{
    sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
    return nId != USHRT_MAX ? GetUIName( nId, rName ) : rName;
}

// Get the programmatic name from the UI name
const OUString& SwStyleNameMapper::GetProgName(
        const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
    return nId != USHRT_MAX ? GetProgName( nId, rName ) : rName;
}

// Get the programmatic name from the UI name in rName and put it into rFillName
void SwStyleNameMapper::FillProgName(
        const OUString& rName, OUString& rFillName,
        SwGetPoolIdFromName const eFlags)
{
    sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
    if ( nId == USHRT_MAX )
    {
        // rName isn't in our UI name table...check if it's in the programmatic one
        nId = GetPoolIdFromProgName ( rName, eFlags );

        rFillName = rName;
        if (nId == USHRT_MAX )
        {
            // It isn't ...make sure the suffix isn't already " (user)"...if it is,
            // we need to add another one
            if (lcl_SuffixIsUser(rFillName))
                rFillName += " (user)";
        }
        else
        {
            // It's in the programmatic name table...append suffix
            rFillName += " (user)";
        }
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId(nId, rFillName, true);
    }

    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName == SwResId(STR_POOLCHR_STANDARD))
        rFillName = "Standard";
}

// Get the UI name from the programmatic name in rName and put it into rFillName
void SwStyleNameMapper::FillUIName(
        const OUString& rName, OUString& rFillName,
        SwGetPoolIdFromName const eFlags)
{
    OUString aName = rName;
    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName == "Standard")
        aName = SwResId(STR_POOLCHR_STANDARD);

    sal_uInt16 nId = GetPoolIdFromProgName ( aName, eFlags );
    if ( nId == USHRT_MAX )
    {
        rFillName = aName;
        // aName isn't in our Prog name table...check if it has a " (user)" suffix, if so remove it
        lcl_CheckSuffixAndDelete ( rFillName );
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId(nId, rFillName, false);
    }
}

const OUString& SwStyleNameMapper::getNameFromId(
        sal_uInt16 const nId, const OUString& rFillName, bool const bProgName)
{
    sal_uInt16 nStt = 0;
    const std::vector<OUString>* pStrArr = nullptr;

    switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & nId )
    {
    case COLL_TEXT_BITS:
        if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        {
            pStrArr = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
            nStt = RES_POOLCOLL_TEXT_BEGIN;
        }
        break;
    case COLL_LISTS_BITS:
        if( RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END )
        {
            pStrArr = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
            nStt = RES_POOLCOLL_LISTS_BEGIN;
        }
        break;
    case COLL_EXTRA_BITS:
        if( RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END )
        {
            pStrArr = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
            nStt = RES_POOLCOLL_EXTRA_BEGIN;
        }
        break;
    case COLL_REGISTER_BITS:
        if( RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END )
        {
            pStrArr = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
            nStt = RES_POOLCOLL_REGISTER_BEGIN;
        }
        break;
    case COLL_DOC_BITS:
        if( RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END )
        {
            pStrArr = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
            nStt = RES_POOLCOLL_DOC_BEGIN;
        }
        break;
    case COLL_HTML_BITS:
        if( RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
            nStt = RES_POOLCOLL_HTML_BEGIN;
        }
        break;
    case POOLGRP_CHARFMT:
        if( RES_POOLCHR_NORMAL_BEGIN <= nId && nId < RES_POOLCHR_NORMAL_END )
        {
            pStrArr = bProgName ? &GetChrFormatProgNameArray() : &GetChrFormatUINameArray();
            nStt = RES_POOLCHR_NORMAL_BEGIN;
        }
        else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLChrFormatProgNameArray() : &GetHTMLChrFormatUINameArray();
            nStt = RES_POOLCHR_HTML_BEGIN;
        }
        break;
    case POOLGRP_FRAMEFMT:
        if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
        {
            pStrArr = bProgName ? &GetFrameFormatProgNameArray() : &GetFrameFormatUINameArray();
            nStt = RES_POOLFRM_BEGIN;
        }
        break;
    case POOLGRP_PAGEDESC:
        if( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END )
        {
            pStrArr = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
            nStt = RES_POOLPAGE_BEGIN;
        }
        break;
    case POOLGRP_NUMRULE:
        if( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END )
        {
            pStrArr = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
            nStt = RES_POOLNUMRULE_BEGIN;
        }
        break;
    case POOLGRP_TABSTYLE:
        if( RES_POOLTABLESTYLE_BEGIN <= nId && nId < RES_POOLTABLESTYLE_END )
        {
            pStrArr = bProgName ? &GetTableStyleProgNameArray() : &GetTableStyleUINameArray();
            nStt = RES_POOLTABLESTYLE_BEGIN;
        }
        break;
    }
    return pStrArr ? pStrArr->operator[](nId - nStt) : rFillName;
}

void SwStyleNameMapper::fillNameFromId(
        sal_uInt16 const nId, OUString& rFillName, bool bProgName)
{
    rFillName = getNameFromId(nId, rFillName, bProgName);
}

// Get the UI name from the pool ID
void SwStyleNameMapper::FillUIName(sal_uInt16 const nId, OUString& rFillName)
{
    fillNameFromId(nId, rFillName, false);
}

// Get the UI name from the pool ID
const OUString& SwStyleNameMapper::GetUIName(
        sal_uInt16 const nId, const OUString& rName)
{
    return getNameFromId(nId, rName, false);
}

// Get the programmatic name from the pool ID
void SwStyleNameMapper::FillProgName(sal_uInt16 nId, OUString& rFillName)
{
    fillNameFromId(nId, rFillName, true);
}

// Get the programmatic name from the pool ID
const OUString&
SwStyleNameMapper::GetProgName(sal_uInt16 const nId, const OUString& rName)
{
    return getNameFromId(nId, rName, true);
}

// This gets the PoolId from the UI Name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromUIName(
        const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, false );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName);
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}

// Get the Pool ID from the programmatic name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromProgName(
            const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, true );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName);
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}

// Hard coded Programmatic Name tables

/// returns an empty array because Cell Names aren't translated
const std::vector<OUString>& SwStyleNameMapper::GetCellStyleUINameArray()
{
    static const std::vector<OUString> s_aCellStyleUINameArray;
    return s_aCellStyleUINameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTextProgNameArray()
{
    static const std::vector<OUString> s_aTextProgNameArray = {
        u"Standard"_ustr, // RES_POOLCOLL_STANDARD
        u"Text body"_ustr,
        u"First line indent"_ustr,
        u"Hanging indent"_ustr,
        u"Text body indent"_ustr,
        u"Salutation"_ustr,
        u"Signature"_ustr,
        u"List Indent"_ustr, // RES_POOLCOLL_CONFRONTATION
        u"Marginalia"_ustr,
        u"Heading"_ustr,
        u"Heading 1"_ustr,
        u"Heading 2"_ustr,
        u"Heading 3"_ustr,
        u"Heading 4"_ustr,
        u"Heading 5"_ustr,
        u"Heading 6"_ustr,
        u"Heading 7"_ustr,
        u"Heading 8"_ustr,
        u"Heading 9"_ustr,
        u"Heading 10"_ustr, // RES_POOLCOLL_TEXT_END
    };
    return s_aTextProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetListsProgNameArray()
{
    static const std::vector<OUString> s_aListsProgNameArray = {
        u"List"_ustr, // STR_POCO_PRGM_NUMBER_BULLET_BASE
        u"Numbering 1 Start"_ustr, // STR_POCO_PRGM_NUM_LEVEL1S
        u"Numbering 1"_ustr,
        u"Numbering 1 End"_ustr,
        u"Numbering 1 Cont."_ustr,
        u"Numbering 2 Start"_ustr,
        u"Numbering 2"_ustr,
        u"Numbering 2 End"_ustr,
        u"Numbering 2 Cont."_ustr,
        u"Numbering 3 Start"_ustr,
        u"Numbering 3"_ustr,
        u"Numbering 3 End"_ustr,
        u"Numbering 3 Cont."_ustr,
        u"Numbering 4 Start"_ustr,
        u"Numbering 4"_ustr,
        u"Numbering 4 End"_ustr,
        u"Numbering 4 Cont."_ustr,
        u"Numbering 5 Start"_ustr,
        u"Numbering 5"_ustr,
        u"Numbering 5 End"_ustr,
        u"Numbering 5 Cont."_ustr,
        u"List 1 Start"_ustr,
        u"List 1"_ustr,
        u"List 1 End"_ustr,
        u"List 1 Cont."_ustr,
        u"List 2 Start"_ustr,
        u"List 2"_ustr,
        u"List 2 End"_ustr,
        u"List 2 Cont."_ustr,
        u"List 3 Start"_ustr,
        u"List 3"_ustr,
        u"List 3 End"_ustr,
        u"List 3 Cont."_ustr,
        u"List 4 Start"_ustr,
        u"List 4"_ustr,
        u"List 4 End"_ustr,
        u"List 4 Cont."_ustr,
        u"List 5 Start"_ustr,
        u"List 5"_ustr,
        u"List 5 End"_ustr,
        u"List 5 Cont."_ustr, // STR_POCO_PRGM_BULLET_NONUM5
    };
    return s_aListsProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetExtraProgNameArray()
{
    static const std::vector<OUString> s_aExtraProgNameArray = {
        u"Header and Footer"_ustr, // RES_POOLCOLL_EXTRA_BEGIN
        u"Header"_ustr,
        u"Header left"_ustr,
        u"Header right"_ustr,
        u"Footer"_ustr,
        u"Footer left"_ustr,
        u"Footer right"_ustr,
        u"Table Contents"_ustr,
        u"Table Heading"_ustr,
        u"Caption"_ustr,
        u"Illustration"_ustr,
        u"Table"_ustr,
        u"Text"_ustr,
        u"Figure"_ustr, // RES_POOLCOLL_LABEL_FIGURE
        u"Frame contents"_ustr,
        u"Footnote"_ustr,
        u"Addressee"_ustr,
        u"Sender"_ustr,
        u"Endnote"_ustr,
        u"Drawing"_ustr,
        u"Comment"_ustr, // RES_POOLCOLL_COMMENT
    };
    return s_aExtraProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetRegisterProgNameArray()
{
    static const std::vector<OUString> s_aRegisterProgNameArray = {
        u"Index"_ustr, // STR_POCO_PRGM_REGISTER_BASE
        u"Index Heading"_ustr, // STR_POCO_PRGM_TOX_IDXH
        u"Index 1"_ustr,
        u"Index 2"_ustr,
        u"Index 3"_ustr,
        u"Index Separator"_ustr,
        u"Contents Heading"_ustr,
        u"Contents 1"_ustr,
        u"Contents 2"_ustr,
        u"Contents 3"_ustr,
        u"Contents 4"_ustr,
        u"Contents 5"_ustr,
        u"User Index Heading"_ustr,
        u"User Index 1"_ustr,
        u"User Index 2"_ustr,
        u"User Index 3"_ustr,
        u"User Index 4"_ustr,
        u"User Index 5"_ustr,
        u"Contents 6"_ustr,
        u"Contents 7"_ustr,
        u"Contents 8"_ustr,
        u"Contents 9"_ustr,
        u"Contents 10"_ustr,
        u"Figure Index Heading"_ustr,
        u"Figure Index 1"_ustr,
        u"Object index heading"_ustr,
        u"Object index 1"_ustr,
        u"Table index heading"_ustr,
        u"Table index 1"_ustr,
        u"Bibliography Heading"_ustr,
        u"Bibliography 1"_ustr,
        u"User Index 6"_ustr,
        u"User Index 7"_ustr,
        u"User Index 8"_ustr,
        u"User Index 9"_ustr,
        u"User Index 10"_ustr, // STR_POCO_PRGM_TOX_USER10
    };
    return s_aRegisterProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetDocProgNameArray()
{
    static const std::vector<OUString> s_aDocProgNameArray = {
        u"Title"_ustr, // STR_POCO_PRGM_DOC_TITLE
        u"Subtitle"_ustr,
        u"Appendix"_ustr,
    };
    return s_aDocProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLProgNameArray()
{
    static const std::vector<OUString> s_aHTMLProgNameArray = {
        u"Quotations"_ustr,
        u"Preformatted Text"_ustr,
        u"Horizontal Line"_ustr,
        u"List Contents"_ustr,
        u"List Heading"_ustr, // STR_POCO_PRGM_HTML_DT
    };
    return s_aHTMLProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetFrameFormatProgNameArray()
{
    static const std::vector<OUString> s_aFrameFormatProgNameArray = {
        u"Frame"_ustr, // RES_POOLFRM_FRAME
        u"Graphics"_ustr,
        u"OLE"_ustr,
        u"Formula"_ustr,
        u"Marginalia"_ustr,
        u"Watermark"_ustr,
        u"Labels"_ustr, // RES_POOLFRM_LABEL
    };
    return s_aFrameFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aChrFormatProgNameArray = {
        u"Footnote Symbol"_ustr, // RES_POOLCHR_FOOTNOTE
        u"Page Number"_ustr,
        u"Caption characters"_ustr,
        u"Drop Caps"_ustr,
        u"Numbering Symbols"_ustr,
        u"Bullet Symbols"_ustr,
        u"Internet link"_ustr,
        u"Visited Internet Link"_ustr,
        u"Placeholder"_ustr,
        u"Index Link"_ustr,
        u"Endnote Symbol"_ustr,
        u"Line numbering"_ustr,
        u"Main index entry"_ustr,
        u"Footnote anchor"_ustr,
        u"Endnote anchor"_ustr,
        u"Rubies"_ustr, // RES_POOLCHR_RUBYTEXT
        u"Vertical Numbering Symbols"_ustr, // RES_POOLCHR_VERT_NUMBER
    };
    return s_aChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aHTMLChrFormatProgNameArray = {
        u"Emphasis"_ustr, // RES_POOLCHR_HTML_EMPHASIS
        u"Citation"_ustr,
        u"Strong Emphasis"_ustr,
        u"Source Text"_ustr,
        u"Example"_ustr,
        u"User Entry"_ustr,
        u"Variable"_ustr,
        u"Definition"_ustr,
        u"Teletype"_ustr, // RES_POOLCHR_HTML_TELETYPE
    };
    return s_aHTMLChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetPageDescProgNameArray()
{
    static const std::vector<OUString> s_aPageDescProgNameArray = {
        u"Standard"_ustr, // STR_POOLPAGE_PRGM_STANDARD
        u"First Page"_ustr,
        u"Left Page"_ustr,
        u"Right Page"_ustr,
        u"Envelope"_ustr,
        u"Index"_ustr,
        u"HTML"_ustr,
        u"Footnote"_ustr,
        u"Endnote"_ustr, // STR_POOLPAGE_PRGM_ENDNOTE
        u"Landscape"_ustr,
    };
    return s_aPageDescProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    static const std::vector<OUString> s_aNumRuleProgNameArray = {
        u"No List"_ustr,
        u"Numbering 123"_ustr, // STR_POOLNUMRULE_PRGM_NUM1
        u"Numbering ABC"_ustr,
        u"Numbering abc"_ustr,
        u"Numbering IVX"_ustr,
        u"Numbering ivx"_ustr,
        u"List 1"_ustr,
        u"List 2"_ustr,
        u"List 3"_ustr,
        u"List 4"_ustr,
        u"List 5"_ustr, // STR_POOLNUMRULE_PRGM_BUL5
    };
    return s_aNumRuleProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTableStyleProgNameArray()
{
    // XXX MUST match the entries of STR_TABSTYLE_ARY in
    // sw/source/core/doc/DocumentStylePoolManager.cxx and MUST match the order of
    // RES_POOL_TABLESTYLE_TYPE in sw/inc/poolfmt.hxx
    static const std::vector<OUString> s_aTableStyleProgNameArray = {
        u"Default Style"_ustr,       // RES_POOLTABLESTYLE_DEFAULT
        u"3D"_ustr,                  // RES_POOLTABLESTYLE_3D
        u"Black 1"_ustr,             // RES_POOLTABLESTYLE_BLACK1
        u"Black 2"_ustr,             // RES_POOLTABLESTYLE_BLACK2
        u"Blue"_ustr,                // RES_POOLTABLESTYLE_BLUE
        u"Brown"_ustr,               // RES_POOLTABLESTYLE_BROWN
        u"Currency"_ustr,            // RES_POOLTABLESTYLE_CURRENCY
        u"Currency 3D"_ustr,         // RES_POOLTABLESTYLE_CURRENCY_3D
        u"Currency Gray"_ustr,       // RES_POOLTABLESTYLE_CURRENCY_GRAY
        u"Currency Lavender"_ustr,   // RES_POOLTABLESTYLE_CURRENCY_LAVENDER
        u"Currency Turquoise"_ustr,  // RES_POOLTABLESTYLE_CURRENCY_TURQUOISE
        u"Gray"_ustr,                // RES_POOLTABLESTYLE_GRAY
        u"Green"_ustr,               // RES_POOLTABLESTYLE_GREEN
        u"Lavender"_ustr,            // RES_POOLTABLESTYLE_LAVENDER
        u"Red"_ustr,                 // RES_POOLTABLESTYLE_RED
        u"Turquoise"_ustr,           // RES_POOLTABLESTYLE_TURQUOISE
        u"Yellow"_ustr,              // RES_POOLTABLESTYLE_YELLOW
        u"Academic"_ustr,            // RES_POOLTABLESTYLE_LO6_ACADEMIC
        u"Box List Blue"_ustr,       // RES_POOLTABLESTYLE_LO6_BOX_LIST_BLUE
        u"Box List Green"_ustr,      // RES_POOLTABLESTYLE_LO6_BOX_LIST_GREEN
        u"Box List Red"_ustr,        // RES_POOLTABLESTYLE_LO6_BOX_LIST_RED
        u"Box List Yellow"_ustr,     // RES_POOLTABLESTYLE_LO6_BOX_LIST_YELLOW
        u"Elegant"_ustr,             // RES_POOLTABLESTYLE_LO6_ELEGANT
        u"Financial"_ustr,           // RES_POOLTABLESTYLE_LO6_FINANCIAL
        u"Simple Grid Columns"_ustr, // RES_POOLTABLESTYLE_LO6_SIMPLE_GRID_COLUMNS
        u"Simple Grid Rows"_ustr,    // RES_POOLTABLESTYLE_LO6_SIMPLE_GRID_ROWS
        u"Simple List Shaded"_ustr,  // RES_POOLTABLESTYLE_LO6_SIMPLE_LIST_SHADED
    };
    return s_aTableStyleProgNameArray;
}

/// returns an empty array because Cell Names aren't translated
const std::vector<OUString>& SwStyleNameMapper::GetCellStyleProgNameArray()
{
    static const std::vector<OUString> s_aCellStyleProgNameArray;
    return s_aCellStyleProgNameArray;
}

const OUString &
SwStyleNameMapper::GetSpecialExtraProgName(const OUString& rExtraUIName)
{
    return lcl_GetSpecialExtraName( rExtraUIName, true );
}

const OUString &
SwStyleNameMapper::GetSpecialExtraUIName(const OUString& rExtraProgName)
{
    return lcl_GetSpecialExtraName( rExtraProgName, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
