/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "DrawDocShell.hxx"
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <tools/urlobj.hxx>
#include <sfx2/progress.hxx>
#include <vcl/waitobj.hxx>
#include <svx/svxids.hrc>
#include <editeng/flstitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/flagitem.hxx>
#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdotext.hxx>
#include <svl/style.hxx>
#include <sfx2/printer.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/sfxecode.hxx>
#include <sot/clsids.hxx>
#include <sot/formats.hxx>
#include <sfx2/request.hxx>
#include <unotools/fltrcfg.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/saveopt.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <comphelper/processfactory.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "strmname.h"
#include "FrameView.hxx"
#include "optsitem.hxx"
#include "Outliner.hxx"
#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "app.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "sdmod.hxx"
#include "OutlineViewShell.hxx"
#include "sdxmlwrp.hxx"
#include "sdpptwrp.hxx"
#include "sdcgmfilter.hxx"
#include "sdgrffilter.hxx"
#include "sdhtmlfilter.hxx"
#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::sd::framework::FrameworkHelper;


namespace sd {

/*************************************************************************
|*
|* SfxPrinter ggf. erzeugen und zurueckgeben
|*
\************************************************************************/

SfxPrinter* DrawDocShell::GetPrinter(sal_Bool bCreate)
{
    if (bCreate && !mpPrinter)
    {
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                            0 );
        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT,
                            SD_MOD()->GetSdOptions(mpDoc->GetDocumentType()));
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        sal_uInt16      nFlags = 0;

        nFlags =  (aPrintItem.GetOptionsPrint().IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.GetOptionsPrint().IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.GetOptionsPrint().IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        mpPrinter = new SfxPrinter(pSet);
        mbOwnPrinter = sal_True;

        // Ausgabequalitaet setzen
        sal_uInt16 nQuality = aPrintItem.GetOptionsPrint().GetOutputQuality();

        sal_uLong nMode = DRAWMODE_DEFAULT;

        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_WHITEFILL | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT;

        mpPrinter->SetDrawMode( nMode );

        MapMode aMM (mpPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        mpPrinter->SetMapMode(aMM);
        UpdateRefDevice();
    }
    return mpPrinter;
}

/*************************************************************************
|*
|* neuen SfxPrinter setzen (Eigentuemeruebergang)
|*
\************************************************************************/

void DrawDocShell::SetPrinter(SfxPrinter *pNewPrinter)
{
    if ( mpViewShell )
    {
        ::sd::View* pView = mpViewShell->GetView();
        if ( pView->IsTextEdit() )
            pView->SdrEndTextEdit();
    }

    if ( mpPrinter && mbOwnPrinter && (mpPrinter != pNewPrinter) )
    {
        delete mpPrinter;
    }

    mpPrinter = pNewPrinter;
    mbOwnPrinter = sal_True;
    if ( mpDoc->GetPrinterIndependentLayout() == ::com::sun::star::document::PrinterIndependentLayout::DISABLED )
        UpdateFontList();
    UpdateRefDevice();
}

void DrawDocShell::UpdateFontList()
{
    delete mpFontList;
    OutputDevice* pRefDevice = NULL;
    if ( mpDoc->GetPrinterIndependentLayout() == ::com::sun::star::document::PrinterIndependentLayout::DISABLED )
        pRefDevice = GetPrinter(sal_True);
    else
        pRefDevice = SD_MOD()->GetVirtualRefDevice();
    mpFontList = new FontList( pRefDevice, NULL, sal_False );
    SvxFontListItem aFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );
}

Printer* DrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(sal_False);
}

void DrawDocShell::OnDocumentPrinterChanged(Printer* pNewPrinter)
{
    // if we already have a printer, see if its the same
    if( mpPrinter )
    {
        // easy case
        if( mpPrinter == pNewPrinter )
            return;

        // compare if its the same printer with the same job setup
        if( (mpPrinter->GetName() == pNewPrinter->GetName()) &&
            (mpPrinter->GetJobSetup() == pNewPrinter->GetJobSetup()))
            return;
    }

    //  if (mpPrinter->IsA(SfxPrinter))
    {
        // Da kein RTTI verfuegbar, wird hart gecasted (...)
        SetPrinter((SfxPrinter*) pNewPrinter);

        // Printer gehoert dem Container
        mbOwnPrinter = sal_False;
    }
}

void DrawDocShell::UpdateRefDevice()
{
    if( mpDoc )
    {
        // Determine the device for which the output will be formatted.
        OutputDevice* pRefDevice = NULL;
        switch (mpDoc->GetPrinterIndependentLayout())
        {
            case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
                pRefDevice = mpPrinter;
                break;

            case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
                pRefDevice = SD_MOD()->GetVirtualRefDevice();
                break;

            default:
                // We are confronted with an invalid or un-implemented
                // layout mode.  Use the printer as formatting device
                // as a fall-back.
                DBG_ASSERT(false, "DrawDocShell::UpdateRefDevice(): Unexpected printer layout mode");

                pRefDevice = mpPrinter;
                break;
        }
        mpDoc->SetRefDevice( pRefDevice );

        ::sd::Outliner* pOutl = mpDoc->GetOutliner( sal_False );

        if( pOutl )
            pOutl->SetRefDevice( pRefDevice );

        ::sd::Outliner* pInternalOutl = mpDoc->GetInternalOutliner( sal_False );

        if( pInternalOutl )
            pInternalOutl->SetRefDevice( pRefDevice );
    }
}

/*************************************************************************
|*
|* InitNew, (Dokument wird neu erzeugt): Streams oeffnen
|*
\************************************************************************/

sal_Bool DrawDocShell::InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    sal_Bool bRet = sal_False;

    bRet = SfxObjectShell::InitNew( xStorage );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {
        mpDoc->SetDrawingLayerPoolDefaults();
        if( !mbSdDataObj )
            mpDoc->NewOrLoadCompleted(NEW_DOC);  // otherwise calling
                                                // NewOrLoadCompleted(NEW_LOADED) in
                                                // SdDrawDocument::AllocModel()
    }
    return bRet;
}

/*************************************************************************
|*
|* Load: Pools und Dokument laden
|*
\************************************************************************/

sal_Bool DrawDocShell::Load( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    sal_Bool    bRet = sal_False;
    bool    bStartPresentation = false;
    ErrCode nError = ERRCODE_NONE;

    SfxItemSet* pSet = rMedium.GetItemSet();


    if( pSet )
    {
        if( (  SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW ) ) && ( (SfxBoolItem&) ( pSet->Get( SID_PREVIEW ) ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( sal_True );
        }

        if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    bRet = SfxObjectShell::Load( rMedium );
    if( bRet )
    {
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );
    }

    if( bRet )
    {
        UpdateTablePointers();

        // If we're an embedded OLE object, use tight bounds
        // for our visArea. No point in showing the user lots of empty
        // space. Had to remove the check for empty VisArea below,
        // since XML load always sets a VisArea before.
        //TODO/LATER: looks a little bit strange!
        if( ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ) && SfxObjectShell::GetVisArea( ASPECT_CONTENT ).IsEmpty() )
        {
            SdPage* pPage = mpDoc->GetSdPage( 0, PK_STANDARD );

            if( pPage )
                SetVisArea( Rectangle( pPage->GetAllObjBoundRect() ) );
        }

        FinishedLoading( SFX_LOADED_ALL );

        const INetURLObject aUrl;
        SfxObjectShell::SetAutoLoad( aUrl, 0, sal_False );
    }
    else
    {
        if( nError == ERRCODE_IO_BROKENPACKAGE )
            SetError( ERRCODE_IO_BROKENPACKAGE, OSL_LOG_PREFIX );

        // TODO/LATER: correct error handling?!
        //pStore->SetError( SVSTREAM_WRONGVERSION, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        else
            SetError( ERRCODE_ABORT, OSL_LOG_PREFIX );
    }

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() || bStartPresentation )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();
        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, bStartPresentation ? 1 : 5 ) );
    }

    return bRet;
}

/*************************************************************************
|*
|* LoadFrom: Inhalte fuer Organizer laden
|*
\************************************************************************/

sal_Bool DrawDocShell::LoadFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    WaitObject* pWait = NULL;
    if( mpViewShell )
        pWait = new WaitObject( (Window*) mpViewShell->GetActiveWindow() );

    sal_Bool bRet = sal_False;

        mpDoc->NewOrLoadCompleted( NEW_DOC );
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();

        // TODO/LATER: nobody is interested in the error code?!
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Organizer, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );


    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pSet = GetMedium()->GetItemSet();

        if( pSet )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }

    delete pWait;

    return bRet;
}

/*************************************************************************
|*
|* ImportFrom: load from 3rd party format
|*
\************************************************************************/

sal_Bool DrawDocShell::ImportFrom( SfxMedium &rMedium, bool bInsert )
{
    const sal_Bool bRet=SfxObjectShell::ImportFrom(rMedium, bInsert);

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
        {
            mpDoc->SetStartWithPresentation( true );

            // tell SFX to change viewshell when in preview mode
            if( IsPreview() )
            {
                SfxItemSet *pMediumSet = GetMedium()->GetItemSet();
                if( pMediumSet )
                    pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, 1 ) );
            }
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* ConvertFrom: aus Fremdformat laden
|*
\************************************************************************/

sal_Bool DrawDocShell::ConvertFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    const rtl::OUString    aFilterName( rMedium.GetFilter()->GetFilterName() );
    sal_Bool            bRet = sal_False;
    bool    bStartPresentation = false;

    SetWaitCursor( sal_True );

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( (  SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW ) ) && ( (SfxBoolItem&) ( pSet->Get( SID_PREVIEW ) ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( sal_True );
        }

        if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    if( aFilterName == pFilterPowerPoint97
        || aFilterName == pFilterPowerPoint97Template
        || aFilterName == pFilterPowerPoint97AutoPlay)
    {
        mpDoc->StopWorkStartupDelay();
        bRet = SdPPTFilter( rMedium, *this, sal_True ).Import();
    }
    else if (aFilterName.indexOf("impress8") >= 0 ||
             aFilterName.indexOf("draw8") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True ).Import( nError );

    }
    else if (aFilterName.indexOf("StarOffice XML (Draw)") >= 0 ||
             aFilterName.indexOf("StarOffice XML (Impress)") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 ).Import( nError );
    }
    else if( aFilterName.equals( "CGM - Computer Graphics Metafile" ) )
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdCGMFilter( rMedium, *this, sal_True ).Import();
    }
    else
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdGRFFilter( rMedium, *this ).Import();
    }

    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();

        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }
    SetWaitCursor( sal_False );

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() || bStartPresentation )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();
        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, bStartPresentation ? 1 : 5 ) );
    }

    return bRet;
}

/*************************************************************************
|*
|* Save: Pools und Dokument in die offenen Streams schreiben
|*
\************************************************************************/

sal_Bool DrawDocShell::Save()
{
    mpDoc->StopWorkStartupDelay();

    //TODO/LATER: why this?!
    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    sal_Bool bRet = SfxObjectShell::Save();

    if( bRet )
    {
        // Call UpdateDocInfoForSave() before export
        UpdateDocInfoForSave();

        bRet = SdXMLFilter( *GetMedium(), *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( GetMedium()->GetStorage() ) ).Export();
    }

    return bRet;
}

/*************************************************************************
|*
|* SaveAs: Pools und Dokument in den angegebenen Storage sichern
|*
\************************************************************************/

sal_Bool DrawDocShell::SaveAs( SfxMedium& rMedium )
{
    mpDoc->StopWorkStartupDelay();

    //TODO/LATER: why this?!
    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    sal_uInt32  nVBWarning = ERRCODE_NONE;
    sal_Bool    bRet = SfxObjectShell::SaveAs( rMedium );

    if( bRet )
    {
        // Call UpdateDocInfoForSave() before export
        UpdateDocInfoForSave();
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Export();
    }

    if( GetError() == ERRCODE_NONE )
        SetError( nVBWarning, OSL_LOG_PREFIX );

    return bRet;
}

/*************************************************************************
|*
|* ConvertTo: im Fremdformat speichern
|*
\************************************************************************/

sal_Bool DrawDocShell::ConvertTo( SfxMedium& rMedium )
{
    sal_Bool bRet = sal_False;

    if( mpDoc->GetPageCount() )
    {
        const SfxFilter*    pMediumFilter = rMedium.GetFilter();
        const rtl::OUString aTypeName( pMediumFilter->GetTypeName() );
        SdFilter*           pFilter = NULL;

        if( aTypeName.indexOf( "graphic_HTML" ) >= 0 )
        {
            pFilter = new SdHTMLFilter( rMedium, *this, sal_True );
        }
        else if( aTypeName.indexOf( "MS_PowerPoint_97" ) >= 0 )
        {
            pFilter = new SdPPTFilter( rMedium, *this, sal_True );
            ((SdPPTFilter*)pFilter)->PreSaveBasic();
        }
        else if ( aTypeName.indexOf( "CGM_Computer_Graphics_Metafile" ) >= 0 )
        {
            pFilter = new SdCGMFilter( rMedium, *this, sal_True );
        }
        else if( aTypeName.indexOf( "draw8" ) >= 0 ||
                 aTypeName.indexOf( "impress8" ) >= 0 )
        {
            pFilter = new SdXMLFilter( rMedium, *this, sal_True );
            UpdateDocInfoForSave();
        }
        else if( aTypeName.indexOf( "StarOffice_XML_Impress" ) >= 0 ||
                 aTypeName.indexOf( "StarOffice_XML_Draw" ) >= 0 )
        {
            pFilter = new SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 );
            UpdateDocInfoForSave();
        }
        else
        {
            pFilter = new SdGRFFilter( rMedium, *this );
        }

        if( pFilter )
        {
            const sal_uLong nOldSwapMode = mpDoc->GetSwapGraphicsMode();

            mpDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

            bRet = pFilter->Export();
            if( !bRet )
                mpDoc->SetSwapGraphicsMode( nOldSwapMode );

            delete pFilter;
        }
    }

    return  bRet;
}

/*************************************************************************
|*
|* SaveCompleted: die eigenen Streams wieder oeffnen, damit kein anderer
|*                                sie "besetzt"
|*
\************************************************************************/

sal_Bool DrawDocShell::SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    sal_Bool bRet = sal_False;

    if( SfxObjectShell::SaveCompleted(xStorage) )
    {
        mpDoc->NbcSetChanged( sal_False );

        if( mpViewShell )
        {
            if( mpViewShell->ISA( OutlineViewShell ) )
                static_cast<OutlineView*>(mpViewShell->GetView())
                    ->GetOutliner()->ClearModifyFlag();

            SdrOutliner* pOutl = mpViewShell->GetView()->GetTextEditOutliner();
            if( pOutl )
            {
                SdrObject* pObj = mpViewShell->GetView()->GetTextEditObject();
                if( pObj )
                    pObj->NbcSetOutlinerParaObject( pOutl->CreateParaObject() );

                pOutl->ClearModifyFlag();
            }
        }

        bRet = sal_True;

        SfxViewFrame* pFrame = ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                               mpViewShell->GetViewFrame() :
                               SfxViewFrame::Current();

        if( pFrame )
            pFrame->GetBindings().Invalidate( SID_NAVIGATOR_STATE, sal_True, sal_False );
    }
    return bRet;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SdDrawDocument* DrawDocShell::GetDoc()
{
    return mpDoc;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SfxStyleSheetBasePool* DrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) mpDoc->GetStyleSheetPool() );
}

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

sal_Bool DrawDocShell::GotoBookmark(const String& rBookmark)
{
    sal_Bool bFound = sal_False;

    if (mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(mpViewShell);
        ViewShellBase& rBase (mpViewShell->GetViewShellBase());

        sal_Bool bIsMasterPage = sal_False;
        sal_uInt16 nPageNumber = SDRPAGE_NOTFOUND;
        SdrObject* pObj = NULL;

        rtl::OUString sBookmark( rBookmark );
        const rtl::OUString sInteraction( "action?" );
        if ( sBookmark.match( sInteraction ) )
        {
            const rtl::OUString sJump( "jump=" );
            if ( sBookmark.match( sJump, sInteraction.getLength() ) )
            {
                rtl::OUString aDestination( sBookmark.copy( sInteraction.getLength() + sJump.getLength() ) );
                if ( aDestination.match( "firstslide" ) )
                {
                    nPageNumber = 1;
                }
                else if ( aDestination.match( "lastslide" ) )
                {
                    nPageNumber = mpDoc->GetPageCount() - 2;
                }
                else if ( aDestination.match( "previousslide" ) )
                {
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    nPageNumber = pPage->GetPageNum();
                    nPageNumber = nPageNumber > 2 ? nPageNumber - 2 : SDRPAGE_NOTFOUND;
                }
                else if ( aDestination.match( "nextslide" ) )
                {
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    nPageNumber = pPage->GetPageNum() + 2;
                    if ( nPageNumber >= mpDoc->GetPageCount() )
                        nPageNumber = SDRPAGE_NOTFOUND;
                }
            }
        }
        else
        {
            String aBookmark( rBookmark );

            // Ist das Bookmark eine Seite?
            nPageNumber = mpDoc->GetPageByName( aBookmark, bIsMasterPage );

            if (nPageNumber == SDRPAGE_NOTFOUND)
            {
                // Ist das Bookmark ein Objekt?
                pObj = mpDoc->GetObj(aBookmark);

                if (pObj)
                {
                    nPageNumber = pObj->GetPage()->GetPageNum();
                }
            }
        }
        if (nPageNumber != SDRPAGE_NOTFOUND)
        {
            // Jump to the bookmarked page.  This is done in three steps.

            bFound = sal_True;
            SdPage* pPage;
            if (bIsMasterPage)
                pPage = (SdPage*) mpDoc->GetMasterPage(nPageNumber);
            else
                pPage = (SdPage*) mpDoc->GetPage(nPageNumber);

            // 1.) Change the view shell to the edit view, the notes view,
            // or the handout view.
            PageKind eNewPageKind = pPage->GetPageKind();

            if( (eNewPageKind != PK_STANDARD) && (mpDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW) )
                return sal_False;

            if (eNewPageKind != pDrawViewShell->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ::rtl::OUString sViewURL;
                switch (eNewPageKind)
                {
                    case PK_STANDARD:
                        sViewURL = FrameworkHelper::msImpressViewURL;
                        break;
                    case PK_NOTES:
                        sViewURL = FrameworkHelper::msNotesViewURL;
                        break;
                    case PK_HANDOUT:
                        sViewURL = FrameworkHelper::msHandoutViewURL;
                        break;
                    default:
                        break;
                }
                if (!sViewURL.isEmpty())
                {
                    ::boost::shared_ptr<FrameworkHelper> pHelper (
                        FrameworkHelper::Instance(rBase));
                    pHelper->RequestView(
                        sViewURL,
                        FrameworkHelper::msCenterPaneURL);
                    pHelper->WaitForUpdate();

                    // Get the new DrawViewShell.
                    mpViewShell = pHelper->GetViewShell(FrameworkHelper::msCenterPaneURL).get();
                    pDrawViewShell = dynamic_cast<sd::DrawViewShell*>(mpViewShell);
                }
                else
                {
                    pDrawViewShell = NULL;
                }
            }

            if (pDrawViewShell != NULL)
            {
                // Set the edit mode to either the normal edit mode or the
                // master page mode.
                EditMode eNewEditMode = EM_PAGE;
                if (bIsMasterPage)
                {
                    eNewEditMode = EM_MASTERPAGE;
                }

                if (eNewEditMode != pDrawViewShell->GetEditMode())
                {
                    // EditMode setzen
                    pDrawViewShell->ChangeEditMode(eNewEditMode, sal_False);
                }

                // Make the bookmarked page the current page.  This is done
                // by using the API because this takes care of all the
                // little things to be done.  Especially writing the view
                // data to the frame view.
                sal_uInt16 nSdPgNum = (nPageNumber - 1) / 2;
                Reference<drawing::XDrawView> xController (rBase.GetController(), UNO_QUERY);
                if (xController.is())
                {
                    Reference<drawing::XDrawPage> xDrawPage (pPage->getUnoPage(), UNO_QUERY);
                    xController->setCurrentPage (xDrawPage);
                }
                else
                {
                    // As a fall back switch to the page via the core.
                    DBG_ASSERT (xController.is(),
                        "DrawDocShell::GotoBookmark: can't switch page via API");
                    pDrawViewShell->SwitchPage(nSdPgNum);
                }

                if (pObj != NULL)
                {
                    // Objekt einblenden und selektieren
                    pDrawViewShell->MakeVisible(pObj->GetLogicRect(),
                        *pDrawViewShell->GetActiveWindow());
                    pDrawViewShell->GetView()->UnmarkAll();
                    pDrawViewShell->GetView()->MarkObj(
                        pObj,
                        pDrawViewShell->GetView()->GetSdrPageView(), sal_False);
                }
            }
        }

        SfxBindings& rBindings = (pDrawViewShell->GetViewFrame()!=NULL
            ? pDrawViewShell->GetViewFrame()
            : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, sal_True, sal_False);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

/*************************************************************************
|*
|* SaveAsOwnFormat: wenn es eine Dokumentvorlage werden soll,
|*
\************************************************************************/

sal_Bool DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{

    const SfxFilter* pFilter = rMedium.GetFilter();

    if (pFilter->IsOwnTemplateFormat())
    {
        // jetzt die StarDraw-Spezialitaeten:
        // die Layoutvorlagen der ersten Seite werden mit dem jetzt
        // bekannten Layoutnamen versehen, die Layoutnamen der betroffenen
        // Masterpages und Seiten werden gesetzt;
        // alle Textobjekte der betroffenen Standard-, Notiz- und
        // Masterpages werden ueber die Namensaenderung informiert

        String aLayoutName;

        SfxStringItem* pLayoutItem;
        if( rMedium.GetItemSet()->GetItemState(SID_TEMPLATE_NAME, sal_False, (const SfxPoolItem**) & pLayoutItem ) == SFX_ITEM_SET )
        {
            aLayoutName = pLayoutItem->GetValue();
        }
        else
        {
            INetURLObject aURL( rMedium.GetName() );
            aURL.removeExtension();
            aLayoutName = aURL.getName();
        }

        if( aLayoutName.Len() )
        {
            String aOldPageLayoutName = mpDoc->GetSdPage(0, PK_STANDARD)->GetLayoutName();
            mpDoc->RenameLayoutTemplate(aOldPageLayoutName, aLayoutName);
        }
    }

    return SfxObjectShell::SaveAsOwnFormat(rMedium);
}

/*************************************************************************
|*
|* FillClass
|*
\************************************************************************/

void DrawDocShell::FillClass(SvGlobalName* pClassName,
                                        sal_uInt32*  pFormat,
                                        String* ,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
    if (nFileFormat == SOFFICE_FILEFORMAT_60)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
                *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                *pFormat = SOT_FORMATSTR_ID_STARDRAW_60;
                *pFullTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_60));
        }
        else
        {
                *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_60;
                *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_60));
        }
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_8)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
                *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                *pFormat = bTemplate ? SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE : SOT_FORMATSTR_ID_STARDRAW_8;
                *pFullTypeName = "Draw 8"; // HACK: method will be removed with new storage API
        }
        else
        {
                *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                *pFormat = bTemplate ? SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE : SOT_FORMATSTR_ID_STARIMPRESS_8;
                *pFullTypeName = "Impress 8"; // HACK: method will be removed with new storage API
        }
    }

    *pShortTypeName = String(SdResId( (meDocType == DOCUMENT_TYPE_DRAW) ?
                                      STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT ));
}

OutputDevice* DrawDocShell::GetDocumentRefDev (void)
{
    OutputDevice* pReferenceDevice = SfxObjectShell::GetDocumentRefDev ();
    // Only when our parent does not have a reference device then we return
    // our own.
    if (pReferenceDevice == NULL && mpDoc != NULL)
        pReferenceDevice = mpDoc->GetRefDevice ();
    return pReferenceDevice;
}

/** executes the SID_OPENDOC slot to let the framework open a document
    with the given URL and this document as a referer */
void DrawDocShell::OpenBookmark( const String& rBookmarkURL )
{
    SfxStringItem   aStrItem( SID_FILE_NAME, rBookmarkURL );
    SfxStringItem   aReferer( SID_REFERER, GetMedium()->GetName() );
    const SfxPoolItem* ppArgs[] = { &aStrItem, &aReferer, 0 };
    ( mpViewShell ? mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings().Execute( SID_OPENHYPERLINK, ppArgs );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
