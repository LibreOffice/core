/*************************************************************************
 *
 *  $RCSfile: vprint.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:48:12 $
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
#ifndef _SFX_PRNMON_HXX
#include <sfx2/prnmon.hxx>
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

// saves some settings from the draw view
class SwDrawViewSave
{
    String sLayerNm;
    SdrView* pDV;
    sal_Bool bPrintDraft;
    sal_Bool bPrintControls;
public:
    SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
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

void SetSwVisArea( ViewShell *pSh, const SwRect &rRect, BOOL bPDFExport )
{
    ASSERT( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->aVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    if (!bPDFExport)
        aPt += pSh->aPrtOffst;
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

    OutputDevice *pOut = bPDFExport ? pSh->GetOut() : pSh->GetPrt();
    MapMode aMapMode( pOut->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pOut->SetMapMode( aMapMode );
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
 *  Methode     :   void ViewShell::SetPrt( SfxPrinter *pNew, OutputDevice *pPDFOut )
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:22
 *  Aenderung   :
 ******************************************************************************/

void ViewShell::InitPrt( SfxPrinter *pPrt, OutputDevice *pPDFOut )
{
    //Fuer den Printer merken wir uns einen negativen Offset, der
    //genau dem Offset de OutputSize entspricht. Das ist notwendig,
    //weil unser Ursprung der linken ober Ecke der physikalischen
    //Seite ist, die Ausgaben (SV) aber den Outputoffset als Urstprung
    //betrachten.
    OutputDevice *pTmpDev = pPDFOut ? pPDFOut : (OutputDevice *) pPrt;
    if ( pTmpDev )
    {
        aPrtOffst = pPrt ? pPrt->GetPageOffset() : Point();

        aPrtOffst += pTmpDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pTmpDev->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pTmpDev->SetMapMode( aMapMode );
        pTmpDev->SetLineColor();
        pTmpDev->SetFillColor();
    }
    else
        aPrtOffst.X() = aPrtOffst.Y() = 0;

    if ( !pWin )
        pOut = pTmpDev;    //Oder was sonst?
}

void ViewShell::SetPrt( SfxPrinter *pNew )
{
    // If the reference device was the printer,
    // we have to set the new reference device
    GetDoc()->SetPrt( pNew, sal_True );
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
        lcl_SetState( *pProgress, 1, nStatMax, pStr, nMergeAct, nMergeCnt, 0,
                      1 );
        pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?

        aAction.SetProgress(pProgress);
    }

    pLayout->StartAllAction();
    for ( USHORT i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        if ( (bPrtJob && !pPrt->IsJobActive()) || Imp()->IsStopPrt())
            break;


        if( pProgress )
        {
            //HACK, damit die Anzeige sich nicht verschluckt.
            if ( i > nStatMax )
                nStatMax = i;
            lcl_SetState( *pProgress, i, nStatMax, pStr, nMergeAct, nMergeCnt, 0, i );
            pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
        }

        if ( bPrtJob && !pPrt->IsJobActive() ||
             Imp()->IsStopPrt())
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
    if (pProgress)
    {
        aAction.SetProgress(NULL);
    }

    pLayout->EndAllAction();
}

/******************************************************************************/

SwDoc * ViewShell::CreatePrtDoc( SfxPrinter* pPrt, SvEmbeddedObjectRef &rDocShellRef)
{
    ASSERT( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    // Wir bauen uns ein neues Dokument
    SwDoc *pPrtDoc = new SwDoc;
    pPrtDoc->AddLink();
    pPrtDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

    // Der Drucker wird uebernommen
    if (pPrt)
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
    return pPrtDoc;
}
SwDoc * ViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    ASSERT( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    // Wir bauen uns ein neues Dokument
//    SwDoc *pPrtDoc = new SwDoc;
//    pPrtDoc->AddLink();
//    pPrtDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

    // Der Drucker wird uebernommen
    //! Make a copy of it since it gets destroyed with the temporary document
    //! used for PDF export
    if (pPrt)
        pPrtDoc->SetPrt( new SfxPrinter(*pPrt) );

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
    return pPrtDoc;
}

/******************************************************************************
 *  Methode     :   void ViewShell::Prt( const SwPrtOptions& rOptions, SfxProgress& rProgress,
 *                                       OutputDevice *pPDFOut )
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 15:33
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


BOOL ViewShell::Prt( SwPrtOptions& rOptions, SfxProgress& rProgress,
                     OutputDevice *pPDFOut )
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Immer die Druckroutine in viewpg.cxx (fuer Seitenvorschau) mitpflegen!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    BOOL bStartJob = FALSE;

    //! Note: Since for PDF export of (multi-)selection a temporary
    //! document is created that contains only the selects parts,
    //! and thus that document is to printed in whole the,
    //! rOptions.bPrintSelection parameter will be false.
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

    SfxPrinter* pPrt = 0;   //!! will be 0 for PDF export !!
    if (pPDFOut)
        pPDFOut->Push();
    else
    {
        // wenn kein Drucker vorhanden ist, wird nicht gedruckt
        pPrt = GetPrt();
        if( !pPrt || !pPrt->GetName().Len() )
        {
            ASSERT( FALSE, "Drucken ohne Drucker?" );
            return bStartJob;
        }

        if( !rOptions.GetJobName().Len() && !pPrt->IsJobActive() )
            return bStartJob;

        // Einstellungen am Drucker merken
        SwPrtOptSave aPrtSave( pPrt );
    }
    OutputDevice *pPrtOrPDFOut = pPDFOut ? pPDFOut : (OutputDevice *) pPrt;

    // eine neue Shell fuer den Printer erzeugen
    ViewShell *pShell;
    SwDoc *pPrtDoc;

    //!! muss warum auch immer hier in diesem scope existieren !!
    //!! (hängt mit OLE Objekten im Dokument zusammen.)
    SvEmbeddedObjectRef aDocShellRef;

    // PDF export for (multi-)selection has already generated a temporary document
    // with the selected text. (see XRenderable implementation in unotxdoc.cxx)
    // Thus we like to go in the 'else' part here in that case.
    // Is is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here in the 'then' part.
    if (bSelection )
    {
        pPrtDoc = CreatePrtDoc( pPrt, aDocShellRef );

        // eine ViewShell darauf
        OutputDevice *pTmpDev = pPDFOut ? pPDFOut : 0;
        pShell = new ViewShell( *pPrtDoc, 0, pOpt, pTmpDev );
        pPrtDoc->SetRefForDocShell( 0 );
    }
    else
    {
        pPrtDoc = GetDoc();
        OutputDevice *pTmpDev = pPDFOut ? pPDFOut : 0;
        pShell = new ViewShell( *this, 0, pTmpDev );
    }

    {   //Zusaetzlicher Scope, damit die CurrShell vor dem zerstoeren der
        //Shell zurueckgesetzt wird.

    SET_CURR_SHELL( pShell );

    if (!pPDFOut)
    {
        Link aLnk = LINK(pShell->Imp(), SwViewImp, SetStopPrt);
        ((SfxPrintProgress &) rProgress).SetCancelHdl(aLnk);
    }


    //JP 01.02.99: das ReadOnly Flag wird NIE mitkopiert; Bug 61335
    if( pOpt->IsReadonly() )
        pShell->pOpt->SetReadonly( TRUE );

    // save options at draw view:
    SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

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

    // Some field types, can require a valid layout
    // (expression fields in tables). For these we do an UpdateFlds
    // here after calculation of the pages.
    pShell->UpdateFlds(TRUE);

    if( !  pShell->Imp()->IsStopPrt() &&
        (pPDFOut || rOptions.GetJobName().Len() || pPrt->IsJobActive()) )
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

            if (pPrt)
            {
                if( rOptions.IsPrintSingleJobs() && sJobName.Len() &&
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
                if (pPrt)
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
            BOOL bSetOrient   = FALSE;
            BOOL bSetPaperSz  = FALSE;
            BOOL bSetPaperBin = FALSE;
            BOOL bSetPrt      = FALSE;
            if (pPrt)
            {
                bSetOrient      = pPrt->HasSupport( SUPPORT_SET_ORIENTATION );
                bSetPaperSz     = pPrt->HasSupport( SUPPORT_SET_PAPERSIZE );
                bSetPaperBin    =  !rOptions.bPaperFromSetup &&
                                    pPrt->HasSupport( SUPPORT_SET_PAPERBIN );
                bSetPrt = bSetOrient || bSetPaperSz || bSetPaperBin;
            }

            if ( rOptions.nPrintPostIts != POSTITS_ONLY )
            {
                while( pStPage && !bStop )
                {
                    // Mag der Anwender noch ?
                    rProgress.Reschedule();

                    if (pPrt)
                    {
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
                    }
                    else if (pShell->Imp()->IsStopPrt())
                    {
                        bStop = TRUE;
                        break;
                    }

                    ::SetSwVisArea( pShell, pStPage->Frm(), 0 != pPDFOut );

                    //  wenn wir einen Umschlag drucken wird ein Offset beachtet
                    if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
                    {
                        aOldMapMode = pPrtOrPDFOut->GetMapMode();
                        Point aNewOrigin = pPrtOrPDFOut->GetMapMode().GetOrigin();
                        aNewOrigin += rOptions.aOffset;
                        MapMode aTmp( pPrtOrPDFOut->GetMapMode() );
                        aTmp.SetOrigin( aNewOrigin );
                        pPrtOrPDFOut->SetMapMode( aTmp );
                    }

                    BOOL bRightPg = pStPage->OnRightPage();
                    if( aMulti.IsSelected( nPageNo ) &&
                        ( (bRightPg && rOptions.bPrintRightPage) ||
                            (!bRightPg && rOptions.bPrintLeftPage) ) )
                    {
                        if ( bSetPrt )
                        {
                            // check for empty page
                            const SwPageFrm* pFormatPage = NULL;

                            // for empty pages, take the format of the partner
                            // page
                            if ( pStPage->IsEmptyPage() )
                            {
                                if ( bRightPg )
                                    pFormatPage = (SwPageFrm*)pStPage->GetNext();
                                else
                                    pFormatPage = (SwPageFrm*)pStPage->GetPrev();
                            }

                            if ( ! pFormatPage )
                                pFormatPage = pStPage;

                            if ( pLastPageDesc != pFormatPage->GetPageDesc() )
                            {
                                pLastPageDesc = pFormatPage->GetPageDesc();

                                const BOOL bLandScp =
                                        pFormatPage->GetPageDesc()->GetLandscape();

                                if( bSetPaperBin )      // Schacht einstellen.
                                    pPrt->SetPaperBin( pFormatPage->GetFmt()->
                                                       GetPaperBin().GetValue() );

                                if (bSetOrient )
                                 {
                                        // Orientation einstellen: Breiter als Hoch
                                        //  -> Landscape, sonst -> Portrait.
                                        if( bLandScp )
                                            pPrt->SetOrientation(ORIENTATION_LANDSCAPE);
                                        else
                                            pPrt->SetOrientation(ORIENTATION_PORTRAIT);
                                 }
                                 if (bSetPaperSz )
                                 {
                                        Size aSize = pStPage->Frm().SSize();

                                        // Let VCL decide which printer paper should be used for printing
                                        pPrt->SetPaperSizeUser( aSize );
                                 }
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
                            if( pPrt && !pPrt->IsJobActive() )
                            {
                                bStartJob = pPrt->StartJob( sJobName );
                                if( !bStartJob )
                                {
                                    nJobStartError = JOBSET_ERR_ERROR;
                                    continue;
                                }
                            }

                            pShell->InitPrt( pPrt, pPDFOut );

                            ::SetSwVisArea( pShell, pStPage->Frm(), 0 != pPDFOut );
                            nJobStartError = JOBSET_ERR_ISSTARTET;
                        }
                        // Bei Selektionsdruck wird ggf. die erste leere Seite ausgelassen
                        if( !bIgnoreEmptyPage || (0==(bIgnoreEmptyPage=TRUE)) ||
                            pStPage->Frm().Height() )
                        {
                            if (pPrt)
                                pPrt->StartPage();
                            pStPage->GetUpper()->Paint( pStPage->Frm() );
//                          SFX_APP()->SpoilDemoOutput( *pShell->GetOut(),
//                                                           pStPage->Frm().SVRect() );
                            if (pPrt)
                                pPrt->EndPage();
                        }
                        SwPaintQueue::Repaint();

                        // OD 03.03.2003 #103602# - printing contents of table,
                        // which doesn't fit on the page on an extra page, doesn't
                        // work as excepted for PDF export. Thus, do *not*
                        // perform this code for PDF export.
                        if ( !pPDFOut )
                        {
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
                                ::SetSwVisArea( pShell, aTmp, 0 != pPDFOut );
                                while ( nDiff > 0 )
                                {
                                    if (pPrt)
                                        pPrt->StartPage();

                                    //VisArea auf die Tabelle schummeln
                                    aNewVis.Pos().Y() += aNewVis.Height()+1;

                                    //Offset in den MapMode schummeln.
                                    MapMode aMap( pPrtOrPDFOut->GetMapMode() );
                                    Point aTmp( aMap.GetOrigin() );
                                    aTmp.Y() -= aNewVis.Height()+1;
                                    aMap.SetOrigin( aTmp );
                                    pPrtOrPDFOut->SetMapMode( aMap );

                                    /// OD 30.08.2002 #102450#
                                    /// determine color of page the table is on
                                    /// for <PaintLayer> method calls
                                    const Color aPageBackgrdColor =
                                            pStPage->GetDrawBackgrdColor();
                                    pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_INIT );
                                    SwTxtFrm::SetMinPrtLine( aNewVis.Pos().Y() );
                                    pFrm->PaintBaBo( aNewVis, pStPage, TRUE );
                                    if ( pShell->Imp()->HasDrawView() )
                                    {
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_LOCKLINES );
                                        /// OD 30.08.2002 #102450# - add 3rd parameter
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->GetHellId(),
                                                        aNewVis, &aPageBackgrdColor );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_PAINTLINES );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_UNLOCKLINES );
                                    }
                                    pFrm->Paint( aNewVis );
                                    if ( pShell->Imp()->HasDrawView() )
                                    {
                                        /// OD 30.08.2002 #102450# - add 3rd parameter
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->GetHeavenId(),
                                                    aNewVis, &aPageBackgrdColor );
                                        pShell->Imp()->PaintLayer( pShell->GetDoc()->GetControlsId(),
                                                                   aNewVis );
                                        pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_PAINTLINES );
                                    }
                                    pRoot->HackPrepareLongTblPaint( HACK_TABLEMODE_EXIT );
                                    SwTxtFrm::SetMinPrtLine( 0 );

//                                  SFX_APP()->SpoilDemoOutput( *pShell->GetOut(),
//                                                               aNewVis.SVRect() );
                                    if (pPrt)
                                        pPrt->EndPage();
                                    SwPaintQueue::Repaint();
                                    nDiff -= pBody->Prt().Height();
                                }
                            }
                        }
                        } // END OF If ( !pPDFOut ); OD 03.03.2003 #103602#

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
                        pPrtOrPDFOut->SetMapMode( aOldMapMode );

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

    if (bSelection )
    {
         // damit das Dokument nicht den Drucker mit ins Grab nimmt
        pPrtDoc->_SetPrt( NULL );

        if ( !pPrtDoc->RemoveLink() )
            delete pPrtDoc;
    }

    // restore settings of OutputDevicef
    if (pPDFOut)
        pPDFOut->Pop();

    return bStartJob;
}

/******************************************************************************
 *  Methode     :   PrtOle2()
 *  Beschreibung:
 *  Erstellt    :   PK 07.12.94
 *  Aenderung   :   MA 16. Feb. 95
 ******************************************************************************/



void ViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, SwPrtOptions& rOptions,
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
        pSh->PrepareForPrint( rOptions );
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


/******************************************************************************
 *  SwDrawViewSave
 *
 *  Saves some settings at the draw view
 ******************************************************************************/

SwDrawViewSave::SwDrawViewSave( SdrView* pSdrView )
    : pDV( pSdrView )
{
    if ( pDV )
    {
        bPrintDraft = pDV->IsLineDraftPrn();
        sLayerNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM("Controls" ) );
        bPrintControls = pDV->IsLayerPrintable( sLayerNm );
    }
}

SwDrawViewSave::~SwDrawViewSave()
{
    if ( pDV )
    {
        pDV->SetLineDraftPrn( bPrintDraft );
        pDV->SetFillDraftPrn( bPrintDraft );
        pDV->SetGrafDraftPrn( bPrintDraft );
        pDV->SetTextDraftPrn( bPrintDraft );
        pDV->SetLayerPrintable( sLayerNm, bPrintControls );
    }
}


// OD 09.01.2003 #i6467# - method also called for page preview
void ViewShell::PrepareForPrint(  const SwPrtOptions &rOptions )
{
    // Viewoptions fuer den Drucker setzen
    pOpt->SetGraphic ( TRUE == rOptions.bPrintGraphic );
    pOpt->SetTable   ( TRUE == rOptions.bPrintTable );
    pOpt->SetDraw    ( TRUE == rOptions.bPrintDraw  );
    pOpt->SetControl ( TRUE == rOptions.bPrintControl );
    pOpt->SetPageBack( TRUE == rOptions.bPrintPageBackground );
    pOpt->SetBlackFont( TRUE == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        FASTBOOL bDraw = rOptions.bPrintDraw;
        // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
        if ( !IsPreView() )
        {
            pDrawView->SetLineDraftPrn( !bDraw );
            pDrawView->SetFillDraftPrn( !bDraw );
            pDrawView->SetGrafDraftPrn( !bDraw );
            pDrawView->SetTextDraftPrn( !bDraw );
        }
        else
        {
            pDrawView->SetLineDraft( !bDraw );
            pDrawView->SetFillDraft( !bDraw );
            pDrawView->SetGrafDraft( !bDraw );
            pDrawView->SetTextDraft( !bDraw );
        }

        String sLayerNm;
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
        // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
        if ( !IsPreView() )
        {
            pDrawView->SetLayerPrintable( sLayerNm, rOptions.bPrintControl );
        }
        else
        {
            pDrawView->SetLayerVisible( sLayerNm, rOptions.bPrintControl );
        }
    }
}
