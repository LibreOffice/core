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
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "sfx2/passwd.hxx"
#include "svtools/miscopt.hxx"

#include "comphelper/storagehelper.hxx"

#include "com/sun/star/text/XTextRange.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"
#include "com/sun/star/security/DocumentDigitalSignatures.hpp"
#include "com/sun/star/security/XCertificate.hpp"

#include <boost/shared_ptr.hpp>

static ResMgr& getPDFFilterResMgr()
{
    static ResMgr *pRes = ResMgr::CreateResMgr( "pdffilter", Application::GetSettings().GetUILanguageTag());
    return *pRes;
}

PDFFilterResId::PDFFilterResId( sal_uInt32 nId ) : ResId( nId, getPDFFilterResMgr() )
{
}

// ----------------
// - ImpPDFDialog -
// ----------------

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// tabbed PDF dialog implementation
// please note: the default used here are the same as per specification,
// they should be the same in  PDFFilter::implExport and  in PDFExport::PDFExport
// -----------------------------------------------------------------------------
ImpPDFTabDialog::ImpPDFTabDialog(Window* pParent, Sequence< PropertyValue >& rFilterData,
    const Reference< XComponent >& rxDoc)
    : SfxTabDialog(pParent, "PdfOptionsDialog","filter/ui/pdfoptionsdialog.ui",
        0, false),

    maConfigItem( "Office.Common/Filter/PDF/Export/", &rFilterData ),
    maConfigI18N( "Office.Common/I18N/CTL/" ),
    mnSigningPageId(0),
    mnSecurityPageId(0),
    mnLinksPage(0),
    mnInterfacePageId(0),
    mnViewPageId(0),
    mnGeneralPageId(0),
    mbIsPresentation( sal_False ),
    mbIsWriter( sal_False ),

    mbSelectionPresent( sal_False ),
    mbUseCTLFont( sal_False ),
    mbUseLosslessCompression( sal_True ),
    mnQuality( 90 ),
    mbReduceImageResolution( sal_False ),
    mnMaxImageResolution( 300 ),
    mbUseTaggedPDF( sal_False ),
    mbExportNotes( sal_True ),
    mbViewPDF( sal_False ),
    mbExportNotesPages( sal_False ),
    mbUseTransitionEffects( sal_False ),
    mbIsSkipEmptyPages( sal_True ),
    mbAddStream( sal_False ),
    mnFormsType( 0 ),
    mbExportFormFields( sal_True ),
    mbAllowDuplicateFieldNames( sal_False ),
    mbExportBookmarks( sal_True ),
    mbExportHiddenSlides ( sal_False),
    mnOpenBookmarkLevels( -1 ),

    mbHideViewerToolbar( sal_False ),
    mbHideViewerMenubar( sal_False ),
    mbHideViewerWindowControls( sal_False ),
    mbResizeWinToInit( sal_False ),
    mbCenterWindow( sal_False ),
    mbOpenInFullScreenMode( sal_False ),
    mbDisplayPDFDocumentTitle( sal_False ),
    mnMagnification( 0 ),
    mnInitialView( 0 ),
    mnZoom( 0 ),
    mnInitialPage( 1 ),
    mnPageLayout( 0 ),
    mbFirstPageLeft( sal_False ),

    mbEncrypt( false ),
    mbRestrictPermissions( false ),
    mnPrint( 0 ),
    mnChangesAllowed( 0 ),
    mbCanCopyOrExtract( false ),
    mbCanExtractForAccessibility( true ),

    mbIsRangeChecked( sal_False ),
    msPageRange( ' ' ),

    mbSelectionIsChecked( sal_False ),
    mbExportRelativeFsysLinks( sal_False ),
    mnViewPDFMode( 0 ),
    mbConvertOOoTargets( sal_False ),
    mbExportBmkToPDFDestination( sal_False ),

    mbSignPDF( sal_False )

{
// check for selection
    try
    {
        Reference< frame::XController > xController( Reference< frame::XModel >( rxDoc, UNO_QUERY )->getCurrentController() );
        if( xController.is() )
        {
            Reference< view::XSelectionSupplier > xView( xController, UNO_QUERY );
            if( xView.is() )
                xView->getSelection() >>= maSelection;
        }
    }
    catch(const RuntimeException &)
    {
    }
    mbSelectionPresent = maSelection.hasValue();
    if ( mbSelectionPresent )
    {
        Reference< drawing::XShapes > xShapes;
        if ( ( maSelection >>= xShapes ) == sal_False ) // XShapes is always a selection
        {
            // even if nothing is selected in writer the selection is not empty
            Reference< container::XIndexAccess > xIndexAccess;
            if ( maSelection >>= xIndexAccess )
            {
                sal_Int32 nLen = xIndexAccess->getCount();
                if ( !nLen )
                    mbSelectionPresent = sal_False;
                else if ( nLen == 1 )
                {
                    Reference< text::XTextRange > xTextRange( xIndexAccess->getByIndex( 0 ), UNO_QUERY );
                    if ( xTextRange.is() && ( xTextRange->getString().isEmpty() ) )
                        mbSelectionPresent = sal_False;
                }
            }
        }
    }

// check if source document is a presentation
    try
    {
        Reference< XServiceInfo > xInfo( rxDoc, UNO_QUERY );
        if ( xInfo.is() )
        {
            if ( xInfo->supportsService( "com.sun.star.presentation.PresentationDocument" ) )
                mbIsPresentation = sal_True;
            if ( xInfo->supportsService( "com.sun.star.text.GenericTextDocument" ) )
                mbIsWriter = sal_True;
        }
    }
    catch(const RuntimeException &)
    {
    }

//get the CTL (Complex Text Layout) from general options, returns sal_True if we have a CTL font on our hands.
    mbUseCTLFont = maConfigI18N.ReadBool( "CTLFont", sal_False );

    mbUseLosslessCompression = maConfigItem.ReadBool( "UseLosslessCompression", sal_False );
    mnQuality = maConfigItem.ReadInt32( "Quality", 90 );
    mbReduceImageResolution = maConfigItem.ReadBool( "ReduceImageResolution", sal_False );
    mnMaxImageResolution = maConfigItem.ReadInt32( "MaxImageResolution", 300 );

    mbUseTaggedPDF = maConfigItem.ReadBool( "UseTaggedPDF", sal_False );
    mnPDFTypeSelection =  maConfigItem.ReadInt32( "SelectPdfVersion", 0 );
    if ( mbIsPresentation )
        mbExportNotesPages = maConfigItem.ReadBool( "ExportNotesPages", sal_False );
    mbExportNotes = maConfigItem.ReadBool( "ExportNotes", sal_False );
    mbViewPDF = maConfigItem.ReadBool( "ViewPDFAfterExport", sal_False );

    mbExportBookmarks = maConfigItem.ReadBool( "ExportBookmarks", sal_True );
    if ( mbIsPresentation )
        mbExportHiddenSlides = maConfigItem.ReadBool( "ExportHiddenSlides", sal_False );
    mnOpenBookmarkLevels = maConfigItem.ReadInt32( "OpenBookmarkLevels", -1 );
    mbUseTransitionEffects = maConfigItem.ReadBool( "UseTransitionEffects", sal_True );
    mbIsSkipEmptyPages = maConfigItem.ReadBool( "IsSkipEmptyPages", sal_False );
    mbAddStream = maConfigItem.ReadBool( "IsAddStream", sal_False );

    mnFormsType = maConfigItem.ReadInt32( "FormsType", 0 );
    mbExportFormFields = maConfigItem.ReadBool( "ExportFormFields", sal_True );
    if ( ( mnFormsType < 0 ) || ( mnFormsType > 3 ) )
        mnFormsType = 0;
    mbAllowDuplicateFieldNames = maConfigItem.ReadBool( "AllowDuplicateFieldNames", sal_False );

//prepare values for the Viewer tab page
    mbHideViewerToolbar = maConfigItem.ReadBool( "HideViewerToolbar", sal_False );
    mbHideViewerMenubar = maConfigItem.ReadBool( "HideViewerMenubar", sal_False );
    mbHideViewerWindowControls = maConfigItem.ReadBool( "HideViewerWindowControls", sal_False );
    mbResizeWinToInit = maConfigItem.ReadBool( "ResizeWindowToInitialPage", sal_False );
    mbCenterWindow = maConfigItem.ReadBool( "CenterWindow", sal_False );
    mbOpenInFullScreenMode = maConfigItem.ReadBool( "OpenInFullScreenMode", sal_False );
    mbDisplayPDFDocumentTitle = maConfigItem.ReadBool( "DisplayPDFDocumentTitle", sal_True );

    mnInitialView = maConfigItem.ReadInt32( "InitialView", 0 );
    mnMagnification = maConfigItem.ReadInt32( "Magnification", 0 );
    mnZoom = maConfigItem.ReadInt32( "Zoom", 100 );
    mnPageLayout = maConfigItem.ReadInt32( "PageLayout", 0 );
    mbFirstPageLeft = maConfigItem.ReadBool( "FirstPageOnLeft", sal_False );
    mnInitialPage = maConfigItem.ReadInt32( "InitialPage", 1 );
    if( mnInitialPage < 1 )
        mnInitialPage = 1;

//prepare values for the security tab page
    mnPrint = maConfigItem.ReadInt32( "Printing", 2 );
    mnChangesAllowed = maConfigItem.ReadInt32( "Changes", 4 );
    mbCanCopyOrExtract = maConfigItem.ReadBool( "EnableCopyingOfContent", sal_True );
    mbCanExtractForAccessibility = maConfigItem.ReadBool( "EnableTextAccessForAccessibilityTools", sal_True );

//prepare values for relative links
    mbExportRelativeFsysLinks = maConfigItem.ReadBool( "ExportLinksRelativeFsys", sal_False );

    mnViewPDFMode = maConfigItem.ReadInt32( "PDFViewSelection", 0 );

    mbConvertOOoTargets = maConfigItem.ReadBool( "ConvertOOoTargetToPDFTarget", sal_False );
    mbExportBmkToPDFDestination = maConfigItem.ReadBool( "ExportBookmarksToPDFDestination", sal_False );

//prepare values for digital signatures
    mbSignPDF = maConfigItem.ReadBool( "SignPDF", sal_False );

//queue the tab pages for later creation (created when first shown)
    mnSigningPageId = AddTabPage("digitalsignatures", ImpPDFTabSigningPage::Create, 0);
    mnSecurityPageId = AddTabPage("security", ImpPDFTabSecurityPage::Create, 0);
    mnLinksPage = AddTabPage("links", ImpPDFTabLinksPage::Create, 0);
    mnInterfacePageId = AddTabPage("userinterface", ImpPDFTabViewerPage::Create, 0);
    mnViewPageId = AddTabPage("initialview", ImpPDFTabOpnFtrPage::Create, 0);

//remove tabpage if experimentalmode is not set
    SvtMiscOptions aMiscOptions;
    if (!aMiscOptions.IsExperimentalMode())
        RemoveTabPage(mnSigningPageId);

    //last queued is the first to be displayed (or so it seems..)
    mnGeneralPageId = AddTabPage("general", ImpPDFTabGeneralPage::Create, 0 );

    //get the string property value (from sfx2/source/dialog/mailmodel.cxx) to overwrite the text for the Ok button
    OUString sOkButtonText = maConfigItem.ReadString( "_OkButtonString", OUString() );

    //change text on the Ok button: get the relevant string from resources, update it on the button
    //according to the exported pdf file destination: send as e-mail or write to file?
    if (!sOkButtonText.isEmpty())
    {
        GetOKButton().SetText(sOkButtonText);
    }

    GetCancelButton().SetClickHdl(LINK(this, ImpPDFTabDialog, CancelHdl));

//remove the reset button, not needed in this tabbed dialog
    RemoveResetButton();
/////////////////
}

ImpPDFTabSecurityPage* ImpPDFTabDialog::getSecurityPage() const
{
    SfxTabPage* pSecurityPage = GetTabPage(mnSecurityPageId);
    if (pSecurityPage)
    {
        return static_cast<ImpPDFTabSecurityPage*>(pSecurityPage);
    }
    return NULL;
}

ImpPDFTabLinksPage* ImpPDFTabDialog::getLinksPage() const
{
    SfxTabPage* pLinksPage = GetTabPage(mnLinksPage);
    if (pLinksPage)
    {
        return static_cast<ImpPDFTabLinksPage*>(pLinksPage);
    }
    return NULL;
}

ImpPDFTabGeneralPage* ImpPDFTabDialog::getGeneralPage() const
{
    SfxTabPage* pGeneralPage = GetTabPage(mnGeneralPageId);
    if (pGeneralPage)
    {
        return static_cast<ImpPDFTabGeneralPage*>(pGeneralPage);
    }
    return NULL;
}

IMPL_LINK_NOARG(ImpPDFTabDialog, CancelHdl)
{
    EndDialog( sal_False );
    return 0;
}

// -----------------------------------------------------------------------------
ImpPDFTabDialog::~ImpPDFTabDialog()
{
//delete the pages, needed because otherwise the child tab pages
//don't get destroyed
    RemoveTabPage(mnGeneralPageId);
    RemoveTabPage(mnInterfacePageId);
    RemoveTabPage(mnViewPageId);
    RemoveTabPage(mnLinksPage);
    RemoveTabPage(mnSecurityPageId);

//remove tabpage if experimentalmode is set
    SvtMiscOptions aMiscOptions;
    if (aMiscOptions.IsExperimentalMode())
        RemoveTabPage(mnSigningPageId);
}

// -----------------------------------------------------------------------------
void ImpPDFTabDialog::PageCreated( sal_uInt16 _nId,
                                   SfxTabPage& _rPage )
{
    if (_nId == mnGeneralPageId)
    {
        ( ( ImpPDFTabGeneralPage* )&_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnInterfacePageId)
    {
        ( ( ImpPDFTabViewerPage* )&_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnViewPageId)
    {
        ( ( ImpPDFTabOpnFtrPage* )&_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnLinksPage)
    {
        ( ( ImpPDFTabLinksPage* )&_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnSecurityPageId)
    {
        ( ( ImpPDFTabSecurityPage* )&_rPage )->SetFilterConfigItem( this );
    }
    else if (_nId == mnSigningPageId)
    {
        ( ( ImpPDFTabSigningPage* )&_rPage )->SetFilterConfigItem( this );
    }
}

// -----------------------------------------------------------------------------
short ImpPDFTabDialog::Ok( )
{
//here the whole mechanism of the base class is not used
//when Ok is hit, the user means 'convert to PDF', so simply close with ok
    return RET_OK;
}

// -----------------------------------------------------------------------------
Sequence< PropertyValue > ImpPDFTabDialog::GetFilterData()
{
// updating the FilterData sequence and storing FilterData to configuration
    if( GetTabPage(mnGeneralPageId) )
        ( ( ImpPDFTabGeneralPage* )GetTabPage(mnGeneralPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnInterfacePageId) )
        ( ( ImpPDFTabViewerPage* )GetTabPage(mnInterfacePageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnViewPageId) )
        ( ( ImpPDFTabOpnFtrPage* )GetTabPage(mnViewPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnLinksPage) )
        ( ( ImpPDFTabLinksPage* )GetTabPage(mnLinksPage) )->GetFilterConfigItem( this );
    if( GetTabPage(mnSecurityPageId) )
        ( ( ImpPDFTabSecurityPage* )GetTabPage(mnSecurityPageId) )->GetFilterConfigItem( this );
    if( GetTabPage(mnSigningPageId) )
        ( ( ImpPDFTabSigningPage* )GetTabPage(mnSigningPageId) )->GetFilterConfigItem( this );

//prepare the items to be returned
    maConfigItem.WriteBool( "UseLosslessCompression", mbUseLosslessCompression );
    maConfigItem.WriteInt32("Quality", mnQuality );
    maConfigItem.WriteBool( "ReduceImageResolution", mbReduceImageResolution );
    maConfigItem.WriteInt32("MaxImageResolution", mnMaxImageResolution );

    maConfigItem.WriteBool( "UseTaggedPDF", mbUseTaggedPDF );
    maConfigItem.WriteInt32("SelectPdfVersion", mnPDFTypeSelection );

    if ( mbIsPresentation )
        maConfigItem.WriteBool( "ExportNotesPages", mbExportNotesPages );
    maConfigItem.WriteBool( "ExportNotes", mbExportNotes );
    maConfigItem.WriteBool( "ViewPDFAfterExport", mbViewPDF );

    maConfigItem.WriteBool( "ExportBookmarks", mbExportBookmarks );
    if ( mbIsPresentation )
        maConfigItem.WriteBool( "ExportHiddenSlides", mbExportHiddenSlides );
    maConfigItem.WriteBool( "UseTransitionEffects", mbUseTransitionEffects );
    maConfigItem.WriteBool( "IsSkipEmptyPages", mbIsSkipEmptyPages );
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

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    int nElementAdded = 11;

    aRet.realloc( aRet.getLength() + nElementAdded );

    // add the encryption enable flag
    sal_uInt32 const nLength(aRet.getLength());
    aRet[ nLength - nElementAdded ].Name = "Watermark";
    aRet[ nLength - nElementAdded ].Value <<= maWatermarkText;
    nElementAdded--;

// add the encryption enable flag
    aRet[ nLength - nElementAdded ].Name = "EncryptFile";
    aRet[ nLength - nElementAdded ].Value <<= mbEncrypt;
    nElementAdded--;

// add the open password
    aRet[ nLength - nElementAdded ].Name = "PreparedPasswords";
    aRet[ nLength - nElementAdded ].Value <<= mxPreparedPasswords;
    nElementAdded--;

//the restrict permission flag (needed to have the scripting consistent with the dialog)
    aRet[ nLength - nElementAdded ].Name = "RestrictPermissions";
    aRet[ nLength - nElementAdded ].Value <<= mbRestrictPermissions;
    nElementAdded--;

//add the permission password
    aRet[ nLength - nElementAdded ].Name = "PreparedPermissionPassword";
    aRet[ nLength - nElementAdded ].Value <<= maPreparedOwnerPassword;
    nElementAdded--;

// this should be the last added...
    if( mbIsRangeChecked )
    {
        aRet[ nLength - nElementAdded ].Name = "PageRange";
        aRet[ nLength - nElementAdded ].Value <<= msPageRange;
        nElementAdded--;
    }
    else if( mbSelectionIsChecked )
    {
        aRet[ nLength - nElementAdded ].Name = "Selection";
        aRet[ nLength - nElementAdded ].Value <<= maSelection;
        nElementAdded--;
    }

    aRet[ nLength - nElementAdded ].Name = "SignatureLocation";
    aRet[ nLength - nElementAdded ].Value <<= msSignLocation;
    nElementAdded--;

    aRet[ nLength - nElementAdded ].Name = "SignatureReason";
    aRet[ nLength - nElementAdded ].Value <<= msSignReason;
    nElementAdded--;

    aRet[ nLength - nElementAdded ].Name = "SignatureContactInfo";
    aRet[ nLength - nElementAdded ].Value <<= msSignContact;
    nElementAdded--;

    aRet[ nLength - nElementAdded ].Name = "SignaturePassword";
    aRet[ nLength - nElementAdded ].Value <<= msSignPassword;
    nElementAdded--;

    aRet[ nLength - nElementAdded ].Name = "SignatureCertificate";
    aRet[ nLength - nElementAdded ].Value <<= maSignCertificate;
    nElementAdded--;

    return aRet;
}

// -----------------------------------------------------------------------------
ImpPDFTabGeneralPage::ImpPDFTabGeneralPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfGeneralPage","filter/ui/pdfgeneralpage.ui", rCoreSet)
    , mbTaggedPDFUserSelection(false)
    , mbExportFormFieldsUserSelection(false)
    , mbIsPresentation(false)
    , mbIsWriter(false)
    , mpaParent(0)
{
    get(mpRbAll, "all");
    get(mpRbRange, "range");
    get(mpRbSelection, "selection");
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
    get(mpCbExportEmptyPages, "emptypages");
    get(mpCbViewPDF, "viewpdf");

    get(mpCbWatermark, "watermark");
    get(mpFtWatermark, "watermarklabel");
    get(mpEdWatermark, "watermarkentry");
}

// -----------------------------------------------------------------------------
ImpPDFTabGeneralPage::~ImpPDFTabGeneralPage()
{
}

// -----------------------------------------------------------------------------
void ImpPDFTabGeneralPage::SetFilterConfigItem( const ImpPDFTabDialog* paParent )
{
    mpaParent = paParent;

//init this class data
    mpRbRange->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, TogglePagesHdl ) );

    mpRbAll->Check();
    TogglePagesHdl( NULL );

    mpRbSelection->Enable( paParent->mbSelectionPresent );
    mbIsPresentation = paParent->mbIsPresentation;
    mbIsWriter = paParent->mbIsWriter;

    mpCbExportEmptyPages->Enable( mbIsWriter );

    mpRbLosslessCompression->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const sal_Bool bUseLosslessCompression = paParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        mpRbLosslessCompression->Check();
    else
        mpRbJPEGCompression->Check();

    mpNfQuality->SetValue( paParent->mnQuality, FUNIT_PERCENT );
    mpQualityFrame->Enable(bUseLosslessCompression == false);

    mpCbReduceImageResolution->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl ) );
    const sal_Bool  bReduceImageResolution = paParent->mbReduceImageResolution;
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
    case 0: mpCbPDFA1b->Check( sal_False ); // PDF 1.4
        break;
    case 1: mpCbPDFA1b->Check(); // PDF/A-1a
        break;
    }
    ToggleExportPDFAHdl( NULL );

    mpCbExportFormFields->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl ) );

// get the form values, for use with PDF/A-1 selection interface
    mbTaggedPDFUserSelection = paParent->mbUseTaggedPDF;
    mbExportFormFieldsUserSelection = paParent->mbExportFormFields;

    if( !mpCbPDFA1b->IsChecked() )
    {// the value for PDF/A set by the ToggleExportPDFAHdl method called before
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
        mpCbExportNotesPages->Show(true);
        mpCbExportNotesPages->Check(paParent->mbExportNotesPages);
        mpCbExportHiddenSlides->Show(true);
        mpCbExportHiddenSlides->Check(paParent->mbExportHiddenSlides);
    }
    else
    {
        mpCbExportNotesPages->Show(false);
        mpCbExportNotesPages->Check(false);
        mpCbExportHiddenSlides->Show(false);
        mpCbExportHiddenSlides->Check(false);
    }

    mpCbExportEmptyPages->Check(!paParent->mbIsSkipEmptyPages);

    mpCbAddStream->Show(true);
    mpCbAddStream->Check(paParent->mbAddStream);

    mpCbAddStream->SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleAddStreamHdl ) );
    // init addstream dependencies
    ToggleAddStreamHdl(NULL);
}

// -----------------------------------------------------------------------------
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
        paParent->mbExportNotesPages = mpCbExportNotesPages->IsChecked();
    paParent->mbExportBookmarks = mpCbExportBookmarks->IsChecked();
    if ( mbIsPresentation )
        paParent->mbExportHiddenSlides = mpCbExportHiddenSlides->IsChecked();

    paParent->mbIsSkipEmptyPages = !mpCbExportEmptyPages->IsChecked();
    paParent->mbAddStream = mpCbAddStream->IsVisible() && mpCbAddStream->IsChecked();

    paParent->mbIsRangeChecked = sal_False;
    if( mpRbRange->IsChecked() )
    {
        paParent->mbIsRangeChecked = sal_True;
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

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabGeneralPage::Create( Window* pParent,
                                           const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabGeneralPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, TogglePagesHdl)
{
    mpEdPages->Enable( mpRbRange->IsChecked() );
    if ( mpRbRange->IsChecked() )
        mpEdPages->GrabFocus();
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl)
{
    mpFormsFrame->Enable(mpCbExportFormFields->IsChecked());
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleCompressionHdl)
{
    mpQualityFrame->Enable(mpRbJPEGCompression->IsChecked());
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl)
{
    mpCoReduceImageResolution->Enable( mpCbReduceImageResolution->IsChecked() );
    return 0;
}


IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleWatermarkHdl)
{
    mpEdWatermark->Enable( mpCbWatermark->IsChecked() );
    mpFtWatermark->Enable (mpCbWatermark->IsChecked() );
    if ( mpCbWatermark->IsChecked() )
        mpEdWatermark->GrabFocus();

    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleAddStreamHdl)
{
    if( mpCbAddStream->IsVisible() )
    {
        if( mpCbAddStream->IsChecked() )
        {
            mpRbAll->Check();
            mpRbRange->Enable( sal_False );
            mpRbSelection->Enable( sal_False );
            mpEdPages->Enable( sal_False );
            mpRbAll->Enable( sal_False );
        }
        else
        {
            mpRbAll->Enable( sal_True );
            mpRbRange->Enable( sal_True );
            mpRbSelection->Enable( sal_True );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(ImpPDFTabGeneralPage, ToggleExportPDFAHdl)
{
    //set the security page status (and its controls as well)
    ImpPDFTabSecurityPage* pSecPage = mpaParent ? mpaParent->getSecurityPage() : NULL;
    if (pSecPage)
    {
        pSecPage->ImplPDFASecurityControl(!mpCbPDFA1b->IsChecked());
    }

//PDF/A-1 needs tagged PDF, so  force disable the control, will be forced in pdfexport.
    bool bPDFA1Sel = mpCbPDFA1b->IsChecked();
    mpFormsFrame->Enable(bPDFA1Sel);
    if(bPDFA1Sel)
    {
//store the values of subordinate controls
        mbTaggedPDFUserSelection = mpCbTaggedPDF->IsChecked();
        mpCbTaggedPDF->Check();
        mpCbTaggedPDF->Enable(false);
        mbExportFormFieldsUserSelection = mpCbExportFormFields->IsChecked();
        mpCbExportFormFields->Check(false);
        mpCbExportFormFields->Enable(false);
    }
    else
    {
//retrieve the values of subordinate controls
        mpCbTaggedPDF->Enable();
        mpCbTaggedPDF->Check( mbTaggedPDFUserSelection );
        mpCbExportFormFields->Check( mbExportFormFieldsUserSelection );
        mpCbExportFormFields->Enable();
    }
    // PDF/A-1 doesn't allow launch action, so enable/disable the selection on
    // Link page
    ImpPDFTabLinksPage* pLinksPage = mpaParent ? mpaParent->getLinksPage() : NULL;
    if (pLinksPage)
        pLinksPage->ImplPDFALinkControl(!mpCbPDFA1b->IsChecked());

    // if a password was set, inform the user that this will not be used in PDF/A case
    if( mpCbPDFA1b->IsChecked() && pSecPage && pSecPage->hasPassword() )
    {
        WarningBox aBox( this, PDFFilterResId( RID_PDF_WARNPDFAPASSWORD ) );
        aBox.Execute();
    }

    return 0;
}

// the option features tab page
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfViewPage","filter/ui/pdfviewpage.ui", rCoreSet)
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

// -----------------------------------------------------------------------------
ImpPDFTabOpnFtrPage::~ImpPDFTabOpnFtrPage()
{
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabOpnFtrPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabOpnFtrPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
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

    paParent->mbFirstPageLeft = ( mbUseCTLFont ) ? mpCbPgLyFirstOnLeft->IsChecked() : sal_False;
}

// -----------------------------------------------------------------------------
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
    };

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
    };

    switch( paParent->mnMagnification )
    {
    default:
    case 0:
        mpRbMagnDefault->Check();
        mpNumZoom->Enable( sal_False );
        break;
    case 1:
        mpRbMagnFitWin->Check();
        mpNumZoom->Enable( sal_False );
        break;
    case 2:
        mpRbMagnFitWidth->Check();
        mpNumZoom->Enable( sal_False );
        break;
    case 3:
        mpRbMagnFitVisible->Check();
        mpNumZoom->Enable( sal_False );
        break;
    case 4:
        mpRbMagnZoom->Check();
        mpNumZoom->Enable( sal_True );
        break;
    };

    mpNumZoom->SetValue( paParent->mnZoom );
    mpNumInitialPage->SetValue( paParent->mnInitialPage );

    if( !mbUseCTLFont )
        mpCbPgLyFirstOnLeft->Hide( );
    else
    {
        mpRbPgLyContinueFacing->SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl ) );
        mpCbPgLyFirstOnLeft->Check( paParent->mbFirstPageLeft );
        ToggleRbPgLyContinueFacingHdl( NULL );
    }
}

IMPL_LINK_NOARG(ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl)
{
    mpCbPgLyFirstOnLeft->Enable( mpRbPgLyContinueFacing->IsChecked() );
    return 0;
}

IMPL_LINK( ImpPDFTabOpnFtrPage, ToggleRbMagnHdl, void*, )
{
    mpNumZoom->Enable( mpRbMagnZoom->IsChecked() );
    return 0;
}

////////////////////////////////////////////////////////
// The Viewer preferences tab page
// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::ImpPDFTabViewerPage( Window* pParent,
                                          const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, "PdfUserInterfacePage","filter/ui/pdfuserinterfacepage.ui", rCoreSet )

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

// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabViewerPage, ToggleRbBookmarksHdl, void*, )
{
    m_pNumBookmarkLevels->Enable( m_pRbVisibleBookmarkLevels->IsChecked() );
    return 0;
}
// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabViewerPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabViewerPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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
        m_pRbAllBookmarkLevels->Check( sal_True );
        m_pNumBookmarkLevels->Enable( sal_False );
    }
    else
    {
        m_pRbVisibleBookmarkLevels->Check( sal_True );
        m_pNumBookmarkLevels->Enable( sal_True );
        m_pNumBookmarkLevels->SetValue( paParent->mnOpenBookmarkLevels );
    }
}

// The Security preferences tab page
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage(Window* i_pParent, const SfxItemSet& i_rCoreSet)
    : SfxTabPage(i_pParent, "PdfSecurityPage","filter/ui/pdfsecuritypage.ui", i_rCoreSet)
    , msUserPwdTitle( PDFFilterResId( STR_PDF_EXPORT_UDPWD ) )
    , mbHaveOwnerPassword( false )
    , mbHaveUserPassword( false )
    , msOwnerPwdTitle( PDFFilterResId( STR_PDF_EXPORT_ODPWD ) )
{
    get(mpPbSetPwd, "setpassword");
    msStrSetPwd = get<Window>("setpasswordstitle")->GetText();

    get(mpUserPwdSet, "userpwdset");
    get(mpUserPwdUnset, "userpwdunset");
    get(mpUserPwdPdfa, "userpwdpdfa");

    get(mpOwnerPwdSet, "ownerpwdset");
    get(mpOwnerPwdUnset, "ownerpwdunset");
    get(mpOwnerPwdPdfa, "ownerpwdpdfa");

    get(mpPrintPermissions, "printing");
    get(mpRbPrintNone, "printnone");
    get(mpRbPrintLowRes, "printlow");
    get(mpRbPrintHighRes, "printnone");

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

// -----------------------------------------------------------------------------
ImpPDFTabSecurityPage::~ImpPDFTabSecurityPage()
{
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabSecurityPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabSecurityPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
void ImpPDFTabSecurityPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
// please note that in PDF/A-1a mode even if this are copied back,
// the security settings are forced disabled in PDFExport::Export
    paParent->mbEncrypt = mbHaveUserPassword;
    paParent->mxPreparedPasswords = mxPreparedPasswords;

    paParent->mbRestrictPermissions = mbHaveOwnerPassword;
    paParent->maPreparedOwnerPassword = maPreparedOwnerPassword;

//verify print status
    paParent->mnPrint = 0;
    if( mpRbPrintLowRes->IsChecked() )
        paParent->mnPrint = 1;
    else if( mpRbPrintHighRes->IsChecked() )
        paParent->mnPrint = 2;

//verify changes permitted
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


// -----------------------------------------------------------------------------
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
    };

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
    };

    mpCbEnableCopy->Check( paParent->mbCanCopyOrExtract );
    mpCbEnableAccessibility->Check( paParent->mbCanExtractForAccessibility );

// set the status of this windows, according to the PDFA selection
    enablePermissionControls();

    ImpPDFTabGeneralPage* pGeneralPage = paParent ? paParent->getGeneralPage() : NULL;

    if (pGeneralPage)
        ImplPDFASecurityControl(!pGeneralPage->IsPdfaSelected());
}

IMPL_LINK_NOARG(ImpPDFTabSecurityPage, ClickmaPbSetPwdHdl)
{
    SfxPasswordDialog aPwdDialog( this, &msUserPwdTitle );
    aPwdDialog.SetMinLen( 0 );
    aPwdDialog.ShowMinLengthText(false);
    aPwdDialog.ShowExtras( SHOWEXTRAS_CONFIRM | SHOWEXTRAS_PASSWORD2 | SHOWEXTRAS_CONFIRM2 );
    aPwdDialog.SetText(msStrSetPwd);
    aPwdDialog.SetGroup2Text(msOwnerPwdTitle);
    aPwdDialog.AllowAsciiOnly();
    if( aPwdDialog.Execute() == RET_OK )  //OK issued get password and set it
    {
        OUString aUserPW( aPwdDialog.GetPassword() );
        OUString aOwnerPW( aPwdDialog.GetPassword2() );

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
    return 0;
}

void ImpPDFTabSecurityPage::enablePermissionControls()
{
    bool bIsPDFASel = false;
    ImpPDFTabDialog* pParent = static_cast<ImpPDFTabDialog*>(GetTabDialog());
    ImpPDFTabGeneralPage* pGeneralPage = pParent ? pParent->getGeneralPage() : NULL;
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

////////////////////////////////////////////////////////
// This tab page is under control of the PDF/A-1a checkbox:
// implement a method to do it.
// -----------------------------------------------------------------------------
void    ImpPDFTabSecurityPage::ImplPDFASecurityControl( sal_Bool bEnableSecurity )
{
    if( bEnableSecurity )
    {
        Enable();
//after enable, check the status of control as if the dialog was initialized
    }
    else
        Enable( sal_False );

    enablePermissionControls();
}

////////////////////////////////////////////////////////
// The link preferences tab page (relative and other stuff)
// -----------------------------------------------------------------------------
ImpPDFTabLinksPage::ImpPDFTabLinksPage( Window* pParent,
                                              const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, "PdfLinksPage","filter/ui/pdflinkspage.ui",rCoreSet ),

    mbOpnLnksDefaultUserState( sal_False ),
    mbOpnLnksLaunchUserState( sal_False ),
    mbOpnLnksBrowserUserState( sal_False )
{
    get(m_pCbExprtBmkrToNmDst,"export");
    get(m_pCbOOoToPDFTargets ,"convert");
    get(m_pCbExportRelativeFsysLinks ,"exporturl");
    get(m_pRbOpnLnksDefault ,"default");
    get(m_pRbOpnLnksLaunch ,"openpdf");
    get(m_pRbOpnLnksBrowser ,"openinternet");
}

// -----------------------------------------------------------------------------
ImpPDFTabLinksPage::~ImpPDFTabLinksPage()
{
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabLinksPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabLinksPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
void ImpPDFTabLinksPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbExportRelativeFsysLinks = m_pCbExportRelativeFsysLinks->IsChecked();

    bool bIsPDFASel = false;
    ImpPDFTabGeneralPage* pGeneralPage = paParent ? paParent->getGeneralPage() : NULL;
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

// -----------------------------------------------------------------------------
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
        mbOpnLnksDefaultUserState = sal_True;
        break;
    case 1:
        m_pRbOpnLnksLaunch->Check();
        mbOpnLnksLaunchUserState = sal_True;
        break;
    case 2:
        m_pRbOpnLnksBrowser->Check();
        mbOpnLnksBrowserUserState = sal_True;
        break;
    }
    // now check the status of PDF/A selection
    // and set the link action accordingly
    // PDF/A-1 doesn't allow launch action on links
    //
    ImpPDFTabGeneralPage* pGeneralPage = paParent ? paParent->getGeneralPage() : NULL;
    if (pGeneralPage)
        ImplPDFALinkControl(!pGeneralPage->mpCbPDFA1b->IsChecked());
}

// -----------------------------------------------------------------------------
// called from general tab, with PDFA/1 selection status
// retrieves/store the status of Launch action selection
void ImpPDFTabLinksPage::ImplPDFALinkControl( sal_Bool bEnableLaunch )
{
// set the value and position of link type selection
    if( bEnableLaunch )
    {
        m_pRbOpnLnksLaunch->Enable();
//restore user state with no PDF/A-1 selected
        m_pRbOpnLnksDefault->Check( mbOpnLnksDefaultUserState );
        m_pRbOpnLnksLaunch->Check( mbOpnLnksLaunchUserState );
        m_pRbOpnLnksBrowser->Check( mbOpnLnksBrowserUserState );
    }
    else
    {
//save user state with no PDF/A-1 selected
        mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
        mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
        mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
        m_pRbOpnLnksLaunch->Enable( sal_False );
        if( mbOpnLnksLaunchUserState )
            m_pRbOpnLnksBrowser->Check();
    }
}

// -----------------------------------------------------------------------------
// reset the memory of Launch action present
// when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl)
{
    mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
    mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
    mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
    return 0;
}

// -----------------------------------------------------------------------------
// reset the memory of a launch action present
// when PDF/A-1 was requested
IMPL_LINK_NOARG(ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl)
{
    mbOpnLnksDefaultUserState = m_pRbOpnLnksDefault->IsChecked();
    mbOpnLnksLaunchUserState = m_pRbOpnLnksLaunch->IsChecked();
    mbOpnLnksBrowserUserState = m_pRbOpnLnksBrowser->IsChecked();
    return 0;
}

ImplErrorDialog::ImplErrorDialog( const std::set< vcl::PDFWriter::ErrorCode >& rErrors ) :
    ModalDialog( NULL, PDFFilterResId( RID_PDF_ERROR_DLG ) ),
    maFI( this, 0 ),
    maProcessText( this, PDFFilterResId( FT_PROCESS ) ),
    maErrors( this, WB_BORDER | WB_AUTOVSCROLL ),
    maExplanation( this, WB_WORDBREAK ),
    maButton( this, WB_DEFBUTTON )

{
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
            sal_uInt16 nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_PDFA_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_TRANSP_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDF13:
        {
            sal_uInt16 nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_VERSION_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_TRANSP_VERSION ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_FormAction_Omitted_PDFA:
        {
            sal_uInt16 nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_FORMACTION_PDFA_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_FORMACTION_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Converted:
        {
            sal_uInt16 nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_CONVERTED_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_TRANSP_CONVERTED ) ) );
        }
        break;
        default:
            break;
        }
    }

    FreeResource();

    if( maErrors.GetEntryCount() > 0 )
    {
        maErrors.SelectEntryPos( 0 );
        String* pStr = reinterpret_cast<String*>(maErrors.GetEntryData( 0 ));
        maExplanation.SetText( pStr ? *pStr : String() );
    }

    // adjust layout
    Image aWarnImage( WarningBox::GetStandardImage() );
    Size aImageSize( aWarnImage.GetSizePixel() );
    Size aDlgSize( GetSizePixel() );
    aImageSize.Width() += 6;
    aImageSize.Height() += 6;
    maFI.SetImage( aWarnImage );
    maFI.SetPosSizePixel( Point( 5, 5 ), aImageSize );
    maFI.Show();

    maProcessText.SetStyle( maProcessText.GetStyle() | WB_VCENTER );
    maProcessText.SetPosSizePixel( Point( aImageSize.Width() + 10, 5 ),
                                   Size(  aDlgSize.Width() - aImageSize.Width() - 15, aImageSize.Height() ) );

    Point aErrorLBPos( 5, aImageSize.Height() + 10 );
    Size aErrorLBSize( aDlgSize.Width()/2 - 10, aDlgSize.Height() - aErrorLBPos.Y() - 35 );
    maErrors.SetPosSizePixel( aErrorLBPos, aErrorLBSize );
    maErrors.SetSelectHdl( LINK( this, ImplErrorDialog, SelectHdl ) );
    maErrors.Show();

    maExplanation.SetPosSizePixel( Point( aErrorLBPos.X() + aErrorLBSize.Width() + 5, aErrorLBPos.Y() ),
                                   Size( aDlgSize.Width() - aErrorLBPos.X() - aErrorLBSize.Width() - 10, aErrorLBSize.Height() ) );
    maExplanation.Show();

    maButton.SetPosSizePixel( Point( (aDlgSize.Width() - 50)/2, aDlgSize.Height() - 30 ),
                              Size( 50, 25 ) );
    maButton.Show();
}

ImplErrorDialog::~ImplErrorDialog()
{
    // free strings again
    for( sal_uInt16 n = 0; n < maErrors.GetEntryCount(); n++ )
        delete (String*)maErrors.GetEntryData( n );
}

IMPL_LINK_NOARG(ImplErrorDialog, SelectHdl)
{
    String* pStr = reinterpret_cast<String*>(maErrors.GetEntryData( maErrors.GetSelectEntryPos() ));
    maExplanation.SetText( pStr ? *pStr : String() );
    return 0;
}

////////////////////////////////////////////////////////
// The digital signatures tab page
// -----------------------------------------------------------------------------
ImpPDFTabSigningPage::ImpPDFTabSigningPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PdfSignPage","filter/ui/pdfsignpage.ui", rCoreSet)
    , maSignCertificate()
{
    get(mpEdSignCert, "cert");
    get(mpPbSignCertSelect, "select");
    get(mpPbSignCertClear, "clear");
    get(mpEdSignPassword, "password");
    get(mpEdSignLocation, "location");
    get(mpEdSignContactInfo, "contact");
    get(mpEdSignReason, "reason");

    mpPbSignCertSelect->Enable( true );
    mpPbSignCertSelect->SetClickHdl( LINK( this, ImpPDFTabSigningPage, ClickmaPbSignCertSelect ) );
    mpPbSignCertClear->SetClickHdl( LINK( this, ImpPDFTabSigningPage, ClickmaPbSignCertClear ) );
}

// -----------------------------------------------------------------------------
ImpPDFTabSigningPage::~ImpPDFTabSigningPage()
{
}

IMPL_LINK_NOARG( ImpPDFTabSigningPage, ClickmaPbSignCertSelect )
{

    Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2" ) );

    maSignCertificate = xSigner->chooseCertificate();

    if (maSignCertificate.is())
    {
        mpEdSignCert->SetText(maSignCertificate->getSubjectName());
        mpPbSignCertClear->Enable( true );
        mpEdSignLocation->Enable( true );
        mpEdSignPassword->Enable( true );
        mpEdSignContactInfo->Enable( true );
        mpEdSignReason->Enable( true );
    }

    return 0;
}

IMPL_LINK_NOARG( ImpPDFTabSigningPage, ClickmaPbSignCertClear )
{
    mpEdSignCert->SetText(OUString(""));
    maSignCertificate.clear();
    mpPbSignCertClear->Enable( false );
    mpEdSignLocation->Enable( false );
    mpEdSignPassword->Enable( false );
    mpEdSignContactInfo->Enable( false );
    mpEdSignReason->Enable( false );

    return 0;
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabSigningPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabSigningPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
void ImpPDFTabSigningPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbSignPDF = maSignCertificate.is();
    paParent->maSignCertificate = maSignCertificate;
    paParent->msSignLocation = mpEdSignLocation->GetText();
    paParent->msSignPassword = mpEdSignPassword->GetText();
    paParent->msSignContact = mpEdSignContactInfo->GetText();
    paParent->msSignReason = mpEdSignReason->GetText();

}

// -----------------------------------------------------------------------------
void ImpPDFTabSigningPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{

    mpEdSignLocation->Enable( false );
    mpEdSignPassword->Enable( false );
    mpEdSignContactInfo->Enable( false );
    mpEdSignReason->Enable( false );
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
