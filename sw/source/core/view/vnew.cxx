/*************************************************************************
 *
 *  $RCSfile: vnew.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 12:03:41 $
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

#include <so3/advise.hxx>
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // Zugriff auf TxtCache
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>        // SdrView
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif

/*************************************************************************
|*
|*  ViewShell::Init()
|*
|*  Letzte Aenderung    MA 14. Jun. 96
|*
|*************************************************************************/

void ViewShell::Init( const SwViewOption *pNewOpt )
{
    bDocSizeChgd = FALSE;

    // Wir gehen auf Nummer sicher:
    // Wir muessen die alten Fontinformationen wegschmeissen,
    // wenn die Druckeraufloesung oder der Zoomfaktor sich aendert.
    // Init() und Reformat() sind die sichersten Stellen.
    pFntCache->Flush( );

    // ViewOptions werden dynamisch angelegt
    if( !pOpt )
    {
        pOpt = new SwViewOption;

        // Ein ApplyViewOptions braucht nicht gerufen zu werden
        if( pNewOpt )
        {
            *pOpt = *pNewOpt;
            // Der Zoomfaktor muss eingestellt werden, weil in der CTOR-
            // phase aus Performancegruenden kein ApplyViewOptions gerufen wird.
            if( GetWin() && 100 != pOpt->GetZoom() )
            {
                MapMode aMode( pWin->GetMapMode() );
                const Fraction aNewFactor( pOpt->GetZoom(), 100 );
                aMode.SetScaleX( aNewFactor );
                aMode.SetScaleY( aNewFactor );
                pWin->SetMapMode( aMode );
            }
        }
    }

    SwDocShell* pDShell = pDoc->GetDocShell();
    pDoc->SetHTMLMode( 0 != ::GetHtmlMode( pDShell ) );

    // JP 02.02.99: Bug 61335 - Readonly-Flag an den ViewOptions setzen,
    //              bevor das Layout angelegt wird. Ansonsten muesste man
    //              nochmals durchformatieren!!
    if( pDShell && pDShell->IsReadOnly() )
        pOpt->SetReadonly( TRUE );

    if( GetPrt( TRUE ^ pDoc->IsBrowseMode() ) )
        InitPrt( GetPrt() );

    if( GetWin() )
    {
        pOpt->Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    //Layout erzeugen wenn es noch nicht vorhanden ist.
    SwRootFrm* pRoot = GetDoc()->GetRootFrm();
    if( !pRoot )
        GetDoc()->SetRootFrm( pRoot = new SwRootFrm( pDoc->GetDfltFrmFmt(), this ) );

    SizeChgNotify( pRoot->Frm().SSize() );
}

/*************************************************************************
|*
|*  ViewShell::ViewShell()  CTor fuer die erste Shell.
|*
|*  Letzte Aenderung    MA 29. Aug. 95
|*
|*************************************************************************/

ViewShell::ViewShell( SwDoc& rDocument, Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    : pDoc( &rDocument ),
    pOpt( 0 ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.GetPrt(TRUE)),
    pRef( 0 ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    aBrowseBorder()
{
    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = FALSE;
    bPaintWorks = bEnableSmooth = TRUE;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    pDoc->AddLink();
    pOutput = pOut;
    Init( pNewOpt );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !pOpt->IsHidden() );

    //In Init wird ein Standard-FrmFmt angelegt.
    if( !pDoc->IsUndoNoResetModified() )
        pDoc->ResetModified();

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( GetDoc()->GetDrawModel() || pOpt->IsGridVisible() )
        Imp()->MakeDrawView();
}

/*************************************************************************
|*
|*  ViewShell::ViewShell()  CTor fuer weitere Shells auf ein Dokument.
|*
|*  Letzte Aenderung    MA 29. Aug. 95
|*
|*************************************************************************/

ViewShell::ViewShell( ViewShell& rShell, Window *pWindow,
                        OutputDevice *pOutput, long nFlags ) :
    Ring( &rShell ),
    pDoc( rShell.GetDoc() ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->GetPrt(TRUE)),
    pRef( 0 ),
    pOpt( 0 ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    aBrowseBorder( rShell.GetBrowseBorder() )
{
    bPaintWorks = bEnableSmooth = TRUE;
    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = FALSE;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    SET_CURR_SHELL( this );

    pDoc->AddLink();
    BOOL bModified = pDoc->IsModified();

    pOutput = pOut;
    Init( rShell.GetViewOptions() );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !pOpt->IsHidden() );

    // in Init wird ein Standard-FrmFmt angelegt
    if( !bModified && !pDoc->IsUndoNoResetModified() )
        pDoc->ResetModified();

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( GetDoc()->GetDrawModel() || pOpt->IsGridVisible() )
        Imp()->MakeDrawView();
}

/******************************************************************************
|*
|*  ViewShell::~ViewShell()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 10. May. 95
|*
******************************************************************************/

ViewShell::~ViewShell()
{
    {
        SET_CURR_SHELL( this );
        bPaintWorks = FALSE;

        //Die Animierten Grafiken abschalten!
        if( pDoc )
        {
            SwNodes& rNds = pDoc->GetNodes();
            SwGrfNode *pGNd;

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                aIdx++;
                if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwClientIter aIter( *pGNd );
                        for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                            pFrm; pFrm = (SwFrm*)aIter.Next() )
                        {
                            ASSERT( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
                            ((SwNoTxtFrm*)pFrm)->StopAnimation( pOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( pOut );
        }

        delete pImp; //Erst loeschen, damit die LayoutViews vernichtet werden.

        if ( pDoc )
        {
            if( !pDoc->RemoveLink() )
                delete pDoc, pDoc = 0;
            else
                pDoc->GetRootFrm()->ResetNewLayout();
        }

        delete pOpt;

        //Format-Cache zurueckschrauben.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        //Ggf. aus der PaintQueue entfernen lassen
        SwPaintQueue::Remove( this );

        ASSERT( !nStartAction, "EndAction() pending." );
    }

    if ( pDoc )
        GetLayout()->DeRegisterShell( this );

    delete pRef;
}

const BOOL ViewShell::HasDrawView() const
{
    return Imp()->HasDrawView();
}

void ViewShell::MakeDrawView()
{
    Imp()->MakeDrawView( );
}

SdrView* ViewShell::GetDrawView()
{
    return Imp()->GetDrawView();
}

SdrView* ViewShell::GetDrawViewWithValidMarkList()
{
    SwDrawView* pDView = Imp()->GetDrawView();
    pDView->ValidateMarkList();
    return pDView;
}

/************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/09/28 11:30:29  jp
      remove old code of using no graphicobject

      Revision 1.1.1.1  2000/09/19 00:08:29  hr
      initial import

      Revision 1.132  2000/09/18 16:04:38  willem.vandorp
      OpenOffice header added.

      Revision 1.131  2000/07/06 16:57:25  jp
      DTOR: dont laod the graphic to stop the animation

      Revision 1.130  2000/07/04 15:21:42  tl
      XHyphenator1 => XHyphenator

      Revision 1.129  2000/03/23 06:48:25  os
      UNOIII

      Revision 1.128  2000/02/16 20:55:52  tl
      #72219# Locale Umstellung

      Revision 1.127  1999/11/23 08:21:54  os
      header corrected

      Revision 1.126  1999/11/19 16:40:19  os
      modules renamed

      Revision 1.125  1999/10/25 19:08:51  tl
      ongoing ONE_LINGU implementation

      Revision 1.124  1999/04/09 15:50:26  MA
      #64467# EndAction und VirDev


      Rev 1.123   09 Apr 1999 17:50:26   MA
   #64467# EndAction und VirDev

      Rev 1.122   02 Mar 1999 16:07:32   AMA
   Fix #62568#: Invalidierungen so sparsam wie moeglich, so gruendlich wie noetig

      Rev 1.121   22 Feb 1999 08:35:22   MA
   1949globale Shell entsorgt, Shells am RootFrm

      Rev 1.120   02 Feb 1999 13:15:20   JP
   Bug #61335#: im Init vorm anlegen des Layouts ReadOnly-Status feststellen

      Rev 1.119   06 Oct 1998 12:44:12   JP
   Bug #57153#: neu GetDrawViewWithValidMarkList

      Rev 1.118   05 Oct 1998 17:44:00   MA
   #57380# Reset fuer NewLayout

      Rev 1.117   09 Sep 1998 14:59:22   MA
   #56106# Array aus im CoreInit initialisieren

      Rev 1.116   09 Sep 1998 11:47:46   MA
   #56106# CurrShell-Verwaltung aufwendiger

      Rev 1.115   27 Jul 1998 16:56:00   JP
   Bug #45529#: LoadStyles muss ggfs. das Zuruecksetzen des ModifiedFlags verhindern

      Rev 1.114   19 Jul 1998 17:55:24   JP
   Bug #52312#: PagePreviewFlag wird im CTOR benoetigt

      Rev 1.113   24 Jun 1998 18:45:20   MA
   DataChanged fuer ScrollBar und Retouche, Retouche ganz umgestellt

      Rev 1.112   19 May 1998 10:12:40   TJ
   include wg. ice

      Rev 1.111   29 Apr 1998 09:31:30   MA
   RetoucheBrush -> RetoucheColor

      Rev 1.110   27 Apr 1998 15:09:10   MA
   ein paar sv2vcl

      Rev 1.109   23 Apr 1998 09:33:36   MA
   #49472# Optional ein Outdev durchschleusen

      Rev 1.108   26 Mar 1998 17:58:50   MA
   Wechsel fuer Drucker/Jobsetup jetzt vollstaendig und am Dokument

      Rev 1.107   20 Nov 1997 12:40:38   MA
   includes

      Rev 1.106   17 Nov 1997 09:47:28   JP
   Umstellung Numerierung

      Rev 1.105   03 Nov 1997 13:07:28   MA
   precomp entfernt

      Rev 1.104   22 Oct 1997 14:28:46   MA
   fix: Brush mit Style NULL fuer Drucker-Retouche

      Rev 1.103   13 Oct 1997 15:54:28   JP
   pNext vom Ring wurde privat; zugriff ueber GetNext()

      Rev 1.102   09 Oct 1997 16:05:20   JP
   Umstellung NodeIndex/-Array/BigPtrArray

*************************************************************************/

