/*************************************************************************
 *
 *  $RCSfile: viewshe3.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:24:36 $
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

#include "ViewShell.hxx"

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <utility>
#include <vector>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#ifndef _SV_PRINTDLG_HXX
#include <svtools/printdlg.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
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
#include <svtools/misccfg.hxx>
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
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
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef SD_WINDOW_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "unslprms.hxx"
#ifndef SD_SLIDE_CHANGE_CHILD_WINDOW_HXX
#include "SlideChangeChildWindow.hxx"
#endif
#ifndef SD_SLIDE_CHANGE_HXX
#include "slidechg.hxx"
#endif
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
//CHINA001 #include "prntopts.hxx"
//CHINA001 #include "printdlg.hxx"
#include "optsitem.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif

// #96090#
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif

#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

namespace sd {

/*************************************************************************
|*
|* EffekteWindow updaten
|*
\************************************************************************/

void ViewShell::UpdateSlideChangeWindow()
{
    USHORT nId = SlideChangeChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        SlideChangeWindow* pSlideChangeWin = static_cast<SlideChangeWindow*>(
            pWindow->GetWindow());
        if( pSlideChangeWin && pSlideChangeWin->IsUpdateMode() )
        {
            SdPage* pPage      = NULL;
            USHORT  nNoOfPages = GetDoc()->GetSdPageCount(PK_STANDARD);
            USHORT  nPage;
            BOOL    bPageSelected = FALSE;

            // falls nichts selektiert ist, machen wir das schnell selbst
            for (USHORT i = 0; i < nNoOfPages; i++)
            {
                pPage = GetDoc()->GetSdPage(i, PK_STANDARD);
                if (pPage->IsSelected())
                {
                    bPageSelected = TRUE;
                    break;
                }
            }

            // ItemSet fuer Dialog (jetzt Window) fuellen
            SfxItemSet aSet(GetDoc()->GetPool(), ATTR_DIA_START, ATTR_DIA_END);

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
                    pPage = GetDoc()->GetSdPage(nPage, PK_STANDARD);
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
                    pPage = GetDoc()->GetSdPage(nPage, PK_STANDARD);
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

void ViewShell::AssignFromSlideChangeWindow()
{
    USHORT nId = SlideChangeChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        SlideChangeWindow* pSlideChangeWin = static_cast<SlideChangeWindow*>(
            pWindow->GetWindow());
        if( pSlideChangeWin )
        {
            SdPage* pPage      = NULL;
            USHORT  nNoOfPages = GetDoc()->GetSdPageCount(PK_STANDARD);
            USHORT  nPage;


            SfxItemSet aSet(GetDoc()->GetPool(), ATTR_DIA_START, ATTR_DIA_END );
            pSlideChangeWin->GetAttr( aSet );

            // Auswertung des ItemSets

            // Undo Gruppe erzeugen
            SdUndoGroup* pUndoGroup = new SdUndoGroup(GetDoc());
            String aComment(SdResId(STR_UNDO_SLIDE_PARAMS));
            pUndoGroup->SetComment(aComment);

            for (nPage = 0; nPage < nNoOfPages; nPage++)
            {
                pPage = GetDoc()->GetSdPage(nPage, PK_STANDARD);
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
                            (this->ISA(SlideViewShell)))
                        {
                            pView->InvalidateAllWin (
                                static_cast<SlideView*>(pView)
                                ->GetFadeIconArea(nPage));
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
                                    (GetDoc(), pPage,
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
                GetViewFrame()->GetChildWindow(
                    PreviewChildWindow::GetChildWindowId());
            if (pPreviewChildWindow)
            {
                PreviewWindow* pPreviewWindow = static_cast<PreviewWindow*>(
                    pPreviewChildWindow->GetWindow());
                if (pPreviewWindow!=NULL && pPreviewWindow->GetDoc()==GetDoc())
                {
                    pPreviewWindow->AnimatePage();
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
            GetDoc()->SetChanged();
        }
    }
}

/*************************************************************************
|*
|* Drucken der Outlinerdaten
|*
\************************************************************************/
void ViewShell::PrintOutline(SfxPrinter& rPrinter,
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
        USHORT nPaperBin = GetDoc()->GetSdPage(nPage, PK_STANDARD)->GetPaperBin();
        rPrinter.SetPaperBin( nPaperBin );
    } */
    // Es wird jetzt (vorlaeufig ?) der Druckerschacht vom Drucker genommen

    const MapMode       aOldMap( rPrinter.GetMapMode() );
    MapMode             aMap( aOldMap );
    const Orientation   eOldOrient = rPrinter.GetOrientation();
    Point               aPageOfs( rPrinter.GetPageOffset() );
    Fraction            aScale(1, 2);
    BOOL                bPrintExcluded = TRUE;

    aMap.SetOrigin(Point() - aPageOfs);
    aMap.SetScaleX(aScale);
    aMap.SetScaleY(aScale);
    rPrinter.SetMapMode(aMap);
    rPrinter.SetOrientation(ORIENTATION_PORTRAIT);

    if ( pPrintOpts )
        bPrintExcluded = pPrintOpts->IsHiddenPages();

    Rectangle aOutRect(aPageOfs, rPrinter.GetOutputSize());

    Link aOldLink;
    Outliner* pOutliner = GetDoc()->GetInternalOutliner();
    pOutliner->Init( OUTLINERMODE_OUTLINEVIEW );
    USHORT nOutlMode = pOutliner->GetMode();
    BOOL bOldUpdateMode = pOutliner->GetUpdateMode();

    if (this->ISA(OutlineViewShell))
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
                            GetDoc()->GetSdPage(nPage, PK_STANDARD);
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

    rPrinter.SetOrientation(eOldOrient);
    rPrinter.SetMapMode( aOldMap );
}

/*************************************************************************
|*
|* Drucken der Handouts
|*
\************************************************************************/
void ViewShell::PrintHandout (
    SfxPrinter& rPrinter,
    SfxProgress& rProgress,
    const MultiSelection& rSelPages,
    const String& rTimeDateStr,
    const Font& rTimeDateFont,
    const SdOptionsPrintItem* pPrintOpts,
    USHORT nPage,
    USHORT nPageMax,
    USHORT nCopies,
    USHORT nProgressOffset,
    USHORT nTotal )
{
    SdrObject* pObj;
    SdPage* pPage = GetDoc()->GetSdPage(0, PK_HANDOUT);
    SdPage& rMaster = (SdPage&)pPage->TRG_GetMasterPage();

    BOOL        bScalePage = TRUE;
    if ( pPrintOpts )
    {
        bScalePage = pPrintOpts->IsPagesize();
    }

    // Papierschacht
    if( pPrintOpts && !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = pPage->GetPaperBin();
        rPrinter.SetPaperBin( nPaperBin );
    }

    // Hoch/Querformat aendern?
    Orientation eOrientation = rMaster.GetOrientation();

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
        const MapMode   aOldMap( rPrinter.GetMapMode() );
        MapMode         aMap( aOldMap );
        Point           aPageOfs( rPrinter.GetPageOffset() );
        DrawView* pPrintView;
        BOOL            bPrintExcluded = TRUE;

        aMap.SetOrigin(Point() - aPageOfs);

        if ( bScalePage )
        {
            Size aPageSize(pPage->GetSize());
            Size aPrintSize( rPrinter.GetOutputSize() );

            double fHorz = (double) aPrintSize.Width()  / aPageSize.Width();
            double fVert = (double) aPrintSize.Height() / aPageSize.Height();

            Fraction    aFract;
            if ( fHorz < fVert )
                aFract = Fraction(aPrintSize.Width(), aPageSize.Width());
            else
                aFract = Fraction(aPrintSize.Height(), aPageSize.Height());

            aMap.SetScaleX(aFract);
            aMap.SetScaleY(aFract);
            aMap.SetOrigin(Point());
        }

        rPrinter.SetMapMode(aMap);

        if (this->ISA(DrawViewShell))
            pPrintView = new DrawView (GetDocSh(), &rPrinter,
                static_cast<DrawViewShell*>(this));
        else
            pPrintView = new DrawView (GetDocSh(), &rPrinter, NULL);

        sd::PresentationObjectList::iterator aIter;
        const sd::PresentationObjectList::iterator aEnd( rMaster.GetPresObjList().end() );
        USHORT  nPageCount = nProgressOffset;

        WriteFrameViewData();

        nPrintedHandoutPageNum = 0;

        if ( pPrintOpts )
            bPrintExcluded = pPrintOpts->IsHiddenPages();

        while ( nPage < nPageMax )
        {
            aIter = rMaster.GetPresObjList().begin();

            // Anzahl ALLER Seiten im Dokument:
            USHORT nAbsPageCnt = GetDoc()->GetPageCount();

            while( (aIter != aEnd) && (nPage < nPageMax) )
            {
                if( (*aIter).meKind == PRESOBJ_HANDOUT )
                {
                    pObj = (*aIter).mpObject;

                    if ( rSelPages.IsSelected(nPage+1) )
                    {
                        //rProgress.SetState( nPageCount, nTotal );
                        //rProgress.SetStateText( nPageCount, nPage+1, nTotal );

                        String aTmp = UniString::CreateFromInt32( nPage+1 );
                        aTmp += String( SdResId( STR_PRINT_HANDOUT ) );
                        rProgress.SetStateText( nPageCount, aTmp, nTotal );

                        nPageCount += nCopies;

                        SdPage* pPg = GetDoc()->GetSdPage(nPage, PK_STANDARD);

                        if ( !pPg->IsExcluded() || bPrintExcluded )
                        {
                            if ( pObj->ISA(SdrPageObj) )
                            {
                                ((SdrPageObj*) pObj)->SetReferencedPage(pPg);
                            }
                            else
                            {
                                DBG_ERROR("SdViewShell::PrintHandout() - PRESOBJ_HANDOUT is no SdrPageObj?" );
                            }
                            aIter++;
                        }
                    }
                    nPage++;
                }
                else
                {
                    aIter++;
                }
            }

            while ( aIter != aEnd )
            {
                if( (*aIter).meKind == PRESOBJ_HANDOUT )
                {
                    pObj = (*aIter).mpObject;
                    // invalidate remaining SdrPageObjs with
                    // an invalid page number
                    if ( pObj->ISA(SdrPageObj) )
                        ((SdrPageObj*) pObj)->SetReferencedPage(0L);
                }
                aIter++;
            }

            nPrintedHandoutPageNum++;

            rPrinter.StartPage();
            pPrintView->ShowPage(pPage, Point());

            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
            pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
            pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

            pPrintView->CompleteRedraw(&rPrinter, Rectangle(Point(0,0),
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

        USHORT nRealPage = GetDoc()->GetSdPage(0, PK_STANDARD)->GetPageNum();

        aIter = rMaster.GetPresObjList().begin();

        pObj = (aIter != aEnd) ? (*aIter++).mpObject : NULL;

        while( aIter != aEnd )
        {
            if( (*aIter).meKind == PRESOBJ_HANDOUT )
            {
                pObj = (*aIter).mpObject;
                // Seitenobjekte wieder auf erste Seite setzen
                if ( pObj->ISA(SdrPageObj) )
                {
                    ((SdrPageObj*) pObj)->SetReferencedPage(GetDoc()->GetPage(nRealPage));
                    nRealPage += 2;
                }
            }
            aIter++;
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
void ViewShell::PrintStdOrNotes(SfxPrinter& rPrinter,
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

    SdPage* pPage = GetDoc()->GetSdPage(nPage, ePageKind);

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
            USHORT nPaperBin = GetDoc()->GetSdPage(nPage, ePageKind)->GetPaperBin();
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
    else if( aPageSize.Width() < aPageSize.Height() )
        eOrientation = ORIENTATION_LANDSCAPE;

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
        const MapMode   aOldMap( rPrinter.GetMapMode() );
        MapMode         aMap( aOldMap );
        Point           aPageOfs( rPrinter.GetPageOffset() );
        DrawView* pPrintView;

        aMap.SetOrigin(Point() - aPageOfs);
        rPrinter.SetMapMode(aMap);
        Size aPrintSize( rPrinter.GetOutputSize() );

        if (this->ISA(DrawViewShell))
            pPrintView = new DrawView (GetDocSh(), &rPrinter,
                static_cast<DrawViewShell*>(this));
        else
            pPrintView = new DrawView (GetDocSh(), &rPrinter, NULL);

        USHORT nPageCount = nProgressOffset;

        // Als Broschuere drucken ?
        if( bPrintBooklet )
        {
            SdPage*                                         pPage;
            MapMode                                         aStdMap( rPrinter.GetMapMode() );
            ::std::vector< USHORT >                         aPageVector;
            ::std::vector< ::std::pair< USHORT, USHORT > >  aPairVector;
            Point                                           aOffset;
            Size                                            aPrintSize_2( aPrintSize );
            Size                                            aPageSize_2( aPageSize );

            if( eOrientation == ORIENTATION_LANDSCAPE )
                aPrintSize_2.Width() >>= 1;
            else
                aPrintSize_2.Height() >>= 1;

            const double fPageWH = (double) aPageSize_2.Width() / aPageSize_2.Height();
            const double fPrintWH = (double) aPrintSize_2.Width() / aPrintSize_2.Height();

            if( fPageWH < fPrintWH )
            {
                aPageSize_2.Width() = (long) ( aPrintSize_2.Height() * fPageWH );
                aPageSize_2.Height()= aPrintSize_2.Height();
            }
            else
            {
                aPageSize_2.Width() = aPrintSize_2.Width();
                aPageSize_2.Height() = (long) ( aPrintSize_2.Width() / fPageWH );
            }

            aMap.SetScaleX( Fraction( aPageSize_2.Width(), aPageSize.Width() ) );
            aMap.SetScaleY( Fraction( aPageSize_2.Height(), aPageSize.Height() ) );

            // calculate adjusted print size
            aPrintSize = OutputDevice::LogicToLogic( aPrintSize, aStdMap, aMap );

            if( eOrientation == ORIENTATION_LANDSCAPE )
            {
                aOffset.X() = ( ( aPrintSize.Width() >> 1 ) - aPageSize.Width() ) >> 1;
                aOffset.Y() = ( aPrintSize.Height() - aPageSize.Height() ) >> 1;
            }
            else
            {
                aOffset.X() = ( aPrintSize.Width() - aPageSize.Width() ) >> 1;
                aOffset.Y() = ( ( aPrintSize.Height() >> 1 ) - aPageSize.Height() ) >> 1;
            }

            // create vector of pages to print
            while( nPage < nPageMax )
            {
                if( rSelPages.IsSelected( nPage + 1 )  )
                {
                    SdPage* pPage = GetDoc()->GetSdPage( nPage, ePageKind );

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
                    aPairVector.push_back( ::std::make_pair( (USHORT) 65535, aPageVector[ nFirstIndex++ ] ) );
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

                    rPrinter.StartPage();

                    pPage = GetDoc()->GetSdPage( aPair.first, ePageKind );

                    if( pPage )
                    {
                        aMap.SetOrigin( aOffset );
                        rPrinter.SetMapMode( aMap );
                        PrintPage( rPrinter, pPrintView, pPage, bPrintMarkedOnly );
                    }

                    pPage = GetDoc()->GetSdPage( aPair.second, ePageKind );

                    if( pPage )
                    {
                        Point aSecondOffset( aOffset );

                        if( eOrientation == ORIENTATION_LANDSCAPE )
                            aSecondOffset.X() += ( aPrintSize.Width() >> 1 );
                        else
                            aSecondOffset.Y() += ( aPrintSize.Height() >> 1 );

                        aMap.SetOrigin( aSecondOffset );
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
                    SdPage* pPage = GetDoc()->GetSdPage(nPage, ePageKind);
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
                        BOOL    bPrint = TRUE;
                        Point   aPageOrigin;
                        long    aPageWidth   = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
                        long    aPageHeight  = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
                        long    aPrintWidth  = aPrintSize.Width();
                        long    aPrintHeight = aPrintSize.Height();

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

                                    if (this->ISA(DrawViewShell) && bPrintMarkedOnly )
                                    {
                                        pView->DrawAllMarked( rPrinter, aPtZero );
                                    }
                                    else
                                        pPrintView->CompleteRedraw( &rPrinter, Rectangle( aPtZero,
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

                            if (this->ISA(DrawViewShell) && bPrintMarkedOnly)
                                pView->DrawAllMarked( rPrinter, aPtZero );
                            else
                                pPrintView->CompleteRedraw(&rPrinter, Rectangle(Point(0,0),
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
        rPrinter.SetMapMode( aOldMap );
    }
}

/*************************************************************************
|*
|* Seite drucken
|*
\************************************************************************/
void ViewShell::PrintPage (
    SfxPrinter& rPrinter,
    ::sd::View* pPrintView,
    SdPage* pPage,
    BOOL bPrintMarkedOnly )
{
    Point aPtZero;
    pPrintView->ShowPage( pPage, aPtZero );

    SdrPageView* pPageView = pPrintView->GetPageView( pPage );
    pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
    pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

    if (this->ISA(DrawViewShell) && bPrintMarkedOnly)
        pView->DrawAllMarked( rPrinter, aPtZero );
    else
        pPrintView->CompleteRedraw( &rPrinter, Rectangle( aPtZero,
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

void  ViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        UINT16 nFamily = (SfxStyleFamily)GetDocSh()->GetStyleFamily();

        SdrView* pDrView = GetDrawView();

        if( pDrView->AreObjectsMarked() )
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

                    GetDocSh()->SetStyleFamily(nFamily);
                }
            }
        }
        rSet.Put(SfxUInt16Item(SID_STYLE_FAMILY, nFamily ));
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETUNDOSTRINGS))
    {
        ImpGetUndoStrings(rSet);
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETREDOSTRINGS))
    {
        ImpGetRedoStrings(rSet);
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_UNDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetUndoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // #87229# Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1072 ff.
            String aTmp(ResId(STR_UNDO, SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetUndoActionComment(0);
            rSet.Put(SfxStringItem(SID_UNDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_UNDO);
        }
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_REDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetRedoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // #87229# Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1081 ff.
            String aTmp(ResId(STR_REDO, SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetRedoActionComment(0);
            rSet.Put(SfxStringItem(SID_REDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_REDO);
        }
    }
}


void ViewShell::SetPreview( bool bVisible )
{
    DBG_ASSERT( GetViewFrame(), "FATAL: no viewframe?" );
    DBG_ASSERT( pWindow, "FATAL: no window?" );

    if( GetViewFrame() && pWindow )
    {
        if ( ! bVisible)
            mpWindowUpdater->UnregisterPreview ();
        GetViewFrame()->SetChildWindow(
            PreviewChildWindow::GetChildWindowId(), bVisible,false);
        if (bVisible)
            mpWindowUpdater->RegisterPreview ();

        const StyleSettings& rStyleSettings = pWindow->GetSettings().GetStyleSettings();

        sal_uInt16 nPreviewSlot;

        SvtAccessibilityOptions aAccOptions;

        if( GetViewFrame()->GetDispatcher() )
        {
            if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
            {
                nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
            }
            else
            {
                nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;
            }

            GetViewFrame()->GetDispatcher()->Execute( nPreviewSlot, SFX_CALLMODE_ASYNCHRON );
        }

        if (bVisible)
            UpdatePreview (GetActualPage());

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate(SID_PREVIEW_WIN);
        rBindings.Invalidate(SID_PREVIEW_QUALITY_COLOR);
        rBindings.Invalidate(SID_PREVIEW_QUALITY_GRAYSCALE);
        rBindings.Invalidate(SID_PREVIEW_QUALITY_BLACKWHITE);
        rBindings.Invalidate(SID_PREVIEW_QUALITY_CONTRAST);

    }
}



} // end of namespace sd
