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

#include <sal/config.h>
#include <sal/log.hxx>

#include <memory>
#include <utility>

#include <DrawDocShell.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <editeng/outlobj.hxx>
#include <tools/urlobj.hxx>
#include <vcl/waitobj.hxx>
#include <svx/svxids.hrc>
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
#include <editeng/flstitem.hxx>
#include <svl/flagitem.hxx>
#include <sot/storage.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/printer.hxx>
#include <svtools/ctrltool.hxx>
#include <comphelper/classids.hxx>
#include <sot/formats.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>

#include <app.hrc>
#include <strings.hrc>
#include <strmname.h>
#include <FrameView.hxx>
#include <optsitem.hxx>
#include <Outliner.hxx>
#include <sdattr.hrc>
#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <sdmod.hxx>
#include <View.hxx>
#include <EffectMigration.hxx>
#include <CustomAnimationEffect.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>
#include <Window.hxx>
#include <OutlineView.hxx>
#include <OutlineViewShell.hxx>
#include <sdxmlwrp.hxx>
#include <sdpptwrp.hxx>
#include <sdcgmfilter.hxx>
#include <sdgrffilter.hxx>
#include <sdhtmlfilter.hxx>
#include <sdpdffilter.hxx>
#include <framework/FrameworkHelper.hxx>

#include <sfx2/zoomitem.hxx>

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
        auto pSet = std::make_unique<SfxItemSet>( GetPool(),
                            svl::Items<SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT>{} );
        // set PrintOptionsSet
        SdOptionsPrintItem aPrintItem( SD_MOD()->GetSdOptions(mpDoc->GetDocumentType()) );
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        SfxPrinterChangeFlags nFlags =
                (aPrintItem.GetOptionsPrint().IsWarningSize() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE) |
                (aPrintItem.GetOptionsPrint().IsWarningOrientation() ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE);
        aFlagItem.SetValue( static_cast<int>(nFlags) );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.GetOptionsPrint().IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        mpPrinter = VclPtr<SfxPrinter>::Create(std::move(pSet));
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
        aMM.SetMapUnit(MapUnit::Map100thMM);
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
    mpFontList.reset();
    OutputDevice* pRefDevice = nullptr;
    if ( mpDoc->GetPrinterIndependentLayout() == css::document::PrinterIndependentLayout::DISABLED )
        pRefDevice = GetPrinter(true);
    else
        pRefDevice = SD_MOD()->GetVirtualRefDevice();
    mpFontList.reset( new FontList(pRefDevice, nullptr) );
    SvxFontListItem aFontListItem( mpFontList.get(), SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );
}

Printer* DrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(false);
}

void DrawDocShell::OnDocumentPrinterChanged(Printer* pNewPrinter)
{
    // if we already have a printer, see if it's the same
    if( mpPrinter )
    {
        // easy case
        if( mpPrinter == pNewPrinter )
            return;

        // compare if it's the same printer with the same job setup
        if( (mpPrinter->GetName() == pNewPrinter->GetName()) &&
            (mpPrinter->GetJobSetup() == pNewPrinter->GetJobSetup()))
            return;
    }

    SfxPrinter* const pSfxPrinter = dynamic_cast<SfxPrinter*>(pNewPrinter);
    if (pSfxPrinter)
    {
        SetPrinter(pSfxPrinter);

        // container owns printer
        mbOwnPrinter = false;
    }
}

void DrawDocShell::UpdateRefDevice()
{
    if( !mpDoc )
        return;

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

    SdOutliner* pOutl = mpDoc->GetOutliner( false );

    if( pOutl )
        pOutl->SetRefDevice( pRefDevice );

    SdOutliner* pInternalOutl = mpDoc->GetInternalOutliner( false );

    if( pInternalOutl )
        pInternalOutl->SetRefDevice( pRefDevice );
}

/**
 * Creates new document, opens streams
 */
bool DrawDocShell::InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage )
{
    bool bRet = SfxObjectShell::InitNew( xStorage );

    ::tools::Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
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
    // If this is an ODF file being loaded, then by default, use legacy processing
    // for tdf#99729 (if required, it will be overridden in *::ReadUserDataSequence())
    if (IsOwnStorageFormat(rMedium))
    {
        mpDoc->SetAnchoredTextOverflowLegacy(true);
    }

    bool       bRet = false;
    bool       bStartPresentation = false;
    ErrCode nError = ERRCODE_NONE;

    SfxItemSet* pSet = rMedium.GetItemSet();

    if( pSet )
    {
        if( (  SfxItemState::SET == pSet->GetItemState(SID_PREVIEW ) ) && pSet->Get( SID_PREVIEW ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            pSet->Get( SID_DOC_STARTPRESENTATION ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    bRet = SfxObjectShell::Load( rMedium );
    if (bRet)
    {
        comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = getEmbeddedObjectContainer();
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(false);
        bRet = SdXMLFilter( rMedium, *this, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );
    }

    if( bRet )
    {
        // for legacy markup in OOoXML filter, convert the animations now
        EffectMigration::DocumentLoaded(*GetDoc());
        UpdateTablePointers();

        // If we're an embedded OLE object, use tight bounds
        // for our visArea. No point in showing the user lots of empty
        // space. Had to remove the check for empty VisArea below,
        // since XML load always sets a VisArea before.
        //TODO/LATER: looks a little bit strange!
        if( ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED ) && SfxObjectShell::GetVisArea( ASPECT_CONTENT ).IsEmpty() )
        {
            SdPage* pPage = mpDoc->GetSdPage( 0, PageKind::Standard );

            if( pPage )
                SetVisArea( pPage->GetAllObjBoundRect() );
        }

        FinishedLoading();

        const INetURLObject aUrl;
        SfxObjectShell::SetAutoLoad( aUrl, 0, false );
    }
    else
    {
        if( nError == ERRCODE_IO_BROKENPACKAGE )
            SetError(ERRCODE_IO_BROKENPACKAGE);

        // TODO/LATER: correct error handling?!
        //pStore->SetError(SVSTREAM_WRONGVERSION);
        else
            SetError(ERRCODE_ABORT);
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
    std::unique_ptr<WaitObject> pWait;
    if( mpViewShell )
        pWait.reset(new WaitObject( static_cast<vcl::Window*>(mpViewShell->GetActiveWindow()) ));

    mpDoc->NewOrLoadCompleted( NEW_DOC );
    mpDoc->CreateFirstPages();
    mpDoc->StopWorkStartupDelay();

    // TODO/LATER: nobody is interested in the error code?!
    ErrCode nError = ERRCODE_NONE;
    bool bRet = SdXMLFilter( rMedium, *this, SDXMLMODE_Organizer, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pSet = GetMedium()->GetItemSet();

        if( pSet )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }

    return bRet;
}

/**
 * load from 3rd party format
 */
bool DrawDocShell::ImportFrom(SfxMedium &rMedium,
        uno::Reference<text::XTextRange> const& xInsertPosition)
{
    const OUString aFilterName( rMedium.GetFilter()->GetFilterName() );
    if (aFilterName == "Impress MS PowerPoint 2007 XML" ||
        aFilterName == "Impress MS PowerPoint 2007 XML AutoPlay" ||
        aFilterName == "Impress MS PowerPoint 2007 XML VBA")
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
        const_cast<EditEngine&>(rOutl.GetEditEngine()).SetControlWord( nControlWord );

        mpDoc->SetSummationOfParagraphs();
    }

    const bool bRet = SfxObjectShell::ImportFrom(rMedium, xInsertPosition);

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            pSet->Get( SID_DOC_STARTPRESENTATION ).GetValue() )
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
    const OUString  aFilterName( rMedium.GetFilter()->GetFilterName() );
    bool            bRet = false;
    bool            bStartPresentation = false;

    SetWaitCursor( true );

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( (  SfxItemState::SET == pSet->GetItemState(SID_PREVIEW ) ) && pSet->Get( SID_PREVIEW ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SfxItemState::SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            pSet->Get( SID_DOC_STARTPRESENTATION ).GetValue() )
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
        bRet = SdPPTFilter( rMedium, *this ).Import();
    }
    else if (aFilterName.indexOf("impress8") >= 0 ||
             aFilterName.indexOf("draw8") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this ).Import( nError );

    }
    else if (aFilterName.indexOf("StarOffice XML (Draw)") >= 0 ||
             aFilterName.indexOf("StarOffice XML (Impress)") >= 0)
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 ).Import( nError );
    }
    else if (aFilterName == "CGM - Computer Graphics Metafile")
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdCGMFilter( rMedium, *this ).Import();
    }
    else if (aFilterName == "draw_pdf_import")
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdPdfFilter(rMedium, *this).Import();
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
        SfxObjectShell::SetVisArea( ::tools::Rectangle() );

    bool bRet = SfxObjectShell::Save();

    if( bRet )
        bRet = SdXMLFilter( *GetMedium(), *this, SDXMLMODE_Normal, SotStorage::GetVersion( GetMedium()->GetStorage() ) ).Export();

    return bRet;
}

/**
 * Writes pools and document to the provided storage
 */
bool DrawDocShell::SaveAs( SfxMedium& rMedium )
{
    mpDoc->setDocAccTitle(OUString());
    if (SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst(this))
    {
        if (vcl::Window* pSysWin = pFrame1->GetWindow().GetSystemWindow())
        {
            pSysWin->SetAccessibleName(OUString());
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
                std::unique_ptr<OutlinerParaObject> pNewText = pOutl->CreateParaObject( 0, pOutl->GetParagraphCount() );
                pObj->SetOutlinerParaObject( std::move(pNewText) );
                pOutl->ClearModifyFlag();
            }
        }
    }

    //TODO/LATER: why this?!
    if( GetCreateMode() == SfxObjectCreateMode::STANDARD )
        SfxObjectShell::SetVisArea( ::tools::Rectangle() );

    bool bRet = SfxObjectShell::SaveAs( rMedium );

    if( bRet )
        bRet = SdXMLFilter( rMedium, *this, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Export();

    if( GetError() == ERRCODE_NONE )
        SetError(ERRCODE_NONE);

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
        std::shared_ptr<const SfxFilter> pMediumFilter = rMedium.GetFilter();
        const OUString aTypeName( pMediumFilter->GetTypeName() );
        std::unique_ptr<SdFilter> xFilter;

        if( aTypeName.indexOf( "graphic_HTML" ) >= 0 )
        {
            xFilter = std::make_unique<SdHTMLFilter>(rMedium, *this);
        }
        else if( aTypeName.indexOf( "MS_PowerPoint_97" ) >= 0 )
        {
            xFilter = std::make_unique<SdPPTFilter>(rMedium, *this);
            static_cast<SdPPTFilter*>(xFilter.get())->PreSaveBasic();
        }
        else if ( aTypeName.indexOf( "CGM_Computer_Graphics_Metafile" ) >= 0 )
        {
            xFilter = std::make_unique<SdCGMFilter>(rMedium, *this);
        }
        else if( aTypeName.indexOf( "draw8" ) >= 0 ||
                 aTypeName.indexOf( "impress8" ) >= 0 )
        {
            xFilter = std::make_unique<SdXMLFilter>(rMedium, *this);
        }
        else if( aTypeName.indexOf( "StarOffice_XML_Impress" ) >= 0 ||
                 aTypeName.indexOf( "StarOffice_XML_Draw" ) >= 0 )
        {
            xFilter = std::make_unique<SdXMLFilter>(rMedium, *this, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60);
        }
        else
        {
            xFilter = std::make_unique<SdGRFFilter>(rMedium, *this);
        }

        if (xFilter)
        {
            if ( mpViewShell )
            {
                ::sd::View* pView = mpViewShell->GetView();
                if ( pView->IsTextEdit() )
                    pView->SdrEndTextEdit();
            }

            bRet = xFilter->Export();
        }
    }

    return bRet;
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

void DrawDocShell::GotoBookmark(const OUString& rBookmark)
{
    auto pDrawViewShell = dynamic_cast<DrawViewShell *>( mpViewShell );
    if (!pDrawViewShell)
        return;

    ViewShellBase& rBase (mpViewShell->GetViewShellBase());

    bool bIsMasterPage = false;
    sal_uInt16 nPageNumber = SDRPAGE_NOTFOUND;
    SdrObject* pObj = nullptr;

    const OUString sInteraction( "action?" );
    if ( rBookmark.match( sInteraction ) )
    {
        const OUString sJump( "jump=" );
        if ( rBookmark.match( sJump, sInteraction.getLength() ) )
        {
            OUString aDestination( rBookmark.copy( sInteraction.getLength() + sJump.getLength() ) );
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
        // Is the bookmark a page?
        nPageNumber = mpDoc->GetPageByName( rBookmark, bIsMasterPage );

        if (nPageNumber == SDRPAGE_NOTFOUND)
        {
            // Is the bookmark a object?
            pObj = mpDoc->GetObj(rBookmark);

            if (pObj)
            {
                nPageNumber = pObj->getSdrPageFromSdrObject()->GetPageNum();
            }
        }
    }
    if (nPageNumber != SDRPAGE_NOTFOUND)
    {
        // Jump to the bookmarked page.  This is done in three steps.

        SdPage* pPage;
        if (bIsMasterPage)
            pPage = static_cast<SdPage*>( mpDoc->GetMasterPage(nPageNumber) );
        else
            pPage = static_cast<SdPage*>( mpDoc->GetPage(nPageNumber) );

        // 1.) Change the view shell to the edit view, the notes view,
        // or the handout view.
        PageKind eNewPageKind = pPage->GetPageKind();

        if( (eNewPageKind != PageKind::Standard) && (mpDoc->GetDocumentType() == DocumentType::Draw) )
            return;

        if (eNewPageKind != pDrawViewShell->GetPageKind())
        {
            // change work area
            GetFrameView()->SetPageKind(eNewPageKind);
            OUString sViewURL;
            switch (eNewPageKind)
            {
                case PageKind::Standard:
                    sViewURL = FrameworkHelper::msImpressViewURL;
                    break;
                case PageKind::Notes:
                    sViewURL = FrameworkHelper::msNotesViewURL;
                    break;
                case PageKind::Handout:
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

            // show page
            SvxZoomItem aZoom;
            aZoom.SetType( SvxZoomType::WHOLEPAGE );
            pDrawViewShell->GetDispatcher()->ExecuteList(SID_ATTR_ZOOM, SfxCallMode::ASYNCHRON, { &aZoom });

            if (pObj != nullptr)
            {
                // select object
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

/**
 * If it should become a document template.
 */
bool DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{

    std::shared_ptr<const SfxFilter> pFilter = rMedium.GetFilter();

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

        if (aLayoutName.isEmpty())
        {
            sal_uInt32 nCount = mpDoc->GetMasterSdPageCount(PageKind::Standard);
            for (sal_uInt32 i = 0; i < nCount; ++i)
            {
                OUString aOldPageLayoutName = mpDoc->GetMasterSdPage(i, PageKind::Standard)->GetLayoutName();
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
        if ( meDocType == DocumentType::Draw )
        {
            *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
            *pFormat = SotClipboardFormatId::STARDRAW_60;
            *pFullTypeName = SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_60);
        }
        else
        {
            *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
            *pFormat = SotClipboardFormatId::STARIMPRESS_60;
            *pFullTypeName = SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_60);
        }
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_8)
    {
        if ( meDocType == DocumentType::Draw )
        {
            *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
            *pFormat = bTemplate ? SotClipboardFormatId::STARDRAW_8_TEMPLATE : SotClipboardFormatId::STARDRAW_8;
            *pFullTypeName = SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_80); // HACK: method will be removed with new storage API
        }
        else
        {
            *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
            *pFormat = bTemplate ? SotClipboardFormatId::STARIMPRESS_8_TEMPLATE : SotClipboardFormatId::STARIMPRESS_8;
            *pFullTypeName = SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_80); // HACK: method will be removed with new storage API
        }
    }

    *pShortTypeName = SdResId((meDocType == DocumentType::Draw) ?
                                     STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT);
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

VclPtr<SfxDocumentInfoDialog> DrawDocShell::CreateDocumentInfoDialog( const SfxItemSet &rSet )
{
    VclPtr<SfxDocumentInfoDialog> pDlg   = VclPtr<SfxDocumentInfoDialog>::Create( nullptr, rSet );
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
    EditMode eNewEditMode = EditMode::Page;
    if (isMasterPage)
    {
        eNewEditMode = EditMode::MasterPage;
    }

    if (eNewEditMode != pDrawViewShell->GetEditMode())
    {
        // Set EditMode
        pDrawViewShell->ChangeEditMode(eNewEditMode, false);
    }
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
