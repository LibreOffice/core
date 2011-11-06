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
#include "precompiled_cui.hxx"

#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <sfx2/sfxsids.hrc>
#define _ANIMATION
#include "macroass.hxx"
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>
#include <svl/urlbmk.hxx>
#include <svx/xoutbmp.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <cuiimapdlg.hrc>
#include <cuiimapwnd.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svdpagv.hxx>
#include <svl/urihelper.hxx>
#include <svx/xfillit.hxx>
#include <svx/xlineit.hxx>

#include <sot/formats.hxx>

#define TRANSCOL Color( COL_WHITE )

/*************************************************************************
|*
|*  URLDlg
|*
\************************************************************************/

URLDlg::URLDlg( Window* pWindow, const String& rURL, const String& rAlternativeText, const String& rDescription, const String& rTarget, const String& rName, TargetList& rTargetList )
: ModalDialog( pWindow, CUI_RES( RID_SVXDLG_IMAPURL ) )
, maFtURL( this, CUI_RES( FT_URL1 ) )
, maEdtURL( this, CUI_RES( EDT_URL ) )
, maFtTarget( this, CUI_RES( FT_TARGET ) )
, maCbbTargets( this, CUI_RES( CBB_TARGETS ) )
, maFtName( this, CUI_RES( FT_NAME ) )
, maEdtName( this, CUI_RES( EDT_NAME ) )
, maFtAlternativeText( this, CUI_RES( FT_URLDESCRIPTION ) )
, maEdtAlternativeText( this, CUI_RES( EDT_URLDESCRIPTION ) )
, maFtDescription( this, CUI_RES( FT_DESCRIPTION ) )
, maEdtDescription( this, CUI_RES( EDT_DESCRIPTION ) )
, maFlURL( this, CUI_RES( FL_URL ) )
, maBtnHelp( this, CUI_RES( BTN_HELP1 ) )
, maBtnOk( this, CUI_RES( BTN_OK1 ) )
, maBtnCancel( this, CUI_RES( BTN_CANCEL1 ) )
{
    FreeResource();

    maEdtURL.SetText( rURL );
    maEdtAlternativeText.SetText( rAlternativeText );
    maEdtDescription.SetText( rDescription );
    maEdtName.SetText( rName );

    for( String* pStr = rTargetList.First(); pStr; pStr = rTargetList.Next() )
        maCbbTargets.InsertEntry( *pStr );

    if( !rTarget.Len() )
        maCbbTargets.SetText( String::CreateFromAscii( "_self" ) );
    else
        maCbbTargets.SetText( rTarget );
}
