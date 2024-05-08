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

#include "impdialog.hxx"
#include <strings.hrc>
#include <officecfg/Office/Common.hxx>
#include <vcl/errinf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/passwd.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sfx2/objsh.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/** Tabbed PDF dialog implementation
    Please note: the default used here are the same as per specification,
    They should be the same in  PDFFilter::implExport and  in PDFExport::PDFExport
 */
ImpPDFTabDialog::ImpPDFTabDialog(weld::Window* pParent, const Sequence< PropertyValue >& rFilterData,
    const Reference< XComponent >& rxDoc)
    : SfxTabDialogController(pParent, u"filter/ui/pdfoptionsdialog.ui"_ustr, u"PdfOptionsDialog"_ustr),
    mrDoc(rxDoc),
    maConfigItem( u"Office.Common/Filter/PDF/Export/", &rFilterData ),
    maConfigI18N( u"Office.Common/I18N/CTL/" ),
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
    mbUseTaggedPDFUserSelection( false ),
    mbExportNotes( true ),
    mbExportNotesInMargin( false ),
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
    mbExportHiddenSlides ( false ),
    mbSinglePageSheets ( false ),
    mnOpenBookmarkLevels( -1 ),

    mbHideViewerToolbar( false ),
    mbHideViewerMenubar( false ),
    mbHideViewerWindowControls( false ),
    mbResizeWinToInit( false ),
    mbCenterWindow( false ),
    mbOpenInFullScreenMode( false ),
    mbDisplayPDFDocumentTitle( false ),
    mnMagnification( 0 ),
    mnInitialView( 1 ),
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

    mbIsPageRangeChecked( false ),
    msPageRange( ' ' ),
    mbIsSheetRangeChecked( false ),
    msSheetRange( ' ' ),

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
        Reference< frame::XController > xController( Reference< frame::XModel >( rxDoc, UNO_QUERY_THROW )->getCurrentController() );
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
            if ( xInfo->supportsService( u"com.sun.star.presentation.PresentationDocument"_ustr ) )
                mbIsPresentation = true;
            if ( xInfo->supportsService( u"com.sun.star.sheet.SpreadsheetDocument"_ustr ) )
                mbIsSpreadsheet = true;
            if ( xInfo->supportsService( u"com.sun.star.text.GenericTextDocument"_ustr ) )
                mbIsWriter = true;
        }
    }
    catch(const RuntimeException &)
    {
    }

    // get the CTL (Complex Text Layout) from general options, returns sal_True if we have a CTL font on our hands.
    mbUseCTLFont = maConfigI18N.ReadBool( u"CTLFont"_ustr, false );

    mbUseLosslessCompression = maConfigItem.ReadBool( u"UseLosslessCompression"_ustr, false );
    mnQuality = maConfigItem.ReadInt32( u"Quality"_ustr, 90 );
    mbReduceImageResolution = maConfigItem.ReadBool( u"ReduceImageResolution"_ustr, false );
    mnMaxImageResolution = maConfigItem.ReadInt32( u"MaxImageResolution"_ustr, 300 );

    // this is always the user selection, independent from the PDF/A forced selection
    mbUseTaggedPDF = maConfigItem.ReadBool( u"UseTaggedPDF"_ustr, false );
    mbUseTaggedPDFUserSelection = mbUseTaggedPDF;

    mnPDFTypeSelection =  maConfigItem.ReadInt32( u"SelectPdfVersion"_ustr, 0 );
    mbPDFUACompliance = maConfigItem.ReadBool(u"PDFUACompliance"_ustr, false);

    if ( mbIsPresentation )
    {
        mbExportNotesPages = maConfigItem.ReadBool( u"ExportNotesPages"_ustr, false );
        mbExportOnlyNotesPages = maConfigItem.ReadBool( u"ExportOnlyNotesPages"_ustr, false );
    }
    mbExportNotes = maConfigItem.ReadBool( u"ExportNotes"_ustr, false );
    if (mbIsWriter)
        mbExportNotesInMargin = maConfigItem.ReadBool( u"ExportNotesInMargin"_ustr, false );
    mbViewPDF = maConfigItem.ReadBool( u"ViewPDFAfterExport"_ustr, false );

    mbExportBookmarks = maConfigItem.ReadBool( u"ExportBookmarks"_ustr, true );
    mbExportBookmarksUserSelection = mbExportBookmarks;
    if ( mbIsPresentation )
        mbExportHiddenSlides = maConfigItem.ReadBool( u"ExportHiddenSlides"_ustr, false );
    if ( mbIsSpreadsheet )
        mbSinglePageSheets = maConfigItem.ReadBool( u"SinglePageSheets"_ustr, false );
    mnOpenBookmarkLevels = maConfigItem.ReadInt32( u"OpenBookmarkLevels"_ustr, -1 );
    mbUseTransitionEffects = maConfigItem.ReadBool( u"UseTransitionEffects"_ustr, true );
    mbIsSkipEmptyPages = maConfigItem.ReadBool( u"IsSkipEmptyPages"_ustr, false );
    mbIsExportPlaceholders = maConfigItem.ReadBool( u"ExportPlaceholders"_ustr, false );
    mbAddStream = maConfigItem.ReadBool( u"IsAddStream"_ustr, false );

    mbExportFormFields = maConfigItem.ReadBool( u"ExportFormFields"_ustr, true );
    mnFormsType = maConfigItem.ReadInt32( u"FormsType"_ustr, 0 );
    if ( ( mnFormsType < 0 ) || ( mnFormsType > 3 ) )
        mnFormsType = 0;
    mbAllowDuplicateFieldNames = maConfigItem.ReadBool( u"AllowDuplicateFieldNames"_ustr, false );

    // prepare values for the Viewer tab page
    mbHideViewerToolbar = maConfigItem.ReadBool( u"HideViewerToolbar"_ustr, false );
    mbHideViewerMenubar = maConfigItem.ReadBool( u"HideViewerMenubar"_ustr, false );
    mbHideViewerWindowControls = maConfigItem.ReadBool( u"HideViewerWindowControls"_ustr, false );
    mbResizeWinToInit = maConfigItem.ReadBool( u"ResizeWindowToInitialPage"_ustr, false );
    mbCenterWindow = maConfigItem.ReadBool( u"CenterWindow"_ustr, false );
    mbOpenInFullScreenMode = maConfigItem.ReadBool( u"OpenInFullScreenMode"_ustr, false );
    mbDisplayPDFDocumentTitle = maConfigItem.ReadBool( u"DisplayPDFDocumentTitle"_ustr, true );

    mnInitialView = maConfigItem.ReadInt32( u"InitialView"_ustr, 0 );
    mnInitialViewUserSelection = mnInitialView;
    mnMagnification = maConfigItem.ReadInt32( u"Magnification"_ustr, 0 );
    mnZoom = maConfigItem.ReadInt32( u"Zoom"_ustr, 100 );
    mnPageLayout = maConfigItem.ReadInt32( u"PageLayout"_ustr, 0 );
    mbFirstPageLeft = maConfigItem.ReadBool( u"FirstPageOnLeft"_ustr, false );
    mnInitialPage = maConfigItem.ReadInt32( u"InitialPage"_ustr, 1 );
    if( mnInitialPage < 1 )
        mnInitialPage = 1;

    // prepare values for the security tab page
    mnPrint = maConfigItem.ReadInt32( u"Printing"_ustr, 2 );
    mnChangesAllowed = maConfigItem.ReadInt32( u"Changes"_ustr, 4 );
    mbCanCopyOrExtract = maConfigItem.ReadBool( u"EnableCopyingOfContent"_ustr, true );
    mbCanExtractForAccessibility = maConfigItem.ReadBool( u"EnableTextAccessForAccessibilityTools"_ustr, true );

    // prepare values for relative links
    mbExportRelativeFsysLinks = maConfigItem.ReadBool( u"ExportLinksRelativeFsys"_ustr, false );

    mnViewPDFMode = maConfigItem.ReadInt32( u"PDFViewSelection"_ustr, 0 );

    mbConvertOOoTargets = maConfigItem.ReadBool( u"ConvertOOoTargetToPDFTarget"_ustr, false );
    mbExportBmkToPDFDestination = maConfigItem.ReadBool( u"ExportBookmarksToPDFDestination"_ustr, false );

    // prepare values for digital signatures
    mbSignPDF = maConfigItem.ReadBool( u"SignPDF"_ustr, false );

    // queue the tab pages for later creation (created when first shown)
    AddTabPage(u"general"_ustr, ImpPDFTabGeneralPage::Create, nullptr );
    if (comphelper::LibreOfficeKit::isActive())
        m_xTabCtrl->remove_page(u"digitalsignatures"_ustr);
    else
        AddTabPage(u"digitalsignatures"_ustr, ImpPDFTabSigningPage::Create, nullptr);
    AddTabPage(u"security"_ustr, ImpPDFTabSecurityPage::Create, nullptr);
    AddTabPage(u"links"_ustr, ImpPDFTabLinksPage::Create, nullptr);
    AddTabPage(u"userinterface"_ustr, ImpPDFTabViewerPage::Create, nullptr);
    AddTabPage(u"initialview"_ustr, ImpPDFTabOpnFtrPage::Create, nullptr);

    SetCurPageId(u"general"_ustr);

    // get the string property value (from sfx2/source/dialog/mailmodel.cxx) to overwrite the text for the Ok button
    OUString sOkButtonText = maConfigItem.ReadString( u"_OkButtonString"_ustr, OUString() );

    // change text on the Ok button: get the relevant string from resources, update it on the button
    // according to the exported pdf file destination: send as e-mail or write to file?
    if (!sOkButtonText.isEmpty())
        GetOKButton().set_label(sOkButtonText);

    GetCancelButton().connect_clicked(LINK(this, ImpPDFTabDialog, CancelHdl));
    GetOKButton().connect_clicked(LINK(this, ImpPDFTabDialog, OkHdl));

    // remove the reset button, not needed in this tabbed dialog
    RemoveResetButton();
}

ImpPDFTabSecurityPage* ImpPDFTabDialog::getSecurityPage() const
{
    SfxTabPage* pSecurityPage = GetTabPage(u"security");
    if (pSecurityPage)
    {
        return static_cast<ImpPDFTabSecurityPage*>(pSecurityPage);
    }
    return nullptr;
}

ImpPDFTabOpnFtrPage * ImpPDFTabDialog::getOpenPage() const
{
    SfxTabPage* pOpenPage = GetTabPage(u"initialview");
    if (pOpenPage)
    {
        return static_cast<ImpPDFTabOpnFtrPage*>(pOpenPage);
    }
    return nullptr;
}

ImpPDFTabLinksPage* ImpPDFTabDialog::getLinksPage() const
{
    SfxTabPage* pLinksPage = GetTabPage(u"links");
    if (pLinksPage)
    {
        return static_cast<ImpPDFTabLinksPage*>(pLinksPage);
    }
    return nullptr;
}


ImpPDFTabGeneralPage* ImpPDFTabDialog::getGeneralPage() const
{
    SfxTabPage* pGeneralPage = GetTabPage(u"general");
    if (pGeneralPage)
    {
        return static_cast<ImpPDFTabGeneralPage*>(pGeneralPage);
    }
    return nullptr;
}

IMPL_LINK_NOARG(ImpPDFTabDialog, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(ImpPDFTabDialog, OkHdl, weld::Button&, void)
{
    if (getGeneralPage()->IsPdfUaSelected())
    {
        SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(mrDoc);
        if (pShell)
        {
            sfx::AccessibilityIssueCollection aCollection = pShell->runAccessibilityCheck();
            auto aIssues = aCollection.getIssues();
            int nIssueCount(aIssues.size());
            if (!aIssues.empty())
            {
                OUString aMessage(FilterResId(STR_WARN_PDFUA_ISSUES, nIssueCount));
                aMessage = aMessage.replaceFirst("%1", OUString::number(nIssueCount));

                std::unique_ptr<weld::MessageDialog> xPDFUADialog(Application::CreateMessageDialog(
                    getGeneralPage()->GetFrameWeld(), VclMessageType::Warning,
                    VclButtonsType::Cancel, aMessage));
                xPDFUADialog->add_button(FilterResId(STR_PDFUA_INVESTIGATE, nIssueCount), RET_NO);
                xPDFUADialog->add_button(FilterResId(STR_PDFUA_IGNORE), RET_YES);
                xPDFUADialog->set_default_response(RET_YES);

                int ret = xPDFUADialog->run();
                if (ret == RET_YES)
                    m_xDialog->response(RET_OK);
                else if (ret == RET_NO)
                {
                    m_xDialog->response(RET_CANCEL);
                    // Show accessibility check Sidebar deck
                    SfxDispatcher* pDispatcher = pShell->GetDispatcher();
                    if (pDispatcher)
                    {
                        const SfxStringItem sDeckName(SID_SIDEBAR_DECK, u"A11yCheckDeck"_ustr);
                        pDispatcher->ExecuteList(SID_SIDEBAR_DECK, SfxCallMode::RECORD,
                                                 { &sDeckName });
                    }
                }
            }
            else
            {
                m_xDialog->response(RET_OK);
            }
        }
        else
        {
            m_xDialog->response(RET_OK);
        }
    }
    else
    {
        m_xDialog->response(RET_OK);
    }
}

ImpPDFTabDialog::~ImpPDFTabDialog()
{
    maConfigItem.WriteModifiedConfig();
    maConfigI18N.WriteModifiedConfig();
}

void ImpPDFTabDialog::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    if (rId == "general")
        static_cast<ImpPDFTabGeneralPage&>(rPage).SetFilterConfigItem(this);
    else if (rId == "userinterface")
    {
        static_cast<ImpPDFTabViewerPage&>(rPage).SetFilterConfigItem(this);
    }
    else if (rId == "initialview")
    {
        static_cast<ImpPDFTabOpnFtrPage&>(rPage).SetFilterConfigItem(this);
    }
    else if (rId == "links")
    {
        static_cast<ImpPDFTabLinksPage&>(rPage).SetFilterConfigItem(this);
    }
    else if (rId == "security")
    {
        static_cast<ImpPDFTabSecurityPage&>(rPage).SetFilterConfigItem(this);
    }
    else if (rId == "digitalsignatures")
    {
        static_cast<ImpPDFTabSigningPage&>(rPage).SetFilterConfigItem(this);
    }
}

Sequence< PropertyValue > ImpPDFTabDialog::GetFilterData()
{
    // updating the FilterData sequence and storing FilterData to configuration
    if (ImpPDFTabGeneralPage* pPage = static_cast<ImpPDFTabGeneralPage*>(GetTabPage(u"general")))
        pPage->GetFilterConfigItem(this);
    if (ImpPDFTabViewerPage* pPage = static_cast<ImpPDFTabViewerPage*>(GetTabPage(u"userinterface")))
        pPage->GetFilterConfigItem(this);
    if (ImpPDFTabOpnFtrPage* pPage = static_cast<ImpPDFTabOpnFtrPage*>(GetTabPage(u"initialview")))
        pPage->GetFilterConfigItem(this);
    if (ImpPDFTabLinksPage* pPage = static_cast<ImpPDFTabLinksPage*>(GetTabPage(u"links")))
        pPage->GetFilterConfigItem(this);
    if (ImpPDFTabSecurityPage* pPage = static_cast<ImpPDFTabSecurityPage*>( GetTabPage(u"security")))
        pPage->GetFilterConfigItem(this);
    if (ImpPDFTabSigningPage* pPage = static_cast<ImpPDFTabSigningPage*>(GetTabPage(u"digitalsignatures")))
        pPage->GetFilterConfigItem(this);

    // prepare the items to be returned
    maConfigItem.WriteBool( u"UseLosslessCompression"_ustr, mbUseLosslessCompression );
    maConfigItem.WriteInt32(u"Quality"_ustr, mnQuality );
    maConfigItem.WriteBool( u"ReduceImageResolution"_ustr, mbReduceImageResolution );
    maConfigItem.WriteInt32(u"MaxImageResolution"_ustr, mnMaxImageResolution );

    // always write the user selection, never the overridden value
    const bool bIsPDFUA = mbPDFUACompliance;
    const bool bIsPDFA = (1 == mnPDFTypeSelection) || (2 == mnPDFTypeSelection) || (3 == mnPDFTypeSelection);
    const bool bUserSelectionTags = bIsPDFA || bIsPDFUA;
    maConfigItem.WriteBool(u"UseTaggedPDF"_ustr, bUserSelectionTags ? mbUseTaggedPDFUserSelection : mbUseTaggedPDF);
    maConfigItem.WriteInt32(u"SelectPdfVersion"_ustr, mnPDFTypeSelection );
    maConfigItem.WriteBool(u"PDFUACompliance"_ustr, mbPDFUACompliance);

    if ( mbIsPresentation )
    {
        maConfigItem.WriteBool( u"ExportNotesPages"_ustr, mbExportNotesPages );
        maConfigItem.WriteBool( u"ExportOnlyNotesPages"_ustr, mbExportOnlyNotesPages );
    }
    maConfigItem.WriteBool( u"ExportNotes"_ustr, mbExportNotes );
    if (mbIsWriter)
        maConfigItem.WriteBool( u"ExportNotesInMargin"_ustr, mbExportNotesInMargin );
    maConfigItem.WriteBool( u"ViewPDFAfterExport"_ustr, mbViewPDF );

    maConfigItem.WriteBool( u"ExportBookmarks"_ustr, mbExportBookmarks );
    if ( mbIsPresentation )
        maConfigItem.WriteBool( u"ExportHiddenSlides"_ustr, mbExportHiddenSlides );
    if ( mbIsSpreadsheet )
        maConfigItem.WriteBool( u"SinglePageSheets"_ustr, mbSinglePageSheets );
    maConfigItem.WriteBool( u"UseTransitionEffects"_ustr, mbUseTransitionEffects );
    maConfigItem.WriteBool( u"IsSkipEmptyPages"_ustr, mbIsSkipEmptyPages );
    maConfigItem.WriteBool( u"ExportPlaceholders"_ustr, mbIsExportPlaceholders );
    maConfigItem.WriteBool( u"IsAddStream"_ustr, mbAddStream );

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    maConfigItem.WriteInt32( u"FormsType"_ustr, mnFormsType );
    maConfigItem.WriteBool( u"ExportFormFields"_ustr, mbExportFormFields );
    maConfigItem.WriteBool( u"AllowDuplicateFieldNames"_ustr, mbAllowDuplicateFieldNames );

    maConfigItem.WriteBool( u"HideViewerToolbar"_ustr, mbHideViewerToolbar );
    maConfigItem.WriteBool( u"HideViewerMenubar"_ustr, mbHideViewerMenubar );
    maConfigItem.WriteBool( u"HideViewerWindowControls"_ustr, mbHideViewerWindowControls );
    maConfigItem.WriteBool( u"ResizeWindowToInitialPage"_ustr, mbResizeWinToInit );
    maConfigItem.WriteBool( u"CenterWindow"_ustr, mbCenterWindow );
    maConfigItem.WriteBool( u"OpenInFullScreenMode"_ustr, mbOpenInFullScreenMode );
    maConfigItem.WriteBool( u"DisplayPDFDocumentTitle"_ustr, mbDisplayPDFDocumentTitle );
    maConfigItem.WriteInt32( u"InitialView"_ustr, mnInitialView );
    maConfigItem.WriteInt32( u"Magnification"_ustr, mnMagnification);
    maConfigItem.WriteInt32( u"Zoom"_ustr, mnZoom );
    maConfigItem.WriteInt32( u"InitialPage"_ustr, mnInitialPage );
    maConfigItem.WriteInt32( u"PageLayout"_ustr, mnPageLayout );
    maConfigItem.WriteBool( u"FirstPageOnLeft"_ustr, mbFirstPageLeft );
    maConfigItem.WriteInt32( u"OpenBookmarkLevels"_ustr, mnOpenBookmarkLevels );

    maConfigItem.WriteBool( u"ExportLinksRelativeFsys"_ustr, mbExportRelativeFsysLinks );
    maConfigItem.WriteInt32(u"PDFViewSelection"_ustr, mnViewPDFMode );
    maConfigItem.WriteBool( u"ConvertOOoTargetToPDFTarget"_ustr, mbConvertOOoTargets );
    maConfigItem.WriteBool( u"ExportBookmarksToPDFDestination"_ustr, mbExportBmkToPDFDestination );

    maConfigItem.WriteBool( u"SignPDF"_ustr, mbSignPDF );

    maConfigItem.WriteInt32( u"Printing"_ustr, mnPrint );
    maConfigItem.WriteInt32( u"Changes"_ustr, mnChangesAllowed );
    maConfigItem.WriteBool( u"EnableCopyingOfContent"_ustr, mbCanCopyOrExtract );
    maConfigItem.WriteBool( u"EnableTextAccessForAccessibilityTools"_ustr, mbCanExtractForAccessibility );

    std::vector<beans::PropertyValue> aRet
    {
        comphelper::makePropertyValue(u"Watermark"_ustr, maWatermarkText),
        comphelper::makePropertyValue(u"EncryptFile"_ustr, mbEncrypt),
        comphelper::makePropertyValue(u"PreparedPasswords"_ustr, mxPreparedPasswords),
        comphelper::makePropertyValue(u"RestrictPermissions"_ustr, mbRestrictPermissions),
        comphelper::makePropertyValue(u"PreparedPermissionPassword"_ustr, maPreparedOwnerPassword)
    };
    if( mbIsPageRangeChecked )
        aRet.push_back(comphelper::makePropertyValue(u"PageRange"_ustr, msPageRange));
    if( mbIsSheetRangeChecked )
        aRet.push_back(comphelper::makePropertyValue(u"SheetRange"_ustr, msSheetRange));
    else if( mbSelectionIsChecked )
        aRet.push_back(comphelper::makePropertyValue(u"Selection"_ustr, maSelection));

    aRet.push_back(comphelper::makePropertyValue(u"SignatureLocation"_ustr, msSignLocation));
    aRet.push_back(comphelper::makePropertyValue(u"SignatureReason"_ustr, msSignReason));
    aRet.push_back(comphelper::makePropertyValue(u"SignatureContactInfo"_ustr, msSignContact));
    aRet.push_back(comphelper::makePropertyValue(u"SignaturePassword"_ustr, msSignPassword));
    aRet.push_back(comphelper::makePropertyValue(u"SignatureCertificate"_ustr, maSignCertificate));
    aRet.push_back(comphelper::makePropertyValue(u"SignatureTSA"_ustr, msSignTSA));
    aRet.push_back(comphelper::makePropertyValue(u"UseReferenceXObject"_ustr, mbUseReferenceXObject));

    return comphelper::concatSequences(maConfigItem.GetFilterData(), comphelper::containerToSequence(aRet));
}


ImpPDFTabGeneralPage::ImpPDFTabGeneralPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"filter/ui/pdfgeneralpage.ui"_ustr, u"PdfGeneralPage"_ustr, &rCoreSet)
    , mbUseTaggedPDFUserSelection(false)
    , mbIsPresentation(false)
    , mbIsSpreadsheet(false)
    , mbIsWriter(false)
    , mpParent(nullptr)
    , mxRbAll(m_xBuilder->weld_radio_button(u"all"_ustr))
    , mxRbPageRange(m_xBuilder->weld_radio_button(u"pagerange"_ustr))
    , mxRbSheetRange(m_xBuilder->weld_radio_button(u"sheetrange"_ustr))
    , mxRbSelection(m_xBuilder->weld_radio_button(u"selection"_ustr))
    , mxEdPages(m_xBuilder->weld_entry(u"pages"_ustr))
    , mxEdSheets(m_xBuilder->weld_entry(u"sheets"_ustr))
    , mxRbLosslessCompression(m_xBuilder->weld_radio_button(u"losslesscompress"_ustr))
    , mxRbJPEGCompression(m_xBuilder->weld_radio_button(u"jpegcompress"_ustr))
    , mxQualityFrame(m_xBuilder->weld_widget(u"qualityframe"_ustr))
    , mxNfQuality(m_xBuilder->weld_metric_spin_button(u"quality"_ustr, FieldUnit::PERCENT))
    , mxCbReduceImageResolution(m_xBuilder->weld_check_button(u"reduceresolution"_ustr))
    , mxCoReduceImageResolution(m_xBuilder->weld_combo_box(u"resolution"_ustr))
    , mxCbPDFA(m_xBuilder->weld_check_button(u"pdfa"_ustr))
    , mxCbPDFUA(m_xBuilder->weld_check_button(u"pdfua"_ustr))
    , mxRbPDFAVersion(m_xBuilder->weld_combo_box(u"pdfaversion"_ustr))
    , mxCbTaggedPDF(m_xBuilder->weld_check_button(u"tagged"_ustr))
    , mxCbExportFormFields(m_xBuilder->weld_check_button(u"forms"_ustr))
    , mxFormsFrame(m_xBuilder->weld_widget(u"formsframe"_ustr))
    , mxLbFormsFormat(m_xBuilder->weld_combo_box(u"format"_ustr))
    , mxCbAllowDuplicateFieldNames(m_xBuilder->weld_check_button(u"allowdups"_ustr))
    , mxCbExportBookmarks(m_xBuilder->weld_check_button(u"bookmarks"_ustr))
    , mxCbExportHiddenSlides(m_xBuilder->weld_check_button(u"hiddenpages"_ustr))
    , mxCbSinglePageSheets(m_xBuilder->weld_check_button(u"singlepagesheets"_ustr))
    , mxCbExportNotes(m_xBuilder->weld_check_button(u"comments"_ustr))
    , mxCbExportNotesInMargin(m_xBuilder->weld_check_button(u"commentsinmargin"_ustr))
    , mxCbViewPDF(m_xBuilder->weld_check_button(u"viewpdf"_ustr))
    , mxCbUseReferenceXObject(m_xBuilder->weld_check_button(u"usereferencexobject"_ustr))
    , mxCbExportNotesPages(m_xBuilder->weld_check_button(u"notes"_ustr))
    , mxCbExportOnlyNotesPages(m_xBuilder->weld_check_button(u"onlynotes"_ustr))
    , mxCbExportEmptyPages(m_xBuilder->weld_check_button(u"emptypages"_ustr))
    , mxCbExportPlaceholders(m_xBuilder->weld_check_button(u"exportplaceholders"_ustr))
    , mxCbAddStream(m_xBuilder->weld_check_button(u"embed"_ustr))
    , mxCbWatermark(m_xBuilder->weld_check_button(u"watermark"_ustr))
    , mxFtWatermark(m_xBuilder->weld_label(u"watermarklabel"_ustr))
    , mxEdWatermark(m_xBuilder->weld_entry(u"watermarkentry"_ustr))
    , mxSlidesFt(m_xBuilder->weld_label(u"slides"_ustr))
    , mxSheetsSelectionFt(m_xBuilder->weld_label(u"selectedsheets"_ustr))
{
}

ImpPDFTabGeneralPage::~ImpPDFTabGeneralPage()
{
    if (mxPasswordUnusedWarnDialog)
        mxPasswordUnusedWarnDialog->response(RET_CANCEL);
}

void ImpPDFTabGeneralPage::SetFilterConfigItem(ImpPDFTabDialog* pParent)
{
    mpParent = pParent;

    // init this class data
    mxRbPageRange->connect_toggled( LINK( this, ImpPDFTabGeneralPage, TogglePagesHdl ) );
    mxRbSheetRange->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleSheetsHdl ) );

    mxRbAll->set_active(true);
    mxRbAll->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleAllHdl ) );
    TogglePagesHdl();
    ToggleSheetsHdl();

    mxRbSelection->set_sensitive( pParent->mbSelectionPresent );
    if ( pParent->mbSelectionPresent )
        mxRbSelection->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleSelectionHdl ) );
    mbIsPresentation = pParent->mbIsPresentation;
    mbIsWriter = pParent->mbIsWriter;
    mbIsSpreadsheet = pParent->mbIsSpreadsheet;

    mxRbLosslessCompression->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const bool bUseLosslessCompression = pParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        mxRbLosslessCompression->set_active(true);
    else
        mxRbJPEGCompression->set_active(true);
    const bool bReadOnlyCompression = !pParent->maConfigItem.IsReadOnly(u"UseLosslessCompression"_ustr);
    mxRbLosslessCompression->set_sensitive(bReadOnlyCompression);
    mxRbJPEGCompression->set_sensitive(bReadOnlyCompression);

    mxNfQuality->set_value( pParent->mnQuality, FieldUnit::PERCENT );
    mxQualityFrame->set_sensitive(
        !bUseLosslessCompression && !pParent->maConfigItem.IsReadOnly(u"Quality"_ustr));

    mxCbReduceImageResolution->connect_toggled(LINK(this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl));
    const bool  bReduceImageResolution = pParent->mbReduceImageResolution;
    mxCbReduceImageResolution->set_active( bReduceImageResolution );
    mxCbReduceImageResolution->set_sensitive(
        !pParent->maConfigItem.IsReadOnly(u"ReduceImageResolution"_ustr));
    OUString aStrRes = OUString::number( pParent->mnMaxImageResolution ) + " DPI";
    mxCoReduceImageResolution->set_entry_text(aStrRes);
    mxCoReduceImageResolution->set_sensitive(
        bReduceImageResolution && !pParent->maConfigItem.IsReadOnly(u"MaxImageResolution"_ustr));

    mxCbWatermark->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleWatermarkHdl ) );
    mxFtWatermark->set_sensitive(false );
    mxEdWatermark->set_sensitive( false );
    mxCbPDFA->connect_toggled(LINK(this, ImpPDFTabGeneralPage, TogglePDFVersionOrUniversalAccessibilityHandle));

    const bool bIsPDFA = (pParent->mnPDFTypeSelection>=1) && (pParent->mnPDFTypeSelection <= 3);
    mxCbPDFA->set_active(bIsPDFA);
    switch( pParent->mnPDFTypeSelection )
    {
    case 1: // PDF/A-1
        mxRbPDFAVersion->set_active_id(u"1"_ustr);
        break;
    case 2: // PDF/A-2
        mxRbPDFAVersion->set_active_id(u"2"_ustr);
        break;
    case 3: // PDF/A-3
    default: // PDF 1.x
        mxRbPDFAVersion->set_active_id(u"3"_ustr);
        break;
    }

    const bool bIsPDFUA = pParent->mbPDFUACompliance;
    mxCbPDFUA->set_active(bIsPDFUA);
    mxCbPDFUA->connect_toggled(LINK(this, ImpPDFTabGeneralPage, TogglePDFVersionOrUniversalAccessibilityHandle));
    mxCbPDFUA->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"PDFUACompliance"_ustr));

    // the TogglePDFVersionOrUniversalAccessibilityHandle handler will read or write the *UserSelection based
    // on the mxCbPDFA (= bIsPDFA) state, so we have to prepare the correct input state.
    if (bIsPDFA || bIsPDFUA)
        mxCbTaggedPDF->set_active(pParent->mbUseTaggedPDFUserSelection);
    else
        mbUseTaggedPDFUserSelection = pParent->mbUseTaggedPDFUserSelection;

    mxCbExportBookmarks->set_active(pParent->mbExportBookmarksUserSelection);
    TogglePDFVersionOrUniversalAccessibilityHandle(*mxCbPDFA);

    mxCbExportFormFields->set_active(pParent->mbExportFormFields);
    mxCbExportFormFields->connect_toggled( LINK( this, ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl ) );
    mxCbExportFormFields->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"ExportFormFields"_ustr));

    mxLbFormsFormat->set_active(static_cast<sal_uInt16>(pParent->mnFormsType));
    mxCbAllowDuplicateFieldNames->set_active( pParent->mbAllowDuplicateFieldNames );
    // FormsFrame contains (and thus sets_sensitive) FormsFormat and AllowDuplicateFieldNames
    mxFormsFrame->set_sensitive(pParent->mbExportFormFields);
    if (pParent->mbExportFormFields)
    {
        if (pParent->maConfigItem.IsReadOnly(u"FormsType"_ustr))
            mxLbFormsFormat->set_sensitive(false);
        if (pParent->maConfigItem.IsReadOnly(u"AllowDuplicateFieldNames"_ustr))
            mxCbAllowDuplicateFieldNames->set_sensitive(false);
    }


    mxCbExportNotes->set_active( pParent->mbExportNotes );
    mxCbExportNotesInMargin->set_active(mbIsWriter && pParent->mbExportNotesInMargin);
    mxCbExportNotesInMargin->set_sensitive(
        mbIsWriter && !pParent->maConfigItem.IsReadOnly(u"ExportNotesInMargin"_ustr));

    if (comphelper::LibreOfficeKit::isActive())
    {
        mxCbViewPDF->hide();
        mxCbViewPDF->set_active(false);
    }
    else
    {
       mxCbViewPDF->set_active(pParent->mbViewPDF);
       mxCbViewPDF->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"ViewPDFAfterExport"_ustr));
    }

    if ( mbIsPresentation )
    {
        mxRbPageRange->set_label(mxSlidesFt->get_label());
        mxCbExportNotesPages->show();
        mxCbExportNotesPages->set_active(pParent->mbExportNotesPages);
        mxCbExportNotesPages->connect_toggled( LINK(this, ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl ) );
        mxCbExportNotesPages->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"ExportNotesPages"_ustr));
        mxCbExportOnlyNotesPages->show();
        mxCbExportOnlyNotesPages->set_active(pParent->mbExportOnlyNotesPages);
        // tdf#116473 Initially enable Export only note pages option depending on the checked state of Export notes pages option
        mxCbExportOnlyNotesPages->set_sensitive(
            mxCbExportNotesPages->get_active() && !pParent->maConfigItem.IsReadOnly(u"ExportOnlyNotesPages"_ustr));
        mxCbExportHiddenSlides->show();
        mxCbExportHiddenSlides->set_active(pParent->mbExportHiddenSlides);
        mxCbExportHiddenSlides->set_sensitive(
            !pParent->maConfigItem.IsReadOnly(u"ExportHiddenSlides"_ustr));
    }
    else
    {
        mxCbExportNotesPages->hide();
        mxCbExportNotesPages->set_active(false);
        mxCbExportOnlyNotesPages->hide();
        mxCbExportOnlyNotesPages->set_active(false);
        mxCbExportHiddenSlides->hide();
        mxCbExportHiddenSlides->set_active(false);
    }

    if( mbIsSpreadsheet )
    {
        mxRbSelection->set_label(mxSheetsSelectionFt->get_label());
        // tdf#105965 Make Selection/Selected sheets the default PDF export range setting for spreadsheets
        mxRbSelection->set_active(true);

        mxRbSheetRange->show();
        mxEdSheets->show();

        mxCbSinglePageSheets->show();
        mxCbSinglePageSheets->set_active(pParent->mbSinglePageSheets);
        mxCbSinglePageSheets->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"SinglePageSheets"_ustr));
    }
    else
    {
        mxCbSinglePageSheets->hide();
        mxCbSinglePageSheets->set_active(false);
        mxRbSheetRange->hide();
        mxRbSheetRange->set_active(false);
        mxEdSheets->hide();
    }

    mxCbExportEmptyPages->set_active(!pParent->mbIsSkipEmptyPages);
    mxCbExportEmptyPages->set_sensitive(
        mbIsWriter && !pParent->maConfigItem.IsReadOnly(u"IsSkipEmptyPages"_ustr));

    mxCbExportPlaceholders->set_visible(mbIsWriter);
    mxCbExportPlaceholders->set_active(pParent->mbIsExportPlaceholders);
    mxCbExportPlaceholders->set_sensitive(
        mbIsWriter && !pParent->maConfigItem.IsReadOnly(u"ExportPlaceholders"_ustr));

    mxCbAddStream->show();
    mxCbAddStream->set_active(pParent->mbAddStream);
    mxCbAddStream->set_sensitive(!pParent->maConfigItem.IsReadOnly(u"IsAddStream"_ustr));

    mxCbAddStream->connect_toggled(LINK(this, ImpPDFTabGeneralPage, ToggleAddStreamHdl));
    ToggleAddStreamHdl(*mxCbAddStream); // init addstream dependencies
}

void ImpPDFTabGeneralPage::GetFilterConfigItem( ImpPDFTabDialog* pParent )
{
    // updating the FilterData sequence and storing FilterData to configuration
    pParent->mbUseLosslessCompression = mxRbLosslessCompression->get_active();
    pParent->mnQuality = static_cast<sal_Int32>(mxNfQuality->get_value(FieldUnit::PERCENT));
    pParent->mbReduceImageResolution = mxCbReduceImageResolution->get_active();
    pParent->mnMaxImageResolution = mxCoReduceImageResolution->get_active_text().toInt32();
    pParent->mbExportNotes = mxCbExportNotes->get_active();
    if (mbIsWriter)
        pParent->mbExportNotesInMargin = mxCbExportNotesInMargin->get_active();
    pParent->mbViewPDF = mxCbViewPDF->get_active();
    pParent->mbUseReferenceXObject = mxCbUseReferenceXObject->get_active();
    if ( mbIsPresentation )
    {
        pParent->mbExportNotesPages = mxCbExportNotesPages->get_active();
        pParent->mbExportOnlyNotesPages = mxCbExportOnlyNotesPages->get_active();
    }
    pParent->mbExportBookmarks = mxCbExportBookmarks->get_active();
    if ( mbIsPresentation )
        pParent->mbExportHiddenSlides = mxCbExportHiddenSlides->get_active();

    if (mbIsSpreadsheet)
        pParent->mbSinglePageSheets = mxCbSinglePageSheets->get_active();

    pParent->mbIsSkipEmptyPages = !mxCbExportEmptyPages->get_active();
    pParent->mbIsExportPlaceholders = mxCbExportPlaceholders->get_active();
    pParent->mbAddStream = mxCbAddStream->get_visible() && mxCbAddStream->get_active();

    pParent->mbIsPageRangeChecked = false;
    if( mxRbPageRange->get_active() )
    {
        pParent->mbIsPageRangeChecked = true;
        pParent->msPageRange = mxEdPages->get_text(); //FIXME all right on other languages ?
    }
    else if ( mxRbSheetRange->get_active() )
    {
        pParent->mbIsSheetRangeChecked = true;
        pParent->msSheetRange = mxEdSheets->get_text();
    }
    else if( mxRbSelection->get_active() )
    {
        pParent->mbSelectionIsChecked = mxRbSelection->get_active();
    }

    pParent->mnPDFTypeSelection = 0;
    pParent->mbUseTaggedPDF = mxCbTaggedPDF->get_active();

    const bool bIsPDFA = mxCbPDFA->get_active();
    const bool bIsPDFUA = mxCbPDFUA->get_active();

    if (bIsPDFA)
    {
        pParent->mnPDFTypeSelection = 3;
        OUString currentPDFAMode = mxRbPDFAVersion->get_active_id();
        if( currentPDFAMode == "1" )
            pParent->mnPDFTypeSelection = 1;
        else if(currentPDFAMode == "2")
            pParent->mnPDFTypeSelection = 2;
    }

    pParent->mbPDFUACompliance = bIsPDFUA;

    if (!bIsPDFA && !bIsPDFUA)
        mbUseTaggedPDFUserSelection = pParent->mbUseTaggedPDF;
    if (!bIsPDFUA)
    {
        pParent->mbExportBookmarksUserSelection = pParent->mbExportBookmarks;
        pParent->mbUseReferenceXObjectUserSelection = pParent->mbUseReferenceXObject;
    }

    pParent->mbUseTaggedPDFUserSelection = mbUseTaggedPDFUserSelection;
    pParent->mbExportFormFields = mxCbExportFormFields->get_active();

    if( mxCbWatermark->get_active() )
        pParent->maWatermarkText = mxEdWatermark->get_text();

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    pParent->mnFormsType = mxLbFormsFormat->get_active();
    pParent->mbAllowDuplicateFieldNames = mxCbAllowDuplicateFieldNames->get_active();
}

std::unique_ptr<SfxTabPage> ImpPDFTabGeneralPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                 const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabGeneralPage>(pPage, pController, *rAttrSet);
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAllHdl, weld::Toggleable&, void)
{
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, TogglePagesHdl, weld::Toggleable&, void)
{
    TogglePagesHdl();
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleSheetsHdl, weld::Toggleable&, void)
{
    ToggleSheetsHdl();
    EnableExportNotesPages();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleSelectionHdl, weld::Toggleable&, void)
{
    EnableExportNotesPages();
}

void ImpPDFTabGeneralPage::TogglePagesHdl()
{
    mxEdPages->set_sensitive( mxRbPageRange->get_active() );
    if (mxRbPageRange->get_active())
        mxEdPages->grab_focus();
}

void ImpPDFTabGeneralPage::ToggleSheetsHdl()
{
    mxEdSheets->set_sensitive( mxRbSheetRange->get_active() );
    if (mxRbSheetRange->get_active())
        mxEdSheets->grab_focus();
}

void ImpPDFTabGeneralPage::EnableExportNotesPages()
{
    if ( mbIsPresentation )
    {
        mxCbExportNotesPages->set_sensitive(
            !mxRbSelection->get_active() && !IsReadOnlyProperty(u"ExportNotesPages"_ustr));
        mxCbExportOnlyNotesPages->set_sensitive(
            !mxRbSelection->get_active() && mxCbExportNotesPages->get_active()
            && !IsReadOnlyProperty(u"ExportOnlyNotesPages"_ustr));
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl, weld::Toggleable&, void)
{
    const bool bExportFormFields = mxCbExportFormFields->get_active();
    // FormsFrame contains (and thus sets_sensitive) FormsFormat and AllowDuplicateFieldNames
    mxFormsFrame->set_sensitive(bExportFormFields);
    if (bExportFormFields)
    {
        if (IsReadOnlyProperty(u"FormsType"_ustr))
            mxLbFormsFormat->set_sensitive(false);
        if (IsReadOnlyProperty(u"AllowDuplicateFieldNames"_ustr))
            mxCbAllowDuplicateFieldNames->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportNotesPagesHdl, weld::Toggleable&, void)
{
    mxCbExportOnlyNotesPages->set_sensitive(
        mxCbExportNotesPages->get_active()
        && !IsReadOnlyProperty(u"ExportOnlyNotesPages"_ustr));
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleCompressionHdl, weld::Toggleable&, void)
{
    mxQualityFrame->set_sensitive(
        mxRbJPEGCompression->get_active() && !IsReadOnlyProperty(u"Quality"_ustr));
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl, weld::Toggleable&, void)
{
    mxCoReduceImageResolution->set_sensitive(
        mxCbReduceImageResolution->get_active()
        && !IsReadOnlyProperty(u"MaxImageResolution"_ustr));
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleWatermarkHdl, weld::Toggleable&, void)
{
    mxEdWatermark->set_sensitive(mxCbWatermark->get_active());
    mxFtWatermark->set_sensitive(mxCbWatermark->get_active());
    if (mxCbWatermark->get_active())
        mxEdWatermark->grab_focus();
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAddStreamHdl, weld::Toggleable&, void)
{
    if (!mxCbAddStream->get_visible())
        return;

    if( mxCbAddStream->get_active() )
    {
        mxRbAll->set_active(true);
        mxRbPageRange->set_sensitive( false );
        mxRbSheetRange->set_sensitive( false );
        mxRbSelection->set_sensitive( false );
        mxEdPages->set_sensitive( false );
        mxRbAll->set_sensitive( false );
    }
    else
    {
        mxRbAll->set_sensitive(true);
        mxRbPageRange->set_sensitive(true);
        mxRbSheetRange->set_sensitive(true);
        mxRbSelection->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(ImpPDFTabGeneralPage, TogglePDFVersionOrUniversalAccessibilityHandle, weld::Toggleable&, void)
{
    const bool bIsPDFA = mxCbPDFA->get_active();
    const bool bIsPDFUA = mxCbPDFUA->get_active();

    // set the security page status (and its controls as well)
    ImpPDFTabSecurityPage* pSecPage = mpParent ? mpParent->getSecurityPage() : nullptr;
    if (pSecPage)
        pSecPage->ImplPDFASecurityControl();

    mxCbTaggedPDF->set_sensitive(
        !bIsPDFA && !bIsPDFUA && !IsReadOnlyProperty(u"UseTaggedPDF"_ustr));
    mxRbPDFAVersion->set_sensitive(
        bIsPDFA && !IsReadOnlyProperty(u"SelectPdfVersion"_ustr));

    if (bIsPDFA || bIsPDFUA)
    {
        // store the users selection of subordinate controls and set required PDF/A values
        mbUseTaggedPDFUserSelection = mxCbTaggedPDF->get_active();
        mxCbTaggedPDF->set_active(true);

        // if a password was set, inform the user that this will not be used
        if (bIsPDFA && pSecPage && pSecPage->hasPassword())
        {
            mxPasswordUnusedWarnDialog =
                std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(m_xContainer.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      FilterResId(STR_WARN_PASSWORD_PDFA)));
            mxPasswordUnusedWarnDialog->runAsync(mxPasswordUnusedWarnDialog, [] (sal_uInt32){ });
        }
    }
    else
    {
        // restore the users values of subordinate controls
        mxCbTaggedPDF->set_active(mbUseTaggedPDFUserSelection);
    }

    if (bIsPDFUA)
    {
        if (mxCbExportBookmarks->get_sensitive())
        {
            if (mpParent)
            {
                mpParent->mbExportBookmarksUserSelection = mxCbExportBookmarks->get_active();
            }
            mxCbExportBookmarks->set_active(true);
        }
        if (mxCbUseReferenceXObject->get_sensitive())
        {
            if (mpParent)
            {
                mpParent->mbUseReferenceXObjectUserSelection = mxCbUseReferenceXObject->get_active();
            }
            mxCbUseReferenceXObject->set_active(false);
        }
    }
    else if (mpParent)
    {
        mxCbExportBookmarks->set_active(mpParent->mbExportBookmarksUserSelection);
        mxCbUseReferenceXObject->set_active(mpParent->mbUseReferenceXObjectUserSelection);
    }
    mxCbExportBookmarks->set_sensitive(
        !bIsPDFUA && !IsReadOnlyProperty(u"ExportBookmarks"_ustr));
    mxCbUseReferenceXObject->set_sensitive(!bIsPDFUA);

    ImpPDFTabOpnFtrPage *const pOpenPage(mpParent ? mpParent->getOpenPage() : nullptr);
    if (pOpenPage)
    {
        pOpenPage->ToggleInitialView(*mpParent);
    }

    // PDF/A doesn't allow launch action, so enable/disable the selection on the Link page
    ImpPDFTabLinksPage* pLinksPage = mpParent ? mpParent->getLinksPage() : nullptr;
    if (pLinksPage)
        pLinksPage->ImplPDFALinkControl(!bIsPDFA);
}

/// The option features tab page
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"filter/ui/pdfviewpage.ui"_ustr, u"PdfViewPage"_ustr, &rCoreSet)
    , mbUseCTLFont(false)
    , mxRbOpnPageOnly(m_xBuilder->weld_radio_button(u"pageonly"_ustr))
    , mxRbOpnOutline(m_xBuilder->weld_radio_button(u"outline"_ustr))
    , mxRbOpnThumbs(m_xBuilder->weld_radio_button(u"thumbs"_ustr))
    , mxNumInitialPage(m_xBuilder->weld_spin_button(u"page"_ustr))
    , mxRbMagnDefault(m_xBuilder->weld_radio_button(u"fitdefault"_ustr))
    , mxRbMagnFitWin(m_xBuilder->weld_radio_button(u"fitwin"_ustr))
    , mxRbMagnFitWidth(m_xBuilder->weld_radio_button(u"fitwidth"_ustr))
    , mxRbMagnFitVisible(m_xBuilder->weld_radio_button(u"fitvis"_ustr))
    , mxRbMagnZoom(m_xBuilder->weld_radio_button(u"fitzoom"_ustr))
    , mxNumZoom(m_xBuilder->weld_spin_button(u"zoom"_ustr))
    , mxRbPgLyDefault(m_xBuilder->weld_radio_button(u"defaultlayout"_ustr))
    , mxRbPgLySinglePage(m_xBuilder->weld_radio_button(u"singlelayout"_ustr))
    , mxRbPgLyContinue(m_xBuilder->weld_radio_button(u"contlayout"_ustr))
    , mxRbPgLyContinueFacing(m_xBuilder->weld_radio_button(u"contfacinglayout"_ustr))
    , mxCbPgLyFirstOnLeft(m_xBuilder->weld_check_button(u"firstonleft"_ustr))
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

std::unique_ptr<SfxTabPage> ImpPDFTabOpnFtrPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabOpnFtrPage>(pPage, pController, *rAttrSet);
}

void ImpPDFTabOpnFtrPage::GetFilterConfigItem( ImpPDFTabDialog* pParent  )
{
    pParent->mnInitialView = 0;
    if( mxRbOpnOutline->get_active() )
        pParent->mnInitialView = 1;
    else if( mxRbOpnThumbs->get_active() )
        pParent->mnInitialView = 2;
    if (!pParent->mbPDFUACompliance)
    {
        pParent->mnInitialViewUserSelection = pParent->mnInitialView;
    }

    pParent->mnMagnification = 0;
    if( mxRbMagnFitWin->get_active() )
        pParent->mnMagnification = 1;
    else if( mxRbMagnFitWidth->get_active() )
        pParent->mnMagnification = 2;
    else if( mxRbMagnFitVisible->get_active() )
        pParent->mnMagnification = 3;
    else if( mxRbMagnZoom->get_active() )
    {
        pParent->mnMagnification = 4;
        pParent->mnZoom = mxNumZoom->get_value();
    }

    pParent->mnInitialPage = mxNumInitialPage->get_value();

    pParent->mnPageLayout = 0;
    if( mxRbPgLySinglePage->get_active() )
        pParent->mnPageLayout = 1;
    else if( mxRbPgLyContinue->get_active() )
        pParent->mnPageLayout = 2;
    else if( mxRbPgLyContinueFacing->get_active() )
        pParent->mnPageLayout = 3;

    pParent->mbFirstPageLeft = mbUseCTLFont && mxCbPgLyFirstOnLeft->get_active();
}

void ImpPDFTabOpnFtrPage::SetFilterConfigItem(ImpPDFTabDialog *const pParent)
{
    mbUseCTLFont = pParent->mbUseCTLFont;
    switch( pParent->mnPageLayout )
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

    switch( pParent->mnInitialView )
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

    switch( pParent->mnMagnification )
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

    mxNumZoom->set_value(pParent->mnZoom);
    mxNumInitialPage->set_value(pParent->mnInitialPage);

    if (!mbUseCTLFont)
        mxCbPgLyFirstOnLeft->hide();
    else
    {
        mxRbPgLyContinueFacing->connect_toggled(LINK(this, ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl));
        mxCbPgLyFirstOnLeft->set_active(pParent->mbFirstPageLeft);
        ToggleRbPgLyContinueFacingHdl();
    }

    // The call from ImpPDFTabGeneralPage::SetFilterConfigItem() did not init
    // the radio buttons correctly because ImpPDFTabOpnFtrPage did not yet exist.
    ToggleInitialView(*pParent);
}

void ImpPDFTabOpnFtrPage::ToggleInitialView(ImpPDFTabDialog & rParent)
{
    bool const bIsPDFUA(rParent.getGeneralPage()->IsPdfUaSelected());
    if (bIsPDFUA)
    {   // only allow Outline for PDF/UA
        if (mxRbOpnOutline->get_sensitive())
        {
            if (mxRbOpnPageOnly->get_active())
            {
                rParent.mnInitialViewUserSelection = 0;
            }
            else if (mxRbOpnOutline->get_active())
            {
                rParent.mnInitialViewUserSelection = 1;
            }
            else if (mxRbOpnThumbs->get_active())
            {
                rParent.mnInitialViewUserSelection = 2;
            }
            mxRbOpnOutline->set_active(true);
        }
    }
    else
    {
        switch (rParent.mnInitialViewUserSelection)
        {
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
    }
    mxRbOpnPageOnly->set_sensitive(!bIsPDFUA);
    mxRbOpnThumbs->set_sensitive(!bIsPDFUA);
    mxRbOpnOutline->set_sensitive(!bIsPDFUA);
}

IMPL_LINK_NOARG(ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl, weld::Toggleable&, void)
{
    ToggleRbPgLyContinueFacingHdl();
}

void ImpPDFTabOpnFtrPage::ToggleRbPgLyContinueFacingHdl()
{
    mxCbPgLyFirstOnLeft->set_sensitive(mxRbPgLyContinueFacing->get_active());
}

IMPL_LINK_NOARG( ImpPDFTabOpnFtrPage, ToggleRbMagnHdl, weld::Toggleable&, void )
{
    mxNumZoom->set_sensitive(mxRbMagnZoom->get_active());
}

/// The Viewer preferences tab page
ImpPDFTabViewerPage::ImpPDFTabViewerPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : SfxTabPage(pPage, pController, u"filter/ui/pdfuserinterfacepage.ui"_ustr, u"PdfUserInterfacePage"_ustr, &rCoreSet)
    , mbIsPresentation(false)
    , m_xCbResWinInit(m_xBuilder->weld_check_button(u"resize"_ustr))
    , m_xCbCenterWindow(m_xBuilder->weld_check_button(u"center"_ustr))
    , m_xCbOpenFullScreen(m_xBuilder->weld_check_button(u"open"_ustr))
    , m_xCbDispDocTitle(m_xBuilder->weld_check_button(u"display"_ustr))
    , m_xCbHideViewerMenubar(m_xBuilder->weld_check_button(u"menubar"_ustr))
    , m_xCbHideViewerToolbar(m_xBuilder->weld_check_button(u"toolbar"_ustr))
    , m_xCbHideViewerWindowControls(m_xBuilder->weld_check_button(u"window"_ustr))
    , m_xCbTransitionEffects(m_xBuilder->weld_check_button(u"effects"_ustr))
    , m_xRbAllBookmarkLevels(m_xBuilder->weld_radio_button(u"allbookmarks"_ustr))
    , m_xRbVisibleBookmarkLevels(m_xBuilder->weld_radio_button(u"visiblebookmark"_ustr))
    , m_xNumBookmarkLevels(m_xBuilder->weld_spin_button(u"visiblelevel"_ustr))
{
    m_xRbAllBookmarkLevels->connect_toggled(LINK(this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl));
    m_xRbVisibleBookmarkLevels->connect_toggled(LINK(this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl));
}

ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
}

IMPL_LINK_NOARG( ImpPDFTabViewerPage, ToggleRbBookmarksHdl, weld::Toggleable&, void )
{
    m_xNumBookmarkLevels->set_sensitive(m_xRbVisibleBookmarkLevels->get_active());
}

std::unique_ptr<SfxTabPage> ImpPDFTabViewerPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabViewerPage>(pPage, pController, *rAttrSet);
}

void ImpPDFTabViewerPage::GetFilterConfigItem( ImpPDFTabDialog* pParent  )
{
    pParent->mbHideViewerMenubar = m_xCbHideViewerMenubar->get_active();
    pParent->mbHideViewerToolbar = m_xCbHideViewerToolbar->get_active();
    pParent->mbHideViewerWindowControls = m_xCbHideViewerWindowControls->get_active();
    pParent->mbResizeWinToInit = m_xCbResWinInit->get_active();
    pParent->mbOpenInFullScreenMode = m_xCbOpenFullScreen->get_active();
    pParent->mbCenterWindow = m_xCbCenterWindow->get_active();
    pParent->mbDisplayPDFDocumentTitle = m_xCbDispDocTitle->get_active();
    pParent->mbUseTransitionEffects = m_xCbTransitionEffects->get_active();
    pParent->mnOpenBookmarkLevels = m_xRbAllBookmarkLevels->get_active() ?
                                     -1 : static_cast<sal_Int32>(m_xNumBookmarkLevels->get_value());
}

void ImpPDFTabViewerPage::SetFilterConfigItem( const  ImpPDFTabDialog* pParent )
{
    m_xCbHideViewerMenubar->set_active( pParent->mbHideViewerMenubar );
    m_xCbHideViewerToolbar->set_active( pParent->mbHideViewerToolbar );
    m_xCbHideViewerWindowControls->set_active( pParent->mbHideViewerWindowControls );

    m_xCbResWinInit->set_active( pParent->mbResizeWinToInit );
    m_xCbOpenFullScreen->set_active( pParent->mbOpenInFullScreenMode );
    m_xCbCenterWindow->set_active( pParent->mbCenterWindow );
    m_xCbDispDocTitle->set_active( pParent->mbDisplayPDFDocumentTitle );
    mbIsPresentation = pParent->mbIsPresentation;
    m_xCbTransitionEffects->set_active( pParent->mbUseTransitionEffects );
    m_xCbTransitionEffects->set_sensitive( mbIsPresentation );
    if( pParent->mnOpenBookmarkLevels < 0 )
    {
        m_xRbAllBookmarkLevels->set_active(true);
        m_xNumBookmarkLevels->set_sensitive( false );
    }
    else
    {
        m_xRbVisibleBookmarkLevels->set_active(true);
        m_xNumBookmarkLevels->set_sensitive(true);
        m_xNumBookmarkLevels->set_value(pParent->mnOpenBookmarkLevels);
    }
}

/// The Security preferences tab page
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& i_rCoreSet)
    : SfxTabPage(pPage, pController, u"filter/ui/pdfsecuritypage.ui"_ustr, u"PdfSecurityPage"_ustr, &i_rCoreSet)
    , msUserPwdTitle( FilterResId( STR_PDF_EXPORT_UDPWD ) )
    , mbHaveOwnerPassword( false )
    , mbHaveUserPassword( false )
    , msOwnerPwdTitle( FilterResId( STR_PDF_EXPORT_ODPWD ) )
    , mxPbSetPwd(m_xBuilder->weld_button(u"setpassword"_ustr))
    , mxUserPwdSet(m_xBuilder->weld_widget(u"userpwdset"_ustr))
    , mxUserPwdUnset(m_xBuilder->weld_widget(u"userpwdunset"_ustr))
    , mxUserPwdPdfa(m_xBuilder->weld_widget(u"userpwdpdfa"_ustr))
    , mxOwnerPwdSet(m_xBuilder->weld_widget(u"ownerpwdset"_ustr))
    , mxOwnerPwdUnset(m_xBuilder->weld_widget(u"ownerpwdunset"_ustr))
    , mxOwnerPwdPdfa(m_xBuilder->weld_widget(u"ownerpwdpdfa"_ustr))
    , mxPrintPermissions(m_xBuilder->weld_widget(u"printing"_ustr))
    , mxRbPrintNone(m_xBuilder->weld_radio_button(u"printnone"_ustr))
    , mxRbPrintLowRes(m_xBuilder->weld_radio_button(u"printlow"_ustr))
    , mxRbPrintHighRes(m_xBuilder->weld_radio_button(u"printhigh"_ustr))
    , mxChangesAllowed(m_xBuilder->weld_widget(u"changes"_ustr))
    , mxRbChangesNone(m_xBuilder->weld_radio_button(u"changenone"_ustr))
    , mxRbChangesInsDel(m_xBuilder->weld_radio_button(u"changeinsdel"_ustr))
    , mxRbChangesFillForm(m_xBuilder->weld_radio_button(u"changeform"_ustr))
    , mxRbChangesComment(m_xBuilder->weld_radio_button(u"changecomment"_ustr))
    , mxRbChangesAnyNoCopy(m_xBuilder->weld_radio_button(u"changeany"_ustr))
    , mxContent(m_xBuilder->weld_widget(u"content"_ustr))
    , mxCbEnableCopy(m_xBuilder->weld_check_button(u"enablecopy"_ustr))
    , mxCbEnableAccessibility(m_xBuilder->weld_check_button(u"enablea11y"_ustr))
    , mxPasswordTitle(m_xBuilder->weld_label(u"setpasswordstitle"_ustr))
    , mxPermissionTitle(m_xBuilder->weld_label(u"label2"_ustr))
{
    msStrSetPwd = mxPasswordTitle->get_label();
    mxPbSetPwd->connect_clicked(LINK(this, ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl));
}

ImpPDFTabSecurityPage::~ImpPDFTabSecurityPage()
{
    if (mpPasswordDialog)
        mpPasswordDialog->response(RET_CANCEL);
    if (mpUnsupportedMsgDialog)
        mpUnsupportedMsgDialog->response(RET_CANCEL);
}

std::unique_ptr<SfxTabPage> ImpPDFTabSecurityPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabSecurityPage>(pPage, pController, *rAttrSet);
}

void ImpPDFTabSecurityPage::GetFilterConfigItem( ImpPDFTabDialog* pParent  )
{
    // please note that in PDF/A-1a mode even if this are copied back,
    // the security settings are forced disabled in PDFExport::Export
    pParent->mbEncrypt = mbHaveUserPassword;
    pParent->mxPreparedPasswords = mxPreparedPasswords;

    pParent->mbRestrictPermissions = mbHaveOwnerPassword;
    pParent->maPreparedOwnerPassword = maPreparedOwnerPassword;

    // verify print status
    pParent->mnPrint = 0;
    if (mxRbPrintLowRes->get_active())
        pParent->mnPrint = 1;
    else if (mxRbPrintHighRes->get_active())
        pParent->mnPrint = 2;

    // verify changes permitted
    pParent->mnChangesAllowed = 0;

    if( mxRbChangesInsDel->get_active() )
        pParent->mnChangesAllowed = 1;
    else if( mxRbChangesFillForm->get_active() )
        pParent->mnChangesAllowed = 2;
    else if( mxRbChangesComment->get_active() )
        pParent->mnChangesAllowed = 3;
    else if( mxRbChangesAnyNoCopy->get_active() )
        pParent->mnChangesAllowed = 4;

    pParent->mbCanCopyOrExtract = mxCbEnableCopy->get_active();
    pParent->mbCanExtractForAccessibility = mxCbEnableAccessibility->get_active();
}

void ImpPDFTabSecurityPage::SetFilterConfigItem( const  ImpPDFTabDialog* pParent )
{
    switch( pParent->mnPrint )
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

    switch( pParent->mnChangesAllowed )
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

    mxCbEnableCopy->set_active(pParent->mbCanCopyOrExtract);
    mxCbEnableAccessibility->set_active(pParent->mbCanExtractForAccessibility);

    // set the status of this windows, according to the PDFA selection
    enablePermissionControls();

    ImpPDFTabGeneralPage* pGeneralPage = pParent->getGeneralPage();

    if (pGeneralPage)
        ImplPDFASecurityControl();
}

IMPL_LINK_NOARG(ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl, weld::Button&, void)
{
    if(mpPasswordDialog)
        mpPasswordDialog->response(RET_CANCEL);

    mpPasswordDialog = std::make_shared<SfxPasswordDialog>(m_xContainer.get(), &msUserPwdTitle);

    mpPasswordDialog->SetMinLen(0);
    mpPasswordDialog->ShowMinLengthText(false);
    mpPasswordDialog->ShowExtras( SfxShowExtras::CONFIRM | SfxShowExtras::PASSWORD2 | SfxShowExtras::CONFIRM2 );
    mpPasswordDialog->set_title(msStrSetPwd);
    mpPasswordDialog->SetGroup2Text(msOwnerPwdTitle);
    mpPasswordDialog->AllowAsciiOnly();

    mpPasswordDialog->PreRun();

    weld::DialogController::runAsync(mpPasswordDialog, [this](sal_Int32 response){
        if (response == RET_OK)
        {
            OUString aUserPW(mpPasswordDialog->GetPassword());
            OUString aOwnerPW(mpPasswordDialog->GetPassword2());

            mbHaveUserPassword = !aUserPW.isEmpty();
            mbHaveOwnerPassword = !aOwnerPW.isEmpty();

            mxPreparedPasswords = vcl::PDFWriter::InitEncryption( aOwnerPW, aUserPW );
            if (!mxPreparedPasswords.is())
            {
                OUString msg;
                ErrorHandler::GetErrorString(ERRCODE_IO_NOTSUPPORTED, msg); //TODO: handle failure
                mpUnsupportedMsgDialog = std::shared_ptr<weld::MessageDialog>(
                Application::CreateMessageDialog(
                    GetFrameWeld(), VclMessageType::Error, VclButtonsType::Ok, msg));

                mpUnsupportedMsgDialog->runAsync(mpUnsupportedMsgDialog, [](sal_Int32){ });
                return;
            }

            if( mbHaveOwnerPassword )
                maPreparedOwnerPassword = comphelper::OStorageHelper::CreatePackageEncryptionData( aOwnerPW );
            else
                maPreparedOwnerPassword = Sequence< NamedValue >();
        }
        if (response != RET_CANCEL)
            enablePermissionControls();
        mpPasswordDialog.reset();
    });
}

void ImpPDFTabSecurityPage::enablePermissionControls()
{
    bool bIsPDFASel = false;
    bool bIsPDFUASel = false;
    ImpPDFTabDialog* pParent = static_cast<ImpPDFTabDialog*>(GetDialogController());
    ImpPDFTabGeneralPage* pGeneralPage = pParent ? pParent->getGeneralPage() : nullptr;
    if (pGeneralPage)
    {
        bIsPDFASel = pGeneralPage->IsPdfaSelected();
        bIsPDFUASel = pGeneralPage->IsPdfUaSelected();
    }
    // ISO 14289-1:2014, Clause: 7.16
    if (bIsPDFUASel)
    {
        mxCbEnableAccessibility->set_active(true);
    }
    mxPermissionTitle->set_sensitive(!bIsPDFASel);
    mxPbSetPwd->set_sensitive(!bIsPDFASel);
    mxCbEnableAccessibility->set_sensitive(!bIsPDFUASel);
    if (bIsPDFASel)
    {
        mxUserPwdPdfa->show();
        mxUserPwdSet->hide();
        mxUserPwdUnset->hide();
    }
    else
    {
        if (mbHaveUserPassword && m_xContainer->get_sensitive())
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

    bool bLocalEnable = mbHaveOwnerPassword && m_xContainer->get_sensitive();
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
void ImpPDFTabSecurityPage::ImplPDFASecurityControl()
{
    // after enable, check the status of control as if the dialog was initialized
    enablePermissionControls();
}

/// The link preferences tab page (relative and other stuff)
ImpPDFTabLinksPage::ImpPDFTabLinksPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"filter/ui/pdflinkspage.ui"_ustr, u"PdfLinksPage"_ustr, &rCoreSet)
    , mbOpnLnksDefaultUserState(false)
    , mbOpnLnksLaunchUserState(false)
    , mbOpnLnksBrowserUserState(false)
    , m_xCbExprtBmkrToNmDst(m_xBuilder->weld_check_button(u"export"_ustr))
    , m_xCbOOoToPDFTargets(m_xBuilder->weld_check_button(u"convert"_ustr))
    , m_xCbExportRelativeFsysLinks(m_xBuilder->weld_check_button(u"exporturl"_ustr))
    , m_xRbOpnLnksDefault(m_xBuilder->weld_radio_button(u"default"_ustr))
    , m_xRbOpnLnksLaunch(m_xBuilder->weld_radio_button(u"openpdf"_ustr))
    , m_xRbOpnLnksBrowser(m_xBuilder->weld_radio_button(u"openinternet"_ustr))
{
}

ImpPDFTabLinksPage::~ImpPDFTabLinksPage()
{
}

std::unique_ptr<SfxTabPage> ImpPDFTabLinksPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabLinksPage>(pPage, pController, *rAttrSet);
}

void ImpPDFTabLinksPage::GetFilterConfigItem( ImpPDFTabDialog* pParent  )
{
    pParent->mbExportRelativeFsysLinks = m_xCbExportRelativeFsysLinks->get_active();

    bool bIsPDFASel = false;
    ImpPDFTabGeneralPage* pGeneralPage = pParent->getGeneralPage();
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
    pParent->mnViewPDFMode = 0;
    if( mbOpnLnksBrowserUserState )
        pParent->mnViewPDFMode = 2;
    else if( mbOpnLnksLaunchUserState )
        pParent->mnViewPDFMode = 1;

    pParent->mbConvertOOoTargets = m_xCbOOoToPDFTargets->get_active();
    pParent->mbExportBmkToPDFDestination = m_xCbExprtBmkrToNmDst->get_active();
}

void ImpPDFTabLinksPage::SetFilterConfigItem( const  ImpPDFTabDialog* pParent )
{
    m_xCbOOoToPDFTargets->set_active(pParent->mbConvertOOoTargets);
    m_xCbExprtBmkrToNmDst->set_active(pParent->mbExportBmkToPDFDestination);

    m_xRbOpnLnksDefault->connect_toggled(LINK(this, ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl));
    m_xRbOpnLnksBrowser->connect_toggled(LINK(this, ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl));

    m_xCbExportRelativeFsysLinks->set_active(pParent->mbExportRelativeFsysLinks);
    switch( pParent->mnViewPDFMode )
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
    // PDF/A-2 doesn't allow launch action on links

    ImpPDFTabGeneralPage* pGeneralPage = pParent->getGeneralPage();
    if (pGeneralPage)
        ImplPDFALinkControl(!pGeneralPage->mxCbPDFA->get_active());
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
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl, weld::Toggleable&, void)
{
    mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
    mbOpnLnksLaunchUserState = m_xRbOpnLnksLaunch->get_active();
    mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
}

/// Reset the memory of a launch action present when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl, weld::Toggleable&, void)
{
    mbOpnLnksDefaultUserState = m_xRbOpnLnksDefault->get_active();
    mbOpnLnksLaunchUserState = m_xRbOpnLnksLaunch->get_active();
    mbOpnLnksBrowserUserState = m_xRbOpnLnksBrowser->get_active();
}

ImplErrorDialog::ImplErrorDialog(weld::Window* pParent, const std::set<vcl::PDFWriter::ErrorCode>& rErrors)
    : MessageDialogController(pParent, u"filter/ui/warnpdfdialog.ui"_ustr, u"WarnPDFDialog"_ustr, u"grid"_ustr)
    , m_xErrors(m_xBuilder->weld_tree_view(u"errors"_ustr))
    , m_xExplanation(m_xBuilder->weld_label(u"message"_ustr))
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
            m_xErrors->append(FilterResId(STR_WARN_TRANSP_PDFA), FilterResId(STR_WARN_TRANSP_PDFA_SHORT), u"dialog-warning"_ustr);
            break;
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDF13:
            m_xErrors->append(FilterResId(STR_WARN_TRANSP_VERSION), FilterResId(STR_WARN_TRANSP_VERSION_SHORT), u"dialog-warning"_ustr);
            break;
        case vcl::PDFWriter::Warning_FormAction_Omitted_PDFA:
            m_xErrors->append(FilterResId(STR_WARN_FORMACTION_PDFA), FilterResId(STR_WARN_FORMACTION_PDFA_SHORT), u"dialog-warning"_ustr);
            break;
        case vcl::PDFWriter::Warning_Transparency_Converted:
            m_xErrors->append(FilterResId(STR_WARN_TRANSP_CONVERTED), FilterResId(STR_WARN_TRANSP_CONVERTED_SHORT), u"dialog-warning"_ustr);
            break;
        case vcl::PDFWriter::Error_Signature_Failed:
            m_xErrors->append(FilterResId(STR_ERR_PDF_EXPORT_ABORTED), FilterResId(STR_ERR_SIGNATURE_FAILED), u"dialog-error"_ustr);
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
ImpPDFTabSigningPage::ImpPDFTabSigningPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"filter/ui/pdfsignpage.ui"_ustr, u"PdfSignPage"_ustr, &rCoreSet)
    , mxEdSignCert(m_xBuilder->weld_entry(u"cert"_ustr))
    , mxPbSignCertSelect(m_xBuilder->weld_button(u"select"_ustr))
    , mxPbSignCertClear(m_xBuilder->weld_button(u"clear"_ustr))
    , mxEdSignPassword(m_xBuilder->weld_entry(u"password"_ustr))
    , mxEdSignLocation(m_xBuilder->weld_entry(u"location"_ustr))
    , mxEdSignContactInfo(m_xBuilder->weld_entry(u"contact"_ustr))
    , mxEdSignReason(m_xBuilder->weld_entry(u"reason"_ustr))
    , mxLBSignTSA(m_xBuilder->weld_combo_box(u"tsa"_ustr))
{
    mxPbSignCertSelect->set_sensitive(true);
    mxPbSignCertSelect->connect_clicked(LINK(this, ImpPDFTabSigningPage, ClickmaPbSignCertSelect));
    mxPbSignCertClear->connect_clicked(LINK(this, ImpPDFTabSigningPage, ClickmaPbSignCertClear));
}

ImpPDFTabSigningPage::~ImpPDFTabSigningPage()
{
}

IMPL_LINK_NOARG(ImpPDFTabSigningPage, ClickmaPbSignCertSelect, weld::Button&, void)
{
    Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createDefault(
            comphelper::getProcessComponentContext()));
    xSigner->setParentWindow(GetFrameWeld()->GetXWindow());

    // The use may provide a description while choosing a certificate.
    OUString aDescription;
    maSignCertificate = xSigner->selectSigningCertificateWithType(
        security::CertificateKind::CertificateKind_X509, aDescription);

    if (!maSignCertificate.is())
        return;

    mxEdSignCert->set_text(maSignCertificate->getSubjectName());
    mxPbSignCertClear->set_sensitive(true);
    mxEdSignLocation->set_sensitive(true);
    mxEdSignPassword->set_sensitive(true);
    mxEdSignContactInfo->set_sensitive(true);
    mxEdSignReason->set_sensitive(true);
    mxEdSignReason->set_text(aDescription);

    try
    {
        std::optional<css::uno::Sequence<OUString>> aTSAURLs(officecfg::Office::Common::Security::Scripting::TSAURLs::get());
        if (aTSAURLs)
        {
            const css::uno::Sequence<OUString>& rTSAURLs = *aTSAURLs;
            for (auto const& elem : rTSAURLs)
            {
                mxLBSignTSA->append_text(elem);
            }
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_INFO_EXCEPTION("filter.pdf", "TSAURLsDialog::TSAURLsDialog()");
    }

    // If more than only the "None" entry is there, enable the ListBox
    if (mxLBSignTSA->get_count() > 1)
        mxLBSignTSA->set_sensitive(true);
}

IMPL_LINK_NOARG(ImpPDFTabSigningPage, ClickmaPbSignCertClear, weld::Button&, void)
{
    mxEdSignCert->set_text(u""_ustr);
    maSignCertificate.clear();
    mxPbSignCertClear->set_sensitive(false);
    mxEdSignLocation->set_sensitive(false);
    mxEdSignPassword->set_sensitive(false);
    mxEdSignContactInfo->set_sensitive(false);
    mxEdSignReason->set_sensitive(false);
    mxLBSignTSA->set_sensitive(false);
}

std::unique_ptr<SfxTabPage> ImpPDFTabSigningPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                 const SfxItemSet* rAttrSet)
{
    return std::make_unique<ImpPDFTabSigningPage>(pPage, pController, *rAttrSet);
}

void ImpPDFTabSigningPage::GetFilterConfigItem( ImpPDFTabDialog* pParent  )
{
    pParent->mbSignPDF = maSignCertificate.is();
    pParent->maSignCertificate = maSignCertificate;
    pParent->msSignLocation = mxEdSignLocation->get_text();
    pParent->msSignPassword = mxEdSignPassword->get_text();
    pParent->msSignContact = mxEdSignContactInfo->get_text();
    pParent->msSignReason = mxEdSignReason->get_text();
    // Entry 0 is 'None'
    if (mxLBSignTSA->get_active() >= 1)
        pParent->msSignTSA = mxLBSignTSA->get_active_text();
}

void ImpPDFTabSigningPage::SetFilterConfigItem( const  ImpPDFTabDialog* pParent )
{
    mxEdSignLocation->set_sensitive(false);
    mxEdSignPassword->set_sensitive(false);
    mxEdSignContactInfo->set_sensitive(false);
    mxEdSignReason->set_sensitive(false);
    mxLBSignTSA->set_sensitive(false);
    mxPbSignCertClear->set_sensitive(false);

    if (pParent->mbSignPDF)
    {
        mxEdSignPassword->set_text(pParent->msSignPassword);
        mxEdSignLocation->set_text(pParent->msSignLocation);
        mxEdSignContactInfo->set_text(pParent->msSignContact);
        mxEdSignReason->set_text(pParent->msSignReason);
        maSignCertificate = pParent->maSignCertificate;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
