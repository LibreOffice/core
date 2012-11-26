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
#include "precompiled_basic.hxx"
#include <tools/config.hxx>
#include <svtools/ctrltool.hxx>
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include "basic.hrc"
#include "apperror.hxx"

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
