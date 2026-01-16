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

#include <cassert>

#include <hintids.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <osl/diagnose.h>
#include <doc.hxx>
#include <IDocumentState.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentListsAccess.hxx>
#include <list.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <fmtcol.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <tblafmt.hxx>
#include <hints.hxx>

using namespace ::com::sun::star;

void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem )
{
    rSet.Put( rItem );
    sal_uInt16 nWhCJK = 0, nWhCTL = 0;
    switch( rItem.Which() )
    {
    case RES_CHRATR_FONTSIZE:
        nWhCJK = RES_CHRATR_CJK_FONTSIZE;
        nWhCTL = RES_CHRATR_CTL_FONTSIZE;
        break;
    case RES_CHRATR_FONT:
        nWhCJK = RES_CHRATR_CJK_FONT;
        nWhCTL = RES_CHRATR_CTL_FONT;
        break;
    case RES_CHRATR_LANGUAGE:
        nWhCJK = RES_CHRATR_CJK_LANGUAGE;
        nWhCTL = RES_CHRATR_CTL_LANGUAGE;
        break;
    case RES_CHRATR_POSTURE:
        nWhCJK = RES_CHRATR_CJK_POSTURE;
        nWhCTL = RES_CHRATR_CTL_POSTURE;
        break;
    case RES_CHRATR_WEIGHT:
        nWhCJK = RES_CHRATR_CJK_WEIGHT;
        nWhCTL = RES_CHRATR_CTL_WEIGHT;
        break;
    }

    if( nWhCJK )
        rSet.Put( rItem.CloneSetWhich(nWhCJK) );
    if( nWhCTL )
        rSet.Put( rItem.CloneSetWhich(nWhCTL) );
}

/// Return the AutoCollection by its Id. If it doesn't
/// exist yet, create it.
/// If the String pointer is defined, then only query for
/// the Attribute descriptions. It doesn't create a style!
SvxFrameDirection GetDefaultFrameDirection(LanguageType nLanguage)
{
    return MsLangId::isRightToLeft(nLanguage) ?
            SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB;
}

namespace {
#ifndef NDEBUG
    bool lcl_isValidUsedStyle(const sw::BroadcastingModify* pModify)
    {
        const bool isParaStyle = dynamic_cast<const SwTextFormatColl*>(pModify);
        const bool isCharStyle = dynamic_cast<const SwCharFormat*>(pModify);
        const bool isFrameStyle = dynamic_cast<const SwFrameFormat*>(pModify);
        const bool isFieldType = dynamic_cast<const SwFieldType*>(pModify); // just for insanity's sake, this is also used on FieldTypes
        return isParaStyle || isCharStyle || isFrameStyle || isFieldType;
    }
#endif
}


// See if the Paragraph/Character/Frame style or Field Type is in use
bool SwDoc::IsUsed( const sw::BroadcastingModify& rModify ) const
{
    assert(lcl_isValidUsedStyle(&rModify));

    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    bool isUsed = false;
    sw::AutoFormatUsedHint aHint(isUsed, GetNodes());
    rModify.CallSwClientNotify(aHint);
    return isUsed;
}

// See if Table style is in use
bool SwDoc::IsUsed( const SwTableAutoFormat& rTableAutoFormat) const
{
    size_t nTableCount = GetTableFrameFormatCount(true);
    for (size_t i=0; i < nTableCount; ++i)
    {
        SwFrameFormat* pFrameFormat = &GetTableFrameFormat(i, true);
        SwTable* pTable = SwTable::FindTable(pFrameFormat);
        if (pTable->GetTableStyleName() == rTableAutoFormat.GetName())
            return true;
    }
    return false;
}

// See if the NumRule is used
bool SwDoc::IsUsed( const SwNumRule& rRule ) const
{
    SwList const*const pList(getIDocumentListsAccess().getListByName(rRule.GetDefaultListId()));
    bool bUsed = rRule.GetTextNodeListSize() > 0 ||
                     rRule.GetParagraphStyleListSize() > 0 ||
                     rRule.IsUsedByRedline()
            // tdf#135014 default num rule is used if any associated num rule is used
            || (pList
                && pList->GetDefaultListStyleName() == rRule.GetName()
                && pList->HasNodes());

    return bUsed;
}

const OUString* SwDoc::GetDocPattern(size_t const nPos) const
{
    if (nPos >= m_PatternNames.size())
        return nullptr;
    return &m_PatternNames[nPos];
}

// Look for the style name's position. If it doesn't exist,
// insert an anew
size_t SwDoc::SetDocPattern(const OUString& rPatternName)
{
    OSL_ENSURE( !rPatternName.isEmpty(), "no Document style name" );

    auto const iter(
        std::find(m_PatternNames.begin(), m_PatternNames.end(), rPatternName));
    if (iter != m_PatternNames.end())
    {
        return std::distance(m_PatternNames.begin(), iter);
    }
    else
    {
        m_PatternNames.push_back(rPatternName);
        getIDocumentState().SetModified();
        return m_PatternNames.size() - 1;
    }
}

SwPoolFormatId GetPoolParent( SwPoolFormatId nId )
{
    SwPoolFormatId nRet = SwPoolFormatId::UNKNOWN;
    if( POOLGRP_NOCOLLID & sal_uInt16(nId) )        // 1 == Formats / 0 == Collections
    {
        switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & sal_uInt16(nId) )
        {
        case POOLGRP_CHARFMT:
        case POOLGRP_FRAMEFMT:
            nRet = SwPoolFormatId::ZERO;           // derived from the default
            break;
        case POOLGRP_PAGEDESC:
        case POOLGRP_NUMRULE:
            break;              // there are no derivations
        }
    }
    else
    {
        switch( COLL_GET_RANGE_BITS & sal_uInt16(nId) )
        {
        case COLL_TEXT_BITS:
            switch( nId )
            {
            case SwPoolFormatId::COLL_STANDARD:
                    nRet = SwPoolFormatId::ZERO;                        break;
            case SwPoolFormatId::COLL_TEXT_IDENT:
            case SwPoolFormatId::COLL_TEXT_NEGIDENT:
            case SwPoolFormatId::COLL_TEXT_MOVE:
            case SwPoolFormatId::COLL_CONFRONTATION:
            case SwPoolFormatId::COLL_MARGINAL:
                    nRet = SwPoolFormatId::COLL_TEXT;                   break;

            case SwPoolFormatId::COLL_TEXT:
            case SwPoolFormatId::COLL_GREETING:
            case SwPoolFormatId::COLL_SIGNATURE:
                    nRet = SwPoolFormatId::COLL_STANDARD;               break;
            default: break;
            }
            break;

        case COLL_LISTS_BITS:
            switch( nId )
            {
            case SwPoolFormatId::COLL_NUMBER_BULLET_BASE:
                    nRet = SwPoolFormatId::COLL_TEXT;                   break;

            default:
                nRet = SwPoolFormatId::COLL_NUMBER_BULLET_BASE;                break;
            }
            break;

        case COLL_EXTRA_BITS:
            switch( nId )
            {
            case SwPoolFormatId::COLL_TABLE_HDLN:
                    nRet = SwPoolFormatId::COLL_TABLE;                 break;

            case SwPoolFormatId::COLL_FRAME:
            case SwPoolFormatId::COLL_TABLE:
            case SwPoolFormatId::COLL_FOOTNOTE:
            case SwPoolFormatId::COLL_ENDNOTE:
            case SwPoolFormatId::COLL_ENVELOPE_ADDRESS:
            case SwPoolFormatId::COLL_SEND_ADDRESS:
            case SwPoolFormatId::COLL_HEADERFOOTER:
            case SwPoolFormatId::COLL_LABEL:
            case SwPoolFormatId::COLL_COMMENT:
                    nRet = SwPoolFormatId::COLL_STANDARD;              break;
            case SwPoolFormatId::COLL_HEADER:
                    nRet = SwPoolFormatId::COLL_HEADERFOOTER;          break;
            case SwPoolFormatId::COLL_HEADERL:
            case SwPoolFormatId::COLL_HEADERR:
                    nRet = SwPoolFormatId::COLL_HEADER;                break;
            case SwPoolFormatId::COLL_FOOTER:
                    nRet = SwPoolFormatId::COLL_HEADERFOOTER;          break;
            case SwPoolFormatId::COLL_FOOTERL:
            case SwPoolFormatId::COLL_FOOTERR:
                    nRet = SwPoolFormatId::COLL_FOOTER;                break;

            case SwPoolFormatId::COLL_LABEL_ABB:
            case SwPoolFormatId::COLL_LABEL_TABLE:
            case SwPoolFormatId::COLL_LABEL_FRAME:
            case SwPoolFormatId::COLL_LABEL_DRAWING:
            case SwPoolFormatId::COLL_LABEL_FIGURE:
                    nRet = SwPoolFormatId::COLL_LABEL;                  break;
            default: break;
            }
            break;

        case COLL_REGISTER_BITS:
            switch( nId )
            {
            case SwPoolFormatId::COLL_REGISTER_BASE:
                    nRet = SwPoolFormatId::COLL_STANDARD;               break;

            case SwPoolFormatId::COLL_TOX_IDXH:
                    nRet = SwPoolFormatId::COLL_HEADLINE_BASE;          break;

            case SwPoolFormatId::COLL_TOX_USERH:
            case SwPoolFormatId::COLL_TOX_CNTNTH:
            case SwPoolFormatId::COLL_TOX_ILLUSH:
            case SwPoolFormatId::COLL_TOX_OBJECTH:
            case SwPoolFormatId::COLL_TOX_TABLESH:
            case SwPoolFormatId::COLL_TOX_AUTHORITIESH:
                    nRet = SwPoolFormatId::COLL_TOX_IDXH;               break;

            default:
                    nRet = SwPoolFormatId::COLL_REGISTER_BASE;          break;
            }
            break;

        case COLL_DOC_BITS:
            switch (nId)
            {
                case SwPoolFormatId::COLL_HEADLINE_BASE:
                    nRet = SwPoolFormatId::COLL_STANDARD;
                    break;
                default:
                    nRet = SwPoolFormatId::COLL_HEADLINE_BASE;
                    break;
            }
            break;

        case COLL_HTML_BITS:
            nRet = SwPoolFormatId::COLL_STANDARD;
            break;
        }
    }

    return nRet;
}

void SwDoc::RemoveAllFormatLanguageDependencies()
{
    /* Restore the language independent pool defaults and styles. */
    GetAttrPool().ResetUserDefaultItem( RES_PARATR_ADJUST );

    SwTextFormatColl * pTextFormatColl = getIDocumentStylePoolAccess().GetTextCollFromPool( SwPoolFormatId::COLL_STANDARD );

    pTextFormatColl->ResetFormatAttr( RES_PARATR_ADJUST );
    /* koreans do not like SvxScriptItem(TRUE) */
    pTextFormatColl->ResetFormatAttr( RES_PARATR_SCRIPTSPACE );

    SvxFrameDirectionItem aFrameDir( SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR );

    size_t nCount = GetPageDescCnt();
    for( size_t i=0; i<nCount; ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );
        rDesc.GetMaster().SetFormatAttr( aFrameDir );
        rDesc.GetLeft().SetFormatAttr( aFrameDir );
    }

    //#i16874# AutoKerning as default for new documents
    GetAttrPool().ResetUserDefaultItem( RES_CHRATR_AUTOKERN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
