/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PrintManager.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 10:32:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "PrintManager.hxx"

#include <svx/editstat.hxx>
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#include "Outliner.hxx"
#include "Window.hxx"
#include "FrameView.hxx"
#include <sfx2/progress.hxx>
#include <svtools/printdlg.hxx>
#include <tools/multisel.hxx>
#include <svtools/misccfg.hxx>
#include <unotools/localedatawrapper.hxx>
#ifndef _SVX_PRTQRY_HXX
#include <svx/prtqry.hxx>
#endif
#ifndef SD_OUTPUT_DEVICE_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#include "sdresid.hxx"
#include <svx/svdetc.hxx>
#ifndef _SVDOPAGE_HXX
#include <svx/svdopage.hxx>
#endif
#include <svx/svdpagv.hxx>
#include "strings.hrc"
#include "sdabstdlg.hxx"
#include "printdlg.hrc"
#include "prntopts.hrc"
#include "app.hrc"
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

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
    USHORT nDiffFlags)
{
    return SetPrinterOptDlg (pNewPrinter,nDiffFlags);
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

        ViewShell* pShell = mrBase.GetMainViewShell();
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
    PrintDialog* pDlg;

    pDlg = new PrintDialog(pParent );

    ViewShell* pShell = mrBase.GetMainViewShell();
    if (pShell!=NULL && ! pShell->ISA(OutlineViewShell))
    {

        if (pShell->ISA(DrawViewShell))
        {
            pDlg->SetRangeText(UniString::CreateFromInt32(
                static_cast<DrawViewShell*>(pShell)->GetCurPageId()));
        }
        else //if( this->ISA( SlideViewShell ) )
        {
            String aStrRange(
                static_cast<SlideViewShell*>(pShell)->GetPageRangeString());
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

    return pDlg;
}





SfxTabPage*  PrintManager::CreatePrintOptionsPage(
    ::Window *pParent,
    const SfxItemSet &rOptions)
{
    DocumentType eDocType = mrBase.GetDocument()->GetDocumentType();
    //CHINA001 SdPrintOptions* pPage = new SdPrintOptions( pParent, rOptions );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( TP_PRINT_OPTIONS );
    DBG_ASSERT(fnCreatePage, "SdAbstractDialogFactory fail!");//CHINA001
    SfxTabPage* pPage = (*fnCreatePage)( pParent, rOptions );
    if( eDocType == DOCUMENT_TYPE_DRAW )
    { //add by CHINA001
        //CHINA001 pPage->SetDrawMode();
        SfxAllItemSet aSet(*(rOptions.GetPool())); //CHINA001
        aSet.Put (SfxUInt32Item(SID_SDMODE_FLAG,SD_DRAW_MODE));
        pPage->PageCreated(aSet);
    }
    return pPage;
}




USHORT  PrintManager::Print (SfxProgress& rProgress, PrintDialog* pDlg)
{
    SfxPrinter* pPrinter = mrBase.GetPrinter(TRUE);
    ViewShell* pShell = mrBase.GetMainViewShell();

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
        SdOptionsPrintItem* pPrintOpts = NULL;
        if (pPrinter->GetOptions().GetItemState(
            ATTR_OPTIONS_PRINT, FALSE,
            (const SfxPoolItem**) &pPrintOpts ) != SFX_ITEM_SET )
        {
            pPrintOpts = NULL;
        }

        // Wenn wir im Gliederungsmodus sind, muss das Model auf Stand gebracht werden
        if (pShell->ISA(OutlineViewShell))
            static_cast<OutlineViewShell*>(pShell)->PrepareClose (FALSE, FALSE);

        // Basisklasse rufen, um Basic anzusprechen
        mrBase.SfxViewShell::Print( rProgress, pDlg );

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

        Orientation eOldOrientation = pPrinter->GetOrientation();

        if( pPrintOpts )
        {
            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();

            if( pPrintOpts->IsDate() )
            {
                aTimeDateStr += GetSdrGlobalData().pLocaleData->getDate( Date() );
                aTimeDateStr += (sal_Unicode)' ';
            }

            if( pPrintOpts->IsTime() )
                aTimeDateStr += GetSdrGlobalData().pLocaleData->getTime( Time(), FALSE, FALSE );

            if( pPrintOpts->IsOutline() )
                bPrintOutline = TRUE;

            if( pPrintOpts->IsHandout() )
                bPrintHandout = TRUE;

            if( pPrintOpts->IsDraw() )
                bPrintDraw = TRUE;

            if( pPrintOpts->IsNotes() )
            {
                bPrintNotes = TRUE;
                ePageKind = PK_NOTES;
            }

            pPrintOpts->SetWarningPrinter( pMisc->IsNotFoundWarning() );
            pPrintOpts->SetWarningSize( pMisc->IsPaperSizeWarning() );
            pPrintOpts->SetWarningOrientation( pMisc->IsPaperOrientationWarning() );

            UINT16  nQuality = pPrintOpts->GetOutputQuality();
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
                bPrintExcluded = pPrintOpts->IsHiddenPages();

            for( USHORT j = nPage; ( j < nPageMax && !bContainsTransparency ); j++ )
            {
                if( aPrintSelection.IsSelected( j + 1 ) )
                {
                    SdPage* pPage = mrBase.GetDocument()->GetSdPage(
                        j, PK_STANDARD );

                    if( pPage && ( !pPage->IsExcluded() || bPrintExcluded ) )
                    {
                        if( !( bContainsTransparency = pPage->HasTransparentObjects() ) )
                        {
                            if(pPage->TRG_HasMasterPage())
                            {
                                bContainsTransparency = pPage->TRG_GetMasterPage().HasTransparentObjects();
                            }
                        }
                    }
                }
            }
        }

        if (pPrinter->InitJob (mrBase.GetWindow(), bContainsTransparency))
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
                    PrintHandout(aInfo, nPage);
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
    BOOL bSilent)
{
    ErrCode nResult = ERRCODE_NONE;

    do
    {
        ViewShell* pShell = mrBase.GetMainViewShell();
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
            && !bSilent
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
                pPrinter, pPrintDialog, bSilent);

            RestrictPrintingToSelection (FALSE);
        }
    }
    while(false);

    return nResult;
}




void PrintManager::PreparePrint (PrintDialog* pPrintDialog)
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
            SdPage* pPage = mrBase.GetDocument()->GetSdPage(0, PK_HANDOUT);

            // Papierschacht
            if (!pPrintOpts->IsPaperbin()) // Drucken NICHT aus Druckereinstellung
            {
                pPrinter->SetPaperBin(pPage->GetPaperBin());
            }

            pPrinter->SetOrientation(pPage->TRG_GetMasterPage().GetOrientation());
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

                SdPage* pPage = mrBase.GetDocument()->GetSdPage(0, ePageKind);
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
            bScalePage = pPrintOpts->IsPagesize();
            bPrintBooklet = pPrintOpts->IsBooklet();
            pPrintOpts->SetCutPage( FALSE );
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
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");
            AbstractSdPrintDlg* pDlg = pFact->CreateSdPrintDlg(ResId( DLG_PRINT_WARNINGS ), mrBase.GetWindow() );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            // Do not show the dialog when the bSilent flag is set.  We do
            // create the dialog anyway so that we can extract the default
            // method of mapping internal pages to printer pages from it.
            if ( ! bSilent)
                nRet = pDlg->Execute();
            if( nRet == RET_OK )
            {
                switch (pDlg->GetAttr())
                {
                    case 1:
                        pPrintOpts->SetPagesize();
                        break;

                    case 2:
                        // Tiling is the default behaviour in
                        // ViewShell::PrintStdOrNotes().  The poperty of
                        // pPrintOpts is ignored there so setting it here
                        // may only lead to unwanted side effects.
                        break;

                    case 3:
                        pPrintOpts->SetCutPage();
                        break;
                }
                bContinuePrinting = true;
            }
            delete pDlg;
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
    if( pPrintOpts && !pPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
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
        bPrintExcluded = rInfo.mpPrintOpts->IsHiddenPages();

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

                nPageCount += rInfo.mnCopies;

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




void PrintManager::PrintHandout (
    PrintInfo& rInfo,
    USHORT nPage)
{
    SdPage* pPage = rInfo.mrViewShell.GetDoc()->GetSdPage(0, PK_HANDOUT);
    SdPage& rMaster = (SdPage&)pPage->TRG_GetMasterPage();
    SdDrawDocument* pDocument = rInfo.mrViewShell.GetDoc();

    BOOL        bScalePage = TRUE;
    if ( rInfo.mpPrintOpts )
    {
        bScalePage = rInfo.mpPrintOpts->IsPagesize();
    }

    // Papierschacht
    if( rInfo.mpPrintOpts && !rInfo.mpPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
    {
        USHORT nPaperBin = pPage->GetPaperBin();
        rInfo.mrPrinter.SetPaperBin( nPaperBin );
    }

    // Hoch/Querformat aendern?
    Orientation eOrientation = rMaster.GetOrientation();

    short nDlgResult = RET_OK;

    if ( !rInfo.mrPrinter.SetOrientation(eOrientation) &&
        (!rInfo.mpPrintOpts || rInfo.mpPrintOpts->IsWarningOrientation()) )
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

        sd::ShapeList& rShapeList = rMaster.GetPresentationShapeList();
        USHORT  nPageCount = rInfo.mnProgressOffset;

        rInfo.mrViewShell.WriteFrameViewData();

        rInfo.mrViewShell.SetPrintedHandoutPageNum(0);

        if ( rInfo.mpPrintOpts )
            bPrintExcluded = rInfo.mpPrintOpts->IsHiddenPages();

        while ( nPage < rInfo.mnPageMax )
        {
            // Anzahl ALLER Seiten im Dokument:
            USHORT nAbsPageCnt = pDocument->GetPageCount();

            SdrObject* pIter = rShapeList.getNextShape(0);
            while( pIter && (nPage < rInfo.mnPageMax) )
            {
                SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(pIter);
                if( pPageObj && (rMaster.GetPresObjKind(pPageObj) == PRESOBJ_HANDOUT) )
                {
                    if (rInfo.mrSelPages.IsSelected(nPage+1) )
                    {
                        //rProgress.SetState( nPageCount, nTotal );
                        //rProgress.SetStateText( nPageCount, nPage+1, nTotal );

                        String aTmp = UniString::CreateFromInt32( nPage+1 );
                        aTmp += String( SdResId( STR_PRINT_HANDOUT ) );
                        rInfo.mrProgress.SetStateText( nPageCount, aTmp, rInfo.mnTotal );

                        nPageCount += rInfo.mnCopies;

                        SdPage* pPg = pDocument->GetSdPage(nPage, PK_STANDARD);

                        if ( !pPg->IsExcluded() || bPrintExcluded )
                        {
                            (pPageObj)->SetReferencedPage(pPg);
                            pIter = rShapeList.getNextShape(pIter);
                        }
                    }
                    nPage++;
                }
                else
                {
                    pIter = rShapeList.getNextShape(pIter);
                }
            }

            while( pIter )
            {
                SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(pIter);
                if( pPageObj && (rMaster.GetPresObjKind(pPageObj) == PRESOBJ_HANDOUT) )
                    pPageObj->SetReferencedPage(0L);

                pIter = rShapeList.getNextShape(pIter);
            }

            rInfo.mrViewShell.SetPrintedHandoutPageNum(rInfo.mrViewShell.GetPrintedHandoutPageNum() + 1);

            rInfo.mrPrinter.StartPage();
            pPrintView->ShowPage(pPage, Point());

            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
            pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
            pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

            pPrintView->CompleteRedraw(&rInfo.mrPrinter, Rectangle(Point(0,0),
                                pPage->GetSize()));

            if ( rInfo.mrTimeDateStr.Len() )
            {
                Font aOldFont = rInfo.mrPrinter.OutputDevice::GetFont();
                rInfo.mrPrinter.SetFont(rInfo.mrTimeDateFont);
                rInfo.mrPrinter.DrawText(aPageOfs, rInfo.mrTimeDateStr);
                rInfo.mrPrinter.SetFont(aOldFont);
            }
            rInfo.mrPrinter.EndPage();
            pPrintView->HidePage(pPrintView->GetPageView(pPage));
        }

        USHORT nRealPage = 0;
        SdrObject* pIter = 0;
        while( pIter = rShapeList.getNextShape(pIter) )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(pIter);
            if( pPageObj && (rMaster.GetPresObjKind(pPageObj) == PRESOBJ_HANDOUT) )
            {
                SdPage* pRealPage = 0;
                if( nRealPage < pDocument->GetSdPageCount( PK_STANDARD ) )
                    pRealPage = pDocument->GetSdPage(nRealPage++,PK_STANDARD);
                pPageObj->SetReferencedPage(pRealPage);
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

    SdPage* pPage = pDocument->GetSdPage(nPage, ePageKind);

    if ( rInfo.mpPrintOpts )
    {
        bScalePage = rInfo.mpPrintOpts->IsPagesize();
        bTilePage = rInfo.mpPrintOpts->IsPagetile();
        bPrintPageName = rInfo.mpPrintOpts->IsPagename();
        bPrintExcluded = rInfo.mpPrintOpts->IsHiddenPages();
        bPrintBooklet = rInfo.mpPrintOpts->IsBooklet();
        bPrintFrontPage = rInfo.mpPrintOpts->IsFrontPage();
        bPrintBackPage = rInfo.mpPrintOpts->IsBackPage();

        // Papierschacht
        if( !rInfo.mpPrintOpts->IsPaperbin() ) // Drucken NICHT aus Druckereinstellung
        {
            USHORT nPaperBin = pDocument->GetSdPage(nPage, ePageKind)->GetPaperBin();
            rInfo.mrPrinter.SetPaperBin( nPaperBin );
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

    if ( !rInfo.mrPrinter.SetOrientation(eOrientation) &&
        (!rInfo.mpPrintOpts || rInfo.mpPrintOpts->IsWarningOrientation()) )
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
            SdPage*                                         pPage;
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
                    SdPage* pPage = pDocument->GetSdPage( nPage, ePageKind );

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

                    rInfo.mrPrinter.StartPage();

                    pPage = pDocument->GetSdPage( aPair.first, ePageKind );

                    if( pPage )
                    {
                        aMap.SetOrigin( aOffset );
                        rInfo.mrPrinter.SetMapMode( aMap );
                        PrintPage(rInfo, pPrintView, pPage, bPrintMarkedOnly );
                    }

                    pPage = pDocument->GetSdPage( aPair.second, ePageKind );

                    if( pPage )
                    {
                        Point aSecondOffset( aOffset );

                        if( eOrientation == ORIENTATION_LANDSCAPE )
                            aSecondOffset.X() += ( aPrintSize.Width() >> 1 );
                        else
                            aSecondOffset.Y() += ( aPrintSize.Height() >> 1 );

                        aMap.SetOrigin( aSecondOffset );
                        rInfo.mrPrinter.SetMapMode( aMap );
                        PrintPage(rInfo, pPrintView, pPage, bPrintMarkedOnly );
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
                    nPageCount += rInfo.mnCopies;

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
                            pPrintView->ShowPage( pPage, aPtZero );

                            SdrPageView* pPageView = pPrintView->GetPageView(pPage);
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
                            pPrintView->HidePage(pPrintView->GetPageView(pPage));

                            bPrint = FALSE;
                        }

                        // For pages larger then the printable area there
                        // are three options:
                        // 1. Scale down to the page to the printable area.
                        // 2. Print only the upper left part of the page.
                        // 3. Split the page into parts of the size of the
                        // printable area.
                        if (bPrint)
                        {
                            if (bScalePage
                                || (rInfo.mpPrintOpts && rInfo.mpPrintOpts->IsCutPage()))
                            {
                                // Handlee 1 and 2.
                                PrintPagePart(
                                    rInfo,
                                    pPage,
                                    *pPrintView,
                                    Point(0,0),
                                    bPrintMarkedOnly,
                                    aPageStr,
                                    aPageOfs);
                            }
                            else
                            {
                                // Handle 3.  Print parts of the page in the
                                // size of the printable area until the
                                // whole page is covered.
                                for (aPageOrigin = Point(0,0);
                                     -aPageOrigin.Y()<aPageHeight;
                                         aPageOrigin.Y() -= aPrintHeight)
                                {
                                    for (aPageOrigin.X()=0;
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
    rPrintView.ShowPage(pPage, Point(0,0));

    // Set origin of the map mode so that the next part of the page is
    // printed.
    MapMode aOriginalMapMode = rInfo.mrPrinter.GetMapMode();
    MapMode aMapMode = rInfo.mrPrinter.GetMapMode();
    aMapMode.SetOrigin(rPageOrigin);
    rInfo.mrPrinter.SetMapMode(aMapMode);

    SdrPageView* pPageView = rPrintView.GetPageView(pPage);
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
    rPrintView.HidePage(rPrintView.GetPageView(pPage));
}





void PrintManager::PrintPage (
    PrintInfo& rInfo,
    ::sd::View* pPrintView,
    SdPage* pPage,
    BOOL bPrintMarkedOnly)
{
    Point aPtZero;
    pPrintView->ShowPage( pPage, aPtZero );

    SdrPageView* pPageView = pPrintView->GetPageView( pPage );
    pPageView->SetVisibleLayers(rInfo.mrViewShell.GetFrameView()->GetVisibleLayers() );
    pPageView->SetPrintableLayers(rInfo.mrViewShell.GetFrameView()->GetPrintableLayers() );

    if (rInfo.mrViewShell.ISA(DrawViewShell) && bPrintMarkedOnly)
        rInfo.mrViewShell.GetView()->DrawAllMarked(rInfo.mrPrinter, Point(0,0));
    else
        pPrintView->CompleteRedraw(
            &rInfo.mrPrinter,
            Rectangle(Point(0,0), pPage->GetSize()));

    pPrintView->HidePage( pPrintView->GetPageView( pPage ) );
}

} // end of namespace sd
