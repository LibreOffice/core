/*************************************************************************
 *
 *  $RCSfile: swatrset.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#include <hintids.hxx>

#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#define ITEMID_LINE             SID_ATTR_LINE_STYLE
#ifndef _SVX_BOLNITEM_HXX //autogen
#include <svx/bolnitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>       // fuer SetModifyAtAttr
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>      // fuer SetModifyAtAttr
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif


SwAttrPool::SwAttrPool( SwDoc* pD )
    : SfxItemPool( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "SWG" )),
                    POOLATTR_BEGIN, POOLATTR_END-1,
                    aSlotTab, aAttrTab ),
    pDoc( pD )
{
    SetVersionMap( 1, 1, 60, pVersionMap1 );
    SetVersionMap( 2, 1, 75, pVersionMap2 );
    SetVersionMap( 3, 1, 86, pVersionMap3 );
}


SwAttrSet::SwAttrSet( SwAttrPool& rPool, USHORT nWh1, USHORT nWh2 )
    : SfxItemSet( rPool, nWh1, nWh2 ), pOldSet( 0 ), pNewSet( 0 )
{
}


SwAttrSet::SwAttrSet( SwAttrPool& rPool, const USHORT* nWhichPairTable )
    : SfxItemSet( rPool, nWhichPairTable ), pOldSet( 0 ), pNewSet( 0 )
{
}


SwAttrSet::SwAttrSet( const SwAttrSet& rSet )
    : SfxItemSet( rSet ), pOldSet( 0 ), pNewSet( 0 )
{
}


int SwAttrSet::Put_BC( const SfxPoolItem& rAttr,
                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    int nRet = 0 != SfxItemSet::Put( rAttr );
    pOldSet = pNewSet = 0;
    return nRet;
}


int SwAttrSet::Put_BC( const SfxItemSet& rSet,
                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    int nRet = 0 != SfxItemSet::Put( rSet );
    pOldSet = pNewSet = 0;
    return nRet;
}



USHORT SwAttrSet::ClearItem_BC( USHORT nWhich,
                        SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    USHORT nRet = SfxItemSet::ClearItem( nWhich );
    pOldSet = pNewSet = 0;
    return nRet;
}


USHORT SwAttrSet::ClearItem_BC( USHORT nWhich1, USHORT nWhich2,
                        SwAttrSet* pOld, SwAttrSet* pNew )
{
    ASSERT( nWhich1 <= nWhich2, "kein gueltiger Bereich" );
    pNewSet = pNew;
    pOldSet = pOld;
    USHORT nRet = 0;
    for( ; nWhich1 <= nWhich2; ++nWhich1 )
        nRet += SfxItemSet::ClearItem( nWhich1 );
    pOldSet = pNewSet = 0;
    return nRet;
}



int SwAttrSet::Intersect_BC( const SfxItemSet& rSet,
                            SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    SfxItemSet::Intersect( rSet );
    pOldSet = pNewSet = 0;
    return pNew ? pNew->Count() : ( pOld ? pOld->Count() : 0 );
}


int SwAttrSet::Differentiate_BC( const SfxItemSet& rSet,
                            SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    SfxItemSet::Differentiate( rSet );
    pOldSet = pNewSet = 0;
    return pNew ? pNew->Count() : ( pOld ? pOld->Count() : 0 );
}


int SwAttrSet::MergeValues_BC( const SfxItemSet& rSet,
                            SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    SfxItemSet::MergeValues( rSet );
    pOldSet = pNewSet = 0;
    return pNew ? pNew->Count() : ( pOld ? pOld->Count() : 0 );
}


    // Notification-Callback
void  SwAttrSet::Changed( const SfxPoolItem& rOld,
                                const SfxPoolItem& rNew )
{
    if( pOldSet )
        pOldSet->PutChgd( rOld );

    if( pNewSet )
        pNewSet->PutChgd( rNew );
}


// ----------------------------------------------------------------
// Sonderbehandlung fuer einige Attribute
// Setze den Modify-Pointer (alten pDefinedIn) bei folgenden Attributen:
//  - SwFmtDropCaps
//  - SwFmtPageDesc
// (Wird beim Einfuegen in Formate/Nodes gerufen)
// ----------------------------------------------------------------
void SwAttrSet::SetModifyAtAttr( const SwModify* pModify )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == GetItemState( RES_PAGEDESC, FALSE, &pItem ) &&
        ((SwFmtPageDesc*)pItem)->GetDefinedIn() != pModify  )
    {
        ((SwFmtPageDesc*)pItem)->ChgDefinedIn( pModify );
    }

    if( SFX_ITEM_SET == GetItemState( RES_PARATR_NUMRULE, FALSE, &pItem ) &&
        ((SwNumRuleItem*)pItem)->GetDefinedIn() != pModify  )
    {
        ((SwNumRuleItem*)pItem)->ChgDefinedIn( pModify );
    }

    if( SFX_ITEM_SET == GetItemState( RES_PARATR_DROP, FALSE, &pItem ) &&
        ((SwFmtDrop*)pItem)->GetDefinedIn() != pModify )
    {
        // CharFormat gesetzt und dann noch in unterschiedlichen
        // Attribut Pools, dann muss das CharFormat kopiert werden!
        SwCharFmt* pCharFmt;
        if( 0 != ( pCharFmt = ((SwFmtDrop*)pItem)->GetCharFmt() )
            && GetPool() != pCharFmt->GetAttrSet().GetPool() )
        {
            pCharFmt = GetDoc()->CopyCharFmt( *pCharFmt );
            ((SwFmtDrop*)pItem)->SetCharFmt( pCharFmt );
        }
        ((SwFmtDrop*)pItem)->ChgDefinedIn( pModify );
    }

    if( SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMULA, FALSE, &pItem ) &&
        ((SwTblBoxFormula*)pItem)->GetDefinedIn() != pModify )
    {
        ((SwTblBoxFormula*)pItem)->ChgDefinedIn( pModify );
    }
}


void SwAttrSet::CopyToModify( SwModify& rMod ) const
{
    // kopiere die Attribute ggfs. ueber Dokumentgrenzen
    SwCntntNode* pCNd = PTR_CAST( SwCntntNode, &rMod );
    SwFmt* pFmt = PTR_CAST( SwFmt, &rMod );

    if( pCNd || pFmt )
    {
        if( Count() )
        {
            const SfxPoolItem* pItem;
            const SwDoc *pSrcDoc = GetDoc();
            SwDoc *pDstDoc = pCNd ? pCNd->GetDoc() : pFmt->GetDoc();

            // muss die NumRule kopiert werden?
            if( pSrcDoc != pDstDoc && SFX_ITEM_SET == GetItemState(
                                    RES_PARATR_NUMRULE, FALSE, &pItem ) )
            {
                const String& rNm = ((SwNumRuleItem*)pItem)->GetValue();
                if( rNm.Len() )
                {
                    SwNumRule* pDestRule = pDstDoc->FindNumRulePtr( rNm );
                    if( pDestRule )
                        pDestRule->SetInvalidRule( TRUE );
                    else
                        pDstDoc->MakeNumRule( rNm,
                                            pSrcDoc->FindNumRulePtr( rNm ) );
                }
            }

            // JP 04.02.99: Task #61467# Seitenvorlagenwechsel mit kopieren
            //              Gegenueber dem alten Verhalten, sie zu entfernen
            const SwPageDesc* pPgDesc;
            if( pSrcDoc != pDstDoc && SFX_ITEM_SET == GetItemState(
                                            RES_PAGEDESC, FALSE, &pItem ) &&
                0 != ( pPgDesc = ((SwFmtPageDesc*)pItem)->GetPageDesc()) )
            {
                SfxItemSet aTmpSet( *this );

                // JP 09.02.99: und jetzt doch wieder nur entfernen
                aTmpSet.ClearItem( RES_PAGEDESC );

/*************************************************************************
                SwPageDesc* pDstPgDesc = pDstDoc->FindPageDescByName(
                                                    pPgDesc->GetName() );
                if( !pDstPgDesc )
                {
                    // dann kopieren, ansonsten den benutzen
                    pDstPgDesc = &pDstDoc->_GetPageDesc( pDstDoc->MakePageDesc(
                                                    pPgDesc->GetName() ));
                    pDstDoc->CopyPageDesc( *pPgDesc, *pDstPgDesc );
                }
                SwFmtPageDesc aDesc( pDstPgDesc );
                aDesc.SetNumOffset( ((SwFmtPageDesc*)pItem)->GetNumOffset() );
                aTmpSet.Put( aDesc );
************************************************************************/

                if( pCNd )
                    pCNd->SetAttr( aTmpSet );
                else
                    pFmt->SetAttr( aTmpSet );
            }
            else if( pCNd )
                pCNd->SetAttr( *this );
            else
                pFmt->SetAttr( *this );
        }
    }
#ifndef PRODUCT
    else
        ASSERT( !this, "weder Format noch ContentNode - keine Attribute kopiert");
#endif
}

// check if ID is InRange of AttrSet-Ids
BOOL IsInRange( const USHORT* pRange, const USHORT nId )
{
    while( *pRange )
    {
        if( *pRange <= nId && nId <= *(pRange+1) )
            return TRUE;
        pRange += 2;
    }
    return FALSE;
}




