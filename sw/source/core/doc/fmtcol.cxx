/*************************************************************************
 *
 *  $RCSfile: fmtcol.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-09 19:06:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>          // fuer GetAttrPool
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif


TYPEINIT1( SwTxtFmtColl, SwFmtColl );
TYPEINIT1( SwGrfFmtColl, SwFmtColl );
TYPEINIT1( SwConditionTxtFmtColl, SwTxtFmtColl );
TYPEINIT1( SwCollCondition, SwClient );

SV_IMPL_PTRARR( SwFmtCollConditions, SwCollConditionPtr );


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

    int bNewParent = FALSE;
    SvxULSpaceItem *pNewULSpace = 0, *pOldULSpace = 0;
    SvxLRSpaceItem *pNewLRSpace = 0, *pOldLRSpace = 0;
    SvxFontHeightItem* aFontSizeArr[3] = {0,0,0};

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
    }

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
                SetAttr( aNew );
                bWeiter = 0 != pOldChgSet || bNewParent;
            }
            // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
            // denn es wird bei uns gesetzt!
            else if( pNewChgSet )
                bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
        }

#ifndef NUM_RELSPACE
        if( !bChg && pOldLRSpace == pNewLRSpace &&
            // pNewChgSet->GetTheChgdSet() == &aSet &&
            GetOutlineLevel() < MAXLEVEL )
        {
            // dann muss der Wert in die OutlineRule uebertragen werden
            GetDoc()->SetOutlineLSpace( GetOutlineLevel(),
                                        pOldLRSpace->GetTxtFirstLineOfst(),
                                        pOldLRSpace->GetTxtLeft() );
        }
#endif
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
            SetAttr( aNew );
            bWeiter = 0 != pOldChgSet || bNewParent;
        }
        // bei uns absolut gesetzt -> nicht weiter propagieren, es sei
        // denn es wird bei uns gesetzt!
        else if( pNewChgSet )
            bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
    }


    for( int nC = 0, nArrLen = sizeof(aFontSizeArr) / sizeof( aFontSizeArr[0]);
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
                SvxFontHeightItem aNew( pFSize->Which() );
                aNew.SetHeight( pFSize->GetHeight(), pOldFSize->GetProp(),
                                pOldFSize->GetPropUnit() );
                if( nTmp != aNew.GetHeight() )
                {
                    SetAttr( aNew );
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

#ifdef USED
    // zum "abfischen" von Aenderungen
void SwConditionTxtFmtColl::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    SwTxtFmtColl::Modify( pOld, pNew );
}
#endif


const SwCollCondition* SwConditionTxtFmtColl::HasCondition(
                        const SwCollCondition& rCond ) const
{
    const SwCollCondition* pFnd = 0;
    for( USHORT n = 0; n < aCondColls.Count(); ++n )
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

//FEATURE::CONDCOLL




