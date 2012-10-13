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

#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <sfx2/sfxsids.hrc>
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
        maCbbTargets.SetText( rtl::OUString("_self") );
    else
        maCbbTargets.SetText( rTarget );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
