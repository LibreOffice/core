/*************************************************************************
 *
 *  $RCSfile: apperror.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 16:28:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif
#include "basic.hrc"
#include "apperror.hxx"

TYPEINIT1(AppError,AppWin);
AppError::AppError( BasicFrame* pParent, String aFileName )
: AppWin( pParent )
{
    SetText( aFileName );   // Muß vor new MsgEdit stehen!!
    pDataEdit = new MsgEdit( this, pParent, WB_HSCROLL | WB_VSCROLL | WB_LEFT );
    LoadIniFile();
    bHasFile = pDataEdit->Load( aFileName );
    DirEntry aEntry( aFileName );
    UpdateFileInfo( HAS_BEEN_LOADED );
    // Icon definieren:
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

    pMenu->EnableItem( RID_EDITUNDO,    FALSE );
    pMenu->EnableItem( RID_EDITREDO,    FALSE );

    return TRUE;
}

long AppError::DeInitMenu( Menu* pMenu )
{
    AppWin::DeInitMenu (pMenu );

    pMenu->EnableItem( RID_EDITUNDO );
    pMenu->EnableItem( RID_EDITREDO );

    return TRUE;
}

USHORT AppError::GetLineNr(){ return pDataEdit->GetLineNr(); }

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
//    ULONG nFontSize = aFontSize.GetValue( FUNIT_POINT );
    ULONG nFontSize = aFontSize.ToInt32();
//    aFont.SetSize( Size( nFontSize, nFontSize ) );
    aFont.SetHeight( nFontSize );

    aFont.SetTransparent( FALSE );
//    aFont.SetAlign( ALIGN_BOTTOM );
//    aFont.SetHeight( aFont.GetHeight()+2 );
    pDataEdit->SetFont( aFont );
}
