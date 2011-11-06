/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif
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
