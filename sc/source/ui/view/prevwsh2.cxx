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

// TOOLS
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _SVMEMPOOL_HXX
#define _CACHESTR_HXX

//SV
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _HELP_HXX
#define _SPIN_HXX
#define _SOUND_HXX

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _STDMENU_HXX
#define _CTRLTOOL_HXX

//SVTOOLS
#define _FILTER_HXX
#define _SVTABBX_HXX

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
#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFXFLAGITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _SFXRNGITEM_HXX

#define _SVX_HYPHEN_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_THESDLG_HXX

#include <svx/svdmodel.hxx>
#include <svl/smplhint.hxx>

#include "prevwsh.hxx"
#include "docsh.hxx"
#include "preview.hxx"
#include "hints.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

void ScPreviewShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    sal_Bool bDataChanged = false;

    if (rHint.ISA(SfxSimpleHint))
    {
        sal_uLong nSlot = ((const SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case FID_DATACHANGED:
            case SID_SCPRINTOPTIONS:
                bDataChanged = sal_True;
                break;
            case SC_HINT_DRWLAYER_NEW:
                {
                    SfxBroadcaster* pDrawBC = pDocShell->GetDocument()->GetDrawBroadcaster();
                    if (pDrawBC)
                        StartListening(*pDrawBC);
                }
                break;
        }
    }
    else if (rHint.ISA(ScPaintHint))
    {
        if ( ((const ScPaintHint&)rHint).GetPrintFlag() )
        {
            sal_uInt16 nParts = ((const ScPaintHint&)rHint).GetParts();
            if (nParts & ( PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE ))
                bDataChanged = sal_True;
        }
    }
    else if (rHint.ISA(SdrHint))
    {
        // SdrHints are no longer used for invalidating, thus react on objectchange instead
        if(HINT_OBJCHG == ((const SdrHint&)rHint).GetKind())
            bDataChanged = sal_True;
    }

    if (bDataChanged)
        pPreview->DataChanged(sal_True);
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
