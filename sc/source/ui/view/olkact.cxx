/*************************************************************************
 *
 *  $RCSfile: olkact.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:37:37 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
#define _PRNDLG_HXX
//#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDMENU_HXX
#define _TABBAR_HXX
//#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX


//sfx
#define _SFXAPPWIN_HXX
#define _SFXCTRLITEM
#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMULTISEL_HXX
#define _SFX_MINFITEM_HXX


//sfxcore.hxx
//#define _SFXINIMGR_HXX ***
//#define _SFXCFGITEM_HXX
//#define _SFX_PRINTER_HXX
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFXDISPATCH_HXX
//#define _SFXMSG_HXX ***
//#define _SFXOBJFACE_HXX ***
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX ***
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX
//#define _SFXCTRLITEM_HXX
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
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX
//#define _SFXITEMSET_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
//#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
//#define _SFXSLSTITM_HXX
//#define _SFXSTYLE_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX
//#define _XPOOL_HXX
//#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
//#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS




#define _SFXBASIC_HXX
#define _SFX_DOCFILE_HXX
#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _SFXFILEDLG_HXX
#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX
#define _SFXMSGPOOL_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSG_HXX
#define _SFX_PRNMON_HXX

//si
#define _SI_NOSBXCONTROLS
#define _SI_NOCONTROL
//#define SI_NOITEMS
//#define SI_NODRW
//#define SI_NOOTHERFORMS
#define _SIDLL_HXX
//#define _VCSBX_HXX
//#define _VCBRW_HXX

//#define _SVDATTR_HXX      <--- der wars
#define _SVDXOUT_HXX
#define _SVDEC_HXX
//#define _SVDIO_HXX
//#define _SVDLAYER_HXX
//#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX

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

#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>

#include "document.hxx"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "drawpage.hxx"
#include "drwlayer.hxx"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

void ActivateOlk( ScViewData* pViewData )
{
    // Browser fuer Virtual Controls fuellen
    //  VC's und den Browser dazu gibts nicht mehr...

    //  GetSbxForm gibt's nicht mehr, muss auch nichts mehr angemeldet werden
}

void DeActivateOlk( ScViewData* pViewData )
{
    // Browser fuer Virtual Controls fuellen
    //  VC's und den Browser dazu gibts nicht mehr...

    //  GetSbxForm gibt's nicht mehr, muss auch nichts mehr angemeldet werden
}





