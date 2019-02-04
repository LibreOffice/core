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

#include <sal/macros.h>
#include <osl/diagnose.h>
#include <hintids.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <doc.hxx>
#include <fmtcol.hxx>
#include <fmtcolfunc.hxx>
#include <hints.hxx>
#include <calc.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <calbck.hxx>
#include <svl/intitem.hxx>

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
        if ( !pTextFormatColl->StayAssignedToListLevelOfOutlineStyle() &&
             pTextFormatColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            if (!pNewNumRuleItem)
            {
                (void)pTextFormatColl->GetItemState(RES_PARATR_NUMRULE, false, reinterpret_cast<const SfxPoolItem**>(&pNewNumRuleItem));
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
    }

    SwNumRule* GetNumRule( SwTextFormatColl& rTextFormatColl )
    {
        SwNumRule* pNumRule( nullptr );

        const SwNumRuleItem* pNumRuleItem(nullptr);
        (void)rTextFormatColl.GetItemState(RES_PARATR_NUMRULE, false, reinterpret_cast<const SfxPoolItem**>(&pNumRuleItem));
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

void SwTextFormatColl::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( GetDoc()->IsInDtor() )
    {
        SwFormatColl::Modify( pOld, pNew );
        return;
    }

    bool bNewParent( false ); // #i66431# - adjust type of <bNewParent>
    const SvxULSpaceItem *pNewULSpace = nullptr, *pOldULSpace = nullptr;
    const SvxLRSpaceItem *pNewLRSpace = nullptr, *pOldLRSpace = nullptr;
    const SvxFontHeightItem* aFontSizeArr[3] = {nullptr,nullptr,nullptr};
    // #i70223#
    const bool bAssignedToListLevelOfOutlineStyle(IsAssignedToListLevelOfOutlineStyle());
    const SwNumRuleItem* pNewNumRuleItem( nullptr );

    const SwAttrSetChg *pNewChgSet = nullptr,  *pOldChgSet = nullptr;

    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        // Only recalculate if we're not the sender!
        pNewChgSet = static_cast<const SwAttrSetChg*>(pNew);
        pOldChgSet = static_cast<const SwAttrSetChg*>(pOld);
        pNewChgSet->GetChgSet()->GetItemState(
            RES_LR_SPACE, false, reinterpret_cast<const SfxPoolItem**>(&pNewLRSpace) );
        pNewChgSet->GetChgSet()->GetItemState(
            RES_UL_SPACE, false, reinterpret_cast<const SfxPoolItem**>(&pNewULSpace) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_FONTSIZE,
                        false, reinterpret_cast<const SfxPoolItem**>(&(aFontSizeArr[0])) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CJK_FONTSIZE,
                        false, reinterpret_cast<const SfxPoolItem**>(&(aFontSizeArr[1])) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CTL_FONTSIZE,
                        false, reinterpret_cast<const SfxPoolItem**>(&(aFontSizeArr[2])) );
        // #i70223#, #i84745#
        // check, if attribute set is applied to this paragraph style
        if ( bAssignedToListLevelOfOutlineStyle &&
             pNewChgSet->GetTheChgdSet() == &GetAttrSet() )
        {
            pNewChgSet->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, false,
                                                   reinterpret_cast<const SfxPoolItem**>(&pNewNumRuleItem) );
        }

        break;

    case RES_FMT_CHG:
        if( GetAttrSet().GetParent() )
        {
            const SfxItemSet* pParent = GetAttrSet().GetParent();
            pNewLRSpace = &pParent->Get( RES_LR_SPACE );
            pNewULSpace = &pParent->Get( RES_UL_SPACE );
            aFontSizeArr[0] = &pParent->Get( RES_CHRATR_FONTSIZE );
            aFontSizeArr[1] = &pParent->Get( RES_CHRATR_CJK_FONTSIZE );
            aFontSizeArr[2] = &pParent->Get( RES_CHRATR_CTL_FONTSIZE );
            // #i66431# - modify has to be propagated, because of new parent format.
            bNewParent = true;
        }
        break;

    case RES_LR_SPACE:
        pNewLRSpace = static_cast<const SvxLRSpaceItem*>(pNew);
        break;
    case RES_UL_SPACE:
        pNewULSpace = static_cast<const SvxULSpaceItem*>(pNew);
        break;
    case RES_CHRATR_FONTSIZE:
        aFontSizeArr[0] = static_cast<const SvxFontHeightItem*>(pNew);
        break;
    case RES_CHRATR_CJK_FONTSIZE:
        aFontSizeArr[1] = static_cast<const SvxFontHeightItem*>(pNew);
        break;
    case RES_CHRATR_CTL_FONTSIZE:
        aFontSizeArr[2] = static_cast<const SvxFontHeightItem*>(pNew);
        break;
    // #i70223#
    case RES_PARATR_NUMRULE:
        if (bAssignedToListLevelOfOutlineStyle)
        {
            pNewNumRuleItem = static_cast<const SwNumRuleItem*>(pNew);
        }
        break;
    default:
        break;
    }

    // #i70223#
    if ( bAssignedToListLevelOfOutlineStyle && pNewNumRuleItem )
    {
        TextFormatCollFunc::CheckTextFormatCollForDeletionOfAssignmentToOutlineStyle(
                                                        this, pNewNumRuleItem );
    }

    bool bContinue = true;

    // Check against the own attributes
    if( pNewLRSpace && SfxItemState::SET == GetItemState( RES_LR_SPACE, false,
                                        reinterpret_cast<const SfxPoolItem**>(&pOldLRSpace) ))
    {
        if( pOldLRSpace != pNewLRSpace )    // Avoid recursion (SetAttr!)
        {
            bool bChg = false;
            SvxLRSpaceItem aNew( *pOldLRSpace );
            // We had a relative value -> recalculate
            if( 100 != aNew.GetPropLeft() )
            {
                long nTmp = aNew.GetLeft();     // keep so that we can compare
                aNew.SetLeft( pNewLRSpace->GetLeft(), aNew.GetPropLeft() );
                bChg |= nTmp != aNew.GetLeft();
            }
            // We had a relative value -> recalculate
            if( 100 != aNew.GetPropRight() )
            {
                long nTmp = aNew.GetRight();    // keep so that we can compare
                aNew.SetRight( pNewLRSpace->GetRight(), aNew.GetPropRight() );
                bChg |= nTmp != aNew.GetRight();
            }
            // We had a relative value -> recalculate
            if( 100 != aNew.GetPropTextFirstLineOfst() )
            {
                short nTmp = aNew.GetTextFirstLineOfst();    // keep so that we can compare
                aNew.SetTextFirstLineOfst( pNewLRSpace->GetTextFirstLineOfst(),
                                            aNew.GetPropTextFirstLineOfst() );
                bChg |= nTmp != aNew.GetTextFirstLineOfst();
            }
            if( bChg )
            {
                SetFormatAttr( aNew );
                bContinue = nullptr != pOldChgSet || bNewParent;
            }
            // We set it to absolute -> do not propagate it further, unless
            // we set it!
            else if( pNewChgSet )
                bContinue = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
        }
    }

    if( pNewULSpace && SfxItemState::SET == GetItemState(
            RES_UL_SPACE, false, reinterpret_cast<const SfxPoolItem**>(&pOldULSpace) ) &&
        pOldULSpace != pNewULSpace )    // Avoid recursion (SetAttr!)
    {
        SvxULSpaceItem aNew( *pOldULSpace );
        bool bChg = false;
        // We had a relative value -> recalculate
        if( 100 != aNew.GetPropUpper() )
        {
            sal_uInt16 nTmp = aNew.GetUpper();      // keep so that we can compare
            aNew.SetUpper( pNewULSpace->GetUpper(), aNew.GetPropUpper() );
            bChg |= nTmp != aNew.GetUpper();
        }
        // We had a relative value -> recalculate
        if( 100 != aNew.GetPropLower() )
        {
            sal_uInt16 nTmp = aNew.GetLower();      // keep so that we can compare
            aNew.SetLower( pNewULSpace->GetLower(), aNew.GetPropLower() );
            bChg |= nTmp != aNew.GetLower();
        }
        if( bChg )
        {
            SetFormatAttr( aNew );
            bContinue = nullptr != pOldChgSet || bNewParent;
        }
        // We set it to absolute -> do not propagate it further, unless
        // we set it!
        else if( pNewChgSet )
            bContinue = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
    }

    for( int nC = 0; nC < int(SAL_N_ELEMENTS(aFontSizeArr)); ++nC )
    {
        const SvxFontHeightItem *pFSize = aFontSizeArr[ nC ], *pOldFSize;
        if( pFSize && SfxItemState::SET == GetItemState(
            pFSize->Which(), false, reinterpret_cast<const SfxPoolItem**>(&pOldFSize) ) &&
            // Avoid recursion (SetAttr!)
            pFSize != pOldFSize )
        {
            if( 100 == pOldFSize->GetProp() &&
                MapUnit::MapRelative == pOldFSize->GetPropUnit() )
            {
                // We set it to absolute -> do not propagate it further, unless
                // we set it!
                if( pNewChgSet )
                    bContinue = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
            }
            else
            {
                // We had a relative value -> recalculate
                sal_uInt32 nTmp = pOldFSize->GetHeight();       // keep so that we can compare
                SvxFontHeightItem aNew(240 , 100, pFSize->Which());
                aNew.SetHeight( pFSize->GetHeight(), pOldFSize->GetProp(),
                                pOldFSize->GetPropUnit() );
                if( nTmp != aNew.GetHeight() )
                {
                    SetFormatAttr( aNew );
                    bContinue = nullptr != pOldChgSet || bNewParent;
                }
                // We set it to absolute -> do not propagate it further, unless
                // we set it!
                else if( pNewChgSet )
                    bContinue = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
            }
        }
    }

    if( bContinue )
        SwFormatColl::Modify( pOld, pNew );
}

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
    // Outline level is no longer a member, it is a attribute now.
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

bool SwTextFormatColl::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( GetItemState( RES_PARATR_NUMRULE ) != SfxItemState::SET )
    {
        // no list style applied to paragraph style
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_LR_SPACE, false ) == SfxItemState::SET )
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
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, false ) == SfxItemState::SET )
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColl"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    GetAttrSet().dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

void SwTextFormatColls::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColls"));
    for (size_t i = 0; i < size(); ++i)
        GetFormat(i)->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

//FEATURE::CONDCOLL

SwCollCondition::SwCollCondition( SwTextFormatColl* pColl, Master_CollCondition nMasterCond,
                                sal_uLong nSubCond )
    : SwClient( pColl ), m_nCondition( nMasterCond ),
      m_nSubCondition( nSubCond )
{
}

SwCollCondition::SwCollCondition( const SwCollCondition& rCopy )
    : SwClient( const_cast<SwModify*>(rCopy.GetRegisteredIn()) ),
      m_nCondition( rCopy.m_nCondition ),
      m_nSubCondition( rCopy.m_nSubCondition )
{
}

SwCollCondition::~SwCollCondition()
{
}

void SwCollCondition::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add( this );
}

bool SwCollCondition::operator==( const SwCollCondition& rCmp ) const
{
    return ( m_nCondition == rCmp.m_nCondition )
        && ( m_nSubCondition == rCmp.m_nSubCondition );
}

void SwCollCondition::SetCondition( Master_CollCondition nCond, sal_uLong nSubCond )
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

// FEATURE::CONDCOLL
void SwTextFormatColl::SetAttrOutlineLevel( int nLevel)
{
    OSL_ENSURE( 0 <= nLevel && nLevel <= MAXLEVEL ,"SwTextFormatColl: Level Out Of Range" );
    SetFormatAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                            static_cast<sal_uInt16>(nLevel) ) );
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
