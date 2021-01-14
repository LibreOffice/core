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

using namespace ::editeng;
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

// See if the Paragraph/Character/Frame/Page style is in use
bool SwDoc::IsUsed( const sw::BroadcastingModify& rModify ) const
{
    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    SwAutoFormatGetDocNode aGetHt( &GetNodes() );
    return !rModify.GetInfo( aGetHt );
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
bool SwDoc::IsUsed( const SwNumRule& rRule )
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

sal_uInt16 GetPoolParent( sal_uInt16 nId )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( POOLGRP_NOCOLLID & nId )        // 1 == Formats / 0 == Collections
    {
        switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & nId )
        {
        case POOLGRP_CHARFMT:
        case POOLGRP_FRAMEFMT:
            nRet = 0;           // derived from the default
            break;
        case POOLGRP_PAGEDESC:
        case POOLGRP_NUMRULE:
            break;              // there are no derivations
        }
    }
    else
    {
        switch( COLL_GET_RANGE_BITS & nId )
        {
        case COLL_TEXT_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_STANDARD:
                    nRet = 0;                                   break;
            case RES_POOLCOLL_TEXT_IDENT:
            case RES_POOLCOLL_TEXT_NEGIDENT:
            case RES_POOLCOLL_TEXT_MOVE:
            case RES_POOLCOLL_CONFRONTATION:
            case RES_POOLCOLL_MARGINAL:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            case RES_POOLCOLL_TEXT:
            case RES_POOLCOLL_GREETING:
            case RES_POOLCOLL_SIGNATURE:
            case RES_POOLCOLL_HEADLINE_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_HEADLINE1:
            case RES_POOLCOLL_HEADLINE2:
            case RES_POOLCOLL_HEADLINE3:
            case RES_POOLCOLL_HEADLINE4:
            case RES_POOLCOLL_HEADLINE5:
            case RES_POOLCOLL_HEADLINE6:
            case RES_POOLCOLL_HEADLINE7:
            case RES_POOLCOLL_HEADLINE8:
            case RES_POOLCOLL_HEADLINE9:
            case RES_POOLCOLL_HEADLINE10:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;
            }
            break;

        case COLL_LISTS_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_NUMBER_BULLET_BASE:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            default:
                nRet = RES_POOLCOLL_NUMBER_BULLET_BASE;                break;
            }
            break;

        case COLL_EXTRA_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_TABLE_HDLN:
                    nRet = RES_POOLCOLL_TABLE;                 break;

            case RES_POOLCOLL_FRAME:
            case RES_POOLCOLL_TABLE:
            case RES_POOLCOLL_FOOTNOTE:
            case RES_POOLCOLL_ENDNOTE:
            case RES_POOLCOLL_ENVELOPE_ADDRESS:
            case RES_POOLCOLL_SEND_ADDRESS:
            case RES_POOLCOLL_HEADERFOOTER:
            case RES_POOLCOLL_LABEL:
                    nRet = RES_POOLCOLL_STANDARD;              break;        
            case RES_POOLCOLL_HEADER:
                    nRet = RES_POOLCOLL_HEADERFOOTER;          break;
            case RES_POOLCOLL_HEADERL:
            case RES_POOLCOLL_HEADERR:
                    nRet = RES_POOLCOLL_HEADER;                break;
            case RES_POOLCOLL_FOOTER:
                    nRet = RES_POOLCOLL_HEADERFOOTER;          break;
            case RES_POOLCOLL_FOOTERL:
            case RES_POOLCOLL_FOOTERR:
                    nRet = RES_POOLCOLL_FOOTER;                break;

            case RES_POOLCOLL_LABEL_ABB:
            case RES_POOLCOLL_LABEL_TABLE:
            case RES_POOLCOLL_LABEL_FRAME:
            case RES_POOLCOLL_LABEL_DRAWING:
            case RES_POOLCOLL_LABEL_FIGURE:
                    nRet = RES_POOLCOLL_LABEL;                  break;
            }
            break;

        case COLL_REGISTER_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_REGISTER_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_TOX_IDXH:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;

            case RES_POOLCOLL_TOX_USERH:
            case RES_POOLCOLL_TOX_CNTNTH:
            case RES_POOLCOLL_TOX_ILLUSH:
            case RES_POOLCOLL_TOX_OBJECTH:
            case RES_POOLCOLL_TOX_TABLESH:
            case RES_POOLCOLL_TOX_AUTHORITIESH:
                    nRet = RES_POOLCOLL_TOX_IDXH;               break;

            default:
                    nRet = RES_POOLCOLL_REGISTER_BASE;          break;
            }
            break;

        case COLL_DOC_BITS:
            nRet = RES_POOLCOLL_HEADLINE_BASE;
            break;

        case COLL_HTML_BITS:
            nRet = RES_POOLCOLL_STANDARD;
            break;
        }
    }

    return nRet;
}

void SwDoc::RemoveAllFormatLanguageDependencies()
{
    /* Restore the language independent pool defaults and styles. */
    GetAttrPool().ResetPoolDefaultItem( RES_PARATR_ADJUST );

    SwTextFormatColl * pTextFormatColl = getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD );

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
    GetAttrPool().ResetPoolDefaultItem( RES_CHRATR_AUTOKERN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
