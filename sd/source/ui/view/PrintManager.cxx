/*************************************************************************
 *
 *  $RCSfile: PrintManager.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:52:08 $
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

#include "PrintManager.hxx"

#include <svx/editstat.hxx>
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#include "sdattr.hxx"
#include "sdpage.hxx"
//CHINA001 #include "printdlg.hxx"
#include "drawdoc.hxx"
//CHINA001 #include "prntopts.hxx"
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
#include <sfx2/progress.hxx>
#include <svtools/printdlg.hxx>
#include <tools/multisel.hxx>
#include <svtools/misccfg.hxx>
#include <unotools/localedatawrapper.hxx>
//CHINA001 #ifndef _SD_PRNTOPTS_HXX
//CHINA001 #include "prntopts.hxx"
//CHINA001 #endif
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
#include "strings.hrc"
#include "sdabstdlg.hxx" //CHINA001
#include "printdlg.hrc" //CHINA001
#include "prntopts.hrc" //CHINA001
#include "app.hrc" //CHINA001
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001
namespace sd {

PrintManager::PrintManager (ViewShellBase& rViewShell)
    : mrViewShell (rViewShell)
{
}




PrintManager::~PrintManager (void)
{
}




SfxPrinter* PrintManager::GetPrinter (BOOL bCreate)
{
    return mrViewShell.GetDocShell()->GetPrinter (bCreate);
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
    mrViewShell.GetDocShell()->SetPrinter(pNewPrinter);

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
                mrViewShell.GetWindow(),
                (WinBits)(WB_YES_NO | WB_DEF_YES),
                String(SdResId(STR_SCALE_OBJS_TO_PAGE)));
            bScaleAll = (aWarnBox.Execute() == RET_YES);
        }

        ViewShell* pShell = mrViewShell.GetMainViewShell();
        if (pShell != NULL && pShell->ISA(DrawViewShell))
        {
            SdPage* pPage = mrViewShell.GetDocument()->GetSdPage(
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

    ViewShell* pShell = mrViewShell.GetMainViewShell();
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
    DocumentType eDocType = mrViewShell.GetDocument()->GetDocumentType();
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
    SfxPrinter* pPrinter = mrViewShell.GetPrinter(TRUE);
    ViewShell* pShell = mrViewShell.GetMainViewShell();

    if( pPrinter )
    {
        const PrinterOptions    aOldPrinterOptions( pPrinter->GetPrinterOptions() );
        MapMode                 aMap( pPrinter->GetMapMode() );
        const MapMode           aOldMap( aMap );
        USHORT                  nOldPaperBin = pPrinter->GetPaperBin();

        aMap.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMap);

        ::Outliner& rOutliner = mrViewShell.GetDocument()->GetDrawOutliner();
        ULONG nOldCntrl = rOutliner.GetControlWord();
        ULONG nCntrl = nOldCntrl;
        nCntrl |= EE_CNTRL_NOREDLINES;
        nCntrl &= ~EE_CNTRL_MARKFIELDS;
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
        rOutliner.SetControlWord( nCntrl );

        // Pruefen des Seitenformates und ggfs. Dialog hochbringen
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

        SdPage* pPage = mrViewShell.GetDocument()->GetSdPage( 0, PK_STANDARD );
        Size    aPageSize( pPage->GetSize() );
        Size    aPrintSize( pPrinter->GetOutputSize() );
        long    nPageWidth = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
        long    nPageHeight = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
        long    nPrintWidth = aPrintSize.Width();
        long    nPrintHeight = aPrintSize.Height();
        USHORT  nRet = RET_OK;

        if( !bScalePage && !bTilePage && !bPrintBooklet &&
            ( ( nPageWidth > nPrintWidth || nPageHeight > nPrintHeight ) &&
              ( nPageWidth > nPrintHeight || nPageHeight > nPrintWidth ) ) )
        {
            //CHINA001 SdPrintDlg aDlg (mrViewShell.GetWindow());
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
            AbstractSdPrintDlg* pDlg = pFact->CreateSdPrintDlg(ResId( DLG_PRINT_WARNINGS ), mrViewShell.GetWindow() );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            nRet = pDlg->Execute(); //CHINA001 nRet = aDlg.Execute();
            if( nRet == RET_OK )
            {
                USHORT nOption = pDlg->GetAttr(); //CHINA001 USHORT nOption = aDlg.GetAttr();

                if( nOption == 1 )
                    pPrintOpts->SetPagesize();

                // ( nOption == 2 ) ist der Default

                if( nOption == 3 )
                    pPrintOpts->SetCutPage();
            }
            delete pDlg; //add by CHINA001
        }

        if( nRet == RET_CANCEL )
        {
            pPrinter->SetPrinterOptions( aOldPrinterOptions );
            pPrinter->SetMapMode( aOldMap );
            return 0;
        }

        // Wenn wir im Gliederungsmodus sind, muss das Model auf Stand gebracht werden
        if (pShell->ISA(OutlineViewShell))
            static_cast<OutlineViewShell*>(pShell)->PrepareClose (FALSE, FALSE);

        // Basisklasse rufen, um Basic anzusprechen
        mrViewShell.SfxViewShell::Print( rProgress, pDlg );

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

        nPageMax = mrViewShell.GetDocument()->GetSdPageCount( ePageKind );
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
                    SdPage* pPage = mrViewShell.GetDocument()->GetSdPage(
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

        if (pPrinter->InitJob (mrViewShell.GetWindow(), bContainsTransparency))
        {
            for( USHORT n = 1; n <= nCollateCopies; n++ )
            {
                if ( bPrintOutline )
                {
                    // siehe unten in PrintOutline()
                    pPrinter->SetPaperBin( nOldPaperBin );

                    pShell->PrintOutline(*pPrinter, rProgress, aPrintSelection,
                                  aTimeDateStr, aTimeDateFont, pPrintOpts,
                                  nPage, nPageMax,
                                  nCollateCopies > 1 ? 1 : nCopies,
                                  nProgressOffset, nTotal );
                    nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
                }

                if ( bPrintHandout )
                {
                    pShell->PrintHandout(*pPrinter, rProgress, aPrintSelection,
                                  aTimeDateStr, aTimeDateFont, pPrintOpts,
                                  nPage, nPageMax,
                                  nCollateCopies > 1 ? 1 : nCopies,
                                  nProgressOffset, nTotal );
                    nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
                }
                if( bPrintDraw )
                {
                    pShell->PrintStdOrNotes(*pPrinter, rProgress, aPrintSelection,
                                     aTimeDateStr, aTimeDateFont, pPrintOpts,
                                     nPage, nPageMax,
                                     nCollateCopies > 1 ? 1 : nCopies,
                                     nProgressOffset, nTotal,
                                     PK_STANDARD, bPrintMarkedOnly);
                    nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
                }
                if( bPrintNotes )
                {
                    pShell->PrintStdOrNotes (*pPrinter, rProgress, aPrintSelection,
                                     aTimeDateStr, aTimeDateFont, pPrintOpts,
                                     nPage, nPageMax,
                                     nCollateCopies > 1 ? 1 : nCopies,
                                     nProgressOffset, nTotal,
                                     PK_NOTES, FALSE);
                    nProgressOffset += ( nSelectCount * ( nCollateCopies > 1 ? 1 : nCopies) );
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

    ViewShell* pShell = mrViewShell.GetMainViewShell();
    if (pShell != NULL)
    {
        const SdrMarkList& rMarkList = pShell->GetView()->GetMarkedObjectList();

        // Retrieve the range of marked pages.
        String sNewPageRange (msPageRange);

        RestrictPrintingToSelection (FALSE);

        // #105477# Don't show query dialog if print dialog has been shown
        if ( !pPrintDialog
            && !bSilent
            && (rMarkList.GetMarkCount() || sNewPageRange.Len()) )
        {
            SvxPrtQryBox aQuery (mrViewShell.GetWindow());

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
                pWindowUpdater->Update (pPrinter, mrViewShell.GetDocument());

            //  Forward the call to SfxViewShell::DoPrint() which
            //  eventually calls Print() (after StartJob etc.)
            nResult = mrViewShell.SfxViewShell::DoPrint (
                pPrinter, pPrintDialog, bSilent);

            RestrictPrintingToSelection (FALSE);
        }
    }

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
            SdPage* pPage = mrViewShell.GetDocument()->GetSdPage(0, PK_HANDOUT);

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

                SdPage* pPage = mrViewShell.GetDocument()->GetSdPage(0, ePageKind);
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


} // end of namespace sd
