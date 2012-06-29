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

#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _MACRODLG_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
#define _PRNDLG_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX

//sfx
#define _SFXCTRLITEM
#define _SFXDISPATCH_HXX
#define _SFXIMGMGR_HXX
#define _SFX_MINFITEM_HXX

//sfxcore.hxx
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXDOCINF_HXX

// SFX
#define _SFXMSGPOOL_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdlg.hxx
#define _SFX_DINFDLG_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _NEWSTYLE_HXX

//sfxitems.hxx
#define _SFXFLAGITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _SFXRNGITEM_HXX

#define _SFXBASIC_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX
#define _SFXMSGPOOL_HXX
#define _SFXMSG_HXX
#define _SFX_PRNMON_HXX

#define _SVTABBX_HXX

#define _SVX_HYPHEN_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_THESDLG_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>

#include "document.hxx"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "drawpage.hxx"
#include "drwlayer.hxx"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

void ActivateOlk( ScViewData* /* pViewData */ )
{
    // Browser fuer Virtual Controls fuellen
    //  VC's und den Browser dazu gibts nicht mehr...

    //  GetSbxForm gibt's nicht mehr, muss auch nichts mehr angemeldet werden
}

void DeActivateOlk( ScViewData* /* pViewData */ )
{
    // Browser fuer Virtual Controls fuellen
    //  VC's und den Browser dazu gibts nicht mehr...

    //  GetSbxForm gibt's nicht mehr, muss auch nichts mehr angemeldet werden
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
