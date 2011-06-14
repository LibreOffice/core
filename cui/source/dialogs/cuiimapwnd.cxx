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

    for( size_t i = 0, n = rTargetList.size(); i < n; ++i )
        maCbbTargets.InsertEntry( *rTargetList[ i ] );

    if( !rTarget.Len() )
        maCbbTargets.SetText( String::CreateFromAscii( "_self" ) );
    else
        maCbbTargets.SetText( rTarget );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
