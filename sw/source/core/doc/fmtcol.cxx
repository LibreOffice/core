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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <sal/macros.h>
#include <hintids.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <doc.hxx>          // fuer GetAttrPool
#include <fmtcol.hxx>
#include <fmtcolfunc.hxx> // #i71574#
#include <hints.hxx>
#include <calc.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <paratr.hxx>

//--> #outlinelevel added by zhaojianwei
#include <svl/intitem.hxx>
//<--end

TYPEINIT1( SwTxtFmtColl, SwFmtColl );
TYPEINIT1( SwGrfFmtColl, SwFmtColl );
TYPEINIT1( SwConditionTxtFmtColl, SwTxtFmtColl );
TYPEINIT1( SwCollCondition, SwClient );

SV_IMPL_PTRARR( SwFmtCollConditions, SwCollConditionPtr );

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
    #if OSL_DEBUG_LEVEL > 1
            OSL_ENSURE( false,
                    "<TxtFmtCollFunc::CheckTxtFmtCollFuncForDeletionOfAssignmentToOutlineStyle> - misuse of method - it's only for instances of <SwTxtFmtColl>" );
    #endif
            return;
        }

        // #i73790#
        if ( !pTxtFmtColl->StayAssignedToListLevelOfOutlineStyle() &&
             pTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )
        // <--
        {
            if ( !pNewNumRuleItem )
            {
                pTxtFmtColl->GetItemState( RES_PARATR_NUMRULE, FALSE, (const SfxPoolItem**)&pNewNumRuleItem );
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
    // <--

    SwNumRule* GetNumRule( SwTxtFmtColl& rTxtFmtColl )
    {
        SwNumRule* pNumRule( 0 );

        const SwNumRuleItem* pNumRuleItem( 0 );
        rTxtFmtColl.GetItemState( RES_PARATR_NUMRULE, FALSE, (const SfxPoolItem**)&pNumRuleItem );
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
// <--

/*
 * SwTxtFmtColl  TXT
 */

void SwTxtFmtColl::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
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
    const bool bAssignedToListLevelOfOutlineStyle(IsAssignedToListLevelOfOutlineStyle());//#outline level ,zhaojianwei
    const SwNumRuleItem* pNewNumRuleItem( 0L );

    SwAttrSetChg *pNewChgSet = 0,  *pOldChgSet = 0;

    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        // nur neu berechnen, wenn nicht wir der "Versender" sind !!!
        pNewChgSet = (SwAttrSetChg*)pNew;
        pOldChgSet = (SwAttrSetChg*)pOld;
        pNewChgSet->GetChgSet()->GetItemState(
            RES_LR_SPACE, FALSE, (const SfxPoolItem**)&pNewLRSpace );
        pNewChgSet->GetChgSet()->GetItemState(
            RES_UL_SPACE, FALSE, (const SfxPoolItem**)&pNewULSpace );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_FONTSIZE,
                        FALSE, (const SfxPoolItem**)&(aFontSizeArr[0]) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CJK_FONTSIZE,
                        FALSE, (const SfxPoolItem**)&(aFontSizeArr[1]) );
        pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CTL_FONTSIZE,
                        FALSE, (const SfxPoolItem**)&(aFontSizeArr[2]) );
        // #i70223#, #i84745#
        // check, if attribute set is applied to this paragraph style
        if ( bAssignedToListLevelOfOutlineStyle &&
             pNewChgSet->GetTheChgdSet() == &GetAttrSet() )
        {
            pNewChgSet->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, FALSE,
                                                   (const SfxPoolItem**)&pNewNumRuleItem );
        }
        // <--

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
    // <--

    int bWeiter = TRUE;

    // dann pruefe doch mal gegen die eigenen Attribute
    if( pNewLRSpace && SFX_ITEM_SET == GetItemState( RES_LR_SPACE, FALSE,
                                        (const SfxPoolItem**)&pOldLRSpace ))
    {
        int bChg = FALSE;
        if( pOldLRSpace != pNewLRSpace )    // verhinder Rekursion (SetAttr!!)
        {
            SvxLRSpaceItem aNew( *pOldLRSpace );
            // wir hatten eine relative Angabe -> neu berechnen
            if( 100 != aNew.GetPropLeft() )
            {
                long nTmp = aNew.GetLeft();     // alten zum Vergleichen
                aNew.SetLeft( pNewLRSpace->GetLeft(), aNew.GetPropLeft() );
                bChg |= nTmp != aNew.GetLeft();
            }
            // wir hatten eine relative Angabe -> neu berechnen
            if( 100 != aNew.GetPropRight() )
            {
                long nTmp = aNew.GetRight();        // alten zum Vergleichen
                aNew.SetRight( pNewLRSpace->GetRight(), aNew.GetPropRight() );
                bChg |= nTmp != aNew.GetRight();
            }
            // wir hatten eine relative Angabe -> neu berechnen
            if( 100 != aNew.GetPropTxtFirstLineOfst() )
            {
                short nTmp = aNew.GetTxtFirstLineOfst();        // alten zum Vergleichen
                aNew.SetTxtFirstLineOfst( pNewLRSpace->GetTxtFirstLineOfst(),
                                            aNew.GetPropTxtFirstLineOfst() );
                bChg |= nTmp != aNew.GetTxtFirstLineOfst();
            }
            if( bChg )
            {
                SetFmtAttr( aNew );
                bWeiter = 0 != pOldChgSet || bNewParent;
            }
            // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
            // denn es wird bei uns gesetzt!
            else if( pNewChgSet )
                bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
        }
    }

    if( pNewULSpace && SFX_ITEM_SET == GetItemState(
            RES_UL_SPACE, FALSE, (const SfxPoolItem**)&pOldULSpace ) &&
        pOldULSpace != pNewULSpace )    // verhinder Rekursion (SetAttr!!)
    {
        SvxULSpaceItem aNew( *pOldULSpace );
        int bChg = FALSE;
        // wir hatten eine relative Angabe -> neu berechnen
        if( 100 != aNew.GetPropUpper() )
        {
            USHORT nTmp = aNew.GetUpper();      // alten zum Vergleichen
            aNew.SetUpper( pNewULSpace->GetUpper(), aNew.GetPropUpper() );
            bChg |= nTmp != aNew.GetUpper();
        }
        // wir hatten eine relative Angabe -> neu berechnen
        if( 100 != aNew.GetPropLower() )
        {
            USHORT nTmp = aNew.GetLower();      // alten zum Vergleichen
            aNew.SetLower( pNewULSpace->GetLower(), aNew.GetPropLower() );
            bChg |= nTmp != aNew.GetLower();
        }
        if( bChg )
        {
            SetFmtAttr( aNew );
            bWeiter = 0 != pOldChgSet || bNewParent;
        }
        // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
        // denn es wird bei uns gesetzt!
        else if( pNewChgSet )
            bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
    }


    for( int nC = 0, nArrLen = SAL_N_ELEMENTS(aFontSizeArr);
            nC < nArrLen; ++nC )
    {
        SvxFontHeightItem *pFSize = aFontSizeArr[ nC ], *pOldFSize;
        if( pFSize && SFX_ITEM_SET == GetItemState(
            pFSize->Which(), FALSE, (const SfxPoolItem**)&pOldFSize ) &&
            // verhinder Rekursion (SetAttr!!)
            pFSize != pOldFSize )
        {
            if( 100 == pOldFSize->GetProp() &&
                SFX_MAPUNIT_RELATIVE == pOldFSize->GetPropUnit() )
            {
                // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
                // denn es wird bei uns gesetzt!
                if( pNewChgSet )
                    bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
            }
            else
            {
                // wir hatten eine relative Angabe -> neu berechnen
                UINT32 nTmp = pOldFSize->GetHeight();       // alten zum Vergleichen
                SvxFontHeightItem aNew(240 , 100, pFSize->Which());
                aNew.SetHeight( pFSize->GetHeight(), pOldFSize->GetProp(),
                                pOldFSize->GetPropUnit() );
                if( nTmp != aNew.GetHeight() )
                {
                    SetFmtAttr( aNew );
                    bWeiter = 0 != pOldChgSet || bNewParent;
                }
                // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
                // denn es wird bei uns gesetzt!
                else if( pNewChgSet )
                    bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
            }
        }
    }

    if( bWeiter )
        SwFmtColl::Modify( pOld, pNew );
}

BOOL SwTxtFmtColl::IsAtDocNodeSet() const
{
    SwClientIter aIter( *(SwModify*)this );
    const SwNodes& rNds = GetDoc()->GetNodes();
    for( SwClient* pC = aIter.First(TYPE(SwCntntNode)); pC; pC = aIter.Next() )
        if( &((SwCntntNode*)pC)->GetNodes() == &rNds )
            return TRUE;

    return FALSE;
}

BOOL SwTxtFmtColl::SetFmtAttr( const SfxPoolItem& rAttr )
{
    const bool bIsNumRuleItem = rAttr.Which() == RES_PARATR_NUMRULE;
    if ( bIsNumRuleItem )
    {
        TxtFmtCollFunc::RemoveFromNumRule( *this );
    }

    const BOOL bRet = SwFmtColl::SetFmtAttr( rAttr );

    if ( bIsNumRuleItem )
    {
        TxtFmtCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

BOOL SwTxtFmtColl::SetFmtAttr( const SfxItemSet& rSet )
{
    const bool bIsNumRuleItemAffected =
                rSet.GetItemState( RES_PARATR_NUMRULE, FALSE ) == SFX_ITEM_SET;
    if ( bIsNumRuleItemAffected )
    {
        TxtFmtCollFunc::RemoveFromNumRule( *this );
    }

    const BOOL bRet = SwFmtColl::SetFmtAttr( rSet );

    if ( bIsNumRuleItemAffected )
    {
        TxtFmtCollFunc::AddToNumRule( *this );
    }

    return bRet;
}

BOOL SwTxtFmtColl::ResetFmtAttr( USHORT nWhich1, USHORT nWhich2 )
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

    const BOOL bRet = SwFmtColl::ResetFmtAttr( nWhich1, nWhich2 );

    return bRet;
}
// <--

// #i73790#
USHORT SwTxtFmtColl::ResetAllFmtAttr()
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
    // <--

    USHORT nRet = SwFmtColl::ResetAllFmtAttr();

    // #i70748#
    if ( nAssignedOutlineStyleLevel != -1 )
    {
        AssignToListLevelOfOutlineStyle( nAssignedOutlineStyleLevel );
    }
    // <--

    mbStayAssignedToListLevelOfOutlineStyle = bOldState;

    return nRet;
}
// <--

bool SwTxtFmtColl::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( GetItemState( RES_PARATR_NUMRULE ) != SFX_ITEM_SET )
    {
        // no list style applied to paragraph style
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_LR_SPACE, FALSE ) == SFX_ITEM_SET )
    {
        // paragraph style has hard-set indent attributes
        bAreListLevelIndentsApplicable = false;
    }
    else if ( GetItemState( RES_PARATR_NUMRULE, FALSE ) == SFX_ITEM_SET )
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
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, FALSE ) == SFX_ITEM_SET )
            {
                // indent attributes found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, FALSE ) == SFX_ITEM_SET )
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
// <--

//FEATURE::CONDCOLL

SwCollCondition::SwCollCondition( SwTxtFmtColl* pColl, ULONG nMasterCond,
                                ULONG nSubCond )
    : SwClient( pColl ), nCondition( nMasterCond )
{
    aSubCondition.nSubCondition = nSubCond;
}


SwCollCondition::SwCollCondition( SwTxtFmtColl* pColl, ULONG nMasterCond,
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


int SwCollCondition::operator==( const SwCollCondition& rCmp ) const
{
    int nRet = 0;
    if( nCondition == rCmp.nCondition )
    {
        if( USRFLD_EXPRESSION & nCondition )
        {
            // in der SubCondition steht die Expression fuer das UserFeld
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


void SwCollCondition::SetCondition( ULONG nCond, ULONG nSubCond )
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
    USHORT n;

    for( n = 0; n < aCondColls.Count(); ++n )
        if( *( pFnd = aCondColls[ n ]) == rCond )
            break;

    return n < aCondColls.Count() ? pFnd : 0;
}


void SwConditionTxtFmtColl::InsertCondition( const SwCollCondition& rCond )
{
    for( USHORT n = 0; n < aCondColls.Count(); ++n )
        if( *aCondColls[ n ] == rCond )
        {
            aCondColls.DeleteAndDestroy( n );
            break;
        }

    // nicht gefunden -> als einfuegen
    SwCollCondition* pNew = new SwCollCondition( rCond );
    aCondColls.Insert( pNew, aCondColls.Count() );
}


BOOL SwConditionTxtFmtColl::RemoveCondition( const SwCollCondition& rCond )
{
    BOOL bRet = FALSE;
    for( USHORT n = 0; n < aCondColls.Count(); ++n )
        if( *aCondColls[ n ] == rCond )
        {
            aCondColls.DeleteAndDestroy( n );
            bRet = TRUE;
        }

    return bRet;
}

void SwConditionTxtFmtColl::SetConditions( const SwFmtCollConditions& rCndClls )
{
    // Kopiere noch die Bedingungen
    // aber erst die alten loeschen!
    if( aCondColls.Count() )
        aCondColls.DeleteAndDestroy( 0, aCondColls.Count() );
    SwDoc& rDoc = *GetDoc();
    for( USHORT n = 0; n < rCndClls.Count(); ++n )
    {
        SwCollCondition* pFnd = rCndClls[ n ];
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
        aCondColls.Insert( pNew, n );
    }
}
//#outline level, zhaojianwei
void SwTxtFmtColl::SetAttrOutlineLevel( int nLevel)
{
    OSL_ENSURE( 0 <= nLevel && nLevel <= MAXLEVEL ,"SwTxtFmtColl: Level Out Of Range" );
    SetFmtAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                            static_cast<UINT16>(nLevel) ) );
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
    SwClientIter aIter( *this );
    SwTxtFmtColl* pDerivedTxtFmtColl =
                dynamic_cast<SwTxtFmtColl*>(aIter.First( TYPE( SwTxtFmtColl ) ));
    while ( pDerivedTxtFmtColl != 0 )
    {
        if ( !pDerivedTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            if ( pDerivedTxtFmtColl->GetItemState( RES_PARATR_NUMRULE, FALSE ) == SFX_ITEM_DEFAULT )
            {
                SwNumRuleItem aItem(aEmptyStr);
                pDerivedTxtFmtColl->SetFmtAttr( aItem );
            }
            if ( pDerivedTxtFmtColl->GetItemState( RES_PARATR_OUTLINELEVEL, FALSE ) == SFX_ITEM_DEFAULT )
            {
                pDerivedTxtFmtColl->SetAttrOutlineLevel( 0 );
            }
        }

        pDerivedTxtFmtColl = dynamic_cast<SwTxtFmtColl*>(aIter.Next());
    }
    // <--
}

void SwTxtFmtColl::DeleteAssignmentToListLevelOfOutlineStyle()
{
    mbAssignedToOutlineStyle = false;
    ResetFmtAttr(RES_PARATR_OUTLINELEVEL);
}
//<-end,zhaojianwei

//FEATURE::CONDCOLL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
