 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impdialog.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-01 10:20:30 $
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

ImpPDFDialog::ImpPDFDialog( Window* pParent, ResMgr& rResMgr, Sequence< PropertyValue >& rFilterData, const Reference< XComponent >& rxDoc ) :
    ModalDialog( pParent, ResId( RID_PDF_EXPORT_DLG, &rResMgr ) ),
    maBtnOK( this, ResId( BT_OK ) ),
    maBtnCancel( this, ResId( BT_CANCEL ) ),
    maBtnHelp( this, ResId( BT_HELP ) ),
    maFlPages( this, ResId( FL_PAGES ) ),
    maRbAll( this, ResId( RB_ALL ) ),
    maRbRange( this, ResId( RB_RANGE ) ),
    maRbSelection( this, ResId( RB_SELECTION ) ),
    maEdPages( this, ResId( ED_PAGES ) ),
    maFlCompression( this, ResId( FL_IMAGES ) ),
    maRbLosslessCompression( this, ResId( RB_LOSSLESSCOMPRESSION ) ),
    maRbJPEGCompression( this, ResId( RB_JPEGCOMPRESSION ) ),
    maFtQuality( this, ResId( FT_QUALITY ) ),
    maNfQuality( this, ResId( NF_QUALITY ) ),
    maCbReduceImageResolution( this, ResId( CB_REDUCEIMAGERESOLUTION ) ),
    maCoReduceImageResolution( this, ResId( CO_REDUCEIMAGERESOLUTION ) ),
    maFlGeneral( this, ResId( FL_GENERAL ) ),
    maCbTaggedPDF( this, ResId( CB_TAGGEDPDF ) ),
    maCbExportNotes( this, ResId( CB_EXPORTNOTES ) ),
    maCbTransitionEffects( this, ResId( CB_TRANSITIONEFFECTS ) ),
    maFtFormsFormat( this, ResId( FT_FORMSFORMAT ) ),
    maLbFormsFormat( this, ResId( LB_FORMSFORMAT ) ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/PDF/Export/" ) ), &rFilterData ),
    mbIsPresentation( sal_False )
{
    FreeResource();
    maRbRange.SetToggleHdl( LINK( this, ImpPDFDialog, TogglePagesHdl ) );

    maRbAll.Check();
    TogglePagesHdl( NULL );

    maNfQuality.SetUnit( FUNIT_PERCENT );
    maNfQuality.SetMin( 1, FUNIT_PERCENT );
    maNfQuality.SetMax( 100, FUNIT_PERCENT );

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
    sal_Bool bHasSelection = maSelection.hasValue();
    if ( bHasSelection )
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
                    bHasSelection = sal_False;
                else if ( nLen == 1 )
                {
                    Reference< text::XTextRange > xTextRange( xIndexAccess->getByIndex( 0 ), UNO_QUERY );
                    if ( xTextRange.is() && ( xTextRange->getString().getLength() == 0 ) )
                        bHasSelection = sal_False;
                }
            }
        }
    }
    maRbSelection.Enable( bHasSelection );


    // check if source document is a presentation
    try
    {
        Reference< XServiceInfo > xInfo( rxDoc, UNO_QUERY );
        if ( xInfo.is() )
        {
            if ( xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) ) )
                mbIsPresentation = sal_True;
        }
    }
    catch( RuntimeException )
    {
    }
    maCbTransitionEffects.Enable( mbIsPresentation );


//  SJ: Dont know if there are Notes available also for writer.
//  maCbExportNotes.Enable( bIsPresentation );


    maRbLosslessCompression.SetToggleHdl( LINK( this, ImpPDFDialog, ToggleCompressionHdl ) );
    const sal_Bool  bUseLosslessCompression = maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), sal_False );
    if ( bUseLosslessCompression )
        maRbLosslessCompression.Check();
    else
        maRbJPEGCompression.Check();
    maNfQuality.SetValue( maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), 90 ), FUNIT_PERCENT );
    maNfQuality.Enable( bUseLosslessCompression == sal_False );

    maCbReduceImageResolution.SetToggleHdl( LINK( this, ImpPDFDialog, ToggleReduceImageResolutionHdl ) );
    const sal_Bool  bReduceImageResolution = maConfigItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), sal_False );
    maCbReduceImageResolution.Check( bReduceImageResolution );
    String aStrRes( String::CreateFromInt32( maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), 300 ) ) );
    aStrRes.Append( String( RTL_CONSTASCII_USTRINGPARAM( " DPI" ) ) );
    maCoReduceImageResolution.SetText( aStrRes );
    maCoReduceImageResolution.Enable( bReduceImageResolution );

    maCbTaggedPDF.Check( maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), sal_False ) );

    if ( mbIsPresentation )
        maCbExportNotes.Check( maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages"  ) ), sal_False ) );
    else
        maCbExportNotes.Check( maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes"  ) ), sal_True ) );

    maCbTransitionEffects.Check( maConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects"  ) ), sal_True ) );

    sal_Int32 nFormsType = maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), 0 );
    if ( ( nFormsType < 0 ) || ( nFormsType > 3 ) )
        nFormsType = 0;
    maLbFormsFormat.SelectEntryPos( (sal_uInt16)nFormsType );
}

// -----------------------------------------------------------------------------

ImpPDFDialog::~ImpPDFDialog()
{
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > ImpPDFDialog::GetFilterData()
{
    // updating the FilterData sequence and storing FilterData to configuration
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), maRbLosslessCompression.IsChecked() );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), maNfQuality.GetValue() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), maCbReduceImageResolution.IsChecked() );
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), maCoReduceImageResolution.GetText().ToInt32() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), maCbTaggedPDF.IsChecked() );
    if ( mbIsPresentation )
        maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) ), maCbExportNotes.IsChecked() );
    else
        maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes" ) ), maCbExportNotes.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects" ) ), maCbTransitionEffects.IsChecked() );
    /*
    * FIXME: the entries are only implicitly defined by the resource file. Should there
    * ever be an additional form submit format this could get invalid.
    */
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), maLbFormsFormat.GetSelectEntryPos() );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    aRet.realloc( aRet.getLength() + 1 );

    if( maRbRange.IsChecked() )
    {
        aRet[ aRet.getLength() - 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) );
        aRet[ aRet.getLength() - 1 ].Value <<= OUString( maEdPages.GetText() );
    }
    else if( maRbSelection.IsChecked() )
    {
        aRet[ aRet.getLength() - 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) );
        aRet[ aRet.getLength() - 1 ].Value <<= maSelection;
    }
    return aRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpPDFDialog, TogglePagesHdl, void*, p )
{
    maEdPages.Enable( maRbRange.IsChecked() );
    maEdPages.SetReadOnly( !maRbRange.IsChecked() );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpPDFDialog, ToggleCompressionHdl, void*, p )
{
    maNfQuality.Enable( maRbJPEGCompression.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpPDFDialog, ToggleReduceImageResolutionHdl, void*, p )
{
    maCoReduceImageResolution.Enable( maCbReduceImageResolution.IsChecked() );
    return 0;
}
