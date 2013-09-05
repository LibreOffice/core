/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdotext.hxx>
#include <svl/style.hxx>
#include <sfx2/printer.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/sfxecode.hxx>
#include <comphelper/classids.hxx>
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
#include "sdmod.hxx"
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

/**
 * Creates (if necessary) and returns a SfxPrinter
 */
SfxPrinter* DrawDocShell::GetPrinter(sal_Bool bCreate)
{
    if (bCreate && !mpPrinter)
    {
        // create ItemSet with special pool area
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                            0 );
        // set PrintOptionsSet
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

        // set output quality
        sal_uInt16 nQuality = aPrintItem.GetOptionsPrint().GetOutputQuality();

        sal_uLong nMode = DRAWMODE_DEFAULT;
        // 1 == Grayscale, 2 == Black & White (with grayscale images)
        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_WHITEFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

        mpPrinter->SetDrawMode( nMode );

        MapMode aMM (mpPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        mpPrinter->SetMapMode(aMM);
        UpdateRefDevice();
    }
    return mpPrinter;
}

/**
 * Set new SfxPrinter (transfer of ownership)
 */
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
        // Since we do not have RTTI we use a hard cast (...)
        SetPrinter((SfxPrinter*) pNewPrinter);

        // container owns printer
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

/**
 * Creates new document, opens streams
 */
sal_Bool DrawDocShell::InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    sal_Bool bRet = SfxObjectShell::InitNew( xStorage );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {
        if( !mbSdDataObj )
            mpDoc->NewOrLoadCompleted(NEW_DOC);  // otherwise calling
                                                // NewOrLoadCompleted(NEW_LOADED) in
                                                // SdDrawDocument::AllocModel()
    }
    return bRet;
}

/**
 * loads pools and document
 */
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
        //pStore->SetError( SVSTREAM_WRONGVERSION, OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
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

/**
 * loads content for organizer
 */
sal_Bool DrawDocShell::LoadFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    WaitObject* pWait = NULL;
    if( mpViewShell )
        pWait = new WaitObject( (Window*) mpViewShell->GetActiveWindow() );

    mpDoc->NewOrLoadCompleted( NEW_DOC );
    mpDoc->CreateFirstPages();
    mpDoc->StopWorkStartupDelay();

    // TODO/LATER: nobody is interested in the error code?!
    ErrCode nError = ERRCODE_NONE;
    sal_Bool bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Organizer, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );


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

/**
 * load from 3rd party format
 */
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

/**
 * load from a foreign format
 */
sal_Bool DrawDocShell::ConvertFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    const OUString    aFilterName( rMedium.GetFilter()->GetFilterName() );
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

/**
 * Writes pools and document to the open streams
 */
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

/**
 * Writes pools and document to the provided storage
 */
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

/**
 * save to foreign format
 */
sal_Bool DrawDocShell::ConvertTo( SfxMedium& rMedium )
{
    sal_Bool bRet = sal_False;

    if( mpDoc->GetPageCount() )
    {
        const SfxFilter*    pMediumFilter = rMedium.GetFilter();
        const OUString aTypeName( pMediumFilter->GetTypeName() );
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

/**
 * Reopen own streams to ensure that nobody else can prevent use from opening
 * them.
 */
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

SdDrawDocument* DrawDocShell::GetDoc()
{
    return mpDoc;
}

SfxStyleSheetBasePool* DrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) mpDoc->GetStyleSheetPool() );
}

sal_Bool DrawDocShell::GotoBookmark(const OUString& rBookmark)
{
    sal_Bool bFound = sal_False;

    if (mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(mpViewShell);
        ViewShellBase& rBase (mpViewShell->GetViewShellBase());

        sal_Bool bIsMasterPage = sal_False;
        sal_uInt16 nPageNumber = SDRPAGE_NOTFOUND;
        SdrObject* pObj = NULL;

        OUString sBookmark( rBookmark );
        const OUString sInteraction( "action?" );
        if ( sBookmark.match( sInteraction ) )
        {
            const OUString sJump( "jump=" );
            if ( sBookmark.match( sJump, sInteraction.getLength() ) )
            {
                OUString aDestination( sBookmark.copy( sInteraction.getLength() + sJump.getLength() ) );
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
            OUString aBookmark( rBookmark );

            // Is the bookmark a page?
            nPageNumber = mpDoc->GetPageByName( aBookmark, bIsMasterPage );

            if (nPageNumber == SDRPAGE_NOTFOUND)
            {
                // Is the bookmark a object?
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
                // change work area
                GetFrameView()->SetPageKind(eNewPageKind);
                OUString sViewURL;
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
                    // show and select object
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

/**
 * If it should become a document template.
 */
sal_Bool DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{

    const SfxFilter* pFilter = rMedium.GetFilter();

    if (pFilter->IsOwnTemplateFormat())
    {
        /* now the StarDraw specialty:
           we assign known layout names to the layout template of the first
           page, we set the layout names of the affected masterpages and pages.
           We inform all text objects of the affected standard, note and
           masterpages about the name change.
        */

        OUString aLayoutName;

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

        if (!aLayoutName.isEmpty())
        {
            sal_uInt32 nCount = mpDoc->GetMasterSdPageCount(PK_STANDARD);
            for (sal_uInt32 i = 0; i < nCount; ++i)
            {
                OUString aOldPageLayoutName = mpDoc->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName();
                OUString aNewLayoutName = aLayoutName;
                // Don't add suffix for the first master page
                if( i > 0 )
                    aNewLayoutName += OUString::number(i);

                mpDoc->RenameLayoutTemplate(aOldPageLayoutName, aNewLayoutName);
            }
        }
    }

    return SfxObjectShell::SaveAsOwnFormat(rMedium);
}


void DrawDocShell::FillClass(SvGlobalName* pClassName,
                                        sal_uInt32*  pFormat,
                                        OUString* ,
                                        OUString* pFullTypeName,
                                        OUString* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
    if (nFileFormat == SOFFICE_FILEFORMAT_60)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
                *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                *pFormat = SOT_FORMATSTR_ID_STARDRAW_60;
                *pFullTypeName = OUString(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_60));
        }
        else
        {
                *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_60;
                *pFullTypeName = OUString(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_60));
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

    *pShortTypeName = OUString(SdResId( (meDocType == DOCUMENT_TYPE_DRAW) ?
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
void DrawDocShell::OpenBookmark( const OUString& rBookmarkURL )
{
    SfxStringItem   aStrItem( SID_FILE_NAME, rBookmarkURL );
    SfxStringItem   aReferer( SID_REFERER, GetMedium()->GetName() );
    const SfxPoolItem* ppArgs[] = { &aStrItem, &aReferer, 0 };
    ( mpViewShell ? mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings().Execute( SID_OPENHYPERLINK, ppArgs );
}

SfxDocumentInfoDialog* DrawDocShell::CreateDocumentInfoDialog( ::Window *pParent, const SfxItemSet &rSet )
{
    SfxDocumentInfoDialog* pDlg   = new SfxDocumentInfoDialog( pParent, rSet );
    DrawDocShell*          pDocSh = PTR_CAST(DrawDocShell,SfxObjectShell::Current());

    if( pDocSh == this )
    {
        pDlg->AddFontTabPage();
    }
    return pDlg;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
