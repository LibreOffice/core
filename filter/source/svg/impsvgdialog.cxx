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


#include "impsvgdialog.hxx"
#include <cstdio>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

inline sal_Int32 implMap( vcl::Window& /*rWnd*/, sal_Int32 nVal )
{
    //return( rWnd.LogicToLogic( Size( nVal, nVal ) ).Height(), MAP_APPFONT, MAP_APPFONT );
    return( nVal << 1 );
}


// - ImpSVGDialog -


ImpSVGDialog::ImpSVGDialog( vcl::Window* pParent, Sequence< PropertyValue >& rFilterData ) :
    ModalDialog( pParent ),
    maFI( VclPtr<FixedLine>::Create(this) ),
    maCBTinyProfile( VclPtr<CheckBox>::Create(this) ),
    maCBEmbedFonts( VclPtr<CheckBox>::Create(this) ),
    maCBUseNativeDecoration( VclPtr<CheckBox>::Create(this) ),
    maBTOK( VclPtr<OKButton>::Create(this, WB_DEF_OK) ),
    maBTCancel( VclPtr<CancelButton>::Create(this) ),
    maBTHelp( VclPtr<HelpButton>::Create(this) ),
    maConfigItem( SVG_EXPORTFILTER_CONFIGPATH, &rFilterData ),
    mbOldNativeDecoration( false )
{
    SetText( "SVG Export Options" );
    SetOutputSizePixel( Size( implMap( *this, 177 ), implMap( *this, 77 ) ) );

    maFI->SetText( "Export" );
    maFI->SetPosSizePixel( Point( implMap( *this, 6 ), implMap( *this, 3 ) ),
                                 Size( implMap( *this, 165 ), implMap( *this, 8 ) ) );

    maCBTinyProfile->SetText( "Use SVG Tiny profile" );
    maCBTinyProfile->SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 14 ) ),
                                     Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBEmbedFonts->SetText( "Embed fonts" );
    maCBEmbedFonts->SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 27 ) ),
                                    Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBUseNativeDecoration->SetText( "Use SVG native text decoration" );
    maCBUseNativeDecoration->SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 41 ) ),
                                             Size( implMap( *this, 142 ), implMap( *this, 10 ) ) );

    maCBTinyProfile->Check( maConfigItem.ReadBool( OUString( SVG_PROP_TINYPROFILE ), false ) );
    maCBEmbedFonts->Check( maConfigItem.ReadBool( OUString( SVG_PROP_EMBEDFONTS ), true ) );
    maCBUseNativeDecoration->Check( maConfigItem.ReadBool( OUString( SVG_PROP_NATIVEDECORATION ), true ) );

    maBTOK->SetPosSizePixel( Point( implMap( *this, 12 ), implMap( *this, 57 ) ),
                            Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );
    maBTCancel->SetPosSizePixel( Point( implMap( *this, 65 ), implMap( *this, 57 ) ),
                                Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );
    maBTHelp->SetPosSizePixel( Point( implMap( *this, 121 ), implMap( *this, 57 ) ),
                              Size( implMap( *this, 50 ), implMap( *this, 14 ) ) );

    maCBTinyProfile->SetToggleHdl( LINK( this, ImpSVGDialog, OnToggleCheckbox ) );
    OnToggleCheckbox( *maCBTinyProfile.get() );

    maFI->Show();

    maCBTinyProfile->Show();
    maCBEmbedFonts->Show();
    maCBUseNativeDecoration->Show();

    maBTOK->Show();
    maBTCancel->Show();
    maBTHelp->Show();
}



ImpSVGDialog::~ImpSVGDialog()
{
    disposeOnce();
}

void ImpSVGDialog::dispose()
{
    maFI.disposeAndClear();
    maCBTinyProfile.disposeAndClear();
    maCBEmbedFonts.disposeAndClear();
    maCBUseNativeDecoration.disposeAndClear();
    maBTOK.disposeAndClear();
    maBTCancel.disposeAndClear();
    maBTHelp.disposeAndClear();
    maConfigItem.WriteModifiedConfig();
    ModalDialog::dispose();
}



Sequence< PropertyValue > ImpSVGDialog::GetFilterData()
{
    maConfigItem.WriteBool( OUString( SVG_PROP_TINYPROFILE ), maCBTinyProfile->IsChecked() );
    maConfigItem.WriteBool( OUString( SVG_PROP_EMBEDFONTS ), maCBEmbedFonts->IsChecked() );
    maConfigItem.WriteBool( OUString( SVG_PROP_NATIVEDECORATION ), maCBUseNativeDecoration->IsChecked() );

    return( maConfigItem.GetFilterData() );
}



IMPL_LINK_TYPED( ImpSVGDialog, OnToggleCheckbox, CheckBox&, rBox, void )
{
    if( &rBox == maCBTinyProfile.get() )
    {
        if( rBox.IsChecked() )
        {
            mbOldNativeDecoration = maCBUseNativeDecoration->IsChecked();

            maCBUseNativeDecoration->Check( false );
            maCBUseNativeDecoration->Disable();
        }
        else
        {
            maCBUseNativeDecoration->Enable();
            maCBUseNativeDecoration->Check( mbOldNativeDecoration );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
