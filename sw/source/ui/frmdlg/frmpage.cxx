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

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>

#include <cmdid.h>
#include <helpid.h>
#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/swframevalidation.hxx>
#include <comphelper/classids.hxx>

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
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <vcl/graphicfilter.hxx>

using namespace ::com::sun::star;
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

static void lcl_InsertVectors(ListBox& rBox,
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
static SvxSwFramePosString::StringId lcl_ChangeResIdToVerticalOrRTL(SvxSwFramePosString::StringId eStringId, bool bVertical, sal_Bool bVerticalL2R, sal_Bool bRTL)
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

SwFrmPage::SwFrmPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FrmTypePage",
        "modules/swriter/ui/frmtypepage.ui", rSet)
    ,
    bAtHorzPosModified( sal_False ),
    bAtVertPosModified( sal_False ),
    bFormat(sal_False),
    bNew(sal_True),
    bNoModifyHdl(sal_True),
    // --> OD 2009-08-31 #mongolianlayout# - no used
//    bVerticalChanged(sal_False),
    bIsVerticalFrame(false),
    bIsVerticalL2R(sal_False),
    bIsInRightToLeft(sal_False),
    bHtmlMode(sal_False),
    nHtmlMode(0),
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
    get(m_pWidthFT, "widthft");
    get(m_pWidthAutoFT, "autowidthft");
    m_aWidthED.set(get<MetricField>("width"));
    get(m_pRelWidthCB, "relwidth");
    get(m_pAutoWidthCB, "autowidth");

    get(m_pHeightFT, "heightft");
    get(m_pHeightAutoFT, "autoheightft");
    m_aHeightED.set(get<MetricField>("height"));
    get(m_pRelHeightCB, "relheight");
    get(m_pAutoHeightCB, "autoheight");

    get(m_pFixedRatioCB, "ratio");
    get(m_pRealSizeBT, "origsize");

    get(m_pAnchorFrame, "anchorframe");
    get(m_pAnchorAtPageRB, "topage");
    get(m_pAnchorAtParaRB, "topara");
    get(m_pAnchorAtCharRB, "tochar");
    get(m_pAnchorAsCharRB, "aschar");
    get(m_pAnchorAtFrameRB, "toframe");

    get(m_pHorizontalFT, "horiposft");
    get(m_pHorizontalDLB, "horipos");
    get(m_pAtHorzPosFT, "horibyft");
    get(m_pAtHorzPosED, "byhori");
    get(m_pHoriRelationFT, "horitoft");
    get(m_pHoriRelationLB, "horianchor");

    get(m_pMirrorPagesCB, "mirror");

    get(m_pVerticalFT, "vertposft");
    get(m_pVerticalDLB, "vertpos");
    get(m_pAtVertPosFT, "vertbyft");
    get(m_pAtVertPosED, "byvert");
    get(m_pVertRelationFT, "verttoft");
    get(m_pVertRelationLB, "vertanchor");

    get(m_pFollowTextFlowCB, "followtextflow");
    get(m_pExampleWN, "preview");

    m_pAtHorzPosED->set_width_request(m_pAtHorzPosED->GetOptimalSize().Width());
    m_pAtHorzPosED->set_width_request(m_pAtVertPosED->GetOptimalSize().Width());

    setOptimalFrmWidth();
    setOptimalRelWidth();

    SetExchangeSupport();

    Link aLk = LINK(this, SwFrmPage, RangeModifyHdl);
    m_aWidthED.SetLoseFocusHdl( aLk );
    m_aHeightED.SetLoseFocusHdl( aLk );
    m_pAtHorzPosED->SetLoseFocusHdl( aLk );
    m_pAtVertPosED->SetLoseFocusHdl( aLk );
    m_pFollowTextFlowCB->SetClickHdl( aLk );

    aLk = LINK(this, SwFrmPage, ModifyHdl);
    m_aWidthED.SetModifyHdl( aLk );
    m_aHeightED.SetModifyHdl( aLk );
    m_pAtHorzPosED->SetModifyHdl( aLk );
    m_pAtVertPosED->SetModifyHdl( aLk );

    aLk = LINK(this, SwFrmPage, AnchorTypeHdl);
    m_pAnchorAtPageRB->SetClickHdl( aLk );
    m_pAnchorAtParaRB->SetClickHdl( aLk );
    m_pAnchorAtCharRB->SetClickHdl( aLk );
    m_pAnchorAsCharRB->SetClickHdl( aLk );
    m_pAnchorAtFrameRB->SetClickHdl( aLk );

    m_pHorizontalDLB->SetSelectHdl(LINK(this, SwFrmPage, PosHdl));
    m_pVerticalDLB->  SetSelectHdl(LINK(this, SwFrmPage, PosHdl));

    m_pHoriRelationLB->SetSelectHdl(LINK(this, SwFrmPage, RelHdl));
    m_pVertRelationLB->SetSelectHdl(LINK(this, SwFrmPage, RelHdl));

    m_pMirrorPagesCB->SetClickHdl(LINK(this, SwFrmPage, MirrorHdl));

    aLk = LINK(this, SwFrmPage, RelSizeClickHdl);
    m_pRelWidthCB->SetClickHdl( aLk );
    m_pRelHeightCB->SetClickHdl( aLk );

    m_pAutoWidthCB->SetClickHdl( LINK( this, SwFrmPage, AutoWidthClickHdl ) );
    m_pAutoHeightCB->SetClickHdl( LINK( this, SwFrmPage, AutoHeightClickHdl ) );
}

namespace
{
    struct FrmMaps
    {
        FrmMap *pMap;
        size_t nCount;
    };
}

void SwFrmPage::setOptimalFrmWidth()
{
    const FrmMaps aMaps[] = {
        { aHPageMap, SAL_N_ELEMENTS(aHPageMap) },
        { aHPageHtmlMap, SAL_N_ELEMENTS(aHPageHtmlMap) },
        { aVPageMap, SAL_N_ELEMENTS(aVPageMap) },
        { aVPageHtmlMap, SAL_N_ELEMENTS(aVPageHtmlMap) },
        { aHFrameMap, SAL_N_ELEMENTS(aHFrameMap) },
        { aHFlyHtmlMap, SAL_N_ELEMENTS(aHFlyHtmlMap) },
        { aVFrameMap, SAL_N_ELEMENTS(aVFrameMap) },
        { aVFlyHtmlMap, SAL_N_ELEMENTS(aVFrameMap) },
        { aHParaMap, SAL_N_ELEMENTS(aHParaMap) },
        { aHParaHtmlMap, SAL_N_ELEMENTS(aHParaHtmlMap) },
        { aHParaHtmlAbsMap, SAL_N_ELEMENTS(aHParaHtmlAbsMap) },
        { aVParaMap, SAL_N_ELEMENTS(aVParaMap) },
        { aVParaHtmlMap, SAL_N_ELEMENTS(aVParaHtmlMap) },
        { aHCharMap, SAL_N_ELEMENTS(aHCharMap) },
        { aHCharHtmlMap, SAL_N_ELEMENTS(aHCharHtmlMap) },
        { aHCharHtmlAbsMap, SAL_N_ELEMENTS(aHCharHtmlAbsMap) },
        { aVCharMap, SAL_N_ELEMENTS(aVCharMap) },
        { aVCharHtmlMap, SAL_N_ELEMENTS(aVCharHtmlMap) },
        { aVCharHtmlAbsMap, SAL_N_ELEMENTS(aVCharHtmlAbsMap) },
        { aVAsCharMap, SAL_N_ELEMENTS(aVAsCharMap) },
        { aVAsCharHtmlMap, SAL_N_ELEMENTS(aVAsCharHtmlMap) }
    };

    std::vector<SvxSwFramePosString::StringId> aFrames;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aMaps); ++i)
    {
        for (size_t j = 0; j < aMaps[i].nCount; ++j)
        {
            aFrames.push_back(aMaps[i].pMap[j].eStrId);
            aFrames.push_back(aMaps[i].pMap[j].eMirrorStrId);
        }
    }

    std::sort(aFrames.begin(), aFrames.end());
    aFrames.erase(std::unique(aFrames.begin(), aFrames.end()), aFrames.end());

    for (std::vector<SvxSwFramePosString::StringId>::const_iterator aI = aFrames.begin(), aEnd = aFrames.end();
        aI != aEnd; ++aI)
    {
        m_pHorizontalDLB->InsertEntry(aFramePosString.GetString(*aI));
    }

    Size aBiggest(m_pHorizontalDLB->GetOptimalSize());
    m_pHorizontalDLB->set_width_request(aBiggest.Width());
    m_pVerticalDLB->set_width_request(aBiggest.Width());
    m_pHorizontalDLB->Clear();
}

namespace
{
    struct RelationMaps
    {
        RelationMap *pMap;
        size_t nCount;
    };
}

void SwFrmPage::setOptimalRelWidth()
{
    const RelationMaps aMaps[] = {
        { aRelationMap, SAL_N_ELEMENTS(aRelationMap) },
        { aAsCharRelationMap, SAL_N_ELEMENTS(aAsCharRelationMap) }
    };

    std::vector<SvxSwFramePosString::StringId> aRels;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aMaps); ++i)
    {
        for (size_t j = 0; j < aMaps[i].nCount; ++j)
        {
            aRels.push_back(aMaps[i].pMap[j].eStrId);
            aRels.push_back(aMaps[i].pMap[j].eMirrorStrId);
        }
    }

    std::sort(aRels.begin(), aRels.end());
    aRels.erase(std::unique(aRels.begin(), aRels.end()), aRels.end());

    for (std::vector<SvxSwFramePosString::StringId>::const_iterator aI = aRels.begin(), aEnd = aRels.end();
        aI != aEnd; ++aI)
    {
        m_pHoriRelationLB->InsertEntry(aFramePosString.GetString(*aI));
    }

    Size aBiggest(m_pHoriRelationLB->GetOptimalSize());
    m_pHoriRelationLB->set_width_request(aBiggest.Width());
    m_pVertRelationLB->set_width_request(aBiggest.Width());
    m_pHoriRelationLB->Clear();
}


SwFrmPage::~SwFrmPage()
{
}

SfxTabPage* SwFrmPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwFrmPage( pParent, rSet );
}

void SwFrmPage::EnableGraficMode( void )
{
    // i#39692 - mustn't be called more than once
    if(!m_pRealSizeBT->IsVisible())
    {
        m_pWidthFT->Show();
        m_pWidthAutoFT->Hide();
        m_pAutoHeightCB->Hide();

        m_pHeightFT->Show();
        m_pHeightAutoFT->Hide();
        m_pAutoWidthCB->Hide();

        m_pRealSizeBT->Show();
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
    m_aWidthED.SetMetric(aMetric);
    m_aHeightED.SetMetric(aMetric);
    SetMetric( *m_pAtHorzPosED, aMetric );
    SetMetric( *m_pAtVertPosED, aMetric );

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
        m_pAnchorFrame->Enable(false);
        m_pFixedRatioCB->Enable(sal_False);
    }
    else
    {
        if (rAnchor.GetAnchorId() != FLY_AT_FLY && !pSh->IsFlyInFly())
            m_pAnchorAtFrameRB->Hide();
        // --> OD 2009-08-31 #mongolianlayout#
//        if ( !bVerticalChanged && pSh->IsFrmVertical(sal_True, bIsInRightToLeft) )
        if ( pSh->IsFrmVertical( sal_True, bIsInRightToLeft, bIsVerticalL2R ) )
        {
            String sHLabel = m_pHorizontalFT->GetText();
            m_pHorizontalFT->SetText(m_pVerticalFT->GetText());
            m_pVerticalFT->SetText(sHLabel);
            bIsVerticalFrame = true;
        }
    }

    if ( sDlgType == "PictureDialog" || sDlgType == "ObjectDialog" )
    {
        OSL_ENSURE(pSh , "shell not found");
        //OS: only for the variant Insert/Graphic/Properties
        if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_GRF_REALSIZE, sal_False, &pItem))
            aGrfSize = ((const SvxSizeItem*)pItem)->GetSize();
        else
            pSh->GetGrfSize( aGrfSize );

        if ( !bNew )
        {
            m_pRealSizeBT->SetClickHdl(LINK(this, SwFrmPage, RealSizeHdl));
            EnableGraficMode();
        }

        if ( sDlgType == "PictureDialog" )
            m_pFixedRatioCB->Check( sal_False );
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
    m_aWidthED.SetBaseValue( m_aWidthED.Normalize(aGrfSize.Width()), FUNIT_TWIP );
    m_aHeightED.SetBaseValue( m_aHeightED.Normalize(aGrfSize.Height()), FUNIT_TWIP );
    //the available space is not yet known so the RefValue has to be calculated from size and relative size values
    //this is needed only if relative values are already set

    const SwFmtFrmSize& rFrmSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);

    if (rFrmSize.GetWidthPercent() != 0xff && rFrmSize.GetWidthPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrmSize.GetWidth() * 100 / rFrmSize.GetWidthPercent();
        m_aWidthED.SetRefValue( nSpace );
    }

    if (rFrmSize.GetHeightPercent() != 0xff && rFrmSize.GetHeightPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrmSize.GetHeight() * 100 / rFrmSize.GetHeightPercent();
        m_aHeightED.SetRefValue( nSpace );
    }

    // general initialisation part
    switch(rAnchor.GetAnchorId())
    {
        case FLY_AT_PAGE: m_pAnchorAtPageRB->Check(); break;
        case FLY_AT_PARA: m_pAnchorAtParaRB->Check(); break;
        case FLY_AT_CHAR: m_pAnchorAtCharRB->Check(); break;
        case FLY_AS_CHAR: m_pAnchorAsCharRB->Check(); break;
        case FLY_AT_FLY: m_pAnchorAtFrameRB->Check();break;
        default:; //prevent warning
    }

    // i#22341 - determine content position of character
    // Note: content position can be NULL
    mpToCharCntntPos = rAnchor.GetCntntAnchor();

    // i#18732 - init checkbox value
    {
        const bool bFollowTextFlow =
            static_cast<const SwFmtFollowTextFlow&>(rSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
        m_pFollowTextFlowCB->Check( bFollowTextFlow );
    }

    if(bHtmlMode)
    {
        m_pAutoHeightCB->Enable(sal_False);
        m_pAutoWidthCB->Enable(sal_False);
        m_pMirrorPagesCB->Show(sal_False);
        if (sDlgType == "FrameDialog")
            m_pFixedRatioCB->Enable(sal_False);
        // i#18732 hide checkbox in HTML mode
        m_pFollowTextFlowCB->Show(sal_False);
    }
    else
    {
        // enable/disable of check box 'Mirror on..'
        m_pMirrorPagesCB->Enable(!m_pAnchorAsCharRB->IsChecked());

        // enable/disable check box 'Follow text flow'.
        // enable check box 'Follow text
        // flow' also for anchor type to-frame.
        m_pFollowTextFlowCB->Enable( m_pAnchorAtParaRB->IsChecked() ||
                                  m_pAnchorAtCharRB->IsChecked() ||
                                  m_pAnchorAtFrameRB->IsChecked() );
    }

    Init( rSet, sal_True );
    m_pAtVertPosED->SaveValue();
    m_pAtHorzPosED->SaveValue();
    m_pFollowTextFlowCB->SaveValue();

    bNoModifyHdl = sal_False;
    //lock PercentFields
    m_aWidthED.LockAutoCalculation(sal_True);
    m_aHeightED.LockAutoCalculation(sal_True);
    RangeModifyHdl(&m_aWidthED);  // set all maximum values initially
    m_aHeightED.LockAutoCalculation(sal_False);
    m_aWidthED.LockAutoCalculation(sal_False);

    m_pAutoHeightCB->SaveValue();
    m_pAutoWidthCB->SaveValue();

    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
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

        sal_uInt16 nMapPos = GetMapPos(pHMap, *m_pHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB);
        short nRel = GetRelation(pHMap, *m_pHoriRelationLB);

        const sal_Int16 eHOri = nAlign;
        const sal_Int16 eRel  = nRel;

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(m_pMirrorPagesCB->IsChecked());

        sal_Bool bMod = m_pAtHorzPosED->GetText() != m_pAtHorzPosED->GetSavedValue();
        bMod |= m_pMirrorPagesCB->GetState() != m_pMirrorPagesCB->GetSavedValue();

        if ( eHOri == text::HoriOrientation::NONE &&
             (bNew || (bAtHorzPosModified || bMod) || nOldH != eHOri ) )
        {
            SwTwips nX = static_cast< SwTwips >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)));
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

        sal_uInt16 nMapPos = GetMapPos(pVMap, *m_pVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB);
        short nRel = GetRelation(pVMap, *m_pVertRelationLB);

        const sal_Int16     eVOri = nAlign;
        const sal_Int16     eRel  = nRel;

        aVertOrient.SetVertOrient    ( eVOri);
        aVertOrient.SetRelationOrient( eRel );

        sal_Bool bMod = m_pAtVertPosED->GetText() != m_pAtVertPosED->GetSavedValue();

        if ( eVOri == text::VertOrientation::NONE &&
             ( bNew || (bAtVertPosModified || bMod) || nOldV != eVOri) )
        {
            // vertival position
            // recalculate offset for character bound frames
            SwTwips nY = static_cast< SwTwips >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)));
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

    bool bValueModified = (m_aWidthED.IsValueModified() || m_aHeightED.IsValueModified());
    bool bCheckChanged = (m_pRelWidthCB->GetSavedValue() != m_pRelWidthCB->IsChecked()
                        || m_pRelHeightCB->GetSavedValue() != m_pRelHeightCB->IsChecked());

    bool bLegalValue = !(!rOldSize.GetWidth () && !rOldSize.GetHeight() &&
                            m_aWidthED .GetValue() == m_aWidthED .GetMin() &&
                            m_aHeightED.GetValue() == m_aHeightED.GetMin());

    if ((bNew && !bFormat) || ((bValueModified || bCheckChanged) && bLegalValue))
    {
        sal_Int64 nNewWidth  = m_aWidthED.DenormalizePercent(m_aWidthED.GetRealValue(FUNIT_TWIP));
        sal_Int64 nNewHeight = m_aHeightED.DenormalizePercent(m_aHeightED.GetRealValue(FUNIT_TWIP));
        aSz.SetWidth (static_cast< SwTwips >(nNewWidth));
        aSz.SetHeight(static_cast< SwTwips >(nNewHeight));

        if (m_pRelWidthCB->IsChecked())
        {
            aSz.SetWidthPercent((sal_uInt8)std::min( static_cast< sal_Int64 >(MAX_PERCENT_WIDTH), m_aWidthED.Convert(m_aWidthED.NormalizePercent(nNewWidth), FUNIT_TWIP, FUNIT_CUSTOM)));
        }
        else
            aSz.SetWidthPercent(0);
        if (m_pRelHeightCB->IsChecked())
            aSz.SetHeightPercent((sal_uInt8)std::min(static_cast< sal_Int64 >(MAX_PERCENT_HEIGHT), m_aHeightED.Convert(m_aHeightED.NormalizePercent(nNewHeight), FUNIT_TWIP, FUNIT_CUSTOM)));
        else
            aSz.SetHeightPercent(0);

        if (m_pFixedRatioCB->IsChecked() && (m_pRelWidthCB->IsChecked() ^ m_pRelHeightCB->IsChecked()))
        {
            if (m_pRelWidthCB->IsChecked())
                aSz.SetHeightPercent(0xff);
            else
                aSz.SetWidthPercent(0xff);
        }
    }
    if( !IsInGraficMode() )
    {
        if( m_pAutoHeightCB->GetState() != m_pAutoHeightCB->GetSavedValue() )
        {
            SwFrmSize eFrmSize = (SwFrmSize) m_pAutoHeightCB->IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrmSize != aSz.GetHeightSizeType() )
                aSz.SetHeightSizeType(eFrmSize);
        }
        if( m_pAutoWidthCB->GetState() != m_pAutoWidthCB->GetSavedValue() )
        {
            SwFrmSize eFrmSize = (SwFrmSize) m_pAutoWidthCB->IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrmSize != aSz.GetWidthSizeType() )
                aSz.SetWidthSizeType( eFrmSize );
        }
    }
    if( !bFormat && m_pFixedRatioCB->GetSavedValue() != m_pFixedRatioCB->IsChecked())
        bRet |= 0 != rSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, m_pFixedRatioCB->IsChecked()));

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
    if(m_pFollowTextFlowCB->IsChecked() != m_pFollowTextFlowCB->GetSavedValue())
    {
        bRet |= 0 != rSet.Put(SwFmtFollowTextFlow(m_pFollowTextFlowCB->IsChecked()));
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
    sal_uInt16 nPos = m_pVerticalDLB->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && pVMap )
    {
        nOldV    = pVMap[nPos].nAlign;

        nPos = m_pVertRelationLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            nOldVRel = ((RelationMap *)m_pVertRelationLB->GetEntryData(nPos))->nRelation;
    }

    nPos = m_pHorizontalDLB->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && pHMap )
    {
        nOldH    = pHMap[nPos].nAlign;

        nPos = m_pHoriRelationLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            nOldHRel = ((RelationMap *)m_pHoriRelationLB->GetEntryData(nPos))->nRelation;
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
    m_pHorizontalDLB->Enable( bEnable );
    m_pHorizontalFT->Enable( bEnable );

    // select current Pos
    // horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = nOldH;
        nHRel = nOldHRel;
    }
    sal_uInt16 nMapPos = FillPosLB(pHMap, nH, nHRel, *m_pHorizontalDLB);
    FillRelLB(pHMap, nMapPos, nH, nHRel, *m_pHoriRelationLB, *m_pHoriRelationFT);

    // vertical
    if ( nV == USHRT_MAX )
    {
        nV    = nOldV;
        nVRel = nOldVRel;
    }
    nMapPos = FillPosLB(pVMap, nV, nVRel, *m_pVerticalDLB);
    FillRelLB(pVMap, nMapPos, nV, nVRel, *m_pVertRelationLB, *m_pVertRelationFT);

    bEnable = nH == text::HoriOrientation::NONE && eId != FLY_AS_CHAR;
    if (!bEnable)
    {
        m_pAtHorzPosED->SetValue( 0, FUNIT_TWIP );
        if (nX != LONG_MAX && bHtmlMode)
            m_pAtHorzPosED->SetModifyFlag();
    }
    else
    {
        if (nX != LONG_MAX)
            m_pAtHorzPosED->SetValue( m_pAtHorzPosED->Normalize(nX), FUNIT_TWIP );
    }
    m_pAtHorzPosFT->Enable( bEnable );
    m_pAtHorzPosED->Enable( bEnable );

    bEnable = nV == text::VertOrientation::NONE;
    if ( !bEnable )
    {
        m_pAtVertPosED->SetValue( 0, FUNIT_TWIP );
        if(nY != LONG_MAX && bHtmlMode)
            m_pAtVertPosED->SetModifyFlag();
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
            m_pAtVertPosED->SetValue( m_pAtVertPosED->Normalize(nY), FUNIT_TWIP );
    }
    m_pAtVertPosFT->Enable( bEnable && m_bAllowVertPositioning );
    m_pAtVertPosED->Enable( bEnable && m_bAllowVertPositioning );
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
            SvxSwFramePosString::StringId eStrId = m_pMirrorPagesCB->IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
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
                                             ( m_pMirrorPagesCB->IsChecked()
                                               ? _pMap[_nLBSelPos].eMirrorStrId
                                               : _pMap[_nLBSelPos].eStrId),
                                             m_pMirrorPagesCB->IsChecked() );
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
                            SvxSwFramePosString::StringId eStrId1 = m_pMirrorPagesCB->IsChecked() ?
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
            && (&_rLB != m_pVertRelationLB || m_bAllowVertPositioning);
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
    if(m_pAnchorAtParaRB->IsChecked())
    {
        nRet = FLY_AT_PARA;
    }
    else if(m_pAnchorAtCharRB->IsChecked())
    {
        nRet = FLY_AT_CHAR;
    }
    else if(m_pAnchorAsCharRB->IsChecked())
    {
        nRet = FLY_AS_CHAR;
    }
    else if(m_pAnchorAtFrameRB->IsChecked())
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
    m_aWidthED.LockAutoCalculation(sal_True);
    m_aHeightED.LockAutoCalculation(sal_True);
    RangeModifyHdl(&m_aWidthED);  // set all maximum values initially
    m_aHeightED.LockAutoCalculation(sal_False);
    m_aWidthED.LockAutoCalculation(sal_False);
    m_pFollowTextFlowCB->SaveValue();
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
    if (pBtn == m_pRelWidthCB)
    {
        m_aWidthED.ShowPercent(pBtn->IsChecked());
        if(pBtn->IsChecked())
            m_aWidthED.get()->SetMax(MAX_PERCENT_WIDTH);
    }
    else // pBtn == m_pRelHeightCB
    {
        m_aHeightED.ShowPercent(pBtn->IsChecked());
        if(pBtn->IsChecked())
            m_aHeightED.get()->SetMax(MAX_PERCENT_HEIGHT);
    }

    if (pBtn)   // only when Handler was called by change of the controller
        RangeModifyHdl(m_aWidthED.get());  // correct the values again

    if (pBtn == m_pRelWidthCB)
        ModifyHdl(m_aWidthED.get());
    else // pBtn == m_pRelHeightCB
        ModifyHdl(m_aHeightED.get());

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
    aVal.bAutoHeight = m_pAutoHeightCB->IsChecked();
    aVal.bAutoWidth = m_pAutoWidthCB->IsChecked();
    aVal.bMirror = m_pMirrorPagesCB->IsChecked();
    aVal.bFollowTextFlow = m_pFollowTextFlowCB->IsChecked();

    if ( pHMap )
    {
        // alignment horizonal
        sal_uInt16 nMapPos = GetMapPos(pHMap, *m_pHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB);
        short nRel = GetRelation(pHMap, *m_pHoriRelationLB);

        aVal.nHoriOrient = nAlign;
        aVal.nHRelOrient = nRel;
    }
    else
        aVal.nHoriOrient = text::HoriOrientation::NONE;

    if ( pVMap )
    {
        // alignment vertical
        sal_uInt16 nMapPos = GetMapPos(pVMap, *m_pVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB);
        short nRel = GetRelation(pVMap, *m_pVertRelationLB);

        aVal.nVertOrient = nAlign;
        aVal.nVRelOrient = nRel;
    }
    else
        aVal.nVertOrient = text::VertOrientation::NONE;

    const long nAtHorzPosVal = static_cast< long >(
                    m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)) );
    const long nAtVertPosVal = static_cast< long >(
                    m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)) );

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, mpToCharCntntPos, true);   // one time, to get reference values for percental values

    // set reference values for percental values (100%) ...
    m_aWidthED.SetRefValue(aVal.aPercentSize.Width());
    m_aHeightED.SetRefValue(aVal.aPercentSize.Height());

    // ... and correctly convert width and height with it
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED. DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
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
    m_aHeightED.SetMin(m_aHeightED.NormalizePercent(aVal.nMinHeight), FUNIT_TWIP);
    m_aWidthED. SetMin(m_aWidthED.NormalizePercent(aVal.nMinWidth), FUNIT_TWIP);

    SwTwips nMaxWidth(aVal.nMaxWidth);
    SwTwips nMaxHeight(aVal.nMaxHeight);

    if (aVal.bAutoHeight && (sDlgType == "PictureDialog" || sDlgType == "ObjectDialog"))
    {
        SwTwips nTmp = std::min(nWidth * nMaxHeight / std::max(nHeight, 1L), nMaxHeight);
        m_aWidthED.SetMax(m_aWidthED.NormalizePercent(nTmp), FUNIT_TWIP);

        nTmp = std::min(nHeight * nMaxWidth / std::max(nWidth, 1L), nMaxWidth);
        m_aHeightED.SetMax(m_aWidthED.NormalizePercent(nTmp), FUNIT_TWIP);
    }
    else
    {
        SwTwips nTmp = static_cast< SwTwips >(m_aHeightED.NormalizePercent(nMaxHeight));
        m_aHeightED.SetMax(nTmp, FUNIT_TWIP);

        nTmp = static_cast< SwTwips >(m_aWidthED.NormalizePercent(nMaxWidth));
        m_aWidthED.SetMax(nTmp, FUNIT_TWIP);
    }

    m_pAtHorzPosED->SetMin(m_pAtHorzPosED->Normalize(aVal.nMinHPos), FUNIT_TWIP);
    m_pAtHorzPosED->SetMax(m_pAtHorzPosED->Normalize(aVal.nMaxHPos), FUNIT_TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        m_pAtHorzPosED->SetValue(m_pAtHorzPosED->Normalize(aVal.nHPos), FUNIT_TWIP);

    const SwTwips nUpperOffset = (aVal.nAnchorType == FLY_AS_CHAR)
        ? nUpperBorder : 0;
    const SwTwips nLowerOffset = (aVal.nAnchorType == FLY_AS_CHAR)
        ? nLowerBorder : 0;

    m_pAtVertPosED->SetMin(m_pAtVertPosED->Normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset), FUNIT_TWIP);
    m_pAtVertPosED->SetMax(m_pAtVertPosED->Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_pAtVertPosED->SetValue(m_pAtVertPosED->Normalize(aVal.nVPos), FUNIT_TWIP);

    return 0;
}

IMPL_LINK_NOARG(SwFrmPage, AnchorTypeHdl)
{
    m_pMirrorPagesCB->Enable(!m_pAnchorAsCharRB->IsChecked());

    // i#18732 - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    // i#22305 - enable check box 'Follow text
    // flow' also for anchor type to-frame.
    m_pFollowTextFlowCB->Enable( m_pAnchorAtParaRB->IsChecked() ||
                              m_pAnchorAtCharRB->IsChecked() ||
                              m_pAnchorAtFrameRB->IsChecked() );

    RndStdIds eId = GetAnchor();

    InitPos( eId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl(0);

    if(bHtmlMode)
    {
        PosHdl(m_pHorizontalDLB);
        PosHdl(m_pVerticalDLB);
    }

    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && FLY_AS_CHAR == eId) );

    return 0;
}

IMPL_LINK( SwFrmPage, PosHdl, ListBox *, pLB )
{
    bool bHori = pLB == m_pHorizontalDLB;
    ListBox *pRelLB = bHori ? m_pHoriRelationLB : m_pVertRelationLB;
    FixedText *pRelFT = bHori ? m_pHoriRelationFT : m_pVertRelationFT;
    FrmMap *pMap = bHori ? pHMap : pVMap;

    sal_uInt16 nMapPos = GetMapPos(pMap, *pLB);
    short nAlign = GetAlignment(pMap, nMapPos, *pLB, *pRelLB);

    if (bHori)
    {
        sal_Bool bEnable = text::HoriOrientation::NONE == nAlign;
        m_pAtHorzPosED->Enable( bEnable );
        m_pAtHorzPosFT->Enable( bEnable );
    }
    else
    {
        sal_Bool bEnable = text::VertOrientation::NONE == nAlign && m_bAllowVertPositioning;
        m_pAtVertPosED->Enable( bEnable );
        m_pAtVertPosFT->Enable( bEnable );
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
        bool bSet = false;
        if(bHori)
        {
            // right is allowed only above - from the left only above
            // from the left at character -> below
            if((text::HoriOrientation::LEFT == nAlign || text::HoriOrientation::RIGHT == nAlign) &&
                    0 == m_pVerticalDLB->GetSelectEntryPos())
            {
                if(text::RelOrientation::FRAME == nRel)
                    m_pVerticalDLB->SelectEntryPos(1);
                else
                    m_pVerticalDLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(text::HoriOrientation::LEFT == nAlign && 1 == m_pVerticalDLB->GetSelectEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(text::HoriOrientation::NONE == nAlign && 1 == m_pVerticalDLB->GetSelectEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(0);
                bSet = true;
            }
            if(bSet)
                PosHdl(m_pVerticalDLB);
        }
        else
        {
            if(text::VertOrientation::TOP == nAlign)
            {
                if(1 == m_pHorizontalDLB->GetSelectEntryPos())
                {
                    m_pHorizontalDLB->SelectEntryPos(0);
                    bSet = true;
                }
                m_pHoriRelationLB->SelectEntryPos(1);
            }
            else if(text::VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == m_pHorizontalDLB->GetSelectEntryPos())
                {
                    m_pHorizontalDLB->SelectEntryPos(0);
                    bSet = true;
                }
                m_pHoriRelationLB->SelectEntryPos(0) ;
            }
            if(bSet)
                PosHdl(m_pHorizontalDLB);
        }

    }
    return 0;
}

/*--------------------------------------------------------------------
    Description:    horizontal Pos
 --------------------------------------------------------------------*/
IMPL_LINK( SwFrmPage, RelHdl, ListBox *, pLB )
{
    bool bHori = pLB == m_pHoriRelationLB;

    UpdateExample();

    if (bHori)
        bAtHorzPosModified = sal_True;
    else
        bAtVertPosModified = sal_True;

    if (bHtmlMode && (FLY_AT_CHAR == GetAnchor()))
    {
        if(bHori)
        {
            sal_uInt16 nRel = GetRelation(pHMap, *m_pHoriRelationLB);
            if(text::RelOrientation::PRINT_AREA == nRel && 0 == m_pVerticalDLB->GetSelectEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(1);
            }
            else if(text::RelOrientation::CHAR == nRel && 1 == m_pVerticalDLB->GetSelectEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(0);
            }
        }
    }
    if (pLB)    // Only when Handler was called by changing of the controller
        RangeModifyHdl(0);

    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwFrmPage, RealSizeHdl)
{
    m_aWidthED.SetUserValue( m_aWidthED. NormalizePercent(aGrfSize.Width() ), FUNIT_TWIP);
    m_aHeightED.SetUserValue(m_aHeightED.NormalizePercent(aGrfSize.Height()), FUNIT_TWIP);
    fWidthHeightRatio = aGrfSize.Height() ? double(aGrfSize.Width()) / double(aGrfSize.Height()) : 1.0;
    UpdateExample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwFrmPage, RealSizeHdl)

IMPL_LINK_NOARG(SwFrmPage, AutoWidthClickHdl)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoWidthCB->IsChecked(), *m_pWidthFT, *m_pWidthAutoFT );
    return 0;
}

IMPL_LINK_NOARG(SwFrmPage, AutoHeightClickHdl)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoHeightCB->IsChecked(), *m_pHeightFT, *m_pHeightAutoFT );
    return 0;
}

IMPL_LINK( SwFrmPage, ModifyHdl, Edit *, pEdit )
{
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
    if ( m_pFixedRatioCB->IsChecked() )
    {
        if (pEdit == m_aWidthED.get())
        {
            nHeight = SwTwips((double)nWidth / fWidthHeightRatio);
            m_aHeightED.SetPrcntValue(m_aHeightED.NormalizePercent(nHeight), FUNIT_TWIP);
        }
        else if (pEdit == m_aHeightED.get())
        {
            nWidth = SwTwips((double)nHeight * fWidthHeightRatio);
            m_aWidthED.SetPrcntValue(m_aWidthED.NormalizePercent(nWidth), FUNIT_TWIP);
        }
    }
    fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
    return 0;
}

void SwFrmPage::UpdateExample()
{
    sal_uInt16 nPos = m_pHorizontalDLB->GetSelectEntryPos();
    if ( pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(pHMap, *m_pHorizontalDLB);
        short nAlign = GetAlignment(pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB);
        short nRel = GetRelation(pHMap, *m_pHoriRelationLB);

        m_pExampleWN->SetHAlign(nAlign);
        m_pExampleWN->SetHoriRel(nRel);
    }

    nPos = m_pVerticalDLB->GetSelectEntryPos();
    if ( pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(pVMap, *m_pVerticalDLB);
        short nAlign = GetAlignment(pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB);
        short nRel = GetRelation(pVMap, *m_pVertRelationLB);

        m_pExampleWN->SetVAlign(nAlign);
        m_pExampleWN->SetVertRel(nRel);
    }

    // size
    long nXPos = static_cast< long >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)));
    long nYPos = static_cast< long >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)));
    m_pExampleWN->SetRelPos(Point(nXPos, nYPos));

    m_pExampleWN->SetAnchor( static_cast< sal_uInt16 >(GetAnchor()) );
    m_pExampleWN->Invalidate();
}

void SwFrmPage::Init(const SfxItemSet& rSet, sal_Bool bReset)
{
    if(!bFormat)
    {
        SwWrtShell* pSh = getFrmDlgParentShell();

        // size
        const sal_Bool bSizeFixed = pSh->IsSelObjProtected( FLYPROTECT_FIXED );

        m_aWidthED .Enable( !bSizeFixed );
        m_aHeightED.Enable( !bSizeFixed );

        // size controls for math OLE objects
        if ( sDlgType == "ObjectDialog" && ! bNew )
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
                    m_pWidthFT->Disable();
                    m_aWidthED.Disable();
                    m_pRelWidthCB->Disable();
                    m_pHeightFT->Disable();
                    m_aHeightED.Disable();
                    m_pRelHeightCB->Disable();
                    m_pFixedRatioCB->Disable();
                    m_pRealSizeBT->Disable();
                    break;
                }
            }

            // TODO/LATER: get correct aspect
            if(0 != (pSh->GetOLEObject()->getStatus( embed::Aspects::MSOLE_CONTENT ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
                m_pRealSizeBT->Disable();
        }
    }

    const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
    sal_Int64 nWidth  = m_aWidthED.NormalizePercent(rSize.GetWidth());
    sal_Int64 nHeight = m_aHeightED.NormalizePercent(rSize.GetHeight());

    if (nWidth != m_aWidthED.GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // value was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            m_aWidthED.SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            m_aWidthED.SetPrcntValue(nWidth, FUNIT_TWIP);
    }

    if (nHeight != m_aHeightED.GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // values was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            m_aHeightED.SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            m_aHeightED.SetPrcntValue(nHeight, FUNIT_TWIP);
    }

    if (!IsInGraficMode())
    {
        SwFrmSize eSize = rSize.GetHeightSizeType();
        sal_Bool bCheck = eSize != ATT_FIX_SIZE;
        m_pAutoHeightCB->Check( bCheck );
        HandleAutoCB( bCheck, *m_pHeightFT, *m_pHeightAutoFT );
        if( eSize == ATT_VAR_SIZE )
            m_aHeightED.SetValue( m_aHeightED.GetMin(), FUNIT_NONE );

        eSize = rSize.GetWidthSizeType();
        bCheck = eSize != ATT_FIX_SIZE;
        m_pAutoWidthCB->Check( bCheck );
        HandleAutoCB( bCheck, *m_pWidthFT, *m_pWidthAutoFT );
        if( eSize == ATT_VAR_SIZE )
            m_aWidthED.SetValue( m_aWidthED.GetMin(), FUNIT_NONE );

        if ( !bFormat )
        {
            SwWrtShell* pSh = getFrmDlgParentShell();
            const SwFrmFmt* pFmt = pSh->GetFlyFrmFmt();
            if( pFmt && pFmt->GetChain().GetNext() )
                m_pAutoHeightCB->Enable( sal_False );
        }
    }
    else
        m_pAutoHeightCB->Hide();

    // organise circulation-gap for character bound frames
    const SvxULSpaceItem &rUL = (const SvxULSpaceItem &)rSet.Get(RES_UL_SPACE);
    nUpperBorder = rUL.GetUpper();
    nLowerBorder = rUL.GetLower();

    if(SFX_ITEM_SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        m_pFixedRatioCB->Check(((const SfxBoolItem&)rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        m_pFixedRatioCB->SaveValue();
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

        m_pMirrorPagesCB->Check(rHori.IsPosToggle());
        m_pMirrorPagesCB->SaveValue();

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
    m_pExampleWN->SetWrap ( static_cast< sal_uInt16 >(rSurround.GetSurround()) );

    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
    {
        const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)rSet.Get(RES_OPAQUE);
        m_pExampleWN->SetTransparent(!rOpaque.GetValue());
    }

    // switch to percent if applicable
    RangeModifyHdl(&m_aWidthED);  // set reference values (for 100%)

    if (rSize.GetWidthPercent() == 0xff || rSize.GetHeightPercent() == 0xff)
        m_pFixedRatioCB->Check(sal_True);
    if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != 0xff &&
        !m_pRelWidthCB->IsChecked())
    {
        m_pRelWidthCB->Check(sal_True);
        RelSizeClickHdl(m_pRelWidthCB);
        m_aWidthED.SetPrcntValue(rSize.GetWidthPercent(), FUNIT_CUSTOM);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != 0xff &&
        !m_pRelHeightCB->IsChecked())
    {
        m_pRelHeightCB->Check(sal_True);
        RelSizeClickHdl(m_pRelHeightCB);
        m_aHeightED.SetPrcntValue(rSize.GetHeightPercent(), FUNIT_CUSTOM);
    }
    m_pRelWidthCB->SaveValue();
    m_pRelHeightCB->SaveValue();
}

sal_uInt16* SwFrmPage::GetRanges()
{
    return aPageRg;
}

void SwFrmPage::SetFormatUsed(sal_Bool bFmt)
{
    bFormat = bFmt;
    if(bFormat)
    {
        m_pAnchorFrame->Hide();
    }
}


void SwFrmPage::EnableVerticalPositioning( bool bEnable )
{
    m_bAllowVertPositioning = bEnable;
    m_pVerticalFT->Enable( bEnable );
    m_pVerticalDLB->Enable( bEnable );
    m_pAtVertPosFT->Enable( bEnable );
    m_pAtVertPosED->Enable( bEnable );
    m_pVertRelationFT->Enable( bEnable );
    m_pVertRelationLB->Enable( bEnable );
}


SwGrfExtPage::SwGrfExtPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "PicturePage",
        "modules/swriter/ui/picturepage.ui", rSet)
    , pGrfDlg(0)
{
    get(m_pMirror, "flipframe");
    get(m_pMirrorVertBox, "vert");
    get(m_pMirrorHorzBox, "hori");
    get(m_pAllPagesRB, "allpages");
    get(m_pLeftPagesRB, "leftpages");
    get(m_pRightPagesRB, "rightpages");
    get(m_pConnectED, "entry");
    get(m_pBrowseBT, "browse");
    get(m_pBmpWin, "preview");
    m_pBmpWin->SetBitmapEx(get<FixedImage>("fallback")->GetImage().GetBitmapEx());

    SetExchangeSupport();
    m_pMirrorHorzBox->SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    m_pMirrorVertBox->SetClickHdl( LINK(this, SwGrfExtPage, MirrorHdl));
    m_pBrowseBT->SetClickHdl    ( LINK(this, SwGrfExtPage, BrowseHdl));
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
        m_pBrowseBT->Enable();
        m_pConnectED->SetReadOnly(sal_False);
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
        case RES_MIRROR_GRAPH_VERT: m_pMirrorHorzBox->Check(sal_True); break;
        case RES_MIRROR_GRAPH_HOR:  m_pMirrorVertBox->Check(sal_True); break;
        case RES_MIRROR_GRAPH_BOTH: m_pMirrorHorzBox->Check(sal_True);
                                    m_pMirrorVertBox->Check(sal_True);
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
                m_pLeftPagesRB->Check();
                m_pMirrorHorzBox->Check(sal_True);
                break;
            case 2: // mirror on all pages
                m_pAllPagesRB->Check();
                break;
            case 3: // mirror on right / odd pages
                m_pRightPagesRB->Check();
                break;
            default:
                m_pAllPagesRB->Check();
                break;
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
    {
        const SvxBrushItem& rBrush = *(SvxBrushItem*)pItem;
        if( rBrush.GetGraphicLink() )
        {
            aGrfName = aNewGrfName = *rBrush.GetGraphicLink();
            m_pConnectED->SetText( aNewGrfName );
        }
        const Graphic* pGrf = rBrush.GetGraphic();
        if( pGrf )
            m_pBmpWin->SetGraphic( *pGrf );
    }

    m_pAllPagesRB->Enable(bEnableMirrorRB);
    m_pLeftPagesRB->Enable(bEnableMirrorRB);
    m_pRightPagesRB->Enable(bEnableMirrorRB);
    m_pMirror->Enable(bEnable);

    m_pAllPagesRB->SaveValue();
    m_pLeftPagesRB->SaveValue();
    m_pRightPagesRB->SaveValue();
    m_pMirrorHorzBox->SaveValue();
    m_pMirrorVertBox->SaveValue();

    m_pBmpWin->MirrorHorz( m_pMirrorVertBox->IsChecked() );
    m_pBmpWin->MirrorVert( m_pMirrorHorzBox->IsChecked() );
    m_pBmpWin->Invalidate();
}

sal_Bool SwGrfExtPage::FillItemSet( SfxItemSet &rSet )
{
    sal_Bool bModified = sal_False;
    if ( m_pMirrorHorzBox->GetSavedValue() != m_pMirrorHorzBox->IsChecked() ||
         m_pMirrorVertBox->GetSavedValue() != m_pMirrorVertBox->IsChecked() ||
         m_pAllPagesRB->GetSavedValue() != m_pAllPagesRB->IsChecked() ||
         m_pLeftPagesRB->GetSavedValue() != m_pLeftPagesRB->IsChecked() ||
         m_pRightPagesRB->GetSavedValue() != m_pRightPagesRB->IsChecked())
    {
        bModified = sal_True;

        bool bHori = false;

        if (m_pMirrorHorzBox->IsChecked() &&
                !m_pLeftPagesRB->IsChecked())
            bHori = true;

        MirrorGraph eMirror;
        eMirror = m_pMirrorVertBox->IsChecked() && bHori ?
                    RES_MIRROR_GRAPH_BOTH : bHori ?
                    RES_MIRROR_GRAPH_VERT : m_pMirrorVertBox->IsChecked() ?
                    RES_MIRROR_GRAPH_HOR  : RES_MIRROR_GRAPH_DONT;

        sal_Bool bMirror = !m_pAllPagesRB->IsChecked();
        SwMirrorGrf aMirror( eMirror );
        aMirror.SetGrfToggle(bMirror );
        rSet.Put( aMirror );
    }

    if( aGrfName != aNewGrfName || m_pConnectED->IsModified() )
    {
        bModified = sal_True;
        aGrfName = m_pConnectED->GetText();
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
        pGrfDlg->SetTitle(get<VclFrame>("linkframe")->get_label());
    }
    pGrfDlg->SetDisplayDirectory( m_pConnectED->GetText() );
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
        m_pConnectED->SetModifyFlag();
        m_pConnectED->SetText( aNewGrfName );
        //reset mirrors because maybe a Bitmap was swapped with
        //another type of graphic that cannot be mirrored.
        m_pMirrorVertBox->Check(sal_False);
        m_pMirrorHorzBox->Check(sal_False);
        m_pAllPagesRB->Enable(sal_False);
        m_pLeftPagesRB->Enable(sal_False);
        m_pRightPagesRB->Enable(sal_False);
        m_pBmpWin->MirrorHorz(sal_False);
        m_pBmpWin->MirrorVert(sal_False);

        Graphic aGraphic;
        GraphicFilter::LoadGraphic( pGrfDlg->GetPath(), aEmptyStr, aGraphic );
        m_pBmpWin->SetGraphic(aGraphic);

        sal_Bool bEnable = GRAPHIC_BITMAP      == aGraphic.GetType() ||
                            GRAPHIC_GDIMETAFILE == aGraphic.GetType();
        m_pMirrorVertBox->Enable(bEnable);
        m_pMirrorHorzBox->Enable(bEnable);
        m_pAllPagesRB->Enable(bEnable);
        m_pLeftPagesRB->Enable(bEnable);
        m_pRightPagesRB->Enable(bEnable);
    }
    return 0;
}

IMPL_LINK_NOARG(SwGrfExtPage, MirrorHdl)
{
    sal_Bool bEnable = m_pMirrorHorzBox->IsChecked();

    m_pBmpWin->MirrorHorz( m_pMirrorVertBox->IsChecked() );
    m_pBmpWin->MirrorVert( bEnable );

    m_pAllPagesRB->Enable(bEnable);
    m_pLeftPagesRB->Enable(bEnable);
    m_pRightPagesRB->Enable(bEnable);

    if (!m_pAllPagesRB->IsChecked() && !m_pLeftPagesRB->IsChecked() && !m_pRightPagesRB->IsChecked())
        m_pAllPagesRB->Check();

    return 0;
}

/*--------------------------------------------------------------------
    Description: example window
 --------------------------------------------------------------------*/
BmpWindow::BmpWindow(Window* pPar, WinBits nStyle)
    : Window(pPar, nStyle)
    , bHorz(false)
    , bVert(false)
    , bGraphic(false)
    , bLeftAlign(false)
{
    // #i119307# use background, the graphic might have transparency
    SetBackground(Wallpaper(Color(COL_WHITE)));
}

Size BmpWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(127 , 66), MapMode(MAP_APPFONT));
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeBmpWindow(Window *pParent, VclBuilder::stringmap &)
{
    return new BmpWindow(pParent, 0);
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

    // #i119307# clear window background, the graphic might have transparency
    Erase();

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

void BmpWindow::SetBitmapEx(const BitmapEx& rBmp)
{
    aBmp = rBmp;
    Invalidate();
}

/***************************************************************************
    Description:    set URL and ImageMap at frames
***************************************************************************/
SwFrmURLPage::SwFrmURLPage( Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent, "FrmURLPage" , "modules/swriter/ui/frmurlpage.ui", rSet)
{
    get(pURLED,"url");
    get(pSearchPB,"search");
    get(pNameED,"name");
    get(pFrameCB,"frame");

    get(pServerCB,"server");
    get(pClientCB,"client");

    pSearchPB->SetClickHdl(LINK(this, SwFrmURLPage, InsertFileHdl));
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
                pFrameCB->InsertEntry( pList->at( i ) );
            }
        }
        delete pList;
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( RES_URL, sal_True, &pItem ) )
    {
        const SwFmtURL* pFmtURL = (const SwFmtURL*)pItem;
        pURLED->SetText( INetURLObject::decode( pFmtURL->GetURL(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        pNameED->SetText( pFmtURL->GetName());

        pClientCB->Enable( pFmtURL->GetMap() != 0 );
        pClientCB->Check ( pFmtURL->GetMap() != 0 );
        pServerCB->Check ( pFmtURL->IsServerMap() );

        pFrameCB->SetText(pFmtURL->GetTargetFrameName());
        pFrameCB->SaveValue();
    }
    else
        pClientCB->Enable( sal_False );

    pServerCB->SaveValue();
    pClientCB->SaveValue();
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
        String sText = pURLED->GetText();

        if( pFmtURL->GetURL() != sText ||
            OUString(pFmtURL->GetName()) != pNameED->GetText() ||
            pServerCB->IsChecked() != pFmtURL->IsServerMap() )
        {
            pFmtURL->SetURL( sText, pServerCB->IsChecked() );
            pFmtURL->SetName( pNameED->GetText() );
            bModified = sal_True;
        }
    }

    if(!pClientCB->IsChecked() && pFmtURL->GetMap() != 0)
    {
        pFmtURL->SetMap(0);
        bModified = sal_True;
    }

    if(OUString(pFmtURL->GetTargetFrameName()) != pFrameCB->GetText())
    {
        pFmtURL->SetTargetFrameName(pFrameCB->GetText());
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
        String sTemp(pURLED->GetText());
        if(sTemp.Len())
            xFP->setDisplayDirectory(sTemp);
    }
    catch( const uno::Exception& rEx )
    {
        (void) rEx;
    }
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        pURLED->SetText( xFP->getFiles().getConstArray()[0] );
    }

    return 0;
}

SwFrmAddPage::SwFrmAddPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FrmAddPage" , "modules/swriter/ui/frmaddpage.ui", rSet)
    , pWrtSh(0)
    , bHtmlMode(sal_False)
    , bFormat(sal_False)
    , bNew(sal_False)
{
    get(pNameFrame, "nameframe");
    get(pNameFT,"name_label");
    get(pNameED,"name");
    get(pAltNameFT,"altname_label");
    get(pAltNameED,"altname");
    get(pPrevFT,"prev_label");
    get(pPrevLB,"prev");
    get(pNextFT,"next_label");
    get(pNextLB,"next");

    get(pProtectFrame,"protect");
    get(pProtectContentCB,"protectcontent");
    get(pProtectFrameCB,"protectframe");
    get(pProtectSizeCB,"protectsize");

    get(pPropertiesFrame,"properties");
    get(pEditInReadonlyCB,"editinreadonly");
    get(pPrintFrameCB,"printframe");
    get(pTextFlowFT,"textflow_label");
    get(pTextFlowLB,"textflow");

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
    if (bHtmlMode)
    {
        pProtectFrame->Hide();
        pEditInReadonlyCB->Hide();
        pPrintFrameCB->Hide();
    }
    if (sDlgType == "PictureDialog" || sDlgType == "ObjectDialog")
    {
        pEditInReadonlyCB->Hide();
        if (bHtmlMode)
        {
            pPropertiesFrame->Hide();
        }
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_SET_FRM_ALT_NAME, sal_False, &pItem))
    {
        pAltNameED->SetText(((const SfxStringItem*)pItem)->GetValue());
        pAltNameED->SaveValue();
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
        {
            if (sDlgType == "PictureDialog")
                aTmpName1 = pWrtSh->GetUniqueGrfName();
            else if (sDlgType == "ObjectDialog")
                aTmpName1 = pWrtSh->GetUniqueOLEName();
            else
                aTmpName1 = pWrtSh->GetUniqueFrameName();
        }

        pNameED->SetText( aTmpName1 );
        pNameED->SaveValue();
    }
    else
    {
        pNameED->Enable( sal_False );
        pAltNameED->Enable(sal_False);
        pNameFT->Enable( sal_False );
        pAltNameFT->Enable(sal_False);
    }
    if (sDlgType == "FrameDialog" && pAltNameFT->IsVisible())
    {
        pAltNameFT->Hide();
        pAltNameED->Hide();
    }
    else
    {
        pNameED->SetModifyHdl(LINK(this, SwFrmAddPage, EditModifyHdl));
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
            lcl_InsertVectors(*pPrevLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(sPrevChain.Len())
            {
                if(LISTBOX_ENTRY_NOTFOUND == pPrevLB->GetEntryPos(sPrevChain))
                    pPrevLB->InsertEntry(sPrevChain, 1);
                pPrevLB->SelectEntry(sPrevChain);
            }
            else
                pPrevLB->SelectEntryPos(0);
            aPrevPageFrames.erase(aPrevPageFrames.begin(), aPrevPageFrames.end());
            aNextPageFrames.erase(aNextPageFrames.begin(), aNextPageFrames.end());
            aThisPageFrames.erase(aThisPageFrames.begin(), aThisPageFrames.end());
            aRemainFrames.erase(aRemainFrames.begin(), aRemainFrames.end());

            pWrtSh->GetConnectableFrmFmts(*pFmt, sPrevChain, sal_True,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            lcl_InsertVectors(*pNextLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(sNextChain.Len())
            {
                if(LISTBOX_ENTRY_NOTFOUND == pNextLB->GetEntryPos(sNextChain))
                    pNextLB->InsertEntry(sNextChain, 1);
                pNextLB->SelectEntry(sNextChain);
            }
            else
                pNextLB->SelectEntryPos(0);
            Link aLink(LINK(this, SwFrmAddPage, ChainModifyHdl));
            pPrevLB->SetSelectHdl(aLink);
            pNextLB->SetSelectHdl(aLink);
        }
    }
    // Pos Protected
    const SvxProtectItem& rProt = (const SvxProtectItem& )rSet.Get(RES_PROTECT);
    pProtectFrameCB->Check(rProt.IsPosProtected());
    pProtectContentCB->Check(rProt.IsCntntProtected());
    pProtectSizeCB->Check(rProt.IsSizeProtected());

    const SwFmtEditInReadonly& rEdit = (const SwFmtEditInReadonly& )rSet.Get(RES_EDIT_IN_READONLY);
    pEditInReadonlyCB->Check(rEdit.GetValue());          pEditInReadonlyCB->SaveValue();

    // print
    const SvxPrintItem& rPrt = (const SvxPrintItem&)rSet.Get(RES_PRINT);
    pPrintFrameCB->Check(rPrt.GetValue());               pPrintFrameCB->SaveValue();

    // textflow
    SfxItemState eState;
    if( (!bHtmlMode || (0 != (nHtmlMode&HTMLMODE_SOME_STYLES)))
            && sDlgType != "PictureDialog" && sDlgType != "ObjectDialog" &&
        SFX_ITEM_UNKNOWN != ( eState = rSet.GetItemState(
                                        RES_FRAMEDIR, sal_True )) )
    {
        pTextFlowFT->Show();
        pTextFlowLB->Show();

        //vertical text flow is not possible in HTML
        if(bHtmlMode)
        {
            sal_uLong nData = FRMDIR_VERT_TOP_RIGHT;
            pTextFlowLB->RemoveEntry(pTextFlowLB->GetEntryPos((void*)nData));
        }
        sal_uInt16 nPos, nVal = ((SvxFrameDirectionItem&)rSet.Get(RES_FRAMEDIR)).GetValue();
        for( nPos = pTextFlowLB->GetEntryCount(); nPos; )
            if( (sal_uInt16)(sal_IntPtr)pTextFlowLB->GetEntryData( --nPos ) == nVal )
                break;
        pTextFlowLB->SelectEntryPos( nPos );
        pTextFlowLB->SaveValue();
    }
    else
    {
        pTextFlowFT->Hide();
        pTextFlowLB->Hide();
    }
}

sal_Bool SwFrmAddPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bRet = sal_False;
    if (pNameED->GetText() != pNameED->GetSavedValue())
        bRet |= 0 != rSet.Put(SfxStringItem(FN_SET_FRM_NAME, pNameED->GetText()));
    if (pAltNameED->GetText()  != pAltNameED->GetSavedValue())
        bRet |= 0 != rSet.Put(SfxStringItem(FN_SET_FRM_ALT_NAME, pAltNameED->GetText()));

    const SfxPoolItem* pOldItem;
    SvxProtectItem aProt ( (const SvxProtectItem& )GetItemSet().Get(RES_PROTECT) );
    aProt.SetCntntProtect( pProtectContentCB->IsChecked() );
    aProt.SetSizeProtect ( pProtectSizeCB->IsChecked() );
    aProt.SetPosProtect  ( pProtectFrameCB->IsChecked() );
    if ( 0 == (pOldItem = GetOldItem(rSet, FN_SET_PROTECT)) ||
                aProt != *pOldItem )
        bRet |= 0 != rSet.Put( aProt);

    sal_Bool bChecked;
    if ( (bChecked = pEditInReadonlyCB->IsChecked()) != pEditInReadonlyCB->GetSavedValue() )
        bRet |= 0 != rSet.Put( SwFmtEditInReadonly( RES_EDIT_IN_READONLY, bChecked));

    if ( (bChecked = pPrintFrameCB->IsChecked()) != pPrintFrameCB->GetSavedValue() )
        bRet |= 0 != rSet.Put( SvxPrintItem( RES_PRINT, bChecked));

    // textflow
    if( pTextFlowLB->IsVisible() )
    {
        sal_uInt16 nPos = pTextFlowLB->GetSelectEntryPos();
        if( nPos != pTextFlowLB->GetSavedValue() )
        {
            nPos = (sal_uInt16)(sal_IntPtr)pTextFlowLB->GetEntryData( nPos );
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
            if(pPrevLB->GetSelectEntryPos())
                sCurrentPrevChain = pPrevLB->GetSelectEntry();
            if(pNextLB->GetSelectEntryPos())
                sCurrentNextChain = pNextLB->GetSelectEntry();
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
    sal_Bool bEnable = !pNameED->GetText().isEmpty();
    pAltNameED->Enable(bEnable);
    pAltNameFT->Enable(bEnable);

    return 0;
}

void SwFrmAddPage::SetFormatUsed(sal_Bool bFmt)
{
    bFormat = bFmt;
    if (bFormat)
    {
        pNameFrame->Hide();
    }
}

IMPL_LINK(SwFrmAddPage, ChainModifyHdl, ListBox*, pBox)
{
    String sCurrentPrevChain, sCurrentNextChain;
    if(pPrevLB->GetSelectEntryPos())
        sCurrentPrevChain = pPrevLB->GetSelectEntry();
    if(pNextLB->GetSelectEntryPos())
        sCurrentNextChain = pNextLB->GetSelectEntry();
    SwFrmFmt* pFmt = pWrtSh->GetFlyFrmFmt();
    if (pFmt)
    {
        bool bNextBox = pNextLB == pBox;
        ListBox& rChangeLB = bNextBox ? *pPrevLB : *pNextLB;
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
