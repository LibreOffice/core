/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layeroptionsdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 13:38:55 $
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#include "strings.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "layeroptionsdlg.hxx"
#include "layeroptionsdlg.hrc"

SdInsertLayerDlg::SdInsertLayerDlg( Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr )
: ModalDialog( pWindow, SdResId( DLG_INSERT_LAYER ) )
, maFtName( this, SdResId( FT_NAME ) )
, maEdtName( this, SdResId( EDT_NAME ) )
, maFtTitle( this, SdResId( FT_TITLE ) )
, maEdtTitle( this, SdResId( EDT_TITLE ) )
, maFtDesc( this, SdResId( FT_DESCRIPTION ) )
, maEdtDesc( this, SdResId( EDT_DESCRIPTION ) )
, maCbxVisible( this, SdResId( CBX_VISIBLE ) )
, maCbxPrintable( this, SdResId( CBX_PRINTABLE ) )
, maCbxLocked( this, SdResId( CBX_LOCKED ) )
, maFixedLine( this, SdResId( FL_SEPARATOR_B ) )
, maBtnHelp( this, SdResId( BTN_HELP ) )
, maBtnOK( this, SdResId( BTN_OK ) )
, maBtnCancel( this, SdResId( BTN_CANCEL ) )
, mrOutAttrs( rInAttrs )
{
    FreeResource();

    SetText( aStr );

    maEdtName.SetText( ( ( const SdAttrLayerName& ) mrOutAttrs.Get( ATTR_LAYER_NAME ) ).GetValue() );
    maEdtTitle.SetText( ( ( const SdAttrLayerTitle& ) mrOutAttrs.Get( ATTR_LAYER_TITLE ) ).GetValue() );
    maEdtDesc.SetText( ( ( const SdAttrLayerDesc& ) mrOutAttrs.Get( ATTR_LAYER_DESC ) ).GetValue() );
    maCbxVisible.Check( ( ( const SdAttrLayerVisible& ) mrOutAttrs.Get( ATTR_LAYER_VISIBLE ) ).GetValue() );
    maCbxPrintable.Check( ( ( const SdAttrLayerPrintable& ) mrOutAttrs.Get( ATTR_LAYER_PRINTABLE ) ).GetValue() );
    maCbxLocked.Check( ( ( const SdAttrLayerLocked& ) mrOutAttrs.Get( ATTR_LAYER_LOCKED ) ).GetValue() );

    if( !bDeletable )
    {
        maFtName.Disable();
        maEdtName.Disable();
    }
}

void SdInsertLayerDlg::GetAttr( SfxItemSet& rAttrs )
{
    rAttrs.Put( SdAttrLayerName( maEdtName.GetText() ) );
    rAttrs.Put( SdAttrLayerTitle( maEdtTitle.GetText() ) );
    rAttrs.Put( SdAttrLayerDesc( maEdtDesc.GetText() ) );
    rAttrs.Put( SdAttrLayerVisible( maCbxVisible.IsChecked() ) );
    rAttrs.Put( SdAttrLayerPrintable( maCbxPrintable.IsChecked() ) );
    rAttrs.Put( SdAttrLayerLocked( maCbxLocked.IsChecked() ) );
}
