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

#include <string.h>

#include "impdialog.hxx"
#include "impdialog.hrc"
#include <officecfg/Office/Common.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include "sfx2/passwd.hxx"
#include "svtools/miscopt.hxx"

#include "comphelper/propertyvalue.hxx"
#include "comphelper/sequence.hxx"
#include "comphelper/storagehelper.hxx"

#include "com/sun/star/text/XTextRange.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"
#include "com/sun/star/security/DocumentDigitalSignatures.hpp"
#include "com/sun/star/security/XCertificate.hpp"

static ResMgr& getPDFFilterResMgr()
{
    static ResMgr *pRes = ResMgr::CreateResMgr( "pdffilter", Application::GetSettings().GetUILanguageTag());
    return *pRes;
}

PDFFilterResId::PDFFilterResId( sal_uInt32 nId ) : ResId( nId, getPDFFilterResMgr() )
{
}


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


/** Tabbed PDF dialog implementation
    Please note: the default used here are the same as per specification,
    They should be the same in  PDFFilter::implExport and  in PDFExport::PDFExport
 */
ImpPDFTabDialog::ImpPDFTabDialog(vcl::Window* pParent, Sequence< PropertyValue >& rFilterData,
    const Reference< XComponent >& rxDoc)
    : SfxTabDialog(pParent, "PdfOptionsDialog","filter/ui/pdfoptionsdialog.ui"),
    maConfigItem( "Office.Common/Filter/PDF/Export/", &rFilterData ),
    maConfigI18N( "Office.Common/I18N/CTL/" ),
    mnSigningPageId(0),
    mnSecurityPageId(0),
    mnLinksPage(0),
    mnInterfacePageId(0),
    mnViewPageId(0),
    mnGeneralPageId(0),
    mbIsPresentation( false ),
    mbIsSpreadsheet( false ),
    mbIsWriter( false ),

    mbSelectionPresent( false ),
    mbUseCTLFont( false ),
    mbUseLosslessCompression( true ),
    mnQuality( 90 ),
    mbReduceImageResolution( false ),
    mnMaxImageResolution( 300 ),
    mbUseTaggedPDF( false ),
    mbExportNotes( true ),
    mbViewPDF( false ),
    mbExportNotesPages( false ),
    mbExportOnlyNotesPages( false ),
    mbUseTransitionEffects( false ),
    mbIsSkipEmptyPages( true ),
    mbIsExportPlaceholders( false ),
    mbAddStream( false ),
    mnFormsType( 0 ),
    mbExportFormFields( true ),
    mbAllowDuplicateFieldNames( false ),
    mbExportBookmarks( true ),
    mbExportHiddenSlides ( false),
    mnOpenBookmarkLevels( -1 ),

    mbHideViewerToolbar( false ),
    mbHideViewerMenubar( false ),
    mbHideViewerWindowControls( false ),
    mbResizeWinToInit( false ),
    mbCenterWindow( false ),
    mbOpenInFullScreenMode( false ),
    mbDisplayPDFDocumentTitle( false ),
    mnMagnification( 0 ),
    mnInitialView( 0 ),
    mnZoom( 0 ),
    mnInitialPage( 1 ),
    mnPageLayout( 0 ),
    mbFirstPageLeft( false ),

    mbEncrypt( false ),
    mbRestrictPermissions( false ),
    mnPrint( 0 ),
    mnChangesAllowed( 0 ),
    mbCanCopyOrExtract( false ),
    mbCanExtractForAccessibility( true ),

    mbIsRangeChecked( false ),
    msPageRange( ' ' ),

    mbSelectionIsChecked( false ),
    mbExportRelativeFsysLinks( false ),
    mnViewPDFMode( 0 ),
    mbConvertOOoTargets( false ),
    mbExportBmkToPDFDestination( false ),

    mbSignPDF( false )
{
    // check for selection
    try
    {
        Reference< frame::XController > xController( Reference< frame::XModel >( rxDoc, UNO_QUERY )->getCurrentController() );
        if( xController.is() )
        {
            Reference< view::XSelectionSupplier > xView( xController, UNO_QUERY );
            if( xView.is() )
                maSelection = xView->getSelection();
        }
    }
    catch(const RuntimeException &)
    {
    }
    mbSelectionPresent = maSelection.hasValue();
    if ( mbSelectionPresent )
    {
        Reference< drawing::XShapes > xShapes;
        if ( !( maSelection >>= xShapes ) ) // XShapes is always a selection
        {
            // even if nothing is selected in writer the selection is not empty
            Reference< container::XIndexAccess > xIndexAccess;
            if ( maSelection >>= xIndexAccess )
            {
                sal_Int32 nLen = xIndexAccess->getCount();
                if ( !nLen )
                    mbSelectionPresent = false;
                else if ( nLen == 1 )
                {
                    Reference< text::XTextRange > xTextRange( xIndexAccess->getByIndex( 0 ), UNO_QUERY );
                    if ( xTextRange.is() && ( xTextRange->getString().isEmpty() ) )
                        mbSelectionPresent = false;
                }
            }
        }
    }

    // check if source document is a presentation or a spredsheet or a textdocument
    try
    {
        Reference< XServiceInfo > xInfo( rxDoc, UNO_QUERY );
        if ( xInfo.is() )
        {
            if ( xInfo->supportsService( "com.sun.star.presentation.PresentationDocument" ) )
                mbIsPresentation = true;
            if ( xInfo->supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
                mbIsSpreadsheet = true;
            if ( xInfo->supportsService( "com.sun.star.text.GenericTextDocument" ) )
                mbIsWriter = true;
        }
    }
    catch(const RuntimeException &)
    {
    }

    // get the CTL (Complex Text Layout) from general options, returns sal_True if we have a CTL font on our hands.
    mbUseCTLFont = maConfigI18N.ReadBool( "CTLFont", false );

    mbUseLosslessCompression = maConfigItem.ReadBool( "UseLosslessCompression", false );
    mnQuality = maConfigItem.ReadInt32( "Quality", 90 );
    mbReduceImageResolution = maConfigItem.ReadBool( "ReduceImageResolution", false );
    mnMaxImageResolution = maConfigItem.ReadInt32( "MaxImageResolution", 300 );

    mbUseTaggedPDF = maConfigItem.ReadBool( "UseTaggedPDF", false );
    mnPDFTypeSelection =  maConfigItem.ReadInt32( "SelectPdfVersion", 0 );
    if ( mbIsPresentation )
    {
        mbExportNotesPages = maConfigItem.ReadBool( "ExportNotesPages", false );
        mbExportOnlyNotesPages = maConfigItem.ReadBool( "ExportOnlyNotesPages", false );
    }
    mbExportNotes = maConfigItem.ReadBool( "ExportNotes", false );
    mbViewPDF = maConfigItem.ReadBool( "ViewPDFAfterExport", false );

    mbExportBookmarks = maConfigItem.ReadBool( "ExportBookmarks", true );
    if ( mbIsPresentation )
        mbExportHiddenSlides = maConfigItem.ReadBool( "ExportHiddenSlides", false );
    mnOpenBookmarkLevels = maConfigItem.ReadInt32( "OpenBookmarkLevels", -1 );
    mbUseTransitionEffects = maConfigItem.ReadBool( "UseTransitionEffects", true );
    mbIsSkipEmptyPages = maConfigItem.ReadBool( "IsSkipEmptyPages", false );
    mbIsExportPlaceholders = maConfigItem.ReadBool( "ExportPlaceholders", false );
    mbAddStream = maConfigItem.ReadBool( "IsAddStream", false );

    mnFormsType = maConfigItem.ReadInt32( "FormsType", 0 );
    mbExportFormFields = maConfigItem.ReadBool( "ExportFormFields", true );
    if ( ( mnFormsType < 0 ) || ( mnFormsType > 3 ) )
        mnFormsType = 0;
    mbAllowDuplicateFieldNames = maConfigItem.ReadBool( "AllowDuplicateFieldNames", false );

    // prepare values for the Viewer tab page
    mbHideViewerToolbar = maConfigItem.ReadBool( "HideViewerToolbar", false );
    mbHideViewerMenubar = maConfigItem.ReadBool( "HideViewerMenubar", false );
    mbHideViewerWindowControls = maConfigItem.ReadBool( "HideViewerWindowControls", false );
    mbResizeWinToInit = maConfigItem.ReadBool( "ResizeWindowToInitialPage", false );
    mbCenterWindow = maConfigItem.ReadBool( "CenterWindow", false );
    mbOpenInFullScreenMode = maConfigItem.ReadBool( "OpenInFullScreenMode", false );
    mbDisplayPDFDocumentTitle = maConfigItem.ReadBool( "DisplayPDFDocumentTitle", true );

    mnInitialView = maConfigItem.ReadInt32( "InitialView", 0 );
    mnMagnification = maConfigItem.ReadInt32( "Magnification", 0 );
    mnZoom = maConfigItem.ReadInt32( "Zoom", 100 );
    mnPageLayout = maConfigItem.ReadInt32( "PageLayout", 0 );
    mbFirstPageLeft = maConfigItem.ReadBool( "FirstPageOnLeft", false );
    mnInitialPage = maConfigItem.ReadInt32( "InitialPage", 1 );
    if( mnInitialPage < 1 )
        mnInitialPage = 1;

    // prepare values for the security tab page
    mnPrint = maConfigItem.ReadInt32( "Printing", 2 );
    mnChangesAllowed = maConfigItem.ReadInt32( "Changes", 4 );
    mbCanCopyOrExtract = maConfigItem.ReadBool( "EnableCopyingOfContent", true );
    mbCanExtractForAccessibility = maConfigItem.ReadBool( "EnableTextAccessForAccessibilityTools", true );

    // prepare values for relative links
    mbExportRelativeFsysLinks = maConfigItem.ReadBool( "ExportLinksRelativeFsys", false );

    mnViewPDFMode = maConfigItem.ReadInt32( "PDFViewSelection", 0 );

    mbConvertOOoTargets = maConfigItem.ReadBool( "ConvertOOoTargetToPDFTarget", false );
    mbExportBmkToPDFDestination = maConfigItem.ReadBool( "ExportBookmarksToPDFDestination", false );

    // prepare values for digital signatures
    mbSignPDF = maConfigItem.ReadBool( "SignPDF", false );

    // queue the tab pages for later creation (created when first shown)
    mnSigningPageId = AddTabPage("digitalsignatures", ImpPDFTabSigningPage::Create, nullptr);
    mnSecurityPageId = AddTabPage("security", ImpPDFTabSecurityPage::Create, nullptr);
    mnLinksPage = AddTabPage("links", ImpPDFTabLinksPage::Create, nullptr);
    mnInterfacePageId = AddTabPage("userinterface", ImpPDFTabViewerPage::Create, nullptr);
    mnViewPageId = AddTabPage("initialview", ImpPDFTabOpnFtrPage::Create, nullptr);

    // last queued is the first to be displayed (or so it seems..)
    mnGeneralPageId = AddTabPage("general", ImpPDFTabGeneralPage::Create, nullptr );

    // get the string property value (from sfx2/source/dialog/mailmodel.cxx) to overwrite the text for the Ok button
    OUString sOkButtonText = maConfigItem.ReadString( "_OkButtonString", OUString() );

    // change text on the Ok button: get the relevant string from resources, update it on the button
    // according to the exported pdf file destination: send as e-mail or write to file?
    if (!sOkButtonText.isEmpty())
    {
        GetOKButton().SetText(sOkButtonText);
    }

    GetCancelButton().SetClickHdl(LINK(this, ImpPDFTabDialog, CancelHdl));

    // remove the reset button, not needed in this tabbed dialog
    RemoveResetButton();

}


ImpPDFTabSecurityPage* ImpPDFTabDialog::getSecurityPage() const
{
    SfxTabPage* pSecurityPage = GetTabPage(mnSecurityPageId);
    if (pSecurityPage)
    {
        return static_cast<ImpPDFTabSecurityPage*>(pSecurityPage);
    }
    return nullptr;
}


ImpPDFTabLinksPage* ImpPDFTabDialog::getLinksPage() const
{
    SfxTabPage* pLinksPage = GetTabPage(mnLinksPage);
    if (pLinksPage)
    {
        return static_cast<ImpPDFTabLinksPage*>(pLinksPage);
    }
    return nullptr;
}


ImpPDFTabGeneralPage* ImpPDFTabDialog::getGeneralPage() const
{
    SfxTabPage* pGeneralPage = GetTabPage(mnGeneralPageId);
    if (pGeneralPage)
    {
        return static_cast<ImpPDFTabGeneralPage*>(pGeneralPage);
    }
    return nullptr;
}


IMPL_LINK_NOARG(ImpPDFTabDialog, CancelHdl, Button*, void)
{
    EndDialog();
}


ImpPDFTabDialog::~ImpPDFTabDialog()
{
    disposeOnce();
}


void ImpPDFTabDialog::dispose()
{
    // delete the pages, needed because otherwise the child tab pages
    // don't get destroyed
    RemoveTabPage(mnGeneralPageId);
    RemoveTabPage(mnInterfacePageId);
    RemoveTabPage(mnViewPageId);
    RemoveTabPage(mnLinksPage);
    RemoveTabPage(mnSecurityPageId);
    RemoveTabPage(mnSigningPageId);
    maConfigItem.WriteModifiedConfig();
    maConfigI18N.WriteModifiedConfig();
    SfxTabDialog::dispose();
}


void ImpPDFTabDialog::PageCreated( sal_uInt16 _nId,
                                   SfxTabPage& _rPage )
{
    if (_nId == mnGeneralPageId)
    {
        static_cast<ImpPDFTabGeneralPage*>( &_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnInterfacePageId)
    {
        static_cast<ImpPDFTabViewerPage*>( &_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnViewPageId)
    {
        static_cast<ImpPDFTabOpnFtrPage*>( &_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnLinksPage)
    {
        static_cast<ImpPDFTabLinksPage*>( &_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnSecurityPageId)
    {
        static_cast<ImpPDFTabSecurityPage*>( &_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnSigningPageId)
    {
        static_cast<ImpPDFTabSigningPage*>( &_rPage )->SetFilterConfigItem( this );
    }
}


short ImpPDFTabDialog::Ok( )
{
    // here the whole mechanism of the base class is not used
    // when Ok is hit, the user means 'convert to PDF', so simply close with ok
    return RET_OK;
}


Sequence< PropertyValue > ImpPDFTabDialog::GetFilterData()
{
    // updating the FilterData sequence and storing FilterData to configuration
    if( GetTabPage(mnGeneralPageId) )
        static_cast<ImpPDFTabGeneralPage*>( GetTabPage(mnGeneralPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnInterfacePageId) )
        static_cast<ImpPDFTabViewerPage*>( GetTabPage(mnInterfacePageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnViewPageId) )
        static_cast<ImpPDFTabOpnFtrPage*>( GetTabPage(mnViewPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnLinksPage) )
        static_cast<ImpPDFTabLinksPage*>( GetTabPage(mnLinksPage) )->GetFilterConfigItem( this );
    if( GetTabPage(mnSecurityPageId) )
        static_cast<ImpPDFTabSecurityPage*>( GetTabPage(mnSecurityPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnSigningPageId) )
        static_cast<ImpPDFTabSigningPage*>( GetTabPage(mnSigningPageId) )->GetFilterConfigItem( this );

    // prepare the items to be returned
    maConfigItem.WriteBool( "UseLosslessCompression", mbUseLosslessCompression );
    maConfigItem.WriteInt32("Quality", mnQuality );
    maConfigItem.WriteBool( "ReduceImageResolution", mbReduceImageResolution );
    maConfigItem.WriteInt32("MaxImageResolution", mnMaxImageResolution );

    maConfigItem.WriteBool( "UseTaggedPDF", mbUseTaggedPDF );
    maConfigItem.WriteInt32("SelectPdfVersion", mnPDFTypeSelection );

    if ( mbIsPresentation )
    {
        maConfigItem.WriteBool( "ExportNotesPages", mbExportNotesPages );
        maConfigItem.WriteBool( "ExportOnlyNotesPages", mbExportOnlyNotesPages );
    }
    maConfigItem.WriteBool( "ExportNotes", mbExportNotes );
    maConfigItem.WriteBool( "ViewPDFAfterExport", mbViewPDF );

    maConfigItem.WriteBool( "ExportBookmarks", mbExportBookmarks );
    if ( mbIsPresentation )
        maConfigItem.WriteBool( "ExportHiddenSlides", mbExportHiddenSlides );
    maConfigItem.WriteBool( "UseTransitionEffects", mbUseTransitionEffects );
    maConfigItem.WriteBool( "IsSkipEmptyPages", mbIsSkipEmptyPages );
    maConfigItem.WriteBool( "ExportPlaceholders", mbIsExportPlaceholders );
    maConfigItem.WriteBool( "IsAddStream", mbAddStream );

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    maConfigItem.WriteInt32( "FormsType", mnFormsType );
    maConfigItem.WriteBool( "ExportFormFields", mbExportFormFields );
    maConfigItem.WriteBool( "AllowDuplicateFieldNames", mbAllowDuplicateFieldNames );

    maConfigItem.WriteBool( "HideViewerToolbar", mbHideViewerToolbar );
    maConfigItem.WriteBool( "HideViewerMenubar", mbHideViewerMenubar );
    maConfigItem.WriteBool( "HideViewerWindowControls", mbHideViewerWindowControls );
    maConfigItem.WriteBool( "ResizeWindowToInitialPage", mbResizeWinToInit );
    maConfigItem.WriteBool( "CenterWindow", mbCenterWindow );
    maConfigItem.WriteBool( "OpenInFullScreenMode", mbOpenInFullScreenMode );
    maConfigItem.WriteBool( "DisplayPDFDocumentTitle", mbDisplayPDFDocumentTitle );
    maConfigItem.WriteInt32( "InitialView", mnInitialView );
    maConfigItem.WriteInt32( "Magnification", mnMagnification);
    maConfigItem.WriteInt32( "Zoom", mnZoom );
    maConfigItem.WriteInt32( "InitialPage", mnInitialPage );
    maConfigItem.WriteInt32( "PageLayout", mnPageLayout );
    maConfigItem.WriteBool( "FirstPageOnLeft", mbFirstPageLeft );
    maConfigItem.WriteInt32( "OpenBookmarkLevels", mnOpenBookmarkLevels );

    maConfigItem.WriteBool( "ExportLinksRelativeFsys", mbExportRelativeFsysLinks );
    maConfigItem.WriteInt32("PDFViewSelection", mnViewPDFMode );
    maConfigItem.WriteBool( "ConvertOOoTargetToPDFTarget", mbConvertOOoTargets );
    maConfigItem.WriteBool( "ExportBookmarksToPDFDestination", mbExportBmkToPDFDestination );

    maConfigItem.WriteBool( "SignPDF", mbSignPDF );

    maConfigItem.WriteInt32( "Printing", mnPrint );
    maConfigItem.WriteInt32( "Changes", mnChangesAllowed );
    maConfigItem.WriteBool( "EnableCopyingOfContent", mbCanCopyOrExtract );
    maConfigItem.WriteBool( "EnableTextAccessForAccessibilityTools", mbCanExtractForAccessibility );

    std::vector<beans::PropertyValue> aRet;

    aRet.push_back(comphelper::makePropertyValue("Watermark", maWatermarkText));
    aRet.push_back(comphelper::makePropertyValue("EncryptFile", mbEncrypt));
    aRet.push_back(comphelper::makePropertyValue("PreparedPasswords", mxPreparedPasswords));
    aRet.push_back(comphelper::makePropertyValue("RestrictPermissions", mbRestrictPermissions));
    aRet.push_back(comphelper::makePropertyValue("PreparedPermissionPassword", maPreparedOwnerPassword));
    if( mbIsRangeChecked )
        aRet.push_back(comphelper::makePropertyValue("PageRange", msPageRange));
    else if( mbSelectionIsChecked )
        aRet.push_back(comphelper::makePropertyValue("Selection", maSelection));

    aRet.push_back(comphelper::makePropertyValue("SignatureLocation", msSignLocation));
    aRet.push_back(comphelper::makePropertyValue("SignatureReason", msSignReason));
    aRet.push_back(comphelper::makePropertyValue("SignatureContactInfo", msSignContact));
    aRet.push_back(comphelper::makePropertyValue("SignaturePassword", msSignPassword));
    aRet.push_back(comphelper::makePropertyValue("SignatureCertificate", maSignCertificate));
    aRet.push_back(comphelper::makePropertyValue("SignatureTSA", msSignTSA));

    return comphelper::concatSequences(maConfigItem.GetFilterData(), comphelper::containerToSequence(aRet));
}


ImpPDFTabGeneralPage::ImpPDFTabGeneralPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfGeneralPage","filter/ui/pdfgeneralpage.ui", &rCoreSet)
    , mbTaggedPDFUserSelection(false)
    , mbExportFormFieldsUserSelection(false)
    , mbIsPresentation(false)
    , mbIsSpreadsheet(false)
    , mbIsWriter(false)
    , mpaParent(nullptr)
{
    get(mpRbAll, "all");
    get(mpRbRange, "range");
    get(mpRbSelection, "selection");
    get(mpSelectedSheets, "selectedsheets");
    get(mpEdPages, "pages");

    get(mpRbLosslessCompression, "losslesscompress");
    get(mpRbJPEGCompression, "jpegcompress");
    get(mpQualityFrame, "qualityframe");
    get(mpNfQuality, "quality");
    get(mpCbReduceImageResolution, "reduceresolution");
    get(mpCoReduceImageResolution, "resolution");

    get(mpCbAddStream, "embed");
    get(mpCbPDFA1b, "pdfa");
    get(mpCbTaggedPDF, "tagged");
    get(mpCbExportFormFields, "forms");

    get(mpFormsFrame, "formsframe");
    get(mpLbFormsFormat, "format");
    get(mpCbAllowDuplicateFieldNames, "allowdups");

    get(mpCbExportBookmarks, "bookmarks");
    get(mpCbExportHiddenSlides, "hiddenpages");
    get(mpCbExportNotes, "comments");
    get(mpCbExportNotesPages, "notes");
    get(mpCbExportOnlyNotesPages, "onlynotes");
    get(mpCbExportEmptyPages, "emptypages");
    get(mpCbExportPlaceholders, "exportplaceholders" );
    get(mpCbViewPDF, "viewpdf");

    get(mpCbWatermark, "watermark");
    get(mpFtWatermark, "watermarklabel");
    get(mpEdWatermark, "watermarkentry");
}


ImpPDFTabGeneralPage::~ImpPDFTabGeneralPage()
{
    disposeOnce();
}


void ImpPDFTabGeneralPage::dispose()
{
    mpRbAll.clear();
    mpRbRange.clear();
    mpRbSelection.clear();
    mpSelectedSheets.clear();
    mpEdPages.clear();
    mpRbLosslessCompression.clear();
    mpRbJPEGCompression.clear();
    mpQualityFrame.clear();
    mpNfQuality.clear();
    mpCbReduceImageResolution.clear();
    mpCoReduceImageResolution.clear();
    mpCbPDFA1b.clear();
    mpCbTaggedPDF.clear();
    mpCbExportFormFields.clear();
    mpFormsFrame.clear();
    mpLbFormsFormat.clear();
    mpCbAllowDuplicateFieldNames.clear();
    mpCbExportBookmarks.clear();
    mpCbExportHiddenSlides.clear();
    mpCbExportNotes.clear();
    mpCbViewPDF.clear();
    mpCbExportNotesPages.clear();
    mpCbExportOnlyNotesPages.clear();
    mpCbExportEmptyPages.clear();
    mpCbExportPlaceholders.clear();
    mpCbAddStream.clear();
    mpCbWatermark.clear();
    mpFtWatermark.clear();
    mpEdWatermark.clear();
    mpaParent.clear();
    SfxTabPage::dispose();
}


void ImpPDFTabGeneralPage::SetFilterConfigItem( ImpPDFTabDialog* paParent )
{
    mpaParent = paParent;

    // init this class data
    mpRbRange->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, TogglePagesHdl ) );

    mpRbAll->Check();
    mpRbAll->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleAllHdl ) );
    TogglePagesHdl();

    mpRbSelection->Enable( paParent->mbSelectionPresent );
    if ( paParent->mbSelectionPresent )
        mpRbSelection->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleSelectionHdl ) );
    mbIsPresentation = paParent->mbIsPresentation;
    mbIsWriter = paParent->mbIsWriter;

    mpCbExportEmptyPages->Enable( mbIsWriter );
    mpCbExportPlaceholders->Enable( mbIsWriter );

    mpRbLosslessCompression->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const bool bUseLosslessCompression = paParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        mpRbLosslessCompression->Check();
    else
        mpRbJPEGCompression->Check();

    mpNfQuality->SetValue( paParent->mnQuality, FUNIT_PERCENT );
    mpQualityFrame->Enable(!bUseLosslessCompression);

    mpCbReduceImageResolution->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl ) );
    const bool  bReduceImageResolution = paParent->mbReduceImageResolution;
    mpCbReduceImageResolution->Check( bReduceImageResolution );
    OUString aStrRes = OUString::number( paParent->mnMaxImageResolution ) + " DPI";
    mpCoReduceImageResolution->SetText( aStrRes );
    mpCoReduceImageResolution->Enable( bReduceImageResolution );
    mpCbWatermark->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleWatermarkHdl ) );
    mpFtWatermark->Enable(false );
    mpEdWatermark->Enable( false );
    mpCbPDFA1b->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleExportPDFAHdl) );
    switch( paParent->mnPDFTypeSelection )
    {
    default:
    case 0: mpCbPDFA1b->Check( false ); // PDF 1.4
        break;
    case 1: mpCbPDFA1b->Check(); // PDF/A-1a
        break;
    }
    ToggleExportPDFAHdl( *mpCbPDFA1b );

    mpCbExportFormFields->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl ) );

    // get the form values, for use with PDF/A-1 selection interface
    mbTaggedPDFUserSelection = paParent->mbUseTaggedPDF;
    mbExportFormFieldsUserSelection = paParent->mbExportFormFields;

    if( !mpCbPDFA1b->IsChecked() )
    {
        // the value for PDF/A set by the ToggleExportPDFAHdl method called before
        mpCbTaggedPDF->Check( mbTaggedPDFUserSelection  );
        mpCbExportFormFields->Check( mbExportFormFieldsUserSelection );
    }

    mpLbFormsFormat->SelectEntryPos( (sal_uInt16)paParent->mnFormsType );
    mpCbAllowDuplicateFieldNames->Check( paParent->mbAllowDuplicateFieldNames );
    mpFormsFrame->Enable( paParent->mbExportFormFields );

    mpCbExportBookmarks->Check( paParent->mbExportBookmarks );

    mpCbExportNotes->Check( paParent->mbExportNotes );
    mpCbViewPDF->Check( paParent->mbViewPDF);

    if ( mbIsPresentation )
    {
        mpRbRange->SetText(get<FixedText>("slides")->GetText());
        mpCbExportNotesPages->Show();
        mpCbExportNotesPages->Check(paParent->mbExportNotesPages);
        mpCbExportNotesPages->SetToggleHdl( LINK(this, ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl ) );
        mpCbExportOnlyNotesPages->Show();
        mpCbExportOnlyNotesPages->Check(paParent->mbExportOnlyNotesPages);
        mpCbExportHiddenSlides->Show();
        mpCbExportHiddenSlides->Check(paParent->mbExportHiddenSlides);
    }
    else
    {
        mpCbExportNotesPages->Show(false);
        mpCbExportNotesPages->Check(false);
        mpCbExportOnlyNotesPages->Show(false);
        mpCbExportOnlyNotesPages->Check(false);
        mpCbExportHiddenSlides->Show(false);
        mpCbExportHiddenSlides->Check(false);
    }
    if( mbIsSpreadsheet )
    {
        mpRbSelection->SetText(get<FixedText>("selectedsheets")->GetText());
    }
    mpCbExportPlaceholders->Show(mbIsWriter);
    if( !mbIsWriter )
    {
        mpCbExportPlaceholders->Check(false);
    }
    mpCbExportEmptyPages->Check(!paParent->mbIsSkipEmptyPages);
    mpCbExportPlaceholders->Check(paParent->mbIsExportPlaceholders);

    mpCbAddStream->Show();
    mpCbAddStream->Check(paParent->mbAddStream);

    mpCbAddStream->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleAddStreamHdl ) );
    ToggleAddStreamHdl(*mpCbAddStream); // init addstream dependencies
}


void ImpPDFTabGeneralPage::GetFilterConfigItem( ImpPDFTabDialog* paParent )
{
    // updating the FilterData sequence and storing FilterData to configuration
    paParent->mbUseLosslessCompression = mpRbLosslessCompression->IsChecked();
    paParent->mnQuality = static_cast<sal_Int32>(mpNfQuality->GetValue());
    paParent->mbReduceImageResolution = mpCbReduceImageResolution->IsChecked();
    paParent->mnMaxImageResolution = mpCoReduceImageResolution->GetText().toInt32();
    paParent->mbExportNotes = mpCbExportNotes->IsChecked();
    paParent->mbViewPDF = mpCbViewPDF->IsChecked();
    if ( mbIsPresentation )
    {
        paParent->mbExportNotesPages = mpCbExportNotesPages->IsChecked();
        paParent->mbExportOnlyNotesPages = mpCbExportOnlyNotesPages->IsChecked();
    }
    paParent->mbExportBookmarks = mpCbExportBookmarks->IsChecked();
    if ( mbIsPresentation )
        paParent->mbExportHiddenSlides = mpCbExportHiddenSlides->IsChecked();

    paParent->mbIsSkipEmptyPages = !mpCbExportEmptyPages->IsChecked();
    paParent->mbIsExportPlaceholders = mpCbExportPlaceholders->IsChecked();
    paParent->mbAddStream = mpCbAddStream->IsVisible() && mpCbAddStream->IsChecked();

    paParent->mbIsRangeChecked = false;
    if( mpRbRange->IsChecked() )
    {
        paParent->mbIsRangeChecked = true;
        paParent->msPageRange = mpEdPages->GetText(); //FIXME all right on other languages ?
    }
    else if( mpRbSelection->IsChecked() )
    {
        paParent->mbSelectionIsChecked = mpRbSelection->IsChecked();
    }

    paParent->mnPDFTypeSelection = 0;
    if( mpCbPDFA1b->IsChecked() )
    {
        paParent->mnPDFTypeSelection = 1;
        paParent->mbUseTaggedPDF =  mbTaggedPDFUserSelection;
        paParent->mbExportFormFields = mbExportFormFieldsUserSelection;
    }
    else
    {
        paParent->mbUseTaggedPDF =  mpCbTaggedPDF->IsChecked();
        paParent->mbExportFormFields = mpCbExportFormFields->IsChecked();
    }

    paParent->maWatermarkText = mpEdWatermark->GetText();

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    paParent->mnFormsType = (sal_Int32) mpLbFormsFormat->GetSelectEntryPos();
    paParent->mbAllowDuplicateFieldNames = mpCbAllowDuplicateFieldNames->IsChecked();
}


VclPtr<SfxTabPage> ImpPDFTabGeneralPage::Create( vcl::Window* pParent,
                                                 const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabGeneralPage>::Create( pParent, *rAttrSet );
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAllHdl, RadioButton&, void)
{
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, TogglePagesHdl, RadioButton&, void)
{
    TogglePagesHdl();
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleSelectionHdl, RadioButton&, void)
{
    EnableExportNotesPages();
}

void ImpPDFTabGeneralPage::TogglePagesHdl()
{
    mpEdPages->Enable( mpRbRange->IsChecked() );
    if ( mpRbRange->IsChecked() )
        mpEdPages->GrabFocus();
}

void ImpPDFTabGeneralPage::EnableExportNotesPages()
{
    if ( mbIsPresentation )
    {
        mpCbExportNotesPages->Enable( !mpRbSelection->IsChecked() );
        mpCbExportOnlyNotesPages->Enable( !mpRbSelection->IsChecked() && mpCbExportNotesPages->IsChecked() );
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl, CheckBox&, void)
{
    mpFormsFrame->Enable(mpCbExportFormFields->IsChecked());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl, CheckBox&, void)
{
    mpCbExportOnlyNotesPages->Enable(mpCbExportNotesPages->IsChecked());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleCompressionHdl, RadioButton&, void)
{
    mpQualityFrame->Enable(mpRbJPEGCompression->IsChecked());
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl, CheckBox&, void)
{
    mpCoReduceImageResolution->Enable( mpCbReduceImageResolution->IsChecked() );
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleWatermarkHdl, CheckBox&, void)
{
    mpEdWatermark->Enable( mpCbWatermark->IsChecked() );
    mpFtWatermark->Enable (mpCbWatermark->IsChecked() );
    if ( mpCbWatermark->IsChecked() )
        mpEdWatermark->GrabFocus();
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAddStreamHdl, CheckBox&, void)
{
    if( mpCbAddStream->IsVisible() )
    {
        if( mpCbAddStream->IsChecked() )
        {
            mpRbAll->Check();
            mpRbRange->Enable( false );
            mpRbSelection->Enable( false );
            mpEdPages->Enable( false );
            mpRbAll->Enable( false );
        }
        else
        {
            mpRbAll->Enable();
            mpRbRange->Enable();
            mpRbSelection->Enable();
        }
    }
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportPDFAHdl, CheckBox&, void)
{
    // set the security page status (and its controls as well)
    ImpPDFTabSecurityPage* pSecPage = mpaParent ? mpaParent->getSecurityPage() : nullptr;
    if (pSecPage)
    {
        pSecPage->ImplPDFASecurityControl(!mpCbPDFA1b->IsChecked());
    }

    // PDF/A-1 needs tagged PDF, so force disable the control, will be forced in pdfexport.
    bool bPDFA1Sel = mpCbPDFA1b->IsChecked();
    mpFormsFrame->Enable(bPDFA1Sel);
    if(bPDFA1Sel)
    {
        // store the values of subordinate controls
        mbTaggedPDFUserSelection = mpCbTaggedPDF->IsChecked();
        mpCbTaggedPDF->Check();
        mpCbTaggedPDF->Enable(false);
        mbExportFormFieldsUserSelection = mpCbExportFormFields->IsChecked();
        mpCbExportFormFields->Check(false);
        mpCbExportFormFields->Enable(false);
    }
    else
    {
        // retrieve the values of subordinate controls
        mpCbTaggedPDF->Enable();
        mpCbTaggedPDF->Check( mbTaggedPDFUserSelection );
        mpCbExportFormFields->Check( mbExportFormFieldsUserSelection );
        mpCbExportFormFields->Enable();
    }

    // PDF/A-1 doesn't allow launch action, so enable/disable the selection on
    // Link page
    ImpPDFTabLinksPage* pLinksPage = mpaParent ? mpaParent->getLinksPage() : nullptr;
    if (pLinksPage)
        pLinksPage->ImplPDFALinkControl(!mpCbPDFA1b->IsChecked());

    // if a password was set, inform the user that this will not be used in PDF/A case
    if( mpCbPDFA1b->IsChecked() && pSecPage && pSecPage->hasPassword() )
    {
        ScopedVclPtrInstance< MessageDialog > aBox(this, PDFFilterResId(STR_WARN_PASSWORD_PDFA), VclMessageType::Warning);
        aBox->Execute();
    }
}


/// The option features tab page
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfViewPage","filter/ui/pdfviewpage.ui", &rCoreSet)
    , mbUseCTLFont(false)
{
    get(mpRbOpnPageOnly, "pageonly");
    get(mpRbOpnOutline, "outline");
    get(mpRbOpnThumbs, "thumbs");
    get(mpNumInitialPage, "page");
    get(mpRbMagnDefault, "fitdefault");
    get(mpRbMagnFitWin, "fitwin");
    get(mpRbMagnFitWidth, "fitwidth");
    get(mpRbMagnFitVisible, "fitvis");
    get(mpRbMagnZoom, "fitzoom");
    get(mpNumZoom, "zoom");
    get(mpRbPgLyDefault, "defaultlayout");
    get(mpRbPgLySinglePage, "singlelayout");
    get(mpRbPgLyContinue, "contlayout");
    get(mpRbPgLyContinueFacing, "contfacinglayout");
    get(mpCbPgLyFirstOnLeft, "firstonleft");

    mpRbMagnDefault->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mpRbMagnFitWin->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mpRbMagnFitWidth->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mpRbMagnFitVisible->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mpRbMagnZoom->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
}


ImpPDFTabOpnFtrPage::~ImpPDFTabOpnFtrPage()
{
    disposeOnce();
}


void ImpPDFTabOpnFtrPage::dispose()
{
    mpRbOpnPageOnly.clear();
    mpRbOpnOutline.clear();
    mpRbOpnThumbs.clear();
    mpNumInitialPage.clear();
    mpRbMagnDefault.clear();
    mpRbMagnFitWin.clear();
    mpRbMagnFitWidth.clear();
    mpRbMagnFitVisible.clear();
    mpRbMagnZoom.clear();
    mpNumZoom.clear();
    mpRbPgLyDefault.clear();
    mpRbPgLySinglePage.clear();
    mpRbPgLyContinue.clear();
    mpRbPgLyContinueFacing.clear();
    mpCbPgLyFirstOnLeft.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> ImpPDFTabOpnFtrPage::Create( vcl::Window* pParent,
                                                const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabOpnFtrPage>::Create( pParent, *rAttrSet );
}


void ImpPDFTabOpnFtrPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mnInitialView = 0;
    if( mpRbOpnOutline->IsChecked() )
        paParent->mnInitialView = 1;
    else if( mpRbOpnThumbs->IsChecked() )
        paParent->mnInitialView = 2;

    paParent->mnMagnification = 0;
    if( mpRbMagnFitWin->IsChecked() )
        paParent->mnMagnification = 1;
    else if( mpRbMagnFitWidth->IsChecked() )
        paParent->mnMagnification = 2;
    else if( mpRbMagnFitVisible->IsChecked() )
        paParent->mnMagnification = 3;
    else if( mpRbMagnZoom->IsChecked() )
    {
        paParent->mnMagnification = 4;
        paParent->mnZoom = static_cast<sal_Int32>(mpNumZoom->GetValue());
    }

    paParent->mnInitialPage = static_cast<sal_Int32>(mpNumInitialPage->GetValue());

    paParent->mnPageLayout = 0;
    if( mpRbPgLySinglePage->IsChecked() )
        paParent->mnPageLayout = 1;
    else if( mpRbPgLyContinue->IsChecked() )
        paParent->mnPageLayout = 2;
    else if( mpRbPgLyContinueFacing->IsChecked() )
        paParent->mnPageLayout = 3;

    paParent->mbFirstPageLeft = mbUseCTLFont && mpCbPgLyFirstOnLeft->IsChecked();
}


void ImpPDFTabOpnFtrPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    mbUseCTLFont = paParent->mbUseCTLFont;
    switch( paParent->mnPageLayout )
    {
    default:
    case 0:
        mpRbPgLyDefault->Check();
        break;
    case 1:
        mpRbPgLySinglePage->Check();
        break;
    case 2:
        mpRbPgLyContinue->Check();
        break;
    case 3:
        mpRbPgLyContinueFacing->Check();
        break;
    }

    switch( paParent->mnInitialView )
    {
    default:
    case 0:
        mpRbOpnPageOnly->Check();
        break;
    case 1:
        mpRbOpnOutline->Check();
        break;
    case 2:
        mpRbOpnThumbs->Check();
        break;
    }

    switch( paParent->mnMagnification )
    {
    default:
    case 0:
        mpRbMagnDefault->Check();
        mpNumZoom->Enable( false );
        break;
    case 1:
        mpRbMagnFitWin->Check();
        mpNumZoom->Enable( false );
        break;
    case 2:
        mpRbMagnFitWidth->Check();
        mpNumZoom->Enable( false );
        break;
    case 3:
        mpRbMagnFitVisible->Check();
        mpNumZoom->Enable( false );
        break;
    case 4:
        mpRbMagnZoom->Check();
        mpNumZoom->Enable();
        break;
    }

    mpNumZoom->SetValue( paParent->mnZoom );
    mpNumInitialPage->SetValue( paParent->mnInitialPage );

    if( !mbUseCTLFont )
        mpCbPgLyFirstOnLeft->Hide( );
    else
    {
        mpRbPgLyContinueFacing->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl ) );
        mpCbPgLyFirstOnLeft->Check( paParent->mbFirstPageLeft );
        ToggleRbPgLyContinueFacingHdl();
    }
}


IMPL_LINK_NOARG(ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl, RadioButton&, void)
{
    ToggleRbPgLyContinueFacingHdl();
}


void ImpPDFTabOpnFtrPage::ToggleRbPgLyContinueFacingHdl()
{
    mpCbPgLyFirstOnLeft->Enable( mpRbPgLyContinueFacing->IsChecked() );
}


IMPL_LINK_NOARG( ImpPDFTabOpnFtrPage, ToggleRbMagnHdl, RadioButton&, void )
{
    mpNumZoom->Enable( mpRbMagnZoom->IsChecked() );
}


/// The Viewer preferences tab page
ImpPDFTabViewerPage::ImpPDFTabViewerPage( vcl::Window* pParent,
                                          const SfxItemSet& rCoreSet )
: SfxTabPage(pParent, "PdfUserInterfacePage","filter/ui/pdfuserinterfacepage.ui", &rCoreSet)
  , mbIsPresentation(false)
{
    get(m_pCbResWinInit,"resize");
    get(m_pCbCenterWindow,"center");
    get(m_pCbOpenFullScreen,"open");
    get(m_pCbDispDocTitle,"display");
    get(m_pCbHideViewerMenubar,"menubar");
    get(m_pCbHideViewerToolbar,"toolbar");
    get(m_pCbHideViewerWindowControls,"window");
    get(m_pCbTransitionEffects,"effects");
    get(m_pRbAllBookmarkLevels,"allbookmarks");
    get(m_pRbVisibleBookmarkLevels,"visiblebookmark");
    get(m_pNumBookmarkLevels,"visiblelevel");

    m_pRbAllBookmarkLevels->SetToggleHdl( LINK( this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl ) );
    m_pRbVisibleBookmarkLevels->SetToggleHdl( LINK( this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl ) );
}


ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
    disposeOnce();
}


void ImpPDFTabViewerPage::dispose()
{
    m_pCbResWinInit.clear();
    m_pCbCenterWindow.clear();
    m_pCbOpenFullScreen.clear();
    m_pCbDispDocTitle.clear();
    m_pCbHideViewerMenubar.clear();
    m_pCbHideViewerToolbar.clear();
    m_pCbHideViewerWindowControls.clear();
    m_pCbTransitionEffects.clear();
    m_pRbAllBookmarkLevels.clear();
    m_pRbVisibleBookmarkLevels.clear();
    m_pNumBookmarkLevels.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_NOARG( ImpPDFTabViewerPage, ToggleRbBookmarksHdl, RadioButton&, void )
{
    m_pNumBookmarkLevels->Enable( m_pRbVisibleBookmarkLevels->IsChecked() );
}


VclPtr<SfxTabPage> ImpPDFTabViewerPage::Create( vcl::Window* pParent,
                                                const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabViewerPage>::Create( pParent, *rAttrSet );
}


void ImpPDFTabViewerPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbHideViewerMenubar = m_pCbHideViewerMenubar->IsChecked();
    paParent->mbHideViewerToolbar = m_pCbHideViewerToolbar->IsChecked( );
    paParent->mbHideViewerWindowControls = m_pCbHideViewerWindowControls->IsChecked();
    paParent->mbResizeWinToInit = m_pCbResWinInit->IsChecked();
    paParent->mbOpenInFullScreenMode = m_pCbOpenFullScreen->IsChecked();
    paParent->mbCenterWindow = m_pCbCenterWindow->IsChecked();
    paParent->mbDisplayPDFDocumentTitle = m_pCbDispDocTitle->IsChecked();
    paParent->mbUseTransitionEffects = m_pCbTransitionEffects->IsChecked();
    paParent->mnOpenBookmarkLevels = m_pRbAllBookmarkLevels->IsChecked() ?
                                     -1 : static_cast<sal_Int32>(m_pNumBookmarkLevels->GetValue());
}


void ImpPDFTabViewerPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    m_pCbHideViewerMenubar->Check( paParent->mbHideViewerMenubar );
    m_pCbHideViewerToolbar->Check( paParent->mbHideViewerToolbar );
    m_pCbHideViewerWindowControls->Check( paParent->mbHideViewerWindowControls );

    m_pCbResWinInit->Check( paParent->mbResizeWinToInit );
    m_pCbOpenFullScreen->Check( paParent->mbOpenInFullScreenMode );
    m_pCbCenterWindow->Check( paParent->mbCenterWindow );
    m_pCbDispDocTitle->Check( paParent->mbDisplayPDFDocumentTitle );
    mbIsPresentation = paParent->mbIsPresentation;
    m_pCbTransitionEffects->Check( paParent->mbUseTransitionEffects );
    m_pCbTransitionEffects->Enable( mbIsPresentation );
    if( paParent->mnOpenBookmarkLevels < 0 )
    {
        m_pRbAllBookmarkLevels->Check();
        m_pNumBookmarkLevels->Enable( false );
    }
    else
    {
        m_pRbVisibleBookmarkLevels->Check();
        m_pNumBookmarkLevels->Enable();
        m_pNumBookmarkLevels->SetValue( paParent->mnOpenBookmarkLevels );
    }
}


/// The Security preferences tab page
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage(vcl::Window* i_pParent, const SfxItemSet& i_rCoreSet)
    : SfxTabPage(i_pParent, "PdfSecurityPage","filter/ui/pdfsecuritypage.ui", &i_rCoreSet)
    , msUserPwdTitle( PDFFilterResId( STR_PDF_EXPORT_UDPWD ) )
    , mbHaveOwnerPassword( false )
    , mbHaveUserPassword( false )
    , msOwnerPwdTitle( PDFFilterResId( STR_PDF_EXPORT_ODPWD ) )
{
    get(mpPbSetPwd, "setpassword");
    msStrSetPwd = get<vcl::Window>("setpasswordstitle")->GetText();

    get(mpUserPwdSet, "userpwdset");
    get(mpUserPwdUnset, "userpwdunset");
    get(mpUserPwdPdfa, "userpwdpdfa");

    get(mpOwnerPwdSet, "ownerpwdset");
    get(mpOwnerPwdUnset, "ownerpwdunset");
    get(mpOwnerPwdPdfa, "ownerpwdpdfa");

    get(mpPrintPermissions, "printing");
    get(mpRbPrintNone, "printnone");
    get(mpRbPrintLowRes, "printlow");
    get(mpRbPrintHighRes, "printhigh");

    get(mpChangesAllowed, "changes");
    get(mpRbChangesNone, "changenone");
    get(mpRbChangesInsDel, "changeinsdel");
    get(mpRbChangesFillForm, "changeform");
    get(mpRbChangesComment, "changecomment");
    get(mpRbChangesAnyNoCopy, "changeany");

    get(mpContent, "content");
    get(mpCbEnableCopy, "enablecopy");
    get(mpCbEnableAccessibility, "enablea11y");

    mpPbSetPwd->SetClickHdl( LINK( this, ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl ) );
}


ImpPDFTabSecurityPage::~ImpPDFTabSecurityPage()
{
    disposeOnce();
}


void ImpPDFTabSecurityPage::dispose()
{
    mpPbSetPwd.clear();
    mpUserPwdSet.clear();
    mpUserPwdUnset.clear();
    mpUserPwdPdfa.clear();
    mpOwnerPwdSet.clear();
    mpOwnerPwdUnset.clear();
    mpOwnerPwdPdfa.clear();
    mpPrintPermissions.clear();
    mpRbPrintNone.clear();
    mpRbPrintLowRes.clear();
    mpRbPrintHighRes.clear();
    mpChangesAllowed.clear();
    mpRbChangesNone.clear();
    mpRbChangesInsDel.clear();
    mpRbChangesFillForm.clear();
    mpRbChangesComment.clear();
    mpRbChangesAnyNoCopy.clear();
    mpContent.clear();
    mpCbEnableCopy.clear();
    mpCbEnableAccessibility.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> ImpPDFTabSecurityPage::Create( vcl::Window* pParent,
                                                  const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabSecurityPage>::Create( pParent, *rAttrSet );
}


void ImpPDFTabSecurityPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    // please note that in PDF/A-1a mode even if this are copied back,
    // the security settings are forced disabled in PDFExport::Export
    paParent->mbEncrypt = mbHaveUserPassword;
    paParent->mxPreparedPasswords = mxPreparedPasswords;

    paParent->mbRestrictPermissions = mbHaveOwnerPassword;
    paParent->maPreparedOwnerPassword = maPreparedOwnerPassword;

    // verify print status
    paParent->mnPrint = 0;
    if( mpRbPrintLowRes->IsChecked() )
        paParent->mnPrint = 1;
    else if( mpRbPrintHighRes->IsChecked() )
        paParent->mnPrint = 2;

    // verify changes permitted
    paParent->mnChangesAllowed = 0;

    if( mpRbChangesInsDel->IsChecked() )
        paParent->mnChangesAllowed = 1;
    else if( mpRbChangesFillForm->IsChecked() )
        paParent->mnChangesAllowed = 2;
    else if( mpRbChangesComment->IsChecked() )
        paParent->mnChangesAllowed = 3;
    else if( mpRbChangesAnyNoCopy->IsChecked() )
        paParent->mnChangesAllowed = 4;

    paParent->mbCanCopyOrExtract = mpCbEnableCopy->IsChecked();
    paParent->mbCanExtractForAccessibility = mpCbEnableAccessibility->IsChecked();
}


void ImpPDFTabSecurityPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    switch( paParent->mnPrint )
    {
    default:
    case 0:
        mpRbPrintNone->Check();
        break;
    case 1:
        mpRbPrintLowRes->Check();
        break;
    case 2:
        mpRbPrintHighRes->Check();
        break;
    }

    switch( paParent->mnChangesAllowed )
    {
    default:
    case 0:
        mpRbChangesNone->Check();
        break;
    case 1:
        mpRbChangesInsDel->Check();
        break;
    case 2:
        mpRbChangesFillForm->Check();
        break;
    case 3:
        mpRbChangesComment->Check();
        break;
    case 4:
        mpRbChangesAnyNoCopy->Check();
        break;
    }

    mpCbEnableCopy->Check( paParent->mbCanCopyOrExtract );
    mpCbEnableAccessibility->Check( paParent->mbCanExtractForAccessibility );

    // set the status of this windows, according to the PDFA selection
    enablePermissionControls();

    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();

    if (pGeneralPage)
        ImplPDFASecurityControl(!pGeneralPage->IsPdfaSelected());
}


IMPL_LINK_NOARG(ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl, Button*, void)
{
    ScopedVclPtrInstance< SfxPasswordDialog > aPwdDialog( this, &msUserPwdTitle );
    aPwdDialog->SetMinLen( 0 );
    aPwdDialog->ShowMinLengthText(false);
    aPwdDialog->ShowExtras( SfxShowExtras::CONFIRM | SfxShowExtras::PASSWORD2 | SfxShowExtras::CONFIRM2 );
    aPwdDialog->SetText(msStrSetPwd);
    aPwdDialog->SetGroup2Text(msOwnerPwdTitle);
    aPwdDialog->AllowAsciiOnly();
    if( aPwdDialog->Execute() == RET_OK )  // OK issued get password and set it
    {
        OUString aUserPW( aPwdDialog->GetPassword() );
        OUString aOwnerPW( aPwdDialog->GetPassword2() );

        mbHaveUserPassword = !aUserPW.isEmpty();
        mbHaveOwnerPassword = !aOwnerPW.isEmpty();

        mxPreparedPasswords = vcl::PDFWriter::InitEncryption( aOwnerPW, aUserPW, true );

        if( mbHaveOwnerPassword )
        {
            maPreparedOwnerPassword = comphelper::OStorageHelper::CreatePackageEncryptionData( aOwnerPW );
        }
        else
            maPreparedOwnerPassword = Sequence< NamedValue >();
    }
    enablePermissionControls();
}

void ImpPDFTabSecurityPage::enablePermissionControls()
{
    bool bIsPDFASel = false;
    ImpPDFTabDialog* pParent = static_cast<ImpPDFTabDialog*>(GetTabDialog());
    ImpPDFTabGeneralPage* pGeneralPage = pParent ? pParent->getGeneralPage() : nullptr;
    if (pGeneralPage)
    {
        bIsPDFASel = pGeneralPage->IsPdfaSelected();
    }
    if (bIsPDFASel)
    {
        mpUserPwdPdfa->Show();
        mpUserPwdSet->Hide();
        mpUserPwdUnset->Hide();
    }
    else
    {
        if (mbHaveUserPassword && IsEnabled())
        {
            mpUserPwdSet->Show();
            mpUserPwdUnset->Hide();
            mpUserPwdPdfa->Hide();
        }
        else
        {
            mpUserPwdUnset->Show();
            mpUserPwdSet->Hide();
            mpUserPwdPdfa->Hide();
        }
    }

    bool bLocalEnable = mbHaveOwnerPassword && IsEnabled();
    if (bIsPDFASel)
    {
        mpOwnerPwdPdfa->Show();
        mpOwnerPwdSet->Hide();
        mpOwnerPwdUnset->Hide();
    }
    else
    {
        if (bLocalEnable)
        {
            mpOwnerPwdSet->Show();
            mpOwnerPwdUnset->Hide();
            mpOwnerPwdPdfa->Hide();
        }
        else
        {
            mpOwnerPwdUnset->Show();
            mpOwnerPwdSet->Hide();
            mpOwnerPwdPdfa->Hide();
        }
    }

    mpPrintPermissions->Enable(bLocalEnable);
    mpChangesAllowed->Enable(bLocalEnable);
    mpContent->Enable(bLocalEnable);
}


// This tab page is under control of the PDF/A-1a checkbox:
// TODO: implement a method to do it.
void    ImpPDFTabSecurityPage::ImplPDFASecurityControl( bool bEnableSecurity )
{
    if( bEnableSecurity )
    {
        Enable();
    // after enable, check the status of control as if the dialog was initialized
    }
    else
        Enable( false );

    enablePermissionControls();
}


/// The link preferences tab page (relative and other stuff)
ImpPDFTabLinksPage::ImpPDFTabLinksPage( vcl::Window* pParent,
                                              const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, "PdfLinksPage","filter/ui/pdflinkspage.ui",&rCoreSet ),

    mbOpnLnksDefaultUserState( false ),
    mbOpnLnksLaunchUserState( false ),
    mbOpnLnksBrowserUserState( false )
{
    get(m_pCbExprtBmkrToNmDst,"export");
    get(m_pCbOOoToPDFTargets ,"convert");
    get(m_pCbExportRelativeFsysLinks ,"exporturl");
    get(m_pRbOpnLnksDefault ,"default");
    get(m_pRbOpnLnksLaunch ,"openpdf");
    get(m_pRbOpnLnksBrowser ,"openinternet");
}


ImpPDFTabLinksPage::~ImpPDFTabLinksPage()
{
    disposeOnce();
}


void ImpPDFTabLinksPage::dispose()
{
    m_pCbExprtBmkrToNmDst.clear();
    m_pCbOOoToPDFTargets.clear();
    m_pCbExportRelativeFsysLinks.clear();
    m_pRbOpnLnksDefault.clear();
    m_pRbOpnLnksLaunch.clear();
    m_pRbOpnLnksBrowser.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> ImpPDFTabLinksPage::Create( vcl::Window* pParent,
                                               const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabLinksPage>::Create( pParent, *rAttrSet );
}


void ImpPDFTabLinksPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbExportRelativeFsysLinks = m_pCbExportRelativeFsysLinks->IsChecked();

    bool bIsPDFASel = false;
    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();
    if (pGeneralPage)
        bIsPDFASel = pGeneralPage->IsPdfaSelected();
    // if PDF/A-1 was not selected while exiting dialog...
    if( !bIsPDFASel )
    {
        // ...get the control states
        mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
        mbOpnLnksLaunchUserState =  m_pRbOpnLnksLaunch->IsChecked();
        mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
    }
    // the control states, or the saved is used
    // to form the stored selection
    paParent->mnViewPDFMode = 0;
    if( mbOpnLnksBrowserUserState )
        paParent->mnViewPDFMode = 2;
    else if( mbOpnLnksLaunchUserState )
        paParent->mnViewPDFMode = 1;

    paParent->mbConvertOOoTargets = m_pCbOOoToPDFTargets->IsChecked();
    paParent->mbExportBmkToPDFDestination = m_pCbExprtBmkrToNmDst->IsChecked();
}


void ImpPDFTabLinksPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    m_pCbOOoToPDFTargets->Check( paParent->mbConvertOOoTargets );
    m_pCbExprtBmkrToNmDst->Check( paParent->mbExportBmkToPDFDestination );

    m_pRbOpnLnksDefault->SetClickHdl( LINK( this, ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl ) );
    m_pRbOpnLnksBrowser->SetClickHdl( LINK( this, ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl ) );

    m_pCbExportRelativeFsysLinks->Check( paParent->mbExportRelativeFsysLinks );
    switch( paParent->mnViewPDFMode )
    {
    default:
    case 0:
        m_pRbOpnLnksDefault->Check();
        mbOpnLnksDefaultUserState = true;
        break;
    case 1:
        m_pRbOpnLnksLaunch->Check();
        mbOpnLnksLaunchUserState = true;
        break;
    case 2:
        m_pRbOpnLnksBrowser->Check();
        mbOpnLnksBrowserUserState = true;
        break;
    }

    // now check the status of PDF/A selection
    // and set the link action accordingly
    // PDF/A-1 doesn't allow launch action on links

    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();
    if (pGeneralPage)
        ImplPDFALinkControl(!pGeneralPage->mpCbPDFA1b->IsChecked());
}


/** Called from general tab, with PDFA/1 selection status.
    Retrieves/store the status of Launch action selection.
 */
void ImpPDFTabLinksPage::ImplPDFALinkControl( bool bEnableLaunch )
{
    // set the value and position of link type selection
    if( bEnableLaunch )
    {
        m_pRbOpnLnksLaunch->Enable();
        // restore user state with no PDF/A-1 selected
        m_pRbOpnLnksDefault->Check( mbOpnLnksDefaultUserState );
        m_pRbOpnLnksLaunch->Check( mbOpnLnksLaunchUserState );
        m_pRbOpnLnksBrowser->Check( mbOpnLnksBrowserUserState );
    }
    else
    {
        // save user state with no PDF/A-1 selected
        mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
        mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
        mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
        m_pRbOpnLnksLaunch->Enable( false );
        if( mbOpnLnksLaunchUserState )
            m_pRbOpnLnksBrowser->Check();
    }
}


/// Reset the memory of Launch action present when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl, Button*, void)
{
    mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
    mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
    mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
}


/// Reset the memory of a launch action present when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl, Button*, void)
{
    mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
    mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
    mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
}


ImplErrorDialog::ImplErrorDialog(const std::set< vcl::PDFWriter::ErrorCode >& rErrors)
    : MessageDialog(nullptr, "WarnPDFDialog", "filter/ui/warnpdfdialog.ui")
{
    get(m_pErrors, "errors");
    get(m_pExplanation, "message");

    Size aSize(LogicToPixel(Size(100, 75), MapMode(MapUnit::MapAppFont)));
    m_pErrors->set_width_request(aSize.Width());
    m_pErrors->set_height_request(aSize.Height());
    m_pExplanation->set_width_request(aSize.Width());
    m_pExplanation->set_height_request(aSize.Height());

    // load images
    Image aWarnImg( BitmapEx( PDFFilterResId( IMG_WARN ) ) );
    Image aErrImg( BitmapEx( PDFFilterResId( IMG_ERR ) ) );

    for( std::set<vcl::PDFWriter::ErrorCode>::const_iterator it = rErrors.begin();
         it != rErrors.end(); ++it )
    {
        switch( *it )
        {
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDFA:
        {
            sal_uInt16 nPos = m_pErrors->InsertEntry( OUString( PDFFilterResId( STR_WARN_TRANSP_PDFA_SHORT ) ),
                                                aWarnImg );
            m_pErrors->SetEntryData( nPos, new OUString( PDFFilterResId( STR_WARN_TRANSP_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDF13:
        {
            sal_uInt16 nPos = m_pErrors->InsertEntry( OUString( PDFFilterResId( STR_WARN_TRANSP_VERSION_SHORT ) ),
                                                aWarnImg );
            m_pErrors->SetEntryData( nPos, new OUString( PDFFilterResId( STR_WARN_TRANSP_VERSION ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_FormAction_Omitted_PDFA:
        {
            sal_uInt16 nPos = m_pErrors->InsertEntry( OUString( PDFFilterResId( STR_WARN_FORMACTION_PDFA_SHORT ) ),
                                                aWarnImg );
            m_pErrors->SetEntryData( nPos, new OUString( PDFFilterResId( STR_WARN_FORMACTION_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Converted:
        {
            sal_uInt16 nPos = m_pErrors->InsertEntry( OUString( PDFFilterResId( STR_WARN_TRANSP_CONVERTED_SHORT ) ),
                                                aWarnImg );
            m_pErrors->SetEntryData( nPos, new OUString( PDFFilterResId( STR_WARN_TRANSP_CONVERTED ) ) );
        }
        break;
        case vcl::PDFWriter::Error_Signature_Failed:
        {
            sal_uInt16 nPos = m_pErrors->InsertEntry( OUString( PDFFilterResId( STR_ERR_SIGNATURE_FAILED ) ),
                                                aErrImg );
            m_pErrors->SetEntryData( nPos, new OUString( PDFFilterResId( STR_ERR_PDF_EXPORT_ABORTED ) ) );
        }
        break;
        default:
            break;
        }
    }

    if( m_pErrors->GetEntryCount() > 0 )
    {
        m_pErrors->SelectEntryPos( 0 );
        OUString* pStr = static_cast<OUString*>(m_pErrors->GetEntryData( 0 ));
        m_pExplanation->SetText( pStr ? *pStr : OUString() );
    }

    m_pErrors->SetSelectHdl( LINK( this, ImplErrorDialog, SelectHdl ) );
}


ImplErrorDialog::~ImplErrorDialog()
{
    disposeOnce();
}


void ImplErrorDialog::dispose()
{
    // free strings again
    for( sal_Int32 n = 0; n < m_pErrors->GetEntryCount(); n++ )
        delete static_cast<OUString*>(m_pErrors->GetEntryData( n ));
    m_pErrors.clear();
    m_pExplanation.clear();
    MessageDialog::dispose();
}


IMPL_LINK_NOARG(ImplErrorDialog, SelectHdl, ListBox&, void)
{
    OUString* pStr = static_cast<OUString*>(m_pErrors->GetSelectEntryData());
    m_pExplanation->SetText( pStr ? *pStr : OUString() );
}


/// The digital signatures tab page
ImpPDFTabSigningPage::ImpPDFTabSigningPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfSignPage","filter/ui/pdfsignpage.ui", &rCoreSet)
    , maSignCertificate()
{
    get(mpEdSignCert, "cert");
    get(mpPbSignCertSelect, "select");
    get(mpPbSignCertClear, "clear");
    get(mpEdSignPassword, "password");
    get(mpEdSignLocation, "location");
    get(mpEdSignContactInfo, "contact");
    get(mpEdSignReason, "reason");
    get(mpLBSignTSA, "tsa");

    mpPbSignCertSelect->Enable();
    mpPbSignCertSelect->SetClickHdl( LINK( this, ImpPDFTabSigningPage, ClickmaPbSignCertSelect ) );
    mpPbSignCertClear->SetClickHdl( LINK( this, ImpPDFTabSigningPage, ClickmaPbSignCertClear ) );
}


ImpPDFTabSigningPage::~ImpPDFTabSigningPage()
{
    disposeOnce();
}


void ImpPDFTabSigningPage::dispose()
{
    mpEdSignCert.clear();
    mpPbSignCertSelect.clear();
    mpPbSignCertClear.clear();
    mpEdSignPassword.clear();
    mpEdSignLocation.clear();
    mpEdSignContactInfo.clear();
    mpEdSignReason.clear();
    mpLBSignTSA.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_NOARG( ImpPDFTabSigningPage, ClickmaPbSignCertSelect, Button*, void )
{

    Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2" ) );

    // The use may provide a description while choosing a certificate.
    OUString aDescription;
    maSignCertificate = xSigner->chooseCertificate(aDescription);

    if (maSignCertificate.is())
    {
        mpEdSignCert->SetText(maSignCertificate->getSubjectName());
        mpPbSignCertClear->Enable();
        mpEdSignLocation->Enable();
        mpEdSignPassword->Enable();
        mpEdSignContactInfo->Enable();
        mpEdSignReason->Enable();
        mpEdSignReason->SetText(aDescription);

        try
        {
            boost::optional<css::uno::Sequence<OUString>> aTSAURLs(officecfg::Office::Common::Security::Scripting::TSAURLs::get());
            if (aTSAURLs)
            {
                const css::uno::Sequence<OUString>& rTSAURLs = aTSAURLs.get();
                for (auto i = rTSAURLs.begin(); i != rTSAURLs.end(); ++i)
                {
                    mpLBSignTSA->InsertEntry( *i );
                }
            }
        }
        catch (const uno::Exception &e)
        {
            SAL_INFO("filter.pdf", "TSAURLsDialog::TSAURLsDialog(): caught exception" << e.Message);
        }

        // If more than only the "None" entry is there, enable the ListBox
        if (mpLBSignTSA->GetEntryCount() > 1)
            mpLBSignTSA->Enable();
    }
}


IMPL_LINK_NOARG( ImpPDFTabSigningPage, ClickmaPbSignCertClear, Button*, void )
{
    mpEdSignCert->SetText("");
    maSignCertificate.clear();
    mpPbSignCertClear->Enable( false );
    mpEdSignLocation->Enable( false );
    mpEdSignPassword->Enable( false );
    mpEdSignContactInfo->Enable( false );
    mpEdSignReason->Enable( false );
    mpLBSignTSA->Enable( false );
}


VclPtr<SfxTabPage> ImpPDFTabSigningPage::Create( vcl::Window* pParent,
                                                 const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabSigningPage>::Create( pParent, *rAttrSet );
}


void ImpPDFTabSigningPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbSignPDF = maSignCertificate.is();
    paParent->maSignCertificate = maSignCertificate;
    paParent->msSignLocation = mpEdSignLocation->GetText();
    paParent->msSignPassword = mpEdSignPassword->GetText();
    paParent->msSignContact = mpEdSignContactInfo->GetText();
    paParent->msSignReason = mpEdSignReason->GetText();
    // Entry 0 is 'None'
    if (mpLBSignTSA->GetSelectEntryPos() >= 1)
        paParent->msSignTSA = mpLBSignTSA->GetSelectEntry();
}


void ImpPDFTabSigningPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{

    mpEdSignLocation->Enable( false );
    mpEdSignPassword->Enable( false );
    mpEdSignContactInfo->Enable( false );
    mpEdSignReason->Enable( false );
    mpLBSignTSA->Enable( false );
    mpPbSignCertClear->Enable( false );

    if (paParent->mbSignPDF)
    {
        mpEdSignPassword->SetText(paParent->msSignPassword);
        mpEdSignLocation->SetText(paParent->msSignLocation);
        mpEdSignContactInfo->SetText(paParent->msSignContact);
        mpEdSignReason->SetText(paParent->msSignReason);
        maSignCertificate = paParent->maSignCertificate;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
