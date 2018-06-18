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
#include <strings.hrc>
#include <officecfg/Office/Common.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/passwd.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/resmgr.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

OUString PDFFilterResId(const char* pId)
{
    return Translate::get(pId, Translate::Create("flt"));
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
    mbUseReferenceXObject( false ),
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

    // check if source document is a presentation or a spreadsheet or a textdocument
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
    aRet.push_back(comphelper::makePropertyValue("UseReferenceXObject", mbUseReferenceXObject));

    return comphelper::concatSequences(maConfigItem.GetFilterData(), comphelper::containerToSequence(aRet));
}


ImpPDFTabGeneralPage::ImpPDFTabGeneralPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "filter/ui/pdfgeneralpage.ui", "PdfGeneralPage", &rCoreSet)
    , mbTaggedPDFUserSelection(false)
    , mbExportFormFieldsUserSelection(false)
    , mbIsPresentation(false)
    , mbIsSpreadsheet(false)
    , mbIsWriter(false)
    , mpaParent(nullptr)
    , mxRbAll(m_xBuilder->weld_radio_button("all"))
    , mxRbRange(m_xBuilder->weld_radio_button("range"))
    , mxRbSelection(m_xBuilder->weld_radio_button("selection"))
    , mxEdPages(m_xBuilder->weld_entry("pages"))
    , mxSelectedSheets(m_xBuilder->weld_label("selectedsheets"))
    , mxRbLosslessCompression(m_xBuilder->weld_radio_button("losslesscompress"))
    , mxRbJPEGCompression(m_xBuilder->weld_radio_button("jpegcompress"))
    , mxQualityFrame(m_xBuilder->weld_widget("qualityframe"))
    , mxNfQuality(m_xBuilder->weld_metric_spin_button("quality", FUNIT_PERCENT))
    , mxCbReduceImageResolution(m_xBuilder->weld_check_button("reduceresolution"))
    , mxCoReduceImageResolution(m_xBuilder->weld_combo_box_text("resolution"))
    , mxCbPDFA1b(m_xBuilder->weld_check_button("pdfa"))
    , mxCbTaggedPDF(m_xBuilder->weld_check_button("tagged"))
    , mxCbExportFormFields(m_xBuilder->weld_check_button("forms"))
    , mxFormsFrame(m_xBuilder->weld_widget("formsframe"))
    , mxLbFormsFormat(m_xBuilder->weld_combo_box_text("format"))
    , mxCbAllowDuplicateFieldNames(m_xBuilder->weld_check_button("allowdups"))
    , mxCbExportBookmarks(m_xBuilder->weld_check_button("bookmarks"))
    , mxCbExportHiddenSlides(m_xBuilder->weld_check_button("hiddenpages"))
    , mxCbExportNotes(m_xBuilder->weld_check_button("comments"))
    , mxCbViewPDF(m_xBuilder->weld_check_button("viewpdf"))
    , mxCbUseReferenceXObject(m_xBuilder->weld_check_button("usereferencexobject"))
    , mxCbExportNotesPages(m_xBuilder->weld_check_button("notes"))
    , mxCbExportOnlyNotesPages(m_xBuilder->weld_check_button("onlynotes"))
    , mxCbExportEmptyPages(m_xBuilder->weld_check_button("emptypages"))
    , mxCbExportPlaceholders(m_xBuilder->weld_check_button("exportplaceholders"))
    , mxCbAddStream(m_xBuilder->weld_check_button("embed"))
    , mxCbWatermark(m_xBuilder->weld_check_button("watermark"))
    , mxFtWatermark(m_xBuilder->weld_label("watermarklabel"))
    , mxEdWatermark(m_xBuilder->weld_entry("watermarkentry"))
    , mxSlidesFt(m_xBuilder->weld_label("slides"))
    , mxSheetsFt(m_xBuilder->weld_label("selectedsheets"))
{
}

ImpPDFTabGeneralPage::~ImpPDFTabGeneralPage()
{
    disposeOnce();
}

void ImpPDFTabGeneralPage::dispose()
{
    mpaParent.clear();
    SfxTabPage::dispose();
}

void ImpPDFTabGeneralPage::SetFilterConfigItem( ImpPDFTabDialog* paParent )
{
    mpaParent = paParent;

    // init this class data
    mxRbRange->connect_toggled( LINK( this, ImpPDFTabGeneralPage, TogglePagesHdl ) );

    mxRbAll->set_active(true);
    mxRbAll->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleAllHdl ) );
    TogglePagesHdl();

    mxRbSelection->set_sensitive( paParent->mbSelectionPresent );
    if ( paParent->mbSelectionPresent )
        mxRbSelection->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleSelectionHdl ) );
    mbIsPresentation = paParent->mbIsPresentation;
    mbIsWriter = paParent->mbIsWriter;
    mbIsSpreadsheet = paParent->mbIsSpreadsheet;

    mxCbExportEmptyPages->set_sensitive( mbIsWriter );
    mxCbExportPlaceholders->set_sensitive( mbIsWriter );

    mxRbLosslessCompression->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const bool bUseLosslessCompression = paParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        mxRbLosslessCompression->set_active(true);
    else
        mxRbJPEGCompression->set_active(true);

    mxNfQuality->set_value( paParent->mnQuality, FUNIT_PERCENT );
    mxQualityFrame->set_sensitive(!bUseLosslessCompression);

    mxCbReduceImageResolution->connect_toggled(LINK(this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl));
    const bool  bReduceImageResolution = paParent->mbReduceImageResolution;
    mxCbReduceImageResolution->set_active( bReduceImageResolution );
    OUString aStrRes = OUString::number( paParent->mnMaxImageResolution ) + " DPI";
    mxCoReduceImageResolution->set_entry_text(aStrRes);
    mxCoReduceImageResolution->set_sensitive( bReduceImageResolution );
    mxCbWatermark->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleWatermarkHdl ) );
    mxFtWatermark->set_sensitive(false );
    mxEdWatermark->set_sensitive( false );
    mxCbPDFA1b->connect_toggled(LINK(this, ImpPDFTabGeneralPage, ToggleExportPDFAHdl));
    switch( paParent->mnPDFTypeSelection )
    {
    default:
    case 0: mxCbPDFA1b->set_active( false ); // PDF 1.5
        break;
    case 1: mxCbPDFA1b->set_active(true); // PDF/A-1a
        break;
    }
    ToggleExportPDFAHdl( *mxCbPDFA1b );

    mxCbExportFormFields->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl ) );

    // get the form values, for use with PDF/A-1 selection interface
    mbTaggedPDFUserSelection = paParent->mbUseTaggedPDF;
    mbExportFormFieldsUserSelection = paParent->mbExportFormFields;

    if( !mxCbPDFA1b->get_active() )
    {
        // the value for PDF/A set by the ToggleExportPDFAHdl method called before
        mxCbTaggedPDF->set_active( mbTaggedPDFUserSelection  );
        mxCbExportFormFields->set_active( mbExportFormFieldsUserSelection );
    }

    mxLbFormsFormat->set_active(static_cast<sal_uInt16>(paParent->mnFormsType));
    mxCbAllowDuplicateFieldNames->set_active( paParent->mbAllowDuplicateFieldNames );
    mxFormsFrame->set_sensitive( paParent->mbExportFormFields );

    mxCbExportBookmarks->set_active( paParent->mbExportBookmarks );

    mxCbExportNotes->set_active( paParent->mbExportNotes );
    mxCbViewPDF->set_active( paParent->mbViewPDF);

    if ( mbIsPresentation )
    {
        mxRbRange->set_label(mxSlidesFt->get_label());
        mxCbExportNotesPages->show();
        mxCbExportNotesPages->set_active(paParent->mbExportNotesPages);
        mxCbExportNotesPages->connect_toggled( LINK(this, ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl ) );
        mxCbExportOnlyNotesPages->show();
        mxCbExportOnlyNotesPages->set_active(paParent->mbExportOnlyNotesPages);
        // tdf#116473 Initially enable Export only note pages option depending on the checked state of Export notes pages option
        mxCbExportOnlyNotesPages->set_sensitive(mxCbExportNotesPages->get_active());
        mxCbExportHiddenSlides->show();
        mxCbExportHiddenSlides->set_active(paParent->mbExportHiddenSlides);
    }
    else
    {
        mxCbExportNotesPages->show(false);
        mxCbExportNotesPages->set_active(false);
        mxCbExportOnlyNotesPages->show(false);
        mxCbExportOnlyNotesPages->set_active(false);
        mxCbExportHiddenSlides->show(false);
        mxCbExportHiddenSlides->set_active(false);
    }

    if( mbIsSpreadsheet )
    {
        mxRbSelection->set_label(mxSheetsFt->get_label());
        // tdf#105965 Make Selection/Selected sheets the default PDF export range setting for spreadsheets
        mxRbSelection->set_active(true);
    }

    mxCbExportPlaceholders->show(mbIsWriter);
    if( !mbIsWriter )
    {
        mxCbExportPlaceholders->set_active(false);
    }
    mxCbExportEmptyPages->set_active(!paParent->mbIsSkipEmptyPages);
    mxCbExportPlaceholders->set_active(paParent->mbIsExportPlaceholders);

    mxCbAddStream->show();
    mxCbAddStream->set_active(paParent->mbAddStream);

    mxCbAddStream->connect_toggled(LINK(this, ImpPDFTabGeneralPage, ToggleAddStreamHdl));
    ToggleAddStreamHdl(*mxCbAddStream); // init addstream dependencies
}

void ImpPDFTabGeneralPage::GetFilterConfigItem( ImpPDFTabDialog* paParent )
{
    // updating the FilterData sequence and storing FilterData to configuration
    paParent->mbUseLosslessCompression = mxRbLosslessCompression->get_active();
    paParent->mnQuality = static_cast<sal_Int32>(mxNfQuality->get_value(FUNIT_PERCENT));
    paParent->mbReduceImageResolution = mxCbReduceImageResolution->get_active();
    paParent->mnMaxImageResolution = mxCoReduceImageResolution->get_active_text().toInt32();
    paParent->mbExportNotes = mxCbExportNotes->get_active();
    paParent->mbViewPDF = mxCbViewPDF->get_active();
    paParent->mbUseReferenceXObject = mxCbUseReferenceXObject->get_active();
    if ( mbIsPresentation )
    {
        paParent->mbExportNotesPages = mxCbExportNotesPages->get_active();
        paParent->mbExportOnlyNotesPages = mxCbExportOnlyNotesPages->get_active();
    }
    paParent->mbExportBookmarks = mxCbExportBookmarks->get_active();
    if ( mbIsPresentation )
        paParent->mbExportHiddenSlides = mxCbExportHiddenSlides->get_active();

    paParent->mbIsSkipEmptyPages = !mxCbExportEmptyPages->get_active();
    paParent->mbIsExportPlaceholders = mxCbExportPlaceholders->get_active();
    paParent->mbAddStream = mxCbAddStream->get_visible() && mxCbAddStream->get_active();

    paParent->mbIsRangeChecked = false;
    if( mxRbRange->get_active() )
    {
        paParent->mbIsRangeChecked = true;
        paParent->msPageRange = mxEdPages->get_text(); //FIXME all right on other languages ?
    }
    else if( mxRbSelection->get_active() )
    {
        paParent->mbSelectionIsChecked = mxRbSelection->get_active();
    }

    paParent->mnPDFTypeSelection = 0;
    if( mxCbPDFA1b->get_active() )
    {
        paParent->mnPDFTypeSelection = 1;
        paParent->mbUseTaggedPDF =  mbTaggedPDFUserSelection;
        paParent->mbExportFormFields = mbExportFormFieldsUserSelection;
    }
    else
    {
        paParent->mbUseTaggedPDF =  mxCbTaggedPDF->get_active();
        paParent->mbExportFormFields = mxCbExportFormFields->get_active();
    }

    if( mxCbWatermark->get_active() )
        paParent->maWatermarkText = mxEdWatermark->get_text();

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    paParent->mnFormsType = mxLbFormsFormat->get_active();
    paParent->mbAllowDuplicateFieldNames = mxCbAllowDuplicateFieldNames->get_active();
}

VclPtr<SfxTabPage> ImpPDFTabGeneralPage::Create( TabPageParent pParent,
                                                 const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabGeneralPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAllHdl, weld::ToggleButton&, void)
{
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, TogglePagesHdl, weld::ToggleButton&, void)
{
    TogglePagesHdl();
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleSelectionHdl, weld::ToggleButton&, void)
{
    EnableExportNotesPages();
}

void ImpPDFTabGeneralPage::TogglePagesHdl()
{
    mxEdPages->set_sensitive( mxRbRange->get_active() );
    if (mxRbRange->get_active())
        mxEdPages->grab_focus();
}

void ImpPDFTabGeneralPage::EnableExportNotesPages()
{
    if ( mbIsPresentation )
    {
        mxCbExportNotesPages->set_sensitive( !mxRbSelection->get_active() );
        mxCbExportOnlyNotesPages->set_sensitive( !mxRbSelection->get_active() && mxCbExportNotesPages->get_active() );
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl, weld::ToggleButton&, void)
{
    mxFormsFrame->set_sensitive(mxCbExportFormFields->get_active());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl, weld::ToggleButton&, void)
{
    mxCbExportOnlyNotesPages->set_sensitive(mxCbExportNotesPages->get_active());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleCompressionHdl, weld::ToggleButton&, void)
{
    mxQualityFrame->set_sensitive(mxRbJPEGCompression->get_active());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl, weld::ToggleButton&, void)
{
    mxCoReduceImageResolution->set_sensitive(mxCbReduceImageResolution->get_active());
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleWatermarkHdl, weld::ToggleButton&, void)
{
    mxEdWatermark->set_sensitive(mxCbWatermark->get_active());
    mxFtWatermark->set_sensitive(mxCbWatermark->get_active());
    if (mxCbWatermark->get_active())
        mxEdWatermark->grab_focus();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAddStreamHdl, weld::ToggleButton&, void)
{
    if (mxCbAddStream->get_visible())
    {
        if( mxCbAddStream->get_active() )
        {
            mxRbAll->set_active(true);
            mxRbRange->set_sensitive( false );
            mxRbSelection->set_sensitive( false );
            mxEdPages->set_sensitive( false );
            mxRbAll->set_sensitive( false );
        }
        else
        {
            mxRbAll->set_sensitive(true);
            mxRbRange->set_sensitive(true);
            mxRbSelection->set_sensitive(true);
        }
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportPDFAHdl, weld::ToggleButton&, void)
{
    // set the security page status (and its controls as well)
    ImpPDFTabSecurityPage* pSecPage = mpaParent ? mpaParent->getSecurityPage() : nullptr;
    if (pSecPage)
    {
        pSecPage->ImplPDFASecurityControl(!mxCbPDFA1b->get_active());
    }

    // PDF/A-1 needs tagged PDF, so force disable the control, will be forced in pdfexport.
    bool bPDFA1Sel = mxCbPDFA1b->get_active();
    mxFormsFrame->set_sensitive(bPDFA1Sel);
    if(bPDFA1Sel)
    {
        // store the values of subordinate controls
        mbTaggedPDFUserSelection = mxCbTaggedPDF->get_active();
        mxCbTaggedPDF->set_active(true);
        mxCbTaggedPDF->set_sensitive(false);
        mbExportFormFieldsUserSelection = mxCbExportFormFields->get_active();
        mxCbExportFormFields->set_active(false);
        mxCbExportFormFields->set_sensitive(false);
    }
    else
    {
        // retrieve the values of subordinate controls
        mxCbTaggedPDF->set_sensitive(true);
        mxCbTaggedPDF->set_active(mbTaggedPDFUserSelection);
        mxCbExportFormFields->set_active(mbExportFormFieldsUserSelection);
        mxCbExportFormFields->set_sensitive(true);
    }

    // PDF/A-1 doesn't allow launch action, so enable/disable the selection on
    // Link page
    ImpPDFTabLinksPage* pLinksPage = mpaParent ? mpaParent->getLinksPage() : nullptr;
    if (pLinksPage)
        pLinksPage->ImplPDFALinkControl(!mxCbPDFA1b->get_active());

    // if a password was set, inform the user that this will not be used in PDF/A case
    if( mxCbPDFA1b->get_active() && pSecPage && pSecPage->hasPassword() )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xContainer.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  PDFFilterResId(STR_WARN_PASSWORD_PDFA)));
        xBox->run();
    }
}

/// The option features tab page
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "filter/ui/pdfviewpage.ui", "PdfViewPage", &rCoreSet)
    , mbUseCTLFont(false)
    , mxRbOpnPageOnly(m_xBuilder->weld_radio_button("pageonly"))
    , mxRbOpnOutline(m_xBuilder->weld_radio_button("outline"))
    , mxRbOpnThumbs(m_xBuilder->weld_radio_button("thumbs"))
    , mxNumInitialPage(m_xBuilder->weld_spin_button("page"))
    , mxRbMagnDefault(m_xBuilder->weld_radio_button("fitdefault"))
    , mxRbMagnFitWin(m_xBuilder->weld_radio_button("fitwin"))
    , mxRbMagnFitWidth(m_xBuilder->weld_radio_button("fitwidth"))
    , mxRbMagnFitVisible(m_xBuilder->weld_radio_button("fitvis"))
    , mxRbMagnZoom(m_xBuilder->weld_radio_button("fitzoom"))
    , mxNumZoom(m_xBuilder->weld_spin_button("zoom"))
    , mxRbPgLyDefault(m_xBuilder->weld_radio_button("defaultlayout"))
    , mxRbPgLySinglePage(m_xBuilder->weld_radio_button("singlelayout"))
    , mxRbPgLyContinue(m_xBuilder->weld_radio_button("contlayout"))
    , mxRbPgLyContinueFacing(m_xBuilder->weld_radio_button("contfacinglayout"))
    , mxCbPgLyFirstOnLeft(m_xBuilder->weld_check_button("firstonleft"))
{
    mxRbMagnDefault->connect_toggled( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mxRbMagnFitWin->connect_toggled( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mxRbMagnFitWidth->connect_toggled( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mxRbMagnFitVisible->connect_toggled( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    mxRbMagnZoom->connect_toggled( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
}

ImpPDFTabOpnFtrPage::~ImpPDFTabOpnFtrPage()
{
}

VclPtr<SfxTabPage> ImpPDFTabOpnFtrPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabOpnFtrPage>::Create(pParent, *rAttrSet);
}

void ImpPDFTabOpnFtrPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mnInitialView = 0;
    if( mxRbOpnOutline->get_active() )
        paParent->mnInitialView = 1;
    else if( mxRbOpnThumbs->get_active() )
        paParent->mnInitialView = 2;

    paParent->mnMagnification = 0;
    if( mxRbMagnFitWin->get_active() )
        paParent->mnMagnification = 1;
    else if( mxRbMagnFitWidth->get_active() )
        paParent->mnMagnification = 2;
    else if( mxRbMagnFitVisible->get_active() )
        paParent->mnMagnification = 3;
    else if( mxRbMagnZoom->get_active() )
    {
        paParent->mnMagnification = 4;
        paParent->mnZoom = mxNumZoom->get_value();
    }

    paParent->mnInitialPage = mxNumInitialPage->get_value();

    paParent->mnPageLayout = 0;
    if( mxRbPgLySinglePage->get_active() )
        paParent->mnPageLayout = 1;
    else if( mxRbPgLyContinue->get_active() )
        paParent->mnPageLayout = 2;
    else if( mxRbPgLyContinueFacing->get_active() )
        paParent->mnPageLayout = 3;

    paParent->mbFirstPageLeft = mbUseCTLFont && mxCbPgLyFirstOnLeft->get_active();
}

void ImpPDFTabOpnFtrPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    mbUseCTLFont = paParent->mbUseCTLFont;
    switch( paParent->mnPageLayout )
    {
    default:
    case 0:
        mxRbPgLyDefault->set_active(true);
        break;
    case 1:
        mxRbPgLySinglePage->set_active(true);
        break;
    case 2:
        mxRbPgLyContinue->set_active(true);
        break;
    case 3:
        mxRbPgLyContinueFacing->set_active(true);
        break;
    }

    switch( paParent->mnInitialView )
    {
    default:
    case 0:
        mxRbOpnPageOnly->set_active(true);
        break;
    case 1:
        mxRbOpnOutline->set_active(true);
        break;
    case 2:
        mxRbOpnThumbs->set_active(true);
        break;
    }

    switch( paParent->mnMagnification )
    {
    default:
    case 0:
        mxRbMagnDefault->set_active(true);
        mxNumZoom->set_sensitive(false);
        break;
    case 1:
        mxRbMagnFitWin->set_active(true);
        mxNumZoom->set_sensitive(false);
        break;
    case 2:
        mxRbMagnFitWidth->set_active(true);
        mxNumZoom->set_sensitive(false);
        break;
    case 3:
        mxRbMagnFitVisible->set_active(true);
        mxNumZoom->set_sensitive(false);
        break;
    case 4:
        mxRbMagnZoom->set_active(true);
        mxNumZoom->set_sensitive(true);
        break;
    }

    mxNumZoom->set_value(paParent->mnZoom);
    mxNumInitialPage->set_value(paParent->mnInitialPage);

    if (!mbUseCTLFont)
        mxCbPgLyFirstOnLeft->hide();
    else
    {
        mxRbPgLyContinueFacing->connect_toggled(LINK(this, ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl));
        mxCbPgLyFirstOnLeft->set_active(paParent->mbFirstPageLeft);
        ToggleRbPgLyContinueFacingHdl();
    }
}

IMPL_LINK_NOARG(ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl, weld::ToggleButton&, void)
{
    ToggleRbPgLyContinueFacingHdl();
}

void ImpPDFTabOpnFtrPage::ToggleRbPgLyContinueFacingHdl()
{
    mxCbPgLyFirstOnLeft->set_sensitive(mxRbPgLyContinueFacing->get_active());
}

IMPL_LINK_NOARG( ImpPDFTabOpnFtrPage, ToggleRbMagnHdl, weld::ToggleButton&, void )
{
    mxNumZoom->set_sensitive(mxRbMagnZoom->get_active());
}

/// The Viewer preferences tab page
ImpPDFTabViewerPage::ImpPDFTabViewerPage(TabPageParent pParent, const SfxItemSet& rCoreSet )
    : SfxTabPage(pParent, "filter/ui/pdfuserinterfacepage.ui", "PdfUserInterfacePage", &rCoreSet)
    , mbIsPresentation(false)
    , m_xCbResWinInit(m_xBuilder->weld_check_button("resize"))
    , m_xCbCenterWindow(m_xBuilder->weld_check_button("center"))
    , m_xCbOpenFullScreen(m_xBuilder->weld_check_button("open"))
    , m_xCbDispDocTitle(m_xBuilder->weld_check_button("display"))
    , m_xCbHideViewerMenubar(m_xBuilder->weld_check_button("menubar"))
    , m_xCbHideViewerToolbar(m_xBuilder->weld_check_button("toolbar"))
    , m_xCbHideViewerWindowControls(m_xBuilder->weld_check_button("window"))
    , m_xCbTransitionEffects(m_xBuilder->weld_check_button("effects"))
    , m_xRbAllBookmarkLevels(m_xBuilder->weld_radio_button("allbookmarks"))
    , m_xRbVisibleBookmarkLevels(m_xBuilder->weld_radio_button("visiblebookmark"))
    , m_xNumBookmarkLevels(m_xBuilder->weld_spin_button("visiblelevel"))
{
    m_xRbAllBookmarkLevels->connect_toggled(LINK(this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl));
    m_xRbVisibleBookmarkLevels->connect_toggled(LINK(this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl));
}

ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
}

IMPL_LINK_NOARG( ImpPDFTabViewerPage, ToggleRbBookmarksHdl, weld::ToggleButton&, void )
{
    m_xNumBookmarkLevels->set_sensitive(m_xRbVisibleBookmarkLevels->get_active());
}

VclPtr<SfxTabPage> ImpPDFTabViewerPage::Create( TabPageParent pParent,
                                                const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabViewerPage>::Create(pParent, *rAttrSet);
}

void ImpPDFTabViewerPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbHideViewerMenubar = m_xCbHideViewerMenubar->get_active();
    paParent->mbHideViewerToolbar = m_xCbHideViewerToolbar->get_active();
    paParent->mbHideViewerWindowControls = m_xCbHideViewerWindowControls->get_active();
    paParent->mbResizeWinToInit = m_xCbResWinInit->get_active();
    paParent->mbOpenInFullScreenMode = m_xCbOpenFullScreen->get_active();
    paParent->mbCenterWindow = m_xCbCenterWindow->get_active();
    paParent->mbDisplayPDFDocumentTitle = m_xCbDispDocTitle->get_active();
    paParent->mbUseTransitionEffects = m_xCbTransitionEffects->get_active();
    paParent->mnOpenBookmarkLevels = m_xRbAllBookmarkLevels->get_active() ?
                                     -1 : static_cast<sal_Int32>(m_xNumBookmarkLevels->get_value());
}

void ImpPDFTabViewerPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    m_xCbHideViewerMenubar->set_active( paParent->mbHideViewerMenubar );
    m_xCbHideViewerToolbar->set_active( paParent->mbHideViewerToolbar );
    m_xCbHideViewerWindowControls->set_active( paParent->mbHideViewerWindowControls );

    m_xCbResWinInit->set_active( paParent->mbResizeWinToInit );
    m_xCbOpenFullScreen->set_active( paParent->mbOpenInFullScreenMode );
    m_xCbCenterWindow->set_active( paParent->mbCenterWindow );
    m_xCbDispDocTitle->set_active( paParent->mbDisplayPDFDocumentTitle );
    mbIsPresentation = paParent->mbIsPresentation;
    m_xCbTransitionEffects->set_active( paParent->mbUseTransitionEffects );
    m_xCbTransitionEffects->set_sensitive( mbIsPresentation );
    if( paParent->mnOpenBookmarkLevels < 0 )
    {
        m_xRbAllBookmarkLevels->set_active(true);
        m_xNumBookmarkLevels->set_sensitive( false );
    }
    else
    {
        m_xRbVisibleBookmarkLevels->set_active(true);
        m_xNumBookmarkLevels->set_sensitive(true);
        m_xNumBookmarkLevels->set_value(paParent->mnOpenBookmarkLevels);
    }
}

/// The Security preferences tab page
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage(TabPageParent i_pParent, const SfxItemSet& i_rCoreSet)
    : SfxTabPage(i_pParent, "filter/ui/pdfsecuritypage.ui", "PdfSecurityPage", &i_rCoreSet)
    , msUserPwdTitle( PDFFilterResId( STR_PDF_EXPORT_UDPWD ) )
    , mbHaveOwnerPassword( false )
    , mbHaveUserPassword( false )
    , msOwnerPwdTitle( PDFFilterResId( STR_PDF_EXPORT_ODPWD ) )
    , mxPbSetPwd(m_xBuilder->weld_button("setpassword"))
    , mxUserPwdSet(m_xBuilder->weld_widget("userpwdset"))
    , mxUserPwdUnset(m_xBuilder->weld_widget("userpwdunset"))
    , mxUserPwdPdfa(m_xBuilder->weld_widget("userpwdpdfa"))
    , mxOwnerPwdSet(m_xBuilder->weld_widget("ownerpwdset"))
    , mxOwnerPwdUnset(m_xBuilder->weld_widget("ownerpwdunset"))
    , mxOwnerPwdPdfa(m_xBuilder->weld_widget("ownerpwdpdfa"))
    , mxPrintPermissions(m_xBuilder->weld_widget("printing"))
    , mxRbPrintNone(m_xBuilder->weld_radio_button("printnone"))
    , mxRbPrintLowRes(m_xBuilder->weld_radio_button("printlow"))
    , mxRbPrintHighRes(m_xBuilder->weld_radio_button("printhigh"))
    , mxChangesAllowed(m_xBuilder->weld_widget("changes"))
    , mxRbChangesNone(m_xBuilder->weld_radio_button("changenone"))
    , mxRbChangesInsDel(m_xBuilder->weld_radio_button("changeinsdel"))
    , mxRbChangesFillForm(m_xBuilder->weld_radio_button("changeform"))
    , mxRbChangesComment(m_xBuilder->weld_radio_button("changecomment"))
    , mxRbChangesAnyNoCopy(m_xBuilder->weld_radio_button("changeany"))
    , mxContent(m_xBuilder->weld_widget("content"))
    , mxCbEnableCopy(m_xBuilder->weld_check_button("enablecopy"))
    , mxCbEnableAccessibility(m_xBuilder->weld_check_button("enablea11y"))
    , mxPasswordTitle(m_xBuilder->weld_label("setpasswordstitle"))
{
    msStrSetPwd = mxPasswordTitle->get_label();
    mxPbSetPwd->connect_clicked(LINK(this, ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl));
}

ImpPDFTabSecurityPage::~ImpPDFTabSecurityPage()
{
}

VclPtr<SfxTabPage> ImpPDFTabSecurityPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabSecurityPage>::Create(pParent, *rAttrSet);
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
    if (mxRbPrintLowRes->get_active())
        paParent->mnPrint = 1;
    else if (mxRbPrintHighRes->get_active())
        paParent->mnPrint = 2;

    // verify changes permitted
    paParent->mnChangesAllowed = 0;

    if( mxRbChangesInsDel->get_active() )
        paParent->mnChangesAllowed = 1;
    else if( mxRbChangesFillForm->get_active() )
        paParent->mnChangesAllowed = 2;
    else if( mxRbChangesComment->get_active() )
        paParent->mnChangesAllowed = 3;
    else if( mxRbChangesAnyNoCopy->get_active() )
        paParent->mnChangesAllowed = 4;

    paParent->mbCanCopyOrExtract = mxCbEnableCopy->get_active();
    paParent->mbCanExtractForAccessibility = mxCbEnableAccessibility->get_active();
}

void ImpPDFTabSecurityPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    switch( paParent->mnPrint )
    {
    default:
    case 0:
        mxRbPrintNone->set_active(true);
        break;
    case 1:
        mxRbPrintLowRes->set_active(true);
        break;
    case 2:
        mxRbPrintHighRes->set_active(true);
        break;
    }

    switch( paParent->mnChangesAllowed )
    {
    default:
    case 0:
        mxRbChangesNone->set_active(true);
        break;
    case 1:
        mxRbChangesInsDel->set_active(true);
        break;
    case 2:
        mxRbChangesFillForm->set_active(true);
        break;
    case 3:
        mxRbChangesComment->set_active(true);
        break;
    case 4:
        mxRbChangesAnyNoCopy->set_active(true);
        break;
    }

    mxCbEnableCopy->set_active(paParent->mbCanCopyOrExtract);
    mxCbEnableAccessibility->set_active(paParent->mbCanExtractForAccessibility);

    // set the status of this windows, according to the PDFA selection
    enablePermissionControls();

    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();

    if (pGeneralPage)
        ImplPDFASecurityControl(!pGeneralPage->IsPdfaSelected());
}

IMPL_LINK_NOARG(ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl, weld::Button&, void)
{
    SfxPasswordDialog aPwdDialog(m_xContainer.get(), &msUserPwdTitle);
    aPwdDialog.SetMinLen(0);
    aPwdDialog.ShowMinLengthText(false);
    aPwdDialog.ShowExtras( SfxShowExtras::CONFIRM | SfxShowExtras::PASSWORD2 | SfxShowExtras::CONFIRM2 );
    aPwdDialog.set_title(msStrSetPwd);
    aPwdDialog.SetGroup2Text(msOwnerPwdTitle);
    aPwdDialog.AllowAsciiOnly();
    if (aPwdDialog.execute() == RET_OK)  // OK issued get password and set it
    {
        OUString aUserPW(aPwdDialog.GetPassword());
        OUString aOwnerPW(aPwdDialog.GetPassword2());

        mbHaveUserPassword = !aUserPW.isEmpty();
        mbHaveOwnerPassword = !aOwnerPW.isEmpty();

        mxPreparedPasswords = vcl::PDFWriter::InitEncryption( aOwnerPW, aUserPW );

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
        mxUserPwdPdfa->show();
        mxUserPwdSet->hide();
        mxUserPwdUnset->hide();
    }
    else
    {
        if (mbHaveUserPassword && IsEnabled())
        {
            mxUserPwdSet->show();
            mxUserPwdUnset->hide();
            mxUserPwdPdfa->hide();
        }
        else
        {
            mxUserPwdUnset->show();
            mxUserPwdSet->hide();
            mxUserPwdPdfa->hide();
        }
    }

    bool bLocalEnable = mbHaveOwnerPassword && IsEnabled();
    if (bIsPDFASel)
    {
        mxOwnerPwdPdfa->show();
        mxOwnerPwdSet->hide();
        mxOwnerPwdUnset->hide();
    }
    else
    {
        if (bLocalEnable)
        {
            mxOwnerPwdSet->show();
            mxOwnerPwdUnset->hide();
            mxOwnerPwdPdfa->hide();
        }
        else
        {
            mxOwnerPwdUnset->show();
            mxOwnerPwdSet->hide();
            mxOwnerPwdPdfa->hide();
        }
    }

    mxPrintPermissions->set_sensitive(bLocalEnable);
    mxChangesAllowed->set_sensitive(bLocalEnable);
    mxContent->set_sensitive(bLocalEnable);
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
ImpPDFTabLinksPage::ImpPDFTabLinksPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "filter/ui/pdflinkspage.ui", "PdfLinksPage", &rCoreSet)
    , mbOpnLnksDefaultUserState(false)
    , mbOpnLnksLaunchUserState(false)
    , mbOpnLnksBrowserUserState(false)
    , m_xCbExprtBmkrToNmDst(m_xBuilder->weld_check_button("export"))
    , m_xCbOOoToPDFTargets(m_xBuilder->weld_check_button("convert"))
    , m_xCbExportRelativeFsysLinks(m_xBuilder->weld_check_button("exporturl"))
    , m_xRbOpnLnksDefault(m_xBuilder->weld_radio_button("default"))
    , m_xRbOpnLnksLaunch(m_xBuilder->weld_radio_button("openpdf"))
    , m_xRbOpnLnksBrowser(m_xBuilder->weld_radio_button("openinternet"))
{
}

ImpPDFTabLinksPage::~ImpPDFTabLinksPage()
{
}

VclPtr<SfxTabPage> ImpPDFTabLinksPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabLinksPage>::Create(pParent, *rAttrSet);
}

void ImpPDFTabLinksPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbExportRelativeFsysLinks = m_xCbExportRelativeFsysLinks->get_active();

    bool bIsPDFASel = false;
    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();
    if (pGeneralPage)
        bIsPDFASel = pGeneralPage->IsPdfaSelected();
    // if PDF/A-1 was not selected while exiting dialog...
    if( !bIsPDFASel )
    {
        // ...get the control states
        mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
        mbOpnLnksLaunchUserState =  m_xRbOpnLnksLaunch->get_active();
        mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
    }
    // the control states, or the saved is used
    // to form the stored selection
    paParent->mnViewPDFMode = 0;
    if( mbOpnLnksBrowserUserState )
        paParent->mnViewPDFMode = 2;
    else if( mbOpnLnksLaunchUserState )
        paParent->mnViewPDFMode = 1;

    paParent->mbConvertOOoTargets = m_xCbOOoToPDFTargets->get_active();
    paParent->mbExportBmkToPDFDestination = m_xCbExprtBmkrToNmDst->get_active();
}

void ImpPDFTabLinksPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    m_xCbOOoToPDFTargets->set_active(paParent->mbConvertOOoTargets);
    m_xCbExprtBmkrToNmDst->set_active(paParent->mbExportBmkToPDFDestination);

    m_xRbOpnLnksDefault->connect_toggled(LINK(this, ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl));
    m_xRbOpnLnksBrowser->connect_toggled(LINK(this, ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl));

    m_xCbExportRelativeFsysLinks->set_active(paParent->mbExportRelativeFsysLinks);
    switch( paParent->mnViewPDFMode )
    {
    default:
    case 0:
        m_xRbOpnLnksDefault->set_active(true);
        mbOpnLnksDefaultUserState = true;
        break;
    case 1:
        m_xRbOpnLnksLaunch->set_active(true);
        mbOpnLnksLaunchUserState = true;
        break;
    case 2:
        m_xRbOpnLnksBrowser->set_active(true);
        mbOpnLnksBrowserUserState = true;
        break;
    }

    // now check the status of PDF/A selection
    // and set the link action accordingly
    // PDF/A-1 doesn't allow launch action on links

    ImpPDFTabGeneralPage* pGeneralPage = paParent->getGeneralPage();
    if (pGeneralPage)
        ImplPDFALinkControl(!pGeneralPage->mxCbPDFA1b->get_active());
}


/** Called from general tab, with PDFA/1 selection status.
    Retrieves/store the status of Launch action selection.
 */
void ImpPDFTabLinksPage::ImplPDFALinkControl( bool bEnableLaunch )
{
    // set the value and position of link type selection
    if( bEnableLaunch )
    {
        m_xRbOpnLnksLaunch->set_sensitive(true);
        // restore user state with no PDF/A-1 selected
        m_xRbOpnLnksDefault->set_active(mbOpnLnksDefaultUserState);
        m_xRbOpnLnksLaunch->set_active(mbOpnLnksLaunchUserState);
        m_xRbOpnLnksBrowser->set_active(mbOpnLnksBrowserUserState);
    }
    else
    {
        // save user state with no PDF/A-1 selected
        mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
        mbOpnLnksLaunchUserState = m_xRbOpnLnksLaunch->get_active();
        mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
        m_xRbOpnLnksLaunch->set_sensitive(false);
        if (mbOpnLnksLaunchUserState)
            m_xRbOpnLnksBrowser->set_active(true);
    }
}

/// Reset the memory of Launch action present when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl, weld::ToggleButton&, void)
{
    mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
    mbOpnLnksLaunchUserState = m_xRbOpnLnksLaunch->get_active();
    mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
}

/// Reset the memory of a launch action present when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl, weld::ToggleButton&, void)
{
    mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
    mbOpnLnksLaunchUserState = m_xRbOpnLnksLaunch->get_active();
    mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
}

ImplErrorDialog::ImplErrorDialog(weld::Window* pParent, const std::set<vcl::PDFWriter::ErrorCode>& rErrors)
    : MessageDialogController(pParent, "filter/ui/warnpdfdialog.ui", "WarnPDFDialog", "grid")
    , m_xErrors(m_xBuilder->weld_tree_view("errors"))
    , m_xExplanation(m_xBuilder->weld_label("message"))
{
    int nWidth = m_xErrors->get_approximate_digit_width() * 26;
    int nHeight = m_xErrors->get_height_rows(9);
    m_xErrors->set_size_request(nWidth, nHeight);
    m_xExplanation->set_size_request(nWidth, nHeight);

    for (auto const& error : rErrors)
    {
        switch(error)
        {
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDFA:
            m_xErrors->append(PDFFilterResId(STR_WARN_TRANSP_PDFA), PDFFilterResId(STR_WARN_TRANSP_PDFA_SHORT), "dialog-warning");
            break;
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDF13:
            m_xErrors->append(PDFFilterResId(STR_WARN_TRANSP_VERSION), PDFFilterResId(STR_WARN_TRANSP_VERSION_SHORT), "dialog-warning");
            break;
        case vcl::PDFWriter::Warning_FormAction_Omitted_PDFA:
            m_xErrors->append(PDFFilterResId(STR_WARN_FORMACTION_PDFA), PDFFilterResId(STR_WARN_FORMACTION_PDFA_SHORT), "dialog-warning");
            break;
        case vcl::PDFWriter::Warning_Transparency_Converted:
            m_xErrors->append(PDFFilterResId(STR_WARN_TRANSP_CONVERTED), PDFFilterResId(STR_WARN_TRANSP_CONVERTED_SHORT), "dialog-warning");
            break;
        case vcl::PDFWriter::Error_Signature_Failed:
            m_xErrors->append(PDFFilterResId(STR_ERR_PDF_EXPORT_ABORTED), PDFFilterResId(STR_ERR_SIGNATURE_FAILED), "dialog-error");
            break;
        default:
            break;
        }
    }

    if (m_xErrors->n_children() > 0)
    {
        m_xErrors->select(0);
        m_xExplanation->set_label(m_xErrors->get_id(0));
    }

    m_xErrors->connect_changed(LINK(this, ImplErrorDialog, SelectHdl));
}

IMPL_LINK_NOARG(ImplErrorDialog, SelectHdl, weld::TreeView&, void)
{
    OUString aExplanation = m_xErrors->get_selected_id();
    m_xExplanation->set_label(aExplanation);
}

/// The digital signatures tab page
ImpPDFTabSigningPage::ImpPDFTabSigningPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "filter/ui/pdfsignpage.ui", "PdfSignPage", &rCoreSet)
    , maSignCertificate()
    , mxEdSignCert(m_xBuilder->weld_entry("cert"))
    , mxPbSignCertSelect(m_xBuilder->weld_button("select"))
    , mxPbSignCertClear(m_xBuilder->weld_button("clear"))
    , mxEdSignPassword(m_xBuilder->weld_entry("password"))
    , mxEdSignLocation(m_xBuilder->weld_entry("location"))
    , mxEdSignContactInfo(m_xBuilder->weld_entry("contact"))
    , mxEdSignReason(m_xBuilder->weld_entry("reason"))
    , mxLBSignTSA(m_xBuilder->weld_combo_box_text("tsa"))
{
    mxPbSignCertSelect->set_sensitive(false);
    mxPbSignCertSelect->connect_clicked(LINK(this, ImpPDFTabSigningPage, ClickmaPbSignCertSelect));
    mxPbSignCertClear->connect_clicked(LINK(this, ImpPDFTabSigningPage, ClickmaPbSignCertClear));
}

ImpPDFTabSigningPage::~ImpPDFTabSigningPage()
{
}

IMPL_LINK_NOARG(ImpPDFTabSigningPage, ClickmaPbSignCertSelect, weld::Button&, void)
{
    Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2" ) );

    // The use may provide a description while choosing a certificate.
    OUString aDescription;
    maSignCertificate = xSigner->chooseCertificate(aDescription);

    if (maSignCertificate.is())
    {
        mxEdSignCert->set_text(maSignCertificate->getSubjectName());
        mxPbSignCertClear->set_sensitive(true);
        mxEdSignLocation->set_sensitive(true);
        mxEdSignPassword->set_sensitive(true);
        mxEdSignContactInfo->set_sensitive(true);
        mxEdSignReason->set_sensitive(true);
        mxEdSignReason->set_text(aDescription);

        try
        {
            boost::optional<css::uno::Sequence<OUString>> aTSAURLs(officecfg::Office::Common::Security::Scripting::TSAURLs::get());
            if (aTSAURLs)
            {
                const css::uno::Sequence<OUString>& rTSAURLs = aTSAURLs.get();
                for (auto const& elem : rTSAURLs)
                {
                    mxLBSignTSA->append_text(elem);
                }
            }
        }
        catch (const uno::Exception &e)
        {
            SAL_INFO("filter.pdf", "TSAURLsDialog::TSAURLsDialog(): " << e);
        }

        // If more than only the "None" entry is there, enable the ListBox
        if (mxLBSignTSA->get_count() > 1)
            mxLBSignTSA->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(ImpPDFTabSigningPage, ClickmaPbSignCertClear, weld::Button&, void)
{
    mxEdSignCert->set_text("");
    maSignCertificate.clear();
    mxPbSignCertClear->set_sensitive(false);
    mxEdSignLocation->set_sensitive(false);
    mxEdSignPassword->set_sensitive(false);
    mxEdSignContactInfo->set_sensitive(false);
    mxEdSignReason->set_sensitive(false);
    mxLBSignTSA->set_sensitive(false);
}

VclPtr<SfxTabPage> ImpPDFTabSigningPage::Create( TabPageParent pParent,
                                                 const SfxItemSet* rAttrSet)
{
    return VclPtr<ImpPDFTabSigningPage>::Create(pParent, *rAttrSet);
}

void ImpPDFTabSigningPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbSignPDF = maSignCertificate.is();
    paParent->maSignCertificate = maSignCertificate;
    paParent->msSignLocation = mxEdSignLocation->get_text();
    paParent->msSignPassword = mxEdSignPassword->get_text();
    paParent->msSignContact = mxEdSignContactInfo->get_text();
    paParent->msSignReason = mxEdSignReason->get_text();
    // Entry 0 is 'None'
    if (mxLBSignTSA->get_active() >= 1)
        paParent->msSignTSA = mxLBSignTSA->get_active_text();
}

void ImpPDFTabSigningPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    mxEdSignLocation->set_sensitive(false);
    mxEdSignPassword->set_sensitive(false);
    mxEdSignContactInfo->set_sensitive(false);
    mxEdSignReason->set_sensitive(false);
    mxLBSignTSA->set_sensitive(false);
    mxPbSignCertClear->set_sensitive(false);

    if (paParent->mbSignPDF)
    {
        mxEdSignPassword->set_text(paParent->msSignPassword);
        mxEdSignLocation->set_text(paParent->msSignLocation);
        mxEdSignContactInfo->set_text(paParent->msSignContact);
        mxEdSignReason->set_text(paParent->msSignReason);
        maSignCertificate = paParent->maSignCertificate;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
