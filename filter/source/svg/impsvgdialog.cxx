 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impsvgdialog.cxx,v $
 *
 *  $Revision: 1.1.2.3 $
 *
 *  last change: $Author: ka $ $Date: 2007/06/15 14:36:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "impsvgdialog.hxx"
#include <cstdio>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

inline sal_Int32 implMap( Window& /*rWnd*/, sal_Int32 nVal )
{
    //return( rWnd.LogicToLogic( Size( nVal, nVal ) ).Height(), MAP_APPFONT, MAP_APPFONT );
    return( nVal << 1 );
}

// ----------------
// - ImpSVGDialog -
// ----------------

ImpSVGDialog::ImpSVGDialog( Window* pParent/*, ResMgr& rResMgr*/, Sequence< PropertyValue >& rFilterData ) :
    ModalDialog( pParent/*KA, ResId( DLG_OPTIONS, &rResMgr*/ ),
    maFI( this ),
    maCBTinyProfile( this ),
    maCBEmbedFonts( this ),
    maCBUseNativeDecoration( this ),
    maBTOK( this, WB_DEF_OK ),
    maBTCancel( this ),
    maBTHelp( this ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( SVG_EXPORTFILTER_CONFIGPATH ) ), &rFilterData ),
    mbOldNativeDecoration( sal_False )
{
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "SVG Export Options" ) ) );
    SetOutputSizePixel( Size( implMap( *this, 177 ), implMap( *this, 77 ) ) );

    maFI.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Export" ) ) );
    maFI.SetPosSizePixel( Point( implMap( *this, 6 ), implMap( *this, 3 ) ),
                                 Size( implMap( *this, 165 ), implMap( *this, 8 ) ) );

    maCBTinyProfile.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Use SVG Tiny profile" ) ) );
    maCBTinyProfile.SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 14 ) ),
                                     Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBEmbedFonts.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Embed fonts" ) ) );
    maCBEmbedFonts.SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 27 ) ),
                                    Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBUseNativeDecoration.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Use SVG native text decoration" ) ) );
    maCBUseNativeDecoration.SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 41 ) ),
                                             Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBTinyProfile.Check( maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_TINYPROFILE ) ), sal_False ) );
    maCBEmbedFonts.Check( maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_EMBEDFONTS ) ), sal_True ) );
    maCBUseNativeDecoration.Check( maConfigItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_NATIVEDECORATION ) ), sal_True ) );

    maBTOK.SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 57 ) ),
                            Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );
    maBTCancel.SetPosSizePixel( Point( implMap( *this, 65 ), implMap( *this, 57 ) ),
                                Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );
    maBTHelp.SetPosSizePixel( Point( implMap( *this, 121 ), implMap( *this, 57 ) ),
                              Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );

    maCBTinyProfile.SetToggleHdl( LINK( this, ImpSVGDialog, OnToggleCheckbox ) );
    OnToggleCheckbox( &maCBTinyProfile );

    maFI.Show();

    maCBTinyProfile.Show();
    maCBEmbedFonts.Show();
    maCBUseNativeDecoration.Show();

    maBTOK.Show();
    maBTCancel.Show();
    maBTHelp.Show();
}

// -----------------------------------------------------------------------------

ImpSVGDialog::~ImpSVGDialog()
{
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > ImpSVGDialog::GetFilterData()
{
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_TINYPROFILE ) ), maCBTinyProfile.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_EMBEDFONTS ) ), maCBEmbedFonts.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( SVG_PROP_NATIVEDECORATION ) ), maCBUseNativeDecoration.IsChecked() );

    return( maConfigItem.GetFilterData() );
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpSVGDialog, OnToggleCheckbox, CheckBox*, pBox )
{
    if( pBox == &maCBTinyProfile )
    {
        if( pBox->IsChecked() )
        {
            mbOldNativeDecoration = maCBUseNativeDecoration.IsChecked();

            maCBUseNativeDecoration.Check( sal_False );
            maCBUseNativeDecoration.Disable();
        }
        else
        {
            maCBUseNativeDecoration.Enable();
            maCBUseNativeDecoration.Check( mbOldNativeDecoration );
        }
    }

    return 0;
}
