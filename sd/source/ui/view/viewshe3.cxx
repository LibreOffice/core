/*************************************************************************
 *
 *  $RCSfile: viewshe3.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-06 16:57:00 $
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

#include <stl/utility>
#include <stl/vector>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#ifdef VCL
#ifndef _SV_PRINTDLG_HXX
#include <svtools/printdlg.hxx>
#endif
#endif

#ifndef _SVX_PRTQRY_HXX
#include <svx/prtqry.hxx>
#endif

#ifndef _SVDOPAGE_HXX //autogen
#include <svx/svdopage.hxx>
#endif

#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDETC_HXX //autogen
#include <svx/svdetc.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef _SFX_MISCCFG_HXX
#include <sfx2/misccfg.hxx>
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif

#include "sdattr.hxx"
#include "viewshel.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "unslprms.hxx"

#include "slidchld.hxx"
#include "slidechg.hxx"

#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#include "prevchld.hxx"
#include "preview.hxx"

#include "sdwindow.hxx"
#include "docshell.hxx"
#include "drviewsh.hxx"
#include "outlnvsh.hxx"
#include "frmview.hxx"
#include "prntopts.hxx"
#include "printdlg.hxx"
#include "optsitem.hxx"

#include "slidvish.hxx"
#include "sdoutl.hxx"

#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif

using namespace ::com::sun::star;

/*************************************************************************
|*
|* EffekteWindow updaten
|*
\************************************************************************/

void SdViewShell::UpdateSlideChangeWindow()
{
    USHORT nId = SdSlideChangeChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        SdSlideChangeWin* pSlideChangeWin = (SdSlideChangeWin*) pWindow->GetWindow();
        if( pSlideChangeWin && pSlideChangeWin->IsUpdateMode() )
        {
            SdPage* pPage      = NULL;
            USHORT  nNoOfPages = pDoc->GetSdPageCount(PK_STANDARD);
            USHORT  nPage;
            BOOL    bPageSelected = FALSE;

            // falls nichts selektiert ist, machen wir das schnell selbst
            for (USHORT i = 0; i < nNoOfPages; i++)
            {
                pPage = pDoc->GetSdPage(i, PK_STANDARD);
                if (pPage->IsSelected())
                {
                    bPageSelected = TRUE;
                    break;
                }
            }
            /*
            if (!(pPage->IsSelected()))
            {
                pPage = pDoc->GetSdPage(0, PK_STANDARD);
                pDoc->SetSelected(pPage, TRUE);

                // ist Selektion sichtbar?
                if( this->ISA(SdSlideViewShell) )
                {
                    ((SdSlideView*)pView)->DrawSelectionRect((USHORT)0);
                }
            } */

            // ItemSet fuer Dialog (jetzt Window) fuellen
            SfxItemSet aSet(pDoc->GetPool(), ATTR_DIA_START, ATTR_DIA_END);

            if( bPageSelected )
            {
                // jetzt werden die Seitenattribute "per Hand" gemerged
                BOOL bSameEffect  = TRUE;         // Annahme: alle Seiten haben die
                BOOL bSameSpeed   = TRUE;         // gleichen Attribute
                BOOL bSameTime    = TRUE;
                BOOL bSameChange  = TRUE;
                BOOL bSameSoundOn = TRUE;
                BOOL bSameSound   = TRUE;

                presentation::FadeEffect eLastEffect;
                FadeSpeed  eLastSpeed;
                ULONG      nLastTime;
                PresChange eLastChange;
                BOOL       bLastSoundOn;
                String     aLastSound;

                // Attribute der ersten selektierten Seite
                for (nPage = 0; nPage < nNoOfPages; nPage++)
                {
                    pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
                    if (pPage->IsSelected())
                    {
                        eLastEffect  = pPage->GetFadeEffect();
                        eLastSpeed   = pPage->GetFadeSpeed();
                        nLastTime    = pPage->GetTime();
                        eLastChange  = pPage->GetPresChange();
                        bLastSoundOn = pPage->IsSoundOn();
                        aLastSound   = pPage->GetSoundFile();
                        break;
                    }
                }

                // mit den anderen selektierten Seiten vergleichen
                for (; nPage < nNoOfPages; nPage++)
                {
                    pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
                    if (pPage->IsSelected())
                    {
                        if (eLastEffect  != pPage->GetFadeEffect()) bSameEffect  = FALSE;
                        if (eLastSpeed   != pPage->GetFadeSpeed())  bSameSpeed   = FALSE;
                        if (nLastTime    != pPage->GetTime())       bSameTime    = FALSE;
                        if (eLastChange  != pPage->GetPresChange()) bSameChange  = FALSE;
                        if (bLastSoundOn != pPage->IsSoundOn())     bSameSoundOn = FALSE;
                        if (aLastSound   != pPage->GetSoundFile())  bSameSound   = FALSE;
                    }
                }

                // das Set besetzen
                if (bSameEffect)  aSet.Put( DiaEffectItem( eLastEffect ) );
                else              aSet.InvalidateItem( ATTR_DIA_EFFECT );

                if (bSameSpeed)   aSet.Put( DiaSpeedItem( eLastSpeed ) );
                else              aSet.InvalidateItem( ATTR_DIA_SPEED );

                if (bSameChange)  aSet.Put( DiaAutoItem( eLastChange ) );
                else              aSet.InvalidateItem( ATTR_DIA_AUTO );

                if (bSameTime)    aSet.Put( DiaTimeItem( nLastTime ) );
                else              aSet.InvalidateItem( ATTR_DIA_TIME );

                if (bSameSoundOn) aSet.Put(SfxBoolItem(ATTR_DIA_SOUND, bLastSoundOn));
                else              aSet.InvalidateItem(ATTR_DIA_SOUND);

                if (bSameSound && aLastSound.Len() > 0)
                {
                    aSet.Put(SfxStringItem(ATTR_DIA_SOUNDFILE, aLastSound));
                }
                else
                    aSet.InvalidateItem(ATTR_DIA_SOUNDFILE);

                pSlideChangeWin->EnableAssignButton();
            }
            else // keine selektierte Seite
            {
                aSet.InvalidateItem( ATTR_DIA_EFFECT );
                aSet.InvalidateItem( ATTR_DIA_SPEED );
                aSet.InvalidateItem( ATTR_DIA_AUTO );
                aSet.InvalidateItem( ATTR_DIA_TIME );
                aSet.InvalidateItem( ATTR_DIA_SOUND );
                aSet.InvalidateItem( ATTR_DIA_SOUNDFILE );

                pSlideChangeWin->EnableAssignButton( FALSE );
            }
            pSlideChangeWin->Update( aSet );
        }
    }
}

/*************************************************************************
|*
|* Vom EffekteWindow zuweisen
|*
\************************************************************************/

void SdViewShell::AssignFromSlideChangeWindow()
{
    USHORT nId = SdSlideChangeChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        SdSlideChangeWin* pSlideChangeWin = (SdSlideChangeWin*) pWindow->GetWindow();
        if( pSlideChangeWin )
        {
            SdPage* pPage      = NULL;
            USHORT  nNoOfPages = pDoc->GetSdPageCount(PK_STANDARD);
            USHORT  nPage;


            SfxItemSet aSet(pDoc->GetPool(), ATTR_DIA_START, ATTR_DIA_END );
            pSlideChangeWin->GetAttr( aSet );

            // Auswertung des ItemSets

            // Undo Gruppe erzeugen
            SdUndoGroup* pUndoGroup = new SdUndoGroup(pDoc);
            String aComment(SdResId(STR_UNDO_SLIDE_PARAMS));
            pUndoGroup->SetComment(aComment);

            for (nPage = 0; nPage < nNoOfPages; nPage++)
            {
                pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
                if (pPage->IsSelected())
                {
                    // alte Attribute fuer UndoAction merken
                    FadeSpeed  eOldFadeSpeed  = pPage->GetFadeSpeed();
                    presentation::FadeEffect eOldFadeEffect = pPage->GetFadeEffect();
                    PresChange eOldChange     = pPage->GetPresChange();
                    UINT32     nOldTime       = pPage->GetTime();
                    BOOL       bOldSoundOn    = pPage->IsSoundOn();
                    String     aOldSoundFile  = pPage->GetSoundFile();

                    if (aSet.GetItemState(ATTR_DIA_EFFECT) == SFX_ITEM_SET)
                    {
                       presentation::FadeEffect eOldEffect = pPage->GetFadeEffect();
                       presentation::FadeEffect eNewEffect = (presentation::FadeEffect)((const DiaEffectItem&)
                                                   aSet.Get(ATTR_DIA_EFFECT)).
                                                       GetValue();

                        pPage->SetFadeEffect((presentation::FadeEffect)((const DiaEffectItem&)
                                aSet.Get(ATTR_DIA_EFFECT)).GetValue());

                        // ist es eine Aenderung und kann man sie sehen?
                        if (((eOldEffect != presentation::FadeEffect_NONE &&
                              eNewEffect == presentation::FadeEffect_NONE)    ||
                             (eOldEffect == presentation::FadeEffect_NONE &&
                              eNewEffect != presentation::FadeEffect_NONE))  &&
                            (this->ISA(SdSlideViewShell)))
                        {
                            pView->InvalidateAllWin(((SdSlideView*)pView)->
                                                          GetFadeIconArea(nPage));
                        }
                    }

                    if (aSet.GetItemState(ATTR_DIA_SPEED) == SFX_ITEM_SET)
                        pPage->SetFadeSpeed((FadeSpeed)((const DiaSpeedItem&)
                                aSet.Get(ATTR_DIA_SPEED)).GetValue());

                    if (aSet.GetItemState(ATTR_DIA_TIME) == SFX_ITEM_SET)
                        pPage->SetTime(((DiaTimeItem&)
                                aSet.Get(ATTR_DIA_TIME)).GetValue());

                    if (aSet.GetItemState(ATTR_DIA_AUTO) == SFX_ITEM_SET)
                        pPage->SetPresChange(((DiaAutoItem&)
                                aSet.Get(ATTR_DIA_AUTO)).GetValue());

                    if (aSet.GetItemState(ATTR_DIA_SOUND) == SFX_ITEM_SET)
                        pPage->SetSound(((SfxBoolItem&)
                                aSet.Get(ATTR_DIA_SOUND)).GetValue());

                    if (aSet.GetItemState(ATTR_DIA_SOUNDFILE) == SFX_ITEM_SET)
                    {
                        String aTemp(((SfxStringItem&)aSet.Get(ATTR_DIA_SOUNDFILE)).GetValue());
                        pPage->SetSoundFile(aTemp);
                    }

                    // Undo-Action erzeugen und in Gruppe stellen
                    SdUndoAction* pUndoAction = new SlideParametersUndoAction
                                    (pDoc, pPage,
                                     eOldFadeSpeed,  pPage->GetFadeSpeed(),
                                     eOldFadeEffect, pPage->GetFadeEffect(),
                                     eOldChange,     pPage->GetPresChange(),
                                     nOldTime,       pPage->GetTime(),
                                     bOldSoundOn,    pPage->IsSoundOn(),
                                     aOldSoundFile,  pPage->GetSoundFile());
                    pUndoGroup->AddAction(pUndoAction);
                }
            }

            // Undo Gruppe dem Undo Manager uebergeben
            GetViewFrame()->GetObjectShell()->GetUndoManager()->
                        AddUndoAction(pUndoGroup);

            /***************************************************************
            |* ggfs. in Preview anzeigen
            \**************************************************************/
            SfxChildWindow* pPreviewChildWindow =
                GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
            if (pPreviewChildWindow)
            {
                SdPreviewWin* pPreviewWin =
                    (SdPreviewWin*)pPreviewChildWindow->GetWindow();
                if (pPreviewWin && pPreviewWin->GetDoc() == pDoc)
                {
                    pPreviewWin->AnimatePage();
                }
            }


            static USHORT SidArray[] = {
                            SID_DIA_EFFECT,
                            SID_DIA_SPEED,
                            SID_DIA_AUTO,
                            SID_DIA_TIME,
                            0 };

            GetViewFrame()->GetBindings().Invalidate( SidArray );

            // Model geaendert
            pDoc->SetChanged();
        }
    }
}

/*************************************************************************
|*
|* aktuellen Drucker des Dokuments zurueckgeben
|*
\************************************************************************/
SfxPrinter* __EXPORT SdViewShell::GetPrinter(BOOL bCreate)
{
    return ( pDocSh->GetPrinter(bCreate) );
}

/*************************************************************************
|*
|* neuen Drucker fuer die Applikation setzen
|*
\************************************************************************/
USHORT __EXPORT SdViewShell::SetPrinter(SfxPrinter* pNewPrinter,
                                            USHORT nDiffFlags)
{
    pDocSh->SetPrinter(pNewPrinter);

    if ( (nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ||
          nDiffFlags & SFX_PRINTER_CHG_SIZE) && pNewPrinter  )
    {
        MapMode aMap = pNewPrinter->GetMapMode();
        aMap.SetMapUnit(MAP_100TH_MM);
        MapMode aOldMap = pNewPrinter->GetMapMode();
        pNewPrinter->SetMapMode(aMap);
        Size aNewSize = pNewPrinter->GetOutputSize();

        WarningBox aWarnBox(pWindow, (WinBits)(WB_YES_NO | WB_DEF_YES),
                            String(SdResId(STR_SCALE_OBJS_TO_PAGE)));
        BOOL bScaleAll = (aWarnBox.Execute() == RET_YES);

        if( this->ISA( SdDrawViewShell ) )
        {
            SetPageSizeAndBorder( ( (SdDrawViewShell*)this )->GetPageKind(),
                        aNewSize, -1,-1,-1,-1, bScaleAll, TRUE,
                        pNewPrinter->GetOrientation() );
        }

        pNewPrinter->SetMapMode(aOldMap);
    }

    return 0;
}

/*************************************************************************
|*
|* Druckdialog erzeugen; virtuell, wird vom SFX aufgerufen
|*
\************************************************************************/
PrintDialog* __EXPORT SdViewShell::CreatePrintDialog(Window *pParent)
{
    PrintDialog* pDlg;

    pDlg = new PrintDialog(pParent );

    if( !this->ISA( SdOutlineViewShell ) )
    {

        if( this->ISA( SdDrawViewShell ) )
        {
            pDlg->SetRangeText( UniString::CreateFromInt32(( (SdDrawViewShell*)this )->GetCurPageId() ));
        }
        else //if( this->ISA( SdSlideViewShell ) )
        {
            String aStrRange( ( (SdSlideViewShell*)this )->GetPageRangeString() );
            if( aStrRange.Len() )
            {
                pDlg->SetRangeText( aStrRange );
                // According #79749 always check PRINTDIALOG_ALL
                // pDlg->CheckRange( PRINTDIALOG_RANGE );
            }
        }
    }
    else
    {
        String aStrRange( ( (SdOutlineViewShell*)this)->GetPageRangeString() );
        if( aStrRange.Len() )
        {
            pDlg->SetRangeText( aStrRange );
            // According #79749 always check PRINTDIALOG_ALL
            // pDlg->CheckRange( PRINTDIALOG_RANGE );
        }
    }
    pDlg->EnableRange( PRINTDIALOG_RANGE );
    pDlg->EnableRange( PRINTDIALOG_ALL );
    pDlg->EnableCollate();

    if( this->ISA( SdDrawViewShell ) && pView->HasMarkedObj() )
    {
        pDlg->EnableRange( PRINTDIALOG_SELECTION );
        // According #79749 always check PRINTDIALOG_ALL
        // pDlg->CheckRange( PRINTDIALOG_SELECTION );
    }

    return pDlg;
}

/*************************************************************************
|*
|* Factory Methode Tabpage Zusaetze (vom Druckdialog)
|*
\************************************************************************/
SfxTabPage* __EXPORT SdViewShell::CreatePrintOptionsPage( Window *pParent,
                                                 const SfxItemSet &rOptions )
{
    BOOL bHide = FALSE;
    DocumentType eDocType = pDoc->GetDocumentType();
    if( eDocType == DOCUMENT_TYPE_DRAW )
        bHide = TRUE;
    SfxTabPage* pPage = new SdPrintOptions( pParent, rOptions, bHide );

    return( pPage );
}


/*************************************************************************
|*
|* Vorbereitung fuers Drucken; virtuell, wird vom SFX aufgerufen
|*
\************************************************************************/
void SdViewShell::PreparePrint(PrintDialog* pPrintDialog)
{
    SfxPrinter* pPrinter = GetPrinter(TRUE);

    if (!pPrinter)
        return;

    const SfxItemSet& rOptions = pPrinter->GetOptions();
    SdOptionsPrintItem* pPrintOpts = NULL;

    if (rOptions.GetItemState( ATTR_OPTIONS_PRINT, FALSE,
        (const SfxPoolItem**) &pPrintOpts) != SFX_ITEM_SET)
    {
        pPrintOpts = NULL;
    }

    // Einstellungen der ersten zu druckenden Seite setzen

    if (pPrintOpts)
    {
        if ( pPrintOpts->IsHandout() )
        {
            // Handzettel
            SdPage* pPage = pDoc->GetSdPage(0, PK_HANDOUT);

            // Papierschacht
            if (!pPrintOpts->IsPaperbin()) // Drucken NICHT aus Druckereinstellung
            {
                pPrinter->SetPaperBin(pPage->GetPaperBin());
            }

            SdPage* pMaster = (SdPage*) pPage->GetMasterPage(0);
            pPrinter->SetOrientation(pMaster->GetOrientation());
        }
        else if ( pPrintOpts->IsDraw() || pPrintOpts->IsNotes() )
        {
            // Standard- oder Notizseiten
            if( !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
            {
                PageKind ePageKind = PK_NOTES;

                if (pPrintOpts->IsDraw())
                {
                    ePageKind = PK_STANDARD;
                }

                SdPage* pPage = pDoc->GetSdPage(0, ePageKind);
                pPrinter->SetPaperBin(pPage->GetPaperBin());

                Orientation eOrientation = ORIENTATION_PORTRAIT;

                if ( !pPrintOpts->IsBooklet() )
                {
                    eOrientation = pPage->GetOrientation();
                }
                else
                {
                    Size aPageSize(pPage->GetSize());

                    if( aPageSize.Width() < aPageSize.Height() )
                        eOrientation = ORIENTATION_LANDSCAPE;
                }

                pPrinter->SetOrientation(eOrientation);
            }
        }
    }
}


/*************************************************************************
|*
|* Drucken; virtuell, wird vom SFX aufgerufen
|*
\************************************************************************/

ErrCode SdViewShell::DoPrint( SfxPrinter *pPrinter, PrintDialog *pPrintDialog, BOOL bSilent )
{
    const SdrMarkList& rMarkList = pView->GetMarkList();

    // retrieve range of marked pages if we are in the slideview
    String sNewPageRange;
    if( ISA( SdSlideViewShell ) )
        sNewPageRange = ((SdSlideViewShell*)this)->GetPageRangeString();

    // retrieve range of marked pages if we are in the outlineview
    if( ISA( SdOutlineViewShell ) )
        sNewPageRange = ((SdOutlineViewShell*)this)->GetPageRangeString();

    bPrintDirectSelected = FALSE;

    if ( !pPrintDialog && !bSilent && rMarkList.GetMarkCount() || sNewPageRange.Len() )
    {
        SvxPrtQryBox aQuery( pWindow );
        short nBtn = aQuery.Execute();

        if ( nBtn == RET_CANCEL )
            return ERRCODE_IO_ABORT;

        if ( nBtn == RET_OK )
        {
            bPrintDirectSelected = TRUE;

            sPageRange.Erase();
            if( sNewPageRange.Len() )
                sPageRange = sNewPageRange;
        }
    }

    FuSlideShow *pShow = pFuSlideShow;

    //  SfxViewShell::DoPrint calls Print (after StartJob etc.)
    ErrCode nRet = SfxViewShell::DoPrint( pPrinter, pPrintDialog, bSilent );

    bPrintDirectSelected = FALSE;

    return nRet;
}

USHORT __EXPORT SdViewShell::Print(SfxProgress& rProgress, PrintDialog* pDlg)
{
    SfxPrinter* pPrinter = GetPrinter(TRUE);

    if ( !pPrinter )
        return 0;

    MapMode aMap( pPrinter->GetMapMode() );
    MapMode aOldMap( aMap );
    aMap.SetMapUnit(MAP_100TH_MM);
    pPrinter->SetMapMode(aMap);

    // Druckerschacht muss sich gemerkt werden, da dieser sonst von uns
    // ueberschrieben wird
    USHORT nOldPaperBin = pPrinter->GetPaperBin();

    Outliner& rOutliner = pDoc->GetDrawOutliner();
    ULONG nOldCntrl = rOutliner.GetControlWord();
    ULONG nCntrl = nOldCntrl;
    nCntrl |= EE_CNTRL_NOREDLINES;
    nCntrl &= ~EE_CNTRL_MARKFIELDS;
    nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rOutliner.SetControlWord(nCntrl);

    // Pruefen des Seitenformates und ggfs. Dialog hochbringen
    const SfxItemSet&   rOptions = pPrinter->GetOptions();
    SdOptionsPrintItem* pPrintOpts = NULL;
    BOOL bScalePage = TRUE;
    BOOL bTilePage = FALSE;
    BOOL bPrintBooklet = FALSE;

    if( rOptions.GetItemState( ATTR_OPTIONS_PRINT, FALSE,
                (const SfxPoolItem**) &pPrintOpts) == SFX_ITEM_SET )
    {
        bScalePage = pPrintOpts->IsPagesize();
        bPrintBooklet = pPrintOpts->IsBooklet();
        pPrintOpts->SetCutPage( FALSE );
    }
    else
        pPrintOpts = NULL;

    SdPage* pPage = pDoc->GetSdPage( 0, PK_STANDARD );
    Size aPageSize( pPage->GetSize() );
    Size aPrintSize( pPrinter->GetOutputSize() );
    long aPageWidth   = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
    long aPageHeight  = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
    long aPrintWidth  = aPrintSize.Width();
    long aPrintHeight = aPrintSize.Height();

    // Hier wird noch nicht unterschieden zwischen Landscape/Portrait !
    // Wie das genau auszusehen hat, sollte mal geklaert werden !
    // Landscape / Portrait wird automatisch umgeschaltet

    // Dialog
    USHORT nRet = RET_OK;
    if( !bScalePage && !bTilePage && !bPrintBooklet &&
        ( ( aPageWidth > aPrintWidth || aPageHeight > aPrintHeight ) &&
          ( aPageWidth > aPrintHeight || aPageHeight > aPrintWidth ) ) )
    {
        SdPrintDlg aDlg( pWindow );
        nRet = aDlg.Execute();
        if( nRet == RET_OK )
        {
            USHORT nOption = aDlg.GetAttr();

            if( nOption == 1 )
                pPrintOpts->SetPagesize();

            // ( nOption == 2 ) ist der Default

            if( nOption == 3 )
                pPrintOpts->SetCutPage();
        }
    }
    if( nRet == RET_CANCEL )
    {
        pPrinter->SetMapMode(aOldMap);
        return 0;
    }

    // Wenn wir im Gliederungsmodus sind, muss das Model auf Stand gebracht werden
    if( this->ISA( SdOutlineViewShell ) )
        ( (SdOutlineViewShell*)this )->PrepareClose(FALSE, FALSE);

    // Basisklasse rufen, um Basic anzusprechen
    SfxViewShell::Print( rProgress, pDlg );

    // Setzen des Textes des Druckmonitors
    rProgress.SetText( String( SdResId( STR_STATSTR_PRINT ) ) );

    PrintDialogRange    eOption;
    MultiSelection      aPrintSelection;
    String      aTimeDateStr;
    Font        aTimeDateFont(FAMILY_SWISS, Size(0, 423));
    PageKind    ePageKind = PK_STANDARD;
    USHORT      nPage, nPageMax;
    USHORT      nTotal, nCopies;
    USHORT      nPrintCount = 0;
    USHORT      nProgressOffset = 0;
    USHORT      nCollateCopies = 1;
    BOOL        bPrintMarkedOnly = FALSE;
    BOOL        bPrintOutline = FALSE;
    BOOL        bPrintHandout = FALSE;
    BOOL        bPrintDraw = FALSE;
    BOOL        bPrintNotes = FALSE;

    Orientation eOldOrientation = pPrinter->GetOrientation();

    if( pPrintOpts )
    {
        if ( pPrintOpts->IsDate() )
        {
            aTimeDateStr += SdrGlobalData::pLocaleData->getDate( Date() );
            aTimeDateStr += (sal_Unicode)' ';
        }

        if ( pPrintOpts->IsTime() )
            aTimeDateStr += SdrGlobalData::pLocaleData->getTime( Time(), FALSE, FALSE );

        if ( pPrintOpts->IsOutline() )
            bPrintOutline = TRUE;
        if ( pPrintOpts->IsHandout() )
            bPrintHandout = TRUE;
        if ( pPrintOpts->IsDraw() )
            bPrintDraw = TRUE;
        if ( pPrintOpts->IsNotes() )
        {
            bPrintNotes = TRUE;
            ePageKind = PK_NOTES;
        }

        SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();
        pPrintOpts->SetWarningPrinter( pMisc->IsNotFoundWarning() );
        pPrintOpts->SetWarningSize( pMisc->IsPaperSizeWarning() );
        pPrintOpts->SetWarningOrientation( pMisc->IsPaperOrientationWarning() );

/// Neu
        // Ausgabequalitaet setzen
        UINT16 nQuality = pPrintOpts->GetOutputQuality();

        ULONG nMode = DRAWMODE_DEFAULT;
        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT |
                    DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL |
                    DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

        pPrinter->SetDrawMode( nMode );
/// Neu

    }
    else
        bPrintDraw = TRUE;

    if ( pDlg )
    {
        eOption = pDlg->GetCheckedRange();
        if ( eOption == PRINTDIALOG_SELECTION )
            bPrintMarkedOnly = TRUE;
    }
    else
        // Bei PrintDirect wird gesamtes Dokument gedruckt
        eOption = PRINTDIALOG_ALL;

    // #72527 If we are in PrintDirect and any objects
    // are selected, then a dialog (see SdViewShell::DoPrint)
    // ask whether the total document should be printed
    // or only the selected objects. If only the selected
    // object, then the flag bPrintDirectSelected is TRUE
    if( bPrintDirectSelected )
    {
        eOption = PRINTDIALOG_SELECTION;
        bPrintMarkedOnly = TRUE;
    }

    nPageMax = pDoc->GetSdPageCount(ePageKind);
    aPrintSelection.SetTotalRange(Range(1, nPageMax));

    switch ( eOption )
    {
        case PRINTDIALOG_ALL:
            aPrintSelection.Select(Range(1, nPageMax));
            break;

        case PRINTDIALOG_RANGE:
            aPrintSelection = MultiSelection(pDlg->GetRangeText());
            break;

        default:
            if( this->ISA( SdDrawViewShell ) )
                aPrintSelection.Select( ( (SdDrawViewShell*)this )->GetCurPageId() );
            else
            {
                if( sPageRange.Len() )
                    aPrintSelection = MultiSelection( sPageRange );
                else
                    aPrintSelection.Select(Range(1, nPageMax));
            }
            break;
    }
    nPage = Min(nPageMax, (USHORT) aPrintSelection.FirstSelected());
    if ( nPage > 0 )
        nPage--;
    nPageMax = Min(nPageMax, (USHORT) aPrintSelection.LastSelected());

    if( bPrintOutline )
        nPrintCount++;
    if( bPrintHandout )
        nPrintCount++;
    if( bPrintDraw )
        nPrintCount++;
    if( bPrintNotes )
        nPrintCount++;

    nCopies = (pDlg ? pDlg->GetCopyCount() : 1);

    USHORT nSelectCount = (USHORT) aPrintSelection.GetSelectCount();
    nTotal = nSelectCount * nCopies * nPrintCount;

    if( pDlg && pDlg->IsCollateEnabled() && pDlg->IsCollateChecked() )
        nCollateCopies = nCopies;

    for( USHORT n = 1; n <= nCollateCopies; n++ )
    {
        if ( bPrintOutline )
        {
            // siehe unten in PrintOutline()
            pPrinter->SetPaperBin( nOldPaperBin );

            PrintOutline(*pPrinter, rProgress, aPrintSelection,
                          aTimeDateStr, aTimeDateFont, pPrintOpts,
                          nPage, nPageMax,
                          nCollateCopies > 1 ? 1 : nCopies,
                          nProgressOffset, nTotal );
            nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
        }

        if ( bPrintHandout )
        {
            PrintHandout(*pPrinter, rProgress, aPrintSelection,
                          aTimeDateStr, aTimeDateFont, pPrintOpts,
                          nPage, nPageMax,
                          nCollateCopies > 1 ? 1 : nCopies,
                          nProgressOffset, nTotal );
            nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
        }
        if( bPrintDraw )
        {
            PrintStdOrNotes(*pPrinter, rProgress, aPrintSelection,
                             aTimeDateStr, aTimeDateFont, pPrintOpts,
                             nPage, nPageMax,
                             nCollateCopies > 1 ? 1 : nCopies,
                             nProgressOffset, nTotal,
                             PK_STANDARD, bPrintMarkedOnly);
            nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
        }
        if( bPrintNotes )
        {
            PrintStdOrNotes(*pPrinter, rProgress, aPrintSelection,
                             aTimeDateStr, aTimeDateFont, pPrintOpts,
                             nPage, nPageMax,
                             nCollateCopies > 1 ? 1 : nCopies,
                             nProgressOffset, nTotal,
                             PK_NOTES, FALSE);
            nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
        }
    }

#ifndef OS2
    // schlaegt unter OS/2 immer fehl
    pPrinter->SetOrientation(eOldOrientation);
#endif

    pPrinter->SetMapMode(aOldMap);

    rOutliner.SetControlWord(nOldCntrl);

    // Druckerschach wieder zuruecksetzen
    pPrinter->SetPaperBin( nOldPaperBin );

    // 3D-Kontext wieder zerstoeren
    Base3D *pBase3D = (Base3D*) pPrinter->Get3DContext();
    if(pBase3D)
        pBase3D->Destroy(pPrinter);

    return 0;
}

/*************************************************************************
|*
|* Drucken der Outlinerdaten
|*
\************************************************************************/
void SdViewShell::PrintOutline(SfxPrinter& rPrinter,
                               SfxProgress& rProgress,
                               const MultiSelection& rSelPages,
                               const String& rTimeDateStr,
                               const Font& rTimeDateFont,
                               const SdOptionsPrintItem* pPrintOpts,
                               USHORT nPage, USHORT nPageMax,
                               USHORT nCopies, USHORT nProgressOffset, USHORT nTotal )
{
    // Papierschacht
    // Seiteneinstellungen koennen nicht fuer die Gliederung gemacht werden
    // (also auch nicht der Druckerschacht), deswegen wird der Druckerschacht
    // von der Seite PK_STANDARD genommen.
    /*
    if( pPrintOpts && !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = pDoc->GetSdPage(nPage, PK_STANDARD)->GetPaperBin();
        rPrinter.SetPaperBin( nPaperBin );
    } */
    // Es wird jetzt (vorlaeufig ?) der Druckerschacht vom Drucker genommen

    MapMode aMap = rPrinter.GetMapMode();
    Point aPageOfs = rPrinter.GetPageOffset();
    aMap.SetOrigin(Point() - aPageOfs);
    Fraction aScale(1, 2);
    aMap.SetScaleX(aScale);
    aMap.SetScaleY(aScale);
    rPrinter.SetMapMode(aMap);
    Orientation eOrient = rPrinter.GetOrientation();
    rPrinter.SetOrientation(ORIENTATION_PORTRAIT);

    BOOL bPrintExcluded = TRUE;
    if ( pPrintOpts )
        bPrintExcluded = pPrintOpts->IsHiddenPages();

    Rectangle aOutRect(aPageOfs, rPrinter.GetOutputSize());

    Link aOldLink;
    Outliner* pOutliner = pDoc->GetInternalOutliner();
    pOutliner->Init( OUTLINERMODE_OUTLINEVIEW );
    USHORT nOutlMode = pOutliner->GetMode();
    BOOL bOldUpdateMode = pOutliner->GetUpdateMode();

    if ( this->ISA( SdOutlineViewShell ) )
        pOutliner->SetMinDepth(0);

    Size aPaperSize = pOutliner->GetPaperSize();
    pOutliner->SetPaperSize(aOutRect.GetSize());
    pOutliner->SetUpdateMode(TRUE);

    long nPageH = aOutRect.GetHeight();

    USHORT nPageCount = nProgressOffset;

    while ( nPage < nPageMax )
    {
        Paragraph* pPara = NULL;
        long nH = 0;

        pOutliner->Clear();
        pOutliner->SetFirstPageNumber(nPage+1);

        while ( nH < nPageH && nPage < nPageMax )
        {
            if ( rSelPages.IsSelected(nPage+1) )
            {
                //rProgress.SetState( nPageCount, nTotal );
                //rProgress.SetStateText( nPageCount, nPage+1, nTotal );

                String aTmp = UniString::CreateFromInt32( nPage+1 );
                aTmp += String( SdResId( STR_PRINT_OUTLINE ) );
                rProgress.SetStateText( nPageCount, aTmp, nTotal );

                nPageCount += nCopies;

                SdPage* pPage = (SdPage*)
                            pDoc->GetSdPage(nPage, PK_STANDARD);
                SdrTextObj* pTextObj = NULL;
                ULONG nObj = 0;

                if ( !pPage->IsExcluded() || bPrintExcluded )
                {
                    while ( !pTextObj && nObj < pPage->GetObjCount() )
                    {
                        SdrObject* pObj = pPage->GetObj(nObj++);
                        if ( pObj->GetObjInventor() == SdrInventor &&
                             pObj->GetObjIdentifier() == OBJ_TITLETEXT )
                            pTextObj = (SdrTextObj*) pObj;
                    }

                    pPara = pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 );

                    if ( pTextObj && !pTextObj->IsEmptyPresObj() &&
                            pTextObj->GetOutlinerParaObject() )
                        pOutliner->AddText( *(pTextObj->GetOutlinerParaObject()) );
                    else
                        pOutliner->Insert(String());

                    pTextObj = NULL;
                    nObj = 0;

                    while ( !pTextObj && nObj < pPage->GetObjCount() )
                    {
                        SdrObject* pObj = pPage->GetObj(nObj++);
                        if ( pObj->GetObjInventor() == SdrInventor &&
                             pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
                            pTextObj = (SdrTextObj*) pObj;
                    }

                    BOOL bSubTitle = FALSE;
                    if (!pTextObj)
                    {
                        bSubTitle = TRUE;
                        pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_TEXT);  // Untertitel vorhanden?
                    }

                    ULONG nParaCount1 = pOutliner->GetParagraphCount();

                    if ( pTextObj && !pTextObj->IsEmptyPresObj() &&
                            pTextObj->GetOutlinerParaObject() )
                        pOutliner->AddText( *(pTextObj->GetOutlinerParaObject()) );

                    if ( bSubTitle )
                    {
                        ULONG nParaCount2 = pOutliner->GetParagraphCount();
                        Paragraph* pPara = NULL;
                        for (ULONG nPara = nParaCount1; nPara < nParaCount2; nPara++)
                        {
                            pPara = pOutliner->GetParagraph(nPara);
                            if(pPara && pOutliner->GetDepth( (USHORT) nPara ) !=1 )
                                pOutliner->SetDepth(pPara, 1);
                        }
                    }

                    nH = pOutliner->GetTextHeight();
                }
            }
            nPage++;
        }

        if ( nH > nPageH && pPara )
        {
            ULONG nCnt = pOutliner->GetAbsPos( pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 ) );
            ULONG nParaPos = pOutliner->GetAbsPos( pPara );
            nCnt -= nParaPos;
            pPara = pOutliner->GetParagraph( ++nParaPos );
            if ( nCnt && pPara )
            {
                pOutliner->Remove(pPara, nCnt);
                nPage--;
            }
        }

        rPrinter.StartPage();
        pOutliner->Draw(&rPrinter, aOutRect);

        if ( rTimeDateStr.Len() )
        {
            Font aOldFont = rPrinter.OutputDevice::GetFont();
            rPrinter.SetFont(rTimeDateFont);
            rPrinter.DrawText(aPageOfs, rTimeDateStr);
            rPrinter.SetFont(aOldFont);
        }
        rPrinter.EndPage();
    }
    pOutliner->SetUpdateMode(bOldUpdateMode);
    pOutliner->SetPaperSize(aPaperSize);
    pOutliner->Init( nOutlMode );
    rPrinter.SetOrientation(eOrient);
}

/*************************************************************************
|*
|* Drucken der Handouts
|*
\************************************************************************/
void SdViewShell::PrintHandout(SfxPrinter& rPrinter,
                                   SfxProgress& rProgress,
                                   const MultiSelection& rSelPages,
                                   const String& rTimeDateStr,
                                   const Font& rTimeDateFont,
                                   const SdOptionsPrintItem* pPrintOpts,
                                   USHORT nPage, USHORT nPageMax,
                                   USHORT nCopies, USHORT nProgressOffset, USHORT nTotal )
{
    SdrObject* pObj;
    SdPage* pPage = pDoc->GetSdPage(0, PK_HANDOUT);
    SdPage* pMaster = (SdPage*) pPage->GetMasterPage(0);

    // Papierschacht
    if( pPrintOpts && !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = pPage->GetPaperBin();
        rPrinter.SetPaperBin( nPaperBin );
    }

    // Hoch/Querformat aendern?
    Orientation eOrientation = pMaster->GetOrientation();

    short nDlgResult = RET_OK;

    if ( !rPrinter.SetOrientation(eOrientation) &&
        (!pPrintOpts || pPrintOpts->IsWarningOrientation()) )
    {
        // eine Warnung anzeigen
        WarningBox aWarnBox(pWindow,(WinBits)(WB_OK_CANCEL | WB_DEF_CANCEL),
                            String(SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
        nDlgResult = aWarnBox.Execute();
    }

    if ( nDlgResult == RET_OK )
    {
        MapMode aMap = rPrinter.GetMapMode();
        MapMode aOldMap = aMap;
        Point aPageOfs = rPrinter.GetPageOffset();
        aMap.SetOrigin(Point() - aPageOfs);
        rPrinter.SetMapMode(aMap);

        SdDrawView* pPrintView;

        if( this->ISA( SdDrawViewShell ) )
            pPrintView = new SdDrawView( pDocSh, &rPrinter, (SdDrawViewShell*)this );
        else
            pPrintView = new SdDrawView( pDocSh, &rPrinter, NULL );

        List*   pList = pMaster->GetPresObjList();

        USHORT nPageCount = nProgressOffset;

        WriteFrameViewData();

        nPrintedHandoutPageNum = 0;

        BOOL bPrintExcluded = TRUE;
        if ( pPrintOpts )
            bPrintExcluded = pPrintOpts->IsHiddenPages();

        while ( nPage < nPageMax )
        {
            pObj = (SdrObject*) pList->First();

            // Anzahl ALLER Seiten im Dokument:
            USHORT nAbsPageCnt = pDoc->GetPageCount();

            while ( pObj && nPage < nPageMax )
            {
                if ( rSelPages.IsSelected(nPage+1) )
                {
                    //rProgress.SetState( nPageCount, nTotal );
                    //rProgress.SetStateText( nPageCount, nPage+1, nTotal );

                    String aTmp = UniString::CreateFromInt32( nPage+1 );
                    aTmp += String( SdResId( STR_PRINT_HANDOUT ) );
                    rProgress.SetStateText( nPageCount, aTmp, nTotal );

                    nPageCount += nCopies;

                    SdPage* pPg = pDoc->GetSdPage(nPage, PK_STANDARD);
                    USHORT nRealPage = pPg->GetPageNum();

                    if ( !pPg->IsExcluded() || bPrintExcluded )
                    {
                        if ( pObj->ISA(SdrPageObj) )
                        {
                            ((SdrPageObj*) pObj)->NbcSetPageNum(nRealPage);
                            pObj = (SdrObject*) pList->Next();
                        }
                    }
                }
                nPage++;
            }

            while ( pObj )
            {   // restliche SdrPageObjs durch Angabe einer
                // ungueltigen Seitennummer ausblenden
                if ( pObj->ISA(SdrPageObj) )
                    ((SdrPageObj*) pObj)->NbcSetPageNum(nAbsPageCnt);
                pObj = (SdrObject*) pList->Next();
            }

            nPrintedHandoutPageNum++;

            rPrinter.StartPage();
            pPrintView->ShowPage(pPage, Point());

            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
            pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
            pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

            pPrintView->InitRedraw(&rPrinter, Rectangle(Point(0,0),
                                pPage->GetSize()));

            if ( rTimeDateStr.Len() )
            {
                Font aOldFont = rPrinter.OutputDevice::GetFont();
                rPrinter.SetFont(rTimeDateFont);
                rPrinter.DrawText(aPageOfs, rTimeDateStr);
                rPrinter.SetFont(aOldFont);
            }
            rPrinter.EndPage();
            pPrintView->HidePage(pPrintView->GetPageView(pPage));
        }

        USHORT nRealPage = pDoc->GetSdPage(0, PK_STANDARD)->GetPageNum();
        pObj = (SdrObject*) pList->First();

        while ( pObj )
        {   // Seitenobjekte wieder auf erste Seite setzen
            if ( pObj->ISA(SdrPageObj) )
            {
                ((SdrPageObj*) pObj)->NbcSetPageNum(nRealPage);
                nRealPage += 2;
            }

            pObj = (SdrObject*) pList->Next();
        }

        nPrintedHandoutPageNum = 1;
        delete pPrintView;

        rPrinter.SetMapMode(aOldMap);
    }
}

/*************************************************************************
|*
|* Drucken der normalen Seiten oder der Notizseiten
|*
\************************************************************************/
void SdViewShell::PrintStdOrNotes(SfxPrinter& rPrinter,
                                      SfxProgress& rProgress,
                                      const MultiSelection& rSelPages,
                                      const String& rTimeDateStr,
                                      const Font& rTimeDateFont,
                                      const SdOptionsPrintItem* pPrintOpts,
                                      USHORT nPage, USHORT nPageMax,
                                      USHORT nCopies, USHORT nProgressOffset, USHORT nTotal,
                                      PageKind ePageKind, BOOL bPrintMarkedOnly)
{
    Fraction    aFract;
    BOOL        bScalePage = TRUE;
    BOOL        bTilePage = FALSE;
    BOOL        bPrintExcluded = TRUE;
    BOOL        bPrintPageName = TRUE;
    BOOL        bPrintBooklet  = FALSE;
    BOOL        bPrintFrontPage = FALSE;
    BOOL        bPrintBackPage = FALSE;

    SdPage* pPage = pDoc->GetSdPage(nPage, ePageKind);

    if ( pPrintOpts )
    {
        bScalePage = pPrintOpts->IsPagesize();
        bTilePage = pPrintOpts->IsPagetile();
        bPrintPageName = pPrintOpts->IsPagename();
        bPrintExcluded = pPrintOpts->IsHiddenPages();
        bPrintBooklet = pPrintOpts->IsBooklet();
        bPrintFrontPage = pPrintOpts->IsFrontPage();
        bPrintBackPage = pPrintOpts->IsBackPage();

        // Papierschacht
        if( !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
        {
            USHORT nPaperBin = pDoc->GetSdPage(nPage, ePageKind)->GetPaperBin();
            rPrinter.SetPaperBin( nPaperBin );
        }

    }

    // Hoch/Querformat aendern?
    Size aPageSize(pPage->GetSize());
    Orientation eOrientation = ORIENTATION_PORTRAIT;

    short nDlgResult = RET_OK;

    if( !bPrintBooklet )
    {
        eOrientation = pPage->GetOrientation();
    }
    else
    {
        if( aPageSize.Width() < aPageSize.Height() )
            eOrientation = ORIENTATION_LANDSCAPE;
    }

    if ( !rPrinter.SetOrientation(eOrientation) &&
        (!pPrintOpts || pPrintOpts->IsWarningOrientation()) )
    {
        // eine Warnung anzeigen
        WarningBox aWarnBox(pWindow,(WinBits)(WB_OK_CANCEL | WB_DEF_CANCEL),
                            String(SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
        nDlgResult = aWarnBox.Execute();
    }

    if ( nDlgResult == RET_OK )
    {
        MapMode aMap = rPrinter.GetMapMode();
        MapMode aOldMap = aMap;
        Point aPageOfs = rPrinter.GetPageOffset();
        aMap.SetOrigin(Point() - aPageOfs);
        rPrinter.SetMapMode(aMap);
        Size aPrintSize = rPrinter.GetOutputSize();

        SdDrawView* pPrintView;
        if( this->ISA( SdDrawViewShell ) )
            pPrintView = new SdDrawView( pDocSh, &rPrinter, (SdDrawViewShell*)this );
        else
            pPrintView = new SdDrawView( pDocSh, &rPrinter, NULL );

        USHORT nPageCount = nProgressOffset;

        // Als Broschuere drucken ?
        if( bPrintBooklet )
        {
            SdPage*                                         pPage;
            MapMode                                         aStdMap( rPrinter.GetMapMode() );
            ::std::vector< USHORT >                         aPageVector;
            ::std::vector< ::std::pair< USHORT, USHORT > >  aPairVector;
            double                                          fHorz = (double) aPrintSize.Width() / (double) aPageSize.Width();
            double                                          fVert = (double) aPrintSize.Height() / (double) aPageSize.Height();

            if( fHorz < fVert )
                aFract = Fraction( aPrintSize.Width(), aPageSize.Width() );
            else
                aFract = Fraction( aPrintSize.Height(), aPageSize.Height() );

            aMap.SetScaleX( aFract );
            aMap.SetScaleY( aFract );

            // create vector of pages to print
            while( nPage < nPageMax )
            {
                if( rSelPages.IsSelected( nPage + 1 )  )
                {
                    SdPage* pPage = pDoc->GetSdPage( nPage, ePageKind );

                    if( pPage && ( !pPage->IsExcluded() || bPrintExcluded ) )
                        aPageVector.push_back( nPage );
                }

                nPage++;
            }

            // create pairs of pages to print on each page
            if( aPageVector.size() )
            {
                sal_uInt32 nFirstIndex = 0, nLastIndex = aPageVector.size() - 1;

                if( aPageVector.size() & 1 )
                    aPairVector.push_back( ::std::make_pair( 65535, aPageVector[ nFirstIndex++ ] ) );
                else
                    aPairVector.push_back( ::std::make_pair( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] ) );

                while( nFirstIndex < nLastIndex )
                {
                    if( nFirstIndex & 1 )
                        aPairVector.push_back( ::std::make_pair( aPageVector[ nFirstIndex++ ], aPageVector[ nLastIndex-- ] ) );
                    else
                        aPairVector.push_back( ::std::make_pair( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] ) );
                }
            }

            for( sal_uInt32 i = 0; i < aPairVector.size(); i++ )
            {
                if( ( !( i & 1 ) && bPrintFrontPage ) || ( ( i & 1 ) && bPrintBackPage ) )
                {
                    const ::std::pair< USHORT, USHORT > aPair( aPairVector[ i ] );
                    Point                               aPt;

                    rPrinter.StartPage();

                    pPage = pDoc->GetSdPage( aPair.first, ePageKind );

                    if( pPage )
                    {
                        aMap.SetOrigin( aPt );
                        rPrinter.SetMapMode( aMap );
                        PrintPage( rPrinter, pPrintView, pPage, bPrintMarkedOnly );
                    }

                    pPage = pDoc->GetSdPage( aPair.second, ePageKind );

                    if( pPage )
                    {
                        if( eOrientation == ORIENTATION_LANDSCAPE )
                            aPt.X() = aPageSize.Width() + aPageOfs.X();
                        else
                            aPt.Y() = aPageSize.Height() + aPageOfs.Y();

                        aMap.SetOrigin( aPt );
                        rPrinter.SetMapMode( aMap );
                        PrintPage( rPrinter, pPrintView, pPage, bPrintMarkedOnly );
                    }

                    rPrinter.EndPage();
                }
            }

            rPrinter.SetMapMode( aStdMap );
        }
        else
        {
            WriteFrameViewData();

            Point aPtZero;

            while ( nPage < nPageMax )
            {
                if ( rSelPages.IsSelected(nPage+1) )
                {
                    SdPage* pPage = pDoc->GetSdPage(nPage, ePageKind);
                    // Kann sich die Seitengroesse geaendert haben?
                    aPageSize = pPage->GetSize();

                    rProgress.SetState( nPageCount, nTotal );
                    nPageCount += nCopies;

                    if ( bScalePage )
                    {
                        double fHorz = (double) aPrintSize.Width()  / aPageSize.Width();
                        double fVert = (double) aPrintSize.Height() / aPageSize.Height();

                        if ( fHorz < fVert )
                            aFract = Fraction(aPrintSize.Width(), aPageSize.Width());
                        else
                            aFract = Fraction(aPrintSize.Height(), aPageSize.Height());

                        aMap.SetScaleX(aFract);
                        aMap.SetScaleY(aFract);
                        aMap.SetOrigin(Point());
                    }

                    if ( !pPage->IsExcluded() || bPrintExcluded )
                    {
                        String aTmp = UniString::CreateFromInt32( nPage+1 );

                        if(ePageKind == PK_NOTES)
                            aTmp += String( SdResId( STR_PRINT_NOTES ) );
                        else
                            aTmp += String( SdResId( STR_PRINT_DRAWING ) );
                        rProgress.SetStateText( nPageCount, aTmp, nTotal );

                        String aPageStr;
                        if ( bPrintPageName )
                        {
                            aPageStr = pPage->GetName();
                            aPageStr += (sal_Unicode)' ';
                        }
                        aPageStr += rTimeDateStr;

                        MapMode aStdMap = rPrinter.GetMapMode();
                        rPrinter.SetMapMode(aMap);

                        //
                        BOOL  bPrint = TRUE;
                        Point aPageOrigin;
                        long aPageWidth   = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
                        long aPageHeight  = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
                        long  aPrintWidth  = aPrintSize.Width();
                        long  aPrintHeight = aPrintSize.Height();

                        // Bugfix zu 44530:
                        // Falls implizit umgestellt wurde (Landscape/Portrait)
                        // wird dies beim Kacheln, bzw. aufteilen (Poster) beruecksichtigt
                        BOOL bSwitchPageSize = FALSE;
                        if( ( aPrintWidth > aPrintHeight && aPageWidth < aPageHeight ) ||
                            ( aPrintWidth < aPrintHeight && aPageWidth > aPageHeight ) )
                        {
                            bSwitchPageSize = TRUE;
                            aPrintWidth  = aPrintSize.Height();
                            aPrintHeight = aPrintSize.Width();
                        }

                        // Eine (kleine) Seite, die gekachelt ausgegeben werden soll
                        if( bTilePage &&
                            aPageWidth < aPrintWidth &&
                            aPageHeight < aPrintHeight )
                        {
                            aPageWidth  += 500;
                            aPageHeight += 500;
                            Point aPrintOrigin;

                            rPrinter.StartPage();
                            pPrintView->ShowPage( pPage, aPtZero );

                            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
                            pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                            pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

                            long nPagesPrinted = 0; // Fuer Bedingung s.u.
                            BOOL bPrintPage;        // Fuer Bedingung s.u.

                            while( bPrint )
                            {
                                BOOL bWidth = aPageWidth < aPrintWidth;
                                BOOL bHeight = aPageHeight < aPrintHeight;
                                if( !bWidth && !bHeight )
                                    bPrint = FALSE;

                                // Etwas umstaendliche Abfrage, die ausdrueckt, ob eine Seite
                                // beim Kacheln abgeschnitten oder gar nicht dargestellt werden
                                // soll. Die Bedingung ist, dass eine 2. Seite (horz. oder vert.)
                                // immer gedruck wird, waehrend eine dritte Seite (oder hoeher)
                                // nur noch gedruckt wird, wenn diese auch noch vollstaendig
                                // aufs Papier passt.
                                if( nPagesPrinted < 4 && !( !bWidth && nPagesPrinted == 2 ) )
                                    bPrintPage = TRUE;
                                else
                                    bPrintPage = FALSE;
                                nPagesPrinted++;

                                if( ( bWidth && bHeight ) || bPrintPage )
                                {
                                    aStdMap.SetOrigin( aPrintOrigin );
                                    rPrinter.SetMapMode( aStdMap );

                                    if( this->ISA( SdDrawViewShell ) && bPrintMarkedOnly )
                                    {
                                        pView->DrawAllMarked( rPrinter, aPtZero );
                                    }
                                    else
                                        pPrintView->InitRedraw( &rPrinter, Rectangle( aPtZero,
                                                                aPageSize ) );
                                }
                                if( bWidth )
                                {
                                    aPrintOrigin.X() += aPageWidth;
                                    aPrintWidth      -= aPageWidth;
                                }
                                else if( bHeight )
                                {
                                    aPrintOrigin.X()  = 0;
                                    aPrintWidth       = bSwitchPageSize ? aPrintSize.Height() : aPrintSize.Width();
                                    aPrintOrigin.Y() += aPageHeight;
                                    aPrintHeight     -= aPageHeight;
                                }
                            }

                            if ( aPageStr.Len() )
                            {
                                Font aOldFont = rPrinter.OutputDevice::GetFont();
                                rPrinter.SetFont(rTimeDateFont);
                                rPrinter.DrawText(aPageOfs, aPageStr);
                                rPrinter.SetFont(aOldFont);
                            }
                            rPrinter.EndPage();
                            pPrintView->HidePage(pPrintView->GetPageView(pPage));

                            bPrint = FALSE;
                        }

                        // Damit ein groessere Seite auch auf einem kleineren Drucker (Format)
                        // ausgegeben werden kann, wird folgende while-Schleife durchlaufen.
                        // Beim "Abschneiden" wird nur eine Seite gedruckt!
                        while( bPrint )
                        {
                            BOOL bWidth = aPageWidth > aPrintWidth;
                            BOOL bHeight = aPageHeight > aPrintHeight;
                            if( bScalePage ||
                                ( !bWidth && !bHeight ) ||
                                ( pPrintOpts && pPrintOpts->IsCutPage() ) )
                                bPrint = FALSE;

                            rPrinter.StartPage();
                            pPrintView->ShowPage(pPage, aPageOrigin);

                            if( bWidth )
                            {
                                aPageOrigin.X() -= aPrintWidth;
                                aPageWidth      -= aPrintWidth;
                            }
                            else if( bHeight )
                            {
                                aPageOrigin.X()  = 0;
                                aPageWidth       = aPageSize.Width() - pPage->GetUppBorder() - pPage->GetLwrBorder();
                                aPageOrigin.Y() -= aPrintHeight;
                                aPageHeight     -= aPrintHeight;
                            }

                            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
                            pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                            pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

                            if( this->ISA( SdDrawViewShell ) && bPrintMarkedOnly )
                                pView->DrawAllMarked( rPrinter, aPtZero );
                            else
                                pPrintView->InitRedraw(&rPrinter, Rectangle(Point(0,0),
                                                        pPage->GetSize()));
                            rPrinter.SetMapMode(aStdMap);

                            if ( aPageStr.Len() )
                            {
                                Font aOldFont = rPrinter.OutputDevice::GetFont();
                                rPrinter.SetFont(rTimeDateFont);
                                rPrinter.DrawText(aPageOfs, aPageStr);
                                rPrinter.SetFont(aOldFont);
                            }
                            rPrinter.EndPage();
                            pPrintView->HidePage(pPrintView->GetPageView(pPage));
                        }
                    }
                }
                nPage++;
            }
        }
        delete pPrintView;

        rPrinter.SetMapMode(aOldMap);
    }
}

/*************************************************************************
|*
|* Seite drucken
|*
\************************************************************************/
void SdViewShell::PrintPage( SfxPrinter& rPrinter, SdView* pPrintView,
                                 SdPage* pPage, BOOL bPrintMarkedOnly )
{
    Point aPtZero;
    pPrintView->ShowPage( pPage, aPtZero );

    SdrPageView* pPageView = pPrintView->GetPageView( pPage );
    pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
    pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

    if( this->ISA( SdDrawViewShell ) && bPrintMarkedOnly )
        pView->DrawAllMarked( rPrinter, aPtZero );
    else
        pPrintView->InitRedraw( &rPrinter, Rectangle( aPtZero,
                                pPage->GetSize() ) );

    pPrintView->HidePage( pPrintView->GetPageView( pPage ) );
    /*
    if ( aPageStr.Len() )
    {
        Font aOldFont = rPrinter.OutputDevice::GetFont();
        rPrinter.SetFont( rTimeDateFont );
        rPrinter.DrawText( aPageOfs, aPageStr );
        rPrinter.SetFont( aOldFont );
    }
    */
}

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void __EXPORT SdViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        UINT16 nFamily = (SfxStyleFamily)pDocSh->GetStyleFamily();

        SdrView* pDrView = GetDrawView();

        if( pDrView->HasMarkedObj() )
        {
            SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
            if( pStyleSheet )
            {
                if (pStyleSheet->GetFamily() == SD_LT_FAMILY)
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                if( pStyleSheet )
                {
                    SfxStyleFamily eFamily = pStyleSheet->GetFamily();
                    if(eFamily == SFX_STYLE_FAMILY_PARA)
                        nFamily = 2;
                    else
                        nFamily = 5;

                    pDocSh->SetStyleFamily(nFamily);
                }
            }
        }
        rSet.Put(SfxUInt16Item(SID_STYLE_FAMILY, nFamily ));
    }
}




