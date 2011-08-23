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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "impdialog.hxx"
#include "impdialog.hrc"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "sfx2/passwd.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/text/XTextRange.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"

#include <boost/shared_ptr.hpp>

static ResMgr& getPDFFilterResMgr()
{
    static ResMgr *pRes = ResMgr::CreateResMgr( "pdffilter", Application::GetSettings().GetUILocale());
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
ImpPDFTabDialog::ImpPDFTabDialog( Window* pParent,
                                  Sequence< PropertyValue >& rFilterData,
                                  const Reference< XComponent >& rxDoc,
                                  const Reference< lang::XMultiServiceFactory >& xFact
                                  ) :
    SfxTabDialog( pParent, PDFFilterResId( RID_PDF_EXPORT_DLG ), 0, FALSE, 0 ),
    mxMSF( xFact ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/PDF/Export/" ) ), &rFilterData ),
    maConfigI18N( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/I18N/CTL/" ) ) ),
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
    mbExportNotesPages( sal_False ),
    mbUseTransitionEffects( sal_False ),
    mbIsSkipEmptyPages( sal_True ),
    mbAddStream( sal_False ),
    mbEmbedStandardFonts( sal_False ),
    mnFormsType( 0 ),
    mbExportFormFields( sal_True ),
    mbAllowDuplicateFieldNames( sal_False ),
    mbExportBookmarks( sal_True ),
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
    mbExportBmkToPDFDestination( sal_False )
{
    FreeResource();
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
    catch( RuntimeException )
    {
    }
    mbSelectionPresent = maSelection.hasValue();
    if ( mbSelectionPresent )
    {
        Reference< drawing::XShapes > xShapes;
        if ( ( maSelection >>= xShapes ) == sal_False )	// XShapes is always a selection
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
                    if ( xTextRange.is() && ( xTextRange->getString().getLength() == 0 ) )
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
            if ( xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) ) )
                mbIsPresentation = sal_True;
            if ( xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.GenericTextDocument" ) ) ) )
                mbIsWriter = sal_True;
        }
    }
    catch( RuntimeException )
    {
    }

//get the CTL (Complex Text Layout) from general options, returns TRUE if we have a CTL font on our hands.
    mbUseCTLFont = maConfigI18N.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "CTLFont" ) ), sal_False );

    mbUseLosslessCompression = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), sal_False );
    mnQuality = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), 90 );
    mbReduceImageResolution = maConfigItem.ReadBool(  OUString( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), sal_False );
    mnMaxImageResolution = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), 300 );

    mbUseTaggedPDF = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), sal_False );
    mnPDFTypeSelection =  maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectPdfVersion" ) ), 0 );
    if ( mbIsPresentation )
        mbExportNotesPages = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages"  ) ), sal_False );
    mbExportNotes = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes"  ) ), sal_False );

    mbExportBookmarks = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarks" ) ), sal_True );
    mnOpenBookmarkLevels = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenBookmarkLevels" ) ), -1 );
    mbUseTransitionEffects = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects"  ) ), sal_True );
    mbIsSkipEmptyPages = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsSkipEmptyPages"  ) ), sal_False );
    mbAddStream = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "IsAddStream" ) ), sal_False );
    mbEmbedStandardFonts = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "EmbedStandardFonts" ) ), sal_False );

    mnFormsType = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), 0 );
    mbExportFormFields = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportFormFields" ) ), sal_True );
    if ( ( mnFormsType < 0 ) || ( mnFormsType > 3 ) )
        mnFormsType = 0;
    mbAllowDuplicateFieldNames = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowDuplicateFieldNames" ) ), sal_False );

//prepare values for the Viewer tab page
    mbHideViewerToolbar = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerToolbar" ) ), sal_False );
    mbHideViewerMenubar = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerMenubar" ) ), sal_False );
    mbHideViewerWindowControls = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerWindowControls" ) ), sal_False );
    mbResizeWinToInit = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ResizeWindowToInitialPage" ) ), sal_False );
    mbCenterWindow = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "CenterWindow" ) ), sal_False );
    mbOpenInFullScreenMode = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenInFullScreenMode" ) ), sal_False );
    mbDisplayPDFDocumentTitle = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "DisplayPDFDocumentTitle" ) ), sal_True );

    mnInitialView = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialView" ) ), 0 );
    mnMagnification = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Magnification" ) ), 0 );
    mnZoom = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Zoom" ) ), 100 );
    mnPageLayout = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayout" ) ), 0 );
    mbFirstPageLeft = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPageOnLeft" ) ), sal_False );
    mnInitialPage = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialPage" ) ), 1 );
    if( mnInitialPage < 1 )
        mnInitialPage = 1;

//prepare values for the security tab page
    mnPrint = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Printing" ) ), 2 );
    mnChangesAllowed = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Changes" ) ), 4 );
    mbCanCopyOrExtract = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableCopyingOfContent" ) ), sal_True );
    mbCanExtractForAccessibility = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableTextAccessForAccessibilityTools" ) ), sal_True );

//prepare values for relative links
    mbExportRelativeFsysLinks = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportLinksRelativeFsys" ) ), sal_False );

    mnViewPDFMode = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PDFViewSelection" ) ), 0 );

    mbConvertOOoTargets = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ConvertOOoTargetToPDFTarget" ) ), sal_False );
    mbExportBmkToPDFDestination = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarksToPDFDestination" ) ), sal_False );

//queue the tab pages for later creation (created when first shown)
    AddTabPage( RID_PDF_TAB_SECURITY, ImpPDFTabSecurityPage::Create, 0 );
    AddTabPage( RID_PDF_TAB_LINKS, ImpPDFTabLinksPage::Create, 0 );
    AddTabPage( RID_PDF_TAB_VPREFER, ImpPDFTabViewerPage::Create, 0 );
    AddTabPage( RID_PDF_TAB_OPNFTR, ImpPDFTabOpnFtrPage::Create, 0 );

//last queued is the first to be displayed (or so it seems..)
    AddTabPage( RID_PDF_TAB_GENER, ImpPDFTabGeneralPage::Create, 0 );

//get the string property value (from sfx2/source/dialog/mailmodel.cxx) to overwrite the text for the Ok button
    ::rtl::OUString sOkButtonText = maConfigItem.ReadString( OUString( RTL_CONSTASCII_USTRINGPARAM( "_OkButtonString" ) ), OUString() );

//change text on the Ok button: get the relevant string from resources, update it on the button
//according to the exported pdf file destination: send as e-mail or write to file?
    GetOKButton().SetText( ( sOkButtonText.getLength() > 0 ) ?
                            sOkButtonText : OUString( String( PDFFilterResId( STR_PDF_EXPORT ) ) ));

//remove the reset button, not needed in this tabbed dialog
    RemoveResetButton();
/////////////////
}

// -----------------------------------------------------------------------------
ImpPDFTabDialog::~ImpPDFTabDialog()
{
//delete the pages, needed because otherwise the child tab pages
//don't get destroyed
    RemoveTabPage( RID_PDF_TAB_GENER );
    RemoveTabPage( RID_PDF_TAB_VPREFER );
    RemoveTabPage( RID_PDF_TAB_OPNFTR );
    RemoveTabPage( RID_PDF_TAB_LINKS );
    RemoveTabPage( RID_PDF_TAB_SECURITY );
}

// -----------------------------------------------------------------------------
void ImpPDFTabDialog::PageCreated( USHORT _nId,
                                   SfxTabPage& _rPage )
{
    switch( _nId )
    {
    case RID_PDF_TAB_GENER:
        ( ( ImpPDFTabGeneralPage* )&_rPage )->SetFilterConfigItem( this );
        break;
    case RID_PDF_TAB_VPREFER:
        ( ( ImpPDFTabViewerPage* )&_rPage )->SetFilterConfigItem( this );
        break;
    case RID_PDF_TAB_OPNFTR:
        ( ( ImpPDFTabOpnFtrPage* )&_rPage )->SetFilterConfigItem( this );
        break;
    case RID_PDF_TAB_LINKS:
        ( ( ImpPDFTabLinksPage* )&_rPage )->SetFilterConfigItem( this );
        break;
    case RID_PDF_TAB_SECURITY:
        ( ( ImpPDFTabSecurityPage* )&_rPage )->SetFilterConfigItem( this );
        break;
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
    if( GetTabPage( RID_PDF_TAB_GENER ) )
        ( ( ImpPDFTabGeneralPage* )GetTabPage( RID_PDF_TAB_GENER ) )->GetFilterConfigItem( this );
    if( GetTabPage( RID_PDF_TAB_VPREFER ) )
        ( ( ImpPDFTabViewerPage* )GetTabPage( RID_PDF_TAB_VPREFER ) )->GetFilterConfigItem( this );
    if( GetTabPage( RID_PDF_TAB_OPNFTR ) )
        ( ( ImpPDFTabOpnFtrPage* )GetTabPage( RID_PDF_TAB_OPNFTR ) )->GetFilterConfigItem( this );
    if( GetTabPage( RID_PDF_TAB_LINKS ) )
        ( ( ImpPDFTabLinksPage* )GetTabPage( RID_PDF_TAB_LINKS ) )->GetFilterConfigItem( this );
    if( GetTabPage( RID_PDF_TAB_SECURITY ) )
        ( ( ImpPDFTabSecurityPage* )GetTabPage( RID_PDF_TAB_SECURITY ) )->GetFilterConfigItem( this );

//prepare the items to be returned
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), mbUseLosslessCompression );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), mnQuality );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), mbReduceImageResolution );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), mnMaxImageResolution );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), mbUseTaggedPDF );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectPdfVersion" ) ), mnPDFTypeSelection );

    if ( mbIsPresentation )
        maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) ), mbExportNotesPages );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes" ) ), mbExportNotes );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarks" ) ), mbExportBookmarks );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects" ) ), mbUseTransitionEffects );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsSkipEmptyPages" ) ), mbIsSkipEmptyPages );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAddStream" ) ), mbAddStream );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EmbedStandardFonts" ) ), mbEmbedStandardFonts );

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), mnFormsType );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportFormFields" ) ), mbExportFormFields );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowDuplicateFieldNames" ) ), mbAllowDuplicateFieldNames );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerToolbar" ) ), mbHideViewerToolbar );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerMenubar" ) ), mbHideViewerMenubar );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerWindowControls" ) ), mbHideViewerWindowControls );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ResizeWindowToInitialPage" ) ), mbResizeWinToInit );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "CenterWindow" ) ), mbCenterWindow );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenInFullScreenMode" ) ), mbOpenInFullScreenMode );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "DisplayPDFDocumentTitle" ) ), mbDisplayPDFDocumentTitle );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialView" ) ), mnInitialView );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Magnification" ) ), mnMagnification);
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Zoom" ) ), mnZoom );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialPage" ) ), mnInitialPage );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayout" ) ), mnPageLayout );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPageOnLeft" ) ), mbFirstPageLeft );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenBookmarkLevels" ) ), mnOpenBookmarkLevels );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportLinksRelativeFsys" ) ), mbExportRelativeFsysLinks );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PDFViewSelection" ) ), mnViewPDFMode );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ConvertOOoTargetToPDFTarget" ) ), mbConvertOOoTargets );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarksToPDFDestination" ) ), mbExportBmkToPDFDestination );

    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Printing" ) ), mnPrint );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Changes" ) ), mnChangesAllowed );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableCopyingOfContent" ) ), mbCanCopyOrExtract );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableTextAccessForAccessibilityTools" ) ), mbCanExtractForAccessibility );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    int nElementAdded = 5;

    aRet.realloc( aRet.getLength() + nElementAdded );

// add the encryption enable flag
    aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "EncryptFile" ) );
    aRet[ aRet.getLength() - nElementAdded ].Value <<= mbEncrypt;
    nElementAdded--;

// add the open password
    aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentOpenPassword" ) );
    aRet[ aRet.getLength() - nElementAdded ].Value <<= OUString( msUserPassword );
    nElementAdded--;

//the restrict permission flag (needed to have the scripting consistent with the dialog)
    aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RestrictPermissions" ) );
    aRet[ aRet.getLength() - nElementAdded ].Value <<= mbRestrictPermissions;
    nElementAdded--;

//add the permission password
    aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PermissionPassword" ) );
    aRet[ aRet.getLength() - nElementAdded ].Value <<= OUString( msOwnerPassword );
    nElementAdded--;

// this should be the last added...
    if( mbIsRangeChecked )
    {
        aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) );
        aRet[ aRet.getLength() - nElementAdded ].Value <<= OUString( msPageRange );
    }
    else if( mbSelectionIsChecked )
    {
        aRet[ aRet.getLength() - nElementAdded ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) );
        aRet[ aRet.getLength() - nElementAdded ].Value <<= maSelection;
    }

    return aRet;
}

// -----------------------------------------------------------------------------
ImpPDFTabGeneralPage::ImpPDFTabGeneralPage( Window* pParent,
                                            const SfxItemSet& rCoreSet
                                            ) :
    SfxTabPage( pParent, PDFFilterResId( RID_PDF_TAB_GENER ), rCoreSet ),

    maFlPages( this, PDFFilterResId( FL_PAGES ) ),
    maRbAll( this, PDFFilterResId( RB_ALL ) ),
    maRbRange( this, PDFFilterResId( RB_RANGE ) ),
    maRbSelection( this, PDFFilterResId( RB_SELECTION ) ),
    maEdPages( this, PDFFilterResId( ED_PAGES ) ),

    maFlCompression( this, PDFFilterResId( FL_IMAGES ) ),
    maRbLosslessCompression( this, PDFFilterResId( RB_LOSSLESSCOMPRESSION ) ),
    maRbJPEGCompression( this, PDFFilterResId( RB_JPEGCOMPRESSION ) ),
    maFtQuality( this, PDFFilterResId( FT_QUALITY ) ),
    maNfQuality( this, PDFFilterResId( NF_QUALITY ) ),
    maCbReduceImageResolution( this, PDFFilterResId( CB_REDUCEIMAGERESOLUTION ) ),
    maCoReduceImageResolution( this, PDFFilterResId( CO_REDUCEIMAGERESOLUTION ) ),

    maFlGeneral( this, PDFFilterResId( FL_GENERAL ) ),
    maCbPDFA1b( this, PDFFilterResId( CB_PDFA_1B_SELECT ) ),

    maCbTaggedPDF( this, PDFFilterResId( CB_TAGGEDPDF ) ),
    mbTaggedPDFUserSelection( sal_False ),

    maCbExportFormFields( this, PDFFilterResId( CB_EXPORTFORMFIELDS ) ),
    mbExportFormFieldsUserSelection( sal_False ),
    mbEmbedStandardFontsUserSelection( sal_False ),
    maFtFormsFormat( this, PDFFilterResId( FT_FORMSFORMAT ) ),
    maLbFormsFormat( this, PDFFilterResId( LB_FORMSFORMAT ) ),
    maCbAllowDuplicateFieldNames( this, PDFFilterResId( CB_ALLOWDUPLICATEFIELDNAMES ) ),

    maCbExportBookmarks( this, PDFFilterResId( CB_EXPORTBOOKMARKS ) ),
    maCbExportNotes( this, PDFFilterResId( CB_EXPORTNOTES ) ),
    maCbExportNotesPages( this, PDFFilterResId( CB_EXPORTNOTESPAGES ) ),
    maCbExportEmptyPages( this, PDFFilterResId( CB_EXPORTEMPTYPAGES ) ),
    maCbAddStream( this, PDFFilterResId( CB_ADDSTREAM ) ),
    maCbEmbedStandardFonts( this, PDFFilterResId( CB_EMBEDSTANDARDFONTS ) ),
    mbIsPresentation( sal_False ),
    mbIsWriter( sal_False),
    mpaParent( 0 )
{
    FreeResource();

    // pb: #i91991# maCbExportEmptyPages double-spaced if necessary
    Size aSize = maCbExportEmptyPages.GetSizePixel();
    Size aMinSize = maCbExportEmptyPages.CalcMinimumSize();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Size aNewSize = maCbExportNotes.GetSizePixel();
        long nDelta = aSize.Height() - aNewSize.Height();
        maCbExportEmptyPages.SetSizePixel( aNewSize );
        Point aNewPos = maCbAddStream.GetPosPixel();
        aNewPos.Y() -= nDelta;
        maCbAddStream.SetPosPixel( aNewPos );
        aNewPos = maCbEmbedStandardFonts.GetPosPixel();
        aNewPos.Y() -= nDelta;
        maCbEmbedStandardFonts.SetPosPixel( aNewPos );
    }
    maCbExportEmptyPages.SetStyle( maCbExportEmptyPages.GetStyle() | WB_VCENTER );
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
    maRbRange.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, TogglePagesHdl ) );

    maRbAll.Check();
    TogglePagesHdl( NULL );

    maNfQuality.SetUnit( FUNIT_PERCENT );
    maNfQuality.SetMin( 1, FUNIT_PERCENT );
    maNfQuality.SetMax( 100, FUNIT_PERCENT );

    maRbSelection.Enable( paParent->mbSelectionPresent );
    mbIsPresentation = paParent->mbIsPresentation;
    mbIsWriter = paParent->mbIsWriter;

    maCbExportEmptyPages.Enable( mbIsWriter );

    maRbLosslessCompression.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const sal_Bool bUseLosslessCompression = paParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        maRbLosslessCompression.Check();
    else
        maRbJPEGCompression.Check();

    maNfQuality.SetValue( paParent->mnQuality, FUNIT_PERCENT );
    maNfQuality.Enable( bUseLosslessCompression == sal_False );

    maCbReduceImageResolution.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl ) );
    const sal_Bool	bReduceImageResolution = paParent->mbReduceImageResolution;
    maCbReduceImageResolution.Check( bReduceImageResolution );
    String aStrRes( String::CreateFromInt32( paParent->mnMaxImageResolution ) );
    aStrRes.Append( String( RTL_CONSTASCII_USTRINGPARAM( " DPI" ) ) );
    maCoReduceImageResolution.SetText( aStrRes );
    maCoReduceImageResolution.Enable( bReduceImageResolution );

    maCbPDFA1b.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleExportPDFAHdl) );
    switch( paParent->mnPDFTypeSelection )
    {
    default:
    case 0: maCbPDFA1b.Check( FALSE ); // PDF 1.4
        break;
    case 1: maCbPDFA1b.Check(); // PDF/A-1a
        break;
    }
    ToggleExportPDFAHdl( NULL );

    maCbExportFormFields.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl ) );

// get the form values, for use with PDF/A-1 selection interface
    mbTaggedPDFUserSelection = paParent->mbUseTaggedPDF;
    mbExportFormFieldsUserSelection = paParent->mbExportFormFields;
    mbEmbedStandardFontsUserSelection = paParent->mbEmbedStandardFonts;

    if( !maCbPDFA1b.IsChecked() )
    {// the value for PDF/A set by the ToggleExportPDFAHdl method called before
        maCbTaggedPDF.Check( mbTaggedPDFUserSelection  );
        maCbExportFormFields.Check( mbExportFormFieldsUserSelection );
        maCbEmbedStandardFonts.Check( mbEmbedStandardFontsUserSelection );
    }

    maLbFormsFormat.SelectEntryPos( (sal_uInt16)paParent->mnFormsType );
    maLbFormsFormat.Enable( paParent->mbExportFormFields );
    maCbAllowDuplicateFieldNames.Check( paParent->mbAllowDuplicateFieldNames );
    maCbAllowDuplicateFieldNames.Enable( paParent->mbExportFormFields );
    
    maCbExportBookmarks.Check( paParent->mbExportBookmarks );
    
    maCbExportNotes.Check( paParent->mbExportNotes );

    if ( mbIsPresentation )
    {
        maCbExportNotesPages.Show( TRUE );
        maCbExportNotesPages.Check( paParent->mbExportNotesPages );
    }
    else
    {
        long nCheckBoxHeight =
            maCbExportNotesPages.LogicToPixel( Size( 13, 13 ), MAP_APPFONT ).Height();

        Point aPos = maCbExportEmptyPages.GetPosPixel();
        maCbExportEmptyPages.SetPosPixel( Point( aPos.X(), aPos.Y() - nCheckBoxHeight ) );
        aPos = maCbAddStream.GetPosPixel();
        maCbAddStream.SetPosPixel( Point( aPos.X(), aPos.Y() - nCheckBoxHeight ) );
        aPos = maCbEmbedStandardFonts.GetPosPixel();
        maCbEmbedStandardFonts.SetPosPixel( Point( aPos.X(), aPos.Y() - nCheckBoxHeight ) );
        maCbExportNotesPages.Show( FALSE );
        maCbExportNotesPages.Check( FALSE );
    }

    maCbExportEmptyPages.Check( !paParent->mbIsSkipEmptyPages );

    Reference< XMultiServiceFactory > xFactory = paParent->getServiceFactory();
    Reference< XInterface > xIfc;
    if( xFactory.is() )
    {
        xIfc = xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.documents.PDFDetector" ) ) );
    }
    if( xIfc.is() )
    {
        maCbAddStream.Show( TRUE );
        maCbAddStream.Check( paParent->mbAddStream );
    }
    else
    {
        maCbAddStream.Show( FALSE );
        maCbAddStream.Check( FALSE );
    }
    maCbAddStream.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleAddStreamHdl ) );
    // init addstream dependencies
    ToggleAddStreamHdl( NULL );
}

// -----------------------------------------------------------------------------
void ImpPDFTabGeneralPage::GetFilterConfigItem( ImpPDFTabDialog* paParent )
{
// updating the FilterData sequence and storing FilterData to configuration
    paParent->mbUseLosslessCompression = maRbLosslessCompression.IsChecked();
    paParent->mnQuality = static_cast<sal_Int32>(maNfQuality.GetValue());
    paParent->mbReduceImageResolution = maCbReduceImageResolution.IsChecked();
    paParent->mnMaxImageResolution = maCoReduceImageResolution.GetText().ToInt32();
    paParent->mbExportNotes = maCbExportNotes.IsChecked();
    if ( mbIsPresentation )
        paParent->mbExportNotesPages = maCbExportNotesPages.IsChecked();
    paParent->mbExportBookmarks = maCbExportBookmarks.IsChecked();

    paParent->mbIsSkipEmptyPages =  !maCbExportEmptyPages.IsChecked();
    paParent->mbAddStream = maCbAddStream.IsVisible() && maCbAddStream.IsChecked();

    paParent->mbIsRangeChecked = sal_False;
    if( maRbRange.IsChecked() )
    {
        paParent->mbIsRangeChecked = sal_True;
        paParent->msPageRange = String( maEdPages.GetText() ); //FIXME all right on other languages ?
    }
    else if( maRbSelection.IsChecked() )
    {
        paParent->mbSelectionIsChecked = maRbSelection.IsChecked();
    }

    paParent->mnPDFTypeSelection = 0;
    if( maCbPDFA1b.IsChecked() )
    {
        paParent->mnPDFTypeSelection = 1;
        paParent->mbUseTaggedPDF =  mbTaggedPDFUserSelection;
        paParent->mbExportFormFields = mbExportFormFieldsUserSelection;
        paParent->mbEmbedStandardFonts = mbEmbedStandardFontsUserSelection;
    }
    else
    {
        paParent->mbUseTaggedPDF =  maCbTaggedPDF.IsChecked();
        paParent->mbExportFormFields = maCbExportFormFields.IsChecked();
        paParent->mbEmbedStandardFonts = maCbEmbedStandardFonts.IsChecked();
    }

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    paParent->mnFormsType = (sal_Int32) maLbFormsFormat.GetSelectEntryPos();
    paParent->mbAllowDuplicateFieldNames = maCbAllowDuplicateFieldNames.IsChecked();
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabGeneralPage::Create( Window* pParent,
                                           const SfxItemSet& rAttrSet)
{
    return ( new  ImpPDFTabGeneralPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, TogglePagesHdl, void*, EMPTYARG )
{
    maEdPages.Enable( maRbRange.IsChecked() );
    maEdPages.SetReadOnly( !maRbRange.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleExportFormFieldsHdl, void*, EMPTYARG )
{
    maLbFormsFormat.Enable( maCbExportFormFields.IsChecked() );
    maCbAllowDuplicateFieldNames.Enable( maCbExportFormFields.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleCompressionHdl, void*, EMPTYARG )
{
    maNfQuality.Enable( maRbJPEGCompression.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl, void*, EMPTYARG )
{
    maCoReduceImageResolution.Enable( maCbReduceImageResolution.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleAddStreamHdl, void*, EMPTYARG )
{
    if( maCbAddStream.IsVisible() )
    {
        if( maCbAddStream.IsChecked() )
        {
            maRbAll.Check();
            maRbRange.Enable( FALSE );
            maRbSelection.Enable( FALSE );
            maEdPages.Enable( FALSE );
            maEdPages.SetReadOnly( TRUE );
            maRbAll.Enable( FALSE );
        }
        else
        {
            maRbAll.Enable( TRUE );
            maRbRange.Enable( TRUE );
            maRbSelection.Enable( TRUE );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleExportPDFAHdl, void*, EMPTYARG )
{
//set the security page status (and its controls as well)
    if( mpaParent && mpaParent->GetTabPage( RID_PDF_TAB_SECURITY ) )
        ( ( ImpPDFTabSecurityPage* )mpaParent->GetTabPage( RID_PDF_TAB_SECURITY ) )->ImplPDFASecurityControl( !maCbPDFA1b.IsChecked() );

//PDF/A-1 needs tagged PDF, so  force disable the control, will be forced in pdfexport.
    sal_Bool bPDFA1Sel = maCbPDFA1b.IsChecked();
    maFtFormsFormat.Enable( !bPDFA1Sel );
    maLbFormsFormat.Enable( !bPDFA1Sel );
    maCbAllowDuplicateFieldNames.Enable( !bPDFA1Sel );
    if(bPDFA1Sel)
    {
//store the values of subordinate controls
        mbTaggedPDFUserSelection = maCbTaggedPDF.IsChecked();
        maCbTaggedPDF.Check();
        maCbTaggedPDF.Enable( sal_False );
        mbExportFormFieldsUserSelection = maCbExportFormFields.IsChecked();
        maCbExportFormFields.Check( sal_False );
        maCbExportFormFields.Enable( sal_False );
        mbEmbedStandardFontsUserSelection = maCbEmbedStandardFonts.IsChecked();
        maCbEmbedStandardFonts.Check( sal_True );
        maCbEmbedStandardFonts.Enable( sal_False );
    }
    else
    {
//retrieve the values of subordinate controls
        maCbTaggedPDF.Enable();
        maCbTaggedPDF.Check( mbTaggedPDFUserSelection );
        maCbExportFormFields.Check( mbExportFormFieldsUserSelection );
        maCbExportFormFields.Enable();
        maCbEmbedStandardFonts.Check( mbEmbedStandardFontsUserSelection );
        maCbEmbedStandardFonts.Enable();
    }
// PDF/A-1 doesn't allow launch action, so enable/disable the selection on
// Link page
    if( mpaParent && mpaParent->GetTabPage( RID_PDF_TAB_LINKS ) )
        ( ( ImpPDFTabLinksPage* )mpaParent->GetTabPage( RID_PDF_TAB_LINKS ) )->ImplPDFALinkControl( !maCbPDFA1b.IsChecked() );

    return 0;
}

/////////////////////////////////////////////////////////////////
// the option features tab page
// -----------------------------------------------------------------------------
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage( Window* pParent,
                                          const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, PDFFilterResId( RID_PDF_TAB_OPNFTR ), rCoreSet ),

    maFlInitialView( this, PDFFilterResId( FL_INITVIEW ) ),
    maRbOpnPageOnly( this, PDFFilterResId( RB_OPNMODE_PAGEONLY ) ),
    maRbOpnOutline( this, PDFFilterResId( RB_OPNMODE_OUTLINE ) ),
    maRbOpnThumbs( this, PDFFilterResId( RB_OPNMODE_THUMBS ) ),
    maFtInitialPage( this, PDFFilterResId( FT_MAGNF_INITIAL_PAGE ) ),
    maNumInitialPage( this, PDFFilterResId( NUM_MAGNF_INITIAL_PAGE ) ),

    maFlMagnification( this, PDFFilterResId( FL_MAGNIFICATION ) ),
    maRbMagnDefault( this, PDFFilterResId( RB_MAGNF_DEFAULT ) ),
    maRbMagnFitWin( this, PDFFilterResId( RB_MAGNF_WIND ) ),
    maRbMagnFitWidth( this, PDFFilterResId( RB_MAGNF_WIDTH ) ),
    maRbMagnFitVisible( this, PDFFilterResId( RB_MAGNF_VISIBLE ) ),
    maRbMagnZoom( this, PDFFilterResId( RB_MAGNF_ZOOM ) ),
    maNumZoom( this, PDFFilterResId( NUM_MAGNF_ZOOM ) ),

    maFlPageLayout( this, PDFFilterResId( FL_PAGE_LAYOUT ) ),
    maRbPgLyDefault( this, PDFFilterResId( RB_PGLY_DEFAULT ) ),
    maRbPgLySinglePage( this, PDFFilterResId( RB_PGLY_SINGPG ) ),
    maRbPgLyContinue( this, PDFFilterResId( RB_PGLY_CONT ) ),
    maRbPgLyContinueFacing( this, PDFFilterResId( RB_PGLY_CONTFAC ) ),
    maCbPgLyFirstOnLeft( this, PDFFilterResId( CB_PGLY_FIRSTLEFT ) ),
    mbUseCTLFont( sal_False )
{
    FreeResource();

    maRbMagnDefault.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    maRbMagnFitWin.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    maRbMagnFitWidth.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    maRbMagnFitVisible.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
    maRbMagnZoom.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbMagnHdl ) );
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
    if( maRbOpnOutline.IsChecked() )
        paParent->mnInitialView = 1;
    else if( maRbOpnThumbs.IsChecked() )
        paParent->mnInitialView = 2;

    paParent->mnMagnification = 0;
    if( maRbMagnFitWin.IsChecked() )
        paParent->mnMagnification = 1;
    else if( maRbMagnFitWidth.IsChecked() )
        paParent->mnMagnification = 2;
    else if( maRbMagnFitVisible.IsChecked() )
        paParent->mnMagnification = 3;
    else if( maRbMagnZoom.IsChecked() )
    {
        paParent->mnMagnification = 4;
        paParent->mnZoom = static_cast<sal_Int32>(maNumZoom.GetValue());
    }

    paParent->mnInitialPage = static_cast<sal_Int32>(maNumInitialPage.GetValue());

    paParent->mnPageLayout = 0;
    if( maRbPgLySinglePage.IsChecked() )
        paParent->mnPageLayout = 1;
    else if( maRbPgLyContinue.IsChecked() )
        paParent->mnPageLayout = 2;
    else if( maRbPgLyContinueFacing.IsChecked() )
        paParent->mnPageLayout = 3;

    paParent->mbFirstPageLeft = ( mbUseCTLFont ) ? maCbPgLyFirstOnLeft.IsChecked() : sal_False;
}

// -----------------------------------------------------------------------------
void ImpPDFTabOpnFtrPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    mbUseCTLFont = paParent->mbUseCTLFont;
    switch( paParent->mnPageLayout )
    {
    default:
    case 0:
        maRbPgLyDefault.Check();
        break;
    case 1:
        maRbPgLySinglePage.Check();
        break;
    case 2:
        maRbPgLyContinue.Check();
        break;
    case 3:
        maRbPgLyContinueFacing.Check();
        break;
    };

    switch( paParent->mnInitialView )
    {
    default:
    case 0:
        maRbOpnPageOnly.Check();
        break;
    case 1:
        maRbOpnOutline.Check();
        break;
    case 2:
        maRbOpnThumbs.Check();
        break;
    };

    switch( paParent->mnMagnification )
    {
    default:
    case 0:
        maRbMagnDefault.Check();
        maNumZoom.Enable( FALSE );
        break;
    case 1:
        maRbMagnFitWin.Check();
        maNumZoom.Enable( FALSE );
        break;
    case 2:
        maRbMagnFitWidth.Check();
        maNumZoom.Enable( FALSE );
        break;
    case 3:
        maRbMagnFitVisible.Check();
        maNumZoom.Enable( FALSE );
        break;
    case 4:
        maRbMagnZoom.Check();
        maNumZoom.Enable( TRUE );
        break;
    };

    maNumZoom.SetValue( paParent->mnZoom );
    maNumInitialPage.SetValue( paParent->mnInitialPage );

    if( !mbUseCTLFont )
        maCbPgLyFirstOnLeft.Hide( );
    else
    {
        maRbPgLyContinueFacing.SetToggleHdl( LINK( this, ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl ) );
        maCbPgLyFirstOnLeft.Check( paParent->mbFirstPageLeft );
        ToggleRbPgLyContinueFacingHdl( NULL );
    }
}

IMPL_LINK( ImpPDFTabOpnFtrPage, ToggleRbPgLyContinueFacingHdl, void*, p )
{
    p = p; //for compiler warning
    maCbPgLyFirstOnLeft.Enable( maRbPgLyContinueFacing.IsChecked() );
    return 0;
}

IMPL_LINK( ImpPDFTabOpnFtrPage, ToggleRbMagnHdl, void*, )
{
    maNumZoom.Enable( maRbMagnZoom.IsChecked() );
    return 0;
}

////////////////////////////////////////////////////////
// The Viewer preferences tab page
// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::ImpPDFTabViewerPage( Window* pParent,
                                          const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, PDFFilterResId( RID_PDF_TAB_VPREFER ), rCoreSet ),

    maFlWindowOptions( this, PDFFilterResId( FL_WINOPT ) ),
    maCbResWinInit( this, PDFFilterResId( CB_WNDOPT_RESINIT ) ),
    maCbCenterWindow( this, PDFFilterResId( CB_WNDOPT_CNTRWIN ) ),
    maCbOpenFullScreen( this, PDFFilterResId( CB_WNDOPT_OPNFULL ) ),
    maCbDispDocTitle( this, PDFFilterResId( CB_DISPDOCTITLE ) ),

    maFlUIOptions( this, PDFFilterResId( FL_USRIFOPT ) ),
    maCbHideViewerMenubar( this, PDFFilterResId( CB_UOP_HIDEVMENUBAR ) ),
    maCbHideViewerToolbar( this, PDFFilterResId( CB_UOP_HIDEVTOOLBAR ) ),
    maCbHideViewerWindowControls( this, PDFFilterResId( CB_UOP_HIDEVWINCTRL ) ),
    maFlTransitions( this, PDFFilterResId( FL_TRANSITIONS ) ),
    maCbTransitionEffects( this, PDFFilterResId( CB_TRANSITIONEFFECTS ) ),
    mbIsPresentation( sal_True ),
    maFlBookmarks( this, PDFFilterResId( FL_BOOKMARKS ) ),
    maRbAllBookmarkLevels( this, PDFFilterResId( RB_ALLBOOKMARKLEVELS ) ),
    maRbVisibleBookmarkLevels( this, PDFFilterResId( RB_VISIBLEBOOKMARKLEVELS ) ),
    maNumBookmarkLevels( this, PDFFilterResId( NUM_BOOKMARKLEVELS ) )
{
    FreeResource();
    maRbAllBookmarkLevels.SetToggleHdl( LINK( this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl ) );
    maRbVisibleBookmarkLevels.SetToggleHdl( LINK( this, ImpPDFTabViewerPage, ToggleRbBookmarksHdl ) );
}

// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabViewerPage, ToggleRbBookmarksHdl, void*, )
{
    maNumBookmarkLevels.Enable( maRbVisibleBookmarkLevels.IsChecked() );
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
    paParent->mbHideViewerMenubar = maCbHideViewerMenubar.IsChecked();
    paParent->mbHideViewerToolbar = maCbHideViewerToolbar.IsChecked( );
    paParent->mbHideViewerWindowControls = maCbHideViewerWindowControls.IsChecked();
    paParent->mbResizeWinToInit = maCbResWinInit.IsChecked();
    paParent->mbOpenInFullScreenMode = maCbOpenFullScreen.IsChecked();
    paParent->mbCenterWindow = maCbCenterWindow.IsChecked();
    paParent->mbDisplayPDFDocumentTitle = maCbDispDocTitle.IsChecked();
    paParent->mbUseTransitionEffects = maCbTransitionEffects.IsChecked();
    paParent->mnOpenBookmarkLevels = maRbAllBookmarkLevels.IsChecked() ?
                                     -1 : static_cast<sal_Int32>(maNumBookmarkLevels.GetValue());
}

// -----------------------------------------------------------------------------
void ImpPDFTabViewerPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    maCbHideViewerMenubar.Check( paParent->mbHideViewerMenubar );
    maCbHideViewerToolbar.Check( paParent->mbHideViewerToolbar );
    maCbHideViewerWindowControls.Check( paParent->mbHideViewerWindowControls );

    maCbResWinInit.Check( paParent->mbResizeWinToInit );
    maCbOpenFullScreen.Check( paParent->mbOpenInFullScreenMode );
    maCbCenterWindow.Check( paParent->mbCenterWindow );
    maCbDispDocTitle.Check( paParent->mbDisplayPDFDocumentTitle );
    mbIsPresentation = paParent->mbIsPresentation;
    maCbTransitionEffects.Check( paParent->mbUseTransitionEffects );
    maCbTransitionEffects.Enable( mbIsPresentation );
    if( paParent->mnOpenBookmarkLevels < 0 )
    {
        maRbAllBookmarkLevels.Check( TRUE );
        maNumBookmarkLevels.Enable( FALSE );
    }
    else
    {
        maRbVisibleBookmarkLevels.Check( TRUE );
        maNumBookmarkLevels.Enable( TRUE );
        maNumBookmarkLevels.SetValue( paParent->mnOpenBookmarkLevels );
    }
}

////////////////////////////////////////////////////////
// The Security preferences tab page
// -----------------------------------------------------------------------------
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage( Window* i_pParent,
                                              const SfxItemSet& i_rCoreSet ) :
    SfxTabPage( i_pParent, PDFFilterResId( RID_PDF_TAB_SECURITY ), i_rCoreSet ),
    maPbUserPwd( this, PDFFilterResId( BTN_USER_PWD ) ),
    maFtUserPwd( this, PDFFilterResId( FT_USER_PWD ) ),
    maUserPwdSet( PDFFilterResId( STR_USER_PWD_SET ) ),
    maUserPwdUnset( PDFFilterResId( STR_USER_PWD_UNSET ) ),

    maPbOwnerPwd( this,	PDFFilterResId( BTN_OWNER_PWD ) ),
    maFtOwnerPwd( this, PDFFilterResId( FT_OWNER_PWD ) ),
    maOwnerPwdSet( PDFFilterResId( STR_OWNER_PWD_SET ) ),
    maOwnerPwdUnset( PDFFilterResId( STR_OWNER_PWD_UNSET ) ),

    maFlPrintPermissions( this, PDFFilterResId( FL_PRINT_PERMISSIONS ) ),
    maRbPrintNone( this, PDFFilterResId( RB_PRINT_NONE ) ),
    maRbPrintLowRes( this, PDFFilterResId( RB_PRINT_LOWRES ) ),
    maRbPrintHighRes( this, PDFFilterResId( RB_PRINT_HIGHRES ) ),

    maFlChangesAllowed( this, PDFFilterResId( FL_CHANGES_ALLOWED ) ),
    maRbChangesNone( this, PDFFilterResId( RB_CHANGES_NONE ) ),
    maRbChangesInsDel( this, PDFFilterResId( RB_CHANGES_INSDEL ) ),
    maRbChangesFillForm( this, PDFFilterResId( RB_CHANGES_FILLFORM ) ),
    maRbChangesComment( this, PDFFilterResId( RB_CHANGES_COMMENT ) ),
    maRbChangesAnyNoCopy( this, PDFFilterResId( RB_CHANGES_ANY_NOCOPY ) ),

    maCbEnableCopy( this, PDFFilterResId( CB_ENDAB_COPY ) ),
    maCbEnableAccessibility( this, PDFFilterResId( CB_ENAB_ACCESS ) ),

    msUserPwdTitle( PDFFilterResId( STR_PDF_EXPORT_UDPWD ) ),

    msOwnerPwdTitle( PDFFilterResId( STR_PDF_EXPORT_ODPWD ) )
{
    maUserPwdSet.Append( sal_Unicode( '\n' ) );
    maUserPwdSet.Append( String( PDFFilterResId( STR_USER_PWD_ENC ) ) );
    
    maUserPwdUnset.Append( sal_Unicode( '\n' ) );
    maUserPwdUnset.Append( String( PDFFilterResId( STR_USER_PWD_UNENC ) ) );
    
    maOwnerPwdSet.Append( sal_Unicode( '\n' ) );
    maOwnerPwdSet.Append( String( PDFFilterResId( STR_OWNER_PWD_REST ) ) );
    
    maOwnerPwdUnset.Append( sal_Unicode( '\n' ) );
    maOwnerPwdUnset.Append( String( PDFFilterResId( STR_OWNER_PWD_UNREST ) ) );
    
    FreeResource();
    
    maFtUserPwd.SetText( maUserPwdUnset );
    maFtOwnerPwd.SetText( maOwnerPwdUnset );

    // pb: #i91991# maRbChangesComment double-spaced if necessary
    Size aSize = maRbChangesComment.GetSizePixel();
    Size aMinSize = maRbChangesComment.CalcMinimumSize();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Size aNewSize = maRbChangesFillForm.GetSizePixel();
        long nDelta = aSize.Height() - aNewSize.Height();
        maRbChangesComment.SetSizePixel( aNewSize );
        Window* pWins[] =
            { &maRbChangesAnyNoCopy, &maCbEnableCopy, &maCbEnableAccessibility, NULL };
        Window** pCurrent = pWins;
        while ( *pCurrent )
        {
            Point aNewPos = (*pCurrent)->GetPosPixel();
            aNewPos.Y() -= nDelta;
            (*pCurrent++)->SetPosPixel( aNewPos );
        }
    }
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
    paParent->mbEncrypt = (msUserPassword.Len() > 0);
    if( paParent->mbEncrypt )
        paParent->msUserPassword = msUserPassword;

    paParent->mbRestrictPermissions = (msOwnerPassword.Len() > 0);
    if( msOwnerPassword.Len() > 0 )
        paParent->msOwnerPassword = msOwnerPassword;

//verify print status
    paParent->mnPrint = 0;
    if( maRbPrintLowRes.IsChecked() )
        paParent->mnPrint = 1;
    else if( maRbPrintHighRes.IsChecked() )
        paParent->mnPrint = 2;

//verify changes permitted
    paParent->mnChangesAllowed = 0;

    if( maRbChangesInsDel.IsChecked() )
        paParent->mnChangesAllowed = 1;
    else if( maRbChangesFillForm.IsChecked() )
        paParent->mnChangesAllowed = 2;
    else if( maRbChangesComment.IsChecked() )
        paParent->mnChangesAllowed = 3;
    else if( maRbChangesAnyNoCopy.IsChecked() )
        paParent->mnChangesAllowed = 4;

    paParent->mbCanCopyOrExtract = maCbEnableCopy.IsChecked();
    paParent->mbCanExtractForAccessibility = maCbEnableAccessibility.IsChecked();
}


// -----------------------------------------------------------------------------
void ImpPDFTabSecurityPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    maPbUserPwd.SetClickHdl( LINK( this, ImpPDFTabSecurityPage, ClickmaPbUserPwdHdl ) );

    maPbOwnerPwd.SetClickHdl( LINK( this, ImpPDFTabSecurityPage, ClickmaPbOwnerPwdHdl ) );

    switch( paParent->mnPrint )
    {
    default:
    case 0:
        maRbPrintNone.Check();
        break;
    case 1:
        maRbPrintLowRes.Check();
        break;
    case 2:
        maRbPrintHighRes.Check();
        break;
    };

    switch( paParent->mnChangesAllowed )
    {
    default:
    case 0:
        maRbChangesNone.Check();
        break;
    case 1:
        maRbChangesInsDel.Check();
        break;
    case 2:
        maRbChangesFillForm.Check();
        break;
    case 3:
        maRbChangesComment.Check();
        break;
    case 4:
        maRbChangesAnyNoCopy.Check();
        break;
    };

    maCbEnableCopy.Check( paParent->mbCanCopyOrExtract );
    maCbEnableAccessibility.Check( paParent->mbCanExtractForAccessibility );

// set the status of this windows, according to the PDFA selection
    enablePermissionControls();
    
    if( paParent && paParent->GetTabPage( RID_PDF_TAB_GENER ) )
        ImplPDFASecurityControl(
            !( ( ImpPDFTabGeneralPage* )paParent->GetTabPage( RID_PDF_TAB_GENER ) )->IsPdfaSelected() );
}

//method common to both the password entry procedures
void ImpPDFTabSecurityPage::ImplPwdPushButton( const String & i_rDlgTitle, String & io_rDestPassword )
{
// string needed: dialog title, message box text, depending on the button clicked
    SfxPasswordDialog aPwdDialog( this );
    aPwdDialog.SetMinLen( 0 );
    aPwdDialog.ShowExtras( SHOWEXTRAS_CONFIRM );
    aPwdDialog.SetText( i_rDlgTitle );
    aPwdDialog.AllowAsciiOnly();
    if( aPwdDialog.Execute() == RET_OK )  //OK issued get password and set it
        io_rDestPassword = aPwdDialog.GetPassword();
    enablePermissionControls();
}

IMPL_LINK( ImpPDFTabSecurityPage, ClickmaPbUserPwdHdl, void*, EMPTYARG )
{
    ImplPwdPushButton(msUserPwdTitle, msUserPassword );
    return 0;
}

IMPL_LINK( ImpPDFTabSecurityPage, ClickmaPbOwnerPwdHdl, void*, EMPTYARG )
{
    ImplPwdPushButton( msOwnerPwdTitle, msOwnerPassword );

    return 0;
}

void ImpPDFTabSecurityPage::enablePermissionControls()
{
    maFtUserPwd.SetText( (msUserPassword.Len() > 0 && IsEnabled()) ? maUserPwdSet : maUserPwdUnset );
    
    sal_Bool bLocalEnable = (msOwnerPassword.Len() > 0) && IsEnabled();
    
    maFtOwnerPwd.SetText( bLocalEnable ? maOwnerPwdSet : maOwnerPwdUnset );

    maFlPrintPermissions.Enable( bLocalEnable );
    maRbPrintNone.Enable( bLocalEnable );
    maRbPrintLowRes.Enable( bLocalEnable );
    maRbPrintHighRes.Enable( bLocalEnable );

    maFlChangesAllowed.Enable( bLocalEnable );
    maRbChangesNone.Enable( bLocalEnable );
    maRbChangesInsDel.Enable( bLocalEnable );
    maRbChangesFillForm.Enable( bLocalEnable );
    maRbChangesComment.Enable( bLocalEnable );
    maRbChangesAnyNoCopy.Enable( bLocalEnable );

    maCbEnableCopy.Enable( bLocalEnable );
    maCbEnableAccessibility.Enable( bLocalEnable );
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
    SfxTabPage( pParent, PDFFilterResId( RID_PDF_TAB_LINKS ), rCoreSet ),

    maCbExprtBmkrToNmDst( this, PDFFilterResId( CB_EXP_BMRK_TO_DEST ) ),
    maCbOOoToPDFTargets( this,  PDFFilterResId( CB_CNV_OOO_DOCTOPDF ) ),
     maCbExportRelativeFsysLinks( this, PDFFilterResId( CB_ENAB_RELLINKFSYS ) ),

    maFlDefaultTitle( this,  PDFFilterResId( FL_DEFAULT_LINK_ACTION ) ),
    maRbOpnLnksDefault( this, PDFFilterResId( CB_VIEW_PDF_DEFAULT ) ),
    mbOpnLnksDefaultUserState( sal_False ),
    maRbOpnLnksLaunch( this, PDFFilterResId( CB_VIEW_PDF_APPLICATION ) ),
    mbOpnLnksLaunchUserState( sal_False ),
    maRbOpnLnksBrowser( this,  PDFFilterResId( CB_VIEW_PDF_BROWSER ) ),
    mbOpnLnksBrowserUserState( sal_False )
{
    FreeResource();

    // pb: #i91991# checkboxes only double-spaced if necessary
    long nDelta = 0;
    Size aSize = maCbExprtBmkrToNmDst.GetSizePixel();
    Size aMinSize = maCbExprtBmkrToNmDst.CalcMinimumSize();
    long nLineHeight =
        maCbExprtBmkrToNmDst.LogicToPixel( Size( 10, 10 ), MAP_APPFONT ).Height();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Size aNewSize( aSize.Width(), nLineHeight );
        nDelta += ( aSize.Height() - nLineHeight );
        maCbExprtBmkrToNmDst.SetSizePixel( aNewSize );
        Point aNewPos = maCbOOoToPDFTargets.GetPosPixel();
        aNewPos.Y() -= nDelta;
        maCbOOoToPDFTargets.SetPosPixel( aNewPos );
    }

    aSize = maCbOOoToPDFTargets.GetSizePixel();
    aMinSize = maCbOOoToPDFTargets.CalcMinimumSize();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Size aNewSize( aSize.Width(), nLineHeight );
        nDelta += ( aSize.Height() - nLineHeight );
        maCbOOoToPDFTargets.SetSizePixel( aNewSize );
        Point aNewPos = maCbExportRelativeFsysLinks.GetPosPixel();
        aNewPos.Y() -= nDelta;
        maCbExportRelativeFsysLinks.SetPosPixel( aNewPos );
    }

    aSize = maCbExportRelativeFsysLinks.GetSizePixel();
    aMinSize = maCbExportRelativeFsysLinks.CalcMinimumSize();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Size aNewSize( aSize.Width(), nLineHeight );
        nDelta += ( aSize.Height() - nLineHeight );
        maCbExportRelativeFsysLinks.SetSizePixel( aNewSize );
    }

    if ( nDelta > 0 )
    {
        Window* pWins[] =
            { &maFlDefaultTitle, &maRbOpnLnksDefault, &maRbOpnLnksLaunch, &maRbOpnLnksBrowser, NULL };
        Window** pCurrent = pWins;
        while ( *pCurrent )
        {
            Point aNewPos = (*pCurrent)->GetPosPixel();
            aNewPos.Y() -= nDelta;
            (*pCurrent++)->SetPosPixel( aNewPos );
        }
    }
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
    paParent->mbExportRelativeFsysLinks = maCbExportRelativeFsysLinks.IsChecked();

    sal_Bool bIsPDFASel =  sal_False;
    if( paParent && paParent->GetTabPage( RID_PDF_TAB_GENER ) )
        bIsPDFASel = ( ( ImpPDFTabGeneralPage* )paParent->
                       GetTabPage( RID_PDF_TAB_GENER ) )->IsPdfaSelected();
// if PDF/A-1 was not selected while exiting dialog...
    if( !bIsPDFASel )
    {
// ...get the control states
        mbOpnLnksDefaultUserState = maRbOpnLnksDefault.IsChecked();
        mbOpnLnksLaunchUserState =  maRbOpnLnksLaunch.IsChecked();
        mbOpnLnksBrowserUserState = maRbOpnLnksBrowser.IsChecked();
    }
// the control states, or the saved is used
// to form the stored selection
    paParent->mnViewPDFMode = 0;
    if(	mbOpnLnksBrowserUserState )
        paParent->mnViewPDFMode = 2;
    else if( mbOpnLnksLaunchUserState )
        paParent->mnViewPDFMode = 1;

    paParent->mbConvertOOoTargets = maCbOOoToPDFTargets.IsChecked();
    paParent->mbExportBmkToPDFDestination = maCbExprtBmkrToNmDst.IsChecked();
}

// -----------------------------------------------------------------------------
void ImpPDFTabLinksPage::SetFilterConfigItem( const  ImpPDFTabDialog* paParent )
{
    maCbOOoToPDFTargets.Check( paParent->mbConvertOOoTargets );
    maCbExprtBmkrToNmDst.Check( paParent->mbExportBmkToPDFDestination );

    maRbOpnLnksDefault.SetClickHdl( LINK( this, ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl ) );
    maRbOpnLnksBrowser.SetClickHdl( LINK( this, ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl ) );

    maCbExportRelativeFsysLinks.Check( paParent->mbExportRelativeFsysLinks );
    switch( paParent->mnViewPDFMode )
    {
    default:
    case 0:
        maRbOpnLnksDefault.Check();
        mbOpnLnksDefaultUserState = sal_True;
        break;
    case 1:
        maRbOpnLnksLaunch.Check();
        mbOpnLnksLaunchUserState = sal_True;
        break;
    case 2:
        maRbOpnLnksBrowser.Check();
        mbOpnLnksBrowserUserState = sal_True;
        break;
    }
// now check the status of PDF/A selection
// and set the link action accordingly
// PDF/A-1 doesn't allow launch action on links
//
    if( paParent && paParent->GetTabPage( RID_PDF_TAB_GENER ) )
        ImplPDFALinkControl(
            !( ( ImpPDFTabGeneralPage* )paParent->
               GetTabPage( RID_PDF_TAB_GENER ) )->maCbPDFA1b.IsChecked() );
}

// -----------------------------------------------------------------------------
// called from general tab, with PDFA/1 selection status
// retrieves/store the status of Launch action selection
void ImpPDFTabLinksPage::ImplPDFALinkControl( sal_Bool bEnableLaunch )
{
// set the value and position of link type selection
    if( bEnableLaunch )
    {
        maRbOpnLnksLaunch.Enable();
//restore user state with no PDF/A-1 selected
        maRbOpnLnksDefault.Check( mbOpnLnksDefaultUserState );
        maRbOpnLnksLaunch.Check( mbOpnLnksLaunchUserState );
        maRbOpnLnksBrowser.Check( mbOpnLnksBrowserUserState );
    }
    else
    {
//save user state with no PDF/A-1 selected
        mbOpnLnksDefaultUserState = maRbOpnLnksDefault.IsChecked();
        mbOpnLnksLaunchUserState = maRbOpnLnksLaunch.IsChecked();
        mbOpnLnksBrowserUserState = maRbOpnLnksBrowser.IsChecked();
        maRbOpnLnksLaunch.Enable( sal_False );
        if( mbOpnLnksLaunchUserState )
            maRbOpnLnksBrowser.Check();
    }
}

// -----------------------------------------------------------------------------
// reset the memory of Launch action present
// when PDF/A-1 was requested
IMPL_LINK( ImpPDFTabLinksPage, ClickRbOpnLnksDefaultHdl, void*, EMPTYARG )
{
    mbOpnLnksDefaultUserState = maRbOpnLnksDefault.IsChecked();
    mbOpnLnksLaunchUserState = maRbOpnLnksLaunch.IsChecked();
    mbOpnLnksBrowserUserState = maRbOpnLnksBrowser.IsChecked();
    return 0;
}

// -----------------------------------------------------------------------------
// reset the memory of a launch action present
// when PDF/A-1 was requested
IMPL_LINK( ImpPDFTabLinksPage, ClickRbOpnLnksBrowserHdl, void*, EMPTYARG )
{
    mbOpnLnksDefaultUserState = maRbOpnLnksDefault.IsChecked();
    mbOpnLnksLaunchUserState = maRbOpnLnksLaunch.IsChecked();
    mbOpnLnksBrowserUserState = maRbOpnLnksBrowser.IsChecked();
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
            USHORT nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_PDFA_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_TRANSP_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Omitted_PDF13:
        {
            USHORT nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_VERSION_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_TRANSP_VERSION ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_FormAction_Omitted_PDFA:
        {
            USHORT nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_FORMACTION_PDFA_SHORT ) ),
                                                aWarnImg );
            maErrors.SetEntryData( nPos, new String( PDFFilterResId( STR_WARN_FORMACTION_PDFA ) ) );
        }
        break;
        case vcl::PDFWriter::Warning_Transparency_Converted:
        {
            USHORT nPos = maErrors.InsertEntry( String( PDFFilterResId( STR_WARN_TRANSP_CONVERTED_SHORT ) ),
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
    for( USHORT n = 0; n < maErrors.GetEntryCount(); n++ )
        delete (String*)maErrors.GetEntryData( n );
}

IMPL_LINK( ImplErrorDialog, SelectHdl, ListBox*, EMPTYARG )
{
    String* pStr = reinterpret_cast<String*>(maErrors.GetEntryData( maErrors.GetSelectEntryPos() ));
    maExplanation.SetText( pStr ? *pStr : String() );
    return 0;
}
