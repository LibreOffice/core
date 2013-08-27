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
: ModalDialog(pWindow, "IMapDialog", "cui/ui/cuiimapdlg.ui")
{
    get(m_pEdtURL, "urlentry");
    get(m_pCbbTargets, "frameCB");
    get(m_pEdtName, "nameentry");
    get(m_pEdtAlternativeText, "textentry");
    get(m_pEdtDescription, "descTV");
    m_pEdtDescription->set_height_request(m_pEdtDescription->GetTextHeight() * 5);
    m_pEdtDescription->set_width_request(m_pEdtDescription->approximate_char_width() * 60);

    m_pEdtURL->SetText( rURL );
    m_pEdtAlternativeText->SetText( rAlternativeText );
    m_pEdtDescription->SetText( rDescription );
    m_pEdtName->SetText( rName );

    for( size_t i = 0, n = rTargetList.size(); i < n; ++i )
        m_pCbbTargets->InsertEntry( rTargetList[ i ] );

    if( !rTarget.Len() )
        m_pCbbTargets->SetText( OUString("_self") );
    else
        m_pCbbTargets->SetText( rTarget );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
