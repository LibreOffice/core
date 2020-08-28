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

bool lcl_SuffixIsUser(const OUString & rString)
{
    const sal_Unicode *pChar = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    bool bRet = false;
    if( nLen > 8 &&
        pChar[nLen-7] == ' ' &&
        pChar[nLen-6] == '(' &&
        pChar[nLen-5] == 'u' &&
        pChar[nLen-4] == 's' &&
        pChar[nLen-3] == 'e' &&
        pChar[nLen-2] == 'r' &&
        pChar[nLen-1] == ')' )
        bRet = true;
    return bRet;
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

        LanguageTag aCurrentLanguage = SvtSysLocale().GetUILanguageTag();
        static std::map<LanguageTag, NameToIdHash> s_aUIMap;

        auto aFound = s_aUIMap.find(aCurrentLanguage);
        if (aFound == s_aUIMap.end())
            s_aUIMap[aCurrentLanguage] = initFunc(false);

        return s_aUIMap[aCurrentLanguage];
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
        RES_POOLTABSTYLE_BEGIN, RES_POOLTABSTYLE_END, bProgName ? &SwStyleNameMapper::GetTableStyleProgNameArray : &SwStyleNameMapper::GetTableStyleUINameArray
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

    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName == SwResId(STR_POOLCOLL_STANDARD))
        rFillName = "Standard";
}

// Get the UI name from the programmatic name in rName and put it into rFillName
void SwStyleNameMapper::FillUIName(
        const OUString& rName, OUString& rFillName,
        SwGetPoolIdFromName const eFlags)
{
    OUString aName = rName;
    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName == "Standard")
        aName = SwResId(STR_POOLCOLL_STANDARD);

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
        if( RES_POOLTABSTYLE_BEGIN <= nId && nId < RES_POOLTABSTYLE_END )
        {
            pStrArr = bProgName ? &GetTableStyleProgNameArray() : &GetTableStyleUINameArray();
            nStt = RES_POOLTABSTYLE_BEGIN;
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
        "Standard", // RES_POOLCOLL_STANDARD
        "Text body",
        "First line indent",
        "Hanging indent",
        "Text body indent",
        "Salutation",
        "Signature",
        "List Indent", // RES_POOLCOLL_CONFRONTATION
        "Marginalia",
        "Heading",
        "Heading 1",
        "Heading 2",
        "Heading 3",
        "Heading 4",
        "Heading 5",
        "Heading 6",
        "Heading 7",
        "Heading 8",
        "Heading 9",
        "Heading 10", // RES_POOLCOLL_TEXT_END
    };
    return s_aTextProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetListsProgNameArray()
{
    static const std::vector<OUString> s_aListsProgNameArray = {
        "List", // STR_POCO_PRGM_NUMBUL_BASE
        "Numbering 1 Start", // STR_POCO_PRGM_NUM_LEVEL1S
        "Numbering 1",
        "Numbering 1 End",
        "Numbering 1 Cont.",
        "Numbering 2 Start",
        "Numbering 2",
        "Numbering 2 End",
        "Numbering 2 Cont.",
        "Numbering 3 Start",
        "Numbering 3",
        "Numbering 3 End",
        "Numbering 3 Cont.",
        "Numbering 4 Start",
        "Numbering 4",
        "Numbering 4 End",
        "Numbering 4 Cont.",
        "Numbering 5 Start",
        "Numbering 5",
        "Numbering 5 End",
        "Numbering 5 Cont.",
        "List 1 Start",
        "List 1",
        "List 1 End",
        "List 1 Cont.",
        "List 2 Start",
        "List 2",
        "List 2 End",
        "List 2 Cont.",
        "List 3 Start",
        "List 3",
        "List 3 End",
        "List 3 Cont.",
        "List 4 Start",
        "List 4",
        "List 4 End",
        "List 4 Cont.",
        "List 5 Start",
        "List 5",
        "List 5 End",
        "List 5 Cont.", // STR_POCO_PRGM_BUL_NONUM5
    };
    return s_aListsProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetExtraProgNameArray()
{
    static const std::vector<OUString> s_aExtraProgNameArray = {
        "Header and Footer", // RES_POOLCOLL_EXTRA_BEGIN
        "Header",
        "Header left",
        "Header right",
        "Footer",
        "Footer left",
        "Footer right",
        "Table Contents",
        "Table Heading",
        "Caption",
        "Illustration",
        "Table",
        "Text",
        "Figure", // RES_POOLCOLL_LABEL_FIGURE
        "Frame contents",
        "Footnote",
        "Addressee",
        "Sender",
        "Endnote",
        "Drawing", // RES_POOLCOLL_LABEL_DRAWING
    };
    return s_aExtraProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetRegisterProgNameArray()
{
    static const std::vector<OUString> s_aRegisterProgNameArray = {
        "Index", // STR_POCO_PRGM_REGISTER_BASE
        "Index Heading", // STR_POCO_PRGM_TOX_IDXH
        "Index 1",
        "Index 2",
        "Index 3",
        "Index Separator",
        "Contents Heading",
        "Contents 1",
        "Contents 2",
        "Contents 3",
        "Contents 4",
        "Contents 5",
        "User Index Heading",
        "User Index 1",
        "User Index 2",
        "User Index 3",
        "User Index 4",
        "User Index 5",
        "Contents 6",
        "Contents 7",
        "Contents 8",
        "Contents 9",
        "Contents 10",
        "Figure Index Heading",
        "Figure Index 1",
        "Object index heading",
        "Object index 1",
        "Table index heading",
        "Table index 1",
        "Bibliography Heading",
        "Bibliography 1",
        "User Index 6",
        "User Index 7",
        "User Index 8",
        "User Index 9",
        "User Index 10", // STR_POCO_PRGM_TOX_USER10
    };
    return s_aRegisterProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetDocProgNameArray()
{
    static const std::vector<OUString> s_aDocProgNameArray = {
        "Title", // STR_POCO_PRGM_DOC_TITLE
        "Subtitle",
    };
    return s_aDocProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLProgNameArray()
{
    static const std::vector<OUString> s_aHTMLProgNameArray = {
        "Quotations",
        "Preformatted Text",
        "Horizontal Line",
        "List Contents",
        "List Heading", // STR_POCO_PRGM_HTML_DT
    };
    return s_aHTMLProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetFrameFormatProgNameArray()
{
    static const std::vector<OUString> s_aFrameFormatProgNameArray = {
        "Frame", // RES_POOLFRM_FRAME
        "Graphics",
        "OLE",
        "Formula",
        "Marginalia",
        "Watermark",
        "Labels", // RES_POOLFRM_LABEL
    };
    return s_aFrameFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aChrFormatProgNameArray = {
        "Footnote Symbol", // RES_POOLCHR_FOOTNOTE
        "Page Number",
        "Caption characters",
        "Drop Caps",
        "Numbering Symbols",
        "Bullet Symbols",
        "Internet link",
        "Visited Internet Link",
        "Placeholder",
        "Index Link",
        "Endnote Symbol",
        "Line numbering",
        "Main index entry",
        "Footnote anchor",
        "Endnote anchor",
        "Rubies", // RES_POOLCHR_RUBYTEXT
        "Vertical Numbering Symbols", // RES_POOLCHR_VERT_NUMBER
    };
    return s_aChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aHTMLChrFormatProgNameArray = {
        "Emphasis", // RES_POOLCHR_HTML_EMPHASIS
        "Citation",
        "Strong Emphasis",
        "Source Text",
        "Example",
        "User Entry",
        "Variable",
        "Definition",
        "Teletype", // RES_POOLCHR_HTML_TELETYPE
    };
    return s_aHTMLChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetPageDescProgNameArray()
{
    static const std::vector<OUString> s_aPageDescProgNameArray = {
        "Standard", // STR_POOLPAGE_PRGM_STANDARD
        "First Page",
        "Left Page",
        "Right Page",
        "Envelope",
        "Index",
        "HTML",
        "Footnote",
        "Endnote", // STR_POOLPAGE_PRGM_ENDNOTE
        "Landscape",
    };
    return s_aPageDescProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    static const std::vector<OUString> s_aNumRuleProgNameArray = {
        "Numbering 123", // STR_POOLNUMRULE_PRGM_NUM1
        "Numbering ABC",
        "Numbering abc",
        "Numbering IVX",
        "Numbering ivx",
        "List 1",
        "List 2",
        "List 3",
        "List 4",
        "List 5", // STR_POOLNUMRULE_PRGM_BUL5
    };
    return s_aNumRuleProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTableStyleProgNameArray()
{
    // XXX MUST match the entries of STR_TABSTYLE_ARY in
    // sw/source/core/doc/DocumentStylePoolManager.cxx and MUST match the order of
    // RES_POOL_TABSTYLE_TYPE in sw/inc/poolfmt.hxx
    static const std::vector<OUString> s_aTableStyleProgNameArray = {
        "Default Style",       // RES_POOLTABLESTYLE_DEFAULT
        "3D",                  // RES_POOLTABLESTYLE_3D
        "Black 1",             // RES_POOLTABLESTYLE_BLACK1
        "Black 2",             // RES_POOLTABLESTYLE_BLACK2
        "Blue",                // RES_POOLTABLESTYLE_BLUE
        "Brown",               // RES_POOLTABLESTYLE_BROWN
        "Currency",            // RES_POOLTABLESTYLE_CURRENCY
        "Currency 3D",         // RES_POOLTABLESTYLE_CURRENCY_3D
        "Currency Gray",       // RES_POOLTABLESTYLE_CURRENCY_GRAY
        "Currency Lavender",   // RES_POOLTABLESTYLE_CURRENCY_LAVENDER
        "Currency Turquoise",  // RES_POOLTABLESTYLE_CURRENCY_TURQUOISE
        "Gray",                // RES_POOLTABLESTYLE_GRAY
        "Green",               // RES_POOLTABLESTYLE_GREEN
        "Lavender",            // RES_POOLTABLESTYLE_LAVENDER
        "Red",                 // RES_POOLTABLESTYLE_RED
        "Turquoise",           // RES_POOLTABLESTYLE_TURQUOISE
        "Yellow",              // RES_POOLTABLESTYLE_YELLOW
        "Academic",            // RES_POOLTABLESTYLE_LO6_ACADEMIC
        "Box List Blue",       // RES_POOLTABLESTYLE_LO6_BOX_LIST_BLUE
        "Box List Green",      // RES_POOLTABLESTYLE_LO6_BOX_LIST_GREEN
        "Box List Red",        // RES_POOLTABLESTYLE_LO6_BOX_LIST_RED
        "Box List Yellow",     // RES_POOLTABLESTYLE_LO6_BOX_LIST_YELLOW
        "Elegant",             // RES_POOLTABLESTYLE_LO6_ELEGANT
        "Financial",           // RES_POOLTABLESTYLE_LO6_FINANCIAL
        "Simple Grid Columns", // RES_POOLTABLESTYLE_LO6_SIMPLE_GRID_COLUMNS
        "Simple Grid Rows",    // RES_POOLTABLESTYLE_LO6_SIMPLE_GRID_ROWS
        "Simple List Shaded",  // RES_POOLTABLESTYLE_LO6_SIMPLE_LIST_SHADED
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
