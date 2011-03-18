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
#include "precompiled_basic.hxx"
#include <tools/config.hxx>
#include <svtools/ctrltool.hxx>
#include <basic/ttstrhlp.hxx>
#include "basic.hrc"
#include "apperror.hxx"

TYPEINIT1(AppError,AppWin);
AppError::AppError( BasicFrame* pParent, String aFileName )
: AppWin( pParent )
{
    SetText( aFileName );   // Call before MsgEdit!!
    pDataEdit = new MsgEdit( this, pParent, WB_HSCROLL | WB_VSCROLL | WB_LEFT );
    LoadIniFile();
    bHasFile = pDataEdit->Load( aFileName );
    DirEntry aEntry( aFileName );
    UpdateFileInfo( HAS_BEEN_LOADED );
    // Define icon
//  pIcon = new Icon( ResId( RID_WORKICON ) );
//  if( pIcon ) SetIcon( *pIcon );

    pDataEdit->Show();
    GrabFocus();
}

AppError::~AppError()
{
    DataEdit* pTemp = pDataEdit;
    pDataEdit = NULL;
    delete pTemp;
}

// Set up the menu
long AppError::InitMenu( Menu* pMenu )
{
    AppWin::InitMenu (pMenu );

    pMenu->EnableItem( RID_EDITUNDO,    sal_False );
    pMenu->EnableItem( RID_EDITREDO,    sal_False );

    return sal_True;
}

long AppError::DeInitMenu( Menu* pMenu )
{
    AppWin::DeInitMenu (pMenu );

    pMenu->EnableItem( RID_EDITUNDO );
    pMenu->EnableItem( RID_EDITREDO );

    return sal_True;
}

sal_uInt16 AppError::GetLineNr(){ return pDataEdit->GetLineNr(); }

FileType AppError::GetFileType()
{
    return FT_RESULT_FILE;
}

void AppError::LoadIniFile()
{
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );
    aBaseDir = DirEntry( aConf.ReadKey("BaseDir") );


    FontList aFontList( pFrame );   // Just some Window is needed
    aConf.SetGroup("Misc");
    String aFontName = String( aConf.ReadKey( "ScriptFontName", "Courier" ), RTL_TEXTENCODING_UTF8 );
    String aFontStyle = String( aConf.ReadKey( "ScriptFontStyle", "normal" ), RTL_TEXTENCODING_UTF8 );
    String aFontSize = String( aConf.ReadKey( "ScriptFontSize", "12" ), RTL_TEXTENCODING_UTF8 );
    Font aFont = aFontList.Get( aFontName, aFontStyle );
//    sal_uIntPtr nFontSize = aFontSize.GetValue( FUNIT_POINT );
    sal_uIntPtr nFontSize = aFontSize.ToInt32();
//    aFont.SetSize( Size( nFontSize, nFontSize ) );
    aFont.SetHeight( nFontSize );

    aFont.SetTransparent( sal_False );
//    aFont.SetAlign( ALIGN_BOTTOM );
//    aFont.SetHeight( aFont.GetHeight()+2 );
    pDataEdit->SetFont( aFont );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
