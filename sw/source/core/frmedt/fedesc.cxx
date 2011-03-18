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

#include <tools/ref.hxx>

#include <hintids.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <tabfrm.hxx>
#include <edimp.hxx>
#include <SwStyleNameMapper.hxx>

/*************************************************************************
|*
|*  SwFEShell::GetPageDescCnt()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 25. Jan. 93
|*
|*************************************************************************/

sal_uInt16 SwFEShell::GetPageDescCnt() const
{
    return GetDoc()->GetPageDescCnt();
}

/*************************************************************************
|*
|*  SwFEShell::ChgCurPageDesc()
|*
|*  Ersterstellung      ST ??
|*  Letzte Aenderung    MA 01. Aug. 94
|*
|*************************************************************************/

void SwFEShell::ChgCurPageDesc( const SwPageDesc& rDesc )
{
#ifdef DBG_UTIL
    //Die SS veraendert keinen PageDesc, sondern setzt nur das Attribut.
    //Der Pagedesc muss im Dokument vorhanden sein!
    sal_Bool bFound = sal_False;
    for ( sal_uInt16 nTst = 0; nTst < GetPageDescCnt(); ++nTst )
        if ( &rDesc == &GetPageDesc( nTst ) )
            bFound = sal_True;
    ASSERT( bFound, "ChgCurPageDesc mit ungueltigem Descriptor." );
#endif

    StartAllAction();

    SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    const SwFrm *pFlow = 0;
    sal_uInt16 nPageNmOffset = 0;

    ASSERT( !GetCrsr()->HasMark(), "ChgCurPageDesc nur ohne Selektion!");

    SET_CURR_SHELL( this );
    while ( pPage )
    {
        pFlow = pPage->FindFirstBodyCntnt();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const SwFmtPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            if( rPgDesc.GetPageDesc() )
            {
                // wir haben ihn den Schlingel
                nPageNmOffset = rPgDesc.GetNumOffset();
                break;
            }
        }
        pPage = (SwPageFrm*) pPage->GetPrev();
    }
    if ( !pPage )
    {
        pPage = (SwPageFrm*) (GetLayout()->Lower());
        pFlow = pPage->FindFirstBodyCntnt();
        if ( !pFlow )
        {
            pPage   = (SwPageFrm*)pPage->GetNext();
            pFlow = pPage->FindFirstBodyCntnt();
            ASSERT( pFlow, "Dokuemnt ohne Inhalt?!?" );
        }
    }

    // Seitennummer mitnehmen
    SwFmtPageDesc aNew( &rDesc );
    aNew.SetNumOffset( nPageNmOffset );

    if ( pFlow->IsInTab() )
        GetDoc()->SetAttr( aNew, *(SwFmt*)pFlow->FindTabFrm()->GetFmt() );
    else
    {
        SwPaM aPaM( *((SwCntntFrm*)pFlow)->GetNode() );
        GetDoc()->InsertPoolItem( aPaM, aNew, 0 );
    }
    EndAllActionAndCall();
}

/*************************************************************************
|*
|*  SwFEShell::ChgPageDesc()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 24. Jan. 95
|*
|*************************************************************************/

void SwFEShell::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    StartAllAction();
    SET_CURR_SHELL( this );
    //Fix i64842: because Undo has a very special way to handle header/footer content
    // we have to copy the page descriptor before calling ChgPageDesc.
    SwPageDesc aDesc( rChged );
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
        GetDoc()->CopyPageDesc(rChged, aDesc);
    }
    GetDoc()->ChgPageDesc( i, aDesc );
    EndAllActionAndCall();
}

/*************************************************************************
|*
|*  SwFEShell::GetPageDesc(), GetCurPageDesc()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 23. Apr. 93
|
|*************************************************************************/

const SwPageDesc& SwFEShell::GetPageDesc( sal_uInt16 i ) const
{
    return const_cast<const SwDoc *>(GetDoc())->GetPageDesc( i );
}

SwPageDesc* SwFEShell::FindPageDescByName( const String& rName,
                                            sal_Bool bGetFromPool,
                                            sal_uInt16* pPos )
{
    SwPageDesc* pDesc = GetDoc()->FindPageDescByName( rName, pPos );
    if( !pDesc && bGetFromPool )
    {
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId &&
            0 != (pDesc = GetDoc()->GetPageDescFromPool( nPoolId ))
            && pPos )
                // werden immer hinten angehaengt
            *pPos = GetDoc()->GetPageDescCnt() - 1 ;
    }
    return pDesc;
}

sal_uInt16 SwFEShell::GetMousePageDesc( const Point &rPt ) const
{
    if( GetLayout() )
    {
        const SwPageFrm* pPage =
            static_cast<const SwPageFrm*>( GetLayout()->Lower() );
        if( pPage )
        {
            while( pPage->GetNext() && rPt.Y() > pPage->Frm().Bottom() )
                pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );
            SwDoc *pMyDoc = GetDoc();
            for ( sal_uInt16 i = 0; i < GetDoc()->GetPageDescCnt(); ++i )
            {
                if ( pPage->GetPageDesc() == &const_cast<const SwDoc *>(pMyDoc)
                     ->GetPageDesc(i) )
                    return i;
            }
        }
    }
    return 0;
}

sal_uInt16 SwFEShell::GetCurPageDesc( const sal_Bool bCalcFrm ) const
{
    const SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
    {
        const SwPageFrm *pPage = pFrm->FindPageFrm();
        if ( pPage )
        {
            SwDoc *pMyDoc = GetDoc();
            for ( sal_uInt16 i = 0; i < GetDoc()->GetPageDescCnt(); ++i )
            {
                if ( pPage->GetPageDesc() == &const_cast<const SwDoc *>(pMyDoc)
                     ->GetPageDesc(i) )
                    return i;
            }
        }
    }
    return 0;
}

// if inside all selection only one PageDesc, return this.
// Otherwise return 0 pointer
const SwPageDesc* SwFEShell::GetSelectedPageDescs() const
{
    const SwCntntNode* pCNd;
    const SwFrm* pMkFrm, *pPtFrm;
    const SwPageDesc* pFnd, *pRetDesc = (SwPageDesc*)0xffffffff;
    const Point aNulPt;

    FOREACHPAM_START(this)

        if( 0 != (pCNd = PCURCRSR->GetCntntNode() ) &&
            0 != ( pPtFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, 0, sal_False )) )
            pPtFrm = pPtFrm->FindPageFrm();
        else
            pPtFrm = 0;

        if( PCURCRSR->HasMark() &&
            0 != (pCNd = PCURCRSR->GetCntntNode( sal_False ) ) &&
            0 != ( pMkFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, 0, sal_False )) )
            pMkFrm = pMkFrm->FindPageFrm();
        else
            pMkFrm = pPtFrm;


        if( !pMkFrm || !pPtFrm )
            pFnd = 0;
        else if( pMkFrm == pPtFrm )
            pFnd = ((SwPageFrm*)pMkFrm)->GetPageDesc();
        else
        {
            // swap pointer if PtFrm before MkFrm
            if( ((SwPageFrm*)pMkFrm)->GetPhyPageNum() >
                ((SwPageFrm*)pPtFrm)->GetPhyPageNum() )
            {
                const SwFrm* pTmp = pMkFrm; pMkFrm = pPtFrm; pPtFrm = pTmp;
            }

            // now check from MkFrm to PtFrm for equal PageDescs
            pFnd = ((SwPageFrm*)pMkFrm)->GetPageDesc();
            while( pFnd && pMkFrm != pPtFrm )
            {
                pMkFrm = pMkFrm->GetNext();
                if( !pMkFrm || pFnd != ((SwPageFrm*)pMkFrm)->GetPageDesc() )
                    pFnd = 0;
            }
        }

        if( (SwPageDesc*)0xffffffff == pRetDesc )
            pRetDesc = pFnd;
        else if( pFnd != pRetDesc )
        {
            pRetDesc = 0;
            break;
        }

    FOREACHPAM_END()

    return pRetDesc;
}



