/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PrintManager.cxx,v $
 * $Revision: 1.22 $
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
#include "precompiled_sd.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include "PrintManager.hxx"

#include <svx/editstat.hxx>
#include "optsitem.hxx"
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "Outliner.hxx"
#include "Window.hxx"
#include "FrameView.hxx"
#include <sfx2/progress.hxx>
#include <svtools/printdlg.hxx>
#include <tools/multisel.hxx>
#include <svtools/misccfg.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svx/prtqry.hxx>
#include "WindowUpdater.hxx"
#include <sfx2/printer.hxx>
#include <svx/svdoutl.hxx>
#include "sdresid.hxx"
#include <svx/svdetc.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svditer.hxx>

#include "strings.hrc"

#include <svx/svdopath.hxx>
#include <goodies/base3d.hxx>

#include "sdabstdlg.hxx"
#include "printdlg.hrc"
#include "prntopts.hrc"
#include "app.hrc"
#include <svtools/intitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/xlnclit.hxx>
#include "printdialog.hxx"

namespace sd {

/** This simple container reduces the number of arguments that have to be
    passed around and makes it easier to modify the set of arguments.
*/
class PrintManager::PrintInfo
{
public:
    PrintInfo (
        ViewShell& rViewShell,
        SfxPrinter& rPrinter,
        SfxProgress& rProgress,
        const MultiSelection& rSelPages,
        const String& rTimeDateStr,
        const Font& rTimeDateFont,
        const SdOptionsPrintItem* pPrintOpts,
        USHORT nPageMax,
        USHORT nCopies,
        USHORT nProgressOffset,
        USHORT nTotal)
        : mrViewShell(rViewShell),
          mrPrinter(rPrinter),
          mrProgress(rProgress),
          mrSelPages(rSelPages),
          mrTimeDateStr(rTimeDateStr),
          mrTimeDateFont(rTimeDateFont),
          mpPrintOpts(pPrintOpts),
          mnPageMax(nPageMax),
          mnCopies(nCopies),
          mnProgressOffset(nProgressOffset),
          mnTotal(nTotal)
    {}

    ViewShell& mrViewShell;
    SfxPrinter& mrPrinter;
    SfxProgress& mrProgress;
    const MultiSelection& mrSelPages;
    const String& mrTimeDateStr;
    const Font& mrTimeDateFont;
    const SdOptionsPrintItem* mpPrintOpts;
    USHORT mnPageMax;
    USHORT mnCopies;
    USHORT mnProgressOffset;
    USHORT mnTotal;
};


PrintManager::PrintManager (ViewShellBase& rBase)
    : mrBase (rBase)
{
}




PrintManager::~PrintManager (void)
{
}




SfxPrinter* PrintManager::GetPrinter (BOOL bCreate)
{
    return mrBase.GetDocShell()->GetPrinter (bCreate);
}




USHORT PrintManager::SetPrinter (
    SfxPrinter* pNewPrinter,
    USHORT nDiffFlags,bool bIsAPI)
{
    return SetPrinterOptDlg (pNewPrinter,nDiffFlags,!bIsAPI);
}




USHORT PrintManager::SetPrinterOptDlg (
    SfxPrinter* pNewPrinter,
    USHORT nDiffFlags,
    BOOL _bShowDialog)
{
    mrBase.GetDocShell()->SetPrinter(pNewPrinter);

    if ( (nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ||
          nDiffFlags & SFX_PRINTER_CHG_SIZE) && pNewPrinter  )
    {
        MapMode aMap = pNewPrinter->GetMapMode();
        aMap.SetMapUnit(MAP_100TH_MM);
        MapMode aOldMap = pNewPrinter->GetMapMode();
        pNewPrinter->SetMapMode(aMap);
        Size aNewSize = pNewPrinter->GetOutputSize();

        BOOL bScaleAll = FALSE;
        if ( _bShowDialog )
        {
            WarningBox aWarnBox (
                mrBase.GetWindow(),
                (WinBits)(WB_YES_NO | WB_DEF_YES),
                String(SdResId(STR_SCALE_OBJS_TO_PAGE)));
            bScaleAll = (aWarnBox.Execute() == RET_YES);
        }

        ViewShell* pShell = mrBase.GetMainViewShell().get();
        if (pShell != NULL && pShell->ISA(DrawViewShell))
        {
            SdPage* pPage = mrBase.GetDocument()->GetSdPage(
                0, PK_STANDARD );
            pShell->SetPageSizeAndBorder (
                static_cast<DrawViewShell*>(pShell)->GetPageKind(),
                aNewSize,
                -1,-1,-1,-1,
                bScaleAll,
                pNewPrinter->GetOrientation(),
                pPage->GetPaperBin(),
                pPage->IsBackgroundFullSize());
        }

        pNewPrinter->SetMapMode(aOldMap);
    }

    return 0;
}




PrintDialog* PrintManager::CreatePrintDialog (::Window *pParent)
{
    const bool bImpress = mrBase.GetDocument()->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

    SdPrintDialog* pDlg = SdPrintDialog::Create(pParent,bImpress);

    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell!=NULL && ! pShell->ISA(OutlineViewShell))
    {

        if (pShell->ISA(DrawViewShell))
        {
            pDlg->SetRangeText(UniString::CreateFromInt32(
                static_cast<DrawViewShell*>(pShell)->GetCurPageId()));
        }
        /*      else if(pShell->ISA(::sd::slidesorter::SlideSorterViewShell))
        {
            String aStrRange(
                dynamic_cast<sd::slidesorter::SlideSorterViewShell*>(pShell)->GetPageRangeString());
            if( aStrRange.Len() )
            {
                pDlg->SetRangeText( aStrRange );
                // According #79749 always check PRINTDIALOG_ALL
                // pDlg->CheckRange( PRINTDIALOG_RANGE );
            }
        }
        */
    }
    else
    {
        String aStrRange(
            static_cast<OutlineViewShell*>(pShell)->GetPageRangeString());
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

    if (pShell->ISA(DrawViewShell) && pShell->GetView()->AreObjectsMarked())
    {
        pDlg->EnableRange( PRINTDIALOG_SELECTION );
        // According #79749 always check PRINTDIALOG_ALL
        // pDlg->CheckRange( PRINTDIALOG_SELECTION );
    }

    if( bImpress )
    {
        const SdOptionsPrintItem* pPrintOpts = 0;

        SfxPrinter* pPrinter = mrBase.GetPrinter(FALSE);
        if( pPrinter )
        {
            const SfxPoolItem* pPoolItem = 0;
            if(pPrinter->GetOptions().GetItemState(ATTR_OPTIONS_PRINT, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                pPrintOpts = dynamic_cast< const SdOptionsPrintItem* >( pPoolItem );
        }
        pDlg->Init( pPrintOpts );
    }

    return pDlg;
}

SfxTabPage*  PrintManager::CreatePrintOptionsPage( ::Window *pParent, const SfxItemSet &rOptions)
{
    SfxTabPage* pPage = 0;
    DocumentType eDocType = mrBase.GetDocument()->GetDocumentType();
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ::CreateTabPage fnCreatePage = pFact ? pFact->GetSdPrintOptionsTabPageCreatorFunc() : 0;
    if( fnCreatePage )
    {
        pPage = (*fnCreatePage)( pParent, rOptions );
        if( eDocType == DOCUMENT_TYPE_DRAW )
        {
            SfxAllItemSet aSet(*(rOptions.GetPool()));
            aSet.Put (SfxUInt32Item(SID_SDMODE_FLAG,SD_DRAW_MODE));
            pPage->PageCreated(aSet);
        }
    }
    return pPage;
}




USHORT  PrintManager::Print (SfxProgress& rProgress, BOOL bIsAPI, PrintDialog* pDlg)
{
    SfxPrinter* pPrinter = mrBase.GetPrinter(TRUE);
    ViewShell* pShell = mrBase.GetMainViewShell().get();

    if (pPrinter!=NULL && pShell!=NULL)
    {
        const PrinterOptions    aOldPrinterOptions( pPrinter->GetPrinterOptions() );
        MapMode                 aMap( pPrinter->GetMapMode() );
        const MapMode           aOldMap( aMap );
        USHORT                  nOldPaperBin = pPrinter->GetPaperBin();

        aMap.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMap);

        ::Outliner& rOutliner = mrBase.GetDocument()->GetDrawOutliner();
        ULONG nOldCntrl = rOutliner.GetControlWord();
        ULONG nCntrl = nOldCntrl;
        nCntrl |= EE_CNTRL_NOREDLINES;
        nCntrl &= ~EE_CNTRL_MARKFIELDS;
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
        rOutliner.SetControlWord( nCntrl );

        // old place of FitPageToPrinterWithDialog().
        SdOptionsPrintItem* pPrintOpts = 0;

        const SfxPoolItem* pPoolItem = 0;
        if (pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, FALSE, &pPoolItem ) == SFX_ITEM_SET )
            pPrintOpts = dynamic_cast< SdOptionsPrintItem* >( const_cast< SfxPoolItem* >( pPoolItem ) ) ;

        if( dynamic_cast<SdPrintDialog*>(pDlg) && static_cast<SdPrintDialog*>(pDlg)->Fill( pPrintOpts ) )
        {
            SfxItemSet aNewOptions( pPrinter->GetOptions() );
            aNewOptions.Put( *pPrintOpts );
            pPrinter->SetOptions( aNewOptions );
            pPrintOpts = dynamic_cast< SdOptionsPrintItem* >( const_cast< SfxPoolItem* >( pPrinter->GetOptions().GetItem( ATTR_OPTIONS_PRINT ) ) );
        }

        // Wenn wir im Gliederungsmodus sind, muss das Model auf Stand gebracht werden
        if (pShell->ISA(OutlineViewShell))
            static_cast<OutlineViewShell*>(pShell)->PrepareClose (FALSE, FALSE);

        // Basisklasse rufen, um Basic anzusprechen
        mrBase.SfxViewShell::Print( rProgress, bIsAPI, pDlg );

        // Setzen des Textes des Druckmonitors
        rProgress.SetText( String( SdResId( STR_STATSTR_PRINT ) ) );

        PrintDialogRange    eOption;
        MultiSelection      aPrintSelection;
        String              aTimeDateStr;
        Font                aTimeDateFont(FAMILY_SWISS, Size(0, 423));
        PageKind            ePageKind = PK_STANDARD;
        USHORT              nPage, nPageMax;
        USHORT              nTotal, nCopies;
        USHORT              nPrintCount = 0;
        USHORT              nProgressOffset = 0;
        USHORT              nCollateCopies = 1;
        BOOL                bPrintMarkedOnly = FALSE;
        BOOL                bPrintOutline = FALSE;
        BOOL                bPrintHandout = FALSE;
        BOOL                bPrintDraw = FALSE;
        BOOL                bPrintNotes = FALSE;
        BOOL                bHandoutHorizontal = TRUE;
        USHORT              nSlidesPerHandout = 6;

        Orientation eOldOrientation = pPrinter->GetOrientation();

        if( pPrintOpts )
        {
            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();

            if( pPrintOpts->GetOptionsPrint().IsDate() )
            {
                aTimeDateStr += GetSdrGlobalData().pLocaleData->getDate( Date() );
                aTimeDateStr += (sal_Unicode)' ';
            }

            if( pPrintOpts->GetOptionsPrint().IsTime() )
                aTimeDateStr += GetSdrGlobalData().pLocaleData->getTime( Time(), FALSE, FALSE );

            if( pPrintOpts->GetOptionsPrint().IsOutline() )
                bPrintOutline = TRUE;

            if( pPrintOpts->GetOptionsPrint().IsHandout() )
            {
                bPrintHandout = TRUE;
                bHandoutHorizontal = pPrintOpts->GetOptionsPrint().IsHandoutHorizontal();
                nSlidesPerHandout = pPrintOpts->GetOptionsPrint().GetHandoutPages();
            }

            if( pPrintOpts->GetOptionsPrint().IsDraw() )
                bPrintDraw = TRUE;

            if( pPrintOpts->GetOptionsPrint().IsNotes() )
            {
                bPrintNotes = TRUE;
                ePageKind = PK_NOTES;
            }

            pPrintOpts->GetOptionsPrint().SetWarningPrinter( pMisc->IsNotFoundWarning() );
            pPrintOpts->GetOptionsPrint().SetWarningSize( pMisc->IsPaperSizeWarning() );
            pPrintOpts->GetOptionsPrint().SetWarningOrientation( pMisc->IsPaperOrientationWarning() );

            UINT16  nQuality = pPrintOpts->GetOptionsPrint().GetOutputQuality();
            ULONG   nMode = DRAWMODE_DEFAULT;

            if( nQuality == 1 )
                nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
            else if( nQuality == 2 )
                nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

            pPrinter->SetDrawMode( nMode );
        }
        else
            bPrintDraw = TRUE;

        if( pDlg )
        {
            eOption = pDlg->GetCheckedRange();

            if( eOption == PRINTDIALOG_SELECTION )
                bPrintMarkedOnly = TRUE;
        }
        else
            // Bei PrintDirect wird gesamtes Dokument gedruckt
            eOption = PRINTDIALOG_ALL;

        // #72527 If we are in PrintDirect and any objects
        // are selected, then a dialog (see ViewShell::DoPrint)
        // ask whether the total document should be printed
        // or only the selected objects. If only the selected
        // object, then the flag bPrintDirectSelected is TRUE
        if (mbPrintDirectSelected)
        {
            eOption = PRINTDIALOG_SELECTION;
            bPrintMarkedOnly = TRUE;
        }

        nPageMax = mrBase.GetDocument()->GetSdPageCount( ePageKind );
        aPrintSelection.SetTotalRange( Range( 1, nPageMax ) );

        switch( eOption )
        {
            case PRINTDIALOG_ALL:
                aPrintSelection.Select(Range(1, nPageMax));
            break;

            case PRINTDIALOG_RANGE:
                aPrintSelection = MultiSelection(pDlg->GetRangeText());
            break;

            default:
            {
                if (pShell->ISA(DrawViewShell))
                    aPrintSelection.Select (
                        static_cast<DrawViewShell*>(pShell)->GetCurPageId());
                else
                {
                    if( msPageRange.Len() )
                        aPrintSelection = MultiSelection( msPageRange );
                    else
                        aPrintSelection.Select(Range(1, nPageMax));
                }
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

        // check if selected range of pages contains transparent objects
        BOOL bContainsTransparency = FALSE;
        BOOL bPrintExcluded = TRUE;

        if( bPrintNotes || bPrintDraw || bPrintHandout )
        {
            if( pPrintOpts )
                bPrintExcluded = pPrintOpts->GetOptionsPrint().IsHiddenPages();

            for( USHORT j = nPage; ( j < nPageMax && !bContainsTransparency ); j++ )
            {
                if( aPrintSelection.IsSelected( j + 1 ) )
                {
                    SdPage* pPage = mrBase.GetDocument()->GetSdPage(
                        j, PK_STANDARD );

                    if( pPage && ( !pPage->IsExcluded() || bPrintExcluded ) )
                    {
                        bContainsTransparency = (BOOL)pPage->HasTransparentObjects();
                        if( !bContainsTransparency )
                        {
                            if(pPage->TRG_HasMasterPage())
                            {
                                bContainsTransparency = (BOOL)pPage->TRG_GetMasterPage().HasTransparentObjects();
                            }
                        }
                    }
                }
            }
        }

        if (pPrinter->InitJob (mrBase.GetWindow(), !bIsAPI && bContainsTransparency))
        {
            PrintInfo aInfo(
                *pShell,
                *pPrinter,
                rProgress,
                aPrintSelection,
                aTimeDateStr,
                aTimeDateFont,
                pPrintOpts,
                nPageMax,
                nCollateCopies > 1 ? 1 : nCopies,
                nProgressOffset,
                nTotal);

            if( bPrintHandout )
                InitHandoutTemplate( aInfo, nSlidesPerHandout, bHandoutHorizontal );

            for( USHORT n = 1; n <= nCollateCopies; n++ )
            {
                if ( bPrintOutline )
                {
                    // siehe unten in PrintOutline()
                    pPrinter->SetPaperBin( nOldPaperBin );

                    PrintOutline(aInfo, nPage);
                    aInfo.mnProgressOffset += (nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies));
                }

                if ( bPrintHandout )
                {
                    PrintHandout(aInfo, nPage );
                    aInfo.mnProgressOffset += (nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies));
                }
                if( bPrintDraw )
                {
                    PrintStdOrNotes(aInfo, nPage, PK_STANDARD, bPrintMarkedOnly);
                    aInfo.mnProgressOffset += (nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies));
                }
                if( bPrintNotes )
                {
                    PrintStdOrNotes(aInfo, nPage, PK_NOTES, FALSE);
                    aInfo.mnProgressOffset += (nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies));
                }

                // When in duplex mode then add an empty page after printing
                // an odd number of pages.  This is to avoid the first page
                // of the next run being printed on the backside of the
                // current page.
                if (nCollateCopies > 1
                    && n<nCollateCopies
                    && (pPrinter->GetCurPage()%2)==0
                    && (pPrinter->GetDuplexMode()==DUPLEX_ON))
                {
                    pPrinter->StartPage();
                    pPrinter->EndPage();
                }
            }
        }

        pPrinter->SetOrientation( eOldOrientation );
        pPrinter->SetPrinterOptions( aOldPrinterOptions );
        pPrinter->SetMapMode( aOldMap );

        rOutliner.SetControlWord(nOldCntrl);

        // Druckerschach wieder zuruecksetzen
        pPrinter->SetPaperBin( nOldPaperBin );

        // 3D-Kontext wieder zerstoeren
        Base3D *pBase3D = (Base3D*) pPrinter->Get3DContext();

        if( pBase3D )
            pBase3D->Destroy( pPrinter );
    }

    return 0;

}




ErrCode PrintManager::DoPrint (
    SfxPrinter* pPrinter,
    PrintDialog* pPrintDialog,
    BOOL bSilent, BOOL bIsAPI )
{
    ErrCode nResult = ERRCODE_NONE;

    do
    {
        ViewShell* pShell = mrBase.GetMainViewShell().get();
        if (pShell == NULL)
            break;

        if ( ! FitPageToPrinterWithDialog(pPrinter, bSilent))
            break;

        const SdrMarkList& rMarkList = pShell->GetView()->GetMarkedObjectList();

        // Retrieve the range of marked pages.
        String sNewPageRange (msPageRange);

        RestrictPrintingToSelection (FALSE);

        // #105477# Don't show query dialog if print dialog has been shown
        if ( !pPrintDialog
            && !bSilent && !bIsAPI
            && (rMarkList.GetMarkCount() || sNewPageRange.Len()) )
        {
            SvxPrtQryBox aQuery (mrBase.GetWindow());

            switch (aQuery.Execute())
            {
                case RET_CANCEL:
                    nResult = ERRCODE_IO_ABORT;
                    break;

                case RET_OK:
                    RestrictPrintingToSelection (TRUE);
                    SetPrintingPageRange (sNewPageRange);
                    break;
            }
        }

        if (nResult == ERRCODE_NONE)
        {
            // Tell the printer which digit language to use.
            WindowUpdater* pWindowUpdater = pShell->GetWindowUpdater();
            if (pWindowUpdater != NULL)
                pWindowUpdater->Update (pPrinter, mrBase.GetDocument());

            //  Forward the call to SfxViewShell::DoPrint() which
            //  eventually calls Print() (after StartJob etc.)
            nResult = mrBase.SfxViewShell::DoPrint (
                pPrinter, pPrintDialog, bSilent, bIsAPI );

            RestrictPrintingToSelection (FALSE);
        }
    }
    while(false);

    return nResult;
}




void PrintManager::PreparePrint (PrintDialog* )
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
        if ( pPrintOpts->GetOptionsPrint().IsHandout() )
        {
            // Handzettel
            SdPage* pPage = mrBase.GetDocument()->GetSdPage(0, PK_HANDOUT);

            // Papierschacht
            if (!pPrintOpts->GetOptionsPrint().IsPaperbin()) // Drucken NICHT aus Druckereinstellung
            {
                pPrinter->SetPaperBin(pPage->GetPaperBin());
            }

            pPrinter->SetOrientation(pPage->TRG_GetMasterPage().GetOrientation());
        }
        else if ( pPrintOpts->GetOptionsPrint().IsDraw() || pPrintOpts->GetOptionsPrint().IsNotes() )
        {
            // Standard- oder Notizseiten
            if( !pPrintOpts->GetOptionsPrint().IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
            {
                PageKind ePageKind = PK_NOTES;

                if (pPrintOpts->GetOptionsPrint().IsDraw())
                {
                    ePageKind = PK_STANDARD;
                }

                SdPage* pPage = mrBase.GetDocument()->GetSdPage(0, ePageKind);
                pPrinter->SetPaperBin(pPage->GetPaperBin());

                Orientation eOrientation = ORIENTATION_PORTRAIT;

                if ( !pPrintOpts->GetOptionsPrint().IsBooklet() )
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




void PrintManager::RestrictPrintingToSelection (bool bFlag)
{
    mbPrintDirectSelected = bFlag ? TRUE : FALSE;
}




void PrintManager::SetPrintingPageRange (const String& rsPageRange)
{
    msPageRange = rsPageRange;
}




bool PrintManager::FitPageToPrinterWithDialog (
    SfxPrinter* pPrinter,
    bool bSilent)
{
    bool bContinuePrinting = true;

    if (pPrinter != NULL)
    {
        // Remember old printer values so that they can be restored when
        // printing is aborted by the user.
        const PrinterOptions aOldPrinterOptions (pPrinter->GetPrinterOptions());
        const MapMode aOldMap (pPrinter->GetMapMode());

        // Get values from the priner in order to decide whether to show a
        // dialog.
        const SfxItemSet&   rOptions = pPrinter->GetOptions();
        SdOptionsPrintItem* pPrintOpts = NULL;
        BOOL                bScalePage = TRUE;
        BOOL                bTilePage = FALSE;
        BOOL                bPrintBooklet = FALSE;

        if( rOptions.GetItemState( ATTR_OPTIONS_PRINT, FALSE, (const SfxPoolItem**) &pPrintOpts ) == SFX_ITEM_SET )
        {
            bScalePage = pPrintOpts->GetOptionsPrint().IsPagesize();
            bPrintBooklet = pPrintOpts->GetOptionsPrint().IsBooklet();
            pPrintOpts->GetOptionsPrint().SetCutPage( FALSE );
        }
        else
            pPrintOpts = NULL;

        SdPage* pPage = mrBase.GetDocument()->GetSdPage( 0, PK_STANDARD );
        Size    aPageSize( pPage->GetSize() );
        Size    aPrintSize( pPrinter->GetOutputSize() );
        long    nPageWidth = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
        long    nPageHeight = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
        long    nPrintWidth = aPrintSize.Width();
        long    nPrintHeight = aPrintSize.Height();
        USHORT  nRet = RET_OK;

        // When necessary then show a dialog that asks the user how to fit
        // the pages to be printed to the (smaller) printer pages.
        if ( !bScalePage
            && !bTilePage
            && !bPrintBooklet
            && ( ( nPageWidth > nPrintWidth || nPageHeight > nPrintHeight )
                && ( nPageWidth > nPrintHeight || nPageHeight > nPrintWidth )
                )
            )
        {
            if (IsScreenFormat())
            {
                // For the screen format the page content is always scaled
                // to the printable area of the printer pages.
                pPrintOpts->GetOptionsPrint().SetPagesize();
            }
            else
            {

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                AbstractSdPrintDlg* pDlg = pFact ? pFact->CreateSdPrintDlg(mrBase.GetWindow() ) : 0;
                // Do not show the dialog when the bSilent flag is set.  We
                // do create the dialog anyway so that we can extract the
                // default method of mapping internal pages to printer pages
                // from it.
                if( ! bSilent && pDlg )
                    nRet = pDlg->Execute();
                if( nRet == RET_OK )
                {
                    switch (pDlg->GetAttr())
                    {
                        case 1:
                            pPrintOpts->GetOptionsPrint().SetPagesize();
                            break;

                        case 2:
                            // Tiling is the default behaviour in
                            // ViewShell::PrintStdOrNotes().  The poperty of
                            // pPrintOpts is ignored there so setting it
                            // here may only lead to unwanted side effects.
                            break;

                        case 3:
                            pPrintOpts->GetOptionsPrint().SetCutPage();
                            break;
                    }
                    bContinuePrinting = true;
                }
                delete pDlg;
            }
        }

        // The user has pressed the 'Cancel' button.  Restore the old values and
        // return a flag to tell the caller to abort printing.
        if( nRet == RET_CANCEL )
        {
            pPrinter->SetPrinterOptions( aOldPrinterOptions );
            pPrinter->SetMapMode( aOldMap );
            bContinuePrinting = false;
        }
    }

    return bContinuePrinting;
}




void PrintManager::PrintOutline (
    PrintInfo& rInfo,
    USHORT nPage)
{
    // Papierschacht
    // Seiteneinstellungen koennen nicht fuer die Gliederung gemacht werden
    // (also auch nicht der Druckerschacht), deswegen wird der Druckerschacht
    // von der Seite PK_STANDARD genommen.
    /*
    if( pPrintOpts && !pPrintOpts->GetOptionsPrint().IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = GetDoc()->GetSdPage(nPage, PK_STANDARD)->GetPaperBin();
        rPrinter.SetPaperBin( nPaperBin );
    } */
    // Es wird jetzt (vorlaeufig ?) der Druckerschacht vom Drucker genommen

    const MapMode       aOldMap( rInfo.mrPrinter.GetMapMode() );
    MapMode             aMap( aOldMap );
    const Orientation   eOldOrient = rInfo.mrPrinter.GetOrientation();
    Point               aPageOfs( rInfo.mrPrinter.GetPageOffset() );
    Fraction            aScale(1, 2);
    BOOL                bPrintExcluded = TRUE;

    aMap.SetOrigin(Point() - aPageOfs);
    aMap.SetScaleX(aScale);
    aMap.SetScaleY(aScale);
    rInfo.mrPrinter.SetMapMode(aMap);
    rInfo.mrPrinter.SetOrientation(ORIENTATION_PORTRAIT);

    if ( rInfo.mpPrintOpts )
        bPrintExcluded = rInfo.mpPrintOpts->GetOptionsPrint().IsHiddenPages();

    Rectangle aOutRect(aPageOfs, rInfo.mrPrinter.GetOutputSize());

    Link aOldLink;
    Outliner* pOutliner = rInfo.mrViewShell.GetDoc()->GetInternalOutliner();
    pOutliner->Init( OUTLINERMODE_OUTLINEVIEW );
    USHORT nOutlMode = pOutliner->GetMode();
    BOOL bOldUpdateMode = pOutliner->GetUpdateMode();

    if (rInfo.mrViewShell.ISA(OutlineViewShell))
        pOutliner->SetMinDepth(0);

    Size aPaperSize = pOutliner->GetPaperSize();
    pOutliner->SetPaperSize(aOutRect.GetSize());
    pOutliner->SetUpdateMode(TRUE);

    long nPageH = aOutRect.GetHeight();

    USHORT nPageCount = rInfo.mnProgressOffset;

    while ( nPage < rInfo.mnPageMax )
    {
        Paragraph* pPara = NULL;
        long nH = 0;

        pOutliner->Clear();
        pOutliner->SetFirstPageNumber(nPage+1);

        while ( nH < nPageH && nPage < rInfo.mnPageMax )
        {
            if ( rInfo.mrSelPages.IsSelected(nPage+1) )
            {
                //rProgress.SetState( nPageCount, nTotal );
                //rProgress.SetStateText( nPageCount, nPage+1, nTotal );

                String aTmp = UniString::CreateFromInt32( nPage+1 );
                aTmp += String( SdResId( STR_PRINT_OUTLINE ) );
                rInfo.mrProgress.SetStateText(nPageCount, aTmp, rInfo.mnTotal );

                nPageCount = nPageCount + rInfo.mnCopies;

                SdPage* pPage = (SdPage*)rInfo.mrViewShell.GetDoc()->GetSdPage(nPage, PK_STANDARD);
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
                        for (ULONG nPara = nParaCount1; nPara < nParaCount2; nPara++)
                        {
                            Paragraph* pP = pOutliner->GetParagraph(nPara);
                            if(pP && pOutliner->GetDepth( (USHORT) nPara ) !=1 )
                                pOutliner->SetDepth(pP, 1);
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

        rInfo.mrPrinter.StartPage();
        pOutliner->Draw(&rInfo.mrPrinter, aOutRect);

        if ( rInfo.mrTimeDateStr.Len() )
        {
            Font aOldFont = rInfo.mrPrinter.OutputDevice::GetFont();
            rInfo.mrPrinter.SetFont(rInfo.mrTimeDateFont);
            rInfo.mrPrinter.DrawText(aPageOfs, rInfo.mrTimeDateStr);
            rInfo.mrPrinter.SetFont(aOldFont);
        }
        rInfo.mrPrinter.EndPage();
    }
    pOutliner->SetUpdateMode(bOldUpdateMode);
    pOutliner->SetPaperSize(aPaperSize);
    pOutliner->Init( nOutlMode );

    rInfo.mrPrinter.SetOrientation(eOldOrient);
    rInfo.mrPrinter.SetMapMode( aOldMap );
}




void PrintManager::PrintHandout( PrintInfo& rInfo, USHORT nPage )
{
    SdPage* pPage = rInfo.mrViewShell.GetDoc()->GetSdPage(0, PK_HANDOUT);
    SdPage& rMaster = (SdPage&)pPage->TRG_GetMasterPage();
    SdDrawDocument* pDocument = rInfo.mrViewShell.GetDoc();

    BOOL        bScalePage = TRUE;
    if ( rInfo.mpPrintOpts )
    {
        bScalePage = rInfo.mpPrintOpts->GetOptionsPrint().IsPagesize();
    }

    // Papierschacht
    if( rInfo.mpPrintOpts && !rInfo.mpPrintOpts->GetOptionsPrint().IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = pPage->GetPaperBin();
        rInfo.mrPrinter.SetPaperBin( nPaperBin );
    }

    // Hoch/Querformat aendern?
    Orientation eOrientation = rMaster.GetOrientation();

    short nDlgResult = RET_OK;

    if ( !rInfo.mrPrinter.SetOrientation(eOrientation) &&
        (!rInfo.mpPrintOpts || rInfo.mpPrintOpts->GetOptionsPrint().IsWarningOrientation()) )
    {
        // eine Warnung anzeigen
        WarningBox aWarnBox(rInfo.mrViewShell.GetActiveWindow(),(WinBits)(WB_OK_CANCEL | WB_DEF_CANCEL),
                            String(SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
        nDlgResult = aWarnBox.Execute();
    }

    if ( nDlgResult == RET_OK )
    {
        const MapMode   aOldMap( rInfo.mrPrinter.GetMapMode() );
        MapMode         aMap( aOldMap );
        Point           aPageOfs( rInfo.mrPrinter.GetPageOffset() );
        DrawView* pPrintView;
        BOOL            bPrintExcluded = TRUE;

        aMap.SetOrigin(Point() - aPageOfs);

        if ( bScalePage )
        {
            Size aPageSize(pPage->GetSize());
            Size aPrintSize( rInfo.mrPrinter.GetOutputSize() );

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

        rInfo.mrPrinter.SetMapMode(aMap);

        if (rInfo.mrViewShell.ISA(DrawViewShell))
            pPrintView = new DrawView (rInfo.mrViewShell.GetDocSh(), &rInfo.mrPrinter,
                static_cast<DrawViewShell*>(&rInfo.mrViewShell));
        else
            pPrintView = new DrawView (rInfo.mrViewShell.GetDocSh(), &rInfo.mrPrinter, NULL);

        USHORT  nPageCount = rInfo.mnProgressOffset;

        rInfo.mrViewShell.WriteFrameViewData();

        rInfo.mrViewShell.SetPrintedHandoutPageNum(0);

        if ( rInfo.mpPrintOpts )
            bPrintExcluded = rInfo.mpPrintOpts->GetOptionsPrint().IsHiddenPages();

        std::vector< SdPage* > aPagesVector;
        while ( nPage < rInfo.mnPageMax )
        {
            nPage++;
            if( rInfo.mrSelPages.IsSelected(nPage) )
            {
                SdPage* pTemp = pDocument->GetSdPage(nPage-1, PK_STANDARD);
                if( !pTemp->IsExcluded() || bPrintExcluded )
                    aPagesVector.push_back( pTemp );
            }
        }

        std::vector< SdrPageObj* > aPageObjVector;
        SdrObjListIter aShapeIter( *pPage );
        while( aShapeIter.IsMore() )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(aShapeIter.Next());
            if( pPageObj )
                aPageObjVector.push_back( pPageObj );
        }

        if( aPageObjVector.empty() )
            return;

/*
        sal_Int32 nHandoutPageCount = aPagesVector.size() / aPageObjVector.size();
        sal_Int32 nHandoutPage = 0;
*/
        std::vector< SdPage* >::iterator aPageIter( aPagesVector.begin() );
        while( aPageIter != aPagesVector.end() )
        {
            std::vector< SdrPageObj* >::iterator aPageObjIter( aPageObjVector.begin() );

            while( (aPageObjIter != aPageObjVector.end()) && (aPageIter != aPagesVector.end()) )
            {
                SdrPageObj* pPageObj = (*aPageObjIter++);
                pPageObj->SetReferencedPage( (*aPageIter++) );
            }

            // if there are more page objects than pages left, set the rest to invisible
            int nHangoverCount = 0;
            while(aPageObjIter != aPageObjVector.end())
            {
                (*aPageObjIter++)->SetReferencedPage(0L);
                nHangoverCount++;
            }

            if( nHangoverCount )
            {
                int nSkip = aPageObjVector.size() - nHangoverCount;
                aShapeIter.Reset();
                while( aShapeIter.IsMore() )
                {
                    SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( aShapeIter.Next() );
                    if( pPathObj )
                    {
                        if( nSkip )
                        {
                            nSkip--;
                        }
                        else
                        {
                            pPathObj->SetMergedItem( XLineStyleItem(XLINE_NONE) );
                        }
                    }
                }
            }

            // todo progress
            String aTmp = UniString::CreateFromInt32( nPage );
            aTmp += String( SdResId( STR_PRINT_HANDOUT ) );
            rInfo.mrProgress.SetStateText( nPageCount, aTmp, rInfo.mnTotal );

            rInfo.mrViewShell.SetPrintedHandoutPageNum(rInfo.mrViewShell.GetPrintedHandoutPageNum() + 1);

            rInfo.mrPrinter.StartPage();
            pPrintView->ShowSdrPage(pPage); // , Point());

            SdrPageView* pPageView = pPrintView->GetSdrPageView(); // pPage);
            pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
            pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

            pPrintView->CompleteRedraw(&rInfo.mrPrinter, Rectangle(Point(0,0), pPage->GetSize()));

            if ( rInfo.mrTimeDateStr.Len() )
            {
                Font aOldFont = rInfo.mrPrinter.OutputDevice::GetFont();
                rInfo.mrPrinter.SetFont(rInfo.mrTimeDateFont);
                rInfo.mrPrinter.DrawText(aPageOfs, rInfo.mrTimeDateStr);
                rInfo.mrPrinter.SetFont(aOldFont);
            }
            rInfo.mrPrinter.EndPage();
            pPrintView->HideSdrPage(); // pPrintView->GetPageView(pPage));

            if( nHangoverCount )
            {
                aShapeIter.Reset();
                while( aShapeIter.IsMore() )
                {
                    SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( aShapeIter.Next() );
                    if( pPathObj )
                        pPathObj->SetMergedItem( XLineStyleItem(XLINE_SOLID) );
                }
            }

        }

        rInfo.mrViewShell.SetPrintedHandoutPageNum(1);
        delete pPrintView;
        rInfo.mrPrinter.SetMapMode(aOldMap);
    }
}




void PrintManager::PrintStdOrNotes (
    PrintInfo& rInfo,
    USHORT nPage,
    PageKind ePageKind,
    BOOL bPrintMarkedOnly)
{
    Fraction    aFract;
    BOOL        bScalePage = TRUE;
    BOOL        bTilePage = FALSE;
    BOOL        bPrintExcluded = TRUE;
    BOOL        bPrintPageName = TRUE;
    BOOL        bPrintBooklet  = FALSE;
    BOOL        bPrintFrontPage = FALSE;
    BOOL        bPrintBackPage = FALSE;
    SdDrawDocument* pDocument = rInfo.mrViewShell.GetDoc();

    SdPage* pRefPage = pDocument->GetSdPage(nPage, ePageKind);

    if ( rInfo.mpPrintOpts )
    {
        bScalePage = rInfo.mpPrintOpts->GetOptionsPrint().IsPagesize();
        bTilePage = rInfo.mpPrintOpts->GetOptionsPrint().IsPagetile();
        bPrintPageName = rInfo.mpPrintOpts->GetOptionsPrint().IsPagename();
        bPrintExcluded = rInfo.mpPrintOpts->GetOptionsPrint().IsHiddenPages();
        bPrintBooklet = rInfo.mpPrintOpts->GetOptionsPrint().IsBooklet();
        bPrintFrontPage = rInfo.mpPrintOpts->GetOptionsPrint().IsFrontPage();
        bPrintBackPage = rInfo.mpPrintOpts->GetOptionsPrint().IsBackPage();

        // Papierschacht
        if( !rInfo.mpPrintOpts->GetOptionsPrint().IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
        {
            USHORT nPaperBin = pDocument->GetSdPage(nPage, ePageKind)->GetPaperBin();
            rInfo.mrPrinter.SetPaperBin( nPaperBin );
        }

    }

    // Hoch/Querformat aendern?
    Size aPageSize(pRefPage->GetSize());
    Orientation eOrientation = ORIENTATION_PORTRAIT;

    short nDlgResult = RET_OK;

    if( !bPrintBooklet )
    {
        eOrientation = pRefPage->GetOrientation();
    }
    else if( aPageSize.Width() < aPageSize.Height() )
        eOrientation = ORIENTATION_LANDSCAPE;

    if ( !rInfo.mrPrinter.SetOrientation(eOrientation) &&
        (!rInfo.mpPrintOpts || rInfo.mpPrintOpts->GetOptionsPrint().IsWarningOrientation()) )
    {
        // eine Warnung anzeigen
        WarningBox aWarnBox(
            rInfo.mrViewShell.GetActiveWindow(),
            (WinBits)(WB_OK_CANCEL | WB_DEF_CANCEL),
            String(SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
        nDlgResult = aWarnBox.Execute();
    }

    if ( nDlgResult == RET_OK )
    {
        const MapMode   aOldMap( rInfo.mrPrinter.GetMapMode() );
        MapMode         aMap( aOldMap );
        Point           aPageOfs( rInfo.mrPrinter.GetPageOffset() );
        DrawView* pPrintView;

        aMap.SetOrigin(Point() - aPageOfs);
        rInfo.mrPrinter.SetMapMode(aMap);
        Size aPrintSize( rInfo.mrPrinter.GetOutputSize() );

        if (rInfo.mrViewShell.ISA(DrawViewShell))
            pPrintView = new DrawView (rInfo.mrViewShell.GetDocSh(), &rInfo.mrPrinter,
                static_cast<DrawViewShell*>(&rInfo.mrViewShell));
        else
            pPrintView = new DrawView (rInfo.mrViewShell.GetDocSh(), &rInfo.mrPrinter, NULL);

        USHORT nPageCount = rInfo.mnProgressOffset;

        // Als Broschuere drucken ?
        if( bPrintBooklet )
        {
            MapMode                                         aStdMap( rInfo.mrPrinter.GetMapMode() );
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
            while( nPage < rInfo.mnPageMax )
            {
                if( rInfo.mrSelPages.IsSelected( nPage + 1 )  )
                {
                    SdPage* pP = pDocument->GetSdPage( nPage, ePageKind );

                    if( pP && ( !pP->IsExcluded() || bPrintExcluded ) )
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

                    rInfo.mrPrinter.StartPage();

                    SdPage* pPageToPrint = pDocument->GetSdPage( aPair.first, ePageKind );

                    if( pPageToPrint )
                    {
                        aMap.SetOrigin( aOffset );
                        rInfo.mrPrinter.SetMapMode( aMap );
                        PrintPage(rInfo, pPrintView, pPageToPrint, bPrintMarkedOnly );
                    }

                    pPageToPrint = pDocument->GetSdPage( aPair.second, ePageKind );

                    if( pPageToPrint )
                    {
                        Point aSecondOffset( aOffset );

                        if( eOrientation == ORIENTATION_LANDSCAPE )
                            aSecondOffset.X() += ( aPrintSize.Width() >> 1 );
                        else
                            aSecondOffset.Y() += ( aPrintSize.Height() >> 1 );

                        aMap.SetOrigin( aSecondOffset );
                        rInfo.mrPrinter.SetMapMode( aMap );
                        PrintPage(rInfo, pPrintView, pPageToPrint, bPrintMarkedOnly );
                    }

                    rInfo.mrPrinter.EndPage();
                }
            }

            rInfo.mrPrinter.SetMapMode( aStdMap );
        }
        else
        {
            rInfo.mrViewShell.WriteFrameViewData();

            Point aPtZero;

            while ( nPage < rInfo.mnPageMax )
            {
                if (rInfo.mrSelPages.IsSelected(nPage+1) )
                {
                    SdPage* pPage = pDocument->GetSdPage(nPage, ePageKind);
                    // Kann sich die Seitengroesse geaendert haben?
                    aPageSize = pPage->GetSize();

                    rInfo.mrProgress.SetState( nPageCount, rInfo.mnTotal );
                    nPageCount = nPageCount + rInfo.mnCopies;

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
                        rInfo.mrProgress.SetStateText( nPageCount, aTmp, rInfo.mnTotal );

                        String aPageStr;
                        if ( bPrintPageName )
                        {
                            aPageStr = pPage->GetName();
                            aPageStr += (sal_Unicode)' ';
                        }
                        aPageStr += rInfo.mrTimeDateStr;

                        MapMode aStdMap = rInfo.mrPrinter.GetMapMode();
                        rInfo.mrPrinter.SetMapMode(aMap);

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

                            rInfo.mrPrinter.StartPage();
                            pPrintView->ShowSdrPage( pPage ); // , aPtZero );

                            SdrPageView* pPageView = pPrintView->GetSdrPageView(); // pPage);
                            pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
                            pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

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
                                    rInfo.mrPrinter.SetMapMode( aStdMap );

                                    if (rInfo.mrViewShell.ISA(DrawViewShell) && bPrintMarkedOnly )
                                    {
                                        rInfo.mrViewShell.GetView()->DrawAllMarked( rInfo.mrPrinter, aPtZero );
                                    }
                                    else
                                        pPrintView->CompleteRedraw( &rInfo.mrPrinter, Rectangle( aPtZero,
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
                                Font aOldFont = rInfo.mrPrinter.OutputDevice::GetFont();
                                rInfo.mrPrinter.SetFont(rInfo.mrTimeDateFont);
                                rInfo.mrPrinter.DrawText(aPageOfs, aPageStr);
                                rInfo.mrPrinter.SetFont(aOldFont);
                            }
                            rInfo.mrPrinter.EndPage();
                            pPrintView->HideSdrPage(); // pPrintView->GetPageView(pPage));

                            bPrint = FALSE;
                        }

                        // For pages larger then the printable area there
                        // are three options:
                        // 1. Scale down to the page to the printable area.
                        // 2. Print only the upper left part of the page
                        //    (without the unprintable borders).
                        // 3. Split the page into parts of the size of the
                        // printable area.
                        if (bPrint)
                        {
                            const bool bCutPage (rInfo.mpPrintOpts && rInfo.mpPrintOpts->GetOptionsPrint().IsCutPage());
                            if (bScalePage || bCutPage)
                            {
                                // Handle 1 and 2.

                                // if CutPage is set then do not move
                                // it, otherwise move the scaled page
                                // to printable area

                                PrintPagePart(
                                    rInfo,
                                    pPage,
                                    *pPrintView,
                                    bCutPage ? Point(-aPageOfs.X(), -aPageOfs.Y()) : Point(0,0),
                                    bPrintMarkedOnly,
                                    aPageStr,
                                    aPageOfs);
                            }
                            else
                            {
                                // Handle 3.  Print parts of the page in the
                                // size of the printable area until the
                                // whole page is covered.

                                // keep the page content at its
                                // position if it fits, otherwise move
                                // it to the printable area
                                Point aOrigin (aPageWidth < aPrintWidth ? -aPageOfs.X() : 0, aPageHeight < aPrintHeight ? -aPageOfs.Y() : 0);

                                for (aPageOrigin = aOrigin;
                                     -aPageOrigin.Y()<aPageHeight;
                                         aPageOrigin.Y() -= aPrintHeight)
                                {
                                    for (aPageOrigin.X()=aOrigin.X();
                                         -aPageOrigin.X()<aPageWidth;
                                             aPageOrigin.X() -= aPrintWidth)
                                    {
                                        PrintPagePart(
                                            rInfo,
                                            pPage,
                                            *pPrintView,
                                            aPageOrigin,
                                            bPrintMarkedOnly,
                                            aPageStr,
                                            aPageOfs);
                                    }
                                }
                            }
                        }
                    }
                }
                nPage++;
            }
        }

        delete pPrintView;
        rInfo.mrPrinter.SetMapMode( aOldMap );
    }
}




void PrintManager::PrintPagePart (
    PrintInfo& rInfo,
    SdPage* pPage,
    DrawView& rPrintView,
    const Point& rPageOrigin,
    BOOL bPrintMarkedOnly,
    const String& rsPageString,
    const Point& rStringOffset)
{
    rInfo.mrPrinter.StartPage();
    rPrintView.ShowSdrPage(pPage); // , Point(0,0));

    // Set origin of the map mode so that the next part of the page is
    // printed.
    MapMode aOriginalMapMode = rInfo.mrPrinter.GetMapMode();
    MapMode aMapMode = rInfo.mrPrinter.GetMapMode();
    aMapMode.SetOrigin(rPageOrigin);
    rInfo.mrPrinter.SetMapMode(aMapMode);

    SdrPageView* pPageView = rPrintView.GetSdrPageView(); // pPage);
    pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
    pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

    if (rInfo.mrViewShell.ISA(DrawViewShell) && bPrintMarkedOnly)
        rInfo.mrViewShell.GetView()->DrawAllMarked( rInfo.mrPrinter, Point(0,0));
    else
        rPrintView.CompleteRedraw(
            &rInfo.mrPrinter,
            Rectangle(
                Point(0,0),
                pPage->GetSize()));
    rInfo.mrPrinter.SetMapMode(aOriginalMapMode);

    if (rsPageString.Len())
    {
        Font aOldFont = rInfo.mrPrinter.OutputDevice::GetFont();
        rInfo.mrPrinter.SetFont(rInfo.mrTimeDateFont);
        rInfo.mrPrinter.DrawText(rStringOffset, rsPageString);
        rInfo.mrPrinter.SetFont(aOldFont);
    }
    rInfo.mrPrinter.EndPage();
    rPrintView.HideSdrPage(); // rPrintView.GetPageView(pPage));
}





void PrintManager::PrintPage (
    PrintInfo& rInfo,
    ::sd::View* pPrintView,
    SdPage* pPage,
    BOOL bPrintMarkedOnly)
{
    Point aPtZero;
    pPrintView->ShowSdrPage( pPage ); // , aPtZero );

    SdrPageView* pPageView = pPrintView->GetSdrPageView(); // pPage );
    pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
    pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

    if (rInfo.mrViewShell.ISA(DrawViewShell) && bPrintMarkedOnly)
        rInfo.mrViewShell.GetView()->DrawAllMarked(rInfo.mrPrinter, Point(0,0));
    else
        pPrintView->CompleteRedraw(
            &rInfo.mrPrinter,
            Rectangle(Point(0,0), pPage->GetSize()), 0);

    pPrintView->HideSdrPage(); //  pPrintView->GetPageView( pPage ) );
}




bool PrintManager::IsScreenFormat (void)
{
    // Determine the page size.
    SdPage* pPage = mrBase.GetDocument()->GetSdPage(0, PK_STANDARD);
    Size aPaperSize (pPage->GetSize());

    // Take Orientation into account.
    if (pPage->GetOrientation() == ORIENTATION_LANDSCAPE)
        Swap(aPaperSize);

    // Check whether paper size is 'Screen'
    SvxPaper ePaper (SvxPaperInfo::GetPaper(aPaperSize, MAP_100TH_MM, TRUE));
    return (ePaper == SVX_PAPER_SCREEN);
}


void PrintManager::InitHandoutTemplate( PrintInfo& /*rInfo*/, USHORT nSlidesPerHandout, BOOL bHandoutHorizontal )
{
    AutoLayout eLayout = AUTOLAYOUT_HANDOUT6;
    switch( nSlidesPerHandout )
    {
    case 1: eLayout = AUTOLAYOUT_HANDOUT1; break;
    case 2: eLayout = AUTOLAYOUT_HANDOUT2; break;
    case 3: eLayout = AUTOLAYOUT_HANDOUT3; break;
    case 4: eLayout = AUTOLAYOUT_HANDOUT4; break;
    case 9: eLayout = AUTOLAYOUT_HANDOUT9; break;
    }

    if( !mrBase.GetDocument() )
        return;

    SdDrawDocument& rModel = *mrBase.GetDocument();

    // first, prepare handout page (not handout master)

    SdPage* pHandout = rModel.GetSdPage(0, PK_HANDOUT);
    if( !pHandout )
        return;

    // delete all previous shapes from handout page
    while( pHandout->GetObjCount() )
    {
        SdrObject* pObj = pHandout->NbcRemoveObject(0);
        if( pObj )
            SdrObject::Free( pObj  );
    }

    const bool bDrawLines = eLayout == AUTOLAYOUT_HANDOUT3;

    std::vector< Rectangle > aAreas;
    SdPage::CalculateHandoutAreas( rModel, eLayout, bHandoutHorizontal, aAreas );

    std::vector< Rectangle >::iterator iter( aAreas.begin() );
    while( iter != aAreas.end() )
    {
        pHandout->NbcInsertObject( new SdrPageObj((*iter++)) );

        if( bDrawLines && (iter != aAreas.end())  )
        {
            Rectangle aRect( (*iter++) );

            basegfx::B2DPolygon aPoly;
            aPoly.insert(0, basegfx::B2DPoint( aRect.Left(), aRect.Top() ) );
            aPoly.insert(1, basegfx::B2DPoint( aRect.Right(), aRect.Top() ) );

            basegfx::B2DHomMatrix aMatrix;
            aMatrix.translate( 0.0, static_cast< double >( aRect.GetHeight() / 7 ) );

            basegfx::B2DPolyPolygon aPathPoly;
            for( sal_uInt16 nLine = 0; nLine < 7; nLine++ )
            {
                aPoly.transform( aMatrix );
                aPathPoly.append( aPoly );
            }

            SdrPathObj* pPathObj = new SdrPathObj(OBJ_PATHLINE, aPathPoly );
            pPathObj->SetMergedItem( XLineStyleItem(XLINE_SOLID) );
            pPathObj->SetMergedItem( XLineColorItem(String(), Color(COL_BLACK)));

            pHandout->NbcInsertObject( pPathObj );
        }
    }
}

} // end of namespace sd
