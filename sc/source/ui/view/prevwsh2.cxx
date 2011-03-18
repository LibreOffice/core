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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

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

//SV
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
#define _HELP_HXX
#define _MDIWIN_HXX
#define _SPIN_HXX
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
#define _SFXDOCKWIN_HXX

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
#define _SDR_NOTRANSFORM
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV


#define _SI_NOSBXCONTROLS
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX




// INCLUDE ---------------------------------------------------------------

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
