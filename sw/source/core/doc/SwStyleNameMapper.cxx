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
#include <names.hxx>

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
        sal_uInt16(SwPoolFormatId::COLL_LABEL_DRAWING) - sal_uInt16(SwPoolFormatId::COLL_EXTRA_BEGIN),
        sal_uInt16(SwPoolFormatId::COLL_LABEL_ABB) - sal_uInt16(SwPoolFormatId::COLL_EXTRA_BEGIN),
        sal_uInt16(SwPoolFormatId::COLL_LABEL_TABLE) - sal_uInt16(SwPoolFormatId::COLL_EXTRA_BEGIN),
        sal_uInt16(SwPoolFormatId::COLL_LABEL_FRAME)- sal_uInt16(SwPoolFormatId::COLL_EXTRA_BEGIN),
        sal_uInt16(SwPoolFormatId::COLL_LABEL_FIGURE)-sal_uInt16(SwPoolFormatId::COLL_EXTRA_BEGIN),
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

template<class T>
void lcl_CheckSuffixAndDelete(T & rString)
{
    if (lcl_SuffixIsUser(rString.toString()))
    {
        rString = T(rString.toString().copy(0, rString.toString().getLength() - 7));
    }
}

void lcl_CheckSuffixAndDelete(ProgName & rString)
{
    if (lcl_SuffixIsUser(rString.toString()))
    {
        rString = ProgName(rString.toString().copy(0, rString.toString().getLength() - 7));
    }
}

NameToIdHash HashFromRange(sal_uInt16 nAcc) { return NameToIdHash(nAcc); }
template <typename... Rest>
NameToIdHash HashFromRange(sal_uInt16 nAcc, SwPoolFormatId nBegin, SwPoolFormatId nEnd,
                           const std::vector<OUString>& (*pFunc)(), Rest... rest)
{
    NameToIdHash hash(HashFromRange(nAcc + sal_uInt16(nEnd) - sal_uInt16(nBegin), rest...));
    sal_uInt16 nIndex, nId;
    const std::vector<OUString>& rStrings = pFunc();
    for (nIndex = 0, nId = sal_uInt16(nBegin); nId < sal_uInt16(nEnd); nId++, nIndex++)
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
        SwPoolFormatId::COLL_TEXT_BEGIN, SwPoolFormatId::COLL_TEXT_END, bProgName ? &SwStyleNameMapper::GetTextProgNameArray : &SwStyleNameMapper::GetTextUINameArray,
        SwPoolFormatId::COLL_LISTS_BEGIN, SwPoolFormatId::COLL_LISTS_END, bProgName ? &SwStyleNameMapper::GetListsProgNameArray : &SwStyleNameMapper::GetListsUINameArray,
        SwPoolFormatId::COLL_EXTRA_BEGIN, SwPoolFormatId::COLL_EXTRA_END, bProgName ? &SwStyleNameMapper::GetExtraProgNameArray : &SwStyleNameMapper::GetExtraUINameArray,
        SwPoolFormatId::COLL_REGISTER_BEGIN, SwPoolFormatId::COLL_REGISTER_END, bProgName ? &SwStyleNameMapper::GetRegisterProgNameArray : &SwStyleNameMapper::GetRegisterUINameArray,
        SwPoolFormatId::COLL_DOC_BEGIN, SwPoolFormatId::COLL_DOC_END, bProgName ? &SwStyleNameMapper::GetDocProgNameArray : &SwStyleNameMapper::GetDocUINameArray,
        SwPoolFormatId::COLL_HTML_BEGIN, SwPoolFormatId::COLL_HTML_END, bProgName ? &SwStyleNameMapper::GetHTMLProgNameArray : &SwStyleNameMapper::GetHTMLUINameArray
    );
};

NameToIdHash GetCharMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::CHR_NORMAL_BEGIN, SwPoolFormatId::CHR_NORMAL_END, bProgName ? &SwStyleNameMapper::GetChrFormatProgNameArray : &SwStyleNameMapper::GetChrFormatUINameArray,
        SwPoolFormatId::CHR_HTML_BEGIN, SwPoolFormatId::CHR_HTML_END, bProgName ? &SwStyleNameMapper::GetHTMLChrFormatProgNameArray : &SwStyleNameMapper::GetHTMLChrFormatUINameArray
    );
};

NameToIdHash GetFrameMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::FRM_BEGIN, SwPoolFormatId::FRM_END, bProgName ? &SwStyleNameMapper::GetFrameFormatProgNameArray : &SwStyleNameMapper::GetFrameFormatUINameArray
    );
};

NameToIdHash GetPageMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::PAGE_BEGIN, SwPoolFormatId::PAGE_END, bProgName ? &SwStyleNameMapper::GetPageDescProgNameArray : &SwStyleNameMapper::GetPageDescUINameArray
    );
};

NameToIdHash GetNumRuleMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::NUMRULE_BEGIN, SwPoolFormatId::NUMRULE_END, bProgName ? &SwStyleNameMapper::GetNumRuleProgNameArray : &SwStyleNameMapper::GetNumRuleUINameArray
    );
};

NameToIdHash GetTableStyleMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::TABLESTYLE_BEGIN, SwPoolFormatId::TABLESTYLE_END, bProgName ? &SwStyleNameMapper::GetTableStyleProgNameArray : &SwStyleNameMapper::GetTableStyleUINameArray
    );
};

NameToIdHash GetCellStyleMap(bool bProgName)
{
    return HashFromRange(0,
        SwPoolFormatId::CELLSTYLE_BEGIN, SwPoolFormatId::CELLSTYLE_END, bProgName ? &SwStyleNameMapper::GetCellStyleProgNameArray : &SwStyleNameMapper::GetCellStyleUINameArray
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

        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_TEXT_BEGIN, SwPoolFormatId::COLL_TEXT_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_LISTS_BEGIN, SwPoolFormatId::COLL_LISTS_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_EXTRA_BEGIN, SwPoolFormatId::COLL_EXTRA_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_REGISTER_BEGIN, SwPoolFormatId::COLL_REGISTER_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_DOC_BEGIN, SwPoolFormatId::COLL_DOC_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, SwPoolFormatId::COLL_HTML_BEGIN, SwPoolFormatId::COLL_HTML_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, SwPoolFormatId::CHR_NORMAL_BEGIN, SwPoolFormatId::CHR_NORMAL_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, SwPoolFormatId::CHR_HTML_BEGIN, SwPoolFormatId::CHR_HTML_END );
        testNameTable( SwGetPoolIdFromName::FrmFmt, SwPoolFormatId::FRM_BEGIN, SwPoolFormatId::FRM_END );
        testNameTable( SwGetPoolIdFromName::PageDesc, SwPoolFormatId::PAGE_BEGIN, SwPoolFormatId::PAGE_END );
        testNameTable( SwGetPoolIdFromName::NumRule, SwPoolFormatId::NUMRULE_BEGIN, SwPoolFormatId::NUMRULE_END );
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
        case SwGetPoolIdFromName::TableStyle:
            return TablePair<GetTableStyleMap>::getMap(bProgName);
        case SwGetPoolIdFromName::CellStyle:
            return TablePair<GetCellStyleMap>::getMap(bProgName);
    }

    assert(false); // must not reach here
    abort();
}

// This gets the UI name from the programmatic name
UIName SwStyleNameMapper::GetUIName(const ProgName& rName,
                                             SwGetPoolIdFromName const eFlags)
{
    SwPoolFormatId nId = GetPoolIdFromProgName ( rName, eFlags );
    return nId != SwPoolFormatId::UNKNOWN ? GetUIName( nId, rName ) : UIName(rName.toString());
}

// Get the programmatic name from the UI name
ProgName SwStyleNameMapper::GetProgName(
        const UIName& rName, SwGetPoolIdFromName const eFlags)
{
    SwPoolFormatId nId = GetPoolIdFromUIName ( rName, eFlags );
    return nId != SwPoolFormatId::UNKNOWN ? GetProgName( nId, rName ) : ProgName(rName.toString());
}

// Get the programmatic name from the UI name in rName and put it into rFillName
void SwStyleNameMapper::FillProgName(
        const UIName& rName, ProgName& rFillName,
        SwGetPoolIdFromName const eFlags)
{
    SwPoolFormatId nId = GetPoolIdFromUIName ( rName, eFlags );
    if ( nId == SwPoolFormatId::UNKNOWN )
    {
        // rName isn't in our UI name table...check if it's in the programmatic one
        nId = GetPoolIdFromProgName ( ProgName(rName.toString()), eFlags );

        rFillName = ProgName(rName.toString());
        if (nId == SwPoolFormatId::UNKNOWN )
        {
            if (eFlags != SwGetPoolIdFromName::TableStyle)
            {
                // check if it has a " (user)" suffix, if so remove it
                lcl_CheckSuffixAndDelete(rFillName);
            }
            else // FIXME don't do this
            {
                // It isn't ...make sure the suffix isn't already " (user)"...if it is,
                // we need to add another one
                if (lcl_SuffixIsUser(rFillName.toString()))
                    rFillName = ProgName(rFillName.toString() + " (user)");
            }
        }
        else
        {
            // It's in the programmatic name table...append suffix
            rFillName = ProgName(rFillName.toString() + " (user)");
        }
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillProgNameFromId(nId, rFillName);
    }

    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName.toString() == SwResId(STR_POOLCHR_STANDARD))
        rFillName = ProgName("Standard");
}

// Get the UI name from the programmatic name in rName and put it into rFillName
void SwStyleNameMapper::FillUIName(
        const ProgName& rName, UIName& rFillName,
        SwGetPoolIdFromName const eFlags)
{
    ProgName aName = rName;
    if (eFlags == SwGetPoolIdFromName::ChrFmt && rName == "Standard")
        aName = ProgName(SwResId(STR_POOLCHR_STANDARD));

    SwPoolFormatId nId = GetPoolIdFromProgName ( aName, eFlags );
    if ( nId == SwPoolFormatId::UNKNOWN )
    {
        rFillName = UIName(aName.toString());
        // TabStyle: unfortunately ODF documents with UIName table styles exist
        if (eFlags == SwGetPoolIdFromName::TableStyle || // see testTdf129568ui
            GetPoolIdFromUIName(UIName(aName.toString()), eFlags) == SwPoolFormatId::UNKNOWN)
        {
            // aName isn't in our Prog name table...check if it has a " (user)" suffix, if so remove it
            lcl_CheckSuffixAndDelete(rFillName);
        }
        else
        {
            rFillName = UIName(rFillName.toString() + " (user)");
        }
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillUINameFromId(nId, rFillName);
    }
}

const OUString& SwStyleNameMapper::getNameFromId(
        SwPoolFormatId const nId, const OUString& rFillName, bool const bProgName)
{
    SwPoolFormatId nStt = SwPoolFormatId::ZERO;
    const std::vector<OUString>* pStrArr = nullptr;

    switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & sal_uInt16(nId) )
    {
    case COLL_TEXT_BITS:
        if( SwPoolFormatId::COLL_TEXT_BEGIN <= nId && nId < SwPoolFormatId::COLL_TEXT_END )
        {
            pStrArr = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
            nStt = SwPoolFormatId::COLL_TEXT_BEGIN;
        }
        break;
    case COLL_LISTS_BITS:
        if( SwPoolFormatId::COLL_LISTS_BEGIN <= nId && nId < SwPoolFormatId::COLL_LISTS_END )
        {
            pStrArr = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
            nStt = SwPoolFormatId::COLL_LISTS_BEGIN;
        }
        break;
    case COLL_EXTRA_BITS:
        if( SwPoolFormatId::COLL_EXTRA_BEGIN <= nId && nId < SwPoolFormatId::COLL_EXTRA_END )
        {
            pStrArr = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
            nStt = SwPoolFormatId::COLL_EXTRA_BEGIN;
        }
        break;
    case COLL_REGISTER_BITS:
        if( SwPoolFormatId::COLL_REGISTER_BEGIN <= nId && nId < SwPoolFormatId::COLL_REGISTER_END )
        {
            pStrArr = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
            nStt = SwPoolFormatId::COLL_REGISTER_BEGIN;
        }
        break;
    case COLL_DOC_BITS:
        if( SwPoolFormatId::COLL_DOC_BEGIN <= nId && nId < SwPoolFormatId::COLL_DOC_END )
        {
            pStrArr = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
            nStt = SwPoolFormatId::COLL_DOC_BEGIN;
        }
        break;
    case COLL_HTML_BITS:
        if( SwPoolFormatId::COLL_HTML_BEGIN <= nId && nId < SwPoolFormatId::COLL_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
            nStt = SwPoolFormatId::COLL_HTML_BEGIN;
        }
        break;
    case sal_uInt16(POOLGRP_CHARFMT):
        if( SwPoolFormatId::CHR_NORMAL_BEGIN <= nId && nId < SwPoolFormatId::CHR_NORMAL_END )
        {
            pStrArr = bProgName ? &GetChrFormatProgNameArray() : &GetChrFormatUINameArray();
            nStt = SwPoolFormatId::CHR_NORMAL_BEGIN;
        }
        else if( SwPoolFormatId::CHR_HTML_BEGIN <= nId && nId < SwPoolFormatId::CHR_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLChrFormatProgNameArray() : &GetHTMLChrFormatUINameArray();
            nStt = SwPoolFormatId::CHR_HTML_BEGIN;
        }
        break;
    case sal_uInt16(POOLGRP_FRAMEFMT):
        if( SwPoolFormatId::FRM_BEGIN <= nId && nId < SwPoolFormatId::FRM_END )
        {
            pStrArr = bProgName ? &GetFrameFormatProgNameArray() : &GetFrameFormatUINameArray();
            nStt = SwPoolFormatId::FRM_BEGIN;
        }
        break;
    case sal_uInt16(POOLGRP_PAGEDESC):
        if( SwPoolFormatId::PAGE_BEGIN <= nId && nId < SwPoolFormatId::PAGE_END )
        {
            pStrArr = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
            nStt = SwPoolFormatId::PAGE_BEGIN;
        }
        break;
    case sal_uInt16(POOLGRP_NUMRULE):
        if( SwPoolFormatId::NUMRULE_BEGIN <= nId && nId < SwPoolFormatId::NUMRULE_END )
        {
            pStrArr = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
            nStt = SwPoolFormatId::NUMRULE_BEGIN;
        }
        break;
    case sal_uInt16(POOLGRP_TABSTYLE):
        if( SwPoolFormatId::TABLESTYLE_BEGIN <= nId && nId < SwPoolFormatId::TABLESTYLE_END )
        {
            pStrArr = bProgName ? &GetTableStyleProgNameArray() : &GetTableStyleUINameArray();
            nStt = SwPoolFormatId::TABLESTYLE_BEGIN;
        }
        break;
    }
    return pStrArr ? pStrArr->operator[](sal_uInt16(nId - nStt)) : rFillName;
}

void SwStyleNameMapper::fillUINameFromId(
        SwPoolFormatId const nId, UIName& rFillName)
{
    rFillName = UIName(getNameFromId(nId, rFillName.toString(), /*bProgName*/false));
}

void SwStyleNameMapper::fillProgNameFromId(
        SwPoolFormatId const nId, ProgName& rFillName)
{
    rFillName = ProgName(getNameFromId(nId, rFillName.toString(), /*bProgName*/true));
}

// Get the UI name from the pool ID
void SwStyleNameMapper::FillUIName(SwPoolFormatId const nId, UIName& rFillName)
{
    fillUINameFromId(nId, rFillName);
}

// Get the UI name from the pool ID
UIName SwStyleNameMapper::GetUIName(
        SwPoolFormatId const nId, const ProgName& rName)
{
    return UIName(getNameFromId(nId, rName.toString(), false));
}

// Get the programmatic name from the pool ID
void SwStyleNameMapper::FillProgName(SwPoolFormatId nId, ProgName& rFillName)
{
    fillProgNameFromId(nId, rFillName);
}

// Get the programmatic name from the pool ID
ProgName
SwStyleNameMapper::GetProgName(SwPoolFormatId const nId, const UIName& rName)
{
    return ProgName(getNameFromId(nId, rName.toString(), true));
}

// This gets the PoolId from the UI Name
SwPoolFormatId SwStyleNameMapper::GetPoolIdFromUIName(
        const UIName& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, false );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName.toString());
    return aIter != rHashMap.end() ? SwPoolFormatId((*aIter).second) : SwPoolFormatId::UNKNOWN;
}

// Get the Pool ID from the programmatic name
SwPoolFormatId SwStyleNameMapper::GetPoolIdFromProgName(
            const ProgName& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, true );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName.toString());
    return aIter != rHashMap.end() ? SwPoolFormatId((*aIter).second) : SwPoolFormatId::UNKNOWN;
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
        u"Standard"_ustr, // SwPoolFormatId::COLL_STANDARD
        u"Text body"_ustr,
        u"First line indent"_ustr,
        u"Hanging indent"_ustr,
        u"Text body indent"_ustr,
        u"Salutation"_ustr,
        u"Signature"_ustr,
        u"List Indent"_ustr, // SwPoolFormatId::COLL_CONFRONTATION
        u"Marginalia"_ustr,
        // SwPoolFormatId::COLL_TEXT_END
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
        u"Header and Footer"_ustr, // SwPoolFormatId::COLL_EXTRA_BEGIN
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
        u"Figure"_ustr, // SwPoolFormatId::COLL_LABEL_FIGURE
        u"Frame contents"_ustr,
        u"Footnote"_ustr,
        u"Addressee"_ustr,
        u"Sender"_ustr,
        u"Endnote"_ustr,
        u"Drawing"_ustr,
        u"Comment"_ustr, // SwPoolFormatId::COLL_COMMENT
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
        u"Heading 10"_ustr,
        // SwPoolFormatId::COLL_DOC_END
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
        u"Frame"_ustr, // SwPoolFormatId::FRM_FRAME
        u"Graphics"_ustr,
        u"OLE"_ustr,
        u"Formula"_ustr,
        u"Marginalia"_ustr,
        u"Watermark"_ustr,
        u"Labels"_ustr, // SwPoolFormatId::FRM_LABEL
        u"Inline Heading"_ustr,
    };
    return s_aFrameFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aChrFormatProgNameArray = {
        u"Footnote Symbol"_ustr, // SwPoolFormatId::CHR_FOOTNOTE
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
        u"Rubies"_ustr, // SwPoolFormatId::CHR_RUBYTEXT
        u"Vertical Numbering Symbols"_ustr, // SwPoolFormatId::CHR_VERT_NUMBER
    };
    return s_aChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFormatProgNameArray()
{
    static const std::vector<OUString> s_aHTMLChrFormatProgNameArray = {
        u"Emphasis"_ustr, // SwPoolFormatId::CHR_HTML_EMPHASIS
        u"Citation"_ustr,
        u"Strong Emphasis"_ustr,
        u"Source Text"_ustr,
        u"Example"_ustr,
        u"User Entry"_ustr,
        u"Variable"_ustr,
        u"Definition"_ustr,
        u"Teletype"_ustr, // SwPoolFormatId::CHR_HTML_TELETYPE
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
        u"Default Style"_ustr,       // SwPoolFormatId::TABLESTYLE_DEFAULT
        u"3D"_ustr,                  // SwPoolFormatId::TABLESTYLE_3D
        u"Black 1"_ustr,             // SwPoolFormatId::TABLESTYLE_BLACK1
        u"Black 2"_ustr,             // SwPoolFormatId::TABLESTYLE_BLACK2
        u"Blue"_ustr,                // SwPoolFormatId::TABLESTYLE_BLUE
        u"Brown"_ustr,               // SwPoolFormatId::TABLESTYLE_BROWN
        u"Currency"_ustr,            // SwPoolFormatId::TABLESTYLE_CURRENCY
        u"Currency 3D"_ustr,         // SwPoolFormatId::TABLESTYLE_CURRENCY_3D
        u"Currency Gray"_ustr,       // SwPoolFormatId::TABLESTYLE_CURRENCY_GRAY
        u"Currency Lavender"_ustr,   // SwPoolFormatId::TABLESTYLE_CURRENCY_LAVENDER
        u"Currency Turquoise"_ustr,  // SwPoolFormatId::TABLESTYLE_CURRENCY_TURQUOISE
        u"Gray"_ustr,                // SwPoolFormatId::TABLESTYLE_GRAY
        u"Green"_ustr,               // SwPoolFormatId::TABLESTYLE_GREEN
        u"Lavender"_ustr,            // SwPoolFormatId::TABLESTYLE_LAVENDER
        u"Red"_ustr,                 // SwPoolFormatId::TABLESTYLE_RED
        u"Turquoise"_ustr,           // SwPoolFormatId::TABLESTYLE_TURQUOISE
        u"Yellow"_ustr,              // SwPoolFormatId::TABLESTYLE_YELLOW
        u"Academic"_ustr,            // SwPoolFormatId::TABLESTYLE_LO6_ACADEMIC
        u"Box List Blue"_ustr,       // SwPoolFormatId::TABLESTYLE_LO6_BOX_LIST_BLUE
        u"Box List Green"_ustr,      // SwPoolFormatId::TABLESTYLE_LO6_BOX_LIST_GREEN
        u"Box List Red"_ustr,        // SwPoolFormatId::TABLESTYLE_LO6_BOX_LIST_RED
        u"Box List Yellow"_ustr,     // SwPoolFormatId::TABLESTYLE_LO6_BOX_LIST_YELLOW
        u"Elegant"_ustr,             // SwPoolFormatId::TABLESTYLE_LO6_ELEGANT
        u"Financial"_ustr,           // SwPoolFormatId::TABLESTYLE_LO6_FINANCIAL
        u"Simple Grid Columns"_ustr, // SwPoolFormatId::TABLESTYLE_LO6_SIMPLE_GRID_COLUMNS
        u"Simple Grid Rows"_ustr,    // SwPoolFormatId::TABLESTYLE_LO6_SIMPLE_GRID_ROWS
        u"Simple List Shaded"_ustr,  // SwPoolFormatId::TABLESTYLE_LO6_SIMPLE_LIST_SHADED
    };
    return s_aTableStyleProgNameArray;
}

/// returns an empty array because Cell Names aren't translated
const std::vector<OUString>& SwStyleNameMapper::GetCellStyleProgNameArray()
{
    static const std::vector<OUString> s_aCellStyleProgNameArray;
    return s_aCellStyleProgNameArray;
}

ProgName
SwStyleNameMapper::GetSpecialExtraProgName(const UIName& rExtraUIName)
{
    return ProgName(lcl_GetSpecialExtraName( rExtraUIName.toString(), true ));
}

UIName
SwStyleNameMapper::GetSpecialExtraUIName(const ProgName& rExtraProgName)
{
    return UIName(lcl_GetSpecialExtraName( rExtraProgName.toString(), false ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
