/*************************************************************************
 *
 *  $RCSfile: fedesc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif

/*************************************************************************
|*
|*  SwFEShell::GetPageDescCnt()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 25. Jan. 93
|*
|*************************************************************************/

USHORT SwFEShell::GetPageDescCnt() const
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
#ifndef PRODUCT
    //Die SS veraendert keinen PageDesc, sondern setzt nur das Attribut.
    //Der Pagedesc muss im Dokument vorhanden sein!
    BOOL bFound = FALSE;
    for ( USHORT nTst = 0; nTst < GetPageDescCnt(); ++nTst )
        if ( &rDesc == &GetPageDesc( nTst ) )
            bFound = TRUE;
    ASSERT( bFound, "ChgCurPageDesc mit ungueltigem Descriptor." );
#endif

    StartAllAction();

    SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    const SwFrm *pFlow;
    USHORT nPageNmOffset = 0;

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
        GetDoc()->Insert( aPaM, aNew );
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

void SwFEShell::ChgPageDesc( USHORT i, const SwPageDesc &rChged )
{
    StartAllAction();
    SET_CURR_SHELL( this );
    GetDoc()->ChgPageDesc( i, rChged );
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

const SwPageDesc& SwFEShell::GetPageDesc( USHORT i ) const
{
    return GetDoc()->GetPageDesc( i );
}

SwPageDesc* SwFEShell::FindPageDescByName( const String& rName,
                                            BOOL bGetFromPool,
                                            USHORT* pPos )
{
    SwPageDesc* pDesc = GetDoc()->FindPageDescByName( rName, pPos );
    if( !pDesc && bGetFromPool )
    {
        USHORT nPoolId = GetDoc()->GetPoolId( rName, GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId &&
            0 != (pDesc = GetDoc()->GetPageDescFromPool( nPoolId ))
            && pPos )
                // werden immer hinten angehaengt
            *pPos = GetDoc()->GetPageDescCnt() - 1 ;
    }
    return pDesc;
}


USHORT SwFEShell::GetCurPageDesc( const BOOL bCalcFrm ) const
{
    const SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
    {
        const SwPageFrm *pPage = pFrm->FindPageFrm();
        if ( pPage )
        {
            SwDoc *pDoc = GetDoc();
            for ( USHORT i = 0; i < GetDoc()->GetPageDescCnt(); ++i )
            {
                if ( pPage->GetPageDesc() == &pDoc->GetPageDesc(i) )
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
            0 != ( pPtFrm = pCNd->GetFrm( &aNulPt, 0, FALSE )) )
            pPtFrm = pPtFrm->FindPageFrm();
        else
            pPtFrm = 0;

        if( PCURCRSR->HasMark() &&
            0 != (pCNd = PCURCRSR->GetCntntNode( FALSE ) ) &&
            0 != ( pMkFrm = pCNd->GetFrm( &aNulPt, 0, FALSE )) )
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



