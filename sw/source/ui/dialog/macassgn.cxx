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

#include <hintids.hxx>

#include <sfx2/htmlmode.hxx>
#include <svx/svxids.hrc>
#include <svl/macitem.hxx>

#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <macassgn.hxx>
#include <docsh.hxx>
#include <strings.hrc>
#include <view.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>


SfxEventNamesItem SwMacroAssignDlg::AddEvents( DlgEventType eType )
{
    SfxEventNamesItem aItem(SID_EVENTCONFIG);

    sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    bool bHtmlMode = nHtmlMode & HTMLMODE_ON;

    switch( eType )
    {
    case MACASSGN_AUTOTEXT:
        aItem.AddEvent( SwResId(STR_EVENT_START_INS_GLOSSARY), OUString(),
                            SvMacroItemId::SwStartInsGlossary );
        aItem.AddEvent( SwResId(STR_EVENT_END_INS_GLOSSARY), OUString(),
                            SvMacroItemId::SwEndInsGlossary);
        // in order for the new handler to become active!
        break;
    case MACASSGN_ALLFRM:
    case MACASSGN_GRAPHIC:          // graphics
        {
            aItem.AddEvent( SwResId(STR_EVENT_IMAGE_ERROR), OUString(),
                                SvMacroItemId::OnImageLoadError);
            aItem.AddEvent( SwResId(STR_EVENT_IMAGE_ABORT), OUString(),
                                SvMacroItemId::OnImageLoadCancel);
            aItem.AddEvent( SwResId(STR_EVENT_IMAGE_LOAD), OUString(),
                                SvMacroItemId::OnImageLoadDone);
        }
        [[fallthrough]];
    case MACASSGN_FRMURL:           // Frame - URL-Attributes
        {
            if( !bHtmlMode &&
                (MACASSGN_FRMURL == eType || MACASSGN_ALLFRM == eType))
            {
                aItem.AddEvent( SwResId( STR_EVENT_FRM_KEYINPUT_A ), OUString(),
                                SvMacroItemId::SwFrmKeyInputAlpha );
                aItem.AddEvent( SwResId( STR_EVENT_FRM_KEYINPUT_NOA ), OUString(),
                                SvMacroItemId::SwFrmKeyInputNoAlpha );
                aItem.AddEvent( SwResId( STR_EVENT_FRM_RESIZE ), OUString(),
                                SvMacroItemId::SwFrmResize );
                aItem.AddEvent( SwResId( STR_EVENT_FRM_MOVE ), OUString(),
                                SvMacroItemId::SwFrmMove );
            }
        }
        [[fallthrough]];
    case MACASSGN_OLE:              // OLE
        {
            if( !bHtmlMode )
                aItem.AddEvent( SwResId(STR_EVENT_OBJECT_SELECT), OUString(),
                                SvMacroItemId::SwObjectSelect );
        }
        [[fallthrough]];
    case MACASSGN_INETFMT:          // INetFormat-Attributes
        {
            aItem.AddEvent( SwResId(STR_EVENT_MOUSEOVER_OBJECT), OUString(),
                                SvMacroItemId::OnMouseOver );
            aItem.AddEvent( SwResId(STR_EVENT_MOUSECLICK_OBJECT), OUString(),
                                SvMacroItemId::OnClick);
            aItem.AddEvent( SwResId(STR_EVENT_MOUSEOUT_OBJECT), OUString(),
                                SvMacroItemId::OnMouseOut);
        }
        break;
    }

    return aItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
