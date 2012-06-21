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




//------------------------------------------------------------------------

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
#define _DYNARY_HXX
#define _SVMEMPOOL_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
#define _HELP_HXX
#define _MDIWIN_HXX
#define _SPIN_HXX
#define _STATUS_HXX
#define _SVTCTRL3_HXX
#define _COLDLG_HXX
#define _SOUND_HXX

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _STDMENU_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX

//SVTOOLS
#define _FILTER_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX

//sfxsh.hxx
#define _SFXMACRO_HXX

// SFX
#define _SFX_SAVEOPT_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdoc.hxx
#define _SFX_DOCFILT_HXX
#define _SFX_IPFRM_HXX

//sfxdlg.hxx
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCMAN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _SFXFLAGITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _SFXRNGITEM_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM

#define _SI_NOSBXCONTROLS
#define _VCATTR_HXX
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
#define _SIDLL_HXX

//------------------------------------------------------------------------

#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdetc.hxx>

#include "futext.hxx"
#include "tabvwsh.hxx"

//------------------------------------------------------------------------

SdrOutliner* FuText::MakeOutliner()
{
    ScViewData* pViewData = pViewShell->GetViewData();
    SdrOutliner* pOutl = SdrMakeOutliner(OUTLINERMODE_OUTLINEOBJECT, pDrDoc);

    pViewData->UpdateOutlinerFlags(*pOutl);

    //  Die EditEngine benutzt beim RTF Export (Clipboard / Drag&Drop)
    //  den MapMode des RefDevices, um die Fontgroesse zu setzen

    //  #i10426# The ref device isn't set to the EditEngine before SdrBeginTextEdit now,
    //  so the device must be taken from the model here.
    OutputDevice* pRef = pDrDoc->GetRefDevice();
    if (pRef && pRef != pWindow)
        pRef->SetMapMode( MapMode(MAP_100TH_MM) );

    return pOutl;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
