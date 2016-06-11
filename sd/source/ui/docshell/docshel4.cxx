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
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
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
#include "CustomAnimationEffect.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "OutlineViewShell.hxx"
#include "sdxmlwrp.hxx"
#include "sdpptwrp.hxx"
#include "sdcgmfilter.hxx"
#include "sdgrffilter.hxx"
#include "sdhtmlfilter.hxx"
#include "framework/FrameworkHelper.hxx"

#include "SdUnoDrawView.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::sd::framework::FrameworkHelper;

namespace sd {

/**
 * Creates (if necessary) and returns a SfxPrinter
 */
SfxPrinter* DrawDocShell::GetPrinter(bool bCreate)
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
        SfxPrinterChangeFlags nFlags =
                (aPrintItem.GetOptionsPrint().IsWarningSize() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE) |
                (aPrintItem.GetOptionsPrint().IsWarningOrientation() ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE);
        aFlagItem.SetValue( static_cast<int>(nFlags) );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.GetOptionsPrint().IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        mpPrinter = VclPtr<SfxPrinter>::Create(pSet);
        mbOwnPrinter = true;

        // set output quality
        sal_uInt16 nQuality = aPrintItem.GetOptionsPrint().GetOutputQuality();

        DrawModeFlags nMode = DrawModeFlags::Default;
        // 1 == Grayscale, 2 == Black & White (with grayscale images)
        if( nQuality == 1 )
            nMode = DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText | DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient;
        else if( nQuality == 2 )
            nMode = DrawModeFlags::BlackLine | DrawModeFlags::WhiteFill | DrawModeFlags::BlackText | DrawModeFlags::GrayBitmap | DrawModeFlags::WhiteGradient;

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

    if ( mpPrinter && mbOwnPrinter && (mpPrinter.get() != pNewPrinter) )
        mpPrinter.disposeAndClear();

    mpPrinter = pNewPrinter;
    mbOwnPrinter = true;
    if ( mpDoc->GetPrinterIndependentLayout() == css::document::PrinterIndependentLayout::DISABLED )
        UpdateFontList();
    UpdateRefDevice();
}

void DrawDocShell::UpdateFontList()
{
    delete mpFontList;
    OutputDevice* pRefDevice = nullptr;
    if ( mpDoc->GetPrinterIndependentLayout() == css::document::PrinterIndependentLayout::DISABLED )
        pRefDevice = GetPrinter(true);
    else
        pRefDevice = SD_MOD()->GetVirtualRefDevice();
    mpFontList = new FontList( pRefDevice, nullptr, false );
    SvxFontListItem aFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );
}

Printer* DrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(false);
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

    //  if 0 != dynamic_cast< SfxPrinter *>( (mpPrinter ))
    {
        // Since we do not have RTTI we use a hard cast (...)
        SetPrinter(static_cast<SfxPrinter*>(pNewPrinter));

        // container owns printer
        mbOwnPrinter = false;
    }
}

void DrawDocShell::UpdateRefDevice()
{
    if( mpDoc )
    {
        // Determine the device for which the output will be formatted.
        VclPtr< OutputDevice > pRefDevice;
        switch (mpDoc->GetPrinterIndependentLayout())
        {
            case css::document::PrinterIndependentLayout::DISABLED:
                pRefDevice = mpPrinter.get();
                break;

            case css::document::PrinterIndependentLayout::ENABLED:
                pRefDevice = SD_MOD()->GetVirtualRefDevice();
                break;

            default:
                // We are confronted with an invalid or un-implemented
                // layout mode.  Use the printer as formatting device
                // as a fall-back.
                SAL_WARN( "sd", "DrawDocShell::UpdateRefDevice(): Unexpected printer layout mode");

                pRefDevice = mpPrinter.get();
                break;
        }
        mpDoc->SetRefDevice( pRefDevice.get() );

        ::sd::Outliner* pOutl = mpDoc->GetOutliner( false );

        if( pOutl )
            pOutl->SetRefDevice( pRefDevice );

        ::sd::Outliner* pInternalOutl = mpDoc->GetInternalOutliner( false );

        if( pInternalOutl )
            pInternalOutl->SetRefDevice( pRefDevice );
    }
}

/**
 * Creates new document, opens streams
 */
bool DrawDocShell::InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage )
{
    bool bRet = SfxObjectShell::InitNew( xStorage );

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
bool DrawDocShell::Load( SfxMedium& rMedium )
{
    mbNewDocument = false;

    bool       bRet = false;
    bool       bStartPresentation = false;
    ErrCode nError = ERRCODE_NONE;

    SfxItemSet* pSet = rMedium.GetItemSet();

    if( pSet )
    {
        if( (  SfxItemState::SET == pSet->GetItemState(SID_PREVIEW ) ) && static_cast<const SfxBoolItem&>( pSet->Get( SID_PREVIEW ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            static_cast<const SfxBoolItem&>( pSet->Get( SID_DOC_STARTPRESENTATION ) ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    bRet = SfxObjectShell::Load( rMedium );
    if( bRet )
    {
        bRet = SdXMLFilter( rMedium, *this, true, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );
    }

    if( bRet )
    {
        UpdateTablePointers();

        // If we're an embedded OLE object, use tight bounds
        // for our visArea. No point in showing the user lots of empty
        // space. Had to remove the check for empty VisArea below,
        // since XML load always sets a VisArea before.
        //TODO/LATER: looks a little bit strange!
        if( ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED ) && SfxObjectShell::GetVisArea( ASPECT_CONTENT ).IsEmpty() )
        {
            SdPage* pPage = mpDoc->GetSdPage( 0, PK_STANDARD );

            if( pPage )
                SetVisArea( Rectangle( pPage->GetAllObjBoundRect() ) );
        }

        FinishedLoading();

        const INetURLObject aUrl;
        SfxObjectShell::SetAutoLoad( aUrl, 0, false );
    }
    else
    {
        if( nError == ERRCODE_IO_BROKENPACKAGE )
            SetError( ERRCODE_IO_BROKENPACKAGE, OSL_LOG_PREFIX );

        // TODO/LATER: correct error handling?!
        //pStore->SetError( SVSTREAM_WRONGVERSION, OUString( OSL_LOG_PREFIX ) );
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
bool DrawDocShell::LoadFrom( SfxMedium& rMedium )
{
    mbNewDocument = false;

    WaitObject* pWait = nullptr;
    if( mpViewShell )
        pWait = new WaitObject( static_cast<vcl::Window*>(mpViewShell->GetActiveWindow()) );

    mpDoc->NewOrLoadCompleted( NEW_DOC );
    mpDoc->CreateFirstPages();
    mpDoc->StopWorkStartupDelay();

    // TODO/LATER: nobody is interested in the error code?!
    ErrCode nError = ERRCODE_NONE;
    bool bRet = SdXMLFilter( rMedium, *this, true, SDXMLMODE_Organizer, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );

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
bool DrawDocShell::ImportFrom(SfxMedium &rMedium,
        uno::Reference<text::XTextRange> const& xInsertPosition)
{
    const OUString aFilterName( rMedium.GetFilter()->GetFilterName() );
    if( aFilterName == "Impress MS PowerPoint 2007 XML" ||
        aFilterName == "Impress MS PowerPoint 2007 XML AutoPlay" )
    {
        // As this is a MSFT format, we should use the "MS Compat"
        // mode for spacing before and after paragraphs.

        // This is copied from what is done for .ppt import in
        // ImplSdPPTImport::Import() in sd/source/filter/ppt/pptin.cxx
        // in. We need to tell both the edit engine of the draw outliner,
        // and the document, to do "summation of paragraphs".
        SdrOutliner& rOutl = mpDoc->GetDrawOutliner();
        EEControlBits nControlWord = rOutl.GetEditEngine().GetControlWord();
        nControlWord |=  EEControlBits::ULSPACESUMMATION;
        nControlWord &=~ EEControlBits::ULSPACEFIRSTPARA;
        ((EditEngine&)rOutl.GetEditEngine()).SetControlWord( nControlWord );

        mpDoc->SetSummationOfParagraphs();
    }

    const bool bRet = SfxObjectShell::ImportFrom(rMedium, xInsertPosition);

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            static_cast<const SfxBoolItem&>( pSet->Get( SID_DOC_STARTPRESENTATION ) ).GetValue() )
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
bool DrawDocShell::ConvertFrom( SfxMedium& rMedium )
{
    mbNewDocument = false;

    const OUString  aFilterName( rMedium.GetFilter()->GetFilterName() );
    bool            bRet = false;
    bool            bStartPresentation = false;

    SetWaitCursor( true );

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( (  SfxItemState::SET == pSet->GetItemState(SID_PREVIEW ) ) && static_cast<const SfxBoolItem&>( pSet->Get( SID_PREVIEW ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            static_cast<const SfxBoolItem&>( pSet->Get( SID_DOC_STARTPRESENTATION ) ).GetValue() )
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
        bRet = SdPPTFilter( rMedium, *this, true ).Import();
    }
    else if (aFilterName.indexOf("impress8") >= 0 ||
             aFilterName.indexOf("draw8") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, true ).Import( nError );

    }
    else if (aFilterName.indexOf("StarOffice XML (Draw)") >= 0 ||
             aFilterName.indexOf("StarOffice XML (Impress)") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, true, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 ).Import( nError );
    }
    else if( aFilterName == "CGM - Computer Graphics Metafile" )
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdCGMFilter( rMedium, *this, true ).Import();
    }
    else
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdGRFFilter( rMedium, *this ).Import();
    }

    FinishedLoading();

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();

        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }
    SetWaitCursor( false );

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
bool DrawDocShell::Save()
{
    mpDoc->StopWorkStartupDelay();

    //TODO/LATER: why this?!
    if( GetCreateMode() == SfxObjectCreateMode::STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    bool bRet = SfxObjectShell::Save();

    if( bRet )
        bRet = SdXMLFilter( *GetMedium(), *this, true, SDXMLMODE_Normal, SotStorage::GetVersion( GetMedium()->GetStorage() ) ).Export();

    return bRet;
}

/**
 * Writes pools and document to the provided storage
 */
bool DrawDocShell::SaveAs( SfxMedium& rMedium )
{
    mpDoc->setDocAccTitle(OUString());
    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        vcl::Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            vcl::Window* pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                pSysWin->SetAccessibleName(OUString());
            }
        }
    }
    mpDoc->StopWorkStartupDelay();

    //With custom animation, if Outliner is modified, update text before saving
    if( mpViewShell )
    {
        SdPage* pPage = mpViewShell->getCurrentPage();
        if( pPage && pPage->getMainSequence()->getCount() )
        {
            SdrObject* pObj = mpViewShell->GetView()->GetTextEditObject();
            SdrOutliner* pOutl = mpViewShell->GetView()->GetTextEditOutliner();
            if( pObj && pOutl && pOutl->IsModified() )
            {
                OutlinerParaObject* pNewText = pOutl->CreateParaObject( 0, pOutl->GetParagraphCount() );
                pObj->SetOutlinerParaObject( pNewText );
                pOutl->ClearModifyFlag();
            }
        }
    }

    //TODO/LATER: why this?!
    if( GetCreateMode() == SfxObjectCreateMode::STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    sal_uInt32  nVBWarning = ERRCODE_NONE;
    bool    bRet = SfxObjectShell::SaveAs( rMedium );

    if( bRet )
        bRet = SdXMLFilter( rMedium, *this, true, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Export();

    if( GetError() == ERRCODE_NONE )
        SetError( nVBWarning, OSL_LOG_PREFIX );

    return bRet;
}

/**
 * save to foreign format
 */
bool DrawDocShell::ConvertTo( SfxMedium& rMedium )
{
    bool bRet = false;

    if( mpDoc->GetPageCount() )
    {
        const SfxFilter*    pMediumFilter = rMedium.GetFilter();
        const OUString aTypeName( pMediumFilter->GetTypeName() );
        SdFilter*           pFilter = nullptr;

        if( aTypeName.indexOf( "graphic_HTML" ) >= 0 )
        {
            pFilter = new SdHTMLFilter( rMedium, *this, true );
        }
        else if( aTypeName.indexOf( "MS_PowerPoint_97" ) >= 0 )
        {
            pFilter = new SdPPTFilter( rMedium, *this, true );
            static_cast<SdPPTFilter*>(pFilter)->PreSaveBasic();
        }
        else if ( aTypeName.indexOf( "CGM_Computer_Graphics_Metafile" ) >= 0 )
        {
            pFilter = new SdCGMFilter( rMedium, *this, true );
        }
        else if( aTypeName.indexOf( "draw8" ) >= 0 ||
                 aTypeName.indexOf( "impress8" ) >= 0 )
        {
            pFilter = new SdXMLFilter( rMedium, *this, true );
        }
        else if( aTypeName.indexOf( "StarOffice_XML_Impress" ) >= 0 ||
                 aTypeName.indexOf( "StarOffice_XML_Draw" ) >= 0 )
        {
            pFilter = new SdXMLFilter( rMedium, *this, true, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 );
        }
        else
        {
            pFilter = new SdGRFFilter( rMedium, *this );
        }

        if( pFilter )
        {
            const SdrSwapGraphicsMode nOldSwapMode = mpDoc->GetSwapGraphicsMode();

            mpDoc->SetSwapGraphicsMode( SdrSwapGraphicsMode::TEMP );

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
bool DrawDocShell::SaveCompleted( const css::uno::Reference< css::embed::XStorage >& xStorage )
{
    bool bRet = false;

    if( SfxObjectShell::SaveCompleted(xStorage) )
    {
        mpDoc->NbcSetChanged( false );

        if( mpViewShell )
        {
            if( dynamic_cast< OutlineViewShell *>( mpViewShell ) !=  nullptr )
                static_cast<OutlineView*>(mpViewShell->GetView())
                    ->GetOutliner().ClearModifyFlag();

            SdrOutliner* pOutl = mpViewShell->GetView()->GetTextEditOutliner();
            if( pOutl )
            {
                SdrObject* pObj = mpViewShell->GetView()->GetTextEditObject();
                if( pObj )
                    pObj->NbcSetOutlinerParaObject( pOutl->CreateParaObject() );

                pOutl->ClearModifyFlag();
            }
        }

        bRet = true;

        SfxViewFrame* pFrame = ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                               mpViewShell->GetViewFrame() :
                               SfxViewFrame::Current();

        if( pFrame )
            pFrame->GetBindings().Invalidate( SID_NAVIGATOR_STATE, true );
    }
    return bRet;
}

SfxStyleSheetBasePool* DrawDocShell::GetStyleSheetPool()
{
    return mpDoc->GetStyleSheetPool();
}

bool DrawDocShell::GotoBookmark(const OUString& rBookmark)
{
    bool bFound = false;

    if (mpViewShell && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
    {
        DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(mpViewShell);
        ViewShellBase& rBase (mpViewShell->GetViewShellBase());

        bool bIsMasterPage = false;
        sal_uInt16 nPageNumber = SDRPAGE_NOTFOUND;
        SdrObject* pObj = nullptr;

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

            bFound = true;
            SdPage* pPage;
            if (bIsMasterPage)
                pPage = static_cast<SdPage*>( mpDoc->GetMasterPage(nPageNumber) );
            else
                pPage = static_cast<SdPage*>( mpDoc->GetPage(nPageNumber) );

            // 1.) Change the view shell to the edit view, the notes view,
            // or the handout view.
            PageKind eNewPageKind = pPage->GetPageKind();

            if( (eNewPageKind != PK_STANDARD) && (mpDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW) )
                return false;

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
                    std::shared_ptr<FrameworkHelper> pHelper (
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
                    pDrawViewShell = nullptr;
                }
            }

            if (pDrawViewShell != nullptr)
            {
                setEditMode(pDrawViewShell, bIsMasterPage);

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

                if (pObj != nullptr)
                {
                    // show and select object
                    pDrawViewShell->MakeVisible(pObj->GetLogicRect(),
                        *pDrawViewShell->GetActiveWindow());
                    pDrawViewShell->GetView()->UnmarkAll();
                    pDrawViewShell->GetView()->MarkObj(
                        pObj,
                        pDrawViewShell->GetView()->GetSdrPageView());
                }
            }
        }

        SfxBindings& rBindings = ((pDrawViewShell && pDrawViewShell->GetViewFrame()!=nullptr)
            ? pDrawViewShell->GetViewFrame()
            : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, true);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

// If object is marked return true else return false.
bool DrawDocShell::IsMarked( SdrObject* pObject )
{
    bool bisMarked =false;

    if (mpViewShell && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
    {
        DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>( mpViewShell );
        if (pObject )
        {
              bisMarked = pDrViewSh->GetView()->IsObjMarked(pObject);
        }
     }
    return  bisMarked;
}

// If object is marked return true else return false.
bool DrawDocShell::GetObjectIsmarked(const OUString& rBookmark)
{
    OSL_TRACE("GotoBookmark %s",
        OUStringToOString(rBookmark, RTL_TEXTENCODING_UTF8).getStr());
    bool bUnMark = false;

    if (mpViewShell && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
    {
        DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>( mpViewShell );

        OUString aBookmark( rBookmark );

        if( rBookmark.startsWith("#") )
            aBookmark = rBookmark.copy( 1 );

        // Is the bookmark a page ?
        bool        bIsMasterPage;
        sal_uInt16  nPgNum = mpDoc->GetPageByName( aBookmark, bIsMasterPage );
        SdrObject*  pObj = nullptr;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // Is the bookmark an object ?
            pObj = mpDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->GetPage()->GetPageNum();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /********************
             * Skip to the page *
             ********************/
            SdPage* pPage = static_cast<SdPage*>( mpDoc->GetPage(nPgNum) );

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // change workspace
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                  mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SfxCallMode::SYNCHRON | SfxCallMode::RECORD );

                // The current ViewShell changed
                pDrViewSh = static_cast<DrawViewShell*>( mpViewShell );
            }

            setEditMode(pDrViewSh, bIsMasterPage);

            // Jump to the page.  This is done by using the API because this
            // takes care of all the little things to be done.  Especially
            // writing the view data to the frame view (see bug #107803#).
            SdUnoDrawView* pUnoDrawView = new SdUnoDrawView (
                *pDrViewSh,
                *pDrViewSh->GetView());
            css::uno::Reference<css::drawing::XDrawPage> xDrawPage( pPage->getUnoPage(), css::uno::UNO_QUERY);
            pUnoDrawView->setCurrentPage (xDrawPage);
            delete pUnoDrawView;

            if (pObj)
            {
                // Show and select object
                pDrViewSh->MakeVisible(pObj->GetLogicRect(),
                                       *pDrViewSh->GetActiveWindow());

                bUnMark = pDrViewSh->GetView()->IsObjMarked(pObj);
            }
        }
    }

    return ( bUnMark);
}

// realize multi-selection of objects
bool DrawDocShell::GotoTreeBookmark(const OUString& rBookmark)
{
    OSL_TRACE("GotoBookmark %s",
        OUStringToOString(rBookmark, RTL_TEXTENCODING_UTF8).getStr());
    bool bFound = false;

    if (mpViewShell && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
    {
        DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>( mpViewShell );

        OUString aBookmark( rBookmark );

        if( rBookmark.startsWith("#") )
            aBookmark = rBookmark.copy( 1 );

        // is the bookmark a page ?
        bool        bIsMasterPage;
        sal_uInt16  nPgNum = mpDoc->GetPageByName( aBookmark, bIsMasterPage );
        SdrObject*  pObj = nullptr;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // is the bookmark an object ?
            pObj = mpDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->GetPage()->GetPageNum();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /********************
             * Skip to the page *
             ********************/
            bFound = true;
            SdPage* pPage = static_cast<SdPage*>( mpDoc->GetPage(nPgNum) );

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // change workspace
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                  mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SfxCallMode::SYNCHRON | SfxCallMode::RECORD );

                // The current ViewShell changed
                pDrViewSh = static_cast<DrawViewShell*>( mpViewShell );
            }

            setEditMode(pDrViewSh, bIsMasterPage);

            // Jump to the page.  This is done by using the API because this
            // takes care of all the little things to be done.  Especially
            // writing the view data to the frame view (see bug #107803#).
            SdUnoDrawView* pUnoDrawView = new SdUnoDrawView (
                *pDrViewSh,
                *pDrViewSh->GetView());
            css::uno::Reference<css::drawing::XDrawPage> xDrawPage( pPage->getUnoPage(), css::uno::UNO_QUERY);
            pUnoDrawView->setCurrentPage (xDrawPage);
            delete pUnoDrawView;

            if (pObj)
            {
                // Show and select object
                pDrViewSh->MakeVisible(pObj->GetLogicRect(),
                                       *pDrViewSh->GetActiveWindow());
                bool bUnMark = pDrViewSh->GetView()->IsObjMarked(pObj);
                pDrViewSh->GetView()->MarkObj(pObj, pDrViewSh->GetView()->GetSdrPageView(), bUnMark);
            }
        }

        SfxBindings& rBindings = ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                                 mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, true);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

/**
 * If it should become a document template.
 */
bool DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
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

        SfxStringItem const * pLayoutItem;
        if( rMedium.GetItemSet()->GetItemState(SID_TEMPLATE_NAME, false, reinterpret_cast<const SfxPoolItem**>(& pLayoutItem) ) == SfxItemState::SET )
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
                                        SotClipboardFormatId* pFormat,
                                        OUString* ,
                                        OUString* pFullTypeName,
                                        OUString* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        bool bTemplate /* = false */) const
{
    if (nFileFormat == SOFFICE_FILEFORMAT_60)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
            *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
            *pFormat = SotClipboardFormatId::STARDRAW_60;
            *pFullTypeName = OUString(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_60));
        }
        else
        {
            *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
            *pFormat = SotClipboardFormatId::STARIMPRESS_60;
            *pFullTypeName = OUString(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_60));
        }
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_8)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
            *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
            *pFormat = bTemplate ? SotClipboardFormatId::STARDRAW_8_TEMPLATE : SotClipboardFormatId::STARDRAW_8;
            *pFullTypeName = "Draw 8"; // HACK: method will be removed with new storage API
        }
        else
        {
            *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
            *pFormat = bTemplate ? SotClipboardFormatId::STARIMPRESS_8_TEMPLATE : SotClipboardFormatId::STARIMPRESS_8;
            *pFullTypeName = "Impress 8"; // HACK: method will be removed with new storage API
        }
    }

    *pShortTypeName = OUString(SdResId( (meDocType == DOCUMENT_TYPE_DRAW) ?
                                      STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT ));
}

OutputDevice* DrawDocShell::GetDocumentRefDev()
{
    OutputDevice* pReferenceDevice = SfxObjectShell::GetDocumentRefDev ();
    // Only when our parent does not have a reference device then we return
    // our own.
    if (pReferenceDevice == nullptr && mpDoc != nullptr)
        pReferenceDevice = mpDoc->GetRefDevice ();
    return pReferenceDevice;
}

/** executes the SID_OPENDOC slot to let the framework open a document
    with the given URL and this document as a referer */
void DrawDocShell::OpenBookmark( const OUString& rBookmarkURL )
{
    SfxStringItem   aStrItem( SID_FILE_NAME, rBookmarkURL );
    SfxStringItem   aReferer( SID_REFERER, GetMedium()->GetName() );
    const SfxPoolItem* ppArgs[] = { &aStrItem, &aReferer, nullptr };
    ( mpViewShell ? mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings().Execute( SID_OPENHYPERLINK, ppArgs );
}

VclPtr<SfxDocumentInfoDialog> DrawDocShell::CreateDocumentInfoDialog( vcl::Window *pParent, const SfxItemSet &rSet )
{
    VclPtr<SfxDocumentInfoDialog> pDlg   = VclPtr<SfxDocumentInfoDialog>::Create( pParent, rSet );
    DrawDocShell*          pDocSh = dynamic_cast< DrawDocShell *>( SfxObjectShell::Current() );

    if( pDocSh == this )
    {
        pDlg->AddFontTabPage();
    }
    return pDlg;
}

void DrawDocShell::setEditMode(DrawViewShell* pDrawViewShell, bool isMasterPage)
{
    // Set the edit mode to either the normal edit mode or the
    // master page mode.
    EditMode eNewEditMode = EM_PAGE;
    if (isMasterPage)
    {
        eNewEditMode = EM_MASTERPAGE;
    }

    if (eNewEditMode != pDrawViewShell->GetEditMode())
    {
        // Set EditMode
        pDrawViewShell->ChangeEditMode(eNewEditMode, false);
    }
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
