/*************************************************************************
 *
 *  $RCSfile: newfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-30 20:32:29 $
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

#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VIRTOUTP_HXX
#include <virtoutp.hxx>
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif


PtPtr pX = &Point::nA;
PtPtr pY = &Point::nB;
SzPtr pWidth = &Size::nA;
SzPtr pHeight = &Size::nB;

SwLayVout     *SwRootFrm::pVout = 0;
BOOL           SwRootFrm::bInPaint = FALSE;
BOOL           SwRootFrm::bNoVirDev = FALSE;

SwCache *SwFrm::pCache = 0;

Bitmap* SwNoTxtFrm::pErrorBmp = 0;
Bitmap* SwNoTxtFrm::pReplaceBmp = 0;

#ifndef PRODUCT
USHORT SwFrm::nLastFrmId=0;
#endif


TYPEINIT1(SwFrm,SwClient);      //rtti fuer SwFrm
TYPEINIT1(SwCntntFrm,SwFrm);    //rtti fuer SwCntntFrm


void _FrmInit()
{
    SwRootFrm::pVout = new SwLayVout();
    SwCache *pNew = new SwCache( 100, 100
#ifndef PRODUCT
    , "static SwBorderAttrs::pCache"
#endif
    );
    SwFrm::SetCache( pNew );
}



void _FrmFinit()
{
#ifndef PRODUCT
    // im Chache duerfen nur noch 0-Pointer stehen
    for( USHORT n = SwFrm::GetCachePtr()->Count(); n; )
        if( (*SwFrm::GetCachePtr())[ --n ] )
        {
            SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
            ASSERT( !pObj, "Wer hat sich nicht ausgetragen?")
        }
#endif
    delete SwRootFrm::pVout;
    delete SwFrm::GetCachePtr();
}

/*************************************************************************
|*
|*  RootFrm::Alles was so zur CurrShell gehoert
|*
|*  Ersterstellung      MA 09. Sep. 98
|*  Letzte Aenderung    MA 18. Feb. 99
|*
|*************************************************************************/

typedef CurrShell* CurrShellPtr;
SV_DECL_PTRARR_SORT(SwCurrShells,CurrShellPtr,4,4)
SV_IMPL_PTRARR_SORT(SwCurrShells,CurrShellPtr)

CurrShell::CurrShell( ViewShell *pNew )
{
    ASSERT( pNew, "0-Shell einsetzen?" );
    pRoot = pNew->GetLayout();
    if ( pRoot )
    {
        pPrev = pRoot->pCurrShell;
        pRoot->pCurrShell = pNew;
        pRoot->pCurrShells->Insert( this );
    }
    else
        pPrev = 0;
}

CurrShell::~CurrShell()
{
    if ( pRoot )
    {
        pRoot->pCurrShells->Remove( this );
        if ( pPrev )
            pRoot->pCurrShell = pPrev;
        if ( !pRoot->pCurrShells->Count() && pRoot->pWaitingCurrShell )
        {
            pRoot->pCurrShell = pRoot->pWaitingCurrShell;
            pRoot->pWaitingCurrShell = 0;
        }
    }
}

void SetShell( ViewShell *pSh )
{
    SwRootFrm *pRoot = pSh->GetLayout();
    if ( !pRoot->pCurrShells->Count() )
        pRoot->pCurrShell = pSh;
    else
        pRoot->pWaitingCurrShell = pSh;
}

void SwRootFrm::DeRegisterShell( ViewShell *pSh )
{
    //Wenn moeglich irgendeine Shell aktivieren
    if ( pCurrShell == pSh )
        pCurrShell = pSh->GetNext() != pSh ? (ViewShell*)pSh->GetNext() : 0;

    //Das hat sich eruebrigt
    if ( pWaitingCurrShell == pSh )
        pWaitingCurrShell = 0;

    //Referenzen entfernen.
    for ( USHORT i = 0; i < pCurrShells->Count(); ++i )
    {
        CurrShell *pC = (*pCurrShells)[i];
        if (pC->pPrev == pSh)
            pC->pPrev = 0;
    }
}

void InitCurrShells( SwRootFrm *pRoot )
{
    pRoot->pCurrShells = new SwCurrShells;
}


/*************************************************************************
|*
|*  SwRootFrm::SwRootFrm()
|*
|*  Beschreibung:
|*      Der RootFrm laesst sich grundsaetzlich vom Dokument ein eigenes
|*      FrmFmt geben. Dieses loescht er dann selbst im DTor.
|*      Das eigene FrmFmt wird vom uebergebenen Format abgeleitet.
|*  Ersterstellung      SS 05-Apr-1991
|*  Letzte Aenderung    MA 12. Dec. 94
|*
|*************************************************************************/


SwRootFrm::SwRootFrm( SwFrmFmt *pFmt, ViewShell * pSh ) :
    SwLayoutFrm( pFmt->GetDoc()->MakeFrmFmt(
        XubString( "Root", RTL_TEXTENCODING_MS_1252 ), pFmt ) ),
    pTurbo( 0 ),
    pLastPage( 0 ),
    pCurrShell( pSh ),
    pWaitingCurrShell( 0 ),
    pDestroy( 0 ),
    nPhyPageNums( 0 ),
    pDrawPage( 0 ),
    nBrowseWidth( MM50*4 )  //2cm Minimum
{
    nType = FRM_ROOT;
    bIdleFormat = bTurboAllowed = bAssertFlyPages = bIsNewLayout = TRUE;
    bCheckSuperfluous = bBrowseWidthValid = FALSE;

    InitCurrShells( this );

    SwDoc *pDoc = pFmt->GetDoc();
    const BOOL bOldIdle = pDoc->IsIdleTimerActive();
    pDoc->StopIdleTimer();
    pDoc->SetRootFrm( this );       //Fuer das Erzeugen der Flys durch MakeFrms()
    bCallbackActionEnabled = FALSE; //vor Verlassen auf TRUE setzen!

#ifdef QUER
    //StarWriter /QUER ? bitteschoen:
    SetFixSize( pHeight );
#else
    SetFixSize( pWidth );
#endif

    SdrModel *pMd = pDoc->GetDrawModel();
    if ( pMd )
    {
        pDrawPage = pMd->GetPage( 0 );
        pDrawPage->SetSize( Frm().SSize() );
    }

    //Initialisierung des Layouts: Seiten erzeugen. Inhalt mit cntnt verbinden
    //usw.
    //Zuerst einiges initialiseren und den ersten Node besorgen (der wird
    //fuer den PageDesc benoetigt).

    SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
    SwCntntNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, TRUE, FALSE );
    SwTableNode *pTblNd= pNode->FindTableNode();

    //PageDesc besorgen (entweder vom FrmFmt des ersten Node oder den
    //initialen.)
    SwPageDesc *pDesc = 0;
    USHORT nPgNum = 1;

    if ( pTblNd )
    {
        const SwFmtPageDesc &rDesc = pTblNd->GetTable().GetFrmFmt()->GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        //#19104# Seitennummeroffset beruecksictigen!!
        bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
    }
    else if ( pNode )
    {
        const SwFmtPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        //#19104# Seitennummeroffset beruecksictigen!!
        bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
    }
    if ( !pDesc )
        pDesc = (SwPageDesc*)&pDoc->GetPageDesc( 0 );
    if( !nPgNum )
        nPgNum = 1;

    //Eine Seite erzeugen und in das Layout stellen
    SwPageFrm *pPage = ::InsertNewPage( *pDesc, this, 0 != nPgNum % 2 , FALSE, 0 );

    //Erstes Blatt im Bodytext-Bereich suchen.
    SwLayoutFrm *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = (SwLayoutFrm*)pLay->Lower();

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), TRUE );
    //Noch nicht ersetzte Master aus der Liste entfernen.
    RemoveMasterObjs( pDrawPage );
    if( pDoc->IsGlobalDoc() )
        pDoc->UpdateRefFlds( NULL );
    if ( bOldIdle )
        pDoc->StartIdleTimer();
    bCallbackActionEnabled = TRUE;
}

/*************************************************************************
|*
|*  SwRootFrm::~SwRootFrm()
|*
|*  Ersterstellung      SS 05-Apr-1991
|*  Letzte Aenderung    MA 12. Dec. 94
|*
|*************************************************************************/



SwRootFrm::~SwRootFrm()
{
    bTurboAllowed = FALSE;
    pTurbo = 0;
    if(pBlink)
        pBlink->FrmDelete( this );
    ((SwFrmFmt*)pRegisteredIn)->GetDoc()->DelFrmFmt( (SwFrmFmt*)pRegisteredIn );
    delete pDestroy;

    //Referenzen entfernen.
    for ( USHORT i = 0; i < pCurrShells->Count(); ++i )
        (*pCurrShells)[i]->pRoot = 0;

    delete pCurrShells;
}

/*************************************************************************
|*
|*  SwRootFrm::SetFixSize()
|*
|*  Ersterstellung      MA 23. Jul. 92
|*  Letzte Aenderung    MA 11. Mar. 93
|*
|*************************************************************************/



void SwRootFrm::SetFixSize( SzPtr pNew )
{
    if ( pNew == pHeight )
    {
        GetFmt()->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
        bVarHeight = bFixWidth = FALSE;
        bFixHeight = TRUE;
    }
    else
    {
        GetFmt()->SetAttr( SwFmtFillOrder( ATT_TOP_DOWN ) );
        bVarHeight = bFixWidth = TRUE;
        bFixHeight = FALSE;
    }
}

/*************************************************************************
|*
|*  SwRootFrm::RemoveMasterObjs()
|*
|*  Ersterstellung      MA 19.10.95
|*  Letzte Aenderung    MA 19.10.95
|*
|*************************************************************************/


void SwRootFrm::RemoveMasterObjs( SdrPage *pPg )
{
    //Alle Masterobjekte aus der Page entfernen. Nicht loeschen!!
    for( ULONG i = pPg ? pPg->GetObjCount() : 0; i; )
    {
        SdrObject* pObj = pPg->GetObj( --i );
        if( pObj->ISA(SwFlyDrawObj ) )
            pPg->RemoveObject( i );
    }
}




