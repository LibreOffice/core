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

#include <memory>
#include <libxml/xmlwriter.h>
#include <editeng/fhgtitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <svl/intitem.hxx>
#include <svl/itemiter.hxx>
#include <calbck.hxx>
#include <doc.hxx>
#include <fmtcol.hxx>
#include <fmtcolfunc.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <swfntcch.hxx>

namespace TextFormatCollFunc
{
    // #i71574#
    void CheckTextFormatCollForDeletionOfAssignmentToOutlineStyle(
                                            SwFormat* pFormat,
                                            const SwNumRuleItem* pNewNumRuleItem )
    {
        SwTextFormatColl* pTextFormatColl = dynamic_cast<SwTextFormatColl*>(pFormat);
        if ( !pTextFormatColl )
        {
            OSL_FAIL( "<TextFormatCollFunc::CheckTextFormatCollFuncForDeletionOfAssignmentToOutlineStyle> - misuse of method - it's only for instances of <SwTextFormatColl>" );
            return;
        }

        // #i73790#
        if ( pTextFormatColl->StayAssignedToListLevelOfOutlineStyle() ||
             !pTextFormatColl->IsAssignedToListLevelOfOutlineStyle() )
            return;

        if (!pNewNumRuleItem)
        {
            pNewNumRuleItem = pTextFormatColl->GetItemIfSet(RES_PARATR_NUMRULE, false);
        }
        if (pNewNumRuleItem)
        {
            const OUString& sNumRuleName = pNewNumRuleItem->GetValue();
            if ( sNumRuleName.isEmpty() ||
                 sNumRuleName != pTextFormatColl->GetDoc()->GetOutlineNumRule()->GetName() )
            {
                // delete assignment of paragraph style to list level of outline style.
                pTextFormatColl->DeleteAssignmentToListLevelOfOutlineStyle();
            }
        }
    }

    SwNumRule* GetNumRule( SwTextFormatColl& rTextFormatColl )
    {
        SwNumRule* pNumRule( nullptr );

        const SwNumRuleItem* pNumRuleItem = rTextFormatColl.GetItemIfSet(RES_PARATR_NUMRULE, false);
        if (pNumRuleItem)
        {
            const OUString& sNumRuleName = pNumRuleItem->GetValue();
            if ( !sNumRuleName.isEmpty() )
            {
                pNumRule = rTextFormatColl.GetDoc()->FindNumRulePtr( sNumRuleName );
            }
        }

        return pNumRule;
    }

    void AddToNumRule( SwTextFormatColl& rTextFormatColl )
    {
        SwNumRule* pNumRule = GetNumRule( rTextFormatColl );
        if ( pNumRule )
        {
            pNumRule->AddParagraphStyle( rTextFormatColl );
        }
    }

    void RemoveFromNumRule( SwTextFormatColl& rTextFormatColl )
    {
        SwNumRule* pNumRule = GetNumRule( rTextFormatColl );
        if ( pNumRule )
        {
            pNumRule->RemoveParagraphStyle( rTextFormatColl );
        }
    }
} // end of namespace TextFormatCollFunc

SwTextFormatColl::~SwTextFormatColl()
{
    if(m_bInSwFntCache)
        pSwFontCache->Delete( this );

    if (GetDoc()->IsInDtor())
    {
        return;
    }

    for (const auto& pCharFormat : *GetDoc()->GetCharFormats())
    {
        if (pCharFormat->GetLinkedParaFormat() == this)
        {
            pCharFormat->SetLinkedParaFormat(nullptr);
        }
    }
    Destr();
}
void SwTextFormatColl::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwAutoFormatUsedHint)
    {
        CallSwClientNotify(rHint);
        return;
    }
    else if (rHint.GetId() == SfxHintId::SwVirtPageNumHint)
    {
        CallSwClientNotify(rHint);
        return;
    }
    else if (rHint.GetId() != SfxHintId::SwLegacyModify && rHint.GetId() != SfxHintId::SwFormatChange
             && rHint.GetId() != SfxHintId::SwAttrSetChange
             && rHint.GetId() != SfxHintId::SwObjectDying
             && rHint.GetId() != SfxHintId::SwUpdateAttr)
        return;
    if(GetDoc()->IsInDtor())
    {
        SwFormatColl::SwClientNotify(rModify, rHint);
        return;
    }
    bool bNewParent( false ); // #i66431# - adjust type of <bNewParent>
    const SvxULSpaceItem *pNewULSpace = nullptr, *pOldULSpace = nullptr;
    const SvxFirstLineIndentItem *pNewFirstLineIndent = nullptr;
    const SvxTextLeftMarginItem *pNewTextLeftMargin = nullptr;
    const SvxRightMarginItem *pNewRightMargin = nullptr;
    const SvxFontHeightItem* aFontSizeArr[3] = {nullptr,nullptr,nullptr};
    // #i70223#
    const bool bAssignedToListLevelOfOutlineStyle(IsAssignedToListLevelOfOutlineStyle());
    const SwNumRuleItem* pNewNumRuleItem( nullptr );

    if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        const auto pNew = pLegacy->m_pNew;

        switch( pLegacy->GetWhich() )
        {
        case RES_MARGIN_FIRSTLINE:
            pNewFirstLineIndent = &pNew->StaticWhichCast(RES_MARGIN_FIRSTLINE);
            break;
        case RES_MARGIN_TEXTLEFT:
            pNewTextLeftMargin = &pNew->StaticWhichCast(RES_MARGIN_TEXTLEFT);
            break;
        case RES_MARGIN_RIGHT:
            pNewRightMargin = &pNew->StaticWhichCast(RES_MARGIN_RIGHT);
            break;
        case RES_UL_SPACE:
            pNewULSpace = &pNew->StaticWhichCast(RES_UL_SPACE);
            break;
        case RES_CHRATR_FONTSIZE:
            aFontSizeArr[0] = &pNew->StaticWhichCast(RES_CHRATR_FONTSIZE);
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            aFontSizeArr[1] = &pNew->StaticWhichCast(RES_CHRATR_CJK_FONTSIZE);
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            aFontSizeArr[2] = &pNew->StaticWhichCast(RES_CHRATR_CTL_FONTSIZE);
            break;
        // #i70223#
        case RES_PARATR_NUMRULE:
            if (bAssignedToListLevelOfOutlineStyle)
            {
                pNewNumRuleItem = &pNew->StaticWhichCast(RES_PARATR_NUMRULE);
            }
            break;
        default:
            break;
        }
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        const SwAttrSetChg *pNewChgSet = pChangeHint->m_pNew;

        // Only recalculate if we're not the sender!
        pNewFirstLineIndent = pNewChgSet->GetChgSet()->GetItemIfSet(RES_MARGIN_FIRSTLINE, false);
        pNewTextLeftMargin = pNewChgSet->GetChgSet()->GetItemIfSet(RES_MARGIN_TEXTLEFT, false);
        pNewRightMargin = pNewChgSet->GetChgSet()->GetItemIfSet(RES_MARGIN_RIGHT, false);
        pNewULSpace = pNewChgSet->GetChgSet()->GetItemIfSet( RES_UL_SPACE, false );
        aFontSizeArr[0] = pNewChgSet->GetChgSet()->GetItemIfSet( RES_CHRATR_FONTSIZE, false );
        aFontSizeArr[1] = pNewChgSet->GetChgSet()->GetItemIfSet( RES_CHRATR_CJK_FONTSIZE, false );
        aFontSizeArr[2] = pNewChgSet->GetChgSet()->GetItemIfSet( RES_CHRATR_CTL_FONTSIZE, false );
        // #i70223#, #i84745#
        // check, if attribute set is applied to this paragraph style
        if ( bAssignedToListLevelOfOutlineStyle &&
             pNewChgSet->GetTheChgdSet() == &GetAttrSet() )
        {
            pNewNumRuleItem = pNewChgSet->GetChgSet()->GetItemIfSet( RES_PARATR_NUMRULE, false );
        }
    }
    else if (rHint.GetId() == SfxHintId::SwFormatChange)
    {
        if( GetAttrSet().GetParent() )
        {
            const SfxItemSet* pParent = GetAttrSet().GetParent();
            pNewFirstLineIndent = &pParent->Get(RES_MARGIN_FIRSTLINE);
            pNewTextLeftMargin = &pParent->Get(RES_MARGIN_TEXTLEFT);
            pNewRightMargin = &pParent->Get(RES_MARGIN_RIGHT);
            pNewULSpace = &pParent->Get( RES_UL_SPACE );
            aFontSizeArr[0] = &pParent->Get( RES_CHRATR_FONTSIZE );
            aFontSizeArr[1] = &pParent->Get( RES_CHRATR_CJK_FONTSIZE );
            aFontSizeArr[2] = &pParent->Get( RES_CHRATR_CTL_FONTSIZE );
            // #i66431# - modify has to be propagated, because of new parent format.
            bNewParent = true;
        }
    }

    // #i70223#
    if ( bAssignedToListLevelOfOutlineStyle && pNewNumRuleItem )
    {
        TextFormatCollFunc::CheckTextFormatCollForDeletionOfAssignmentToOutlineStyle(
                                                        this, pNewNumRuleItem );
    }

    sal_uInt32 nNoNotify = 0; // track handled changes: no need to notify if all are handled here

    // Check against the own attributes
    const SvxFirstLineIndentItem *pOldFirstLineIndent(GetItemIfSet(RES_MARGIN_FIRSTLINE, false));
    if (pNewFirstLineIndent && pOldFirstLineIndent)
    {
        if (!SfxPoolItem::areSame(pOldFirstLineIndent, pNewFirstLineIndent)) // Avoid recursion (SetAttr!)
        {
            bool bChg = false;
            SvxFirstLineIndentItem aNew(*pOldFirstLineIndent);
            // We had a relative value -> recalculate
            if (100 != pOldFirstLineIndent->GetPropTextFirstLineOffset())
            {
                const auto stOld = pOldFirstLineIndent->GetTextFirstLineOffset();
                aNew.SetTextFirstLineOffset(pNewFirstLineIndent->GetTextFirstLineOffset(),
                                            pOldFirstLineIndent->GetPropTextFirstLineOffset());
                bChg = (stOld != aNew.GetTextFirstLineOffset());
            }
            if( bChg )
            {
                SetFormatAttr(aNew); // triggered separate notification about only this one property
            }
            ++nNoNotify;
        }
    }
    const SvxTextLeftMarginItem *pOldTextLeftMargin(GetItemIfSet(RES_MARGIN_TEXTLEFT, false));
    if (pNewTextLeftMargin && pOldTextLeftMargin)
    {
        if (!SfxPoolItem::areSame(pOldTextLeftMargin, pNewTextLeftMargin)) // Avoid recursion (SetAttr!)
        {
            bool bChg = false;
            SvxTextLeftMarginItem aNew(*pOldTextLeftMargin);
            // We had a relative value -> recalculate
            if (100 != pOldTextLeftMargin->GetPropLeft())
            {
                // note: changing from Left to TextLeft - looked wrong with Left
                const auto stOld = pOldTextLeftMargin->GetTextLeft();
                aNew.SetTextLeft(pNewTextLeftMargin->GetTextLeft(),
                                 pOldTextLeftMargin->GetPropLeft());
                bChg = (stOld != aNew.GetTextLeft());
            }
            if( bChg )
            {
                SetFormatAttr( aNew );
            }
            ++nNoNotify;
        }
    }
    const SvxRightMarginItem *pOldRightMargin(GetItemIfSet(RES_MARGIN_RIGHT, false));
    if (pNewRightMargin && pOldRightMargin)
    {
        if (!SfxPoolItem::areSame(pOldRightMargin, pNewRightMargin)) // Avoid recursion (SetAttr!)
        {
            bool bChg = false;
            SvxRightMarginItem aNew(*pOldRightMargin);
            // We had a relative value -> recalculate
            if (100 != pOldRightMargin->GetPropRight())
            {
                const auto stOld = pOldRightMargin->GetRight();
                aNew.SetRight(pNewRightMargin->GetRight(), pOldRightMargin->GetPropRight());
                bChg = (stOld != aNew.GetRight());
            }
            if( bChg )
            {
                SetFormatAttr( aNew );
            }
            ++nNoNotify;
        }
    }

    if( pNewULSpace && (pOldULSpace = GetItemIfSet(RES_UL_SPACE, false)) &&
        !SfxPoolItem::areSame(pOldULSpace, pNewULSpace) )    // Avoid recursion (SetAttr!)
    {
        SvxULSpaceItem aNew( *pOldULSpace );
        bool bChg = false;
        // We had a relative value -> recalculate
        if (100 != pOldULSpace->GetPropUpper())
        {
            const sal_uInt16 nOld = pOldULSpace->GetUpper();
            aNew.SetUpper(pNewULSpace->GetUpper(), pOldULSpace->GetPropUpper());
            bChg = nOld != aNew.GetUpper();
        }
        // We had a relative value -> recalculate
        if (100 != pOldULSpace->GetPropLower())
        {
            const sal_uInt16 nOld = pOldULSpace->GetLower();
            aNew.SetLower(pNewULSpace->GetLower(), pOldULSpace->GetPropLower());
            bChg |= nOld != aNew.GetLower();
        }
        if( bChg )
        {
            SetFormatAttr( aNew );
        }
        ++nNoNotify;
    }

    for (const SvxFontHeightItem *pFSize : aFontSizeArr)
    {
        const SvxFontHeightItem *pOldFSize;
        if( pFSize && (SfxItemState::SET == GetItemState(
            pFSize->Which(), false, reinterpret_cast<const SfxPoolItem**>(&pOldFSize) )) &&
            // Avoid recursion (SetAttr!)
            !SfxPoolItem::areSame(pFSize, pOldFSize) )
        {
            if (100 != pOldFSize->GetProp() || MapUnit::MapRelative != pOldFSize->GetPropUnit())
            {
                // We had a relative value -> recalculate
                const sal_uInt32 nOld = pOldFSize->GetHeight();
                SvxFontHeightItem aNew(240 , 100, pFSize->Which());
                aNew.SetHeight( pFSize->GetHeight(), pOldFSize->GetProp(),
                                pOldFSize->GetPropUnit() );
                if (nOld != aNew.GetHeight())
                {
                    SetFormatAttr( aNew );
                }
            }
            ++nNoNotify;
        }
    }

    // if the parent changed, we can't know how many properties are involved: always notify a change
    if (rHint.GetId() == SfxHintId::SwLegacyModify
        || rHint.GetId() == SfxHintId::SwObjectDying
        || rHint.GetId() == SfxHintId::SwUpdateAttr)
    {
        if (bNewParent || !nNoNotify)
            SwFormatColl::SwClientNotify(rModify, rHint);
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        const SwAttrSetChg *pOldChgSet = pChangeHint->m_pOld;

        if (bNewParent || !nNoNotify || (pOldChgSet && pOldChgSet->GetChgSet()->Count() > nNoNotify))
            SwFormatColl::SwClientNotify(rModify, rHint);
    }
    else if (rHint.GetId() == SfxHintId::SwFormatChange)
    {
        if (bNewParent || !nNoNotify)
            SwFormatColl::SwClientNotify(rModify, rHint);
    }
}

void SwTextFormatColl::SetLinkedCharFormat(SwCharFormat* pLink) { mpLinkedCharFormat = pLink; }

const SwCharFormat* SwTextFormatColl::GetLinkedCharFormat() const { return mpLinkedCharFormat; }

bool SwTextFormatColl::IsAtDocNodeSet() const
{
    SwIterator<SwContentNode,SwFormatColl> aIter( *this );
    const SwNodes& rNds = GetDoc()->GetNodes();
    for( SwContentNode* pNode = aIter.First(); pNode; pNode = aIter.Next() )
        if( &(pNode->GetNodes()) == &rNds )
            return true;

    return false;
}

bool SwTextFormatColl::SetFormatAttr( const SfxPoolItem& rAttr )
{
    const bool bIsNumRuleItem = rAttr.Which() == RES_PARATR_NUMRULE;
    if ( bIsNumRuleItem )
    {
        TextFormatCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFormatColl::SetFormatAttr( rAttr );

    if ( bIsNumRuleItem )
    {
        TextFormatCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

bool SwTextFormatColl::SetFormatAttr( const SfxItemSet& rSet )
{
    const bool bIsNumRuleItemAffected =
                rSet.GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET;
    if ( bIsNumRuleItemAffected )
    {
        TextFormatCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFormatColl::SetFormatAttr( rSet );

    if ( bIsNumRuleItemAffected )
    {
        TextFormatCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

bool SwTextFormatColl::ResetFormatAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    const bool bIsNumRuleItemAffected =
                ( nWhich2 != 0 && nWhich2 > nWhich1 )
                ? ( nWhich1 <= RES_PARATR_NUMRULE &&
                    RES_PARATR_NUMRULE <= nWhich2 )
                : nWhich1 == RES_PARATR_NUMRULE;
    if ( bIsNumRuleItemAffected )
    {
        TextFormatCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFormatColl::ResetFormatAttr( nWhich1, nWhich2 );

    return bRet;
}

// #i73790#
sal_uInt16 SwTextFormatColl::ResetAllFormatAttr()
{
    const bool bOldState( mbStayAssignedToListLevelOfOutlineStyle );
    mbStayAssignedToListLevelOfOutlineStyle = true;
    // #i70748#
    // Outline level is no longer a member, it is an attribute now.
    // Thus, it needs to be restored, if the paragraph style is assigned
    // to the outline style
    const int nAssignedOutlineStyleLevel = IsAssignedToListLevelOfOutlineStyle()
                                     ? GetAssignedOutlineStyleLevel()
                                     : -1;

    sal_uInt16 nRet = SwFormatColl::ResetAllFormatAttr();

    // #i70748#
    if ( nAssignedOutlineStyleLevel != -1 )
    {
        AssignToListLevelOfOutlineStyle( nAssignedOutlineStyleLevel );
    }

    mbStayAssignedToListLevelOfOutlineStyle = bOldState;

    return nRet;
}

::sw::ListLevelIndents SwTextFormatColl::AreListLevelIndentsApplicable() const
{
    ::sw::ListLevelIndents ret(::sw::ListLevelIndents::No);
    if (AreListLevelIndentsApplicableImpl(RES_MARGIN_FIRSTLINE))
    {
        ret |= ::sw::ListLevelIndents::FirstLine;
    }
    if (AreListLevelIndentsApplicableImpl(RES_MARGIN_TEXTLEFT))
    {
        ret |= ::sw::ListLevelIndents::LeftMargin;
    }
    return ret;
}

bool SwTextFormatColl::AreListLevelIndentsApplicableImpl(sal_uInt16 const nWhich) const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( GetItemState( RES_PARATR_NUMRULE ) != SfxItemState::SET )
    {
        // no list style applied to paragraph style
        bAreListLevelIndentsApplicable = false;
    }
    else if (GetItemState(nWhich, false ) == SfxItemState::SET)
    {
        // paragraph style has hard-set indent attributes
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET )
    {
        // list style is directly applied to paragraph style and paragraph
        // style has no hard-set indent attributes
        bAreListLevelIndentsApplicable = true;
    }
    else
    {
        // list style is applied through one of the parent paragraph styles and
        // paragraph style has no hard-set indent attributes

        // check parent paragraph styles
        const SwTextFormatColl* pColl = dynamic_cast<const SwTextFormatColl*>(DerivedFrom());
        while ( pColl )
        {
            if (pColl->GetAttrSet().GetItemState(nWhich, false) == SfxItemState::SET)
            {
                // indent attributes found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET )
            {
                // paragraph style with the list style found and until now no
                // indent attributes are found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = true;
                break;
            }

            pColl = dynamic_cast<const SwTextFormatColl*>(pColl->DerivedFrom());
            OSL_ENSURE( pColl,
                    "<SwTextFormatColl::AreListLevelIndentsApplicable()> - something wrong in paragraph style hierarchy. The applied list style is not found." );
        }
    }

    return bAreListLevelIndentsApplicable;
}

void SwTextFormatColl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColl"));
    if (mpNextTextFormatColl)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("next"), BAD_CAST(mpNextTextFormatColl->GetName().toUtf8().getStr()));
    }
    if (mpLinkedCharFormat)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("linked"), BAD_CAST(mpLinkedCharFormat->GetName().toUtf8().getStr()));
    }

    SwFormat::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

void SwTextFormatColls::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColls"));
    for (size_t i = 0; i < size(); ++i)
        GetFormat(i)->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

//FEATURE::CONDCOLL

SwCollCondition::SwCollCondition( SwTextFormatColl* pColl, Master_CollCondition nMasterCond, sal_uInt32 nSubCond )
    : m_nCondition(nMasterCond)
    , m_nSubCondition(nSubCond)
    , m_pCollection(pColl)
{
}

SwCollCondition::SwCollCondition( const SwCollCondition& rCopy )
    : m_nCondition(rCopy.m_nCondition)
    , m_nSubCondition(rCopy.m_nSubCondition)
    , m_pCollection(rCopy.m_pCollection)
{
}

SwCollCondition::~SwCollCondition()
{
}

void SwCollCondition::RegisterToFormat(SwTextFormatColl& rColl)
{
    m_pCollection = &rColl;
}

bool SwCollCondition::operator==( const SwCollCondition& rCmp ) const
{
    return ( m_nCondition == rCmp.m_nCondition )
        && ( m_nSubCondition == rCmp.m_nSubCondition );
}

void SwCollCondition::SetCondition( Master_CollCondition nCond, sal_uInt32 nSubCond )
{
    m_nCondition = nCond;
    m_nSubCondition = nSubCond;
}

SwConditionTextFormatColl::~SwConditionTextFormatColl()
{
}

const SwCollCondition* SwConditionTextFormatColl::HasCondition(
                        const SwCollCondition& rCond ) const
{
    for (const auto &rpFnd : m_CondColls)
    {
        if (*rpFnd == rCond)
            return rpFnd.get();
    }

    return nullptr;
}

void SwConditionTextFormatColl::InsertCondition( const SwCollCondition& rCond )
{
    for (SwFormatCollConditions::size_type n = 0; n < m_CondColls.size(); ++n)
    {
        if (*m_CondColls[ n ] == rCond)
        {
            m_CondColls.erase( m_CondColls.begin() + n );
            break;
        }
    }

    // Not found -> so insert it
    m_CondColls.push_back( std::make_unique<SwCollCondition> (rCond) );
}

void SwConditionTextFormatColl::RemoveCondition( const SwCollCondition& rCond )
{
    for (SwFormatCollConditions::size_type n = 0; n < m_CondColls.size(); ++n)
    {
        if (*m_CondColls[ n ] == rCond)
        {
            m_CondColls.erase( m_CondColls.begin() + n );
        }
    }
}

void SwConditionTextFormatColl::SetConditions( const SwFormatCollConditions& rCndClls )
{
    // Copy the Conditions, but first delete the old ones
    m_CondColls.clear();
    SwDoc& rDoc = *GetDoc();
    for (const auto &rpFnd : rCndClls)
    {
        SwTextFormatColl *const pTmpColl = rpFnd->GetTextFormatColl()
                            ? rDoc.CopyTextColl( *rpFnd->GetTextFormatColl() )
                            : nullptr;
        std::unique_ptr<SwCollCondition> pNew;
        pNew.reset(new SwCollCondition( pTmpColl, rpFnd->GetCondition(),
                                            rpFnd->GetSubCondition() ));
        m_CondColls.push_back( std::move(pNew) );
    }
}

void SwTextFormatColl::SetAttrOutlineLevel( int nLevel)
{
    OSL_ENSURE( 0 <= nLevel && nLevel <= MAXLEVEL ,"SwTextFormatColl: Level Out Of Range" );
    SetFormatAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                            o3tl::narrowing<sal_uInt16>(nLevel) ) );
}

int SwTextFormatColl::GetAttrOutlineLevel() const
{
    return GetFormatAttr(RES_PARATR_OUTLINELEVEL).GetValue();
}

int SwTextFormatColl::GetAssignedOutlineStyleLevel() const
{
    OSL_ENSURE( IsAssignedToListLevelOfOutlineStyle(),
        "<SwTextFormatColl::GetAssignedOutlineStyleLevel()> - misuse of method");
    return GetAttrOutlineLevel() - 1;
}

void SwTextFormatColl::AssignToListLevelOfOutlineStyle(const int nAssignedListLevel)
{
    mbAssignedToOutlineStyle = true;
    SetAttrOutlineLevel(nAssignedListLevel+1);

    // #i100277#
    SwIterator<SwTextFormatColl,SwFormatColl> aIter( *this );
    SwTextFormatColl* pDerivedTextFormatColl = aIter.First();
    while ( pDerivedTextFormatColl != nullptr )
    {
        if ( !pDerivedTextFormatColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            if ( pDerivedTextFormatColl->GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::DEFAULT )
            {
                SwNumRuleItem aItem;
                pDerivedTextFormatColl->SetFormatAttr( aItem );
            }
            if ( pDerivedTextFormatColl->GetItemState( RES_PARATR_OUTLINELEVEL, false ) == SfxItemState::DEFAULT )
            {
                pDerivedTextFormatColl->SetAttrOutlineLevel( 0 );
            }
        }

        pDerivedTextFormatColl = aIter.Next();
    }
}

void SwTextFormatColl::DeleteAssignmentToListLevelOfOutlineStyle()
{
    mbAssignedToOutlineStyle = false;
    ResetFormatAttr(RES_PARATR_OUTLINELEVEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
