/*************************************************************************
 *
 *  $RCSfile: frmpage.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:37 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif

#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTEIRO_HXX //autogen
#include <fmteiro.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FRMDLG_HXX
#include <frmdlg.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _FRMPAGE_HXX
#include <frmpage.hxx>
#endif
#ifndef _WRAP_HXX
#include <wrap.hxx>
#endif
#ifndef _COLMGR_HXX
#include <colmgr.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif

#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif
#ifndef _FRMPAGE_HRC
#include <frmpage.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

struct FrmMap
{
    USHORT nStrId;
    USHORT nMirrorStrId;
    USHORT nAlign;
    ULONG  nLBRelations;
};

struct RelationMap
{
    USHORT nStrId;
    USHORT nMirrorStrId;
    ULONG  nLBRelation;
    USHORT nRelation;
};

#define LB_FRAME                0x00000001L // Textbereich des Absatzes
#define LB_PRTAREA              0x00000002L // Textbereich des Absatzes + Einzuege
#define LB_REL_PG_LEFT          0x00000004L // Linker Seitenrand
#define LB_REL_PG_RIGHT         0x00000008L // Rechter Seitenrand
#define LB_REL_FRM_LEFT         0x00000010L // Linker Absatzrand
#define LB_REL_FRM_RIGHT        0x00000020L // Rechter Absatzrand
#define LB_REL_PG_FRAME         0x00000040L // Gesamte Seite
#define LB_REL_PG_PRTAREA       0x00000080L // Textbereich der Seite

#define LB_FLY_REL_PG_LEFT      0x00000100L // Linker Rahmenrand
#define LB_FLY_REL_PG_RIGHT     0x00000200L // Rechter Rahmenrand
#define LB_FLY_REL_PG_FRAME     0x00000400L // Gesamte Rahmen
#define LB_FLY_REL_PG_PRTAREA   0x00000800L // Rahmeninneres

#define LB_VERT_FRAME           0x00001000L // Vertikaler Textbereich des Absatzes
#define LB_VERT_PRTAREA         0x00002000L // Vertikaler Textbereich des Absatzes + Einzuege

#define LB_REL_BASE             0x00010000L // Zeichenausrichtung Basis
#define LB_REL_CHAR             0x00020000L // Zeichenausrichtung Zeichen
#define LB_REL_ROW              0x00040000L // Zeichenausrichtung Zeile

static RelationMap __FAR_DATA aRelationMap[] =
{
    STR_FRAME,              STR_FRAME,                  LB_FRAME,               FRAME,
    STR_PRTAREA,            STR_PRTAREA,                LB_PRTAREA,             PRTAREA,
    STR_REL_PG_LEFT,        STR_MIR_REL_PG_LEFT,        LB_REL_PG_LEFT,         REL_PG_LEFT,
    STR_REL_PG_RIGHT,       STR_MIR_REL_PG_RIGHT,       LB_REL_PG_RIGHT,        REL_PG_RIGHT,
    STR_REL_FRM_LEFT,       STR_MIR_REL_FRM_LEFT,       LB_REL_FRM_LEFT,        REL_FRM_LEFT,
    STR_REL_FRM_RIGHT,      STR_MIR_REL_FRM_RIGHT,      LB_REL_FRM_RIGHT,       REL_FRM_RIGHT,
    STR_REL_PG_FRAME,       STR_REL_PG_FRAME,           LB_REL_PG_FRAME,        REL_PG_FRAME,
    STR_REL_PG_PRTAREA,     STR_REL_PG_PRTAREA,         LB_REL_PG_PRTAREA,      REL_PG_PRTAREA,
    STR_REL_CHAR,           STR_REL_CHAR,               LB_REL_CHAR,            REL_CHAR,

    STR_FLY_REL_PG_LEFT,    STR_FLY_MIR_REL_PG_LEFT,    LB_FLY_REL_PG_LEFT,     REL_PG_LEFT,
    STR_FLY_REL_PG_RIGHT,   STR_FLY_MIR_REL_PG_RIGHT,   LB_FLY_REL_PG_RIGHT,    REL_PG_RIGHT,
    STR_FLY_REL_PG_FRAME,   STR_FLY_REL_PG_FRAME,       LB_FLY_REL_PG_FRAME,    REL_PG_FRAME,
    STR_FLY_REL_PG_PRTAREA, STR_FLY_REL_PG_PRTAREA,     LB_FLY_REL_PG_PRTAREA,  REL_PG_PRTAREA,

    STR_REL_BORDER,         STR_REL_BORDER,             LB_VERT_FRAME,          FRAME,
    STR_REL_PRTAREA,        STR_REL_PRTAREA,            LB_VERT_PRTAREA,        PRTAREA,
};

static RelationMap __FAR_DATA aAsCharRelationMap[] =
{
    STR_REL_BASE,   STR_REL_BASE,   LB_REL_BASE,    FRAME,
    STR_REL_CHAR,   STR_REL_CHAR,   LB_REL_CHAR,    FRAME,
    STR_REL_ROW,    STR_REL_ROW,    LB_REL_ROW,     FRAME,
};

/*--------------------------------------------------------------------
    Beschreibung: Seite verankert
 --------------------------------------------------------------------*/

#define HORI_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_PG_LEFT| \
                        LB_REL_PG_RIGHT)

static FrmMap __FAR_DATA aHPageMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      HORI_PAGE_REL,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     HORI_PAGE_REL,
    STR_CENTER_HORI,    STR_CENTER_HORI,    HORI_CENTER,    HORI_PAGE_REL,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      HORI_PAGE_REL
};

static FrmMap __FAR_DATA aHPageHtmlMap[] =
{
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      LB_REL_PG_FRAME
};

#define VERT_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap __FAR_DATA aVPageMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,       VERT_PAGE_REL,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_BOTTOM,    VERT_PAGE_REL,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CENTER,    VERT_PAGE_REL,
    STR_FROMTOP,        STR_FROMTOP,        SVX_VERT_NONE,      VERT_PAGE_REL,
};

static FrmMap __FAR_DATA aVPageHtmlMap[] =
{
    STR_FROMTOP,        STR_FROMTOP,        SVX_VERT_NONE,      LB_REL_PG_FRAME
};

/*--------------------------------------------------------------------
    Beschreibung: Rahmen verankert
 --------------------------------------------------------------------*/

#define HORI_FRAME_REL  (LB_FLY_REL_PG_FRAME|LB_FLY_REL_PG_PRTAREA| \
                        LB_FLY_REL_PG_LEFT|LB_FLY_REL_PG_RIGHT)

static FrmMap __FAR_DATA aHFrameMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,  HORI_FRAME_REL,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     HORI_FRAME_REL,
    STR_CENTER_HORI,    STR_CENTER_HORI,    HORI_CENTER,    HORI_FRAME_REL,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      HORI_FRAME_REL
};

static FrmMap __FAR_DATA aHFlyHtmlMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      LB_FLY_REL_PG_FRAME,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      LB_FLY_REL_PG_FRAME
};

static FrmMap __FAR_DATA aVFlyHtmlMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,       LB_VERT_FRAME,
    STR_FROMTOP,        STR_FROMTOP,        SVX_VERT_NONE,      LB_VERT_FRAME
};

/*--------------------------------------------------------------------
    Beschreibung: Absatz verankert
 --------------------------------------------------------------------*/

#define HORI_PARA_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT)

static FrmMap __FAR_DATA aHParaMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      HORI_PARA_REL,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     HORI_PARA_REL,
    STR_CENTER_HORI,    STR_CENTER_HORI,    HORI_CENTER,    HORI_PARA_REL,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      HORI_PARA_REL
};

#define HTML_HORI_PARA_REL  (LB_FRAME|LB_PRTAREA)

static FrmMap __FAR_DATA aHParaHtmlMap[] =
{
    STR_LEFT,   STR_LEFT,   HORI_LEFT,      HTML_HORI_PARA_REL,
    STR_RIGHT,  STR_RIGHT,  HORI_RIGHT,     HTML_HORI_PARA_REL,
};

static FrmMap __FAR_DATA aHParaHtmlAbsMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      HTML_HORI_PARA_REL,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     HTML_HORI_PARA_REL,
};

#define VERT_PARA_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA)

static FrmMap __FAR_DATA aVParaMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,       VERT_PARA_REL,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_BOTTOM,    VERT_PARA_REL,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CENTER,    VERT_PARA_REL,
    STR_FROMTOP,        STR_FROMTOP,        SVX_VERT_NONE,      VERT_PARA_REL,
};

static FrmMap __FAR_DATA aVParaHtmlMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,       LB_VERT_PRTAREA,
};

/*--------------------------------------------------------------------
    Beschreibung: Relativ zum Zeichen verankert
 --------------------------------------------------------------------*/

#define HORI_CHAR_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT|LB_REL_CHAR)

static FrmMap __FAR_DATA aHCharMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      HORI_CHAR_REL,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     HORI_CHAR_REL,
    STR_CENTER_HORI,    STR_CENTER_HORI,    HORI_CENTER,    HORI_CHAR_REL,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      HORI_CHAR_REL
};

#define HTML_HORI_CHAR_REL  (LB_FRAME|LB_PRTAREA|LB_REL_CHAR)

static FrmMap __FAR_DATA aHCharHtmlMap[] =
{
    STR_LEFT,           STR_LEFT,           HORI_LEFT,      HTML_HORI_CHAR_REL,
    STR_RIGHT,          STR_RIGHT,          HORI_RIGHT,     HTML_HORI_CHAR_REL,
};

static FrmMap __FAR_DATA aHCharHtmlAbsMap[] =
{
    STR_LEFT,           STR_MIR_LEFT,       HORI_LEFT,      LB_PRTAREA|LB_REL_CHAR,
    STR_RIGHT,          STR_MIR_RIGHT,      HORI_RIGHT,     LB_PRTAREA,
    STR_FROMLEFT,       STR_MIR_FROMLEFT,   HORI_NONE,      LB_REL_PG_FRAME
};

#define VERT_CHAR_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA|LB_REL_CHAR)

static FrmMap __FAR_DATA aVCharMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,           VERT_CHAR_REL,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_BOTTOM,        VERT_CHAR_REL,
    STR_BELOW,          STR_BELOW,          SVX_VERT_CHAR_BOTTOM,   LB_REL_CHAR,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CENTER,        VERT_CHAR_REL,
    STR_FROMTOP,        STR_FROMTOP,        SVX_VERT_NONE,          VERT_CHAR_REL,
};


static FrmMap __FAR_DATA aVCharHtmlMap[] =
{
    STR_BELOW,          STR_BELOW,          SVX_VERT_CHAR_BOTTOM,   LB_REL_CHAR,
};

static FrmMap __FAR_DATA aVCharHtmlAbsMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,           LB_REL_CHAR,
    STR_BELOW,          STR_BELOW,          SVX_VERT_CHAR_BOTTOM,   LB_REL_CHAR
};
/*--------------------------------------------------------------------
    Beschreibung: Als Zeichen verankert
 --------------------------------------------------------------------*/

static FrmMap __FAR_DATA aVAsCharMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,           LB_REL_BASE,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_BOTTOM,        LB_REL_BASE,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CENTER,        LB_REL_BASE,

    STR_TOP,            STR_TOP,            SVX_VERT_CHAR_TOP,      LB_REL_CHAR,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_CHAR_BOTTOM,   LB_REL_CHAR,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CHAR_CENTER,   LB_REL_CHAR,

    STR_TOP,            STR_TOP,            SVX_VERT_LINE_TOP,      LB_REL_ROW,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_LINE_BOTTOM,   LB_REL_ROW,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_LINE_CENTER,   LB_REL_ROW,

    STR_FROMBOTTOM,     STR_FROMBOTTOM,     SVX_VERT_NONE,          LB_REL_BASE
};

static FrmMap __FAR_DATA aVAsCharHtmlMap[] =
{
    STR_TOP,            STR_TOP,            SVX_VERT_TOP,           LB_REL_BASE,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_CENTER,        LB_REL_BASE,

    STR_TOP,            STR_TOP,            SVX_VERT_CHAR_TOP,      LB_REL_CHAR,

    STR_TOP,            STR_TOP,            SVX_VERT_LINE_TOP,      LB_REL_ROW,
    STR_BOTTOM,         STR_BOTTOM,         SVX_VERT_LINE_BOTTOM,   LB_REL_ROW,
    STR_CENTER_VERT,    STR_CENTER_VERT,    SVX_VERT_LINE_CENTER,   LB_REL_ROW,
};


static FrmMap* pVMap = 0;
static FrmMap* pHMap = 0;

static BOOL bProtPos   = FALSE;
static BOOL bProtSize = FALSE;
static BOOL bProtCntnt = FALSE;

static const USHORT nPathLen = 30;

static USHORT __FAR_DATA aPageRg[] = {
    RES_FRM_SIZE, RES_FRM_SIZE,
    RES_VERT_ORIENT, RES_ANCHOR,
    RES_COL, RES_COL,
    0
};
static USHORT __FAR_DATA aAddPgRg[] = {
    RES_PROTECT,            RES_PROTECT,
    RES_PRINT,              RES_PRINT,
    FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
    FN_SET_FRM_ALT_NAME,    FN_SET_FRM_ALT_NAME,
    0
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT lcl_GetFrmMapCount(FrmMap* pMap)
{
    if ( pMap )
    {
        int aSizeOf = sizeof(FrmMap);
        if( pMap == aVParaHtmlMap)
            return sizeof(aVParaHtmlMap) / aSizeOf;
        if( pMap == aVAsCharHtmlMap)
            return sizeof(aVAsCharHtmlMap) / aSizeOf;
        if( pMap == aHParaHtmlMap)
            return sizeof(aHParaHtmlMap) / aSizeOf;
        if( pMap == aHParaHtmlAbsMap)
            return sizeof(aHParaHtmlAbsMap) / aSizeOf;
        if ( pMap == aVPageMap )
            return sizeof(aVPageMap) / aSizeOf;
        if ( pMap == aVPageHtmlMap )
            return sizeof(aVPageHtmlMap) / aSizeOf;
        if ( pMap == aVAsCharMap )
            return sizeof(aVAsCharMap) / aSizeOf;
        if ( pMap == aVParaMap )
            return sizeof(aVParaMap) / aSizeOf;
        if ( pMap == aHParaMap )
            return sizeof(aHParaMap) / aSizeOf;
        if ( pMap == aHFrameMap )
            return sizeof(aHFrameMap) / aSizeOf;
        if ( pMap == aHCharMap )
            return sizeof(aHCharMap) / aSizeOf;
        if ( pMap == aHCharHtmlMap )
            return sizeof(aHCharHtmlMap) / aSizeOf;
        if ( pMap == aHCharHtmlAbsMap )
            return sizeof(aHCharHtmlAbsMap) / aSizeOf;
        if ( pMap == aVCharMap )
            return sizeof(aVCharMap) / aSizeOf;
        if ( pMap == aVCharHtmlMap )
            return sizeof(aVCharHtmlMap) / aSizeOf;
        if ( pMap == aVCharHtmlAbsMap )
            return sizeof(aVCharHtmlAbsMap) / aSizeOf;
        if ( pMap == aHPageHtmlMap )
            return sizeof(aHPageHtmlMap) / aSizeOf;
        if ( pMap == aHFlyHtmlMap )
            return sizeof(aHFlyHtmlMap) / aSizeOf;
        if ( pMap == aVFlyHtmlMap )
            return sizeof(aVFlyHtmlMap) / aSizeOf;
        return sizeof(aHPageMap) / aSizeOf;
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long lcl_GetTwipValue(MetricField& rMetric)
{
    return rMetric.Denormalize(rMetric.GetValue(FUNIT_TWIP));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void lcl_SetTwipValue(MetricField& rMetric, long nValue)
{
    rMetric.SetValue(rMetric.Normalize(nValue),FUNIT_TWIP);
}

/*--------------------------------------------------------------------
    Beschreibung:   StandardRahmenTabPage
 --------------------------------------------------------------------*/

SwFrmPage::SwFrmPage ( Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage      (pParent, SW_RES(TP_FRM_STD), rSet),
    aWidthFT        (this, SW_RES(FT_WIDTH)),
    aWidthED        (this, SW_RES(ED_WIDTH)),
    aRelWidthCB     (this, SW_RES(CB_REL_WIDTH)),
    aHeightFT       (this, SW_RES(FT_HEIGHT)),
    aHeightED       (this, SW_RES(ED_HEIGHT)),
    aRelHeightCB    (this, SW_RES(CB_REL_HEIGHT)),
    aAutoHeightCB   (this, SW_RES(CB_AUTOHEIGHT)),
    aFixedRatioCB   (this, SW_RES(CB_FIXEDRATIO)),
    aSizeGB         (this, SW_RES(GB_SIZE)),

    aAnchorTypeRB   (this, SW_RES(RB_ANCHOR_TYPE)),
    aAnchorAsCharRB (this, SW_RES(RB_ANCHOR_CHAR)),
    aAnchorTypeLB   (this, SW_RES(LB_ANCHOR_TYPE)),
    aTypeGB         (this, SW_RES(GB_TYPE)),

    aHorizontalFT   (this, SW_RES(FT_HORIZONTAL)),
    aHorizontalDLB  (this, SW_RES(DLB_HORIZONTAL)),
    aAtHorzPosFT    (this, SW_RES(FT_AT_HORZ_POS)),
    aAtHorzPosED    (this, SW_RES(ED_AT_HORZ_POS)),
    aHoriRelationFT (this, SW_RES(FT_HORI_RELATION)),
    aHoriRelationLB (this, SW_RES(LB_HORI_RELATION)),
    aMirrorPagesCB  (this, SW_RES(CB_MIRROR)),
    aVerticalFT     (this, SW_RES(FT_VERTICAL)),
    aVerticalDLB    (this, SW_RES(DLB_VERTICAL)),
    aAtVertPosFT    (this, SW_RES(FT_AT_VERT_POS)),
    aAtVertPosED    (this, SW_RES(ED_AT_VERT_POS)),
    aVertRelationFT (this, SW_RES(FT_VERT_RELATION)),
    aVertRelationLB (this, SW_RES(LB_VERT_RELATION)),
    aPositionGB     (this, SW_RES(GB_POSITION)),
    aRealSizeBT     (this, SW_RES(BT_REALSIZE)),
    aExampleWN      (this, SW_RES(WN_BSP)),
    bFormat(FALSE),
    bNew(TRUE),
    nDlgType(0),
    nUpperBorder(0),
    nLowerBorder(0),
    nOldH(HORI_CENTER),
    nOldHRel(0),
    nOldV(SVX_VERT_TOP),
    nOldVRel(PRTAREA),
    bAtHorzPosModified( FALSE ),
    bAtVertPosModified( FALSE ),
    bWidthLastChanged ( TRUE ),
    nHtmlMode(0),
    bHtmlMode(FALSE),
    bNoModifyHdl(TRUE)
{
    FreeResource();
    SetExchangeSupport();

    Link aLk = LINK(this, SwFrmPage, RangeModifyHdl);
    aWidthED.    SetLoseFocusHdl( aLk );
    aHeightED.   SetLoseFocusHdl( aLk );
    aAtHorzPosED.SetLoseFocusHdl( aLk );
    aAtVertPosED.SetLoseFocusHdl( aLk );

    aLk = LINK(this, SwFrmPage, ModifyHdl);
    aWidthED.    SetModifyHdl( aLk );
    aHeightED.   SetModifyHdl( aLk );
    aAtHorzPosED.SetModifyHdl( aLk );
    aAtVertPosED.SetModifyHdl( aLk );

    aLk = LINK(this, SwFrmPage, TypHdl);
    aAnchorTypeRB.SetClickHdl( aLk );
    aAnchorAsCharRB.SetClickHdl( aLk );
    aAnchorTypeLB.SetSelectHdl( aLk );
    aAnchorTypeLB.SelectEntryPos(0);
    aAnchorTypeLB.SetEntryData(0, (void*)FLY_PAGE);
    aAnchorTypeLB.SetEntryData(1, (void*)FLY_AT_CNTNT);
    aAnchorTypeLB.SetEntryData(2, (void*)FLY_AUTO_CNTNT);
    aAnchorTypeLB.SetEntryData(3, (void*)FLY_AT_FLY);

    aHorizontalDLB.SetSelectHdl(LINK(this, SwFrmPage, PosHdl));
    aVerticalDLB.  SetSelectHdl(LINK(this, SwFrmPage, PosHdl));

    aHoriRelationLB.SetSelectHdl(LINK(this, SwFrmPage, RelHdl));
    aVertRelationLB.SetSelectHdl(LINK(this, SwFrmPage, RelHdl));

    aMirrorPagesCB.SetClickHdl(LINK(this, SwFrmPage, MirrorHdl));

    aLk = LINK(this, SwFrmPage, RelSizeClickHdl);
    aRelWidthCB.SetClickHdl( aLk );
    aRelHeightCB.SetClickHdl( aLk );

    aFixedRatioCB.SetClickHdl(LINK(this, SwFrmPage, AspectRatioCheckHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:   Dtor
 --------------------------------------------------------------------*/

SwFrmPage::~SwFrmPage()
{
}

SfxTabPage* SwFrmPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmPage( pParent, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFrmPage::Reset( const SfxItemSet &rSet )
{

    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell() :
            ((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();

    nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;

    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric( aWidthED, aMetric );
    SetMetric( aHeightED, aMetric );
    SetMetric( aAtHorzPosED, aMetric );
    SetMetric( aAtVertPosED, aMetric );


    const SwFmtAnchor& rAnchor = (const SwFmtAnchor&)rSet.Get(RES_ANCHOR);

    if (bFormat)
    {
        // Bei Formaten keine Ankerbearbeitung
        aAnchorTypeRB.Enable( FALSE );
        aAnchorTypeLB.Enable( FALSE );
        aAnchorAsCharRB.Enable( FALSE );
        aTypeGB.Enable( FALSE );
        aFixedRatioCB.Enable(FALSE);
    }
    else if (rAnchor.GetAnchorId() != FLY_AT_FLY && !pSh->IsFlyInFly())
        aAnchorTypeLB.RemoveEntry(3);

    if ( nDlgType == DLG_FRM_GRF || nDlgType == DLG_FRM_OLE )
    {
        ASSERT(pSh , "shell not found");
        const SfxPoolItem* pItem;
        //OS: nur fuer die Variante Einfuegen/Grafik/Eigenschaften
        if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_GRF_REALSIZE, FALSE, &pItem))
            aGrfSize = ((const SvxSizeItem*)pItem)->GetSize();
        else
            pSh->GetGrfSize( aGrfSize );

        if ( !bNew )
        {
            aRealSizeBT.Show();
            aRealSizeBT.SetClickHdl(LINK(this, SwFrmPage, RealSizeHdl));
        }
        else
            aTypeGB.SetSizePixel(Size(aTypeGB.GetSizePixel().Width(), aSizeGB.GetSizePixel().Height()));

        if ( nDlgType == DLG_FRM_GRF )
            aFixedRatioCB.Check( FALSE );
        else
        {
            if ( bNew )
                SetText( SW_RESSTR(STR_OLE_INSERT));
            else
                SetText( SW_RESSTR(STR_OLE_EDIT));
        }
    }
    else
    {
        aTypeGB.SetSizePixel(Size(aTypeGB.GetSizePixel().Width(), aSizeGB.GetSizePixel().Height()));
        aGrfSize = ((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetSize();
    }

    //Prozenteingabe ermoeglichen.
    aWidthED. SetBaseValue( aWidthED.Normalize(aGrfSize.Width()), FUNIT_TWIP );
    aHeightED.SetBaseValue( aHeightED.Normalize(aGrfSize.Height()), FUNIT_TWIP );

    // Allgemeiner Initialisierungteil
    USHORT nPos = 0;
    if (rAnchor.GetAnchorId() == FLY_IN_CNTNT)
        aAnchorAsCharRB.Check();
    else
    {
        aAnchorTypeRB.Check();

        while ((USHORT)(ULONG)aAnchorTypeLB.GetEntryData(nPos) != rAnchor.GetAnchorId())
            nPos++;
        aAnchorTypeLB.SelectEntryPos(nPos);
    }

    if(bHtmlMode)
    {
        if(nDlgType == DLG_FRM_STD &&
                0 == (nHtmlMode & HTMLMODE_FULL_ABS_POS))
        {
            aHeightFT   .Enable( FALSE );
            aHeightED   .Enable( FALSE );
            aRelHeightCB.Enable( FALSE );
        }
        if( 0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS))
        {
            if (GetAnchor() == FLY_PAGE )
            {
                nPos = 0;
                while ((USHORT)(ULONG)aAnchorTypeLB.GetEntryData(nPos) != FLY_AT_CNTNT)
                    nPos++;
                aAnchorTypeLB.SelectEntryPos(nPos);
            }
            aAnchorTypeLB.RemoveEntry(0);
        }
        aAutoHeightCB.Enable(FALSE);
        aMirrorPagesCB.Show(FALSE);
        if(nDlgType == DLG_FRM_STD)
            aFixedRatioCB.Enable(FALSE);
    }

    Init( rSet, TRUE );
    aAtVertPosED.SaveValue();
    aAtHorzPosED.SaveValue();

    bNoModifyHdl = FALSE;
    RangeModifyHdl(&aWidthED);  // Alle Maximalwerte initial setzen

    aAutoHeightCB.SaveValue();
}


/*--------------------------------------------------------------------
    Beschreibung:   Attribute in den Set stopfen bei OK
 --------------------------------------------------------------------*/

BOOL SwFrmPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bRet = FALSE;
    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                        : ((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();
    ASSERT( pSh , "shell not found");
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOldItem = 0;

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();

    if ( !bFormat )
    {
        pOldItem = GetOldItem(rSet, RES_ANCHOR);
        if (bNew || !pOldItem || eAnchorId != ((const SwFmtAnchor*)pOldItem)->GetAnchorId())
        {
            SwFmtAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
            bRet = 0 != rSet.Put( aAnc );
        }
    }

    if ( pHMap )
    {
        SwFmtHoriOrient aHoriOrient( (const SwFmtHoriOrient&)
                                                rOldSet.Get(RES_HORI_ORIENT) );

        USHORT nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        USHORT nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        USHORT nRel = GetRelation(pHMap, aHoriRelationLB);

        const SwHoriOrient      eHOri = (SwHoriOrient)nAlign;
        const SwRelationOrient  eRel  = (SwRelationOrient)nRel;

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(aMirrorPagesCB.IsChecked());

        BOOL bMod = aAtHorzPosED.GetText() != aAtHorzPosED.GetSavedValue();
        bMod |= aMirrorPagesCB.GetState() != aMirrorPagesCB.GetSavedValue();

        if ( eHOri == HORI_NONE &&
             (bNew || (bAtHorzPosModified || bMod) || nOldH != eHOri ) )
        {
            SwTwips nX = aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP));
            aHoriOrient.SetPos( nX );
        }

        pOldItem = GetOldItem(rSet, FN_HORI_ORIENT);
        BOOL bSame = FALSE;
        if ((bNew == bFormat) && pOldItem)
        {
             bSame = bFormat ?
                aHoriOrient.GetHoriOrient() == ((SwFmtHoriOrient*)pOldItem)->GetHoriOrient() &&
                aHoriOrient.GetRelationOrient() == ((SwFmtHoriOrient*)pOldItem)->GetRelationOrient() &&
                aHoriOrient.GetPos() == ((SwFmtHoriOrient*)pOldItem)->GetPos()

                    : aHoriOrient == (SwFmtHoriOrient&)*pOldItem;
        }
        if ((bNew && !bFormat) || ((bAtHorzPosModified || bMod) && !bSame))
        {
            bRet |= 0 != rSet.Put( aHoriOrient );
        }
    }

    if ( pVMap )
    {
        // Ausrichtung Vertikal
        SwFmtVertOrient aVertOrient( (const SwFmtVertOrient&)
                                                rOldSet.Get(RES_VERT_ORIENT) );

        USHORT nMapPos = GetMapPos(pVMap, aVerticalDLB);
        USHORT nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        USHORT nRel = GetRelation(pVMap, aVertRelationLB);

        const SvxFrameVertOrient    eVOri = (SvxFrameVertOrient)nAlign;
        const SwRelationOrient      eRel  = (SwRelationOrient)nRel;

        aVertOrient.SetVertOrient    ( (SwVertOrient)eVOri);
        aVertOrient.SetRelationOrient( eRel );

        BOOL bMod = aAtVertPosED.GetText() != aAtVertPosED.GetSavedValue();

        if ( eVOri == SVX_VERT_NONE &&
             ( bNew || (bAtVertPosModified || bMod) || nOldV != eVOri) )
        {
            // Vertikale Position
            // fuer zeichengebundene Rahmen Offset umrechenen
            SwTwips nY = aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP));
            if (eAnchorId == FLY_IN_CNTNT)
                nY *= -1;
            aVertOrient.SetPos( nY );
        }
        pOldItem = GetOldItem(rSet, FN_VERT_ORIENT);
        BOOL bSame = FALSE;
        if((bNew == bFormat) && pOldItem)
        {
             bSame = bFormat ?
                aVertOrient.GetVertOrient() == ((SwFmtVertOrient*)pOldItem)->GetVertOrient() &&
                aVertOrient.GetRelationOrient() == ((SwFmtVertOrient*)pOldItem)->GetRelationOrient() &&
                aVertOrient.GetPos() == ((SwFmtVertOrient*)pOldItem)->GetPos()

                    : aVertOrient == (SwFmtVertOrient&)*pOldItem;
        }
        if( ( bNew && !bFormat ) || ((bAtVertPosModified || bMod) && !bSame ))
        {
            bRet |= 0 != rSet.Put( aVertOrient );
        }
    }

    //Size setzen
    //Neuer Sonderfall: Ist die Groesse von pMgr(0, 0), so werden die
    //Eigenschaften fuer eine Grafik festgelegt, die noch gar nicht geladen
    //ist. Dann wird hier kein SetSize() gemacht, wenn die
    //Groesseneinstellungen vom Anwender nicht veraendert wurden.
    const SwFmtFrmSize& rOldSize = (const SwFmtFrmSize& )rOldSet.Get(RES_FRM_SIZE);
    SwFmtFrmSize aSz( rOldSize );

/*  // Folgende Zeilen nur zum debuggen:
    BOOL bModified = aWidthED.IsValueModified();
    USHORT nOldWidth = rOldSize.GetWidth ();
    USHORT nWidth = aWidthED .GetValue();
    USHORT nMinWidth = aWidthED .GetMin();*/

    BOOL bValueModified = (aWidthED.IsValueModified() || aHeightED.IsValueModified());
    BOOL bCheckChanged = (aRelWidthCB.GetSavedValue() != aRelWidthCB.IsChecked()
                        || aRelHeightCB.GetSavedValue() != aRelHeightCB.IsChecked());

    BOOL bLegalValue = !(!rOldSize.GetWidth () && !rOldSize.GetHeight() &&
                            aWidthED .GetValue() == aWidthED .GetMin() &&
                            aHeightED.GetValue() == aHeightED.GetMin());

    if ((bNew && !bFormat) || (bValueModified || bCheckChanged) && bLegalValue)
    {
        long nNewWidth = aWidthED.Denormalize (aWidthED.GetRealValue(FUNIT_TWIP));
        long nNewHeight = aHeightED.Denormalize(aHeightED.GetRealValue(FUNIT_TWIP));
        aSz.SetWidth (nNewWidth);
        aSz.SetHeight(nNewHeight);

        if (aRelWidthCB.IsChecked())
        {
            aSz.SetWidthPercent((BYTE)Min(100L, aWidthED.Convert(aWidthED.Normalize(nNewWidth), FUNIT_TWIP, FUNIT_CUSTOM)));
        }
        else
            aSz.SetWidthPercent(0);
        if (aRelHeightCB.IsChecked())
            aSz.SetHeightPercent((BYTE)Min(100L, aHeightED.Convert(aHeightED.Normalize(nNewHeight), FUNIT_TWIP, FUNIT_CUSTOM)));
        else
            aSz.SetHeightPercent(0);

        if (aFixedRatioCB.IsChecked() && (aRelWidthCB.IsChecked() ^ aRelHeightCB.IsChecked()))
        {
            if (aRelWidthCB.IsChecked())
                aSz.SetHeightPercent(0xff);
            else
                aSz.SetWidthPercent(0xff);
        }
    }
    if ( nDlgType != DLG_FRM_GRF )
    {
        if (aAutoHeightCB.GetState() != aAutoHeightCB.GetSavedValue())
        {
            SwFrmSize eFrmSize = (SwFrmSize) aAutoHeightCB.IsChecked() ?
                                                    ATT_MIN_SIZE : ATT_FIX_SIZE;
            if(eFrmSize != aSz.GetSizeType())
            {
                aSz.SetSizeType(eFrmSize);
            }
        }
    }
    if( !bFormat && aFixedRatioCB.GetSavedValue() != aFixedRatioCB.IsChecked())
        bRet |= 0 != rSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, aFixedRatioCB.IsChecked()));

    pOldItem = GetOldItem(rSet, RES_FRM_SIZE);

    if ((pOldItem && aSz != *pOldItem) || (!pOldItem && !bFormat) ||
            (bFormat &&
                (aSz.GetWidth() > 0 || aSz.GetWidthPercent() > 0) &&
                    (aSz.GetHeight() > 0 || aSz.GetHeightPercent() > 0)))
    {
        if (aSz.GetSizeType() == ATT_VAR_SIZE)  // VAR_SIZE gibts nicht bei Rahmen
            aSz.SetSizeType(ATT_MIN_SIZE);      // Bug #45776 (Vorlagen ohne Breite/Hoehe)

        bRet |= 0 != rSet.Put( aSz );
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Horizontale und vertikale Pos initialisieren
 --------------------------------------------------------------------*/

void SwFrmPage::InitPos(USHORT nId,
                                USHORT nH,
                                USHORT nHRel,
                                USHORT nV,
                                USHORT nVRel,
                                long   nX,
                                long   nY)
{
    USHORT nPos = aVerticalDLB.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && pVMap )
    {
        nOldV    = pVMap[nPos].nAlign;

        nPos = aVertRelationLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            nOldVRel = ((RelationMap *)aVertRelationLB.GetEntryData(nPos))->nRelation;
    }

    nPos = aHorizontalDLB.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && pHMap )
    {
        nOldH    = pHMap[nPos].nAlign;

        nPos = aHoriRelationLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            nOldHRel = ((RelationMap *)aHoriRelationLB.GetEntryData(nPos))->nRelation;
    }

    BOOL bEnable = TRUE;
    if ( nId == FLY_PAGE )
    {
        pVMap = bHtmlMode ? aVPageHtmlMap : aVPageMap;
        pHMap = bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if ( nId == FLY_AT_FLY )
    {
        pVMap = bHtmlMode ? aVFlyHtmlMap : aVParaMap;
        pHMap = bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if ( nId == FLY_AT_CNTNT )
    {
        if(bHtmlMode)
        {
            pVMap = aVParaHtmlMap;
            pHMap = nHtmlMode & HTMLMODE_SOME_ABS_POS ? aHParaHtmlAbsMap : aHParaHtmlMap;
        }
        else
        {
            pVMap = aVParaMap;
            pHMap = aHParaMap;
        }
    }
    else if ( nId == FLY_AUTO_CNTNT )
    {
        if(bHtmlMode)
        {
            pVMap = nHtmlMode & HTMLMODE_SOME_ABS_POS ? aVCharHtmlAbsMap    : aVCharHtmlMap;
            pHMap = nHtmlMode & HTMLMODE_SOME_ABS_POS ? aHCharHtmlAbsMap    : aHCharHtmlMap;
        }
        else
        {
            pVMap = aVCharMap;
            pHMap = aHCharMap;
        }
    }
    else if ( nId == FLY_IN_CNTNT )
    {
        pVMap = bHtmlMode ? aVAsCharHtmlMap     : aVAsCharMap;
        pHMap = 0;
        bEnable = FALSE;
    }
    aHorizontalDLB.Enable( bEnable );
    aHorizontalFT.Enable( bEnable );

    // aktuelle Pos selektieren
    // Horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = nOldH;
        nHRel = nOldHRel;
    }
    USHORT nMapPos = FillPosLB(pHMap, nH, aHorizontalDLB);
    FillRelLB(pHMap, nMapPos, nH, nHRel, aHoriRelationLB);

    // Vertikal
    if ( nV == USHRT_MAX )
    {
        nV    = nOldV;
        nVRel = nOldVRel;
    }
    nMapPos = FillPosLB(pVMap, nV, aVerticalDLB);
    FillRelLB(pVMap, nMapPos, nV, nVRel, aVertRelationLB);

    // Edits init
    bEnable = nH == HORI_NONE && nId != FLY_IN_CNTNT;//#61359# warum nicht in Formaten&& !bFormat;
    if (!bEnable)
    {
        aAtHorzPosED.SetValue( 0, FUNIT_TWIP );
        if (nX != LONG_MAX && bHtmlMode)
            aAtHorzPosED.SetModifyFlag();
    }
    else
    {
        if (nX != LONG_MAX)
            aAtHorzPosED.SetValue( aAtHorzPosED.Normalize(nX), FUNIT_TWIP );
    }
    aAtHorzPosFT.Enable( bEnable );
    aAtHorzPosED.Enable( bEnable );

    bEnable = nV == SVX_VERT_NONE && !bFormat;
    if ( !bEnable )
    {
        aAtVertPosED.SetValue( 0, FUNIT_TWIP );
        if(nY != LONG_MAX && bHtmlMode)
            aAtVertPosED.SetModifyFlag();
    }
    else
    {
        if ( nId == FLY_IN_CNTNT )
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            aAtVertPosED.SetValue( aAtVertPosED.Normalize(nY), FUNIT_TWIP );
    }
    aAtVertPosFT.Enable( bEnable );
    aAtVertPosED.Enable( bEnable );
    UpdateExample();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFrmPage::FillPosLB(FrmMap *pMap, USHORT nAlign, ListBox &rLB)
{
    String sSelEntry, sOldEntry;
    sOldEntry = rLB.GetSelectEntry();

    rLB.Clear();

    // Listbox fuellen
    USHORT nCount = ::lcl_GetFrmMapCount(pMap);
    for (USHORT i = 0; pMap && i < nCount; ++i)
    {
//      #61359# Warum nicht von links/von innen bzw. von oben?
//      if (!bFormat || (pMap[i].nStrId != STR_FROMLEFT && pMap[i].nStrId != STR_FROMTOP))
        {
            USHORT nResId = aMirrorPagesCB.IsChecked() ? pMap[i].nMirrorStrId : pMap[i].nStrId;
            String sEntry(SW_RES(nResId));
            sEntry.EraseAllChars( '~' );
            if (rLB.GetEntryPos(sEntry) == LISTBOX_ENTRY_NOTFOUND)
                // bei zeichengebundenen Rahmen keine doppelten Eintraege einfuegen
                rLB.InsertEntry(sEntry);
            if (nAlign == pMap[i].nAlign)
                sSelEntry = sEntry;
        }
    }

    rLB.SelectEntry(sSelEntry);
    if (!rLB.GetSelectEntryCount())
        rLB.SelectEntry(sOldEntry);

    if (!rLB.GetSelectEntryCount())
        rLB.SelectEntryPos(0);

    PosHdl(&rLB);

    return GetMapPos(pMap, rLB);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ULONG SwFrmPage::FillRelLB(FrmMap *pMap, USHORT nMapPos, USHORT nAlign, USHORT nRel, ListBox &rLB)
{
    String sSelEntry;
    ULONG  nLBRelations = 0;
    USHORT nMapCount = ::lcl_GetFrmMapCount(pMap);

    rLB.Clear();

    if (nMapPos < nMapCount)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            String sOldEntry(rLB.GetSelectEntry());
            USHORT nRelCount = sizeof(aAsCharRelationMap) / sizeof(RelationMap);
            USHORT nStrId = pMap[nMapPos].nStrId;

            for (USHORT nMapPos = 0; nMapPos < nMapCount; nMapPos++)
            {
                if (pMap[nMapPos].nStrId == nStrId)
                {
                    nLBRelations = pMap[nMapPos].nLBRelations;
                    for (USHORT nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            USHORT nResId = aAsCharRelationMap[nRelPos].nStrId;

                            String sEntry(SW_RES(nResId));
                            USHORT nPos = rLB.InsertEntry(sEntry);
                            rLB.SetEntryData(nPos, &aAsCharRelationMap[nRelPos]);
                            if (pMap[nMapPos].nAlign == nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (sSelEntry.Len())
                rLB.SelectEntry(sSelEntry);
            else
            {
                rLB.SelectEntry(sOldEntry);

                if (!rLB.GetSelectEntryCount())
                {
                    for (USHORT i = 0; i < rLB.GetEntryCount(); i++)
                    {
                        RelationMap *pEntry = (RelationMap *)rLB.GetEntryData(i);
                        if (pEntry->nLBRelation == LB_REL_CHAR) // Default
                        {
                            rLB.SelectEntryPos(i);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            USHORT nRelCount = sizeof(aRelationMap) / sizeof(RelationMap);

            nLBRelations = pMap[nMapPos].nLBRelations;

            for (ULONG nBit = 1; nBit < 0x80000000; nBit <<= 1)
            {
                if (nLBRelations & nBit)
                {
                    for (USHORT nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == nBit)
                        {
                            USHORT nResId = aMirrorPagesCB.IsChecked() ? aRelationMap[nRelPos].nMirrorStrId : aRelationMap[nRelPos].nStrId;
                            String sEntry(SW_RES(nResId));
                            USHORT nPos = rLB.InsertEntry(sEntry);
                            rLB.SetEntryData(nPos, &aRelationMap[nRelPos]);
                            if (!sSelEntry.Len() && aRelationMap[nRelPos].nRelation == nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (sSelEntry.Len())
                rLB.SelectEntry(sSelEntry);
            else
            {
                // Warscheinlich Ankerwechsel. Daher aehnliche Relation suchen
                switch (nRel)
                {
                    case FRAME:             nRel = REL_PG_FRAME;    break;
                    case PRTAREA:           nRel = REL_PG_PRTAREA;  break;
                    case REL_PG_LEFT:       nRel = REL_FRM_LEFT;    break;
                    case REL_PG_RIGHT:      nRel = REL_FRM_RIGHT;   break;
                    case REL_FRM_LEFT:      nRel = REL_PG_LEFT;     break;
                    case REL_FRM_RIGHT:     nRel = REL_PG_RIGHT;    break;
                    case REL_PG_FRAME:      nRel = FRAME;           break;
                    case REL_PG_PRTAREA:    nRel = PRTAREA;         break;

                    default:
                        if (rLB.GetEntryCount())
                        {
                            RelationMap *pEntry = (RelationMap *)rLB.GetEntryData(rLB.GetEntryCount() - 1);
                            nRel = pEntry->nRelation;
                        }
                        break;
                }

                for (USHORT i = 0; i < rLB.GetEntryCount(); i++)
                {
                    RelationMap *pEntry = (RelationMap *)rLB.GetEntryData(i);
                    if (pEntry->nRelation == nRel)
                    {
                        rLB.SelectEntryPos(i);
                        break;
                    }
                }

                if (!rLB.GetSelectEntryCount())
                    rLB.SelectEntryPos(0);
            }
        }
    }

    rLB.Enable(rLB.GetEntryCount() != 0);

    RelHdl(&rLB);

    return nLBRelations;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFrmPage::GetRelation(FrmMap *pMap, ListBox &rRelationLB)
{
    USHORT nRel = 0;
    USHORT nPos = rRelationLB.GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        RelationMap *pEntry = (RelationMap *)rRelationLB.GetEntryData(nPos);
        nRel = pEntry->nRelation;
    }

    return nRel;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFrmPage::GetAlignment(FrmMap *pMap, USHORT nMapPos, ListBox &rAlignLB, ListBox &rRelationLB)
{
    USHORT nAlign = 0;

    if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
    {
        if (rRelationLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            ULONG  nRel = ((RelationMap *)rRelationLB.GetEntryData(rRelationLB.GetSelectEntryPos()))->nLBRelation;
            USHORT nMapCount = ::lcl_GetFrmMapCount(pMap);
            USHORT nStrId = pMap[nMapPos].nStrId;

            for (USHORT i = 0; i < nMapCount; i++)
            {
                if (pMap[i].nStrId == nStrId)
                {
                    ULONG nLBRelations = pMap[i].nLBRelations;
                    if (nLBRelations & nRel)
                    {
                        nAlign = pMap[i].nAlign;
                        break;
                    }
                }
            }
        }
    }
    else if (pMap)
        nAlign = pMap[nMapPos].nAlign;

    return nAlign;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFrmPage::GetMapPos(FrmMap *pMap, ListBox &rAlignLB)
{
    USHORT nMapPos = 0;
    USHORT nLBSelPos = rAlignLB.GetSelectEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            USHORT nMapCount = ::lcl_GetFrmMapCount(pMap);
            String sSelEntry(rAlignLB.GetSelectEntry());

            for (USHORT i = 0; i < nMapCount; i++)
            {
                USHORT nResId = pMap[i].nStrId;

                String sEntry(SW_RES(nResId));
                sEntry.EraseAllChars( '~' );

                if (sEntry == sSelEntry)
                {
                    nMapPos = i;
                    break;
                }
            }
        }
        else
            nMapPos = nLBSelPos;
    }

    return nMapPos;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFrmPage::GetAnchor()
{
    if (aAnchorTypeRB.IsChecked())
        return (USHORT)(ULONG)aAnchorTypeLB.GetEntryData(aAnchorTypeLB.GetSelectEntryPos());
    else
        return FLY_IN_CNTNT;
}

/*--------------------------------------------------------------------
    Beschreibung:   Bsp - Update
 --------------------------------------------------------------------*/

void SwFrmPage::ActivatePage(const SfxItemSet& rSet)
{
    bNoModifyHdl = TRUE;
    Init(rSet);
    bNoModifyHdl = FALSE;
    RangeModifyHdl(&aWidthED);  // Alle Maximalwerte initial setzen
}

int SwFrmPage::DeactivatePage(SfxItemSet * pSet)
{
    if ( pSet )
    {
        FillItemSet( *pSet );

        //FillItemSet setzt den Anker nicht in den Set, wenn er mit dem
        //Original uebereinstimmt. Fuer die anderen Pages brauchen wir aber
        //den aktuellen Anker.
        SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                            : ((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();
        RndStdIds eAnchorId = (RndStdIds)GetAnchor();
        SwFmtAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
        pSet->Put( aAnc );
    }

    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung: Links/rechts mit Innen/aussen tauschen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, MirrorHdl, CheckBox *, pBtn )
{
    USHORT nId = GetAnchor();
    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, RelSizeClickHdl, CheckBox *, pBtn )
{
    if (pBtn == &aRelWidthCB)
        aWidthED.ShowPercent(pBtn->IsChecked());
    else
        aHeightED.ShowPercent(pBtn->IsChecked());

    if (pBtn)   // Nur wenn Handler durch Aenderung des Controllers gerufen wurde
        RangeModifyHdl(&aWidthED);  // Werte wieder korrigieren

    if (pBtn == &aRelWidthCB)
        ModifyHdl(&aWidthED);
    else
        ModifyHdl(&aHeightED);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Bereichspruefung
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, RangeModifyHdl, Edit *, pEdit )
{
    if (bNoModifyHdl)
        return 0;

    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                        :((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();
    ASSERT(pSh , "shell not found");
    SwFlyFrmAttrMgr aMgr( bNew, pSh, (const SwAttrSet&)GetItemSet() );
    SwFrmValid      aVal;

    aVal.eArea = (RndStdIds)GetAnchor();
    aVal.bAuto = aAutoHeightCB.IsChecked();
    aVal.bMirror = aMirrorPagesCB.IsChecked();

    if ( pHMap )
    {
        // Ausrichtung Horizontal
        USHORT nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        USHORT nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        USHORT nRel = GetRelation(pHMap, aHoriRelationLB);

        aVal.eHori = (SwHoriOrient)nAlign;
        aVal.eHRel = (SwRelationOrient)nRel;
    }
    else
        aVal.eHori = HORI_NONE;

    if ( pVMap )
    {
        // Ausrichtung Vertikal
        USHORT nMapPos = GetMapPos(pVMap, aVerticalDLB);
        USHORT nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        USHORT nRel = GetRelation(pVMap, aVertRelationLB);

        aVal.eVert = (SvxFrameVertOrient)nAlign;
        aVal.eVRel = (SwRelationOrient)nRel;
    }
    else
        aVal.eVert = SVX_VERT_NONE;

    const long nAtHorzPosVal =
                    aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP));
    const long nAtVertPosVal =
                    aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP));

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, TRUE);   // Einmal, um Referenzwerte fuer prozentuale Werte zu erhalten

    // Referenzwerte fuer fuer Prozentwerte setzen (100%) ...
    aWidthED.SetRefValue(aVal.aPercentSize.Width());
    aHeightED.SetRefValue(aVal.aPercentSize.Height());

    // ... und damit Breite und Hoehe korrekt umrechnen
    SwTwips nWidth = aWidthED. Denormalize(aWidthED.GetValue(FUNIT_TWIP));
    SwTwips nHeight = aHeightED.Denormalize(aHeightED.GetValue(FUNIT_TWIP));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    aMgr.ValidateMetrics(aVal); // Nochmal um mit korrekter Breite und Hoehe alle restlichen Werte zu ermitteln

    // alle Spalten muessen passen
    if(GetTabDialog()->GetExampleSet() &&
            SFX_ITEM_DEFAULT <= GetTabDialog()->GetExampleSet()->GetItemState(RES_COL))
    {
        const SwFmtCol& rCol = (const SwFmtCol&)GetTabDialog()->GetExampleSet()->Get(RES_COL);
        if ( rCol.GetColumns().Count() > 1 )
        {
            for ( USHORT i = 0; i < rCol.GetColumns().Count(); ++i )
            {
                aVal.nMinWidth += rCol.GetColumns()[i]->GetLeft() +
                                  rCol.GetColumns()[i]->GetRight() +
                                  MINFLY;
            }
            aVal.nMinWidth -= MINFLY;//einen hatten wir schon mit drin!
        }
    }

    nWidth = aVal.nWidth;
    nHeight = aVal.nHeight;

    // Mindestbreite auch fuer Vorlage
    aHeightED.SetMin(aHeightED.Normalize(aVal.nMinHeight), FUNIT_TWIP);
    aWidthED. SetMin(aWidthED.Normalize(aVal.nMinWidth), FUNIT_TWIP);

    SwTwips nMaxWidth(aVal.nMaxWidth);
    SwTwips nMaxHeight(aVal.nMaxHeight);

    if (aVal.bAuto && (nDlgType == DLG_FRM_GRF || nDlgType == DLG_FRM_OLE))
    {
        SwTwips nTmp = Min(nWidth * nMaxHeight / Max(nHeight, 1L), nMaxHeight);
        aWidthED.SetMax(aWidthED.Normalize(nTmp), FUNIT_TWIP);

        nTmp = Min(nHeight * nMaxWidth / Max(nWidth, 1L), nMaxWidth);
        aHeightED.SetMax(aWidthED.Normalize(nTmp), FUNIT_TWIP);
    }
    else
    {
        SwTwips nTmp = aHeightED.Normalize(nMaxHeight);
        aHeightED.SetMax(nTmp, FUNIT_TWIP);

        nTmp = aWidthED.Normalize(nMaxWidth);
        aWidthED.SetMax(nTmp, FUNIT_TWIP);
    }

    aAtHorzPosED.SetMin(aAtHorzPosED.Normalize(aVal.nMinHPos), FUNIT_TWIP);
    aAtHorzPosED.SetMax(aAtHorzPosED.Normalize(aVal.nMaxHPos), FUNIT_TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        aAtHorzPosED.SetValue(aAtHorzPosED.Normalize(aVal.nHPos), FUNIT_TWIP);

    SwTwips nUpperOffset = aVal.eArea == FLY_IN_CNTNT ? nUpperBorder : 0;
    SwTwips nLowerOffset = aVal.eArea == FLY_IN_CNTNT ? nLowerBorder : 0;

    aAtVertPosED.SetMin(aAtVertPosED.Normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset), FUNIT_TWIP);
    aAtVertPosED.SetMax(aAtVertPosED.Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        aAtVertPosED.SetValue(aAtVertPosED.Normalize(aVal.nVPos), FUNIT_TWIP);

    if (pEdit == &aWidthED)
        bWidthLastChanged = TRUE;
    else if (pEdit == &aHeightED)
        bWidthLastChanged = FALSE;
    return 0;
}

IMPL_LINK( SwFrmPage, TypHdl, ListBox *, pLB )
{
    if (pLB == &aAnchorTypeLB)
        aAnchorTypeRB.Check();

    aMirrorPagesCB.Enable(aAnchorTypeRB.IsChecked());

    USHORT nId = GetAnchor();

    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    if (pLB)    // Nur wenn Handler durch Aenderung des Controllers gerufen wurde
        RangeModifyHdl(0);

    if(bHtmlMode)
    {
        PosHdl(&aHorizontalDLB);
        PosHdl(&aVerticalDLB);
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, PosHdl, ListBox *, pLB )
{
    BOOL bHori = pLB == &aHorizontalDLB;
    ListBox *pRelLB = bHori ? &aHoriRelationLB : &aVertRelationLB;
    FrmMap *pMap = bHori ? pHMap : pVMap;

    USHORT nLBSelPos = pLB->GetSelectEntryPos();
    //BOOL bEnable = nLBSelPos == pLB->GetEntryCount()-1 && !bFormat;

    USHORT nMapPos = GetMapPos(pMap, *pLB);
    USHORT nAlign = GetAlignment(pMap, nMapPos, *pLB, *pRelLB);

    if (bHori)
    {
        BOOL bEnable = HORI_NONE == nAlign && !bFormat;
        aAtHorzPosED.Enable( bEnable );
        aAtHorzPosFT.Enable( bEnable );
    }
    else
    {
        BOOL bEnable = SVX_VERT_NONE == nAlign && !bFormat;
        aAtVertPosED.Enable( bEnable );
        aAtVertPosFT.Enable( bEnable );
    }

    if (pLB)    // Nur wenn Handler durch Aenderung des Controllers gerufen wurde
        RangeModifyHdl( 0 );

    USHORT nRel = 0;
    if (pLB->GetSelectEntryCount())
    {

        if (pRelLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
            nRel = ((RelationMap *)pRelLB->GetEntryData(pRelLB->GetSelectEntryPos()))->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB);
    }
    else
        pRelLB->Clear();

    UpdateExample();

    if (bHori)
        bAtHorzPosModified = TRUE;
    else
        bAtVertPosModified = TRUE;

    // Sonderbehandlung fuer HTML-Mode mit horz-vert-Abhaengigkeiten
    if(bHtmlMode && nHtmlMode&HTMLMODE_SOME_ABS_POS &&
            FLY_AUTO_CNTNT == GetAnchor())
    {
        BOOL bSet = FALSE;
        if(bHori)
        {
            // rechts ist nur unterhalb erlaubt - von links nur oben
            // von links am Zeichen -> unterhalb
            if((HORI_LEFT == nAlign || HORI_RIGHT == nAlign) &&
                    0 == aVerticalDLB.GetSelectEntryPos())
            {
                if(FRAME == nRel)
                    aVerticalDLB.SelectEntryPos(1);
                else
                    aVerticalDLB.SelectEntryPos(0);
                bSet = TRUE;
            }
            else if(HORI_LEFT == nAlign && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
                bSet = TRUE;
            }
            else if(HORI_NONE == nAlign && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
                bSet = TRUE;
            }
            if(bSet)
                PosHdl(&aVerticalDLB);
        }
        else
        {
            if(SVX_VERT_TOP == nAlign)
            {
                if(1 == aHorizontalDLB.GetSelectEntryPos())
                {
                    aHorizontalDLB.SelectEntryPos(0);
                    bSet = TRUE;
                }
                aHoriRelationLB.SelectEntryPos(1);
            }
            else if(SVX_VERT_CHAR_BOTTOM == nAlign)
            {
                if(2 == aHorizontalDLB.GetSelectEntryPos())
                {
                    aHorizontalDLB.SelectEntryPos(0);
                    bSet = TRUE;
                }
                aHoriRelationLB.SelectEntryPos(0) ;
            }
            if(bSet)
                PosHdl(&aHorizontalDLB);
        }

    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Horizontale Pos
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, RelHdl, ListBox *, pLB )
{
    BOOL bHori = pLB == &aHoriRelationLB;

    UpdateExample();

    if (bHori)
        bAtHorzPosModified = TRUE;
    else
        bAtVertPosModified = TRUE;

    if (!bHori && pVMap == aVCharMap)
    {
        // Ausrichtung Vertikal
        String sEntry;
        USHORT nMapPos = GetMapPos(pVMap, aVerticalDLB);
        USHORT nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        USHORT nRel = GetRelation(pVMap, aVertRelationLB);

        if (nRel == REL_CHAR)
            sEntry = SW_RESSTR(STR_FROMBOTTOM);
        else
            sEntry = SW_RESSTR(STR_FROMTOP);

        USHORT nOldPos = aVerticalDLB.GetSelectEntryPos();

        String sName = aVerticalDLB.GetEntry(aVerticalDLB.GetEntryCount() - 1);
        if (sName != sEntry)
        {
            aVerticalDLB.RemoveEntry(aVerticalDLB.GetEntryCount() - 1);
            aVerticalDLB.InsertEntry(sEntry);
            aVerticalDLB.SelectEntryPos(nOldPos);
        }
    }
    if(bHtmlMode  && FLY_AUTO_CNTNT == GetAnchor()) // wieder Sonderbehandlung
    {
        if(bHori)
        {
            USHORT nRel = GetRelation(pHMap, aHoriRelationLB);
            if(PRTAREA == nRel && 0 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(1);
            }
            else if(REL_CHAR == nRel && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
            }
        }
    }
    if (pLB)    // Nur wenn Handler durch Aenderung des Controllers gerufen wurde
        RangeModifyHdl(0);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFrmPage, AspectRatioCheckHdl, CheckBox *, pCB )
{
    if ( aFixedRatioCB.IsChecked() &&
            (aWidthED.IsValueModified() || aHeightED.IsValueModified()) &&
            nDlgType != DLG_FRM_GRF || (aGrfSize.Width() && aGrfSize.Height()) )
    {
        if (!aRelWidthCB.IsChecked() && !aRelHeightCB.IsChecked())
        {
            if (bWidthLastChanged)
            {
                SwTwips nHFac = aGrfSize.Height() * 1000 / aGrfSize.Width();
                SwTwips nTmp;
                if ( aWidthED.IsValueModified() )
                    nTmp = aWidthED.Denormalize(aWidthED.GetValue(FUNIT_TWIP));
                else
                {   //Rundungsfehler vermeiden
                    const SwFmtFrmSize &rSz = (const SwFmtFrmSize&)GetItemSet().
                                                                    Get(RES_FRM_SIZE);
                    nTmp = rSz.GetWidth();
                }
                nTmp = nTmp * nHFac / 1000;
                aHeightED.SetUserValue(aHeightED.Normalize(nTmp), FUNIT_TWIP);
            }
            else
            {
                SwTwips nVFac = aGrfSize.Width() * 1000 / aGrfSize.Height();
                SwTwips nTmp;
                if ( aHeightED.IsValueModified() )
                    nTmp = aHeightED.Denormalize(aHeightED.GetValue(FUNIT_TWIP));
                else
                {   //Rundungsfehler vermeiden
                    const SwFmtFrmSize &rSz = (const SwFmtFrmSize&)GetItemSet().
                                                                    Get(RES_FRM_SIZE);
                    nTmp = rSz.GetHeight();
                }
                nTmp = nTmp * nVFac / 1000;
                aWidthED.SetUserValue(aWidthED.Normalize(nTmp), FUNIT_TWIP);
            }
        }
        UpdateExample();
    }

    if (pCB)    // Nur wenn Handler durch Aenderung des Controllers gerufen wurde
        RangeModifyHdl(&aWidthED);  // Alle Maximalwerte neu initialisieren

    return 0;
}

IMPL_LINK_INLINE_START( SwFrmPage, RealSizeHdl, Button *, EMPTYARG )
{
    aWidthED.SetUserValue( aWidthED. Normalize(aGrfSize.Width() ), FUNIT_TWIP);
    aHeightED.SetUserValue(aHeightED.Normalize(aGrfSize.Height()), FUNIT_TWIP);
    UpdateExample();
    return 0;
}
IMPL_LINK_INLINE_END( SwFrmPage, RealSizeHdl, Button *, EMPTYARG )

IMPL_LINK_INLINE_START( SwFrmPage, ManualHdl, Button *, EMPTYARG )
{
    return 0;
}
IMPL_LINK_INLINE_END( SwFrmPage, ManualHdl, Button *, EMPTYARG )

IMPL_LINK( SwFrmPage, ModifyHdl, Edit *, pEdit )
{
    if (pEdit == &aWidthED)
        bWidthLastChanged = TRUE;
    else if (pEdit == &aHeightED)
        bWidthLastChanged = FALSE;

    if ( aFixedRatioCB.IsChecked() )
    {
        BOOL bWidthRelative = aRelWidthCB.IsChecked();
        BOOL bHeightRelative = aRelHeightCB.IsChecked();
        if ( pEdit == &aWidthED )
        {
            const SwTwips nTmp = aHeightED.Normalize(aGrfSize.Height() *
                           aWidthED.Denormalize(aWidthED.GetValue(FUNIT_TWIP)) /
                                                Max(aGrfSize.Width(), 1L));
            const SwTwips nMaxHeight = bHeightRelative ?
                aHeightED.GetRefValue() * ( aHeightED.GetOldDigits() == 1 ? 10 : 100):
                            aHeightED.GetMax(FUNIT_TWIP);
            if(nMaxHeight >= nTmp)
                aHeightED.SetValue(nTmp, FUNIT_TWIP);
            else
            {
                //cut selected width
                const SwTwips nNewWidth = aWidthED.Normalize(aGrfSize.Width() *
                                aHeightED.Denormalize(nMaxHeight) /
                                Max(aGrfSize.Height(),1L));
                aWidthED.SetUserValue(nNewWidth, FUNIT_TWIP);
                //aHeightED.SetValue(nMaxHeight, FUNIT_TWIP);
                aHeightED.SetUserValue(nMaxHeight, FUNIT_TWIP);

            }
        }
        else
        {
            const SwTwips nTmp = aWidthED.Normalize(aGrfSize.Width() *
                            aHeightED.Denormalize(aHeightED.GetValue(FUNIT_TWIP)) /
                            Max(aGrfSize.Height(),1L));
            const SwTwips nMaxWidth = bWidthRelative ?
                            aWidthED.GetRefValue() * (aWidthED.GetOldDigits()== 1 ? 10 : 100):
                            aWidthED.GetMax(FUNIT_TWIP);
            if(nMaxWidth >= nTmp)
                aWidthED.SetValue(nTmp, FUNIT_TWIP);
            else
            {
                //cut selected height
                const SwTwips nNewHeight = aHeightED.Normalize(aGrfSize.Height() *
                                aWidthED.Denormalize(nMaxWidth) /
                                Max(aGrfSize.Width(),1L));
                aHeightED.SetUserValue(nNewHeight, FUNIT_TWIP);
                //aWidthED.SetValue(nMaxWidth, FUNIT_TWIP);
                aWidthED.SetUserValue(nMaxWidth, FUNIT_TWIP);

            }
        }
    }

    UpdateExample();
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFrmPage::UpdateExample()
{
    USHORT nPos = aHorizontalDLB.GetSelectEntryPos();
    if ( pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        USHORT nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        USHORT nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        USHORT nRel = GetRelation(pHMap, aHoriRelationLB);

        aExampleWN.SetHAlign(nAlign);
        aExampleWN.SetHoriRel(nRel);
    }

    nPos = aVerticalDLB.GetSelectEntryPos();
    if ( pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        USHORT nMapPos = GetMapPos(pVMap, aVerticalDLB);
        USHORT nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        USHORT nRel = GetRelation(pVMap, aVertRelationLB);

        aExampleWN.SetVAlign(nAlign);
        aExampleWN.SetVertRel(nRel);
    }

    // Size
    long nXPos = aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP));
    long nYPos = aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP));
    aExampleWN.SetRelPos(Point(nXPos, nYPos));

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();
    aExampleWN.SetAnchor( eAnchorId );
    aExampleWN.Invalidate();
}

void SwFrmPage::Init(const SfxItemSet& rSet, BOOL bReset)
{
    if(!bFormat)
    {
        SwWrtShell* pSh = ((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();

        // Size
        const FASTBOOL bSizeFixed = pSh->IsSelObjProtected( FLYPROTECT_FIXED );

        aWidthED .Enable( !bSizeFixed );
        aHeightED.Enable( !bSizeFixed );
    }

    const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
    long nWidth = aWidthED.Normalize(rSize.GetWidth());
    long nHeight = aHeightED.Normalize(rSize.GetHeight());

    if (nWidth != aWidthED.GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // Wert wurde von Umlauf-Tabpage geaendert und muss
            // mit Modify-Flag gesetzt werden
            aWidthED.SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            aWidthED.SetValue(nWidth, FUNIT_TWIP);
    }

    if (nHeight != aHeightED.GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // Wert wurde von Umlauf-Tabpage geaendert und muss
            // mit Modify-Flag gesetzt werden
            aHeightED.SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            aHeightED.SetValue(nHeight, FUNIT_TWIP);
    }

    if (nDlgType != DLG_FRM_GRF && nDlgType != DLG_FRM_OLE)
    {
        aAutoHeightCB.Check(rSize.GetSizeType() != ATT_FIX_SIZE);
        if ( !bFormat )
        {
            SwWrtShell* pSh = ((SwFrmDlg*)GetParent()->GetParent())->GetWrtShell();
            const SwFrmFmt* pFmt = pSh->GetFlyFrmFmt();
            if( pFmt && pFmt->GetChain().GetNext() )
                aAutoHeightCB.Enable( FALSE );
        }
    }
    else
        aAutoHeightCB.Hide();

    // Umlauf-Abstand fuer zeichengebundene Rahmen organisieren
    const SvxULSpaceItem &rUL = (const SvxULSpaceItem &)rSet.Get(RES_UL_SPACE);
    nUpperBorder = rUL.GetUpper();
    nLowerBorder = rUL.GetLower();

    if(SFX_ITEM_SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        aFixedRatioCB.Check(((const SfxBoolItem&)rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        aFixedRatioCB.SaveValue();
    }

    // Spalten
    SwFmtCol aCol( (const SwFmtCol&)rSet.Get(RES_COL) );
    ::FitToActualSize( aCol, (USHORT)rSize.GetWidth() );

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();

    if ( bNew && !bFormat )
        InitPos (eAnchorId, USHRT_MAX, 0, USHRT_MAX, USHRT_MAX, LONG_MAX, LONG_MAX);
    else
    {
        const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)rSet.Get(RES_HORI_ORIENT);
        const SwFmtVertOrient& rVert = (const SwFmtVertOrient&)rSet.Get(RES_VERT_ORIENT);
        nOldH    = rHori.GetHoriOrient();
        nOldHRel = rHori.GetRelationOrient();
        nOldV    = rVert.GetVertOrient(),
        nOldVRel = rVert.GetRelationOrient();

        if (eAnchorId == FLY_PAGE)
        {
            if (nOldHRel == FRAME)
                nOldHRel = REL_PG_FRAME;
            else if (nOldHRel == PRTAREA)
                nOldHRel = REL_PG_PRTAREA;
            if (nOldVRel == FRAME)
                nOldVRel = REL_PG_FRAME;
            else if (nOldVRel == PRTAREA)
                nOldVRel = REL_PG_PRTAREA;
        }

        aMirrorPagesCB.Check(rHori.IsPosToggle());
        aMirrorPagesCB.SaveValue();

        InitPos(eAnchorId,
                nOldH,
                nOldHRel,
                nOldV,
                nOldVRel,
                rHori.GetPos(),
                rVert.GetPos());
    }

    // Transparent fuers Bsp
    // Umlauf fuers Bsp
    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);
    aExampleWN.SetWrap ( rSurround.GetSurround() );

    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
    {
        const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)rSet.Get(RES_OPAQUE);
        aExampleWN.SetTransparent(!rOpaque.GetValue());
    }

    // ggf auf Prozent umschalten
    RangeModifyHdl(&aWidthED);  // Referenzwerte setzen (fuer 100%)

    if (rSize.GetWidthPercent() == 0xff || rSize.GetHeightPercent() == 0xff)
        aFixedRatioCB.Check(TRUE);
    if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != 0xff &&
        !aRelWidthCB.IsChecked())
    {
        aRelWidthCB.Check(TRUE);
        RelSizeClickHdl(&aRelWidthCB);
        aWidthED.SetValue(rSize.GetWidthPercent(), FUNIT_CUSTOM);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != 0xff &&
        !aRelHeightCB.IsChecked())
    {
        aRelHeightCB.Check(TRUE);
        RelSizeClickHdl(&aRelHeightCB);
        aHeightED.SetValue(rSize.GetHeightPercent(), FUNIT_CUSTOM);
    }
    aRelWidthCB.SaveValue();
    aRelHeightCB.SaveValue();
}

USHORT* SwFrmPage::GetRanges()
{
    return aPageRg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwGrfExtPage::SwGrfExtPage(Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage( pParent, SW_RES(TP_GRF_EXT), rSet ),
    aBmpWin                 (this, WN_BMP, Graphic(), Bitmap(BMP_EXAMPLE) ),
    aConnectGB              (this, SW_RES( GB_CONNECT )),
    aConnectFT              (this, SW_RES( FT_CONNECT )),
    aConnectED              (this, SW_RES( ED_CONNECT )),
    aBrowseBT               (this, SW_RES( PB_BROWSE )),
    aMirrorVertBox          (this, SW_RES( CB_VERT )),
    aMirrorHorzBox          (this, SW_RES( CB_HOR )),
//  aMirrorPagesLB          (this, SW_RES( LB_MIRROR_PAGES )),
//  aMirrorPagesFT          (this, SW_RES( FT_MIRROR_PAGES )),
    aAllPagesRB(            this, SW_RES( RB_MIRROR_ALL_PAGES )),
    aLeftPagesRB(           this, SW_RES( RB_MIRROR_LEFT_PAGES )),
    aRightPagesRB(          this, SW_RES( RB_MIRROR_RIGHT_PAGES )),

    aMirrorFrm              (this, SW_RES( GB_MIRROR )),
    pGrfDlg( 0 )
{
    FreeResource();
    SetExchangeSupport();
    aMirrorHorzBox.SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    aMirrorVertBox.SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    aBrowseBT.SetClickHdl    ( LINK(this, SwGrfExtPage, BrowseHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwGrfExtPage::~SwGrfExtPage()
{
    delete pGrfDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwGrfExtPage::Create( Window *pParent, const SfxItemSet &rSet )
{
    return new SwGrfExtPage( pParent, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwGrfExtPage::Reset(const SfxItemSet &rSet)
{
    const SfxPoolItem* pItem;
    USHORT nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_GRF_CONNECT, TRUE, &pItem)
        && ((const SfxBoolItem *)pItem)->GetValue() )
    {
        aBrowseBT.Enable();
        aConnectED.SetReadOnly(FALSE);
    }

    ActivatePage(rSet);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwGrfExtPage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxProtectItem& rProt = (const SvxProtectItem& )rSet.Get(RES_PROTECT);
    bProtCntnt = rProt.IsCntntProtected();

    const SfxPoolItem* pItem = 0;
    BOOL bEnable = FALSE;
    BOOL bEnableMirrorRB = FALSE;

    SfxItemState eState = rSet.GetItemState(RES_GRFATR_MIRRORGRF, TRUE, &pItem);
    if( SFX_ITEM_UNKNOWN != eState && !bProtCntnt && !bHtmlMode )
    {
        if( SFX_ITEM_SET != eState )
            pItem = &rSet.Get( RES_GRFATR_MIRRORGRF );

        bEnable = TRUE;

        USHORT eMirror = (GRFMIRROR) ((const SwMirrorGrf* )pItem)->GetValue();
        switch( eMirror )
        {
        case RES_DONT_MIRROR_GRF: break;
        case RES_MIRROR_GRF_VERT: aMirrorHorzBox.Check(TRUE); break;
        case RES_MIRROR_GRF_HOR:  aMirrorVertBox.Check(TRUE); break;
        case RES_MIRROR_GRF_BOTH: aMirrorHorzBox.Check(TRUE);
                                    aMirrorVertBox.Check(TRUE);
                                    break;
        }

        USHORT nPos = ((const SwMirrorGrf* )pItem)->IsGrfToggle() ? 1 : 0;
        nPos += (eMirror == RES_MIRROR_GRF_VERT || eMirror == RES_MIRROR_GRF_BOTH)
                 ? 2 : 0;

        bEnableMirrorRB = nPos != 0;

        switch (nPos)
        {
            case 1: // Auf linken bzw geraden Seiten spiegeln
                aLeftPagesRB.Check();
                aMirrorHorzBox.Check(TRUE);
                break;
            case 2: // Auf allen Seiten spiegeln
                aAllPagesRB.Check();
                break;
            case 3: // Auf rechten bzw ungeraden Seiten spiegeln
                aRightPagesRB.Check();
                break;
            default:
                aAllPagesRB.Check();
                break;
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, FALSE, &pItem ) )
    {
        const SvxBrushItem& rBrush = *(SvxBrushItem*)pItem;
        if( rBrush.GetGraphicLink() )
        {
            aGrfName = aNewGrfName = *rBrush.GetGraphicLink();
            aConnectED.SetText( aNewGrfName );
        }
        const Graphic* pGrf = rBrush.GetGraphic();
        if( pGrf )
            aBmpWin.SetGraphic( *pGrf );
    }

    aAllPagesRB .Enable(bEnableMirrorRB);
    aLeftPagesRB.Enable(bEnableMirrorRB);
    aRightPagesRB.Enable(bEnableMirrorRB);
    aMirrorHorzBox.Enable(bEnable);
    aMirrorVertBox.Enable(bEnable);
    aMirrorFrm.Enable(bEnable);

    aAllPagesRB .SaveValue();
    aLeftPagesRB.SaveValue();
    aRightPagesRB.SaveValue();
    aMirrorHorzBox.SaveValue();
    aMirrorVertBox.SaveValue();

    aBmpWin.MirrorHorz( aMirrorVertBox.IsChecked() );
    aBmpWin.MirrorVert( aMirrorHorzBox.IsChecked() );
    aBmpWin.Invalidate();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwGrfExtPage::FillItemSet( SfxItemSet &rSet )
{
    BOOL bModified = FALSE;
    if ( aMirrorHorzBox.GetSavedValue() != aMirrorHorzBox.IsChecked() ||
         aMirrorVertBox.GetSavedValue() != aMirrorVertBox.IsChecked() ||
         aAllPagesRB .GetSavedValue() != aAllPagesRB .IsChecked() ||
         aLeftPagesRB.GetSavedValue() != aLeftPagesRB.IsChecked() ||
         aRightPagesRB.GetSavedValue() != aRightPagesRB.IsChecked())
    {
        bModified = TRUE;

        BOOL bHori = FALSE;

        if (aMirrorHorzBox.IsChecked() &&
                !aLeftPagesRB.IsChecked())
            bHori = TRUE;

        GRFMIRROR eMirror;
        eMirror = aMirrorVertBox.IsChecked() && bHori ?
                    RES_MIRROR_GRF_BOTH : bHori ?
                    RES_MIRROR_GRF_VERT : aMirrorVertBox.IsChecked() ?
                    RES_MIRROR_GRF_HOR  : RES_DONT_MIRROR_GRF;

        BOOL bMirror = !aAllPagesRB.IsChecked();
        SwMirrorGrf aMirror( eMirror );
        aMirror.SetGrfToggle(bMirror );
        rSet.Put( aMirror );
    }

    if( aGrfName != aNewGrfName || aConnectED.IsModified() )
    {
        bModified = TRUE;
        aGrfName = aConnectED.GetText();
        rSet.Put( SvxBrushItem( aGrfName, aFilterName, GPOS_LT,
                                SID_ATTR_GRAF_GRAPHIC ));
    }
    return bModified;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SwGrfExtPage::DeactivatePage(SfxItemSet *pSet)
{
    if( pSet )
        FillItemSet( *pSet );
    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwGrfExtPage, BrowseHdl, Button *, EMPTYARG )
{
    if(!pGrfDlg)
        pGrfDlg = new SvxImportGraphicDialog( this,
                            SW_RESSTR(STR_EDIT_GRF), ENABLE_STANDARD );
    pGrfDlg->SetPath( aConnectED.GetText(), FALSE, TRUE );

    if ( pGrfDlg->Execute() == RET_OK )
    {   // ausgewaehlten Filter merken
        aFilterName = pGrfDlg->GetCurFilter();
        aNewGrfName = INetURLObject::decode( pGrfDlg->GetPath(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 );
        aConnectED.SetModifyFlag();
        aConnectED.SetText( aNewGrfName );
        //Spiegeln zuruecksetzen, da u. U. eine Bitmap
        //gegen eine andere Art von Grafik ausgetauscht worden ist,
        //die nicht gespiegelt werden kann.
        aMirrorVertBox.Check(FALSE);
        aMirrorHorzBox.Check(FALSE);
        aAllPagesRB .Enable(FALSE);
        aLeftPagesRB.Enable(FALSE);
        aRightPagesRB.Enable(FALSE);
        aBmpWin.MirrorHorz(FALSE);
        aBmpWin.MirrorVert(FALSE);
        BOOL bEnable = FALSE;
        Graphic* pGrf = pGrfDlg->GetGraphic();
        if(pGrf)
        {
            aBmpWin.SetGraphic(*pGrf);

            if( GRAPHIC_BITMAP      == pGrf->GetType() ||
                GRAPHIC_GDIMETAFILE == pGrf->GetType())
                bEnable = TRUE;
        }
        aMirrorVertBox.Enable(bEnable);
        aMirrorHorzBox.Enable(bEnable);
        aAllPagesRB .Enable(bEnable);
        aLeftPagesRB.Enable(bEnable);
        aRightPagesRB.Enable(bEnable);
    }
    return 0;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwGrfExtPage, MirrorHdl, CheckBox *, EMPTYARG )
{
    BOOL bHori = FALSE;
    BOOL bEnable = aMirrorHorzBox.IsChecked();

    aBmpWin.MirrorHorz( aMirrorVertBox.IsChecked() );
    aBmpWin.MirrorVert( bEnable );

    aAllPagesRB .Enable(bEnable);
    aLeftPagesRB.Enable(bEnable);
    aRightPagesRB.Enable(bEnable);

    if (!aAllPagesRB.IsChecked() && !aLeftPagesRB.IsChecked() && !aRightPagesRB.IsChecked())
        aAllPagesRB.Check();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: BeispielWindow
 --------------------------------------------------------------------*/

BmpWindow::BmpWindow(Window* pPar, USHORT nId,
                    const Graphic& rGraphic, const Bitmap& rBmp) :
    Window(pPar, SW_RES(nId)),
    aGraphic(rGraphic),
    aBmp(rBmp),
    bHorz(FALSE),
    bVert(FALSE),
    bGraphic(FALSE),
    bLeftAlign(FALSE)
{
}

void BmpWindow::Paint( const Rectangle& )
{
    Point aPntPos;
    Size  aPntSz( GetSizePixel() );
    Size  aGrfSize;
    if(bGraphic)
        aGrfSize = ::GetGraphicSizeTwip(aGraphic, this);
    else
        aGrfSize =  PixelToLogic(aBmp.GetSizePixel());

    // u.U. ist nichts gesetzt
    if(aGrfSize.Width() && aGrfSize.Height())
    {
        long nRelGrf = aGrfSize.Width() * 100L / aGrfSize.Height();
        long nRelWin = aPntSz.Width() * 100L / aPntSz.Height();
        if(nRelGrf < nRelWin)
        {
            const long nWidth = aPntSz.Width();
            aPntSz.Width() = aPntSz.Height() * nRelGrf /100;
            if(!bLeftAlign)
                aPntPos.X() += nWidth - aPntSz.Width() ;
        }

        if ( bHorz )
        {
            aPntPos.Y()     += aPntSz.Height();
            aPntPos.Y() --;
            aPntSz.Height() *= -1;
        }
        if ( bVert )
        {
            aPntPos.X()     += aPntSz.Width();
            aPntPos.X()--;
            aPntSz.Width()  *= -1;
        }
        if(bGraphic)
            aGraphic.Draw(this, aPntPos, aPntSz);
        else
            DrawBitmap( aPntPos, aPntSz, aBmp );
    }
}

BmpWindow::~BmpWindow()
{
}

/***************************************************************************
    Beschreibung:   URL und ImageMap an Rahmen einstellen
***************************************************************************/


SwFrmURLPage::SwFrmURLPage( Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent,     SW_RES(TP_FRM_URL), rSet),
    aHyperLinkGB    (this, SW_RES( GB_HYPERLINK )),
    aURLFT          (this, SW_RES( FT_URL    )),
    aURLED          (this, SW_RES( ED_URL    )),
    aNameFT         (this, SW_RES( FT_NAME   )),
    aNameED         (this, SW_RES( ED_NAME   )),
    aFrameFT        (this, SW_RES( FT_FRAME   )),
    aFrameCB        (this, SW_RES( CB_FRAME   )),
    aSearchPB       (this, SW_RES( PB_SEARCH  )),
    aImageGB        (this, SW_RES( GB_IMAGE   )),
    aServerCB       (this, SW_RES( CB_SERVER  )),
    aClientCB       (this, SW_RES( CB_CLIENT  ))
{
    FreeResource();
    aSearchPB.SetClickHdl(LINK(this, SwFrmURLPage, InsertFileHdl));
}

/***************************************************************************
    Beschreibung:
***************************************************************************/

SwFrmURLPage::~SwFrmURLPage()
{
}

/***************************************************************************
    Beschreibung:
***************************************************************************/

void SwFrmURLPage::Reset( const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_DOCFRAME, TRUE, &pItem))
    {
        TargetList* pList = new TargetList;
        ((const SfxFrameItem*)pItem)->GetFrame()->GetTargetList(*pList);
        USHORT nCount = (USHORT)pList->Count();
        if( nCount )
        {
            for ( USHORT i = 0; i < nCount; i++ )
            {
                aFrameCB.InsertEntry(*pList->GetObject(i));
            }
            for ( i = nCount; i; i-- )
            {
                delete pList->GetObject( i - 1 );
            }
        }
        delete pList;
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( RES_URL, TRUE, &pItem ) )
    {
        const SwFmtURL* pFmtURL = (const SwFmtURL*)pItem;
        aURLED.SetText( INetURLObject::decode( pFmtURL->GetURL(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));
        aNameED.SetText( pFmtURL->GetName());

        aClientCB.Enable( pFmtURL->GetMap() != 0 );
        aClientCB.Check ( pFmtURL->GetMap() != 0 );
        aServerCB.Check ( pFmtURL->IsServerMap() );

        aFrameCB.SetText(pFmtURL->GetTargetFrameName());
        aFrameCB.SaveValue();
    }
    else
        aClientCB.Enable( FALSE );

    aServerCB.SaveValue();
    aClientCB.SaveValue();
}



/***************************************************************************
    Beschreibung:
***************************************************************************/

BOOL SwFrmURLPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bModified = FALSE;
    const SwFmtURL* pOldURL = (SwFmtURL*)GetOldItem(rSet, RES_URL);
    SwFmtURL* pFmtURL;
    if(pOldURL)
        pFmtURL = (SwFmtURL*)pOldURL->Clone();
    else
        pFmtURL = new SwFmtURL();

    {
        String sText = aURLED.GetText();
        if( sText.Len() )
            sText = URIHelper::SmartRelToAbs( sText );

        if( pFmtURL->GetURL() != sText ||
            pFmtURL->GetName() != aNameED.GetText() ||
            aServerCB.IsChecked() != pFmtURL->IsServerMap() )
        {
            pFmtURL->SetURL( sText, aServerCB.IsChecked() );
            pFmtURL->SetName( aNameED.GetText() );
            bModified = TRUE;
        }
    }

    if(!aClientCB.IsChecked() && pFmtURL->GetMap() != 0)
    {
        pFmtURL->SetMap(0);
        bModified = TRUE;
    }

    if(pFmtURL->GetTargetFrameName() != aFrameCB.GetText())
    {
        pFmtURL->SetTargetFrameName(aFrameCB.GetText());
        bModified = TRUE;
    }
    rSet.Put(*pFmtURL);
    delete pFmtURL;
    return bModified;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/

SfxTabPage* SwFrmURLPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmURLPage( pParent, rSet );
}

/***************************************************************************
    Beschreibung:
***************************************************************************/

IMPL_LINK( SwFrmURLPage, InsertFileHdl, PushButton *, pBtn )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog(pBtn, WB_OPEN);
    pFileDlg->DisableSaveLastDirectory();
    pFileDlg->SetHelpId(HID_FILEDLG_CHARDLG);
    String sTemp(aURLED.GetText());
    if(sTemp.Len())
        pFileDlg->SetPath(sTemp);

    if(RET_OK == pFileDlg->Execute())
    {
        aURLED.SetText( URIHelper::SmartRelToAbs( pFileDlg->GetPath(), FALSE,
                                        INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_WITH_CHARSET ));
    }
    delete pFileDlg;

    return 0;
}

/*-----------------13.11.96 13.15-------------------

--------------------------------------------------*/

SwFrmAddPage::SwFrmAddPage(Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent,     SW_RES(TP_FRM_ADD), rSet),
    aNameFT            (this, SW_RES(FT_NAME)),
    aNameED            (this, SW_RES(ED_NAME)),
    aAltNameFT         (this, SW_RES(FT_ALT_NAME)),
    aAltNameED         (this, SW_RES(ED_ALT_NAME)),
    aPrevFT            (this, SW_RES(FT_PREV)),
    aPrevED            (this, SW_RES(ED_PREV)),
    aNextFT            (this, SW_RES(FT_NEXT)),
    aNextED            (this, SW_RES(ED_NEXT)),
    aNamesGB           (this, SW_RES(GB_NAME)),

    aProtectContentCB  (this, SW_RES(CB_PROTECT_CONTENT)),
    aProtectFrameCB    (this, SW_RES(CB_PROTECT_FRAME)),
    aProtectSizeCB     (this, SW_RES(CB_PROTECT_SIZE)),
    aProtectGB         (this, SW_RES(GB_PROTECT)),

    aEditInReadonlyCB  (this, SW_RES(CB_EDIT_IN_READONLY)),
    aPrintFrameCB      (this, SW_RES(CB_PRINT_FRAME)),
    aExtGB             (this, SW_RES(GB_EXT)),
    nDlgType(0),
    pWrtSh(0),
    bFormat(FALSE),
    bHtmlMode(FALSE)
{
    FreeResource();

}

/*-----------------13.11.96 13.15-------------------

--------------------------------------------------*/

SwFrmAddPage::~SwFrmAddPage()
{
}

/*-----------------13.11.96 13.15-------------------

--------------------------------------------------*/

SfxTabPage* SwFrmAddPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmAddPage(pParent, rSet);
}

/*-----------------13.11.96 13.14-------------------

--------------------------------------------------*/

void SwFrmAddPage::Reset(const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    USHORT nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;
    if(bHtmlMode)
    {
        aProtectContentCB .Hide();
        aProtectFrameCB   .Hide();
        aProtectSizeCB    .Hide();
        aEditInReadonlyCB .Hide();
        aPrintFrameCB     .Hide();
        aExtGB            .Hide();
        aProtectGB.Hide();
    }
    if ( DLG_FRM_GRF == nDlgType || DLG_FRM_OLE == nDlgType )
        aEditInReadonlyCB.Hide();

    if(SFX_ITEM_SET == rSet.GetItemState(FN_SET_FRM_ALT_NAME, FALSE, &pItem))
    {
        aAltNameED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aAltNameED.SaveValue();
    }

    if(!bFormat)
    {
        // Grafik einfuegen - Eigenschaften
        // bNew ist nicht gesetzt, deswegen ueber Selektion erkennen
        String aTmpName1;
        if(SFX_ITEM_SET == rSet.GetItemState(FN_SET_FRM_NAME, FALSE, &pItem))
        {
            aTmpName1 = ((const SfxStringItem*)pItem)->GetValue();
        }

        DBG_ASSERT(pWrtSh, "keine Shell?")
        if( bNew || !aTmpName1.Len() )

            switch( nDlgType )
            {
                case DLG_FRM_GRF:
                    aTmpName1 = pWrtSh->GetUniqueGrfName();
                    break;
                case DLG_FRM_OLE:
                    aTmpName1 = pWrtSh->GetUniqueOLEName();
                    break;
                default:
                    aTmpName1 = pWrtSh->GetUniqueFrameName();
                    break;
            }

        aNameED.SetText( aTmpName1 );
        aNameED.SaveValue();
    }
    else
    {
        aNameED.Enable( FALSE );
        aAltNameED.Enable(FALSE);
        aNameFT.Enable( FALSE );
        aAltNameFT.Enable(FALSE);
        aNamesGB.Enable(FALSE);
    }
    if(nDlgType == DLG_FRM_STD)
    {
        aAltNameFT.Hide();
        aAltNameED.Hide();
    }
    else
    {
        aNameED.SetModifyHdl(LINK(this, SwFrmAddPage, EditModifyHdl));
    }

    BOOL bNoPrev = TRUE, bNoNext = TRUE;
    if (!bNew)
    {
        const SwFrmFmt* pFmt = pWrtSh->GetFlyFrmFmt();

        if (pFmt)
        {
            const SwFmtChain &rChain = pFmt->GetChain();
            const SwFlyFrmFmt* pFlyFmt;

            if ((pFlyFmt = rChain.GetPrev()) != 0)
            {
                aPrevED.SetText(pFlyFmt->GetName());
                bNoPrev = FALSE;
            }

            if ((pFlyFmt = rChain.GetNext()) != 0)
            {
                aNextED.SetText(pFlyFmt->GetName());
                bNoNext = FALSE;
            }
        }
    }

    if (bNoPrev)
    {
        aPrevFT.Disable();
        aPrevED.Disable();
    }
    if (bNoNext)
    {
        aNextFT.Disable();
        aNextED.Disable();
    }

    // Pos Protected
    const SvxProtectItem& rProt = (const SvxProtectItem& )rSet.Get(RES_PROTECT);
    aProtectFrameCB.Check(rProt.IsPosProtected());
    aProtectContentCB.Check(rProt.IsCntntProtected());
    aProtectSizeCB.Check(rProt.IsSizeProtected());

    const SwFmtEditInReadonly& rEdit = (const SwFmtEditInReadonly& )rSet.Get(RES_EDIT_IN_READONLY);
    aEditInReadonlyCB.Check(rEdit.GetValue());          aEditInReadonlyCB.SaveValue();

    // drucken
    const SvxPrintItem& rPrt = (const SvxPrintItem&)rSet.Get(RES_PRINT);
    aPrintFrameCB.Check(rPrt.GetValue());               aPrintFrameCB.SaveValue();
}

/*-----------------13.11.96 13.20-------------------

--------------------------------------------------*/

BOOL SwFrmAddPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bRet = FALSE;
    if (aNameED.GetText() != aNameED.GetSavedValue())
        bRet |= 0 != rSet.Put(SfxStringItem(FN_SET_FRM_NAME, aNameED.GetText()));
    if (aAltNameED.GetText()  != aAltNameED.GetSavedValue())
        bRet |= 0 != rSet.Put(SfxStringItem(FN_SET_FRM_ALT_NAME, aAltNameED.GetText()));

    const SfxPoolItem* pOldItem;
    SvxProtectItem aProt ( (const SvxProtectItem& )GetItemSet().Get(RES_PROTECT) );
    aProt.SetCntntProtect( aProtectContentCB.IsChecked() );
    aProt.SetSizeProtect ( aProtectSizeCB.IsChecked() );
    aProt.SetPosProtect  ( aProtectFrameCB.IsChecked() );
    if ( 0 == (pOldItem = GetOldItem(rSet, FN_SET_PROTECT)) ||
                aProt != *pOldItem )
        bRet |= 0 != rSet.Put( aProt);

    BOOL bChecked;
    if ( (bChecked = aEditInReadonlyCB.IsChecked()) != aEditInReadonlyCB.GetSavedValue() )
        bRet |= 0 != rSet.Put( SwFmtEditInReadonly( RES_EDIT_IN_READONLY, bChecked));

    if ( (bChecked = aPrintFrameCB.IsChecked()) != aPrintFrameCB.GetSavedValue() )
        bRet |= 0 != rSet.Put( SvxPrintItem( RES_PRINT, bChecked));

    return bRet;
}

/*-----------------13.11.96 15.05-------------------

--------------------------------------------------*/

USHORT* SwFrmAddPage::GetRanges()
{
    return aAddPgRg;
}

/*-----------------13.11.96 16.12-------------------

--------------------------------------------------*/

IMPL_LINK(SwFrmAddPage, EditModifyHdl, Edit*, EMPTYARG)
{
    BOOL bEnable = 0 != aNameED.GetText().Len();
    aAltNameED.Enable(bEnable);
    aAltNameFT.Enable(bEnable);

    return 0;
}
/*--------------------------------------------------------------------
   $Log: not supported by cvs2svn $
   Revision 1.255  2000/09/18 16:05:33  willem.vandorp
   OpenOffice header added.

   Revision 1.254  2000/08/25 14:09:49  jp
   Graphic Crop-Attribut and TabPage exported to SVX

   Revision 1.253  2000/08/17 11:38:50  jp
   remove the SW graphicmanager and UI with decoded URLs

   Revision 1.252  2000/06/26 13:13:31  os
   INetURLObject::SmartRelToAbs removed

   Revision 1.251  2000/05/10 14:05:18  os
   #75737# ReadioButtons used for hori mirror type

   Revision 1.250  2000/04/26 14:49:17  os
   GetName() returns const String&

   Revision 1.249  2000/04/19 12:56:34  os
   include sfx2/filedlg.hxx removed

   Revision 1.248  2000/04/18 15:14:56  os
   UNICODE

   Revision 1.247  2000/02/11 14:47:20  hr
   #70473# changes for unicode ( patched by automated patchtool )

   Revision 1.246  2000/01/24 12:45:28  os
   #72153# call SfxFileDialog::DisableSaveLastDirectory

   Revision 1.245  1999/11/23 13:18:55  os
   #69479# relative/fixed ratio handling corrected

   Revision 1.244  1999/06/21 11:30:30  OS
   #64885# width/height matching corrected


      Rev 1.243   21 Jun 1999 13:30:30   OS
   #64885# width/height matching corrected

      Rev 1.242   09 Jun 1999 10:48:10   OS
   #66733# CropPage: reset relative values

      Rev 1.241   09 Apr 1999 12:17:24   OS
   #64438# Grenzwerte fuer L+R richtig berechnen

--------------------------------------------------*/
