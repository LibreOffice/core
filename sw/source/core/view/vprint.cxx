/*************************************************************************
 *
 *  $RCSfile: vprint.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:25:44 $
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

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif


#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
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
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>      // Imp->SetFirstVisPageInvalid()
#endif
#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>      // PostItFld /-Type
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>       // _SetGetExpFld
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>       // SwModify/SwClientIter
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>     // SwPrtOptions
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>      // fuer RES_POOLPAGE_JAKET
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // Ansteuern der Statusleiste
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>      //      -- " --
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // MinPrtLine
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>       // SwShellCrsr
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>      // SwFmtPageDesc
#endif

#define JOBSET_ERR_DEFAULT          0
#define JOBSET_ERR_ERROR            1
#define JOBSET_ERR_ISSTARTET        2

//--------------------------------------------------------------------
//Klasse zum Puffern von Paints
class SwQueuedPaint
{
public:
    SwQueuedPaint *pNext;
    ViewShell      *pSh;
    SwRect          aRect;

    SwQueuedPaint( ViewShell *pNew, const SwRect &rRect ) :
        pNext( 0 ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::pQueue = 0;

//Klasse zum Speichern einiger Druckereinstellungen
class SwPrtOptSave
{
    Printer *pPrt;
    Size aSize;
    Paper ePaper;
    Orientation eOrientation;
    USHORT nPaperBin;
public:
    SwPrtOptSave( Printer *pPrinter );
    ~SwPrtOptSave();
};




void SwPaintQueue::Add( ViewShell *pNew, const SwRect &rNew )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        while ( pPt->pSh != pNew && pPt->pNext )
            pPt = pPt->pNext;
        if ( pPt->pSh == pNew )
        {
            pPt->aRect.Union( rNew );
            return;
        }
    }
    SwQueuedPaint *pNQ = new SwQueuedPaint( pNew, rNew );
    if ( pPt )
        pPt->pNext = pNQ;
    else
        pQueue = pNQ;
}



void SwPaintQueue::Repaint()
{
    if ( !SwRootFrm::IsInPaint() && pQueue )
    {
        SwQueuedPaint *pPt = pQueue;
        do
        {   ViewShell *pSh = pPt->pSh;
            SET_CURR_SHELL( pSh );
            if ( pSh->IsPreView() )
            {
                if ( pSh->GetWin() )
                {
                    //Fuer PreView aussenherum, weil im PaintHdl (UI) die
                    //Zeilen/Spalten bekannt sind.
                    pSh->GetWin()->Invalidate();
                    pSh->GetWin()->Update();
                }
            }
            else
                pSh->Paint( pPt->aRect.SVRect() );
            pPt = pPt->pNext;
        } while ( pPt );

        do
        {   pPt = pQueue;
            pQueue = pQueue->pNext;
            delete pPt;
        } while ( pQueue );
    }
}



void SwPaintQueue::Remove( ViewShell *pSh )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        SwQueuedPaint *pPrev = 0;
        while ( pPt && pPt->pSh != pSh )
        {
            pPrev = pPt;
            pPt = pPt->pNext;
        }
        if ( pPt )
        {
            if ( pPrev )
                pPrev->pNext = pPt->pNext;
            else if ( pPt == pQueue )
                pQueue = 0;
            delete pPt;
        }
    }
}


const XubString& SwPrtOptions::MakeNextJobName()
{
    static char __READONLY_DATA sDelim[] = " - ";
    USHORT nDelim = sJobName.SearchAscii( sDelim );
    if( STRING_NOTFOUND == nDelim )
        sJobName.AppendAscii(sDelim);
    else
        sJobName.Erase( nDelim + 3/*Zeichen!*/ );

    return sJobName += XubString::CreateFromInt32( ++nJobNo );
}

/******************************************************************************
 *  Methode     :   void SetSwVisArea( ViewShell *pSh, Point aPrtOffset, ...
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 16:27
 *  Aenderung   :
 ******************************************************************************/

void SetSwVisArea( ViewShell *pSh, const SwRect &rRect )
{
    ASSERT( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->aVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    aPt += pSh->aPrtOffst;
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

    MapMode aMapMode( pSh->GetPrt()->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pSh->GetPrt()->SetMapMode( aMapMode );
}

/******************************************************************************
 *  Methode     :   struct _PostItFld : public _SetGetExpFld
 *  Beschreibung:   Update an das PostItFeld
 *  Erstellt    :   OK 07.11.94 10:18
 *  Aenderung   :
 ******************************************************************************/
struct _PostItFld : public _SetGetExpFld
{
    _PostItFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,
                    const SwIndex* pIdx = 0 )
        : _SetGetExpFld( rNdIdx, pFld, pIdx ) {}

    USHORT GetPageNo( MultiSelection &rMulti, BOOL bRgt, BOOL bLft,
                        USHORT& rVirtPgNo, USHORT& rLineNo );
    SwPostItField* GetPostIt() const
        { return (SwPostItField*) GetFld()->GetFld().GetFld(); }
};



USHORT _PostItFld::GetPageNo( MultiSelection &rMulti, BOOL bRgt, BOOL bLft,
                                USHORT& rVirtPgNo, USHORT& rLineNo )
{
    //Problem: Wenn ein PostItFld in einem Node steht, der von mehr als
    //einer Layout-Instanz repraesentiert wird, steht die Frage im Raum,
    //ob das PostIt nur ein- oder n-mal gedruck werden soll.
    //Wahrscheinlich nur einmal, als Seitennummer soll hier keine Zufaellige
    //sondern die des ersten Auftretens des PostIts innerhalb des selektierten
    //Bereichs ermittelt werden.
    rVirtPgNo = 0;
    USHORT nPos = GetCntnt();
    SwClientIter aIter( (SwModify &)GetFld()->GetTxtNode() );
    for( SwTxtFrm* pFrm = (SwTxtFrm*)aIter.First( TYPE( SwFrm ));
            pFrm;  pFrm = (SwTxtFrm*)aIter.Next() )
    {
        if( pFrm->GetOfst() > nPos ||
            pFrm->HasFollow() && pFrm->GetFollow()->GetOfst() <= nPos )
            continue;
        USHORT nPgNo = pFrm->GetPhyPageNum();
        BOOL bRight = pFrm->OnRightPage();
        if( rMulti.IsSelected( nPgNo ) &&
            ( (bRight && bRgt) || (!bRight && bLft) ) )
        {
            rLineNo = (USHORT)(pFrm->GetLineCount( nPos ) +
                      pFrm->GetAllLines() - pFrm->GetThisLines());
            rVirtPgNo = pFrm->GetVirtPageNum();
            return nPgNo;
        }
    }
    return 0;
}

/******************************************************************************
 *  Methode     :   void lcl_GetPostIts( SwDoc* pDoc, _SetGetExpFlds& ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:20
 *  Aenderung   :
 ******************************************************************************/


void lcl_GetPostIts( SwDoc* pDoc, _SetGetExpFlds& rSrtLst )
{
    SwFieldType* pFldType = pDoc->GetSysFldType( RES_POSTITFLD );
    ASSERT( pFldType, "kein PostItType ? ");

    if( pFldType->GetDepends() )
    {
        // Modify-Object gefunden, trage alle Felder ins Array ein
        SwClientIter aIter( *pFldType );
        SwClient* pLast;
        const SwTxtFld* pTxtFld;

        for( pLast = aIter.First( TYPE(SwFmtFld)); pLast; pLast = aIter.Next() )
            if( 0 != ( pTxtFld = ((SwFmtFld*)pLast)->GetTxtFld() ) &&
                pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                SwNodeIndex aIdx( pTxtFld->GetTxtNode() );
                _PostItFld* pNew = new _PostItFld( aIdx, pTxtFld );
                rSrtLst.Insert( pNew );
            }
    }
}

/******************************************************************************
 *  Methode     :   void lcl_FormatPostIt( SwDoc* pDoc, SwPaM& aPam, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:20
 *  Aenderung   :
 ******************************************************************************/


void lcl_FormatPostIt( SwDoc* pDoc, SwPaM& aPam, SwPostItField* pField,
                           USHORT nPageNo, USHORT nLineNo )
{
    static char __READONLY_DATA sTmp[] = " : ";

    ASSERT( ViewShell::GetShellRes(), "missing ShellRes" );

    String aStr(    ViewShell::GetShellRes()->aPostItPage   );
    aStr.AppendAscii(sTmp);

    aStr += XubString::CreateFromInt32( nPageNo );
    aStr += ' ';
    if( nLineNo )
    {
        aStr += ViewShell::GetShellRes()->aPostItLine;
        aStr.AppendAscii(sTmp);
        aStr += XubString::CreateFromInt32( nLineNo );
        aStr += ' ';
    }
    aStr += ViewShell::GetShellRes()->aPostItAuthor;
    aStr.AppendAscii(sTmp);
    aStr += pField->GetPar1();
    aStr += ' ';
    aStr += GetAppLocaleData().getDate( pField->GetDate() );
    pDoc->Insert( aPam, aStr );

    pDoc->SplitNode( *aPam.GetPoint() );
    aStr = pField->GetPar2();
#ifdef MAC
    // Fuer den MAC alle CR durch LF ersetzen
    for( USHORT n = 0; n < aStr.Len(); ++n )
        if( aStr[n] == '\r' )
            aStr[n] = '\n';
#endif
#if defined( WIN ) || defined( WNT ) || defined( PM2 )
    // Bei Windows und Co alle CR rausschmeissen
    aStr.EraseAllChars( '\r' );
#endif
    pDoc->Insert( aPam, aStr );
    pDoc->SplitNode( *aPam.GetPoint() );
    pDoc->SplitNode( *aPam.GetPoint() );
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostIts( ViewShell* pPrtShell )
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:21
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


void lcl_PrintPostIts( ViewShell* pPrtShell, const XubString& rJobName,
                        BOOL& rStartJob, int& rJobStartError, BOOL bReverse)
{
    // Formatieren und Ausdrucken
    pPrtShell->CalcLayout();

    SfxPrinter* pPrn = pPrtShell->GetPrt();

    //Das Druckdokument ist ein default Dokument, mithin arbeitet es auf der
    //StandardSeite.
    SwFrm *pPage = pPrtShell->GetLayout()->Lower();

    SwPrtOptSave aPrtSave( pPrn );

    pPrn->SetOrientation( ORIENTATION_PORTRAIT );
    pPrn->SetPaperBin( pPage->GetAttrSet()->GetPaperBin().GetValue() );

    if( !rStartJob &&  JOBSET_ERR_DEFAULT == rJobStartError &&
        rJobName.Len() )
    {
        if( !pPrn->IsJobActive() )
        {
            rStartJob = pPrn->StartJob( rJobName );
            if( !rStartJob )
            {
                rJobStartError = JOBSET_ERR_ERROR;
                return;
            }
        }
        pPrtShell->InitPrt( pPrn );
        rJobStartError = JOBSET_ERR_ISSTARTET;
    }

    // Wir koennen auch rueckwaerts:
    if ( bReverse )
        pPage = pPrtShell->GetLayout()->GetLastPage();

    while( pPage )
    {
        //Mag der Anwender noch?, Abbruch erst in Prt()
        GetpApp()->Reschedule();
        ::SetSwVisArea( pPrtShell, pPage->Frm() );
        pPrn->StartPage();
        pPage->GetUpper()->Paint( pPage->Frm() );
//      SFX_APP()->SpoilDemoOutput( *pPrtShell->GetOut(), pPage->Frm().SVRect());
        SwPaintQueue::Repaint();
        pPrn->EndPage();
        pPage = bReverse ? pPage->GetPrev() : pPage->GetNext();
    }
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostItsEndDoc( ViewShell* pPrtShell, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:21
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


void lcl_PrintPostItsEndDoc( ViewShell* pPrtShell,
            _SetGetExpFlds& rPostItFields, MultiSelection &rMulti,
            const XubString& rJobName, BOOL& rStartJob, int& rJobStartError,
            BOOL bRgt, BOOL bLft, BOOL bRev )
{
    USHORT nPostIts = rPostItFields.Count();
    if( !nPostIts )
        // Keine Arbeit
        return;

    SET_CURR_SHELL( pPrtShell );

    SwDoc* pPrtDoc = pPrtShell->GetDoc();

    // Dokument leeren und ans Dokumentende gehen
    SwPaM aPam( pPrtDoc->GetNodes().GetEndOfContent() );
    aPam.Move( fnMoveBackward, fnGoDoc );
    aPam.SetMark();
    aPam.Move( fnMoveForward, fnGoDoc );
    pPrtDoc->Delete( aPam );

    for( USHORT i = 0, nVirtPg, nLineNo; i < nPostIts; ++i )
    {
        _PostItFld& rPostIt = (_PostItFld&)*rPostItFields[ i ];
        if( rPostIt.GetPageNo( rMulti, bRgt, bLft, nVirtPg, nLineNo ) )
            lcl_FormatPostIt( pPrtShell->GetDoc(), aPam,
                           rPostIt.GetPostIt(), nVirtPg, nLineNo );
    }

    lcl_PrintPostIts( pPrtShell, rJobName, rStartJob, rJobStartError, bRev );
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostItsEndPage( ViewShell* pPrtShell, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:22
 *  Aenderung   :
 ******************************************************************************/


void lcl_PrintPostItsEndPage( ViewShell* pPrtShell,
            _SetGetExpFlds& rPostItFields, USHORT nPageNo, MultiSelection &rMulti,
            const XubString& rJobName, BOOL& rStartJob, int& rJobStartError,
            BOOL bRgt, BOOL bLft, BOOL bRev )
{
    USHORT nPostIts = rPostItFields.Count();
    if( !nPostIts )
        // Keine Arbeit
        return;

    SET_CURR_SHELL( pPrtShell );

    USHORT i = 0, nVirtPg, nLineNo;
    while( ( i < nPostIts ) &&
           ( nPageNo != ((_PostItFld&)*rPostItFields[ i ]).
                                GetPageNo( rMulti,bRgt, bLft, nVirtPg, nLineNo )))
        ++i;
    if(i == nPostIts)
        // Nix zu drucken
        return;

    SwDoc* pPrtDoc = pPrtShell->GetDoc();

    // Dokument leeren und ans Dokumentende gehen
    SwPaM aPam( pPrtDoc->GetNodes().GetEndOfContent() );
    aPam.Move( fnMoveBackward, fnGoDoc );
    aPam.SetMark();
    aPam.Move( fnMoveForward, fnGoDoc );
    pPrtDoc->Delete( aPam );

    while( i < nPostIts )
    {
        _PostItFld& rPostIt = (_PostItFld&)*rPostItFields[ i ];
        if( nPageNo == rPostIt.GetPageNo( rMulti, bRgt, bLft, nVirtPg, nLineNo ) )
            lcl_FormatPostIt( pPrtShell->GetDoc(), aPam,
                                rPostIt.GetPostIt(), nVirtPg, nLineNo );
        ++i;
    }
    lcl_PrintPostIts( pPrtShell, rJobName, rStartJob, rJobStartError, bRev );
}

/******************************************************************************
 *  Methode     :   void ViewShell::SetPrt( SfxPrinter *pNew )
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:22
 *  Aenderung   :
 ******************************************************************************/

void ViewShell::InitPrt( SfxPrinter *pPrt )
{
    //Fuer den Printer merken wir uns einen negativen Offset, der
    //genau dem Offset de OutputSize entspricht. Das ist notwendig,
    //weil unser Ursprung der linken ober Ecke der physikalischen
    //Seite ist, die Ausgaben (SV) aber den Outputoffset als Urstprung
    //betrachten.
    if ( pPrt )
    {
        aPrtOffst = pPrt->GetPageOffset();
        aPrtOffst += pPrt->GetMapMode().GetOrigin();
        MapMode aMapMode( pPrt->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pPrt->SetMapMode( aMapMode );
        pPrt->SetLineColor();
        pPrt->SetFillColor();
    }
    else
        aPrtOffst.X() = aPrtOffst.Y() = 0;

    if ( !pWin )
        pOut = pPrt;    //Oder was sonst?
}

void ViewShell::SetPrt( SfxPrinter *pNew )
{
    GetDoc()->SetPrt( pNew );
}

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageOrientation( USHORT eOri )
{
    ASSERT( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    USHORT nAll = GetDoc()->GetPageDescCnt();
    BOOL bNewOri = Orientation(eOri) == ORIENTATION_PORTRAIT ? FALSE : TRUE;

    for( USHORT i = 0; i < nAll; ++ i )
    {
        const SwPageDesc& rOld = GetDoc()->GetPageDesc( i );
        if( rOld.GetLandscape() != bNewOri )
        {
            SwPageDesc aNew( rOld );
            aNew.SetLandscape( bNewOri );
            SwFrmFmt& rFmt = aNew.GetMaster();
            SwFmtFrmSize aSz( rFmt.GetFrmSize() );
            // Groesse anpassen.
            // PORTRAIT  -> Hoeher als Breit
            // LANDSCAPE -> Breiter als Hoch
            // Hoehe ist die VarSize, Breite ist die FixSize (per Def.)
            if( bNewOri ? aSz.GetHeight() > aSz.GetWidth()
                        : aSz.GetHeight() < aSz.GetWidth() )
            {
                SwTwips aTmp = aSz.GetHeight();
                aSz.SetHeight( aSz.GetWidth() );
                aSz.SetWidth( aTmp );
                rFmt.SetAttr( aSz );
            }
            GetDoc()->ChgPageDesc( i, aNew );
        }
    }
}

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageSize( Size &rSz )
{
    ASSERT( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    SwDoc* pDoc = GetDoc();
    USHORT nAll = pDoc->GetPageDescCnt();

    for( USHORT i = 0; i < nAll; ++i )
    {
        // Fuer WIN95 als Pointer anlegen! (falsche Optimierung!!)
        SwPageDesc* pNew = new SwPageDesc( pDoc->GetPageDesc( i ) );
        SwFrmFmt& rPgFmt = pNew->GetMaster();
        Size aSz( rSz );
        const BOOL bOri = pNew->GetLandscape();
        if( bOri  ? aSz.Height() > aSz.Width()
                  : aSz.Height() < aSz.Width() )
        {
            SwTwips aTmp = aSz.Height();
            aSz.Height() = aSz.Width();
            aSz.Width()  = aTmp;
        }

        SwFmtFrmSize aFrmSz( rPgFmt.GetFrmSize() );
        aFrmSz.SetSize( aSz );
        rPgFmt.SetAttr( aFrmSz );
        pDoc->ChgPageDesc( i, *pNew );
        delete pNew;
    }
}

/******************************************************************************
 *  Methode     :   void ViewShell::CalcPagesForPrint( short nMax, BOOL ...
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 15:33
 *  Aenderung   :   MA 07. Jun. 95
 ******************************************************************************/



void lcl_SetState( SfxProgress& rProgress, ULONG nPage, ULONG nMax,
    const XubString *pStr, ULONG nAct, ULONG nCnt, ULONG nOffs, ULONG nPageNo )
{
    XubString aTmp = XubString::CreateFromInt64( nPageNo );
    if( pStr )
    {
        aTmp += ' ';
        aTmp += *pStr;
        if( nCnt )
        {
            nMax *= 2;
            rProgress.SetStateText( (nAct-1)*nMax+nPage+nOffs,
                                        aTmp, nCnt*nMax );
        }
        else
            rProgress.SetStateText( nPage, aTmp, nMax );
    }
    else
    {
        aTmp += ' '; aTmp += '('; aTmp += XubString::CreateFromInt64( nPage );
        aTmp += '/'; aTmp += XubString::CreateFromInt64( nMax ); aTmp += ')';
        rProgress.SetStateText( nPage, aTmp, nMax );
    }
}



void ViewShell::CalcPagesForPrint( USHORT nMax, SfxProgress* pProgress,
    const XubString* pStr, ULONG nMergeAct, ULONG nMergeCnt )
{
    SET_CURR_SHELL( this );

    //Seitenweise durchformatieren, by the way kann die Statusleiste
    //angetriggert werden, damit der Anwender sieht worauf er wartet.
    //Damit der Vorgang moeglichst transparent gestaltet werden kann
    //Versuchen wir mal eine Schaetzung.
    SfxPrinter* pPrt = GetPrt();
    BOOL bPrtJob = pPrt ? pPrt->IsJobActive() : FALSE;
    SwRootFrm* pLayout = GetLayout();
    ULONG nStatMax = pLayout->GetPageNum();

    const SwFrm *pPage = pLayout->Lower();
    SwLayAction aAction( pLayout, Imp() );

    if( pProgress )
    {
        // HACK, damit die Anzeige sich nicht verschluckt.
        const XubString aTmp( SW_RES( STR_STATSTR_PRINT ) );
        pProgress->SetText( aTmp );
        lcl_SetState( *pProgress, 1, nStatMax, pStr, nMergeAct, nMergeCnt, 0, 1 );
    }

    pLayout->StartAllAction();
    for ( USHORT i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        if ( bPrtJob && !pPrt->IsJobActive() )
            break;

        if( pProgress )
        {
            //HACK, damit die Anzeige sich nicht verschluckt.
            if ( i > nStatMax )
                nStatMax = i;
            lcl_SetState( *pProgress, i, nStatMax, pStr, nMergeAct, nMergeCnt, 0, i );
            pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
        }

        if ( bPrtJob && !pPrt->IsJobActive() )
            break;
        pPage->Calc();
        SwRect aOldVis( VisArea() );
        aVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( FALSE );
        aAction.SetWaitAllowed( FALSE );
        aAction.SetReschedule( TRUE );
        aAction.Action();
        aVisArea = aOldVis;             //Zuruecksetzen wg. der Paints!
        Imp()->SetFirstVisPageInvalid();
        SwPaintQueue::Repaint();
    }
    pLayout->EndAllAction();
}

/******************************************************************************
 *  Methode     :   void ViewShell::Prt( const SwPrtOptions& rOptions )
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 15:33
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


BOOL ViewShell::Prt( SwPrtOptions& rOptions, SfxProgress& rProgress )
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Immer die Druckroutine in viewpg.cxx (fuer Seitenvorschau) mitpflegen!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    BOOL bStartJob = FALSE;

    BOOL bSelection = rOptions.bPrintSelection;
    // Damit beim Selektionsdruck nicht mit einer leeren Seite gestartet wird
    BOOL bIgnoreEmptyPage = bSelection;

    MultiSelection aMulti( rOptions.aMulti );

    if ( !aMulti.GetSelectCount() )
        return bStartJob;

    Range aPages( aMulti.FirstSelected(), aMulti.LastSelected() );
    if ( aPages.Max() > USHRT_MAX )
        aPages.Max() = USHRT_MAX;

    ASSERT( aPages.Min() > 0,
            "Seite 0 Drucken?" );
    ASSERT( aPages.Min() <= aPages.Max(),
            "MinSeite groesser MaxSeite." );
    // wenn kein Drucker vorhanden ist, wird nicht gedruckt
    SfxPrinter* pPrt = GetPrt();
    if( !pPrt || !pPrt->GetName().Len() )
    {
        ASSERT( FALSE, "Drucken ohne Drucker?" );
        return bStartJob;
    }

    if( !rOptions.GetJobName().Len() && !pPrt->IsJobActive() )
        return bStartJob;

    // Einstellungen am Drucker merken
    SwPrtOptSave aPrtSave( pPrt );

    // eine neue Shell fuer den Printer erzeugen
    ViewShell *pShell;
    SwDoc *pPrtDoc;
    SvEmbeddedObjectRef aDocShellRef;

    if ( bSelection )
    {
        ASSERT( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
        SwFEShell* pFESh = (SwFEShell*)this;
        // Wir bauen uns ein neues Dokument
        pPrtDoc = new SwDoc;
        pPrtDoc->AddLink();
        pPrtDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)aDocShellRef );
        pPrtDoc->LockExpFlds();

        // Der Drucker wird uebernommen
        pPrtDoc->SetPrt( pPrt );

        const SfxPoolItem* pCpyItem;
        const SfxItemPool& rPool = GetAttrPool();
        for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
            if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
                pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

        // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
        //                              the PrintDoc - will be replaced!
        pPrtDoc->ReplaceStyles( *GetDoc() );

        SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
        SwShellCrsr *pFirstCrsr = (SwShellCrsr*)*((SwCursor*)pActCrsr->GetNext());
        if( !pActCrsr->HasMark() ) // bei Multiselektion kann der aktuelle Cursor leer sein
            pActCrsr = (SwShellCrsr*)*((SwCursor*)pActCrsr->GetPrev());
        // Die Y-Position der ersten Selektion
        long nMinY = pFESh->IsTableMode() ? pFESh->GetTableCrsr()->GetSttPos().Y()
                                   : pFirstCrsr->GetSttPos().Y();
        SwPageFrm* pPage = (SwPageFrm*)GetLayout()->Lower();
        // Suche die zugehoerige Seite
        while ( pPage->GetNext() && nMinY >= pPage->GetNext()->Frm().Top() )
            pPage = (SwPageFrm*)pPage->GetNext();
        // und ihren Seitendescribtor
        SwPageDesc *pSrc = pPage->GetPageDesc();
        SwPageDesc* pPageDesc = pPrtDoc->FindPageDescByName(
                                    pPage->GetPageDesc()->GetName() );

        if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
        {   // Am letzten Absatz die Absatzattribute richten:
            SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
            SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
            SwCntntNode *pLastNd =
                pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );
            // Hier werden die Absatzattribute des ersten Absatzes uebertragen
            if( pLastNd && pLastNd->IsTxtNode() )
                ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
        }

        // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
//      if( aDocShellRef.Is() )
//          SwDataExchange::InitOle( aDocShellRef, pPrtDoc );
        // und fuellen es mit dem selektierten Bereich
        pFESh->Copy( pPrtDoc );

        //Jetzt noch am ersten Absatz die Seitenvorlage setzen
        {
            SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
            SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx ); // gehe zum 1. ContentNode
            if( pFESh->IsTableMode() )
            {
                SwTableNode* pTNd = pCNd->FindTableNode();
                if( pTNd )
                    pTNd->GetTable().GetFrmFmt()->SetAttr( SwFmtPageDesc( pPageDesc ) );
            }
            else
            {
                pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
                if( pFirstCrsr->HasMark() )
                {
                    SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                    if( pTxtNd )
                    {
                        SwCntntNode *pFirstNd =
                            pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );
                        // Hier werden die Absatzattribute des ersten Absatzes uebertragen
                        if( pFirstNd && pFirstNd->IsTxtNode() )
                            ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                    }
                }
            }
        }

        // eine ViewShell darauf
        pShell = new ViewShell( *pPrtDoc, 0, pOpt );
        pPrtDoc->SetRefForDocShell( 0 );
    }
    else
    {
        pPrtDoc = GetDoc();
        pShell = new ViewShell( *this, 0 );
    }

    {   //Zusaetzlicher Scope, damit die CurrShell vor dem zerstoeren der
        //Shell zurueckgesetzt wird.

    SET_CURR_SHELL( pShell );

    //JP 01.02.99: das ReadOnly Flag wird NIE mitkopiert; Bug 61335
    if( pOpt->IsReadonly() )
        pShell->pOpt->SetReadonly( TRUE );

    pShell->PrepareForPrint( rOptions );

    // gibt es versteckte Absatzfelder, braucht nicht beruecksichtigt werden,
    // wenn diese bereits ausgeblendet sind
    BOOL bHiddenFlds = FALSE;
    SwHiddenParaFieldType* pFldType = 0;
    if ( GetViewOptions()->IsShowHiddenPara() )
    {
        pFldType    = (SwHiddenParaFieldType*)pPrtDoc->
                                          GetSysFldType(RES_HIDDENPARAFLD);
        bHiddenFlds = pFldType && pFldType->GetDepends();
        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
        }
    }


    XubString *pStr;
    ULONG nMergeAct = rOptions.nMergeAct, nMergeCnt = rOptions.nMergeCnt;
    if( nMergeAct )
    {
        pStr = new SW_RESSTR(STR_STATSTR_LETTER);
        *pStr += ' ';
        *pStr += XubString::CreateFromInt64( nMergeAct );
        if( nMergeCnt )
        {
            *pStr += '/';
            *pStr += XubString::CreateFromInt64( nMergeCnt );
        }
    }
    else
    {
        pStr = 0;
        ++nMergeAct;
    }

    // Seiten fuers Drucken formatieren
    pShell->CalcPagesForPrint( (USHORT)aPages.Max(), &rProgress, pStr,
                                nMergeAct, nMergeCnt );

    if( rOptions.GetJobName().Len() || pPrt->IsJobActive() )
    {
        BOOL bStop = FALSE;
        int nJobStartError = JOBSET_ERR_DEFAULT;

        USHORT nCopyCnt = rOptions.bCollate ? rOptions.nCopyCount : 1;

        USHORT nPrintCount = 1;
        XubString sJobName( rOptions.GetJobName() );

        for ( USHORT nCnt = 0; !bStop && nCnt < nCopyCnt; nCnt++ )
        {
            const SwPageFrm *pStPage  = (SwPageFrm*)pShell->GetLayout()->Lower();
            const SwFrm     *pEndPage = pStPage;

            USHORT nFirstPageNo = 0;
            USHORT nLastPageNo  = 0;
            USHORT nPageNo      = 1;

            if( rOptions.bSinglePrtJobs && sJobName.Len() &&
                ( bStartJob || rOptions.bJobStartet ) )
            {
                pPrt->EndJob();
                bStartJob = FALSE;
                rOptions.bJobStartet = TRUE;

                // Reschedule statt Yield, da Yield keine Events abarbeitet
                // und es sonst eine Endlosschleife gibt.
                while( pPrt->IsPrinting() )
                        rProgress.Reschedule();

                sJobName = rOptions.MakeNextJobName();
                nJobStartError = JOBSET_ERR_DEFAULT;
            }

            for( USHORT i = 1; i <= (USHORT)aPages.Max(); ++i )
            {
                if( i < (USHORT)aPages.Min() )
                {
                    if( !pStPage->GetNext() )
                        break;
                    pStPage = (SwPageFrm*)pStPage->GetNext();
                    pEndPage= pStPage;
                }
                else if( i == (USHORT)aPages.Min() )
                {
                    nFirstPageNo = i;
                    nLastPageNo = nFirstPageNo;
                    if( !pStPage->GetNext() || (i == (USHORT)aPages.Max()) )
                        break;
                    pEndPage = pStPage->GetNext();
                }
                else if( i > (USHORT)aPages.Min() )
                {
                    nLastPageNo = i;
                    if( !pEndPage->GetNext() || (i == (USHORT)aPages.Max()) )
                        break;
                    pEndPage = pEndPage->GetNext();
                }
            }

            if( !nFirstPageNo )
            {
                if( bHiddenFlds )
                {
                    SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
                    pFldType->Modify( &aHnt, 0);
                    CalcPagesForPrint( (USHORT)aPages.Max(), &rProgress, pStr,
                                        nMergeAct, nMergeCnt );
                }
                bStop = TRUE;
                break;
            }

// HACK: Hier muss von der MultiSelection noch eine akzeptable Moeglichkeit
// geschaffen werden, alle Seiten von Seite x an zu deselektieren.
// Z.B. durch SetTotalRange ....

//          aMulti.Select( Range( nLastPageNo+1, SELECTION_MAX ), FALSE );
            MultiSelection aTmpMulti( Range( 1, nLastPageNo ) );
            long nTmpIdx = aMulti.FirstSelected();
            while ( SFX_ENDOFSELECTION != nTmpIdx && nTmpIdx <= long(nLastPageNo) )
            {
                aTmpMulti.Select( nTmpIdx );
                nTmpIdx = aMulti.NextSelected();
            }
            aMulti = aTmpMulti;
// Ende des HACKs

            const USHORT nSelCount = USHORT(aMulti.GetSelectCount()
                            /* * nCopyCnt*/);

            rProgress.SetText( SW_RESSTR(STR_STATSTR_PRINT) );
            lcl_SetState( rProgress, 1, nSelCount, pStr,
                                nMergeAct, nMergeCnt, nSelCount, 1 );

            if ( rOptions.bPrintReverse )
            {
                const SwFrm *pTmp = pStPage;
                pStPage  = (SwPageFrm*)pEndPage;
                pEndPage = pTmp;
                nPageNo  = nLastPageNo;
            }
            else
                nPageNo = nFirstPageNo;

            // PostitListe holen
            _SetGetExpFlds aPostItFields;
            SwDoc*     pPostItDoc   = 0;
            ViewShell* pPostItShell = 0;
            if( rOptions.nPrintPostIts != POSTITS_NONE )
            {
                lcl_GetPostIts( pDoc, aPostItFields );
                pPostItDoc   = new SwDoc;
                pPostItDoc->SetPrt( pPrt );
                pPostItShell = new ViewShell( *pPostItDoc, 0,
                                               pShell->GetViewOptions() );
                // Wenn PostIts am Dokumentenende gedruckt werden sollen,
                // die Druckreihenfolge allerdings umgekehrt ist, dann hier
                if ( ( rOptions.nPrintPostIts == POSTITS_ENDDOC ) &&
                        rOptions.bPrintReverse )
                        lcl_PrintPostItsEndDoc( pPostItShell, aPostItFields,
                        aMulti, sJobName, bStartJob, nJobStartError,
                        rOptions.bPrintRightPage, rOptions.bPrintLeftPage, TRUE );

            }

            // aOldMapMode wird fuer das Drucken von Umschlaegen gebraucht.
            MapMode aOldMapMode;

            const SwPageDesc *pLastPageDesc = NULL;
            const BOOL bSetOrient = pPrt->HasSupport( SUPPORT_SET_ORIENTATION );
            const BOOL bSetPaperSz = pPrt->HasSupport( SUPPORT_SET_PAPERSIZE );
            const BOOL bSetPaperBin =  !rOptions.bPaperFromSetup &&
                                    pPrt->HasSupport( SUPPORT_SET_PAPERBIN );
            const BOOL bSetPrt = bSetOrient || bSetPaperSz || bSetPaperBin;

            if ( rOptions.nPrintPostIts != POSTITS_ONLY )
            {
                while( pStPage && !bStop )
                {
                    // Mag der Anwender noch ?
                    rProgress.Reschedule();
                    if( ( JOBSET_ERR_ERROR == nJobStartError )
                        || ( !pPrt->IsJobActive() &&
                            ( !sJobName.Len() || bStartJob ) ) )
                    {
                        if( bHiddenFlds )
                        {
                            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
                            pFldType->Modify( &aHnt, 0);
                            CalcPagesForPrint( (USHORT)aPages.Max() );
                        }
                        bStop = TRUE;
                        break;
                    }

                    ::SetSwVisArea( pShell, pStPage->Frm() );

                    //  wenn wir einen Umschlag drucken wird ein Offset beachtet
                    if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
                    {
                        aOldMapMode = pPrt->GetMapMode();
                        Point aNewOrigin = pPrt->GetMapMode().GetOrigin();
                        aNewOrigin += rOptions.aOffset;
                        MapMode aTmp( pPrt->GetMapMode() );
                        aTmp.SetOrigin( aNewOrigin );
                        pPrt->SetMapMode( aTmp );
                    }

                    BOOL bRightPg = pStPage->OnRightPage();
                    if( aMulti.IsSelected( nPageNo ) &&
                        ( (bRightPg && rOptions.bPrintRightPage) ||
                            (!bRightPg && rOptions.bPrintLeftPage) ) )
                    {
                        if ( bSetPrt && pLastPageDesc != pStPage->GetPageDesc() )
                        {
                            pLastPageDesc = pStPage->GetPageDesc();
                            BOOL bLandScp = pLastPageDesc->GetLandscape();

                            if( bSetPaperBin )      // Schacht einstellen.
                                pPrt->SetPaperBin( pStPage->GetFmt()->
                                                    GetPaperBin().GetValue() );
                            if ( bSetPaperSz )
                            {
                                Size aSize = pStPage->Frm().SSize();
                                if ( bLandScp && bSetOrient )
                                {
                                    long nWidth = aSize.Width();
                                    aSize.Width() = aSize.Height();
                                    aSize.Height() = nWidth;
                                }
                                Paper ePaper = SvxPaperInfo::GetSvPaper(aSize,MAP_TWIP,TRUE);
                                if ( PAPER_USER == ePaper )
                                    pPrt->SetPaperSizeUser( aSize );
                                else
                                    pPrt->SetPaper( ePaper );
                            }
                            if ( bSetOrient )
                            {
                                // Orientation einstellen: Breiter als Hoch
                                //  -> Landscape, sonst -> Portrait.
                                if( bLandScp )
                                    pPrt->SetOrientation(ORIENTATION_LANDSCAPE);
                                else
                                    pPrt->SetOrientation(ORIENTATION_PORTRAIT);
                            }
                        }
                        // Wenn PostIts nach Seite gedruckt werden sollen,
                        // jedoch Reverse eingestellt ist ...
                        if( rOptions.bPrintReverse &&
                            rOptions.nPrintPostIts == POSTITS_ENDPAGE )
                                lcl_PrintPostItsEndPage( pPostItShell, aPostItFields,
                                    nPageNo, aMulti, sJobName, bStartJob, nJobStartError,
                                    rOptions.bPrintRightPage, rOptions.bPrintLeftPage,
                                    rOptions.bPrintReverse );

                        lcl_SetState( rProgress, nPrintCount++, nSelCount,
                                            pStr, nMergeAct, nMergeCnt,
                                            nSelCount, nPageNo );

                        if( !bStartJob && JOBSET_ERR_DEFAULT == nJobStartError
                            && sJobName.Len() )
                        {
                            if( !pPrt->IsJobActive() )
                            {
                                bStartJob = pPrt->StartJob( sJobName );
                                if( !bStartJob )
                                {
                                    nJobStartError = JOBSET_ERR_ERROR;
                                    continue;
                                }
                            }
                            pShell->InitPrt( pPrt );
                            ::SetSwVisArea( pShell, pStPage->Frm() );
                            nJobStartError = JOBSET_ERR_ISSTARTET;
                        }
                        // Bei Selektionsdruck wird ggf. die erste leere Seite ausgelassen
                        if( !bIgnoreEmptyPage || (0==(bIgnoreEmptyPage=TRUE)) ||
                            pStPage->Frm().Height() )
                        {
                            pPrt->StartPage();
                            pStPage->GetUpper()->Paint( pStPage->Frm() );
//                          SFX_APP()->SpoilDemoOutput( *pShell->GetOut(),
//                                                           pStPage->Frm().SVRect() );
                            pPrt->EndPage();
                        }
                        SwPaintQueue::Repaint();

                        //Wenn eine Tabelle heraushaengt, so wird der Rest der
                        //Tabelle auf zusaetzliche Seiten verteilt.
                        const SwFrm *pFrm = pStPage->FindLastBodyCntnt();
                        if ( pFrm && pFrm->IsInTab() )
                        {
                            pFrm = pFrm->FindTabFrm();
                            const SwFrm *pBody = pStPage->FindBodyCont();
                            long nBottom = pBody->Prt().Bottom();
                            nBottom += pBody->Frm().Top();
                            if ( nBottom < pFrm->Frm().Bottom() )
                            {
                                SwRootFrm *pRoot = (SwRootFrm*)pStPage->GetUpper();
                                long nDiff = pFrm->Frm().Bottom() - nBottom;
                                SwRect aNewVis( pBody->Prt() );
                                aNewVis += pBody->Frm().Pos();
                                SwRect aTmp( pShell->VisArea() );
                                aTmp.SSize().Height() = LONG_MAX - aTmp.Top();
                                aTmp.SSize().Width()  = LONG_MAX - aTmp.Left();
                                ::SetSwVisArea( pShell, aTmp );
                                while ( nDiff > 0 )
                                {
                                    pPrt->StartPage();

                                    //VisArea auf die Tabelle schummeln
                                    aNewVis.Pos().Y() += aNewVis.Height()+1;

                                    //Offset in den MapMode schummeln.
                                    MapMode aMap( pPrt->GetMapMode() );
                                    Point aTmp( aMap.GetOrigin() );
                                    aTmp.Y() -= aNewVis.Height()+1;
                                    aMap.SetOrigin( aTmp );
                                    pPrt->SetMapMode( aMap );

                                    pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_INIT );
                                    SwTxtFrm::SetMinPrtLine( aNewVis.Pos().Y() );
                                    pFrm->PaintBaBo( aNewVis, pStPage, TRUE );
                                    if ( pShell->Imp()->HasDrawView() )
                                    {
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_LOCKLINES );
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->GetHellId(), aNewVis );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_PAINTLINES );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_UNLOCKLINES );
                                    }
                                    pFrm->Paint( aNewVis );
                                    if ( pShell->Imp()->HasDrawView() )
                                    {
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->GetHeavenId(),
                                                                   aNewVis );
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->
                                                                GetControlsId(), aNewVis );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_PAINTLINES );
                                    }
                                    pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_EXIT );
                                    SwTxtFrm::SetMinPrtLine( 0 );

//                                  SFX_APP()->SpoilDemoOutput( *pShell->GetOut(),
//                                                               aNewVis.SVRect() );
                                    pPrt->EndPage();
                                    SwPaintQueue::Repaint();
                                    nDiff -= pBody->Prt().Height();
                                }
                            }
                        }

                        // Wenn PostIts nach Seite gedruckt werden sollen ...
                        if( (!rOptions.bPrintReverse) &&
                            rOptions.nPrintPostIts == POSTITS_ENDPAGE )
                                lcl_PrintPostItsEndPage( pPostItShell, aPostItFields,
                                    nPageNo, aMulti, sJobName, bStartJob, nJobStartError,
                                    rOptions.bPrintRightPage, rOptions.bPrintLeftPage,
                                    rOptions.bPrintReverse );
                    }

                    // den eventl. fuer Umschlaege modifizierte OutDevOffset wieder
                    // zuruecksetzen.
                    if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
                        pPrt->SetMapMode( aOldMapMode );

                    if ( pStPage == pEndPage )
                        pStPage = 0;
                    else if ( rOptions.bPrintReverse )
                    {
                        --nPageNo;
                        pStPage = (SwPageFrm*)pStPage->GetPrev();
                    }
                    else
                    {   ++nPageNo;
                        pStPage = (SwPageFrm*)pStPage->GetNext();
                    }
                }
                if ( bStop )
                    break;
            }

            // Wenn PostIts am Dokumentenende gedruckt werden sollen, dann hier machen
            if( ((rOptions.nPrintPostIts == POSTITS_ENDDOC) && !rOptions.bPrintReverse)
                || (rOptions.nPrintPostIts == POSTITS_ONLY) )
                    lcl_PrintPostItsEndDoc( pPostItShell, aPostItFields, aMulti,
                        sJobName, bStartJob, nJobStartError,
                        rOptions.bPrintRightPage, rOptions.bPrintLeftPage,
                        rOptions.bPrintReverse );

            if( pPostItShell )
            {
                pPostItDoc->_SetPrt( 0 );   //damit am echten DOC der Drucker bleibt
                delete pPostItShell;        //Nimmt das PostItDoc mit ins Grab.
            }

            if( bStartJob )
                rOptions.bJobStartet = TRUE;
        }

        if( bHiddenFlds && !bStop )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
            CalcPagesForPrint( (USHORT)aPages.Max() );
        }
    }
    delete pStr;

    }  //Zus. Scope wg. CurShell!

    delete pShell;

    if ( bSelection )
    {
         // damit das Dokument nicht den Drucker mit ins Grab nimmt
        pPrtDoc->_SetPrt( NULL );

        if ( !pPrtDoc->RemoveLink() )
            delete pPrtDoc;
    }
    return bStartJob;
}

/******************************************************************************
 *  Methode     :   PrtOle2()
 *  Beschreibung:
 *  Erstellt    :   PK 07.12.94
 *  Aenderung   :   MA 16. Feb. 95
 ******************************************************************************/



void ViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
    //Wir brauchen eine Shell fuer das Drucken. Entweder hat das Doc schon
    //eine, dann legen wir uns eine neue Sicht an, oder das Doc hat noch
    //keine, dann erzeugen wir die erste Sicht.
    ViewShell *pSh;
    if( pDoc->GetRootFrm() && pDoc->GetRootFrm()->GetCurrShell() )
        pSh = new ViewShell( *pDoc->GetRootFrm()->GetCurrShell(), 0, pOleOut );
    else
        pSh = new ViewShell( *pDoc, 0, pOpt, pOleOut );

    {
        SET_CURR_SHELL( pSh );
        pSh->SetPrtFormatOption( TRUE );

        SwRect aSwRect( rRect );
        pSh->aVisArea = aSwRect;

        if ( pDoc->IsBrowseMode() && pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( FALSE );
            pDoc->GetRootFrm()->Lower()->InvalidateSize();
        }

        // Seiten fuers Drucken formatieren
        pSh->CalcPagesForPrint( SHRT_MAX );

        //#39275# jetzt will der Meyer doch ein Clipping
        pOleOut->Push( PUSH_CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );
//      SFX_APP()->SpoilDemoOutput( *pOleOut, rRect );
        pOleOut->Pop();

        // erst muss das CurrShell Object zerstoert werden!!
    }
    delete pSh;
}

/******************************************************************************
 *  Methode     :   IsAnyFieldInDoc()
 *  Beschreibung:   Stellt fest, ob im DocNodesArray Felder verankert sind
 *  Erstellt    :   JP 27.07.95
 *  Aenderung   :   JP 10.12.97
 ******************************************************************************/



BOOL ViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    USHORT nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    for( USHORT n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
                return TRUE;
        }
    return FALSE;
}



/******************************************************************************
 *  Klasse      :   SwPrtOptSave
 *  Erstellt    :   AMA 12.07.95
 *  Aenderung   :   AMA 12.07.95
 *  Holt sich im Ctor folgende Einstellungen des Druckers, die im Dtor dann
 *  wieder im Drucker gesetzt werden (falls sie sich ueberhaupt geaendert haben)
 *  - PaperBin - Orientation - PaperSize -
 ******************************************************************************/



SwPrtOptSave::SwPrtOptSave( Printer *pPrinter )
    : pPrt( pPrinter )
{
    if ( pPrt )
    {
        ePaper = pPrt->GetPaper();
        if ( PAPER_USER == ePaper )
            aSize = pPrt->GetPaperSize();
        eOrientation = pPrt->GetOrientation();
        nPaperBin = pPrt->GetPaperBin();

    }
}



SwPrtOptSave::~SwPrtOptSave()
{
    if ( pPrt )
    {
        if ( PAPER_USER == ePaper )
        {
            if( pPrt->GetPaperSize() != aSize )
                pPrt->SetPaperSizeUser( aSize );
        }
        else if ( pPrt->GetPaper() != ePaper )
            pPrt->SetPaper( ePaper );
        if ( pPrt->GetOrientation() != eOrientation)
            pPrt->SetOrientation( eOrientation );
        if ( pPrt->GetPaperBin() != nPaperBin )
            pPrt->SetPaperBin( nPaperBin );
    }
}



void ViewShell::PrepareForPrint(  const SwPrtOptions &rOptions )
{
    // Viewoptions fuer den Drucker setzen
    pOpt->SetGraphic ( TRUE == rOptions.bPrintGraph );
    pOpt->SetTable   ( TRUE == rOptions.bPrintTable );
    pOpt->SetDraw    ( TRUE == rOptions.bPrintDraw  );
    pOpt->SetControl ( TRUE == rOptions.bPrintControl );
    pOpt->SetPageBack( TRUE == rOptions.bPrintPageBackground );
    pOpt->SetBlackFont( TRUE == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        FASTBOOL bDraw = rOptions.bPrintDraw;
        pDrawView->SetLineDraftPrn( !bDraw );
        pDrawView->SetFillDraftPrn( !bDraw );
        pDrawView->SetGrafDraftPrn( !bDraw );
        pDrawView->SetTextDraftPrn( !bDraw );
    }
}

/************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.4  2001/02/14 09:57:12  jp
      changes: international -> localdatawrapper

      Revision 1.3  2001/01/17 12:09:31  jp
      remove compiler warning

      Revision 1.2  2000/10/25 12:03:41  jp
      Spellchecker/Hyphenator are not longer member of the shells

      Revision 1.1.1.1  2000/09/19 00:08:29  hr
      initial import

      Revision 1.193  2000/09/18 16:04:38  willem.vandorp
      OpenOffice header added.

      Revision 1.192  2000/06/15 19:06:54  jp
      Prt: don't forget to start the printjob, if sortet, copies and single printjobes are desired

      Revision 1.191  2000/05/23 17:37:30  jp
      Bugfixes for Unicode

      Revision 1.190  2000/05/10 13:43:09  ama
      Unicode changes

      Revision 1.189  2000/05/09 11:43:25  ama
      Unicode changes

      Revision 1.188  2000/04/27 07:37:23  os
      UNICODE

      Revision 1.187  2000/03/28 20:57:23  jp
      Bug #74562#: Prt - on the printer can be run a job, its not an error

      Revision 1.186  2000/02/21 16:01:07  ama
      Fix #73147#: No spoildemo for free versions

      Revision 1.185  2000/02/11 14:36:15  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.184  1999/12/16 14:31:58  hjs
      #65293# static instead of const - ask jp

      Revision 1.183  1999/12/14 14:28:34  jp
      Bug #69595#: print can create single Jobs

      Revision 1.182  1999/10/25 19:09:24  tl
      ongoing ONE_LINGU implementation

      Revision 1.181  1999/08/31 08:45:54  TL
      #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.180   31 Aug 1999 10:45:54   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.179   26 Aug 1999 11:35:14   AMA
   Fix #68153#: Draft mode for controls

      Rev 1.178   29 Jul 1999 17:13:46   JP
   Bug #67951#: Prt - replace all templates in dest Doc

      Rev 1.177   08 Mar 1999 15:55:34   AMA
   Fix #62873#: Vorschau von HTML-Vorlagen

      Rev 1.176   02 Mar 1999 16:07:56   AMA
   Fix #62568#: Invalidierungen so sparsam wie moeglich, so gruendlich wie noetig

      Rev 1.175   22 Feb 1999 08:35:24   MA
   1949globale Shell entsorgt, Shells am RootFrm

      Rev 1.174   01 Feb 1999 19:41:16   JP
   Bug #61335#: ReadOnlyFlag vorm Drucken uebertragen

      Rev 1.173   25 Nov 1998 11:50:20   MA
   #54599# InitPrt auch mit NULL

      Rev 1.172   28 Oct 1998 14:02:36   AMA
   Fix #58223#58219#: Seiten/Absatzformate beim Selektionsdruck beachten.

      Rev 1.171   27 Jul 1998 17:35:24   AMA
   Fix #53934#: Endlosschleife beim Speichern durch selektiertes Objekt

      Rev 1.170   15 Jul 1998 14:05:38   AMA
   Fix #53012#: Das InitPrt muss nach dem StartJob erfolgen

      Rev 1.169   14 Jul 1998 10:26:28   AMA
   Fix #52725#: Verzoegerten PrintJobStart beachten beim Formatieren

      Rev 1.168   07 Jul 1998 14:38:24   AMA
   Chg: DoPrint uebernimmt das Drucken

      Rev 1.167   03 Jul 1998 17:26:24   AMA
   Chg: DoPrint uebernimmt das Drucken

      Rev 1.166   29 Jun 1998 12:05:20   AMA
   Feat.: Notizen mit Zeilenangabe ausdrucken

      Rev 1.165   23 Jun 1998 16:05:06   AMA
   Fix #51465#: Die _virtuelle_ Seitennummer entscheidet ueber rechts/links

      Rev 1.164   17 Jun 1998 22:40:22   JP
   PostIts: GetPageNo - immer mit der virtuellen abpruefen, aber die virtuelle drucken

      Rev 1.163   04 Jun 1998 18:20:42   AMA
   Chg: UNO-Controls jetzt im eigenen Drawing-Layer

      Rev 1.162   27 Apr 1998 15:09:08   MA
   ein paar sv2vcl

      Rev 1.161   24 Apr 1998 11:17:44   AMA
   Fix #49654#49450#: Selektiondrucken: GPF bei OLE; Sprachverlust wg. Poolattr.

      Rev 1.160   23 Apr 1998 09:33:36   MA
   #49472# Optional ein Outdev durchschleusen

      Rev 1.159   26 Mar 1998 17:58:52   MA
   Wechsel fuer Drucker/Jobsetup jetzt vollstaendig und am Dokument

      Rev 1.158   24 Mar 1998 18:20:02   MA
   #39275# Jetzt will der Meyer doch ein Clipping

      Rev 1.157   27 Jan 1998 22:35:36   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.156   22 Jan 1998 20:08:58   JP
   CTOR des SwPaM umgestellt

      Rev 1.155   10 Dec 1997 17:05:34   JP
   IsAnyField: if abfrage gerichtet

      Rev 1.154   28 Nov 1997 09:11:24   MA
   includes

      Rev 1.153   20 Nov 1997 12:44:20   MA
   includes

      Rev 1.152   18 Nov 1997 13:39:50   MA
   #45516# LongTableHack, auch Grafiken drucken

      Rev 1.151   03 Nov 1997 13:07:28   MA
   precomp entfernt

      Rev 1.150   13 Oct 1997 15:54:26   JP
   pNext vom Ring wurde privat; zugriff ueber GetNext()

      Rev 1.149   13 Oct 1997 10:30:12   MA
   Umbau/Vereinfachung Paint

      Rev 1.148   09 Oct 1997 16:05:20   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.147   10 Sep 1997 15:33:56   AMA
   Fix #43570#: PrtFormat-Flag formatiert auch im BrowseMode fuer den Drucker

      Rev 1.146   09 Sep 1997 10:53:32   MA
   pragmas entfernt

      Rev 1.145   15 Aug 1997 12:24:08   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.144   12 Aug 1997 13:43:16   OS
   Header-Umstellung

      Rev 1.143   07 Aug 1997 15:01:20   OM
   Headerfile-Umstellung

      Rev 1.142   18 Jun 1997 09:18:18   MA
   #40790# VisArea fuer PrtOle und BrowseMode

      Rev 1.141   06 Jun 1997 12:45:42   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.140   14 Apr 1997 18:21:26   MA
   #38806# Options auch fuer Prospect, jetzt mit eigener Methode

      Rev 1.139   18 Feb 1997 16:54:12   MA
   #36138# toleranz fuer GetSvPaper

      Rev 1.138   06 Feb 1997 13:42:48   AMA
   Fix #34400#: MinPrtLine verhindert doppelten Druck von Tabellenzellenzeilen.

      Rev 1.137   28 Nov 1996 15:43:04   OS
   neu: schwarz drucken

      Rev 1.136   11 Nov 1996 09:57:56   MA
   ResMgr

      Rev 1.135   29 Oct 1996 15:40:10   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.134   27 Sep 1996 13:57:02   MA
   #29411# lange Tabellenzeilen drucken

      Rev 1.133   24 Aug 1996 17:10:38   JP
   svdraw.hxx entfernt

      Rev 1.132   17 Jul 1996 10:53:04   OS
   svdraw unter OS/2 ausserhalb der PCH

      Rev 1.131   16 Jul 1996 15:52:36   MA
   new: PrintPageBackground

      Rev 1.130   11 Jul 1996 12:51:12   HJS
   clooks

      Rev 1.129   27 Jun 1996 16:24:26   MA
   includes

      Rev 1.128   24 Jun 1996 15:47:14   AMA
   Fix #28641#: Detaillierte Ausgabe im Druckmonitor.

      Rev 1.127   21 Jun 1996 17:11:22   AMA
   Fix #28641#: Aktuelle Seitennummer beim Druck anzeigen.

      Rev 1.126   20 Jun 1996 16:33:06   MA
   JAKET-Einstellung erst nach dem SetVisArea

      Rev 1.125   11 Jun 1996 19:44:40   JP
   Bug #27584#: kein ULONG_MAX als Item verschicken -> eigene MessageId definieren

      Rev 1.124   21 Mar 1996 14:06:50   MA
   svhxx entfernt

      Rev 1.123   21 Mar 1996 14:04:46   OM
   Umstellung 311

      Rev 1.122   19 Mar 1996 14:23:32   HJS
   clooks

      Rev 1.121   07 Mar 1996 16:59:20   AMA
   New/Opt.: Progress-Anzeige beim Serienbrief

      Rev 1.120   05 Mar 1996 16:57:46   AMA
   New: SwPrtOptions fuer den Serienbriefdruck-Progress durchreichen

      Rev 1.119   05 Mar 1996 15:31:16   HJS
   clook-defs nur fuer win16

      Rev 1.118   29 Feb 1996 21:40:24   HJS
   clooks

      Rev 1.117   19 Jan 1996 09:46:32   OM
   CLOOKs entfernt

      Rev 1.116   14 Dec 1995 16:58:08   WKC
   MAC only: aText -> aStr

      Rev 1.115   13 Dec 1995 17:01:08   MA
   opt: International

      Rev 1.114   05 Dec 1995 14:28:48   MA
   fix: Repaint beim Druck fuer PreView per Invalidate/Update

      Rev 1.113   28 Nov 1995 21:22:48   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.112   24 Nov 1995 17:11:42   OM
   PCH->PRECOMPILED

      Rev 1.111   23 Nov 1995 15:41:46   MA
   fix/opt: blc Warnings

      Rev 1.110   14 Nov 1995 11:01:32   MA
   kommentar

      Rev 1.109   13 Nov 1995 12:18:44   MA
   chg: static -> lcl_

      Rev 1.108   09 Nov 1995 14:14:34   AMA
   Fix 22084(HACK): MultiSelection jetzt mit long-Ranges

      Rev 1.107   08 Nov 1995 12:18:38   AMA
   Set statt Change (301)

      Rev 1.106   03 Nov 1995 13:19:52   MA
   chg: Draft des Drawing

      Rev 1.105   01 Nov 1995 09:31:42   MA
   opt: String

      Rev 1.104   27 Oct 1995 15:06:48   MA
   new: Demo

      Rev 1.103   12 Oct 1995 12:59:24   JP
   Bug20370: Code umgestellt

      Rev 1.102   04 Sep 1995 19:13:42   JP
   ClientIter: spz. Pointer mit First/Next geben lassen

      Rev 1.101   17 Aug 1995 13:28:58   MA
   fix: Hack fuer Print jetzt richtig

      Rev 1.100   11 Aug 1995 18:53:50   MA
   Hack: Endlosschleife beim Druck

      Rev 1.99   11 Aug 1995 14:48:48   AMA
   Fix: Kopienanzahl beim Drucken quadriert

      Rev 1.98   08 Aug 1995 18:30:24   MA
   chg: USHORT anstelle Orientation uebergeben

      Rev 1.97   08 Aug 1995 14:34:44   MA
   ChgAllxxx in die ViewShell verlagert.

      Rev 1.96   27 Jul 1995 17:52:38   JP
   neu: IsAnyFieldInDoc - stelle fest, ob im Doc Felder existieren

      Rev 1.95   12 Jul 1995 14:10:16   AMA
   Fix: Druckereinstellungen zuruecksetzen

      Rev 1.94   11 Jul 1995 19:15:14   JP
   _SetGetExpFld: fuer bedingte Bereiche erweitert

      Rev 1.93   11 Jul 1995 08:54:44   AMA
   New: SetPaper, SetPaperSizeUser am Drucker aufrufen.

      Rev 1.92   06 Jul 1995 12:41:24   AMA
   New: PrintSelection funktioniert jetzt.

      Rev 1.91   05 Jul 1995 17:09:58   MA
   new: Flag fuer Controls nicht drucken

      Rev 1.90   04 Jul 1995 13:52:44   AMA
   New: PrintSelection, noch nicht scharf geschaltet.

      Rev 1.89   30 Jun 1995 16:32:54   AMA
   FIx 14789: Vor Progress-Status auf Druck-Abbruch reagieren

      Rev 1.88   26 Jun 1995 13:41:40   JP
   PrtOle2: vorm loeschen der Shell das Stack-Object loeschen

      Rev 1.87   23 Jun 1995 13:32:32   MA
   Zeichnungen nicht drucken

      Rev 1.86   07 Jun 1995 19:19:36   MA
   Reschedules bei Progress.

      Rev 1.85   10 May 1995 14:01:26   MA
   fix: Painten waehrend des Druckens.

      Rev 1.84   05 May 1995 19:36:20   AMA
   Umbau pProgress;

      Rev 1.83   04 May 1995 19:19:54   AMA
   Fix: PrtOle2

      Rev 1.82   04 May 1995 18:56:42   AMA
   Fix: Druckenmonitor

      Rev 1.81   03 May 1995 20:33:56   AMA
   Umbau: SfxProgress etc.

      Rev 1.80   28 Apr 1995 18:34:36   MA
   FlowFrm neu, compilierbar aber nicht mehr lauffaehig.

      Rev 1.79   05 Apr 1995 21:36:22   JP
   PrtOle2: neuen Parameter - Rectangle in dem auszugeben ist

      Rev 1.78   26 Feb 1995 15:04:50   JP
   auf die richtigen Shells casten

      Rev 1.77   25 Feb 1995 17:37:18   AMA
   PrintDialog -> MultiSelection

      Rev 1.76   16 Feb 1995 19:27:44   MA
   fix: Root aber keine Shell.

      Rev 1.75   16 Feb 1995 18:49:54   MA
   chg: PrtOle2 umgestellt.

*************************************************************************/


