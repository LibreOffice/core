/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"

#include <svl/itemset.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
