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

#include <sal/macros.h>
#include <hintids.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <doc.hxx>          // for GetAttrPool
#include <fmtcol.hxx>
#include <fmtcolfunc.hxx>
#include <hints.hxx>
#include <calc.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <switerator.hxx>
#include <svl/intitem.hxx>

TYPEINIT1( SwTxtFmtColl, SwFmtColl );
TYPEINIT1( SwGrfFmtColl, SwFmtColl );
TYPEINIT1( SwConditionTxtFmtColl, SwTxtFmtColl );
TYPEINIT1( SwCollCondition, SwClient );

namespace TxtFmtCollFunc
{
    // #i71574#
    void CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle(
                                            SwFmt* pFmt,
                                            const SwNumRuleItem* pNewNumRuleItem )
    {
        SwTxtFmtColl* pTxtFmtColl = dynamic_cast<SwTxtFmtColl*>(pFmt);
        if ( !pTxtFmtColl )
        {
            OSL_FAIL( "<TxtFmtCollFunc::CheckTxtFmtCollFuncForDeletionOfAssignmentToOutlineStyle> - misuse of method - it's only for instances of <SwTxtFmtColl>" );
            return;
        }

        // #i73790#
        if ( !pTxtFmtColl->StayAssignedToListLevelOfOutlineStyle() &&
             pTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            if ( !pNewNumRuleItem )
            {
                pTxtFmtColl->GetItemState( RES_PARATR_NUMRULE, sal_False, (const SfxPoolItem**)&pNewNumRuleItem );
            }
            if ( pNewNumRuleItem )
            {
                String sNumRuleName = pNewNumRuleItem->GetValue();
                if ( sNumRuleName.Len() == 0 ||
                     sNumRuleName != pTxtFmtColl->GetDoc()->GetOutlineNumRule()->GetName() )
                {
                    // delete assignment of paragraph style to list level of outline style.
                    pTxtFmtColl->DeleteAssignmentToListLevelOfOutlineStyle();
                }
            }
        }
    }

    SwNumRule* GetNumRule( SwTxtFmtColl& rTxtFmtColl )
    {
        SwNumRule* pNumRule( 0 );

        const SwNumRuleItem* pNumRuleItem( 0 );
        rTxtFmtColl.GetItemState( RES_PARATR_NUMRULE, sal_False, (const SfxPoolItem**)&pNumRuleItem );
        if ( pNumRuleItem )
        {
            const String sNumRuleName = pNumRuleItem->GetValue();
            if ( sNumRuleName.Len() > 0 )
            {
                pNumRule = rTxtFmtColl.GetDoc()->FindNumRulePtr( sNumRuleName );
            }
        }

        return pNumRule;
    }

    void AddToNumRule( SwTxtFmtColl& rTxtFmtColl )
    {
        SwNumRule* pNumRule = GetNumRule( rTxtFmtColl );
        if ( pNumRule )
        {
            pNumRule->AddParagraphStyle( rTxtFmtColl );
        }
    }

    void RemoveFromNumRule( SwTxtFmtColl& rTxtFmtColl )
    {
        SwNumRule* pNumRule = GetNumRule( rTxtFmtColl );
        if ( pNumRule )
        {
            pNumRule->RemoveParagraphStyle( rTxtFmtColl );
        }
    }
} // end of namespace TxtFmtCollFunc

void SwTxtFmtColl::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( GetDoc()->IsInDtor() )
    {
        SwFmtColl::Modify( pOld, pNew );
        return;
    }

    bool bNewParent( false ); // #i66431# - adjust type of <bNewParent>
    SvxULSpaceItem *pNewULSpace = 0, *pOldULSpace = 0;
    SvxLRSpaceItem *pNewLRSpace = 0, *pOldLRSpace = 0;
    SvxFontHeightItem* aFontSizeArr[3] = {0,0,0};
    // #i70223#
    const bool bAssignedToListLevelOfOutlineStyle(IsAssignedToListLevelOfOutlineStyle());
    const SwNumRuleItem* pNewNumRuleItem( 0L );

    SwAttrSetChg *pNewChgSet = 0,  *pOldChgSet = 0;

    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        // Only recalculate if we're not the sender!
        pNewChgSet = (SwAttrSetChg*)pNew;
        pOldChgSet = (SwAttrSetChg*)pOld;
        pNewChgSet->GetChgSet()->GetItemState(
            RES_LR_SPACE, sal_False, (const SfxPoolItem**)&pNewLRSpace );
        pNewChgSet->GetChgSet()->GetItemState(
            RES_UL_SPACE, sal_False, (const SfxPoolItem**)&pNewULSpace );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_FONTSIZE,
                        sal_False, (const SfxPoolItem**)&(aFontSizeArr[0]) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CJK_FONTSIZE,
                        sal_False, (const SfxPoolItem**)&(aFontSizeArr[1]) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CTL_FONTSIZE,
                        sal_False, (const SfxPoolItem**)&(aFontSizeArr[2]) );
        // #i70223#, #i84745#
        // check, if attribute set is applied to this paragraph style
        if ( bAssignedToListLevelOfOutlineStyle &&
             pNewChgSet->GetTheChgdSet() == &GetAttrSet() )
        {
            pNewChgSet->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, sal_False,
                                                   (const SfxPoolItem**)&pNewNumRuleItem );
        }

        break;

    case RES_FMT_CHG:
        if( GetAttrSet().GetParent() )
        {
            const SfxItemSet* pParent = GetAttrSet().GetParent();
            pNewLRSpace = (SvxLRSpaceItem*)&pParent->Get( RES_LR_SPACE );
            pNewULSpace = (SvxULSpaceItem*)&pParent->Get( RES_UL_SPACE );
            aFontSizeArr[0] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_FONTSIZE );
            aFontSizeArr[1] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_CJK_FONTSIZE );
            aFontSizeArr[2] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_CTL_FONTSIZE );
            // #i66431# - modify has to be propagated, because of new parent format.
            bNewParent = true;
        }
        break;

    case RES_LR_SPACE:
        pNewLRSpace = (SvxLRSpaceItem*)pNew;
        break;
    case RES_UL_SPACE:
        pNewULSpace = (SvxULSpaceItem*)pNew;
        break;
    case RES_CHRATR_FONTSIZE:
        aFontSizeArr[0] = (SvxFontHeightItem*)pNew;
        break;
    case RES_CHRATR_CJK_FONTSIZE:
        aFontSizeArr[1] = (SvxFontHeightItem*)pNew;
        break;
    case RES_CHRATR_CTL_FONTSIZE:
        aFontSizeArr[2] = (SvxFontHeightItem*)pNew;
        break;
    // #i70223#
    case RES_PARATR_NUMRULE:
    {
        if ( bAssignedToListLevelOfOutlineStyle )
        {
            pNewNumRuleItem = (SwNumRuleItem*)pNew;
        }
    }
    default:
        break;
    }

    // #i70223#
    if ( bAssignedToListLevelOfOutlineStyle && pNewNumRuleItem )
    {
        TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle(
                                                        this, pNewNumRuleItem );
    }

    int bWeiter = sal_True;

    // Check against the own attributes
    if( pNewLRSpace && SFX_ITEM_SET == GetItemState( RES_LR_SPACE, sal_False,
                                        (const SfxPoolItem**)&pOldLRSpace ))
    {
        int bChg = sal_False;
        if( pOldLRSpace != pNewLRSpace )    // Avoid recursion (SetAttr!)
        {
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
            if( 100 != aNew.GetPropTxtFirstLineOfst() )
            {
                short nTmp = aNew.GetTxtFirstLineOfst();    // keep so that we can compare
                aNew.SetTxtFirstLineOfst( pNewLRSpace->GetTxtFirstLineOfst(),
                                            aNew.GetPropTxtFirstLineOfst() );
                bChg |= nTmp != aNew.GetTxtFirstLineOfst();
            }
            if( bChg )
            {
                SetFmtAttr( aNew );
                bWeiter = 0 != pOldChgSet || bNewParent;
            }
            // We set it to absolute -> do not propagate it further, unless
            // we set it!
            else if( pNewChgSet )
                bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
        }
    }

    if( pNewULSpace && SFX_ITEM_SET == GetItemState(
            RES_UL_SPACE, sal_False, (const SfxPoolItem**)&pOldULSpace ) &&
        pOldULSpace != pNewULSpace )    // Avoid recursion (SetAttr!)
    {
        SvxULSpaceItem aNew( *pOldULSpace );
        int bChg = sal_False;
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
            SetFmtAttr( aNew );
            bWeiter = 0 != pOldChgSet || bNewParent;
        }
        // We set it to absolute -> do not propagate it further, unless
        // we set it!
        else if( pNewChgSet )
            bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
    }

    for( int nC = 0, nArrLen = sizeof(aFontSizeArr) / sizeof( aFontSizeArr[0]);
            nC < nArrLen; ++nC )
    {
        SvxFontHeightItem *pFSize = aFontSizeArr[ nC ], *pOldFSize;
        if( pFSize && SFX_ITEM_SET == GetItemState(
            pFSize->Which(), sal_False, (const SfxPoolItem**)&pOldFSize ) &&
            // Avoid recursion (SetAttr!)
            pFSize != pOldFSize )
        {
            if( 100 == pOldFSize->GetProp() &&
                SFX_MAPUNIT_RELATIVE == pOldFSize->GetPropUnit() )
            {
                // We set it to absolute -> do not propagate it further, unless
                // we set it!
                if( pNewChgSet )
                    bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
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
                    SetFmtAttr( aNew );
                    bWeiter = 0 != pOldChgSet || bNewParent;
                }
                // We set it to absolute -> do not propagate it further, unless
                // we set it!
                else if( pNewChgSet )
                    bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
            }
        }
    }

    if( bWeiter )
        SwFmtColl::Modify( pOld, pNew );
}

bool SwTxtFmtColl::IsAtDocNodeSet() const
{
    SwIterator<SwCntntNode,SwFmtColl> aIter( *this );
    const SwNodes& rNds = GetDoc()->GetNodes();
    for( SwCntntNode* pNode = aIter.First(); pNode; pNode = aIter.Next() )
        if( &(pNode->GetNodes()) == &rNds )
            return true;

    return false;
}

bool SwTxtFmtColl::SetFmtAttr( const SfxPoolItem& rAttr )
{
    const bool bIsNumRuleItem = rAttr.Which() == RES_PARATR_NUMRULE;
    if ( bIsNumRuleItem )
    {
        TxtFmtCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFmtColl::SetFmtAttr( rAttr );

    if ( bIsNumRuleItem )
    {
        TxtFmtCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

bool SwTxtFmtColl::SetFmtAttr( const SfxItemSet& rSet )
{
    const bool bIsNumRuleItemAffected =
                rSet.GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET;
    if ( bIsNumRuleItemAffected )
    {
        TxtFmtCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFmtColl::SetFmtAttr( rSet );

    if ( bIsNumRuleItemAffected )
    {
        TxtFmtCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

bool SwTxtFmtColl::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    const bool bIsNumRuleItemAffected =
                ( nWhich2 != 0 && nWhich2 > nWhich1 )
                ? ( nWhich1 <= RES_PARATR_NUMRULE &&
                    RES_PARATR_NUMRULE <= nWhich2 )
                : nWhich1 == RES_PARATR_NUMRULE;
    if ( bIsNumRuleItemAffected )
    {
        TxtFmtCollFunc::RemoveFromNumRule( *this );
    }

    const bool bRet = SwFmtColl::ResetFmtAttr( nWhich1, nWhich2 );

    return bRet;
}

// #i73790#
sal_uInt16 SwTxtFmtColl::ResetAllFmtAttr()
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

    sal_uInt16 nRet = SwFmtColl::ResetAllFmtAttr();

    // #i70748#
    if ( nAssignedOutlineStyleLevel != -1 )
    {
        AssignToListLevelOfOutlineStyle( nAssignedOutlineStyleLevel );
    }

    mbStayAssignedToListLevelOfOutlineStyle = bOldState;

    return nRet;
}

bool SwTxtFmtColl::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( GetItemState( RES_PARATR_NUMRULE ) != SFX_ITEM_SET )
    {
        // no list style applied to paragraph style
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_LR_SPACE, sal_False ) == SFX_ITEM_SET )
    {
        // paragraph style has hard-set indent attributes
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET )
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
        const SwTxtFmtColl* pColl = dynamic_cast<const SwTxtFmtColl*>(DerivedFrom());
        while ( pColl )
        {
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, sal_False ) == SFX_ITEM_SET )
            {
                // indent attributes found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET )
            {
                // paragraph style with the list style found and until now no
                // indent attributes are found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = true;
                break;
            }

            pColl = dynamic_cast<const SwTxtFmtColl*>(pColl->DerivedFrom());
            OSL_ENSURE( pColl,
                    "<SwTxtFmtColl::AreListLevelIndentsApplicable()> - something wrong in paragraph style hierarchy. The applied list style is not found." );
        }
    }

    return bAreListLevelIndentsApplicable;
}

//FEATURE::CONDCOLL

SwCollCondition::SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                                sal_uLong nSubCond )
    : SwClient( pColl ), nCondition( nMasterCond )
{
    aSubCondition.nSubCondition = nSubCond;
}

SwCollCondition::SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                                    const String& rSubExp )
    : SwClient( pColl ), nCondition( nMasterCond )
{
    if( USRFLD_EXPRESSION & nCondition )
        aSubCondition.pFldExpression = new String( rSubExp );
    else
        aSubCondition.nSubCondition = 0;
}

SwCollCondition::SwCollCondition( const SwCollCondition& rCopy )
    : SwClient( (SwModify*)rCopy.GetRegisteredIn() ), nCondition( rCopy.nCondition )
{
    if( USRFLD_EXPRESSION & rCopy.nCondition )
        aSubCondition.pFldExpression = new String( *rCopy.GetFldExpression() );
    else
        aSubCondition.nSubCondition = rCopy.aSubCondition.nSubCondition;
}

SwCollCondition::~SwCollCondition()
{
    if( USRFLD_EXPRESSION & nCondition )
        delete aSubCondition.pFldExpression;
}

void SwCollCondition::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

int SwCollCondition::operator==( const SwCollCondition& rCmp ) const
{
    int nRet = 0;
    if( nCondition == rCmp.nCondition )
    {
        if( USRFLD_EXPRESSION & nCondition )
        {
            // The SubCondition contains the expression for the UserField
            const String* pTmp = aSubCondition.pFldExpression;
            if( !pTmp )
                pTmp = rCmp.aSubCondition.pFldExpression;
            if( pTmp )
            {
                SwTxtFmtColl* pColl = GetTxtFmtColl();
                if( !pColl )
                    pColl = rCmp.GetTxtFmtColl();

                if( pColl )
                {
                    SwCalc aCalc( *pColl->GetDoc() );
                    nRet = 0 != aCalc.Calculate( *pTmp ).GetBool();
                }
            }
        }
        else if( aSubCondition.nSubCondition ==
                    rCmp.aSubCondition.nSubCondition )
            nRet = 1;
    }
    return nRet;
}

void SwCollCondition::SetCondition( sal_uLong nCond, sal_uLong nSubCond )
{
    if( USRFLD_EXPRESSION & nCondition )
        delete aSubCondition.pFldExpression;
    nCondition = nCond;
    aSubCondition.nSubCondition = nSubCond;
}

SwConditionTxtFmtColl::~SwConditionTxtFmtColl()
{
}

const SwCollCondition* SwConditionTxtFmtColl::HasCondition(
                        const SwCollCondition& rCond ) const
{
    const SwCollCondition* pFnd = 0;
    sal_uInt16 n;

    for( n = 0; n < aCondColls.size(); ++n )
        if( *( pFnd = &aCondColls[ n ]) == rCond )
            break;

    return n < aCondColls.size() ? pFnd : 0;
}

void SwConditionTxtFmtColl::InsertCondition( const SwCollCondition& rCond )
{
    for( sal_uInt16 n = 0; n < aCondColls.size(); ++n )
        if( aCondColls[ n ] == rCond )
        {
            aCondColls.erase( aCondColls.begin() + n );
            break;
        }

    // Not found -> so insert it
    SwCollCondition* pNew = new SwCollCondition( rCond );
    aCondColls.push_back( pNew );
}

bool SwConditionTxtFmtColl::RemoveCondition( const SwCollCondition& rCond )
{
    bool bRet = false;
    for( sal_uInt16 n = 0; n < aCondColls.size(); ++n )
        if( aCondColls[ n ] == rCond )
        {
            aCondColls.erase( aCondColls.begin() + n );
            bRet = true;
        }

    return bRet;
}

void SwConditionTxtFmtColl::SetConditions( const SwFmtCollConditions& rCndClls )
{
    // Copy the Conditions, but first delete the old ones
    aCondColls.clear();
    SwDoc& rDoc = *GetDoc();
    for( sal_uInt16 n = 0; n < rCndClls.size(); ++n )
    {
        const SwCollCondition* pFnd = &rCndClls[ n ];
        SwTxtFmtColl* pTmpColl = pFnd->GetTxtFmtColl()
                                    ? rDoc.CopyTxtColl( *pFnd->GetTxtFmtColl() )
                                    : 0;
        SwCollCondition* pNew;
        if( USRFLD_EXPRESSION & pFnd->GetCondition() )
            pNew = new SwCollCondition( pTmpColl, pFnd->GetCondition(),
                                        *pFnd->GetFldExpression() );
        else
            pNew = new SwCollCondition( pTmpColl, pFnd->GetCondition(),
                                        pFnd->GetSubCondition() );
        aCondColls.push_back( pNew );
    }
}

void SwTxtFmtColl::SetAttrOutlineLevel( int nLevel)
{
    OSL_ENSURE( 0 <= nLevel && nLevel <= MAXLEVEL ,"SwTxtFmtColl: Level Out Of Range" );
    SetFmtAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                            static_cast<sal_uInt16>(nLevel) ) );
}

int SwTxtFmtColl::GetAttrOutlineLevel() const
{
    return ((const SfxUInt16Item &)GetFmtAttr(RES_PARATR_OUTLINELEVEL)).GetValue();
}

int SwTxtFmtColl::GetAssignedOutlineStyleLevel() const
{
    OSL_ENSURE( IsAssignedToListLevelOfOutlineStyle(),
        "<SwTxtFmtColl::GetAssignedOutlineStyleLevel()> - misuse of method");
    return GetAttrOutlineLevel() - 1;
}

void SwTxtFmtColl::AssignToListLevelOfOutlineStyle(const int nAssignedListLevel)
{
    mbAssignedToOutlineStyle = true;
    SetAttrOutlineLevel(nAssignedListLevel+1);

    // #i100277#
    SwIterator<SwTxtFmtColl,SwFmtColl> aIter( *this );
    SwTxtFmtColl* pDerivedTxtFmtColl = aIter.First();
    while ( pDerivedTxtFmtColl != 0 )
    {
        if ( !pDerivedTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            if ( pDerivedTxtFmtColl->GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_DEFAULT )
            {
                SwNumRuleItem aItem(aEmptyStr);
                pDerivedTxtFmtColl->SetFmtAttr( aItem );
            }
            if ( pDerivedTxtFmtColl->GetItemState( RES_PARATR_OUTLINELEVEL, sal_False ) == SFX_ITEM_DEFAULT )
            {
                pDerivedTxtFmtColl->SetAttrOutlineLevel( 0 );
            }
        }

        pDerivedTxtFmtColl = aIter.Next();
    }
}

void SwTxtFmtColl::DeleteAssignmentToListLevelOfOutlineStyle()
{
    mbAssignedToOutlineStyle = false;
    ResetFmtAttr(RES_PARATR_OUTLINELEVEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
