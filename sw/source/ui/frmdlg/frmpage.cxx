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

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>

#include <cmdid.h>
#include <helpid.h>
#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <svx/htmlmode.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/swframevalidation.hxx>
#include <sot/clsids.hxx>

#include <sfx2/viewfrm.hxx>
#include <fmturl.hxx>
#include <fmteiro.hxx>
#include <fmtcnct.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <uitool.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <frmdlg.hxx>
#include <frmmgr.hxx>
#include <frmpage.hxx>
#include <wrap.hxx>
#include <colmgr.hxx>
#include <grfatr.hxx>
#include <uiitems.hxx>
#include <fmtfollowtextflow.hxx>

#include <frmui.hrc>
#include <frmpage.hrc>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <svtools/filter.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
using namespace ::sfx2;

#define SwFPos SvxSwFramePosString

struct FrmMap
{
    SvxSwFramePosString::StringId eStrId;
    SvxSwFramePosString::StringId eMirrorStrId;
    sal_uInt16 nAlign;
    sal_uLong  nLBRelations;
};

struct RelationMap
{
    SvxSwFramePosString::StringId eStrId;
    SvxSwFramePosString::StringId eMirrorStrId;
    sal_uLong  nLBRelation;
    sal_uInt16 nRelation;
};

struct StringIdPair_Impl
{
    SvxSwFramePosString::StringId eHori;
    SvxSwFramePosString::StringId eVert;
};

#define MAX_PERCENT_WIDTH   254L
#define MAX_PERCENT_HEIGHT  254L

#define LB_FRAME                0x00000001L // text region of the paragraph
#define LB_PRTAREA              0x00000002L // text region of the paragraph + indentions
#define LB_VERT_FRAME           0x00000004L // vertical text region of the paragraph
#define LB_VERT_PRTAREA         0x00000008L // vertival text region of the paragraph + indentions
#define LB_REL_FRM_LEFT         0x00000010L // left paragraph edge
#define LB_REL_FRM_RIGHT        0x00000020L // right paragraph edge

#define LB_REL_PG_LEFT          0x00000040L // left page edge
#define LB_REL_PG_RIGHT         0x00000080L    // right page edge
#define LB_REL_PG_FRAME         0x00000100L // whole page
#define LB_REL_PG_PRTAREA       0x00000200L    // text region of the page

#define LB_FLY_REL_PG_LEFT      0x00000400L    // left frame edge
#define LB_FLY_REL_PG_RIGHT     0x00000800L    // right frame edge
#define LB_FLY_REL_PG_FRAME     0x00001000L    // whole frame
#define LB_FLY_REL_PG_PRTAREA   0x00002000L    // inside of the frame

#define LB_REL_BASE             0x00010000L // character alignment Base
#define LB_REL_CHAR             0x00020000L // character alignment Character
#define LB_REL_ROW              0x00040000L // character alignment Row

#define LB_FLY_VERT_FRAME       0x00100000L // vertical entire frame
#define LB_FLY_VERT_PRTAREA     0x00200000L // vertical frame text area

#define LB_VERT_LINE            0x00400000L // vertical text line

static RelationMap aRelationMap[] =
{
    {SwFPos::FRAME,  SwFPos::FRAME, LB_FRAME, text::RelOrientation::FRAME},
    {SwFPos::PRTAREA,           SwFPos::PRTAREA,                LB_PRTAREA,             text::RelOrientation::PRINT_AREA},
    {SwFPos::REL_PG_LEFT,       SwFPos::MIR_REL_PG_LEFT,        LB_REL_PG_LEFT,         text::RelOrientation::PAGE_LEFT},
    {SwFPos::REL_PG_RIGHT,      SwFPos::MIR_REL_PG_RIGHT,       LB_REL_PG_RIGHT,        text::RelOrientation::PAGE_RIGHT},
    {SwFPos::REL_FRM_LEFT,      SwFPos::MIR_REL_FRM_LEFT,       LB_REL_FRM_LEFT,        text::RelOrientation::FRAME_LEFT},
    {SwFPos::REL_FRM_RIGHT,     SwFPos::MIR_REL_FRM_RIGHT,      LB_REL_FRM_RIGHT,       text::RelOrientation::FRAME_RIGHT},
    {SwFPos::REL_PG_FRAME,      SwFPos::REL_PG_FRAME,           LB_REL_PG_FRAME,        text::RelOrientation::PAGE_FRAME},
    {SwFPos::REL_PG_PRTAREA,    SwFPos::REL_PG_PRTAREA,         LB_REL_PG_PRTAREA,      text::RelOrientation::PAGE_PRINT_AREA},
    {SwFPos::REL_CHAR,          SwFPos::REL_CHAR,               LB_REL_CHAR,            text::RelOrientation::CHAR},

    {SwFPos::FLY_REL_PG_LEFT,       SwFPos::FLY_MIR_REL_PG_LEFT,    LB_FLY_REL_PG_LEFT,     text::RelOrientation::PAGE_LEFT},
    {SwFPos::FLY_REL_PG_RIGHT,      SwFPos::FLY_MIR_REL_PG_RIGHT,   LB_FLY_REL_PG_RIGHT,    text::RelOrientation::PAGE_RIGHT},
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB_FLY_REL_PG_FRAME,    text::RelOrientation::PAGE_FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB_FLY_REL_PG_PRTAREA,  text::RelOrientation::PAGE_PRINT_AREA},

    {SwFPos::REL_BORDER,        SwFPos::REL_BORDER,             LB_VERT_FRAME,          text::RelOrientation::FRAME},
    {SwFPos::REL_PRTAREA,       SwFPos::REL_PRTAREA,            LB_VERT_PRTAREA,        text::RelOrientation::PRINT_AREA},

    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,   LB_FLY_VERT_FRAME,      text::RelOrientation::FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB_FLY_VERT_PRTAREA,    text::RelOrientation::PRINT_AREA},

    {SwFPos::REL_LINE,  SwFPos::REL_LINE,   LB_VERT_LINE,   text::RelOrientation::TEXT_LINE}
};

static RelationMap aAsCharRelationMap[] =
{
    {SwFPos::REL_BASE,  SwFPos::REL_BASE,   LB_REL_BASE,    text::RelOrientation::FRAME},
    {SwFPos::REL_CHAR,   SwFPos::REL_CHAR,   LB_REL_CHAR,   text::RelOrientation::FRAME},
    {SwFPos::REL_ROW,    SwFPos::REL_ROW,   LB_REL_ROW,     text::RelOrientation::FRAME}
};

/*--------------------------------------------------------------------
    Description: site anchored
 --------------------------------------------------------------------*/
#define HORI_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_PG_LEFT| \
                        LB_REL_PG_RIGHT)

static FrmMap aHPageMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrmMap aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

#define VERT_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrmMap aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB_REL_PG_FRAME}
};

/*--------------------------------------------------------------------
    Description: frame anchored
 --------------------------------------------------------------------*/
#define HORI_FRAME_REL  (LB_FLY_REL_PG_FRAME|LB_FLY_REL_PG_PRTAREA| \
                        LB_FLY_REL_PG_LEFT|LB_FLY_REL_PG_RIGHT)

static FrmMap aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrmMap aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB_FLY_REL_PG_FRAME},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_FLY_REL_PG_FRAME}
};

// own vertical alignment map for objects anchored to frame
#define VERT_FRAME_REL   (LB_FLY_VERT_FRAME|LB_FLY_VERT_PRTAREA)

static FrmMap aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrmMap aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB_FLY_VERT_FRAME},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB_FLY_VERT_FRAME}
};

/*--------------------------------------------------------------------
    Description: paragraph anchored
 --------------------------------------------------------------------*/
#define HORI_PARA_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT)

static FrmMap aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB_FRAME|LB_PRTAREA)

static FrmMap aHParaHtmlMap[] =
{
    {SwFPos::LEFT,  SwFPos::LEFT,   text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT, SwFPos::RIGHT,  text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

static FrmMap aHParaHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

// allow vertical alignment at page areas
#define VERT_PARA_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PARA_REL}
};

static FrmMap aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB_VERT_PRTAREA}
};

/*--------------------------------------------------------------------
    Description: anchored relative to the character
 --------------------------------------------------------------------*/
#define HORI_CHAR_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT|LB_REL_CHAR)

static FrmMap aHCharMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_CHAR_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_CHAR_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_CHAR_REL}
};

#define HTML_HORI_CHAR_REL  (LB_FRAME|LB_PRTAREA|LB_REL_CHAR)

static FrmMap aHCharHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::LEFT,           text::HoriOrientation::LEFT,      HTML_HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::RIGHT,          text::HoriOrientation::RIGHT,     HTML_HORI_CHAR_REL}
};

static FrmMap aHCharHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB_PRTAREA|LB_REL_CHAR},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     LB_PRTAREA},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

// allow vertical alignment at page areas
#define VERT_CHAR_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVCharMap[] =
{
    // introduce mappings for new vertical alignment at top of line <LB_VERT_LINE>
    // and correct mapping for vertical alignment at character for position <FROM_BOTTOM>
    // Note: Because of these adjustments the map becomes ambigous in its values
    //       <eStrId>/<eMirrorStrId> and <nAlign>. These ambiguities are considered
    //       in the methods <SwFrmPage::FillRelLB(..)>, <SwFrmPage::GetAlignment(..)>
    //       and <SwFrmPage::FillPosLB(..)>
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,           VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,        VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::BELOW,         SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,          VERT_CHAR_REL},
    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     text::VertOrientation::NONE,          LB_REL_CHAR|LB_VERT_LINE},
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB_VERT_LINE},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB_VERT_LINE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB_VERT_LINE}
};

static FrmMap aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};

static FrmMap aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,           LB_REL_CHAR},
    {SwFPos::BELOW,             SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};

/*--------------------------------------------------------------------
    Description: anchored as character
 --------------------------------------------------------------------*/
static FrmMap aVAsCharMap[] =
{
    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::TOP,           LB_REL_BASE},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,        LB_REL_BASE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        LB_REL_BASE},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::CHAR_TOP,      LB_REL_CHAR},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CHAR_CENTER,   LB_REL_CHAR},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB_REL_ROW},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB_REL_ROW},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB_REL_ROW},

    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     text::VertOrientation::NONE,          LB_REL_BASE}
};

static FrmMap aVAsCharHtmlMap[] =
{
    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::TOP,           LB_REL_BASE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        LB_REL_BASE},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::CHAR_TOP,      LB_REL_CHAR},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB_REL_ROW},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB_REL_ROW},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB_REL_ROW}
};

static sal_uInt16 aPageRg[] = {
    RES_FRM_SIZE, RES_FRM_SIZE,
    RES_VERT_ORIENT, RES_ANCHOR,
    RES_COL, RES_COL,
    RES_FOLLOW_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW,
    0
};
static sal_uInt16 aAddPgRg[] = {
    RES_PROTECT,            RES_PROTECT,
    RES_PRINT,              RES_PRINT,
    FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
    FN_SET_FRM_ALT_NAME,    FN_SET_FRM_ALT_NAME,
    0
};

static size_t lcl_GetFrmMapCount( const FrmMap* pMap)
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
        if ( pMap == aVFrameMap )
            return sizeof(aVFrameMap) / aSizeOf;
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

void lcl_InsertVectors(ListBox& rBox,
    const ::std::vector< String >& rPrev, const ::std::vector< String >& rThis,
    const ::std::vector< String >& rNext, const ::std::vector< String >& rRemain)
{
    ::std::vector< String >::const_iterator aIt;
    sal_uInt16 nEntry = 0;
    for(aIt = rPrev.begin(); aIt != rPrev.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    for(aIt = rThis.begin(); aIt != rThis.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    for(aIt = rNext.begin(); aIt != rNext.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    rBox.SetSeparatorPos(nEntry);
    //now insert all strings sorted
    sal_uInt16 nStartPos = rBox.GetEntryCount();

    for(aIt = rPrev.begin(); aIt != rPrev.end(); ++aIt)
        ::InsertStringSorted(*aIt, rBox, nStartPos );
    for(aIt = rThis.begin(); aIt != rThis.end(); ++aIt)
        ::InsertStringSorted(*aIt, rBox, nStartPos );
    for(aIt = rNext.begin(); aIt != rNext.end(); ++aIt)
        ::InsertStringSorted(*aIt, rBox, nStartPos );
    for(aIt = rRemain.begin(); aIt != rRemain.end(); ++aIt)
        ::InsertStringSorted(*aIt, rBox, nStartPos );
}

// --> OD 2009-08-31 #mongolianlayout#
// add input parameter
SvxSwFramePosString::StringId lcl_ChangeResIdToVerticalOrRTL(SvxSwFramePosString::StringId eStringId, sal_Bool bVertical, sal_Bool bVerticalL2R, sal_Bool bRTL)
{
    //special handling of STR_FROMLEFT
    if ( SwFPos::FROMLEFT == eStringId )
    {
        eStringId = bVertical
                    ? ( bRTL
                        ? SwFPos::FROMBOTTOM
                        : SwFPos::FROMTOP )
                    : ( bRTL
                        ? SwFPos::FROMRIGHT
                        : SwFPos::FROMLEFT );
        return eStringId;
    }
    // --> OD 2009-08-31 #mongolianlayout#
    // special handling of STR_FROMTOP in case of mongolianlayout (vertical left-to-right)
    if ( SwFPos::FROMTOP == eStringId &&
         bVertical && bVerticalL2R )
    {
        eStringId = SwFPos::FROMLEFT;
        return eStringId;
    }
    if ( bVertical )
    {
        //exchange horizontal strings with vertical strings and vice versa
        static const StringIdPair_Impl aHoriIds[] =
        {
            {SwFPos::LEFT,           SwFPos::TOP},
            {SwFPos::RIGHT,          SwFPos::BOTTOM},
            {SwFPos::CENTER_HORI,    SwFPos::CENTER_VERT},
            {SwFPos::FROMTOP,        SwFPos::FROMRIGHT},
            {SwFPos::REL_PG_LEFT,    SwFPos::REL_PG_TOP},
            {SwFPos::REL_PG_RIGHT,   SwFPos::REL_PG_BOTTOM} ,
            {SwFPos::REL_FRM_LEFT,   SwFPos::REL_FRM_TOP},
            {SwFPos::REL_FRM_RIGHT,  SwFPos::REL_FRM_BOTTOM}
        };
        static const StringIdPair_Impl aVertIds[] =
        {
            {SwFPos::TOP,            SwFPos::RIGHT},
            {SwFPos::BOTTOM,         SwFPos::LEFT },
            {SwFPos::CENTER_VERT,    SwFPos::CENTER_HORI},
            {SwFPos::FROMTOP,        SwFPos::FROMRIGHT },
            {SwFPos::REL_PG_TOP,     SwFPos::REL_PG_LEFT },
            {SwFPos::REL_PG_BOTTOM,  SwFPos::REL_PG_RIGHT } ,
            {SwFPos::REL_FRM_TOP,    SwFPos::REL_FRM_LEFT },
            {SwFPos::REL_FRM_BOTTOM, SwFPos::REL_FRM_RIGHT }
        };
        // --> OD 2009-08-31 #monglianlayout#
        static const StringIdPair_Impl aVertL2RIds[] =
        {
            {SwFPos::TOP,            SwFPos::LEFT },
            {SwFPos::BOTTOM,         SwFPos::RIGHT },
            {SwFPos::CENTER_VERT,    SwFPos::CENTER_HORI },
            {SwFPos::FROMTOP,        SwFPos::FROMLEFT },
            {SwFPos::REL_PG_TOP,     SwFPos::REL_PG_LEFT },
            {SwFPos::REL_PG_BOTTOM,  SwFPos::REL_PG_RIGHT } ,
            {SwFPos::REL_FRM_TOP,    SwFPos::REL_FRM_LEFT },
            {SwFPos::REL_FRM_BOTTOM, SwFPos::REL_FRM_RIGHT }
        };
        sal_uInt16 nIndex;
        for(nIndex = 0; nIndex < sizeof(aHoriIds) / sizeof(StringIdPair_Impl); ++nIndex)
        {
            if(aHoriIds[nIndex].eHori == eStringId)
            {
                eStringId = aHoriIds[nIndex].eVert;
                return eStringId;
            }
        }
        nIndex = 0;
        for(nIndex = 0; nIndex < sizeof(aVertIds) / sizeof(StringIdPair_Impl); ++nIndex)
        {
            // --> OD 2009-08-31 #mongolianlayout#
            if ( !bVerticalL2R )
            {
                if(aVertIds[nIndex].eHori == eStringId)
                {
                    eStringId = aVertIds[nIndex].eVert;
                    break;
                }
            }
            else
            {
                if(aVertL2RIds[nIndex].eHori == eStringId)
                {
                    eStringId = aVertL2RIds[nIndex].eVert;
                    break;
                }
            }
        }
    }
    return eStringId;
}

// helper method in order to determine all possible
// listbox relations in a relation map for a given relation
static sal_uLong lcl_GetLBRelationsForRelations( const sal_uInt16 _nRel )
{
    sal_uLong nLBRelations = 0L;

    sal_uInt16 nRelMapSize = sizeof(aRelationMap) / sizeof(RelationMap);
    for ( sal_uInt16 nRelMapPos = 0; nRelMapPos < nRelMapSize; ++nRelMapPos )
    {
        if ( aRelationMap[nRelMapPos].nRelation == _nRel )
        {
            nLBRelations |= aRelationMap[nRelMapPos].nLBRelation;
        }
    }

    return nLBRelations;
}

// helper method on order to determine all possible
// listbox relations in a relation map for a given string ID
static sal_uLong lcl_GetLBRelationsForStrID( const FrmMap* _pMap,
                                             const SvxSwFramePosString::StringId _eStrId,
                                             const bool _bUseMirrorStr )
{
    sal_uLong nLBRelations = 0L;

    size_t nRelMapSize = lcl_GetFrmMapCount( _pMap );
    for ( size_t nRelMapPos = 0; nRelMapPos < nRelMapSize; ++nRelMapPos )
    {
        if ( ( !_bUseMirrorStr && _pMap[nRelMapPos].eStrId == _eStrId ) ||
             ( _bUseMirrorStr && _pMap[nRelMapPos].eMirrorStrId == _eStrId ) )
        {
            nLBRelations |= _pMap[nRelMapPos].nLBRelations;
        }
    }

    return nLBRelations;
}

/*--------------------------------------------------------------------
    Description:    standard frame TabPage
 --------------------------------------------------------------------*/
namespace
{
    void HandleAutoCB( sal_Bool _bChecked, FixedText& _rFT_man, FixedText& _rFT_auto )
    {
        _rFT_man.Show( !_bChecked );
        _rFT_auto.Show( _bChecked );
    }
}

SwFrmPage::SwFrmPage ( Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage      (pParent, SW_RES(TP_FRM_STD), rSet),

    aSizeFL         (this, SW_RES(FL_SIZE)),
    aWidthFT        (this, SW_RES(FT_WIDTH)),
    aWidthAutoFT    (this, SW_RES(FT_WIDTH_AUTO)),
    aWidthED        (this, SW_RES(ED_WIDTH)),
    aRelWidthCB     (this, SW_RES(CB_REL_WIDTH)),
    aAutoWidthCB    (this, SW_RES(CB_AUTOWIDTH)),
    aHeightFT       (this, SW_RES(FT_HEIGHT)),
    aHeightAutoFT   (this, SW_RES(FT_HEIGHT_AUTO)),
    aHeightED       (this, SW_RES(ED_HEIGHT)),
    aRelHeightCB    (this, SW_RES(CB_REL_HEIGHT)),
    aAutoHeightCB   (this, SW_RES(CB_AUTOHEIGHT)),
    aFixedRatioCB   (this, SW_RES(CB_FIXEDRATIO)),
    aRealSizeBT     (this, SW_RES(BT_REALSIZE)),

    aTypeSepFL     (this, SW_RES(FL_TYPE_SEP)),
    aTypeFL        (this, SW_RES(FL_TYPE)),
    aAnchorAtPageRB (this, SW_RES(RB_ANCHOR_PAGE)),
    aAnchorAtParaRB (this, SW_RES(RB_ANCHOR_PARA)),
    aAnchorAtCharRB (this, SW_RES(RB_ANCHOR_AT_CHAR)),
    aAnchorAsCharRB (this, SW_RES(RB_ANCHOR_AS_CHAR)),
    aAnchorAtFrameRB(this, SW_RES(RB_ANCHOR_FRAME)),

    aPositionFL     (this, SW_RES(FL_POSITION)),
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
    aFollowTextFlowCB(this, SW_RES(CB_FOLLOWTEXTFLOW)),

    aExampleWN      (this, SW_RES(WN_BSP)),

    bAtHorzPosModified( sal_False ),
    bAtVertPosModified( sal_False ),
    bFormat(sal_False),
    bNew(sal_True),
    bNoModifyHdl(sal_True),
    // --> OD 2009-08-31 #mongolianlayout# - no used
//    bVerticalChanged(sal_False),
    bIsVerticalFrame(sal_False),
    bIsVerticalL2R(sal_False),
    bIsInRightToLeft(sal_False),
    bHtmlMode(sal_False),
    nHtmlMode(0),
    nDlgType(0),
    nUpperBorder(0),
    nLowerBorder(0),
    fWidthHeightRatio(1.0),
    mpToCharCntntPos( NULL ),

    nOldH(text::HoriOrientation::CENTER),
    nOldHRel(text::RelOrientation::FRAME),
    nOldV(text::VertOrientation::TOP),
    nOldVRel(text::RelOrientation::PRINT_AREA),
    pVMap( 0 ),
    pHMap( 0 ),
    m_bAllowVertPositioning( true ),
    m_bIsMathOLE( false ),
    m_bIsMathBaselineAlignment( true )
{
    FreeResource();
    SetExchangeSupport();

    aRealSizeBT.SetAccessibleRelationMemberOf(&aSizeFL);

    Link aLk = LINK(this, SwFrmPage, RangeModifyHdl);
    aWidthED.    SetLoseFocusHdl( aLk );
    aHeightED.   SetLoseFocusHdl( aLk );
    aAtHorzPosED.SetLoseFocusHdl( aLk );
    aAtVertPosED.SetLoseFocusHdl( aLk );
    aFollowTextFlowCB.SetClickHdl( aLk );

    aLk = LINK(this, SwFrmPage, ModifyHdl);
    aWidthED.    SetModifyHdl( aLk );
    aHeightED.   SetModifyHdl( aLk );
    aAtHorzPosED.SetModifyHdl( aLk );
    aAtVertPosED.SetModifyHdl( aLk );

    aLk = LINK(this, SwFrmPage, AnchorTypeHdl);
    aAnchorAtPageRB.SetClickHdl( aLk );
    aAnchorAtParaRB.SetClickHdl( aLk );
    aAnchorAtCharRB.SetClickHdl( aLk );
    aAnchorAsCharRB.SetClickHdl( aLk );
    aAnchorAtFrameRB.SetClickHdl( aLk );

    aHorizontalDLB.SetSelectHdl(LINK(this, SwFrmPage, PosHdl));
    aVerticalDLB.  SetSelectHdl(LINK(this, SwFrmPage, PosHdl));

    aHoriRelationLB.SetSelectHdl(LINK(this, SwFrmPage, RelHdl));
    aVertRelationLB.SetSelectHdl(LINK(this, SwFrmPage, RelHdl));

    aMirrorPagesCB.SetClickHdl(LINK(this, SwFrmPage, MirrorHdl));

    aLk = LINK(this, SwFrmPage, RelSizeClickHdl);
    aRelWidthCB.SetClickHdl( aLk );
    aRelHeightCB.SetClickHdl( aLk );

    aAutoWidthCB.SetClickHdl( LINK( this, SwFrmPage, AutoWidthClickHdl ) );
    aAutoHeightCB.SetClickHdl( LINK( this, SwFrmPage, AutoHeightClickHdl ) );
}

/*--------------------------------------------------------------------
    Description:    Dtor
 --------------------------------------------------------------------*/
SwFrmPage::~SwFrmPage()
{
}

SfxTabPage* SwFrmPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmPage( pParent, rSet );
}

namespace
{
    void MoveControl( Control& _rCtrl, long _nOffsetY )
    {
        Point aPt( _rCtrl.GetPosPixel() );
        aPt.Move( 0, _nOffsetY );
        _rCtrl.SetPosPixel( aPt );
    }
}

void SwFrmPage::EnableGraficMode( void )
{
    // i#39692 - mustn't be called more than once
    if(!aRealSizeBT.IsVisible())
    {
        long nOffset1 = aRelWidthCB.GetPosPixel().Y() - aAutoWidthCB.GetPosPixel().Y();
        long nOffset2 = nOffset1 + aRelHeightCB.GetPosPixel().Y() - aAutoHeightCB.GetPosPixel().Y();

        MoveControl( aHeightFT, nOffset1 );
        MoveControl( aHeightED, nOffset1 );
        MoveControl( aRelHeightCB, nOffset1 );
        MoveControl( aFixedRatioCB, nOffset2 );

        aWidthFT.Show();
        aWidthAutoFT.Hide();
        aAutoHeightCB.Hide();

        aHeightFT.Show();
        aHeightAutoFT.Hide();
        aAutoWidthCB.Hide();

        aRealSizeBT.Show();
    }
}

SwWrtShell *SwFrmPage::getFrmDlgParentShell()
{
    return ((SwFrmDlg*)GetParentDialog())->GetWrtShell();
}

void SwFrmPage::Reset( const SfxItemSet &rSet )
{
    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell() :
            getFrmDlgParentShell();

    nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric( aWidthED, aMetric );
    SetMetric( aHeightED, aMetric );
    SetMetric( aAtHorzPosED, aMetric );
    SetMetric( aAtVertPosED, aMetric );

    const SfxPoolItem* pItem = NULL;
    const SwFmtAnchor& rAnchor = (const SwFmtAnchor&)rSet.Get(RES_ANCHOR);

    if (SFX_ITEM_SET == rSet.GetItemState(FN_OLE_IS_MATH, sal_False, &pItem))
        m_bIsMathOLE = ((const SfxBoolItem*)pItem)->GetValue();
    if (SFX_ITEM_SET == rSet.GetItemState(FN_MATH_BASELINE_ALIGNMENT, sal_False, &pItem))
        m_bIsMathBaselineAlignment = ((const SfxBoolItem*)pItem)->GetValue();
    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && FLY_AS_CHAR == rAnchor.GetAnchorId()) );

    if (bFormat)
    {
        // at formats no anchor editing
        aAnchorAtPageRB.Enable( sal_False );
        aAnchorAtParaRB.Enable( sal_False );
        aAnchorAtCharRB.Enable( sal_False );
        aAnchorAsCharRB.Enable( sal_False );
        aAnchorAtFrameRB.Enable( sal_False );
        aTypeFL.Enable( sal_False );
        aFixedRatioCB.Enable(sal_False);
    }
    else
    {
        if (rAnchor.GetAnchorId() != FLY_AT_FLY && !pSh->IsFlyInFly())
            aAnchorAtFrameRB.Hide();
        // --> OD 2009-08-31 #mongolianlayout#
//        if ( !bVerticalChanged && pSh->IsFrmVertical(sal_True, bIsInRightToLeft) )
        if ( pSh->IsFrmVertical( sal_True, bIsInRightToLeft, bIsVerticalL2R ) )
        {
            String sHLabel = aHorizontalFT.GetText();
            aHorizontalFT.SetText(aVerticalFT.GetText());
            aVerticalFT.SetText(sHLabel);
            bIsVerticalFrame = sal_True;
        }
    }

    if ( nDlgType == DLG_FRM_GRF || nDlgType == DLG_FRM_OLE )
    {
        OSL_ENSURE(pSh , "shell not found");
        //OS: only for the variant Insert/Graphic/Properties
        if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_GRF_REALSIZE, sal_False, &pItem))
            aGrfSize = ((const SvxSizeItem*)pItem)->GetSize();
        else
            pSh->GetGrfSize( aGrfSize );

        if ( !bNew )
        {
            aRealSizeBT.SetClickHdl(LINK(this, SwFrmPage, RealSizeHdl));
            EnableGraficMode();
        }

        if ( nDlgType == DLG_FRM_GRF )
            aFixedRatioCB.Check( sal_False );
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
        aGrfSize = ((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetSize();
    }

    // entering procent value made possible
    aWidthED. SetBaseValue( aWidthED.Normalize(aGrfSize.Width()), FUNIT_TWIP );
    aHeightED.SetBaseValue( aHeightED.Normalize(aGrfSize.Height()), FUNIT_TWIP );
    //the available space is not yet known so the RefValue has to be calculated from size and relative size values
    //this is needed only if relative values are already set

    const SwFmtFrmSize& rFrmSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);

    if (rFrmSize.GetWidthPercent() != 0xff && rFrmSize.GetWidthPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrmSize.GetWidth() * 100 / rFrmSize.GetWidthPercent();
        aWidthED. SetRefValue( nSpace );
    }

    if (rFrmSize.GetHeightPercent() != 0xff && rFrmSize.GetHeightPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrmSize.GetHeight() * 100 / rFrmSize.GetHeightPercent();
        aHeightED.SetRefValue( nSpace );
    }

    // general initialisation part
    switch(rAnchor.GetAnchorId())
    {
        case FLY_AT_PAGE: aAnchorAtPageRB.Check(); break;
        case FLY_AT_PARA: aAnchorAtParaRB.Check(); break;
        case FLY_AT_CHAR: aAnchorAtCharRB.Check(); break;
        case FLY_AS_CHAR: aAnchorAsCharRB.Check(); break;
        case FLY_AT_FLY: aAnchorAtFrameRB.Check();break;
        default:; //prevent warning
    }

    // i#22341 - determine content position of character
    // Note: content position can be NULL
    mpToCharCntntPos = rAnchor.GetCntntAnchor();

    // i#18732 - init checkbox value
    {
        const bool bFollowTextFlow =
            static_cast<const SwFmtFollowTextFlow&>(rSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
        aFollowTextFlowCB.Check( bFollowTextFlow );
    }

    if(bHtmlMode)
    {
        aAutoHeightCB.Enable(sal_False);
        aAutoWidthCB.Enable(sal_False);
        aMirrorPagesCB.Show(sal_False);
        if(nDlgType == DLG_FRM_STD)
            aFixedRatioCB.Enable(sal_False);
        // i#18732 hide checkbox in HTML mode
        aFollowTextFlowCB.Show(sal_False);
    }
    else
    {
        // enable/disable of check box 'Mirror on..'
        aMirrorPagesCB.Enable(!aAnchorAsCharRB.IsChecked());

        // enable/disable check box 'Follow text flow'.
        // enable check box 'Follow text
        // flow' also for anchor type to-frame.
        aFollowTextFlowCB.Enable( aAnchorAtParaRB.IsChecked() ||
                                  aAnchorAtCharRB.IsChecked() ||
                                  aAnchorAtFrameRB.IsChecked() );
    }

    Init( rSet, sal_True );
    aAtVertPosED.SaveValue();
    aAtHorzPosED.SaveValue();
    aFollowTextFlowCB.SaveValue();

    bNoModifyHdl = sal_False;
    //lock PercentFields
    aWidthED.LockAutoCalculation(sal_True);
    aHeightED.LockAutoCalculation(sal_True);
    RangeModifyHdl(&aWidthED);  // set all maximum values initially
    aHeightED.LockAutoCalculation(sal_False);
    aWidthED.LockAutoCalculation(sal_False);

    aAutoHeightCB.SaveValue();
    aAutoWidthCB.SaveValue();

    SwTwips nWidth  = static_cast< SwTwips >(aWidthED.DenormalizePercent(aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(aHeightED.DenormalizePercent(aHeightED.GetValue(FUNIT_TWIP)));
    fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
}

/*--------------------------------------------------------------------
    Description:    stuff attributes into the set when OK
 --------------------------------------------------------------------*/
sal_Bool SwFrmPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bRet = sal_False;
    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                        : getFrmDlgParentShell();
    OSL_ENSURE( pSh , "shell not found");
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

        sal_uInt16 nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        short nRel = GetRelation(pHMap, aHoriRelationLB);

        const sal_Int16 eHOri = nAlign;
        const sal_Int16 eRel  = nRel;

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(aMirrorPagesCB.IsChecked());

        sal_Bool bMod = aAtHorzPosED.GetText() != aAtHorzPosED.GetSavedValue();
        bMod |= aMirrorPagesCB.GetState() != aMirrorPagesCB.GetSavedValue();

        if ( eHOri == text::HoriOrientation::NONE &&
             (bNew || (bAtHorzPosModified || bMod) || nOldH != eHOri ) )
        {
            SwTwips nX = static_cast< SwTwips >(aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP)));
            aHoriOrient.SetPos( nX );
        }

        pOldItem = GetOldItem(rSet, FN_HORI_ORIENT);
        sal_Bool bSame = sal_False;
        if ((bNew == bFormat) && pOldItem)
        {
             bSame = static_cast< sal_Bool >(aHoriOrient == (SwFmtHoriOrient&)*pOldItem);
        }
        if ((bNew && !bFormat) || ((bAtHorzPosModified || bMod) && !bSame))
        {
            bRet |= 0 != rSet.Put( aHoriOrient );
        }
    }

    if ( pVMap )
    {
        // alignment vertical
        SwFmtVertOrient aVertOrient( (const SwFmtVertOrient&)
                                                rOldSet.Get(RES_VERT_ORIENT) );

        sal_uInt16 nMapPos = GetMapPos(pVMap, aVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        short nRel = GetRelation(pVMap, aVertRelationLB);

        const sal_Int16     eVOri = nAlign;
        const sal_Int16     eRel  = nRel;

        aVertOrient.SetVertOrient    ( eVOri);
        aVertOrient.SetRelationOrient( eRel );

        sal_Bool bMod = aAtVertPosED.GetText() != aAtVertPosED.GetSavedValue();

        if ( eVOri == text::VertOrientation::NONE &&
             ( bNew || (bAtVertPosModified || bMod) || nOldV != eVOri) )
        {
            // vertival position
            // recalculate offset for character bound frames
            SwTwips nY = static_cast< SwTwips >(aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP)));
            if (eAnchorId == FLY_AS_CHAR)
            {
                nY *= -1;
            }
            aVertOrient.SetPos( nY );
        }
        pOldItem = GetOldItem(rSet, FN_VERT_ORIENT);
        sal_Bool bSame = sal_False;
        if((bNew == bFormat) && pOldItem)
        {
             bSame = static_cast< sal_Bool >( bFormat ?
                aVertOrient.GetVertOrient() == ((SwFmtVertOrient*)pOldItem)->GetVertOrient() &&
                aVertOrient.GetRelationOrient() == ((SwFmtVertOrient*)pOldItem)->GetRelationOrient() &&
                aVertOrient.GetPos() == ((SwFmtVertOrient*)pOldItem)->GetPos()

                    : aVertOrient == (SwFmtVertOrient&)*pOldItem );
        }
        if( ( bNew && !bFormat ) || ((bAtVertPosModified || bMod) && !bSame ))
        {
            bRet |= 0 != rSet.Put( aVertOrient );
        }
    }

    // set size
    // new exception: when the size of pMgr(, 0), then the properties
    // for a graphic that isn't even loaded, are set. Then no SetSize
    // is done here when the size settings were not changed by the
    // user.
    const SwFmtFrmSize& rOldSize = (const SwFmtFrmSize& )rOldSet.Get(RES_FRM_SIZE);
    SwFmtFrmSize aSz( rOldSize );

    sal_Bool bValueModified = (aWidthED.IsValueModified() || aHeightED.IsValueModified());
    sal_Bool bCheckChanged = (aRelWidthCB.GetSavedValue() != aRelWidthCB.IsChecked()
                        || aRelHeightCB.GetSavedValue() != aRelHeightCB.IsChecked());

    sal_Bool bLegalValue = !(!rOldSize.GetWidth () && !rOldSize.GetHeight() &&
                            aWidthED .GetValue() == aWidthED .GetMin() &&
                            aHeightED.GetValue() == aHeightED.GetMin());

    if ((bNew && !bFormat) || ((bValueModified || bCheckChanged) && bLegalValue))
    {
        sal_Int64 nNewWidth  = aWidthED.DenormalizePercent(aWidthED.GetRealValue(FUNIT_TWIP));
        sal_Int64 nNewHeight = aHeightED.DenormalizePercent(aHeightED.GetRealValue(FUNIT_TWIP));
        aSz.SetWidth (static_cast< SwTwips >(nNewWidth));
        aSz.SetHeight(static_cast< SwTwips >(nNewHeight));

        if (aRelWidthCB.IsChecked())
        {
            aSz.SetWidthPercent((sal_uInt8)Min( static_cast< sal_Int64 >(MAX_PERCENT_WIDTH), aWidthED.Convert(aWidthED.NormalizePercent(nNewWidth), FUNIT_TWIP, FUNIT_CUSTOM)));
        }
        else
            aSz.SetWidthPercent(0);
        if (aRelHeightCB.IsChecked())
            aSz.SetHeightPercent((sal_uInt8)Min(static_cast< sal_Int64 >(MAX_PERCENT_HEIGHT), aHeightED.Convert(aHeightED.NormalizePercent(nNewHeight), FUNIT_TWIP, FUNIT_CUSTOM)));
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
    if( !IsInGraficMode() )
    {
        if( aAutoHeightCB.GetState() != aAutoHeightCB.GetSavedValue() )
        {
            SwFrmSize eFrmSize = (SwFrmSize) aAutoHeightCB.IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrmSize != aSz.GetHeightSizeType() )
                aSz.SetHeightSizeType(eFrmSize);
        }
        if( aAutoWidthCB.GetState() != aAutoWidthCB.GetSavedValue() )
        {
            SwFrmSize eFrmSize = (SwFrmSize) aAutoWidthCB.IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrmSize != aSz.GetWidthSizeType() )
                aSz.SetWidthSizeType( eFrmSize );
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
        if (aSz.GetHeightSizeType() == ATT_VAR_SIZE)    // there is no VAR_SIZE in frames
            aSz.SetHeightSizeType(ATT_MIN_SIZE);

        bRet |= 0 != rSet.Put( aSz );
    }
    if(aFollowTextFlowCB.IsChecked() != aFollowTextFlowCB.GetSavedValue())
    {
        bRet |= 0 != rSet.Put(SwFmtFollowTextFlow(aFollowTextFlowCB.IsChecked()));
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Description:    initialise horizonal and vertical Pos
 --------------------------------------------------------------------*/
void SwFrmPage::InitPos(RndStdIds eId,
                                sal_uInt16 nH,
                                sal_uInt16 nHRel,
                                sal_uInt16 nV,
                                sal_uInt16 nVRel,
                                long   nX,
                                long   nY)
{
    sal_uInt16 nPos = aVerticalDLB.GetSelectEntryPos();
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

    sal_Bool bEnable = sal_True;
    if ( eId == FLY_AT_PAGE )
    {
        pVMap = bHtmlMode ? aVPageHtmlMap : aVPageMap;
        pHMap = bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if ( eId == FLY_AT_FLY )
    {
        // own vertical alignment map for to frame
        // anchored objects.
        pVMap = bHtmlMode ? aVFlyHtmlMap : aVFrameMap;
        pHMap = bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if ( eId == FLY_AT_PARA )
    {
        if(bHtmlMode)
        {
            pVMap = aVParaHtmlMap;
            pHMap = aHParaHtmlAbsMap;
        }
        else
        {
            pVMap = aVParaMap;
            pHMap = aHParaMap;
        }
    }
    else if ( eId == FLY_AT_CHAR )
    {
        if(bHtmlMode)
        {
            pVMap = aVCharHtmlAbsMap;
            pHMap = aHCharHtmlAbsMap;
        }
        else
        {
            pVMap = aVCharMap;
            pHMap = aHCharMap;
        }
    }
    else if ( eId == FLY_AS_CHAR )
    {
        pVMap = bHtmlMode ? aVAsCharHtmlMap     : aVAsCharMap;
        pHMap = 0;
        bEnable = sal_False;
    }
    aHorizontalDLB.Enable( bEnable );
    aHorizontalFT.Enable( bEnable );

    // select current Pos
    // horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = nOldH;
        nHRel = nOldHRel;
    }
    sal_uInt16 nMapPos = FillPosLB(pHMap, nH, nHRel, aHorizontalDLB);
    FillRelLB(pHMap, nMapPos, nH, nHRel, aHoriRelationLB, aHoriRelationFT);

    // vertical
    if ( nV == USHRT_MAX )
    {
        nV    = nOldV;
        nVRel = nOldVRel;
    }
    nMapPos = FillPosLB(pVMap, nV, nVRel, aVerticalDLB);
    FillRelLB(pVMap, nMapPos, nV, nVRel, aVertRelationLB, aVertRelationFT);

    bEnable = nH == text::HoriOrientation::NONE && eId != FLY_AS_CHAR;
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

    bEnable = nV == text::VertOrientation::NONE;
    if ( !bEnable )
    {
        aAtVertPosED.SetValue( 0, FUNIT_TWIP );
        if(nY != LONG_MAX && bHtmlMode)
            aAtVertPosED.SetModifyFlag();
    }
    else
    {
        if ( eId == FLY_AS_CHAR )
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            aAtVertPosED.SetValue( aAtVertPosED.Normalize(nY), FUNIT_TWIP );
    }
    aAtVertPosFT.Enable( bEnable && m_bAllowVertPositioning );
    aAtVertPosED.Enable( bEnable && m_bAllowVertPositioning );
    UpdateExample();
}

sal_uInt16 SwFrmPage::FillPosLB(const FrmMap* _pMap,
                            const sal_uInt16 _nAlign,
                            const sal_uInt16 _nRel,
                            ListBox& _rLB )
{
    String sSelEntry, sOldEntry;
    sOldEntry = _rLB.GetSelectEntry();

    _rLB.Clear();

    // i#22341 determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const sal_uLong nLBRelations = (_pMap != aVCharMap)
                               ? 0L
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill Listbox
    size_t nCount = ::lcl_GetFrmMapCount(_pMap);
    for (size_t i = 0; _pMap && i < nCount; ++i)
    {
//      Why not from the left/from inside or from above?
        {
            SvxSwFramePosString::StringId eStrId = aMirrorPagesCB.IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
            // --> OD 2009-08-31 #mongolianlayout#
            eStrId = lcl_ChangeResIdToVerticalOrRTL( eStrId,
                                                     bIsVerticalFrame,
                                                     bIsVerticalL2R,
                                                     bIsInRightToLeft);
            String sEntry(aFramePosString.GetString(eStrId));
            if (_rLB.GetEntryPos(sEntry) == LISTBOX_ENTRY_NOTFOUND)
            {
                // don't insert entries when frames are character bound
                _rLB.InsertEntry(sEntry);
            }
            // i#22341 - add condition to handle map <aVCharMap>
            // that is ambigous in the alignment.
            if ( _pMap[i].nAlign == _nAlign &&
                 ( !(_pMap == aVCharMap) || _pMap[i].nLBRelations & nLBRelations ) )
            {
                sSelEntry = sEntry;
            }
        }
    }

    _rLB.SelectEntry(sSelEntry);
    if (!_rLB.GetSelectEntryCount())
        _rLB.SelectEntry(sOldEntry);

    if (!_rLB.GetSelectEntryCount())
        _rLB.SelectEntryPos(0);

    PosHdl(&_rLB);

    return GetMapPos(_pMap, _rLB);
}

sal_uLong SwFrmPage::FillRelLB( const FrmMap* _pMap,
                            const sal_uInt16 _nLBSelPos,
                            const sal_uInt16 _nAlign,
                            sal_uInt16 _nRel,
                            ListBox& _rLB,
                            FixedText& _rFT )
{
    String sSelEntry;
    sal_uLong  nLBRelations = 0;
    size_t nMapCount = ::lcl_GetFrmMapCount(_pMap);

    _rLB.Clear();

    if (_nLBSelPos < nMapCount)
    {
        if (_pMap == aVAsCharHtmlMap || _pMap == aVAsCharMap)
        {
            String sOldEntry(_rLB.GetSelectEntry());
            sal_uInt16 nRelCount = sizeof(aAsCharRelationMap) / sizeof(RelationMap);
            SvxSwFramePosString::StringId eStrId = _pMap[_nLBSelPos].eStrId;

            for (size_t nMapPos = 0; nMapPos < nMapCount; nMapPos++)
            {
                if (_pMap[nMapPos].eStrId == eStrId)
                {
                    nLBRelations = _pMap[nMapPos].nLBRelations;
                    for (sal_uInt16 nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = aAsCharRelationMap[nRelPos].eStrId;

                            // --> OD 2009-08-31 #mongolianlayout#
                            sStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( sStrId1,
                                                                bIsVerticalFrame,
                                                                bIsVerticalL2R,
                                                                bIsInRightToLeft);
                            String sEntry = aFramePosString.GetString(sStrId1);
                            sal_uInt16 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, &aAsCharRelationMap[nRelPos]);
                            if (_pMap[nMapPos].nAlign == _nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (sSelEntry.Len())
                _rLB.SelectEntry(sSelEntry);
            else
            {
                _rLB.SelectEntry(sOldEntry);

                if (!_rLB.GetSelectEntryCount())
                {
                    for (sal_uInt16 i = 0; i < _rLB.GetEntryCount(); i++)
                    {
                        RelationMap *pEntry = (RelationMap *)_rLB.GetEntryData(i);
                        if (pEntry->nLBRelation == LB_REL_CHAR) // default
                        {
                            _rLB.SelectEntryPos(i);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            sal_uInt16 nRelCount = sizeof(aRelationMap) / sizeof(RelationMap);

            // special handling for map <aVCharMap>,
            // because its ambigous in its <eStrId>/<eMirrorStrId>.
            if ( _pMap == aVCharMap )
            {
                nLBRelations = ::lcl_GetLBRelationsForStrID( _pMap,
                                             ( aMirrorPagesCB.IsChecked()
                                               ? _pMap[_nLBSelPos].eMirrorStrId
                                               : _pMap[_nLBSelPos].eStrId),
                                             aMirrorPagesCB.IsChecked() );
            }
            else
            {
                nLBRelations = _pMap[_nLBSelPos].nLBRelations;
            }

            for (sal_uLong nBit = 1; nBit < 0x80000000; nBit <<= 1)
            {
                if (nLBRelations & nBit)
                {
                    for (sal_uInt16 nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == nBit)
                        {
                            SvxSwFramePosString::StringId eStrId1 = aMirrorPagesCB.IsChecked() ?
                                            aRelationMap[nRelPos].eMirrorStrId : aRelationMap[nRelPos].eStrId;
                            // --> OD 2009-08-31 #mongolianlayout#
                            eStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( eStrId1,
                                                                bIsVerticalFrame,
                                                                bIsVerticalL2R,
                                                                bIsInRightToLeft);
                            String sEntry = aFramePosString.GetString(eStrId1);
                            sal_uInt16 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, &aRelationMap[nRelPos]);
                            if (!sSelEntry.Len() && aRelationMap[nRelPos].nRelation == _nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (sSelEntry.Len())
                _rLB.SelectEntry(sSelEntry);
            else
            {
                // Probably anchor switch. So look for similar relation
                switch (_nRel)
                {
                    case text::RelOrientation::FRAME:           _nRel = text::RelOrientation::PAGE_FRAME;   break;
                    case text::RelOrientation::PRINT_AREA:      _nRel = text::RelOrientation::PAGE_PRINT_AREA; break;
                    case text::RelOrientation::PAGE_LEFT:       _nRel = text::RelOrientation::FRAME_LEFT;   break;
                    case text::RelOrientation::PAGE_RIGHT:      _nRel = text::RelOrientation::FRAME_RIGHT;  break;
                    case text::RelOrientation::FRAME_LEFT:      _nRel = text::RelOrientation::PAGE_LEFT;    break;
                    case text::RelOrientation::FRAME_RIGHT:     _nRel = text::RelOrientation::PAGE_RIGHT;   break;
                    case text::RelOrientation::PAGE_FRAME:      _nRel = text::RelOrientation::FRAME;          break;
                    case text::RelOrientation::PAGE_PRINT_AREA: _nRel = text::RelOrientation::PRINT_AREA;        break;

                    default:
                        if (_rLB.GetEntryCount())
                        {
                            RelationMap *pEntry = (RelationMap *)_rLB.GetEntryData(_rLB.GetEntryCount() - 1);
                            _nRel = pEntry->nRelation;
                        }
                        break;
                }

                for (sal_uInt16 i = 0; i < _rLB.GetEntryCount(); i++)
                {
                    RelationMap *pEntry = (RelationMap *)_rLB.GetEntryData(i);
                    if (pEntry->nRelation == _nRel)
                    {
                        _rLB.SelectEntryPos(i);
                        break;
                    }
                }

                if (!_rLB.GetSelectEntryCount())
                    _rLB.SelectEntryPos(0);
            }
        }
    }

    const bool bEnable = _rLB.GetEntryCount() != 0
            && (&_rLB != &aVertRelationLB || m_bAllowVertPositioning);
    _rLB.Enable( bEnable );
    _rFT.Enable( bEnable );

    RelHdl(&_rLB);

    return nLBRelations;
}

short SwFrmPage::GetRelation(FrmMap * /*pMap*/, ListBox &rRelationLB)
{
    short nRel = 0;
    sal_uInt16 nPos = rRelationLB.GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        RelationMap *pEntry = (RelationMap *)rRelationLB.GetEntryData(nPos);
        nRel = pEntry->nRelation;
    }

    return nRel;
}

short SwFrmPage::GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos,
        ListBox &/*rAlignLB*/, ListBox &rRelationLB)
{
    short nAlign = 0;

    // i#22341 special handling also for map <aVCharMap>,
    // because it contains ambigous items for alignment
    if ( pMap == aVAsCharHtmlMap || pMap == aVAsCharMap ||
         pMap == aVCharMap )
    {
        if (rRelationLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            sal_uLong  nRel = ((RelationMap *)rRelationLB.GetEntryData(rRelationLB.GetSelectEntryPos()))->nLBRelation;
            size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (size_t i = 0; i < nMapCount; i++)
            {
                if (pMap[i].eStrId == eStrId)
                {
                    sal_uLong nLBRelations = pMap[i].nLBRelations;
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

sal_uInt16 SwFrmPage::GetMapPos( const FrmMap *pMap, ListBox &rAlignLB )
{
    sal_uInt16 nMapPos = 0;
    sal_uInt16 nLBSelPos = rAlignLB.GetSelectEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            String sSelEntry(rAlignLB.GetSelectEntry());

            for (size_t i = 0; i < nMapCount; i++)
            {
                SvxSwFramePosString::StringId eResId = pMap[i].eStrId;

                String sEntry = aFramePosString.GetString(eResId);
                sEntry = MnemonicGenerator::EraseAllMnemonicChars( sEntry );

                if (sEntry == sSelEntry)
                {
                    nMapPos = static_cast< sal_uInt16 >(i);
                    break;
                }
            }
        }
        else
            nMapPos = nLBSelPos;
    }

    return nMapPos;
}

RndStdIds SwFrmPage::GetAnchor()
{
    RndStdIds nRet = FLY_AT_PAGE;
    if(aAnchorAtParaRB.IsChecked())
    {
        nRet = FLY_AT_PARA;
    }
    else if(aAnchorAtCharRB.IsChecked())
    {
        nRet = FLY_AT_CHAR;
    }
    else if(aAnchorAsCharRB.IsChecked())
    {
        nRet = FLY_AS_CHAR;
    }
    else if(aAnchorAtFrameRB.IsChecked())
    {
        nRet = FLY_AT_FLY;
    }
    return nRet;
}

/*--------------------------------------------------------------------
    Description:    Bsp - Update
 --------------------------------------------------------------------*/
void SwFrmPage::ActivatePage(const SfxItemSet& rSet)
{
    bNoModifyHdl = sal_True;
    Init(rSet);
    bNoModifyHdl = sal_False;
    //lock PercentFields
    aWidthED.LockAutoCalculation(sal_True);
    aHeightED.LockAutoCalculation(sal_True);
    RangeModifyHdl(&aWidthED);  // set all maximum values initially
    aHeightED.LockAutoCalculation(sal_False);
    aWidthED.LockAutoCalculation(sal_False);
    aFollowTextFlowCB.SaveValue();
}

int SwFrmPage::DeactivatePage(SfxItemSet * _pSet)
{
    if ( _pSet )
    {
        FillItemSet( *_pSet );

        //FillItemSet doesn't set the anchor into the set when it matches
        //the original. But for the other pages we need the current anchor.
        SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                            : getFrmDlgParentShell();
        RndStdIds eAnchorId = (RndStdIds)GetAnchor();
        SwFmtAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
        _pSet->Put( aAnc );
    }

    return sal_True;
}

/*--------------------------------------------------------------------
    Description: swap left/right with inside/outside
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFrmPage, MirrorHdl)
{
    RndStdIds eId = GetAnchor();
    InitPos( eId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);

    return 0;
}

IMPL_LINK( SwFrmPage, RelSizeClickHdl, CheckBox *, pBtn )
{
    if (pBtn == &aRelWidthCB)
    {
        aWidthED.ShowPercent(pBtn->IsChecked());
        if(pBtn->IsChecked())
            aWidthED.MetricField::SetMax(MAX_PERCENT_WIDTH);
    }
    else // pBtn == &aRelHeightCB
    {
        aHeightED.ShowPercent(pBtn->IsChecked());
        if(pBtn->IsChecked())
            aHeightED.MetricField::SetMax(MAX_PERCENT_HEIGHT);
    }

    if (pBtn)   // only when Handler was called by change of the controller
        RangeModifyHdl(&aWidthED);  // correct the values again

    if (pBtn == &aRelWidthCB)
        ModifyHdl(&aWidthED);
    else // pBtn == &aRelHeightCB
        ModifyHdl(&aHeightED);

    return 0;
}

/*--------------------------------------------------------------------
    Description:    range check
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFrmPage, RangeModifyHdl)
{
    if (bNoModifyHdl)
        return 0;

    SwWrtShell* pSh = bFormat ? ::GetActiveWrtShell()
                        : getFrmDlgParentShell();
    OSL_ENSURE(pSh , "shell not found");
    SwFlyFrmAttrMgr aMgr( bNew, pSh, (const SwAttrSet&)GetItemSet() );
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = static_cast< sal_uInt16 >(GetAnchor());
    aVal.bAutoHeight = aAutoHeightCB.IsChecked();
    aVal.bAutoWidth = aAutoWidthCB.IsChecked();
    aVal.bMirror = aMirrorPagesCB.IsChecked();
    aVal.bFollowTextFlow = aFollowTextFlowCB.IsChecked();

    if ( pHMap )
    {
        // alignment horizonal
        sal_uInt16 nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        short nRel = GetRelation(pHMap, aHoriRelationLB);

        aVal.nHoriOrient = nAlign;
        aVal.nHRelOrient = nRel;
    }
    else
        aVal.nHoriOrient = text::HoriOrientation::NONE;

    if ( pVMap )
    {
        // alignment vertical
        sal_uInt16 nMapPos = GetMapPos(pVMap, aVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        short nRel = GetRelation(pVMap, aVertRelationLB);

        aVal.nVertOrient = nAlign;
        aVal.nVRelOrient = nRel;
    }
    else
        aVal.nVertOrient = text::VertOrientation::NONE;

    const long nAtHorzPosVal = static_cast< long >(
                    aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP)) );
    const long nAtVertPosVal = static_cast< long >(
                    aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP)) );

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, mpToCharCntntPos, sal_True);   // one time, to get reference values for percental values

    // set reference values for percental values (100%) ...
    aWidthED.SetRefValue(aVal.aPercentSize.Width());
    aHeightED.SetRefValue(aVal.aPercentSize.Height());

    // ... and correctly convert width and height with it
    SwTwips nWidth  = static_cast< SwTwips >(aWidthED. DenormalizePercent(aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(aHeightED.DenormalizePercent(aHeightED.GetValue(FUNIT_TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    aMgr.ValidateMetrics(aVal, mpToCharCntntPos);    // one more time, to determine all remaining values with correct width and height.

    // all columns have to be correct
    if(GetTabDialog()->GetExampleSet() &&
            SFX_ITEM_DEFAULT <= GetTabDialog()->GetExampleSet()->GetItemState(RES_COL))
    {
        const SwFmtCol& rCol = (const SwFmtCol&)GetTabDialog()->GetExampleSet()->Get(RES_COL);
        if ( rCol.GetColumns().size() > 1 )
        {
            for ( sal_uInt16 i = 0; i < rCol.GetColumns().size(); ++i )
            {
                aVal.nMinWidth += rCol.GetColumns()[i].GetLeft() +
                                  rCol.GetColumns()[i].GetRight() +
                                  MINFLY;
            }
            aVal.nMinWidth -= MINFLY;//one was already in there!
        }
    }

    nWidth = aVal.nWidth;
    nHeight = aVal.nHeight;

    // minimum range also for template
    aHeightED.SetMin(aHeightED.NormalizePercent(aVal.nMinHeight), FUNIT_TWIP);
    aWidthED. SetMin(aWidthED.NormalizePercent(aVal.nMinWidth), FUNIT_TWIP);

    SwTwips nMaxWidth(aVal.nMaxWidth);
    SwTwips nMaxHeight(aVal.nMaxHeight);

    if (aVal.bAutoHeight && (nDlgType == DLG_FRM_GRF || nDlgType == DLG_FRM_OLE))
    {
        SwTwips nTmp = Min(nWidth * nMaxHeight / Max(nHeight, 1L), nMaxHeight);
        aWidthED.SetMax(aWidthED.NormalizePercent(nTmp), FUNIT_TWIP);

        nTmp = Min(nHeight * nMaxWidth / Max(nWidth, 1L), nMaxWidth);
        aHeightED.SetMax(aWidthED.NormalizePercent(nTmp), FUNIT_TWIP);
    }
    else
    {
        SwTwips nTmp = static_cast< SwTwips >(aHeightED.NormalizePercent(nMaxHeight));
        aHeightED.SetMax(nTmp, FUNIT_TWIP);

        nTmp = static_cast< SwTwips >(aWidthED.NormalizePercent(nMaxWidth));
        aWidthED.SetMax(nTmp, FUNIT_TWIP);
    }

    aAtHorzPosED.SetMin(aAtHorzPosED.Normalize(aVal.nMinHPos), FUNIT_TWIP);
    aAtHorzPosED.SetMax(aAtHorzPosED.Normalize(aVal.nMaxHPos), FUNIT_TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        aAtHorzPosED.SetValue(aAtHorzPosED.Normalize(aVal.nHPos), FUNIT_TWIP);

    const SwTwips nUpperOffset = (aVal.nAnchorType == FLY_AS_CHAR)
        ? nUpperBorder : 0;
    const SwTwips nLowerOffset = (aVal.nAnchorType == FLY_AS_CHAR)
        ? nLowerBorder : 0;

    aAtVertPosED.SetMin(aAtVertPosED.Normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset), FUNIT_TWIP);
    aAtVertPosED.SetMax(aAtVertPosED.Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        aAtVertPosED.SetValue(aAtVertPosED.Normalize(aVal.nVPos), FUNIT_TWIP);

    return 0;
}

IMPL_LINK_NOARG(SwFrmPage, AnchorTypeHdl)
{
    aMirrorPagesCB.Enable(!aAnchorAsCharRB.IsChecked());

    // i#18732 - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    // i#22305 - enable check box 'Follow text
    // flow' also for anchor type to-frame.
    aFollowTextFlowCB.Enable( aAnchorAtParaRB.IsChecked() ||
                              aAnchorAtCharRB.IsChecked() ||
                              aAnchorAtFrameRB.IsChecked() );

    RndStdIds eId = GetAnchor();

    InitPos( eId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl(0);

    if(bHtmlMode)
    {
        PosHdl(&aHorizontalDLB);
        PosHdl(&aVerticalDLB);
    }

    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && FLY_AS_CHAR == eId) );

    return 0;
}

IMPL_LINK( SwFrmPage, PosHdl, ListBox *, pLB )
{
    sal_Bool bHori = pLB == &aHorizontalDLB;
    ListBox *pRelLB = bHori ? &aHoriRelationLB : &aVertRelationLB;
    FixedText *pRelFT = bHori ? &aHoriRelationFT : &aVertRelationFT;
    FrmMap *pMap = bHori ? pHMap : pVMap;

    sal_uInt16 nMapPos = GetMapPos(pMap, *pLB);
    short nAlign = GetAlignment(pMap, nMapPos, *pLB, *pRelLB);

    if (bHori)
    {
        sal_Bool bEnable = text::HoriOrientation::NONE == nAlign;
        aAtHorzPosED.Enable( bEnable );
        aAtHorzPosFT.Enable( bEnable );
    }
    else
    {
        sal_Bool bEnable = text::VertOrientation::NONE == nAlign && m_bAllowVertPositioning;
        aAtVertPosED.Enable( bEnable );
        aAtVertPosFT.Enable( bEnable );
    }

    if (pLB)    // only when Handler was called by changing of the controller
        RangeModifyHdl( 0 );

    sal_uInt16 nRel = 0;
    if (pLB->GetSelectEntryCount())
    {

        if (pRelLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
            nRel = ((RelationMap *)pRelLB->GetEntryData(pRelLB->GetSelectEntryPos()))->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->Clear();

    UpdateExample();

    if (bHori)
        bAtHorzPosModified = sal_True;
    else
        bAtVertPosModified = sal_True;

    // special treatment for HTML-Mode with horizonal-vertical-dependencies
    if(bHtmlMode && (FLY_AT_CHAR == GetAnchor()))
    {
        sal_Bool bSet = sal_False;
        if(bHori)
        {
            // right is allowed only above - from the left only above
            // from the left at character -> below
            if((text::HoriOrientation::LEFT == nAlign || text::HoriOrientation::RIGHT == nAlign) &&
                    0 == aVerticalDLB.GetSelectEntryPos())
            {
                if(text::RelOrientation::FRAME == nRel)
                    aVerticalDLB.SelectEntryPos(1);
                else
                    aVerticalDLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            else if(text::HoriOrientation::LEFT == nAlign && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            else if(text::HoriOrientation::NONE == nAlign && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            if(bSet)
                PosHdl(&aVerticalDLB);
        }
        else
        {
            if(text::VertOrientation::TOP == nAlign)
            {
                if(1 == aHorizontalDLB.GetSelectEntryPos())
                {
                    aHorizontalDLB.SelectEntryPos(0);
                    bSet = sal_True;
                }
                aHoriRelationLB.SelectEntryPos(1);
            }
            else if(text::VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == aHorizontalDLB.GetSelectEntryPos())
                {
                    aHorizontalDLB.SelectEntryPos(0);
                    bSet = sal_True;
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
    Description:    horizontal Pos
 --------------------------------------------------------------------*/
IMPL_LINK( SwFrmPage, RelHdl, ListBox *, pLB )
{
    sal_Bool bHori = pLB == &aHoriRelationLB;

    UpdateExample();

    if (bHori)
        bAtHorzPosModified = sal_True;
    else
        bAtVertPosModified = sal_True;

    if (bHtmlMode && (FLY_AT_CHAR == GetAnchor()))
    {
        if(bHori)
        {
            sal_uInt16 nRel = GetRelation(pHMap, aHoriRelationLB);
            if(text::RelOrientation::PRINT_AREA == nRel && 0 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(1);
            }
            else if(text::RelOrientation::CHAR == nRel && 1 == aVerticalDLB.GetSelectEntryPos())
            {
                aVerticalDLB.SelectEntryPos(0);
            }
        }
    }
    if (pLB)    // Only when Handler was called by changing of the controller
        RangeModifyHdl(0);

    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwFrmPage, RealSizeHdl)
{
    aWidthED.SetUserValue( aWidthED. NormalizePercent(aGrfSize.Width() ), FUNIT_TWIP);
    aHeightED.SetUserValue(aHeightED.NormalizePercent(aGrfSize.Height()), FUNIT_TWIP);
    fWidthHeightRatio = aGrfSize.Height() ? double(aGrfSize.Width()) / double(aGrfSize.Height()) : 1.0;
    UpdateExample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFrmPage, RealSizeHdl)

IMPL_LINK_NOARG(SwFrmPage, AutoWidthClickHdl)
{
    if( !IsInGraficMode() )
        HandleAutoCB( aAutoWidthCB.IsChecked(), aWidthFT, aWidthAutoFT );
    return 0;
}

IMPL_LINK_NOARG(SwFrmPage, AutoHeightClickHdl)
{
    if( !IsInGraficMode() )
        HandleAutoCB( aAutoHeightCB.IsChecked(), aHeightFT, aHeightAutoFT );
    return 0;
}

IMPL_LINK( SwFrmPage, ModifyHdl, Edit *, pEdit )
{
    SwTwips nWidth  = static_cast< SwTwips >(aWidthED.DenormalizePercent(aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(aHeightED.DenormalizePercent(aHeightED.GetValue(FUNIT_TWIP)));
    if ( aFixedRatioCB.IsChecked() )
    {
        if ( pEdit == &aWidthED )
        {
            nHeight = SwTwips((double)nWidth / fWidthHeightRatio);
            aHeightED.SetPrcntValue(aHeightED.NormalizePercent(nHeight), FUNIT_TWIP);
        }
        else if(pEdit == &aHeightED)
        {
            nWidth = SwTwips((double)nHeight * fWidthHeightRatio);
            aWidthED.SetPrcntValue(aWidthED.NormalizePercent(nWidth), FUNIT_TWIP);
        }
    }
    fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
    return 0;
}

void SwFrmPage::UpdateExample()
{
    sal_uInt16 nPos = aHorizontalDLB.GetSelectEntryPos();
    if ( pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(pHMap, aHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, aHorizontalDLB, aHoriRelationLB);
        short nRel = GetRelation(pHMap, aHoriRelationLB);

        aExampleWN.SetHAlign(nAlign);
        aExampleWN.SetHoriRel(nRel);
    }

    nPos = aVerticalDLB.GetSelectEntryPos();
    if ( pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(pVMap, aVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, aVerticalDLB, aVertRelationLB);
        short nRel = GetRelation(pVMap, aVertRelationLB);

        aExampleWN.SetVAlign(nAlign);
        aExampleWN.SetVertRel(nRel);
    }

    // size
    long nXPos = static_cast< long >(aAtHorzPosED.Denormalize(aAtHorzPosED.GetValue(FUNIT_TWIP)));
    long nYPos = static_cast< long >(aAtVertPosED.Denormalize(aAtVertPosED.GetValue(FUNIT_TWIP)));
    aExampleWN.SetRelPos(Point(nXPos, nYPos));

    aExampleWN.SetAnchor( static_cast< sal_uInt16 >(GetAnchor()) );
    aExampleWN.Invalidate();
}

void SwFrmPage::Init(const SfxItemSet& rSet, sal_Bool bReset)
{
    if(!bFormat)
    {
        SwWrtShell* pSh = getFrmDlgParentShell();

        // size
        const sal_Bool bSizeFixed = pSh->IsSelObjProtected( FLYPROTECT_FIXED );

        aWidthED .Enable( !bSizeFixed );
        aHeightED.Enable( !bSizeFixed );

        // size controls for math OLE objects
        if ( DLG_FRM_OLE == nDlgType && ! bNew )
        {
            // disable width and height for math objects
            const SvGlobalName& rFactNm( pSh->GetOLEObject()->getClassID() );

            struct _GlobalNameId {
                sal_uInt32 n1;
                sal_uInt16 n2, n3;
                sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
            } aGlbNmIds[4] = { { SO3_SM_CLASSID_60 }, { SO3_SM_CLASSID_50 },
                               { SO3_SM_CLASSID_40 }, { SO3_SM_CLASSID_30 } };

            for ( int i = 0; i < 4; ++i ) {
                const _GlobalNameId& rId = aGlbNmIds[ i ];

                SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                                     rId.b8, rId.b9, rId.b10, rId.b11,
                                     rId.b12, rId.b13, rId.b14, rId.b15 );

                if( rFactNm == aGlbNm )
                {
                    // disable size controls for math OLE objects
                    aWidthFT.Disable();
                    aWidthED.Disable();
                    aRelWidthCB.Disable();
                    aHeightFT.Disable();
                    aHeightED.Disable();
                    aRelHeightCB.Disable();
                    aFixedRatioCB.Disable();
                    aRealSizeBT.Disable();
                    break;
                }
            }

            // TODO/LATER: get correct aspect
            if(0 != (pSh->GetOLEObject()->getStatus( embed::Aspects::MSOLE_CONTENT ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
                aRealSizeBT.Disable();
        }
    }

    const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
    sal_Int64 nWidth  = aWidthED.NormalizePercent(rSize.GetWidth());
    sal_Int64 nHeight = aHeightED.NormalizePercent(rSize.GetHeight());

    if (nWidth != aWidthED.GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // value was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            aWidthED.SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            aWidthED.SetPrcntValue(nWidth, FUNIT_TWIP);
    }

    if (nHeight != aHeightED.GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // values was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            aHeightED.SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            aHeightED.SetPrcntValue(nHeight, FUNIT_TWIP);
    }

    if (!IsInGraficMode())
    {
        SwFrmSize eSize = rSize.GetHeightSizeType();
        sal_Bool bCheck = eSize != ATT_FIX_SIZE;
        aAutoHeightCB.Check( bCheck );
        HandleAutoCB( bCheck, aHeightFT, aHeightAutoFT );
        if( eSize == ATT_VAR_SIZE )
            aHeightED.SetValue( aHeightED.GetMin(), FUNIT_NONE );

        eSize = rSize.GetWidthSizeType();
        bCheck = eSize != ATT_FIX_SIZE;
        aAutoWidthCB.Check( bCheck );
        HandleAutoCB( bCheck, aWidthFT, aWidthAutoFT );
        if( eSize == ATT_VAR_SIZE )
            aWidthED.SetValue( aWidthED.GetMin(), FUNIT_NONE );

        if ( !bFormat )
        {
            SwWrtShell* pSh = getFrmDlgParentShell();
            const SwFrmFmt* pFmt = pSh->GetFlyFrmFmt();
            if( pFmt && pFmt->GetChain().GetNext() )
                aAutoHeightCB.Enable( sal_False );
        }
    }
    else
        aAutoHeightCB.Hide();

    // organise circulation-gap for character bound frames
    const SvxULSpaceItem &rUL = (const SvxULSpaceItem &)rSet.Get(RES_UL_SPACE);
    nUpperBorder = rUL.GetUpper();
    nLowerBorder = rUL.GetLower();

    if(SFX_ITEM_SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        aFixedRatioCB.Check(((const SfxBoolItem&)rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        aFixedRatioCB.SaveValue();
    }

    // columns
    SwFmtCol aCol( (const SwFmtCol&)rSet.Get(RES_COL) );
    ::FitToActualSize( aCol, (sal_uInt16)rSize.GetWidth() );

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();

    if ( bNew && !bFormat )
        InitPos(eAnchorId, USHRT_MAX, 0, USHRT_MAX, USHRT_MAX, LONG_MAX, LONG_MAX);
    else
    {
        const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)rSet.Get(RES_HORI_ORIENT);
        const SwFmtVertOrient& rVert = (const SwFmtVertOrient&)rSet.Get(RES_VERT_ORIENT);
        nOldH    = rHori.GetHoriOrient();
        nOldHRel = rHori.GetRelationOrient();
        nOldV    = rVert.GetVertOrient(),
        nOldVRel = rVert.GetRelationOrient();

        if (eAnchorId == FLY_AT_PAGE)
        {
            if (nOldHRel == text::RelOrientation::FRAME)
                nOldHRel = text::RelOrientation::PAGE_FRAME;
            else if (nOldHRel == text::RelOrientation::PRINT_AREA)
                nOldHRel = text::RelOrientation::PAGE_PRINT_AREA;
            if (nOldVRel == text::RelOrientation::FRAME)
                nOldVRel = text::RelOrientation::PAGE_FRAME;
            else if (nOldVRel == text::RelOrientation::PRINT_AREA)
                nOldVRel = text::RelOrientation::PAGE_PRINT_AREA;
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

    // transparent for example
    // circulation for example
    const SwFmtSurround& rSurround = (const SwFmtSurround&)rSet.Get(RES_SURROUND);
    aExampleWN.SetWrap ( static_cast< sal_uInt16 >(rSurround.GetSurround()) );

    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
    {
        const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)rSet.Get(RES_OPAQUE);
        aExampleWN.SetTransparent(!rOpaque.GetValue());
    }

    // switch to percent if applicable
    RangeModifyHdl(&aWidthED);  // set reference values (for 100%)

    if (rSize.GetWidthPercent() == 0xff || rSize.GetHeightPercent() == 0xff)
        aFixedRatioCB.Check(sal_True);
    if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != 0xff &&
        !aRelWidthCB.IsChecked())
    {
        aRelWidthCB.Check(sal_True);
        RelSizeClickHdl(&aRelWidthCB);
        aWidthED.SetPrcntValue(rSize.GetWidthPercent(), FUNIT_CUSTOM);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != 0xff &&
        !aRelHeightCB.IsChecked())
    {
        aRelHeightCB.Check(sal_True);
        RelSizeClickHdl(&aRelHeightCB);
        aHeightED.SetPrcntValue(rSize.GetHeightPercent(), FUNIT_CUSTOM);
    }
    aRelWidthCB.SaveValue();
    aRelHeightCB.SaveValue();
}

sal_uInt16* SwFrmPage::GetRanges()
{
    return aPageRg;
}

void SwFrmPage::SetFormatUsed(sal_Bool bFmt)
{
    bFormat     = bFmt;
    if(bFormat)
    {
        aAnchorAtPageRB.Hide();
        aAnchorAtParaRB.Hide();
        aAnchorAtCharRB.Hide();
        aAnchorAsCharRB.Hide();
        aAnchorAtFrameRB.Hide();
        aTypeFL.Hide();
        aTypeSepFL.Hide();
    }
}


void SwFrmPage::EnableVerticalPositioning( bool bEnable )
{
    m_bAllowVertPositioning = bEnable;
    aVerticalFT.Enable( bEnable );
    aVerticalDLB.Enable( bEnable );
    aAtVertPosFT.Enable( bEnable );
    aAtVertPosED.Enable( bEnable );
    aVertRelationFT.Enable( bEnable );
    aVertRelationLB.Enable( bEnable );
}


SwGrfExtPage::SwGrfExtPage(Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage( pParent, SW_RES(TP_GRF_EXT), rSet ),
    aMirrorFL       (this, SW_RES( FL_MIRROR )),
    aMirrorVertBox  (this, SW_RES( CB_VERT )),
    aMirrorHorzBox  (this, SW_RES( CB_HOR )),
    aAllPagesRB     (this, SW_RES( RB_MIRROR_ALL_PAGES )),
    aLeftPagesRB    (this, SW_RES( RB_MIRROR_LEFT_PAGES )),
    aRightPagesRB   (this, SW_RES( RB_MIRROR_RIGHT_PAGES )),
    aBmpWin         (this, WN_BMP, Graphic(), BitmapEx(SW_RES(BMP_EXAMPLE))),
    aConnectFL      (this, SW_RES( FL_CONNECT )),
    aConnectFT      (this, SW_RES( FT_CONNECT )),
    aConnectED      (this, SW_RES( ED_CONNECT )),
    aBrowseBT       (this, SW_RES( PB_BROWSE )),
    pGrfDlg         ( 0 )
{
    FreeResource();

    aBrowseBT.SetAccessibleRelationMemberOf(&aConnectFL);

    SetExchangeSupport();
    aMirrorHorzBox.SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    aMirrorVertBox.SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    aBrowseBT.SetClickHdl    ( LINK(this, SwGrfExtPage, BrowseHdl));
}

SwGrfExtPage::~SwGrfExtPage()
{
    delete pGrfDlg;
}

SfxTabPage* SwGrfExtPage::Create( Window *pParent, const SfxItemSet &rSet )
{
    return new SwGrfExtPage( pParent, rSet );
}

void SwGrfExtPage::Reset(const SfxItemSet &rSet)
{
    const SfxPoolItem* pItem;
    sal_uInt16 nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_GRF_CONNECT, sal_True, &pItem)
        && ((const SfxBoolItem *)pItem)->GetValue() )
    {
        aBrowseBT.Enable();
        aConnectED.SetReadOnly(sal_False);
    }

    ActivatePage(rSet);
}

void SwGrfExtPage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxProtectItem& rProt = (const SvxProtectItem& )rSet.Get(RES_PROTECT);
    sal_Bool bProtCntnt = rProt.IsCntntProtected();

    const SfxPoolItem* pItem = 0;
    sal_Bool bEnable = sal_False;
    sal_Bool bEnableMirrorRB = sal_False;

    SfxItemState eState = rSet.GetItemState(RES_GRFATR_MIRRORGRF, sal_True, &pItem);
    if( SFX_ITEM_UNKNOWN != eState && !bProtCntnt && !bHtmlMode )
    {
        if( SFX_ITEM_SET != eState )
            pItem = &rSet.Get( RES_GRFATR_MIRRORGRF );

        bEnable = sal_True;

        MirrorGraph eMirror = static_cast< MirrorGraph >(((const SwMirrorGrf* )pItem)->GetValue());
        switch( eMirror )
        {
        case RES_MIRROR_GRAPH_DONT: break;
        case RES_MIRROR_GRAPH_VERT: aMirrorHorzBox.Check(sal_True); break;
        case RES_MIRROR_GRAPH_HOR:  aMirrorVertBox.Check(sal_True); break;
        case RES_MIRROR_GRAPH_BOTH: aMirrorHorzBox.Check(sal_True);
                                    aMirrorVertBox.Check(sal_True);
                                    break;
        default:
            ;
        }

        sal_uInt16 nPos = ((const SwMirrorGrf* )pItem)->IsGrfToggle() ? 1 : 0;
        nPos += (eMirror == RES_MIRROR_GRAPH_VERT || eMirror == RES_MIRROR_GRAPH_BOTH)
                 ? 2 : 0;

        bEnableMirrorRB = nPos != 0;

        switch (nPos)
        {
            case 1: // mirror at left / even pages
                aLeftPagesRB.Check();
                aMirrorHorzBox.Check(sal_True);
                break;
            case 2: // mirror on all pages
                aAllPagesRB.Check();
                break;
            case 3: // mirror on right / odd pages
                aRightPagesRB.Check();
                break;
            default:
                aAllPagesRB.Check();
                break;
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
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
    aMirrorFL.Enable(bEnable);

    aAllPagesRB .SaveValue();
    aLeftPagesRB.SaveValue();
    aRightPagesRB.SaveValue();
    aMirrorHorzBox.SaveValue();
    aMirrorVertBox.SaveValue();

    aBmpWin.MirrorHorz( aMirrorVertBox.IsChecked() );
    aBmpWin.MirrorVert( aMirrorHorzBox.IsChecked() );
    aBmpWin.Invalidate();
}

sal_Bool SwGrfExtPage::FillItemSet( SfxItemSet &rSet )
{
    sal_Bool bModified = sal_False;
    if ( aMirrorHorzBox.GetSavedValue() != aMirrorHorzBox.IsChecked() ||
         aMirrorVertBox.GetSavedValue() != aMirrorVertBox.IsChecked() ||
         aAllPagesRB .GetSavedValue() != aAllPagesRB .IsChecked() ||
         aLeftPagesRB.GetSavedValue() != aLeftPagesRB.IsChecked() ||
         aRightPagesRB.GetSavedValue() != aRightPagesRB.IsChecked())
    {
        bModified = sal_True;

        sal_Bool bHori = sal_False;

        if (aMirrorHorzBox.IsChecked() &&
                !aLeftPagesRB.IsChecked())
            bHori = sal_True;

        MirrorGraph eMirror;
        eMirror = aMirrorVertBox.IsChecked() && bHori ?
                    RES_MIRROR_GRAPH_BOTH : bHori ?
                    RES_MIRROR_GRAPH_VERT : aMirrorVertBox.IsChecked() ?
                    RES_MIRROR_GRAPH_HOR  : RES_MIRROR_GRAPH_DONT;

        sal_Bool bMirror = !aAllPagesRB.IsChecked();
        SwMirrorGrf aMirror( eMirror );
        aMirror.SetGrfToggle(bMirror );
        rSet.Put( aMirror );
    }

    if( aGrfName != aNewGrfName || aConnectED.IsModified() )
    {
        bModified = sal_True;
        aGrfName = aConnectED.GetText();
        rSet.Put( SvxBrushItem( aGrfName, aFilterName, GPOS_LT,
                                SID_ATTR_GRAF_GRAPHIC ));
    }
    return bModified;
}

int SwGrfExtPage::DeactivatePage(SfxItemSet *_pSet)
{
    if( _pSet )
        FillItemSet( *_pSet );
    return sal_True;
}

IMPL_LINK_NOARG(SwGrfExtPage, BrowseHdl)
{
    if(!pGrfDlg)
    {
        pGrfDlg = new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
                SFXWB_GRAPHIC );
        pGrfDlg->SetTitle(SW_RESSTR(STR_EDIT_GRF ));
    }
    pGrfDlg->SetDisplayDirectory( aConnectED.GetText() );
    uno::Reference < ui::dialogs::XFilePicker > xFP = pGrfDlg->GetFilePicker();
    uno::Reference < ui::dialogs::XFilePickerControlAccess > xCtrlAcc(xFP, uno::UNO_QUERY);
    sal_Bool bTrue = sal_True;
    uno::Any aVal(&bTrue, ::getBooleanCppuType());
    xCtrlAcc->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aVal);

    if ( pGrfDlg->Execute() == ERRCODE_NONE )
    {   // remember selected filter
        aFilterName = pGrfDlg->GetCurrentFilter();
        aNewGrfName = INetURLObject::decode( pGrfDlg->GetPath(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 );
        aConnectED.SetModifyFlag();
        aConnectED.SetText( aNewGrfName );
        //reset mirrors because maybe a Bitmap was swapped with
        //another type of graphic that cannot be mirrored.
        aMirrorVertBox.Check(sal_False);
        aMirrorHorzBox.Check(sal_False);
        aAllPagesRB .Enable(sal_False);
        aLeftPagesRB.Enable(sal_False);
        aRightPagesRB.Enable(sal_False);
        aBmpWin.MirrorHorz(sal_False);
        aBmpWin.MirrorVert(sal_False);

        Graphic aGraphic;
        GraphicFilter::LoadGraphic( pGrfDlg->GetPath(), aEmptyStr, aGraphic );
        aBmpWin.SetGraphic(aGraphic);

        sal_Bool bEnable = GRAPHIC_BITMAP      == aGraphic.GetType() ||
                            GRAPHIC_GDIMETAFILE == aGraphic.GetType();
        aMirrorVertBox.Enable(bEnable);
        aMirrorHorzBox.Enable(bEnable);
        aAllPagesRB .Enable(bEnable);
        aLeftPagesRB.Enable(bEnable);
        aRightPagesRB.Enable(bEnable);
    }
    return 0;
}

IMPL_LINK_NOARG(SwGrfExtPage, MirrorHdl)
{
    sal_Bool bEnable = aMirrorHorzBox.IsChecked();

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
    Description: example window
 --------------------------------------------------------------------*/
BmpWindow::BmpWindow( Window* pPar, sal_uInt16 nId,
                        const Graphic& rGraphic, const BitmapEx& rBmp ) :
    Window(pPar, SW_RES(nId)),
    aGraphic(rGraphic),
    aBmp(rBmp),
    bHorz(sal_False),
    bVert(sal_False),
    bGraphic(sal_False),
    bLeftAlign(sal_False)
{
    SetBackground();
    SetPaintTransparent(sal_True);
}

void BmpWindow::Paint( const Rectangle& )
{
    Point aPntPos;
    Size  aPntSz( GetSizePixel() );
    Size  aGrfSize;
    if(bGraphic)
        aGrfSize = ::GetGraphicSizeTwip(aGraphic, this);
    //it should show the default bitmap also if no graphic can be found
    if(!aGrfSize.Width() && !aGrfSize.Height())
        aGrfSize = PixelToLogic(aBmp.GetSizePixel());

    long nRelGrf = aGrfSize.Width() * 100L / aGrfSize.Height();
    long nRelWin = aPntSz.Width() * 100L / aPntSz.Height();
    if(nRelGrf < nRelWin)
    {
        const long nWidth = aPntSz.Width();
        // if we use a replacement preview, try to draw at original size
        if ( !bGraphic && ( aGrfSize.Width() <= aPntSz.Width() ) && ( aGrfSize.Height() <= aPntSz.Height() ) )
        {
            const long nHeight = aPntSz.Height();
            aPntSz.Width() = aGrfSize.Width();
            aPntSz.Height() = aGrfSize.Height();
            aPntPos.Y() += (nHeight - aPntSz.Height()) / 2;
        }
        else
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

    if ( bGraphic )
        aGraphic.Draw( this, aPntPos, aPntSz );
    else
    {
        DrawBitmapEx( aPntPos, aPntSz, aBmp );
    }
}

BmpWindow::~BmpWindow()
{
}

void BmpWindow::SetGraphic(const Graphic& rGrf)
{
    aGraphic = rGrf;
    Size aGrfSize = ::GetGraphicSizeTwip(aGraphic, this);
    bGraphic = aGrfSize.Width() && aGrfSize.Height();
    Invalidate();
}

/***************************************************************************
    Description:    set URL and ImageMap at frames
***************************************************************************/
SwFrmURLPage::SwFrmURLPage( Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent,     SW_RES(TP_FRM_URL), rSet),
    aHyperLinkFL    (this, SW_RES( FL_HYPERLINK )),
    aURLFT          (this, SW_RES( FT_URL    )),
    aURLED          (this, SW_RES( ED_URL    )),
    aSearchPB       (this, SW_RES( PB_SEARCH  )),
    aNameFT         (this, SW_RES( FT_NAME   )),
    aNameED         (this, SW_RES( ED_NAME   )),
    aFrameFT        (this, SW_RES( FT_FRAME   )),
    aFrameCB        (this, SW_RES( CB_FRAME   )),

    aImageFL        (this, SW_RES( FL_IMAGE   )),
    aServerCB       (this, SW_RES( CB_SERVER  )),
    aClientCB       (this, SW_RES( CB_CLIENT  ))
{
    FreeResource();
    aSearchPB.SetClickHdl(LINK(this, SwFrmURLPage, InsertFileHdl));
}

SwFrmURLPage::~SwFrmURLPage()
{
}

void SwFrmURLPage::Reset( const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_DOCFRAME, sal_True, &pItem))
    {
        TargetList* pList = new TargetList;
        ((const SfxFrameItem*)pItem)->GetFrame()->GetTargetList(*pList);
        if( !pList->empty() )
        {
            size_t nCount = pList->size();
            for ( size_t i = 0; i < nCount; i++ )
            {
                aFrameCB.InsertEntry( *pList->at( i ) );
            }
            for ( size_t i = nCount; i; )
            {
                delete pList->at( --i );
            }
        }
        delete pList;
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( RES_URL, sal_True, &pItem ) )
    {
        const SwFmtURL* pFmtURL = (const SwFmtURL*)pItem;
        aURLED.SetText( INetURLObject::decode( pFmtURL->GetURL(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        aNameED.SetText( pFmtURL->GetName());

        aClientCB.Enable( pFmtURL->GetMap() != 0 );
        aClientCB.Check ( pFmtURL->GetMap() != 0 );
        aServerCB.Check ( pFmtURL->IsServerMap() );

        aFrameCB.SetText(pFmtURL->GetTargetFrameName());
        aFrameCB.SaveValue();
    }
    else
        aClientCB.Enable( sal_False );

    aServerCB.SaveValue();
    aClientCB.SaveValue();
}

sal_Bool SwFrmURLPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bModified = sal_False;
    const SwFmtURL* pOldURL = (SwFmtURL*)GetOldItem(rSet, RES_URL);
    SwFmtURL* pFmtURL;
    if(pOldURL)
        pFmtURL = (SwFmtURL*)pOldURL->Clone();
    else
        pFmtURL = new SwFmtURL();

    {
        String sText = aURLED.GetText();

        if( pFmtURL->GetURL() != sText ||
            pFmtURL->GetName() != aNameED.GetText() ||
            aServerCB.IsChecked() != pFmtURL->IsServerMap() )
        {
            pFmtURL->SetURL( sText, aServerCB.IsChecked() );
            pFmtURL->SetName( aNameED.GetText() );
            bModified = sal_True;
        }
    }

    if(!aClientCB.IsChecked() && pFmtURL->GetMap() != 0)
    {
        pFmtURL->SetMap(0);
        bModified = sal_True;
    }

    if(pFmtURL->GetTargetFrameName() != aFrameCB.GetText())
    {
        pFmtURL->SetTargetFrameName(aFrameCB.GetText());
        bModified = sal_True;
    }
    rSet.Put(*pFmtURL);
    delete pFmtURL;
    return bModified;
}

SfxTabPage* SwFrmURLPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmURLPage( pParent, rSet );
}

IMPL_LINK_NOARG(SwFrmURLPage, InsertFileHdl)
{
    FileDialogHelper aDlgHelper( ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    uno::Reference < ui::dialogs::XFilePicker > xFP = aDlgHelper.GetFilePicker();

    try
    {
        String sTemp(aURLED.GetText());
        if(sTemp.Len())
            xFP->setDisplayDirectory(sTemp);
    }
    catch( const uno::Exception& rEx )
    {
        (void) rEx;
    }
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        aURLED.SetText( xFP->getFiles().getConstArray()[0] );
    }

    return 0;
}

static void lcl_Move(Window& rWin, sal_Int32 nDiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() -= nDiff;
    rWin.SetPosPixel(aPos);
}

SwFrmAddPage::SwFrmAddPage(Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent,     SW_RES(TP_FRM_ADD), rSet),

    aNamesFL           (this, SW_RES(FL_NAME)),
    aNameFT            (this, SW_RES(FT_NAME)),
    aNameED            (this, SW_RES(ED_NAME)),
    aAltNameFT         (this, SW_RES(FT_ALT_NAME)),
    aAltNameED         (this, SW_RES(ED_ALT_NAME)),
    aPrevFT            (this, SW_RES(FT_PREV)),
    aPrevLB            (this, SW_RES(LB_PREV)),
    aNextFT            (this, SW_RES(FT_NEXT)),
    aNextLB            (this, SW_RES(LB_NEXT)),

    aProtectFL         (this, SW_RES(FL_PROTECT)),
    aProtectContentCB  (this, SW_RES(CB_PROTECT_CONTENT)),
    aProtectFrameCB    (this, SW_RES(CB_PROTECT_FRAME)),
    aProtectSizeCB     (this, SW_RES(CB_PROTECT_SIZE)),
    aExtFL             (this, SW_RES(FL_EXT)),

    aEditInReadonlyCB  (this, SW_RES(CB_EDIT_IN_READONLY)),
    aPrintFrameCB      (this, SW_RES(CB_PRINT_FRAME)),
    aTextFlowFT        (this, SW_RES(FT_TEXTFLOW)),
    aTextFlowLB        (this, SW_RES(LB_TEXTFLOW)),

    pWrtSh(0),

    nDlgType(0),
    bHtmlMode(sal_False),
    bFormat(sal_False),
    bNew(sal_False)
{
    FreeResource();

}

SwFrmAddPage::~SwFrmAddPage()
{
}

SfxTabPage* SwFrmAddPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmAddPage(pParent, rSet);
}

void SwFrmAddPage::Reset(const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    sal_uInt16 nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;
    if(bHtmlMode)
    {
        aProtectContentCB .Hide();
        aProtectFrameCB   .Hide();
        aProtectSizeCB    .Hide();
        aEditInReadonlyCB .Hide();
        aPrintFrameCB     .Hide();
        aExtFL            .Hide();
        aProtectFL.Hide();
    }
    if ( DLG_FRM_GRF == nDlgType || DLG_FRM_OLE == nDlgType )
    {
        aEditInReadonlyCB.Hide();
        aPrintFrameCB.SetPosPixel(aEditInReadonlyCB.GetPosPixel());
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_SET_FRM_ALT_NAME, sal_False, &pItem))
    {
        aAltNameED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aAltNameED.SaveValue();
    }

    if(!bFormat)
    {
        // insert graphic - properties
        // bNew is not set, so recognise by selection
        String aTmpName1;
        if(SFX_ITEM_SET == rSet.GetItemState(FN_SET_FRM_NAME, sal_False, &pItem))
        {
            aTmpName1 = ((const SfxStringItem*)pItem)->GetValue();
        }

        OSL_ENSURE(pWrtSh, "keine Shell?");
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
        aNameED.Enable( sal_False );
        aAltNameED.Enable(sal_False);
        aNameFT.Enable( sal_False );
        aAltNameFT.Enable(sal_False);
        aNamesFL.Enable(sal_False);
    }
    if(nDlgType == DLG_FRM_STD && aAltNameFT.IsVisible())
    {
        aAltNameFT.Hide();
        aAltNameED.Hide();
        //move all controls one step up
        Window* aWindows[] =
        {
            &aPrevFT,
            &aPrevLB,
            &aNextFT,
            &aNextLB,
            &aNamesFL,
            &aProtectContentCB,
            &aProtectFrameCB,
            &aProtectSizeCB,
            &aProtectFL,
            &aEditInReadonlyCB,
            &aPrintFrameCB,
            &aTextFlowFT,
            &aTextFlowLB,
            &aExtFL,
            0
        };
        sal_Int32 nOffset = aAltNameED.GetPosPixel().Y() - aNameED.GetPosPixel().Y();
        sal_Int32 nIdx = 0;
        while(aWindows[nIdx])
        {
            lcl_Move(*aWindows[nIdx++], nOffset);
        }
    }
    else
    {
        aNameED.SetModifyHdl(LINK(this, SwFrmAddPage, EditModifyHdl));
    }

    if (!bNew)
    {
        SwFrmFmt* pFmt = pWrtSh->GetFlyFrmFmt();

        if (pFmt)
        {
            const SwFmtChain &rChain = pFmt->GetChain();
            const SwFlyFrmFmt* pFlyFmt;
            String sNextChain, sPrevChain;
            if ((pFlyFmt = rChain.GetPrev()) != 0)
            {
                sPrevChain = pFlyFmt->GetName();
            }

            if ((pFlyFmt = rChain.GetNext()) != 0)
            {
                sNextChain = pFlyFmt->GetName();
            }
            //determine chainable frames
            ::std::vector< String > aPrevPageFrames;
            ::std::vector< String > aThisPageFrames;
            ::std::vector< String > aNextPageFrames;
            ::std::vector< String > aRemainFrames;
            pWrtSh->GetConnectableFrmFmts(*pFmt, sNextChain, sal_False,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            lcl_InsertVectors(aPrevLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(sPrevChain.Len())
            {
                if(LISTBOX_ENTRY_NOTFOUND == aPrevLB.GetEntryPos(sPrevChain))
                    aPrevLB.InsertEntry(sPrevChain, 1);
                aPrevLB.SelectEntry(sPrevChain);
            }
            else
                aPrevLB.SelectEntryPos(0);
            aPrevPageFrames.erase(aPrevPageFrames.begin(), aPrevPageFrames.end());
            aNextPageFrames.erase(aNextPageFrames.begin(), aNextPageFrames.end());
            aThisPageFrames.erase(aThisPageFrames.begin(), aThisPageFrames.end());
            aRemainFrames.erase(aRemainFrames.begin(), aRemainFrames.end());

            pWrtSh->GetConnectableFrmFmts(*pFmt, sPrevChain, sal_True,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            lcl_InsertVectors(aNextLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(sNextChain.Len())
            {
                if(LISTBOX_ENTRY_NOTFOUND == aNextLB.GetEntryPos(sNextChain))
                    aNextLB.InsertEntry(sNextChain, 1);
                aNextLB.SelectEntry(sNextChain);
            }
            else
                aNextLB.SelectEntryPos(0);
            Link aLink(LINK(this, SwFrmAddPage, ChainModifyHdl));
            aPrevLB.SetSelectHdl(aLink);
            aNextLB.SetSelectHdl(aLink);
        }
    }
    // Pos Protected
    const SvxProtectItem& rProt = (const SvxProtectItem& )rSet.Get(RES_PROTECT);
    aProtectFrameCB.Check(rProt.IsPosProtected());
    aProtectContentCB.Check(rProt.IsCntntProtected());
    aProtectSizeCB.Check(rProt.IsSizeProtected());

    const SwFmtEditInReadonly& rEdit = (const SwFmtEditInReadonly& )rSet.Get(RES_EDIT_IN_READONLY);
    aEditInReadonlyCB.Check(rEdit.GetValue());          aEditInReadonlyCB.SaveValue();

    // print
    const SvxPrintItem& rPrt = (const SvxPrintItem&)rSet.Get(RES_PRINT);
    aPrintFrameCB.Check(rPrt.GetValue());               aPrintFrameCB.SaveValue();

    // textflow
    SfxItemState eState;
    if( (!bHtmlMode || (0 != (nHtmlMode&HTMLMODE_SOME_STYLES)))
            && DLG_FRM_GRF != nDlgType && DLG_FRM_OLE != nDlgType &&
        SFX_ITEM_UNKNOWN != ( eState = rSet.GetItemState(
                                        RES_FRAMEDIR, sal_True )) )
    {
        aTextFlowFT.Show();
        aTextFlowLB.Show();

        //vertical text flow is not possible in HTML
        if(bHtmlMode)
        {
            sal_uLong nData = FRMDIR_VERT_TOP_RIGHT;
            aTextFlowLB.RemoveEntry(aTextFlowLB.GetEntryPos((void*)nData));
        }
        sal_uInt16 nPos, nVal = ((SvxFrameDirectionItem&)rSet.Get(RES_FRAMEDIR)).GetValue();
        for( nPos = aTextFlowLB.GetEntryCount(); nPos; )
            if( (sal_uInt16)(long)aTextFlowLB.GetEntryData( --nPos ) == nVal )
                break;
        aTextFlowLB.SelectEntryPos( nPos );
        aTextFlowLB.SaveValue();
    }
    else
    {
        aTextFlowFT.Hide();
        aTextFlowLB.Hide();
    }
}

sal_Bool SwFrmAddPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bRet = sal_False;
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

    sal_Bool bChecked;
    if ( (bChecked = aEditInReadonlyCB.IsChecked()) != aEditInReadonlyCB.GetSavedValue() )
        bRet |= 0 != rSet.Put( SwFmtEditInReadonly( RES_EDIT_IN_READONLY, bChecked));

    if ( (bChecked = aPrintFrameCB.IsChecked()) != aPrintFrameCB.GetSavedValue() )
        bRet |= 0 != rSet.Put( SvxPrintItem( RES_PRINT, bChecked));

    // textflow
    if( aTextFlowLB.IsVisible() )
    {
        sal_uInt16 nPos = aTextFlowLB.GetSelectEntryPos();
        if( nPos != aTextFlowLB.GetSavedValue() )
        {
            nPos = (sal_uInt16)(long)aTextFlowLB.GetEntryData( nPos );
            bRet |= 0 != rSet.Put( SvxFrameDirectionItem(
                                    (SvxFrameDirection)nPos, RES_FRAMEDIR ));
        }
    }
    if(pWrtSh)
    {
        const SwFrmFmt* pFmt = pWrtSh->GetFlyFrmFmt();
        if (pFmt)
        {
            String sCurrentPrevChain, sCurrentNextChain;
            if(aPrevLB.GetSelectEntryPos())
                sCurrentPrevChain = aPrevLB.GetSelectEntry();
            if(aNextLB.GetSelectEntryPos())
                sCurrentNextChain = aNextLB.GetSelectEntry();
            const SwFmtChain &rChain = pFmt->GetChain();
            const SwFlyFrmFmt* pFlyFmt;
            String sNextChain, sPrevChain;
            if ((pFlyFmt = rChain.GetPrev()) != 0)
                sPrevChain = pFlyFmt->GetName();

            if ((pFlyFmt = rChain.GetNext()) != 0)
                sNextChain = pFlyFmt->GetName();
            if(sPrevChain != sCurrentPrevChain)
                bRet |= 0 != rSet.Put(SfxStringItem(FN_PARAM_CHAIN_PREVIOUS, sCurrentPrevChain));
            if(sNextChain != sCurrentNextChain)
                bRet |= 0 != rSet.Put(SfxStringItem(FN_PARAM_CHAIN_NEXT, sCurrentNextChain));
        }
    }
    return bRet;
}

sal_uInt16* SwFrmAddPage::GetRanges()
{
    return aAddPgRg;
}

IMPL_LINK_NOARG(SwFrmAddPage, EditModifyHdl)
{
    sal_Bool bEnable = 0 != aNameED.GetText().Len();
    aAltNameED.Enable(bEnable);
    aAltNameFT.Enable(bEnable);

    return 0;
}

void    SwFrmAddPage::SetFormatUsed(sal_Bool bFmt)
{
    bFormat  = bFmt;
    if(bFormat)
    {
        aNameFT.Show(sal_False);
        aNameED.Show(sal_False);
        aAltNameFT.Show(sal_False);
        aAltNameED.Show(sal_False);
        aPrevFT.Show(sal_False);
        aPrevLB.Show(sal_False);
        aNextFT.Show(sal_False);
        aNextLB.Show(sal_False);
        aNamesFL.Show(sal_False);

        sal_Int32 nDiff = aExtFL.GetPosPixel().Y() - aNamesFL.GetPosPixel().Y();
        Window* aWindows[] =
        {
            &aProtectContentCB,
            &aProtectFrameCB,
            &aProtectSizeCB,
            &aProtectFL,
            &aEditInReadonlyCB,
            &aPrintFrameCB,
            &aExtFL,
            &aTextFlowFT,
            &aTextFlowLB,
            0
        };
        sal_Int32 nIdx = 0;
        while(aWindows[nIdx])
            lcl_Move(*aWindows[nIdx++], nDiff);
    }
}

IMPL_LINK(SwFrmAddPage, ChainModifyHdl, ListBox*, pBox)
{
    String sCurrentPrevChain, sCurrentNextChain;
    if(aPrevLB.GetSelectEntryPos())
        sCurrentPrevChain = aPrevLB.GetSelectEntry();
    if(aNextLB.GetSelectEntryPos())
        sCurrentNextChain = aNextLB.GetSelectEntry();
    SwFrmFmt* pFmt = pWrtSh->GetFlyFrmFmt();
    if (pFmt)
    {
        sal_Bool bNextBox = &aNextLB == pBox;
        ListBox& rChangeLB = bNextBox ? aPrevLB : aNextLB;
        for(sal_uInt16 nEntry = rChangeLB.GetEntryCount(); nEntry > 1; nEntry--)
            rChangeLB.RemoveEntry(nEntry - 1);
        //determine chainable frames
        ::std::vector< String > aPrevPageFrames;
        ::std::vector< String > aThisPageFrames;
        ::std::vector< String > aNextPageFrames;
        ::std::vector< String > aRemainFrames;
        pWrtSh->GetConnectableFrmFmts(*pFmt, bNextBox ? sCurrentNextChain : sCurrentPrevChain, !bNextBox,
                        aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
        lcl_InsertVectors(rChangeLB,
                aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
        String sToSelect = bNextBox ? sCurrentPrevChain : sCurrentNextChain;
        if(rChangeLB.GetEntryPos(sToSelect) != LISTBOX_ENTRY_NOTFOUND)
            rChangeLB.SelectEntry(sToSelect);
        else
            rChangeLB.SelectEntryPos(0);

    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
