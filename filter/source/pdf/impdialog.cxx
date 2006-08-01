 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impdialog.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:28:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "impdialog.hxx"
#include "impdialog.hrc"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

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
                                  ResMgr& rResMgr,
                                  Sequence< PropertyValue >& rFilterData,
                                  const Reference< XComponent >& rxDoc ) :
    SfxTabDialog( pParent, ResId( RID_PDF_EXPORT_DLG, &rResMgr ), 0, FALSE, 0 ),
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
    mbExportNotesBoth( sal_True ),
    mbUseTransitionEffects( sal_False ),
    mbIsSkipEmptyPages( sal_False ),
    mnFormsType( 0 ),

    mbHideViewerToolbar( sal_False ),
    mbHideViewerMenubar( sal_False ),
    mbHideViewerWindowControls( sal_False ),
    mbResizeWinToInit( sal_False ),
    mbCenterWindow( sal_False ),
    mbOpenInFullScreenMode( sal_False ),
    mbDisplayPDFDocumentTitle( sal_False ),
    mnMagnification( 0 ),
    mnInitialView( 0 ),
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

    mbSelectionIsChecked( sal_False )
{
    FreeResource();
    mprResMgr = &rResMgr;
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
            if ( xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ) ) ) )
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

    mbUseTaggedPDF = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), sal_False );
    if ( mbIsPresentation )
        mbExportNotesBoth = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages"  ) ), sal_False );
    else
        mbExportNotesBoth = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes"  ) ), sal_True );

    mbUseTransitionEffects = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects"  ) ), sal_True );
    mbIsSkipEmptyPages = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "IsSkipEmptyPages"  ) ), sal_False );

    mnFormsType = maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), 0 );
    if ( ( mnFormsType < 0 ) || ( mnFormsType > 3 ) )
        mnFormsType = 0;

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
    mnPageLayout = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayout" ) ), 0 );
    mbFirstPageLeft = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPageOnLeft" ) ), sal_False );

//prepare values for the security tab page
    mnPrint = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Printing" ) ), 2 );
    mnChangesAllowed = maConfigItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Changes" ) ), 4 );
    mbCanCopyOrExtract = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableCopyingOfContent" ) ), sal_True );
    mbCanExtractForAccessibility = maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableTextAccessForAccessibilityTools" ) ), sal_True );

//queue the tab pages for later creation (created when first shown)
    AddTabPage( RID_PDF_TAB_SECURITY, ImpPDFTabSecurityPage::Create, 0 );
    AddTabPage( RID_PDF_TAB_VPREFER, ImpPDFTabViewerPage::Create, 0 );
    AddTabPage( RID_PDF_TAB_OPNFTR, ImpPDFTabOpnFtrPage::Create, 0 );

//last queued is the first to be displayed (or so it seems..)
    AddTabPage( RID_PDF_TAB_GENER, ImpPDFTabGeneralPage::Create, 0 );

//change test on the Ok button: get the relevant string from resources, update it on the button
    GetOKButton().SetText( OUString( String( ResId( STR_PDF_EXPORT, &rResMgr ) ) ) );
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

//prepare the items to be returned
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), mbUseLosslessCompression );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), mnQuality );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), mbReduceImageResolution );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), mnMaxImageResolution );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), mbUseTaggedPDF );
    if ( mbIsPresentation )
        maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) ), mbExportNotesBoth );
    else
        maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes" ) ), mbExportNotesBoth );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects" ) ), mbUseTransitionEffects );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsSkipEmptyPages" ) ), mbIsSkipEmptyPages );

    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), mnFormsType );

    if( GetTabPage( RID_PDF_TAB_VPREFER ) )
        ( ( ImpPDFTabViewerPage* )GetTabPage( RID_PDF_TAB_VPREFER ) )->GetFilterConfigItem( this );

    if( GetTabPage( RID_PDF_TAB_OPNFTR ) )
        ( ( ImpPDFTabOpnFtrPage* )GetTabPage( RID_PDF_TAB_OPNFTR ) )->GetFilterConfigItem( this );

    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerToolbar" ) ), mbHideViewerToolbar );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerMenubar" ) ), mbHideViewerMenubar );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "HideViewerWindowControls" ) ), mbHideViewerWindowControls );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ResizeWindowToInitialPage" ) ), mbResizeWinToInit );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "CenterWindow" ) ), mbCenterWindow );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenInFullScreenMode" ) ), mbOpenInFullScreenMode );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "DisplayPDFDocumentTitle" ) ), mbDisplayPDFDocumentTitle );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialView" ) ), mnInitialView );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Magnification" ) ), mnMagnification);
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayout" ) ), mnPageLayout );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPageOnLeft" ) ), mbFirstPageLeft );

    if( GetTabPage( RID_PDF_TAB_SECURITY ) )
        ( ( ImpPDFTabSecurityPage* )GetTabPage( RID_PDF_TAB_SECURITY ) )->GetFilterConfigItem( this );

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
                                            const SfxItemSet& rCoreSet,
                                            ResMgr* paResMgr ) :
    SfxTabPage( pParent, ResId( RID_PDF_TAB_GENER, paResMgr ), rCoreSet ),

    maFlPages( this, ResId( FL_PAGES, paResMgr ) ),
    maRbAll( this, ResId( RB_ALL, paResMgr) ),
    maRbRange( this, ResId( RB_RANGE, paResMgr ) ),
    maRbSelection( this, ResId( RB_SELECTION, paResMgr ) ),
    maEdPages( this, ResId( ED_PAGES, paResMgr ) ),

    maFlCompression( this, ResId( FL_IMAGES, paResMgr ) ),
    maRbLosslessCompression( this, ResId( RB_LOSSLESSCOMPRESSION, paResMgr ) ),
    maRbJPEGCompression( this, ResId( RB_JPEGCOMPRESSION, paResMgr ) ),
    maFtQuality( this, ResId( FT_QUALITY, paResMgr ) ),
    maNfQuality( this, ResId( NF_QUALITY, paResMgr ) ),
    maCbReduceImageResolution( this, ResId( CB_REDUCEIMAGERESOLUTION, paResMgr ) ),
    maCoReduceImageResolution( this, ResId( CO_REDUCEIMAGERESOLUTION, paResMgr ) ),

    maFlGeneral( this, ResId( FL_GENERAL, paResMgr ) ),
    maCbTaggedPDF( this, ResId( CB_TAGGEDPDF, paResMgr ) ),
    maCbExportNotes( this, ResId( CB_EXPORTNOTES, paResMgr ) ),
    maCbTransitionEffects( this, ResId( CB_TRANSITIONEFFECTS, paResMgr ) ),

    maFtFormsFormat( this, ResId( FT_FORMSFORMAT, paResMgr ) ),
    maLbFormsFormat( this, ResId( LB_FORMSFORMAT, paResMgr ) ),
    maCbExportEmptyPages( this, ResId( CB_EXPORTEMPTYPAGES, paResMgr ) ),
    mbIsPresentation( sal_False ),
    mbIsWriter( sal_False)
{
    mpaResMgr = paResMgr;
    FreeResource();
}

// -----------------------------------------------------------------------------
ImpPDFTabGeneralPage::~ImpPDFTabGeneralPage()
{
    delete mpaResMgr;
}

// -----------------------------------------------------------------------------
void ImpPDFTabGeneralPage::SetFilterConfigItem( const ImpPDFTabDialog* paParent )
{
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

    maCbTransitionEffects.Enable( mbIsPresentation );
    maCbExportEmptyPages.Enable( mbIsWriter );

//  SJ: Dont know if there are Notes available also for writer.
//  maCbExportNotes.Enable( paParent->mbIsPresentation );

    maRbLosslessCompression.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleCompressionHdl ) );
    const sal_Bool bUseLosslessCompression = paParent->mbUseLosslessCompression;
    if ( bUseLosslessCompression )
        maRbLosslessCompression.Check();
    else
        maRbJPEGCompression.Check();

    maNfQuality.SetValue( paParent->mnQuality, FUNIT_PERCENT );
    maNfQuality.Enable( bUseLosslessCompression == sal_False );

    maCbReduceImageResolution.SetToggleHdl( LINK( this, ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl ) );
    const sal_Bool  bReduceImageResolution = paParent->mbReduceImageResolution;
    maCbReduceImageResolution.Check( bReduceImageResolution );
    String aStrRes( String::CreateFromInt32( paParent->mnMaxImageResolution ) );
    aStrRes.Append( String( RTL_CONSTASCII_USTRINGPARAM( " DPI" ) ) );
    maCoReduceImageResolution.SetText( aStrRes );
    maCoReduceImageResolution.Enable( bReduceImageResolution );

    maCbTaggedPDF.Check( paParent->mbUseTaggedPDF );

    if ( mbIsPresentation )
        maCbExportNotes.Check( paParent->mbExportNotesBoth );
    else
        maCbExportNotes.Check( paParent->mbExportNotesBoth );

    maCbTransitionEffects.Check( paParent->mbUseTransitionEffects );
    maCbExportEmptyPages.Check( !paParent->mbIsSkipEmptyPages );

    maLbFormsFormat.SelectEntryPos( (sal_uInt16)paParent->mnFormsType );
}

// -----------------------------------------------------------------------------
void ImpPDFTabGeneralPage::GetFilterConfigItem( ImpPDFTabDialog* paParent )
{
// updating the FilterData sequence and storing FilterData to configuration
    paParent->mbUseLosslessCompression = maRbLosslessCompression.IsChecked();
    paParent->mnQuality = maNfQuality.GetValue();
    paParent->mbReduceImageResolution = maCbReduceImageResolution.IsChecked();
    paParent->mnMaxImageResolution = maCoReduceImageResolution.GetText().ToInt32();
    paParent->mbUseTaggedPDF =  maCbTaggedPDF.IsChecked();
    paParent->mbExportNotesBoth = maCbExportNotes.IsChecked();

    paParent->mbUseTransitionEffects = maCbTransitionEffects.IsChecked();
    paParent->mbIsSkipEmptyPages =  !maCbExportEmptyPages.IsChecked();

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
    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    paParent->mnFormsType = (sal_Int32) maLbFormsFormat.GetSelectEntryPos();
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabGeneralPage::Create( Window* pParent,
                                           const SfxItemSet& rAttrSet)
{
    ByteString aResMgrName( "pdffilter" );
    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    ResMgr* paResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
    return ( new  ImpPDFTabGeneralPage( pParent, rAttrSet, paResMgr ) );
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, TogglePagesHdl, void*, p )
{
    p = p; //for compiler warning
    maEdPages.Enable( maRbRange.IsChecked() );
    maEdPages.SetReadOnly( !maRbRange.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleCompressionHdl, void*, p )
{
    p = p; //for compiler warning
    maNfQuality.Enable( maRbJPEGCompression.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------
IMPL_LINK( ImpPDFTabGeneralPage, ToggleReduceImageResolutionHdl, void*, p )
{
    p = p; //for compiler warning
    maCoReduceImageResolution.Enable( maCbReduceImageResolution.IsChecked() );
    return 0;
}

/////////////////////////////////////////////////////////////////
// the option features tab page
// -----------------------------------------------------------------------------
ImpPDFTabOpnFtrPage::ImpPDFTabOpnFtrPage( Window* pParent,
                                          const SfxItemSet& rCoreSet,
                                          ResMgr* paResMgr ) :
    SfxTabPage( pParent, ResId( RID_PDF_TAB_OPNFTR, paResMgr ), rCoreSet ),

    maFlInitialView( this, ResId( FL_INITVIEW, paResMgr ) ),
    maRbOpnPageOnly( this, ResId( RB_OPNMODE_PAGEONLY, paResMgr ) ),
    maRbOpnOutline( this, ResId( RB_OPNMODE_OUTLINE, paResMgr ) ),
    maRbOpnThumbs( this, ResId( RB_OPNMODE_THUMBS, paResMgr ) ),

    maFlMagnification( this, ResId( FL_MAGNIFICATION, paResMgr ) ),
    maRbMagnDefault( this, ResId( RB_MAGNF_DEFAULT, paResMgr ) ),
    maRbMagnFitWin( this, ResId( RB_MAGNF_WIND, paResMgr ) ),
    maRbMagnFitWidth( this, ResId( RB_MAGNF_WIDTH, paResMgr ) ),
    maRbMagnFitVisible( this, ResId( RB_MAGNF_VISIBLE, paResMgr ) ),

    maFlPageLayout( this, ResId( FL_PAGE_LAYOUT, paResMgr ) ),
    maRbPgLyDefault( this, ResId( RB_PGLY_DEFAULT, paResMgr ) ),
    maRbPgLySinglePage( this, ResId( RB_PGLY_SINGPG, paResMgr ) ),
    maRbPgLyContinue( this, ResId( RB_PGLY_CONT, paResMgr ) ),
    maRbPgLyContinueFacing( this, ResId( RB_PGLY_CONTFAC, paResMgr ) ),
    maCbPgLyFirstOnLeft( this, ResId( CB_PGLY_FIRSTLEFT, paResMgr ) ),
    mbUseCTLFont( sal_False )
{
    mpaResMgr = paResMgr;
    FreeResource();
}

// -----------------------------------------------------------------------------
ImpPDFTabOpnFtrPage::~ImpPDFTabOpnFtrPage()
{
    delete mpaResMgr;
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabOpnFtrPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    ByteString aResMgrName( "pdffilter" );
    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    ResMgr* paResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
    return ( new  ImpPDFTabOpnFtrPage( pParent, rAttrSet, paResMgr ) );
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
        break;
    case 1:
        maRbMagnFitWin.Check();
        break;
    case 2:
        maRbMagnFitWidth.Check();
        break;
    case 3:
        maRbMagnFitVisible.Check();
        break;
    };

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

////////////////////////////////////////////////////////
// The Viewer preferences tab page
// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::ImpPDFTabViewerPage( Window* pParent,
                                          const SfxItemSet& rCoreSet,
                                          ResMgr* paResMgr ) :
    SfxTabPage( pParent, ResId( RID_PDF_TAB_VPREFER, paResMgr ), rCoreSet ),

    maFlWindowOptions( this, ResId( FL_WINOPT, paResMgr ) ),
    maCbResWinInit( this, ResId( CB_WNDOPT_RESINIT, paResMgr ) ),
    maCbCenterWindow( this, ResId( CB_WNDOPT_CNTRWIN, paResMgr ) ),
    maCbOpenFullScreen( this, ResId( CB_WNDOPT_OPNFULL, paResMgr ) ),
    maCbDispDocTitle( this, ResId( CB_DISPDOCTITLE, paResMgr ) ),

    maFlUIOptions( this, ResId( FL_USRIFOPT, paResMgr ) ),
    maCbHideViewerMenubar( this, ResId( CB_UOP_HIDEVMENUBAR, paResMgr ) ),
    maCbHideViewerToolbar( this, ResId( CB_UOP_HIDEVTOOLBAR, paResMgr ) ),
    maCbHideViewerWindowControls( this, ResId( CB_UOP_HIDEVWINCTRL, paResMgr ) )
{
    mpaResMgr = paResMgr;
    FreeResource();
}

// -----------------------------------------------------------------------------
ImpPDFTabViewerPage::~ImpPDFTabViewerPage()
{
    delete mpaResMgr;
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabViewerPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    ByteString aResMgrName( "pdffilter" );
    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    ResMgr* paResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
    return ( new  ImpPDFTabViewerPage( pParent, rAttrSet, paResMgr ) );
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
}

////////////////////////////////////////////////////////
// The Security preferences tab page
// -----------------------------------------------------------------------------
ImpPDFTabSecurityPage::ImpPDFTabSecurityPage( Window* pParent,
                                              const SfxItemSet& rCoreSet,
                                              ResMgr* paResMgr ) :
    SfxTabPage( pParent, ResId( RID_PDF_TAB_SECURITY, paResMgr ), rCoreSet ),
    maCbEncrypt( this, ResId( CB_SEC_ENCRYPT , paResMgr ) ),

    maPbUserPwd( this, ResId( BTN_USER_PWD , paResMgr ) ),
    maFtUserPwdEmpty( this, ResId( FT_USER_PWD_EMPTY , paResMgr ) ),

    maFlPermissions( this, ResId( FL_PERMISSIONS, paResMgr ) ),
    maCbPermissions( this, ResId( CB_SEL_PERMISSIONS, paResMgr ) ),

    maPbOwnerPwd( this, ResId( BTN_OWNER_PWD , paResMgr ) ),
    maFtOwnerPwdEmpty( this, ResId( FT_OWNER_PWD_EMPTY , paResMgr ) ),

    maFlPrintPermissions( this, ResId( FL_PRINT_PERMISSIONS , paResMgr ) ),
    maRbPrintNone( this, ResId( RB_PRINT_NONE, paResMgr ) ),
    maRbPrintLowRes( this, ResId( RB_PRINT_LOWRES , paResMgr ) ),
    maRbPrintHighRes( this, ResId( RB_PRINT_HIGHRES , paResMgr ) ),

    maFlChangesAllowed( this, ResId( FL_CHANGES_ALLOWED , paResMgr ) ),
    maRbChangesNone( this, ResId( RB_CHANGES_NONE , paResMgr ) ),
    maRbChangesInsDel( this, ResId( RB_CHANGES_INSDEL , paResMgr ) ),
    maRbChangesFillForm( this, ResId( RB_CHANGES_FILLFORM , paResMgr ) ),
    maRbChangesComment( this, ResId( RB_CHANGES_COMMENT , paResMgr ) ),
    maRbChangesAnyNoCopy( this, ResId( RB_CHANGES_ANY_NOCOPY , paResMgr ) ),

    maCbEnableCopy( this, ResId( CB_ENDAB_COPY , paResMgr ) ),
    maCbEnableAccessibility( this, ResId( CB_ENAB_ACCESS , paResMgr ) ),

    msSetUserPwd( ResId( STR_PDF_EXPORT_USPWD, paResMgr ) ),
    msUserPwdTitle( ResId( STR_PDF_EXPORT_UDPWD, paResMgr ) ),

    msRestrPermissions( ResId( STR_PDF_EXPORT_CB_PERM, paResMgr ) ),
    msSetOwnerPwd( ResId( STR_PDF_EXPORT_OSPWD, paResMgr ) ),
    msOwnerPwdTitle( ResId( STR_PDF_EXPORT_ODPWD, paResMgr ) )
{
    mpaResMgr = paResMgr;
    FreeResource();
    maCbPermissions.SetText( OUString( msRestrPermissions ) );
}

// -----------------------------------------------------------------------------
ImpPDFTabSecurityPage::~ImpPDFTabSecurityPage()
{
    delete mpaResMgr;
}

// -----------------------------------------------------------------------------
SfxTabPage*  ImpPDFTabSecurityPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet)
{
    ByteString aResMgrName( "pdffilter" );
    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    ResMgr* paResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
    return ( new  ImpPDFTabSecurityPage( pParent, rAttrSet, paResMgr ) );
}

// -----------------------------------------------------------------------------
void ImpPDFTabSecurityPage::GetFilterConfigItem( ImpPDFTabDialog* paParent  )
{
    paParent->mbEncrypt = maCbEncrypt.IsChecked();

    if( paParent->mbEncrypt )
        paParent->msUserPassword = msUserPassword;

    paParent->mbRestrictPermissions = maCbPermissions.IsChecked();

    if( maCbPermissions.IsChecked() && msOwnerPassword.Len() > 0 )
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
    maPbUserPwd.SetText( OUString( msSetUserPwd ) );
    maPbUserPwd.SetClickHdl( LINK( this, ImpPDFTabSecurityPage, ClickmaPbUserPwdHdl ) );

    maPbOwnerPwd.SetText( OUString( msSetOwnerPwd ) );
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

    maCbEncrypt.SetToggleHdl( LINK( this, ImpPDFTabSecurityPage, TogglemaCbEncryptHdl ) );
    maCbEncrypt.Check( sal_False );
    TogglemaCbEncryptHdl( NULL );

    maCbPermissions.SetToggleHdl( LINK( this, ImpPDFTabSecurityPage, TogglemaCbPermissionsHdl ) );
    maCbPermissions.Check( sal_False );
    TogglemaCbPermissionsHdl( NULL );
}

IMPL_LINK( ImpPDFTabSecurityPage, TogglemaCbEncryptHdl, void*, p )
{
    p = p; //for compiler warning
    maPbUserPwd.Enable( maCbEncrypt.IsChecked() );
    return 0;
}

IMPL_LINK( ImpPDFTabSecurityPage, TogglemaCbPermissionsHdl, void*, p )
{
    p = p; //for compiler warning
//now enable the ones needed
    maPbOwnerPwd.Enable( maCbPermissions.IsChecked() );

    sal_Bool bLocalEnable = ( maCbPermissions.IsChecked() && msOwnerPassword.Len() > 0 );

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
    return 0;
}

//method common to both the password entry procedures
void ImpPDFTabSecurityPage::ImplPwdPushButton( String & sDlgTitle, String & sDestPassword, FixedText & aFixedText )
{
// string needed: dialog title, message box text, depending on the button clicked
    SfxPasswordDialog aPwdDialog( this );
    aPwdDialog.SetMinLen( 0 );
    aPwdDialog.ShowExtras( SHOWEXTRAS_CONFIRM );
    aPwdDialog.SetText( sDlgTitle );
    if( aPwdDialog.Execute() == RET_OK )  //OK issued get password and set it
        sDestPassword = aPwdDialog.GetPassword();

    if( sDestPassword.Len() == 0 )
        aFixedText.Show();
    else
        aFixedText.Hide();
}

IMPL_LINK( ImpPDFTabSecurityPage, ClickmaPbUserPwdHdl, void*, p )
{
    p = p; //for compiler warning
    ImplPwdPushButton(msUserPwdTitle, msUserPassword, maFtUserPwdEmpty);
//check if len(password) is > 0 then set button text to Set, else set to Change
    return 0;
}

IMPL_LINK( ImpPDFTabSecurityPage, ClickmaPbOwnerPwdHdl, void*, p )
{
    p = p; //for compiler warning
    ImplPwdPushButton( msOwnerPwdTitle, msOwnerPassword, maFtOwnerPwdEmpty );
    TogglemaCbPermissionsHdl( NULL );

    if( msOwnerPassword.Len() == 0 )
        maCbPermissions.Check( false );

    return 0;
}
