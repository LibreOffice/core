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
#include <editeng/adjustitem.hxx>
#include <svx/sdtaitm.hxx>
#include <sal/macros.h>

#include <frmui.hrc>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <vcl/graphicfilter.hxx>
#include <vcl/builderfactory.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::sfx2;

#define SwFPos SvxSwFramePosString

struct FrameMap
{
    SvxSwFramePosString::StringId eStrId;
    SvxSwFramePosString::StringId eMirrorStrId;
    sal_Int16  nAlign;
    sal_uLong  nLBRelations;
};

struct RelationMap
{
    SvxSwFramePosString::StringId eStrId;
    SvxSwFramePosString::StringId eMirrorStrId;
    sal_uLong  nLBRelation;
    sal_Int16  nRelation;
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
#define LB_VERT_PRTAREA         0x00000008L // vertical text region of the paragraph + indentions
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

// site anchored
#define HORI_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_PG_LEFT| \
                        LB_REL_PG_RIGHT)

static FrameMap aHPageMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrameMap aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

#define VERT_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrameMap aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrameMap aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB_REL_PG_FRAME}
};

// frame anchored
#define HORI_FRAME_REL  (LB_FLY_REL_PG_FRAME|LB_FLY_REL_PG_PRTAREA| \
                        LB_FLY_REL_PG_LEFT|LB_FLY_REL_PG_RIGHT)

static FrameMap aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrameMap aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB_FLY_REL_PG_FRAME},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_FLY_REL_PG_FRAME}
};

// own vertical alignment map for objects anchored to frame
#define VERT_FRAME_REL   (LB_FLY_VERT_FRAME|LB_FLY_VERT_PRTAREA)

static FrameMap aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrameMap aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB_FLY_VERT_FRAME},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB_FLY_VERT_FRAME}
};

// paragraph anchored
#define HORI_PARA_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT)

static FrameMap aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB_FRAME|LB_PRTAREA)

static FrameMap aHParaHtmlMap[] =
{
    {SwFPos::LEFT,  SwFPos::LEFT,   text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT, SwFPos::RIGHT,  text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

static FrameMap aHParaHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

// allow vertical alignment at page areas
#define VERT_PARA_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrameMap aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PARA_REL}
};

static FrameMap aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB_VERT_PRTAREA}
};

// anchored relative to the character
#define HORI_CHAR_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT|LB_REL_CHAR)

static FrameMap aHCharMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_CHAR_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_CHAR_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_CHAR_REL}
};

#define HTML_HORI_CHAR_REL  (LB_FRAME|LB_PRTAREA|LB_REL_CHAR)

static FrameMap aHCharHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::LEFT,           text::HoriOrientation::LEFT,      HTML_HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::RIGHT,          text::HoriOrientation::RIGHT,     HTML_HORI_CHAR_REL}
};

static FrameMap aHCharHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB_PRTAREA|LB_REL_CHAR},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     LB_PRTAREA},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

// allow vertical alignment at page areas
#define VERT_CHAR_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrameMap aVCharMap[] =
{
    // introduce mappings for new vertical alignment at top of line <LB_VERT_LINE>
    // and correct mapping for vertical alignment at character for position <FROM_BOTTOM>
    // Note: Because of these adjustments the map becomes ambigous in its values
    //       <eStrId>/<eMirrorStrId> and <nAlign>. These ambiguities are considered
    //       in the methods <SwFramePage::FillRelLB(..)>, <SwFramePage::GetAlignment(..)>
    //       and <SwFramePage::FillPosLB(..)>
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

static FrameMap aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};

static FrameMap aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,           LB_REL_CHAR},
    {SwFPos::BELOW,             SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};

// anchored as character
static FrameMap aVAsCharMap[] =
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

static FrameMap aVAsCharHtmlMap[] =
{
    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::TOP,           LB_REL_BASE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        LB_REL_BASE},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::CHAR_TOP,      LB_REL_CHAR},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB_REL_ROW},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB_REL_ROW},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB_REL_ROW}
};

const sal_uInt16 SwFramePage::aPageRg[] = {
    RES_FRM_SIZE, RES_FRM_SIZE,
    RES_VERT_ORIENT, RES_ANCHOR,
    RES_COL, RES_COL,
    RES_FOLLOW_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW,
    0
};
const sal_uInt16 SwFrameAddPage::aAddPgRg[] = {
    RES_PROTECT,            RES_PROTECT,
    RES_PRINT,              RES_PRINT,
    FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
    FN_SET_FRM_ALT_NAME,    FN_SET_FRM_ALT_NAME,
    0
};

static size_t lcl_GetFrameMapCount( const FrameMap* pMap)
{
    if ( pMap )
    {
        if( pMap == aVParaHtmlMap)
            return SAL_N_ELEMENTS(aVParaHtmlMap);
        if( pMap == aVAsCharHtmlMap)
            return SAL_N_ELEMENTS(aVAsCharHtmlMap);
        if( pMap == aHParaHtmlMap)
            return SAL_N_ELEMENTS(aHParaHtmlMap);
        if( pMap == aHParaHtmlAbsMap)
            return SAL_N_ELEMENTS(aHParaHtmlAbsMap);
        if ( pMap == aVPageMap )
            return SAL_N_ELEMENTS(aVPageMap);
        if ( pMap == aVPageHtmlMap )
            return SAL_N_ELEMENTS(aVPageHtmlMap);
        if ( pMap == aVAsCharMap )
            return SAL_N_ELEMENTS(aVAsCharMap);
        if ( pMap == aVParaMap )
            return SAL_N_ELEMENTS(aVParaMap);
        if ( pMap == aHParaMap )
            return SAL_N_ELEMENTS(aHParaMap);
        if ( pMap == aHFrameMap )
            return SAL_N_ELEMENTS(aHFrameMap);
        if ( pMap == aVFrameMap )
            return SAL_N_ELEMENTS(aVFrameMap);
        if ( pMap == aHCharMap )
            return SAL_N_ELEMENTS(aHCharMap);
        if ( pMap == aHCharHtmlMap )
            return SAL_N_ELEMENTS(aHCharHtmlMap);
        if ( pMap == aHCharHtmlAbsMap )
            return SAL_N_ELEMENTS(aHCharHtmlAbsMap);
        if ( pMap == aVCharMap )
            return SAL_N_ELEMENTS(aVCharMap);
        if ( pMap == aVCharHtmlMap )
            return SAL_N_ELEMENTS(aVCharHtmlMap);
        if ( pMap == aVCharHtmlAbsMap )
            return SAL_N_ELEMENTS(aVCharHtmlAbsMap);
        if ( pMap == aHPageHtmlMap )
            return SAL_N_ELEMENTS(aHPageHtmlMap);
        if ( pMap == aHFlyHtmlMap )
            return SAL_N_ELEMENTS(aHFlyHtmlMap);
        if ( pMap == aVFlyHtmlMap )
            return SAL_N_ELEMENTS(aVFlyHtmlMap);
        return SAL_N_ELEMENTS(aHPageMap);
    }
    return 0;
}

static void lcl_InsertVectors(ListBox& rBox,
    const ::std::vector< OUString >& rPrev, const ::std::vector< OUString >& rThis,
    const ::std::vector< OUString >& rNext, const ::std::vector< OUString >& rRemain)
{
    ::std::vector< OUString >::const_iterator aIt;
    sal_Int32 nEntry = 0;
    for(aIt = rPrev.begin(); aIt != rPrev.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    for(aIt = rThis.begin(); aIt != rThis.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    for(aIt = rNext.begin(); aIt != rNext.end(); ++aIt)
        nEntry = rBox.InsertEntry(*aIt);
    rBox.SetSeparatorPos(nEntry);
    //now insert all strings sorted
    const sal_Int32 nStartPos = rBox.GetEntryCount();

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
static SvxSwFramePosString::StringId lcl_ChangeResIdToVerticalOrRTL(SvxSwFramePosString::StringId eStringId, bool bVertical, bool bVerticalL2R, bool bRTL)
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
        for(size_t nIndex = 0; nIndex < SAL_N_ELEMENTS(aHoriIds); ++nIndex)
        {
            if(aHoriIds[nIndex].eHori == eStringId)
            {
                eStringId = aHoriIds[nIndex].eVert;
                return eStringId;
            }
        }
        for(size_t nIndex = 0; nIndex < SAL_N_ELEMENTS(aVertIds); ++nIndex)
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
static sal_uLong lcl_GetLBRelationsForRelations( const sal_Int16 _nRel )
{
    sal_uLong nLBRelations = 0L;

    const size_t nRelMapSize = SAL_N_ELEMENTS(aRelationMap);
    for ( size_t nRelMapPos = 0; nRelMapPos < nRelMapSize; ++nRelMapPos )
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
static sal_uLong lcl_GetLBRelationsForStrID( const FrameMap* _pMap,
                                             const SvxSwFramePosString::StringId _eStrId,
                                             const bool _bUseMirrorStr )
{
    sal_uLong nLBRelations = 0L;

    size_t nRelMapSize = lcl_GetFrameMapCount( _pMap );
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

// standard frame TabPage
namespace
{
    void HandleAutoCB( bool _bChecked, FixedText& _rFT_man, FixedText& _rFT_auto, MetricField& _rPF_Edit)
    {
        _rFT_man.Show( !_bChecked );
        _rFT_auto.Show( _bChecked );
        OUString accName = _bChecked ? _rFT_auto.GetText() : _rFT_man.GetText();
        _rPF_Edit.SetAccessibleName(accName);
    }
}

SwFramePage::SwFramePage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FrameTypePage",
        "modules/swriter/ui/frmtypepage.ui", &rSet)
    ,
    m_bAtHorzPosModified( false ),
    m_bAtVertPosModified( false ),
    m_bFormat(false),
    m_bNew(true),
    m_bNoModifyHdl(true),
    m_bIsVerticalFrame(false),
    m_bIsVerticalL2R(false),
    m_bIsInRightToLeft(false),
    m_bHtmlMode(false),
    m_nHtmlMode(0),
    m_nUpperBorder(0),
    m_nLowerBorder(0),
    m_fWidthHeightRatio(1.0),
    mpToCharContentPos( nullptr ),

    m_nOldH(text::HoriOrientation::CENTER),
    m_nOldHRel(text::RelOrientation::FRAME),
    m_nOldV(text::VertOrientation::TOP),
    m_nOldVRel(text::RelOrientation::PRINT_AREA),
    m_pVMap( nullptr ),
    m_pHMap( nullptr ),
    m_bAllowVertPositioning( true ),
    m_bIsMathOLE( false ),
    m_bIsMathBaselineAlignment( true )
{
    get(m_pWidthFT, "widthft");
    get(m_pWidthAutoFT, "autowidthft");
    m_aWidthED.set(get<MetricField>("width"));
    get(m_pRelWidthCB, "relwidth");
    get(m_pRelWidthRelationLB, "relwidthrelation");
    get(m_pAutoWidthCB, "autowidth");

    get(m_pHeightFT, "heightft");
    get(m_pHeightAutoFT, "autoheightft");
    m_aHeightED.set(get<MetricField>("height"));
    get(m_pRelHeightCB, "relheight");
    get(m_pRelHeightRelationLB, "relheightrelation");
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

    setOptimalFrameWidth();
    setOptimalRelWidth();

    SetExchangeSupport();

    Link<Control&,void> aLk3 = LINK(this, SwFramePage, RangeModifyLoseFocusHdl);
    m_aWidthED.SetLoseFocusHdl( aLk3 );
    m_aHeightED.SetLoseFocusHdl( aLk3 );
    m_pAtHorzPosED->SetLoseFocusHdl( aLk3 );
    m_pAtVertPosED->SetLoseFocusHdl( aLk3 );
    m_pFollowTextFlowCB->SetClickHdl( LINK(this, SwFramePage, RangeModifyClickHdl) );

    Link<Edit&,void> aLk = LINK(this, SwFramePage, ModifyHdl);
    m_aWidthED.SetModifyHdl( aLk );
    m_aHeightED.SetModifyHdl( aLk );
    m_pAtHorzPosED->SetModifyHdl( aLk );
    m_pAtVertPosED->SetModifyHdl( aLk );

    Link<Button*,void> aLk2 = LINK(this, SwFramePage, AnchorTypeHdl);
    m_pAnchorAtPageRB->SetClickHdl( aLk2 );
    m_pAnchorAtParaRB->SetClickHdl( aLk2 );
    m_pAnchorAtCharRB->SetClickHdl( aLk2 );
    m_pAnchorAsCharRB->SetClickHdl( aLk2 );
    m_pAnchorAtFrameRB->SetClickHdl( aLk2 );

    m_pHorizontalDLB->SetSelectHdl(LINK(this, SwFramePage, PosHdl));
    m_pVerticalDLB->  SetSelectHdl(LINK(this, SwFramePage, PosHdl));

    m_pHoriRelationLB->SetSelectHdl(LINK(this, SwFramePage, RelHdl));
    m_pVertRelationLB->SetSelectHdl(LINK(this, SwFramePage, RelHdl));

    m_pMirrorPagesCB->SetClickHdl(LINK(this, SwFramePage, MirrorHdl));

    aLk2 = LINK(this, SwFramePage, RelSizeClickHdl);
    m_pRelWidthCB->SetClickHdl( aLk2 );
    m_pRelHeightCB->SetClickHdl( aLk2 );

    m_pAutoWidthCB->SetClickHdl( LINK( this, SwFramePage, AutoWidthClickHdl ) );
    m_pAutoHeightCB->SetClickHdl( LINK( this, SwFramePage, AutoHeightClickHdl ) );
}

SwFramePage::~SwFramePage()
{
    disposeOnce();
}

void SwFramePage::dispose()
{
    m_pWidthFT.clear();
    m_pWidthAutoFT.clear();
    m_pRelWidthCB.clear();
    m_pRelWidthRelationLB.clear();
    m_pAutoWidthCB.clear();
    m_pHeightFT.clear();
    m_pHeightAutoFT.clear();
    m_pRelHeightCB.clear();
    m_pRelHeightRelationLB.clear();
    m_pAutoHeightCB.clear();
    m_pFixedRatioCB.clear();
    m_pRealSizeBT.clear();
    m_pAnchorFrame.clear();
    m_pAnchorAtPageRB.clear();
    m_pAnchorAtParaRB.clear();
    m_pAnchorAtCharRB.clear();
    m_pAnchorAsCharRB.clear();
    m_pAnchorAtFrameRB.clear();
    m_pHorizontalFT.clear();
    m_pHorizontalDLB.clear();
    m_pAtHorzPosFT.clear();
    m_pAtHorzPosED.clear();
    m_pHoriRelationFT.clear();
    m_pHoriRelationLB.clear();
    m_pMirrorPagesCB.clear();
    m_pVerticalFT.clear();
    m_pVerticalDLB.clear();
    m_pAtVertPosFT.clear();
    m_pAtVertPosED.clear();
    m_pVertRelationFT.clear();
    m_pVertRelationLB.clear();
    m_pFollowTextFlowCB.clear();
    m_pExampleWN.clear();
    SfxTabPage::dispose();
}

namespace
{
    struct FrameMaps
    {
        FrameMap *pMap;
        size_t nCount;
    };
}

void SwFramePage::setOptimalFrameWidth()
{
    const FrameMaps aMaps[] = {
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
        m_pHorizontalDLB->InsertEntry(m_aFramePosString.GetString(*aI));
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

void SwFramePage::setOptimalRelWidth()
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
        m_pHoriRelationLB->InsertEntry(m_aFramePosString.GetString(*aI));
    }

    Size aBiggest(m_pHoriRelationLB->GetOptimalSize());
    m_pHoriRelationLB->set_width_request(aBiggest.Width());
    m_pVertRelationLB->set_width_request(aBiggest.Width());
    m_pRelWidthRelationLB->set_width_request(aBiggest.Width());
    m_pRelHeightRelationLB->set_width_request(aBiggest.Width());
    m_pHoriRelationLB->Clear();
}

VclPtr<SfxTabPage> SwFramePage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFramePage>::Create( pParent, *rSet );
}

void SwFramePage::EnableGraficMode()
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

SwWrtShell *SwFramePage::getFrameDlgParentShell()
{
    return static_cast<SwFrameDlg*>(GetParentDialog())->GetWrtShell();
}

void SwFramePage::Reset( const SfxItemSet *rSet )
{
    SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell() :
            getFrameDlgParentShell();

    m_nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
    m_bHtmlMode = (m_nHtmlMode & HTMLMODE_ON) != 0;

    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    m_aWidthED.SetMetric(aMetric);
    m_aHeightED.SetMetric(aMetric);
    SetMetric( *m_pAtHorzPosED, aMetric );
    SetMetric( *m_pAtVertPosED, aMetric );

    const SfxPoolItem* pItem = nullptr;
    const SwFormatAnchor& rAnchor = static_cast<const SwFormatAnchor&>(rSet->Get(RES_ANCHOR));

    if (SfxItemState::SET == rSet->GetItemState(FN_OLE_IS_MATH, false, &pItem))
        m_bIsMathOLE = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    if (SfxItemState::SET == rSet->GetItemState(FN_MATH_BASELINE_ALIGNMENT, false, &pItem))
        m_bIsMathBaselineAlignment = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && FLY_AS_CHAR == rAnchor.GetAnchorId()) );

    if (m_bFormat)
    {
        // at formats no anchor editing
        m_pAnchorFrame->Enable(false);
        m_pFixedRatioCB->Enable(false);
    }
    else
    {
        if (rAnchor.GetAnchorId() != FLY_AT_FLY && !pSh->IsFlyInFly())
            m_pAnchorAtFrameRB->Hide();
        if ( pSh->IsFrameVertical( true, m_bIsInRightToLeft, m_bIsVerticalL2R ) )
        {
            OUString sHLabel = m_pHorizontalFT->GetText();
            m_pHorizontalFT->SetText(m_pVerticalFT->GetText());
            m_pVerticalFT->SetText(sHLabel);
            m_bIsVerticalFrame = true;
        }
    }

    if ( m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog" )
    {
        OSL_ENSURE(pSh , "shell not found");
        //OS: only for the variant Insert/Graphic/Properties
        if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_GRF_REALSIZE, false, &pItem))
            m_aGrfSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();
        else
            pSh->GetGrfSize( m_aGrfSize );

        if ( !m_bNew )
        {
            m_pRealSizeBT->SetClickHdl(LINK(this, SwFramePage, RealSizeHdl));
            EnableGraficMode();
        }

        if ( m_sDlgType == "PictureDialog" )
            m_pFixedRatioCB->Check( false );
        else
        {
            if ( m_bNew )
                SetText( SW_RESSTR(STR_OLE_INSERT));
            else
                SetText( SW_RESSTR(STR_OLE_EDIT));
        }
    }
    else
    {
        m_aGrfSize = static_cast<const SwFormatFrameSize&>(rSet->Get(RES_FRM_SIZE)).GetSize();
    }

    // entering procent value made possible
    m_aWidthED.SetBaseValue( m_aWidthED.Normalize(m_aGrfSize.Width()), FUNIT_TWIP );
    m_aHeightED.SetBaseValue( m_aHeightED.Normalize(m_aGrfSize.Height()), FUNIT_TWIP );
    //the available space is not yet known so the RefValue has to be calculated from size and relative size values
    //this is needed only if relative values are already set

    const SwFormatFrameSize& rFrameSize = static_cast<const SwFormatFrameSize&>(rSet->Get(RES_FRM_SIZE));

    m_pRelWidthRelationLB->InsertEntry(m_aFramePosString.GetString(SwFPos::FRAME));
    m_pRelWidthRelationLB->InsertEntry(m_aFramePosString.GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetWidthPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetWidthPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrameSize.GetWidth() * 100 / rFrameSize.GetWidthPercent();
        m_aWidthED.SetRefValue( nSpace );

        m_pRelWidthRelationLB->Enable();
    }
    else
        m_pRelWidthRelationLB->Disable();

    m_pRelHeightRelationLB->InsertEntry(m_aFramePosString.GetString(SwFPos::FRAME));
    m_pRelHeightRelationLB->InsertEntry(m_aFramePosString.GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetHeightPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetHeightPercent() != 0)
    {
        //calculate the rerference value from the with and relative width values
        sal_Int32 nSpace = rFrameSize.GetHeight() * 100 / rFrameSize.GetHeightPercent();
        m_aHeightED.SetRefValue( nSpace );

        m_pRelHeightRelationLB->Enable();
    }
    else
        m_pRelHeightRelationLB->Disable();

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
    mpToCharContentPos = rAnchor.GetContentAnchor();

    // i#18732 - init checkbox value
    {
        const bool bFollowTextFlow =
            static_cast<const SwFormatFollowTextFlow&>(rSet->Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
        m_pFollowTextFlowCB->Check( bFollowTextFlow );
    }

    if(m_bHtmlMode)
    {
        m_pAutoHeightCB->Enable(false);
        m_pAutoWidthCB->Enable(false);
        m_pMirrorPagesCB->Show(false);
        if (m_sDlgType == "FrameDialog")
            m_pFixedRatioCB->Enable(false);
        // i#18732 hide checkbox in HTML mode
        m_pFollowTextFlowCB->Show(false);
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

    Init( *rSet, true );
    m_pAtVertPosED->SaveValue();
    m_pAtHorzPosED->SaveValue();
    m_pFollowTextFlowCB->SaveValue();

    m_bNoModifyHdl = false;
    //lock PercentFields
    m_aWidthED.LockAutoCalculation(true);
    m_aHeightED.LockAutoCalculation(true);
    RangeModifyHdl();  // set all maximum values initially
    m_aHeightED.LockAutoCalculation(false);
    m_aWidthED.LockAutoCalculation(false);

    m_pAutoHeightCB->SaveValue();
    m_pAutoWidthCB->SaveValue();

    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
}

// stuff attributes into the set when OK
bool SwFramePage::FillItemSet(SfxItemSet *rSet)
{
    bool bRet = false;
    SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell()
                        : getFrameDlgParentShell();
    OSL_ENSURE( pSh , "shell not found");
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOldItem = nullptr;

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();

    if ( !m_bFormat )
    {
        pOldItem = GetOldItem(*rSet, RES_ANCHOR);
        if (m_bNew || !pOldItem || eAnchorId != static_cast<const SwFormatAnchor*>(pOldItem)->GetAnchorId())
        {
            SwFormatAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
            bRet = nullptr != rSet->Put( aAnc );
        }
    }

    if ( m_pHMap )
    {
        SwFormatHoriOrient aHoriOrient( static_cast<const SwFormatHoriOrient&>(
                                                rOldSet.Get(RES_HORI_ORIENT)) );

        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        const sal_Int16 eHOri = GetAlignment(m_pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB);
        const sal_Int16 eRel = GetRelation(m_pHMap, *m_pHoriRelationLB);

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(m_pMirrorPagesCB->IsChecked());

        bool bMod = m_pAtHorzPosED->IsValueChangedFromSaved();
        bMod |= m_pMirrorPagesCB->IsValueChangedFromSaved();

        if ( eHOri == text::HoriOrientation::NONE &&
             (m_bNew || (m_bAtHorzPosModified || bMod) || m_nOldH != eHOri ) )
        {
            SwTwips nX = static_cast< SwTwips >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)));
            aHoriOrient.SetPos( nX );
        }

        pOldItem = GetOldItem(*rSet, FN_HORI_ORIENT);
        bool bSame = false;
        if ((m_bNew == m_bFormat) && pOldItem)
        {
             bSame = aHoriOrient == static_cast<const SwFormatHoriOrient&>(*pOldItem);
        }
        if ((m_bNew && !m_bFormat) || ((m_bAtHorzPosModified || bMod) && !bSame))
        {
            bRet |= nullptr != rSet->Put( aHoriOrient );
        }
    }

    if ( m_pVMap )
    {
        // alignment vertical
        SwFormatVertOrient aVertOrient( static_cast<const SwFormatVertOrient&>(
                                                rOldSet.Get(RES_VERT_ORIENT)) );

        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        const sal_Int16 eVOri = GetAlignment(m_pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB);
        const sal_Int16 eRel = GetRelation(m_pVMap, *m_pVertRelationLB);

        aVertOrient.SetVertOrient    ( eVOri);
        aVertOrient.SetRelationOrient( eRel );

        bool bMod = m_pAtVertPosED->IsValueChangedFromSaved();

        if ( eVOri == text::VertOrientation::NONE &&
             ( m_bNew || (m_bAtVertPosModified || bMod) || m_nOldV != eVOri) )
        {
            // vertical position
            // recalculate offset for character bound frames
            SwTwips nY = static_cast< SwTwips >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)));
            if (eAnchorId == FLY_AS_CHAR)
            {
                nY *= -1;
            }
            aVertOrient.SetPos( nY );
        }
        pOldItem = GetOldItem(*rSet, FN_VERT_ORIENT);
        bool bSame = false;
        if((m_bNew == m_bFormat) && pOldItem)
        {
             bSame = m_bFormat ?
                      aVertOrient.GetVertOrient() == static_cast<const SwFormatVertOrient*>(pOldItem)->GetVertOrient() &&
                      aVertOrient.GetRelationOrient() == static_cast<const SwFormatVertOrient*>(pOldItem)->GetRelationOrient() &&
                      aVertOrient.GetPos() == static_cast<const SwFormatVertOrient*>(pOldItem)->GetPos()
                    : aVertOrient == static_cast<const SwFormatVertOrient&>(*pOldItem);
        }
        if( ( m_bNew && !m_bFormat ) || ((m_bAtVertPosModified || bMod) && !bSame ))
        {
            bRet |= nullptr != rSet->Put( aVertOrient );
        }
    }

    // set size
    // new exception: when the size of pMgr(, 0), then the properties
    // for a graphic that isn't even loaded, are set. Then no SetSize
    // is done here when the size settings were not changed by the
    // user.
    const SwFormatFrameSize& rOldSize = static_cast<const SwFormatFrameSize& >(rOldSet.Get(RES_FRM_SIZE));
    SwFormatFrameSize aSz( rOldSize );

    sal_Int32 nRelWidthRelation = m_pRelWidthRelationLB->GetSelectEntryPos();
    if (nRelWidthRelation != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nRelWidthRelation == 0)
            aSz.SetWidthPercentRelation(text::RelOrientation::FRAME);
        else if (nRelWidthRelation == 1)
            aSz.SetWidthPercentRelation(text::RelOrientation::PAGE_FRAME);
    }
    sal_Int32 nRelHeightRelation = m_pRelHeightRelationLB->GetSelectEntryPos();
    if (nRelHeightRelation != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nRelHeightRelation == 0)
            aSz.SetHeightPercentRelation(text::RelOrientation::FRAME);
        else if (nRelHeightRelation == 1)
            aSz.SetHeightPercentRelation(text::RelOrientation::PAGE_FRAME);
    }

    bool bValueModified = (m_aWidthED.IsValueModified() || m_aHeightED.IsValueModified());
    bool bCheckChanged = m_pRelWidthCB->IsValueChangedFromSaved()
                         || m_pRelHeightCB->IsValueChangedFromSaved();

    bool bLegalValue = !(!rOldSize.GetWidth () && !rOldSize.GetHeight() &&
                            m_aWidthED .GetValue() == m_aWidthED .GetMin() &&
                            m_aHeightED.GetValue() == m_aHeightED.GetMin());

    if ((m_bNew && !m_bFormat) || ((bValueModified || bCheckChanged) && bLegalValue))
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

        if (m_pFixedRatioCB->IsChecked() && (m_pRelWidthCB->IsChecked() != m_pRelHeightCB->IsChecked()))
        {
            if (m_pRelWidthCB->IsChecked())
                aSz.SetHeightPercent(SwFormatFrameSize::SYNCED);
            else
                aSz.SetWidthPercent(SwFormatFrameSize::SYNCED);
        }
    }
    if( !IsInGraficMode() )
    {
        if( m_pAutoHeightCB->IsValueChangedFromSaved() )
        {
            SwFrameSize eFrameSize = m_pAutoHeightCB->IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrameSize != aSz.GetHeightSizeType() )
                aSz.SetHeightSizeType(eFrameSize);
        }
        if( m_pAutoWidthCB->IsValueChangedFromSaved() )
        {
            SwFrameSize eFrameSize = m_pAutoWidthCB->IsChecked()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrameSize != aSz.GetWidthSizeType() )
                aSz.SetWidthSizeType( eFrameSize );
        }
    }
    if( !m_bFormat && m_pFixedRatioCB->IsValueChangedFromSaved() )
        bRet |= nullptr != rSet->Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, m_pFixedRatioCB->IsChecked()));

    pOldItem = GetOldItem(*rSet, RES_FRM_SIZE);

    if ((pOldItem && aSz != *pOldItem) || (!pOldItem && !m_bFormat) ||
            (m_bFormat &&
                (aSz.GetWidth() > 0 || aSz.GetWidthPercent() > 0) &&
                    (aSz.GetHeight() > 0 || aSz.GetHeightPercent() > 0)))
    {
        if (aSz.GetHeightSizeType() == ATT_VAR_SIZE)    // there is no VAR_SIZE in frames
            aSz.SetHeightSizeType(ATT_MIN_SIZE);

        bRet |= nullptr != rSet->Put( aSz );
    }
    if(m_pFollowTextFlowCB->IsValueChangedFromSaved())
    {
        bRet |= nullptr != rSet->Put(SwFormatFollowTextFlow(m_pFollowTextFlowCB->IsChecked()));
    }
    return bRet;
}

// initialise horizonal and vertical Pos
void SwFramePage::InitPos(RndStdIds eId,
                                sal_Int16 nH,
                                sal_Int16 nHRel,
                                sal_Int16 nV,
                                sal_Int16 nVRel,
                                long   nX,
                                long   nY)
{
    sal_Int32 nPos = m_pVerticalDLB->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pVMap )
    {
        m_nOldV    = m_pVMap[nPos].nAlign;

        nPos = m_pVertRelationLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldVRel = static_cast<RelationMap *>(m_pVertRelationLB->GetEntryData(nPos))->nRelation;
    }

    nPos = m_pHorizontalDLB->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pHMap )
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_pHoriRelationLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldHRel = static_cast<RelationMap *>(m_pHoriRelationLB->GetEntryData(nPos))->nRelation;
    }

    bool bEnable = true;
    if ( eId == FLY_AT_PAGE )
    {
        m_pVMap = m_bHtmlMode ? aVPageHtmlMap : aVPageMap;
        m_pHMap = m_bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if ( eId == FLY_AT_FLY )
    {
        // own vertical alignment map for to frame
        // anchored objects.
        m_pVMap = m_bHtmlMode ? aVFlyHtmlMap : aVFrameMap;
        m_pHMap = m_bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if ( eId == FLY_AT_PARA )
    {
        if(m_bHtmlMode)
        {
            m_pVMap = aVParaHtmlMap;
            m_pHMap = aHParaHtmlAbsMap;
        }
        else
        {
            m_pVMap = aVParaMap;
            m_pHMap = aHParaMap;
        }
    }
    else if ( eId == FLY_AT_CHAR )
    {
        if(m_bHtmlMode)
        {
            m_pVMap = aVCharHtmlAbsMap;
            m_pHMap = aHCharHtmlAbsMap;
        }
        else
        {
            m_pVMap = aVCharMap;
            m_pHMap = aHCharMap;
        }
    }
    else if ( eId == FLY_AS_CHAR )
    {
        m_pVMap = m_bHtmlMode ? aVAsCharHtmlMap     : aVAsCharMap;
        m_pHMap = nullptr;
        bEnable = false;
    }
    m_pHorizontalDLB->Enable( bEnable );
    m_pHorizontalFT->Enable( bEnable );

    // select current Pos
    // horizontal
    if ( nH < 0 )
    {
        nH    = m_nOldH;
        nHRel = m_nOldHRel;
    }
    sal_Int32 nMapPos = FillPosLB(m_pHMap, nH, nHRel, *m_pHorizontalDLB);
    FillRelLB(m_pHMap, nMapPos, nH, nHRel, *m_pHoriRelationLB, *m_pHoriRelationFT);

    // vertical
    if ( nV < 0 )
    {
        nV    = m_nOldV;
        nVRel = m_nOldVRel;
    }
    nMapPos = FillPosLB(m_pVMap, nV, nVRel, *m_pVerticalDLB);
    FillRelLB(m_pVMap, nMapPos, nV, nVRel, *m_pVertRelationLB, *m_pVertRelationFT);

    bEnable = nH == text::HoriOrientation::NONE && eId != FLY_AS_CHAR;
    if (!bEnable)
    {
        m_pAtHorzPosED->SetValue( 0, FUNIT_TWIP );
        if (nX != LONG_MAX && m_bHtmlMode)
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
        if(nY != LONG_MAX && m_bHtmlMode)
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

sal_Int32 SwFramePage::FillPosLB(const FrameMap* _pMap,
                            const sal_Int16 _nAlign,
                            const sal_Int16 _nRel,
                            ListBox& _rLB )
{
    OUString sSelEntry;
    const OUString sOldEntry = _rLB.GetSelectEntry();

    _rLB.Clear();

    // i#22341 determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const sal_uLong nLBRelations = (_pMap != aVCharMap)
                               ? 0L
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill Listbox
    size_t nCount = ::lcl_GetFrameMapCount(_pMap);
    for (size_t i = 0; _pMap && i < nCount; ++i)
    {
//      Why not from the left/from inside or from above?
        {
            SvxSwFramePosString::StringId eStrId = m_pMirrorPagesCB->IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
            // --> OD 2009-08-31 #mongolianlayout#
            eStrId = lcl_ChangeResIdToVerticalOrRTL( eStrId,
                                                     m_bIsVerticalFrame,
                                                     m_bIsVerticalL2R,
                                                     m_bIsInRightToLeft);
            OUString sEntry(m_aFramePosString.GetString(eStrId));
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

    PosHdl(_rLB);

    return GetMapPos(_pMap, _rLB);
}

sal_uLong SwFramePage::FillRelLB( const FrameMap* _pMap,
                            const sal_uInt16 _nLBSelPos,
                            const sal_Int16 _nAlign,
                            const sal_Int16 _nRel,
                            ListBox& _rLB,
                            FixedText& _rFT )
{
    OUString sSelEntry;
    sal_uLong  nLBRelations = 0;
    size_t nMapCount = ::lcl_GetFrameMapCount(_pMap);

    _rLB.Clear();

    if (_nLBSelPos < nMapCount)
    {
        if (_pMap == aVAsCharHtmlMap || _pMap == aVAsCharMap)
        {
            const OUString sOldEntry(_rLB.GetSelectEntry());
            const size_t nRelCount = SAL_N_ELEMENTS(aAsCharRelationMap);
            SvxSwFramePosString::StringId eStrId = _pMap[_nLBSelPos].eStrId;

            for (size_t nMapPos = 0; nMapPos < nMapCount; nMapPos++)
            {
                if (_pMap[nMapPos].eStrId == eStrId)
                {
                    nLBRelations = _pMap[nMapPos].nLBRelations;
                    for (size_t nRelPos = 0; nRelPos < nRelCount; ++nRelPos)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = aAsCharRelationMap[nRelPos].eStrId;

                            // --> OD 2009-08-31 #mongolianlayout#
                            sStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( sStrId1,
                                                                m_bIsVerticalFrame,
                                                                m_bIsVerticalL2R,
                                                                m_bIsInRightToLeft);
                            const OUString sEntry = m_aFramePosString.GetString(sStrId1);
                            sal_Int32 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, &aAsCharRelationMap[nRelPos]);
                            if (_pMap[nMapPos].nAlign == _nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                _rLB.SelectEntry(sSelEntry);
            else
            {
                _rLB.SelectEntry(sOldEntry);

                if (!_rLB.GetSelectEntryCount())
                {
                    for (sal_Int32 i = 0; i < _rLB.GetEntryCount(); i++)
                    {
                        RelationMap *pEntry = static_cast<RelationMap *>(_rLB.GetEntryData(i));
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

            const size_t nRelCount = SAL_N_ELEMENTS(aRelationMap);

            for (sal_uLong nBit = 1; nBit < 0x80000000; nBit <<= 1)
            {
                if (nLBRelations & nBit)
                {
                    for (size_t nRelPos = 0; nRelPos < nRelCount; ++nRelPos)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == nBit)
                        {
                            SvxSwFramePosString::StringId eStrId1 = m_pMirrorPagesCB->IsChecked() ?
                                            aRelationMap[nRelPos].eMirrorStrId : aRelationMap[nRelPos].eStrId;
                            // --> OD 2009-08-31 #mongolianlayout#
                            eStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( eStrId1,
                                                                m_bIsVerticalFrame,
                                                                m_bIsVerticalL2R,
                                                                m_bIsInRightToLeft);
                            const OUString sEntry = m_aFramePosString.GetString(eStrId1);
                            sal_Int32 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, &aRelationMap[nRelPos]);
                            if (sSelEntry.isEmpty() && aRelationMap[nRelPos].nRelation == _nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                _rLB.SelectEntry(sSelEntry);
            else
            {
                // Probably anchor switch. So look for similar relation
                sal_Int16 nSimRel = -1;
                switch (_nRel)
                {
                    case text::RelOrientation::FRAME:
                        nSimRel = text::RelOrientation::PAGE_FRAME;
                        break;
                    case text::RelOrientation::PRINT_AREA:
                        nSimRel = text::RelOrientation::PAGE_PRINT_AREA;
                        break;
                    case text::RelOrientation::PAGE_LEFT:
                        nSimRel = text::RelOrientation::FRAME_LEFT;
                        break;
                    case text::RelOrientation::PAGE_RIGHT:
                        nSimRel = text::RelOrientation::FRAME_RIGHT;
                        break;
                    case text::RelOrientation::FRAME_LEFT:
                        nSimRel = text::RelOrientation::PAGE_LEFT;
                        break;
                    case text::RelOrientation::FRAME_RIGHT:
                        nSimRel = text::RelOrientation::PAGE_RIGHT;
                        break;
                    case text::RelOrientation::PAGE_FRAME:
                        nSimRel = text::RelOrientation::FRAME;
                        break;
                    case text::RelOrientation::PAGE_PRINT_AREA:
                        nSimRel = text::RelOrientation::PRINT_AREA;
                        break;

                    default:
                        if (_rLB.GetEntryCount())
                        {
                            RelationMap *pEntry = static_cast<RelationMap *>(_rLB.GetEntryData(_rLB.GetEntryCount() - 1));
                            nSimRel = pEntry->nRelation;
                        }
                        break;
                }

                for (sal_Int32 i = 0; i < _rLB.GetEntryCount(); i++)
                {
                    RelationMap *pEntry = static_cast<RelationMap *>(_rLB.GetEntryData(i));
                    if (pEntry->nRelation == nSimRel)
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

    RelHdl(_rLB);

    return nLBRelations;
}

sal_Int16 SwFramePage::GetRelation(FrameMap * /*pMap*/, ListBox &rRelationLB)
{
    const sal_Int32 nPos = rRelationLB.GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        RelationMap *pEntry = static_cast<RelationMap *>(rRelationLB.GetEntryData(nPos));
        return pEntry->nRelation;
    }

    return 0;
}

sal_Int16 SwFramePage::GetAlignment(FrameMap *pMap, sal_Int32 nMapPos,
        ListBox &/*rAlignLB*/, ListBox &rRelationLB)
{
    if (!pMap || nMapPos < 0)
        return 0;

    const size_t nMapCount = ::lcl_GetFrameMapCount(pMap);

    if (static_cast<size_t>(nMapPos) >= nMapCount)
        return 0;

    // i#22341 special handling also for map <aVCharMap>,
    // because it contains ambigous items for alignment
    if ( pMap != aVAsCharHtmlMap && pMap != aVAsCharMap && pMap != aVCharMap )
        return pMap[nMapPos].nAlign;

    if (rRelationLB.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        return 0;

    const RelationMap *const pRelationMap = static_cast<const RelationMap *>(
        rRelationLB.GetSelectEntryData());
    const sal_uLong nRel = pRelationMap->nLBRelation;
    const SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

    for (size_t i = 0; i < nMapCount; ++i)
    {
        if (pMap[i].eStrId == eStrId && (pMap[i].nLBRelations & nRel))
            return pMap[i].nAlign;
    }

    return 0;
}

sal_Int32 SwFramePage::GetMapPos( const FrameMap *pMap, ListBox &rAlignLB )
{
    sal_Int32 nMapPos = 0;
    sal_Int32 nLBSelPos = rAlignLB.GetSelectEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            const size_t nMapCount = ::lcl_GetFrameMapCount(pMap);
            const OUString sSelEntry(rAlignLB.GetSelectEntry());

            for (size_t i = 0; i < nMapCount; i++)
            {
                SvxSwFramePosString::StringId eResId = pMap[i].eStrId;

                OUString sEntry = m_aFramePosString.GetString(eResId);
                sEntry = MnemonicGenerator::EraseAllMnemonicChars( sEntry );

                if (sEntry == sSelEntry)
                {
                    nMapPos = static_cast< sal_Int32 >(i);
                    break;
                }
            }
        }
        else
            nMapPos = nLBSelPos;
    }

    return nMapPos;
}

RndStdIds SwFramePage::GetAnchor()
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

// Bsp - Update
void SwFramePage::ActivatePage(const SfxItemSet& rSet)
{
    m_bNoModifyHdl = true;
    Init(rSet);
    m_bNoModifyHdl = false;
    //lock PercentFields
    m_aWidthED.LockAutoCalculation(true);
    m_aHeightED.LockAutoCalculation(true);
    RangeModifyHdl();  // set all maximum values initially
    m_aHeightED.LockAutoCalculation(false);
    m_aWidthED.LockAutoCalculation(false);
    m_pFollowTextFlowCB->SaveValue();
}

SfxTabPage::sfxpg SwFramePage::DeactivatePage(SfxItemSet * _pSet)
{
    if ( _pSet )
    {
        FillItemSet( _pSet );

        //FillItemSet doesn't set the anchor into the set when it matches
        //the original. But for the other pages we need the current anchor.
        SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell()
                            : getFrameDlgParentShell();
        RndStdIds eAnchorId = (RndStdIds)GetAnchor();
        SwFormatAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
        _pSet->Put( aAnc );
    }

    return LEAVE_PAGE;
}

// swap left/right with inside/outside
IMPL_LINK_NOARG_TYPED(SwFramePage, MirrorHdl, Button*, void)
{
    RndStdIds eId = GetAnchor();
    InitPos( eId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
}

IMPL_LINK_TYPED( SwFramePage, RelSizeClickHdl, Button *, p, void )
{
    CheckBox* pBtn = static_cast<CheckBox*>(p);
    if (pBtn == m_pRelWidthCB)
    {
        m_aWidthED.ShowPercent(pBtn->IsChecked());
        m_pRelWidthRelationLB->Enable(pBtn->IsChecked());
        if(pBtn->IsChecked())
            m_aWidthED.get()->SetMax(MAX_PERCENT_WIDTH);
    }
    else // pBtn == m_pRelHeightCB
    {
        m_aHeightED.ShowPercent(pBtn->IsChecked());
        m_pRelHeightRelationLB->Enable(pBtn->IsChecked());
        if(pBtn->IsChecked())
            m_aHeightED.get()->SetMax(MAX_PERCENT_HEIGHT);
    }

    RangeModifyHdl();  // correct the values again

    if (pBtn == m_pRelWidthCB)
        ModifyHdl(*m_aWidthED.get());
    else // pBtn == m_pRelHeightCB
        ModifyHdl(*m_aHeightED.get());
}

// range check
IMPL_LINK_NOARG_TYPED(SwFramePage, RangeModifyClickHdl, Button*, void)
{
    RangeModifyHdl();
}
IMPL_LINK_NOARG_TYPED(SwFramePage, RangeModifyLoseFocusHdl, Control&, void)
{
    RangeModifyHdl();
}
void SwFramePage::RangeModifyHdl()
{
    if (m_bNoModifyHdl)
        return;

    SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell()
                        : getFrameDlgParentShell();
    OSL_ENSURE(pSh , "shell not found");
    SwFlyFrameAttrMgr aMgr( m_bNew, pSh, static_cast<const SwAttrSet&>(GetItemSet()) );
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = static_cast< sal_Int16 >(GetAnchor());
    aVal.bAutoHeight = m_pAutoHeightCB->IsChecked();
    aVal.bAutoWidth = m_pAutoWidthCB->IsChecked();
    aVal.bMirror = m_pMirrorPagesCB->IsChecked();
    aVal.bFollowTextFlow = m_pFollowTextFlowCB->IsChecked();

    if ( m_pHMap )
    {
        // alignment horizonal
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        aVal.nHoriOrient = GetAlignment(m_pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB);
        aVal.nHRelOrient = GetRelation(m_pHMap, *m_pHoriRelationLB);
    }
    else
        aVal.nHoriOrient = text::HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // alignment vertical
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        aVal.nVertOrient = GetAlignment(m_pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB);
        aVal.nVRelOrient = GetRelation(m_pVMap, *m_pVertRelationLB);
    }
    else
        aVal.nVertOrient = text::VertOrientation::NONE;

    const long nAtHorzPosVal = static_cast< long >(
                    m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)) );
    const long nAtVertPosVal = static_cast< long >(
                    m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)) );

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, mpToCharContentPos, true);   // one time, to get reference values for percental values

    // set reference values for percental values (100%) ...
    m_aWidthED.SetRefValue(aVal.aPercentSize.Width());
    m_aHeightED.SetRefValue(aVal.aPercentSize.Height());

    // ... and correctly convert width and height with it
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED. DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    aMgr.ValidateMetrics(aVal, mpToCharContentPos);    // one more time, to determine all remaining values with correct width and height.

    // all columns have to be correct
    if(GetTabDialog()->GetExampleSet() &&
            SfxItemState::DEFAULT <= GetTabDialog()->GetExampleSet()->GetItemState(RES_COL))
    {
        const SwFormatCol& rCol = static_cast<const SwFormatCol&>(GetTabDialog()->GetExampleSet()->Get(RES_COL));
        if ( rCol.GetColumns().size() > 1 )
        {
            for ( size_t i = 0; i < rCol.GetColumns().size(); ++i )
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

    if (aVal.bAutoHeight && (m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"))
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
        ? m_nUpperBorder : 0;
    const SwTwips nLowerOffset = (aVal.nAnchorType == FLY_AS_CHAR)
        ? m_nLowerBorder : 0;

    m_pAtVertPosED->SetMin(m_pAtVertPosED->Normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset), FUNIT_TWIP);
    m_pAtVertPosED->SetMax(m_pAtVertPosED->Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_pAtVertPosED->SetValue(m_pAtVertPosED->Normalize(aVal.nVPos), FUNIT_TWIP);
}

IMPL_LINK_NOARG_TYPED(SwFramePage, AnchorTypeHdl, Button*, void)
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

    InitPos( eId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl();

    if(m_bHtmlMode)
    {
        PosHdl(*m_pHorizontalDLB);
        PosHdl(*m_pVerticalDLB);
    }

    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && FLY_AS_CHAR == eId) );
}

IMPL_LINK_TYPED( SwFramePage, PosHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHorizontalDLB;
    ListBox *pRelLB = bHori ? m_pHoriRelationLB : m_pVertRelationLB;
    FixedText *pRelFT = bHori ? m_pHoriRelationFT : m_pVertRelationFT;
    FrameMap *pMap = bHori ? m_pHMap : m_pVMap;

    const sal_Int32 nMapPos = GetMapPos(pMap, rLB);
    const sal_Int16 nAlign = GetAlignment(pMap, nMapPos, rLB, *pRelLB);

    if (bHori)
    {
        bool bEnable = text::HoriOrientation::NONE == nAlign;
        m_pAtHorzPosED->Enable( bEnable );
        m_pAtHorzPosFT->Enable( bEnable );
    }
    else
    {
        bool bEnable = text::VertOrientation::NONE == nAlign && m_bAllowVertPositioning;
        m_pAtVertPosED->Enable( bEnable );
        m_pAtVertPosFT->Enable( bEnable );
    }

    RangeModifyHdl();

    sal_Int16 nRel = 0;
    if (rLB.GetSelectEntryCount())
    {

        if (pRelLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
            nRel = static_cast<RelationMap *>(pRelLB->GetSelectEntryData())->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->Clear();

    UpdateExample();

    if (bHori)
        m_bAtHorzPosModified = true;
    else
        m_bAtVertPosModified = true;

    // special treatment for HTML-Mode with horizonal-vertical-dependencies
    if(m_bHtmlMode && (FLY_AT_CHAR == GetAnchor()))
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
                PosHdl(*m_pVerticalDLB);
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
                PosHdl(*m_pHorizontalDLB);
        }

    }
}

//  horizontal Pos
IMPL_LINK_TYPED( SwFramePage, RelHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHoriRelationLB;

    UpdateExample();

    if (bHori)
        m_bAtHorzPosModified = true;
    else
        m_bAtVertPosModified = true;

    if (m_bHtmlMode && (FLY_AT_CHAR == GetAnchor()))
    {
        if(bHori)
        {
            const sal_Int16 nRel = GetRelation(m_pHMap, *m_pHoriRelationLB);
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
    RangeModifyHdl();
}

IMPL_LINK_NOARG_TYPED(SwFramePage, RealSizeHdl, Button*, void)
{
    m_aWidthED.SetUserValue( m_aWidthED. NormalizePercent(m_aGrfSize.Width() ), FUNIT_TWIP);
    m_aHeightED.SetUserValue(m_aHeightED.NormalizePercent(m_aGrfSize.Height()), FUNIT_TWIP);
    m_fWidthHeightRatio = m_aGrfSize.Height() ? double(m_aGrfSize.Width()) / double(m_aGrfSize.Height()) : 1.0;
    UpdateExample();
}

IMPL_LINK_NOARG_TYPED(SwFramePage, AutoWidthClickHdl, Button*, void)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoWidthCB->IsChecked(), *m_pWidthFT, *m_pWidthAutoFT, *m_aWidthED.get() );
}

IMPL_LINK_NOARG_TYPED(SwFramePage, AutoHeightClickHdl, Button*, void)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoHeightCB->IsChecked(), *m_pHeightFT, *m_pHeightAutoFT, *m_aWidthED.get() );
}

IMPL_LINK_TYPED( SwFramePage, ModifyHdl, Edit&, rEdit, void )
{
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FUNIT_TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FUNIT_TWIP)));
    if ( m_pFixedRatioCB->IsChecked() )
    {
        if (&rEdit == m_aWidthED.get())
        {
            nHeight = SwTwips((double)nWidth / m_fWidthHeightRatio);
            m_aHeightED.SetPrcntValue(m_aHeightED.NormalizePercent(nHeight), FUNIT_TWIP);
        }
        else if (&rEdit == m_aHeightED.get())
        {
            nWidth = SwTwips((double)nHeight * m_fWidthHeightRatio);
            m_aWidthED.SetPrcntValue(m_aWidthED.NormalizePercent(nWidth), FUNIT_TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
}

void SwFramePage::UpdateExample()
{
    sal_Int32 nPos = m_pHorizontalDLB->GetSelectEntryPos();
    if ( m_pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        m_pExampleWN->SetHAlign(GetAlignment(m_pHMap, nMapPos, *m_pHorizontalDLB, *m_pHoriRelationLB));
        m_pExampleWN->SetHoriRel(GetRelation(m_pHMap, *m_pHoriRelationLB));
    }

    nPos = m_pVerticalDLB->GetSelectEntryPos();
    if ( m_pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        m_pExampleWN->SetVAlign(GetAlignment(m_pVMap, nMapPos, *m_pVerticalDLB, *m_pVertRelationLB));
        m_pExampleWN->SetVertRel(GetRelation(m_pVMap, *m_pVertRelationLB));
    }

    // size
    long nXPos = static_cast< long >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FUNIT_TWIP)));
    long nYPos = static_cast< long >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FUNIT_TWIP)));
    m_pExampleWN->SetRelPos(Point(nXPos, nYPos));

    m_pExampleWN->SetAnchor( static_cast< sal_Int16 >(GetAnchor()) );
    m_pExampleWN->Invalidate();
}

void SwFramePage::Init(const SfxItemSet& rSet, bool bReset)
{
    if(!m_bFormat)
    {
        SwWrtShell* pSh = getFrameDlgParentShell();

        // size
        const bool bSizeFixed = pSh->IsSelObjProtected( FlyProtectFlags::Fixed ) != FlyProtectFlags::NONE;

        m_aWidthED .Enable( !bSizeFixed );
        m_aHeightED.Enable( !bSizeFixed );

        // size controls for math OLE objects
        if ( m_sDlgType == "ObjectDialog" && ! m_bNew )
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

    const SwFormatFrameSize& rSize = static_cast<const SwFormatFrameSize&>(rSet.Get(RES_FRM_SIZE));
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
        SwFrameSize eSize = rSize.GetHeightSizeType();
        bool bCheck = eSize != ATT_FIX_SIZE;
        m_pAutoHeightCB->Check( bCheck );
        HandleAutoCB( bCheck, *m_pHeightFT, *m_pHeightAutoFT, *m_aWidthED.get() );
        if( eSize == ATT_VAR_SIZE )
            m_aHeightED.SetValue( m_aHeightED.GetMin() );

        eSize = rSize.GetWidthSizeType();
        bCheck = eSize != ATT_FIX_SIZE;
        m_pAutoWidthCB->Check( bCheck );
        HandleAutoCB( bCheck, *m_pWidthFT, *m_pWidthAutoFT, *m_aWidthED.get() );
        if( eSize == ATT_VAR_SIZE )
            m_aWidthED.SetValue( m_aWidthED.GetMin() );

        if ( !m_bFormat )
        {
            SwWrtShell* pSh = getFrameDlgParentShell();
            const SwFrameFormat* pFormat = pSh->GetFlyFrameFormat();
            if( pFormat && pFormat->GetChain().GetNext() )
                m_pAutoHeightCB->Enable( false );
        }
    }
    else
        m_pAutoHeightCB->Hide();

    // organise circulation-gap for character bound frames
    const SvxULSpaceItem &rUL = static_cast<const SvxULSpaceItem &>(rSet.Get(RES_UL_SPACE));
    m_nUpperBorder = rUL.GetUpper();
    m_nLowerBorder = rUL.GetLower();

    if(SfxItemState::SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        m_pFixedRatioCB->Check(static_cast<const SfxBoolItem&>(rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        m_pFixedRatioCB->SaveValue();
    }

    // columns
    SwFormatCol aCol( static_cast<const SwFormatCol&>(rSet.Get(RES_COL)) );
    ::FitToActualSize( aCol, (sal_uInt16)rSize.GetWidth() );

    RndStdIds eAnchorId = (RndStdIds)GetAnchor();

    if ( m_bNew && !m_bFormat )
        InitPos(eAnchorId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
    else
    {
        const SwFormatHoriOrient& rHori = static_cast<const SwFormatHoriOrient&>(rSet.Get(RES_HORI_ORIENT));
        const SwFormatVertOrient& rVert = static_cast<const SwFormatVertOrient&>(rSet.Get(RES_VERT_ORIENT));
        m_nOldH    = rHori.GetHoriOrient();
        m_nOldHRel = rHori.GetRelationOrient();
        m_nOldV    = rVert.GetVertOrient();
        m_nOldVRel = rVert.GetRelationOrient();

        if (eAnchorId == FLY_AT_PAGE)
        {
            if (m_nOldHRel == text::RelOrientation::FRAME)
                m_nOldHRel = text::RelOrientation::PAGE_FRAME;
            else if (m_nOldHRel == text::RelOrientation::PRINT_AREA)
                m_nOldHRel = text::RelOrientation::PAGE_PRINT_AREA;
            if (m_nOldVRel == text::RelOrientation::FRAME)
                m_nOldVRel = text::RelOrientation::PAGE_FRAME;
            else if (m_nOldVRel == text::RelOrientation::PRINT_AREA)
                m_nOldVRel = text::RelOrientation::PAGE_PRINT_AREA;
        }

        m_pMirrorPagesCB->Check(rHori.IsPosToggle());
        m_pMirrorPagesCB->SaveValue();

        InitPos(eAnchorId,
                m_nOldH,
                m_nOldHRel,
                m_nOldV,
                m_nOldVRel,
                rHori.GetPos(),
                rVert.GetPos());
    }

    // transparent for example
    // circulation for example
    const SwFormatSurround& rSurround = static_cast<const SwFormatSurround&>(rSet.Get(RES_SURROUND));
    m_pExampleWN->SetWrap ( static_cast< sal_uInt16 >(rSurround.GetSurround()) );

    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
    {
        const SvxOpaqueItem& rOpaque = static_cast<const SvxOpaqueItem&>(rSet.Get(RES_OPAQUE));
        m_pExampleWN->SetTransparent(!rOpaque.GetValue());
    }

    // switch to percent if applicable
    RangeModifyHdl();  // set reference values (for 100%)

    if (rSize.GetWidthPercent() == SwFormatFrameSize::SYNCED || rSize.GetHeightPercent() == SwFormatFrameSize::SYNCED)
        m_pFixedRatioCB->Check();
    if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED &&
        !m_pRelWidthCB->IsChecked())
    {
        m_pRelWidthCB->Check();
        RelSizeClickHdl(m_pRelWidthCB);
        m_aWidthED.SetPrcntValue(rSize.GetWidthPercent(), FUNIT_CUSTOM);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != SwFormatFrameSize::SYNCED &&
        !m_pRelHeightCB->IsChecked())
    {
        m_pRelHeightCB->Check();
        RelSizeClickHdl(m_pRelHeightCB);
        m_aHeightED.SetPrcntValue(rSize.GetHeightPercent(), FUNIT_CUSTOM);
    }
    m_pRelWidthCB->SaveValue();
    m_pRelHeightCB->SaveValue();

    if (rSize.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
        m_pRelWidthRelationLB->SelectEntryPos(1);
    else
        m_pRelWidthRelationLB->SelectEntryPos(0);

    if (rSize.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
        m_pRelHeightRelationLB->SelectEntryPos(1);
    else
        m_pRelHeightRelationLB->SelectEntryPos(0);
}

void SwFramePage::SetFormatUsed(bool bFormatUsed)
{
    m_bFormat = bFormatUsed;
    if (m_bFormat)
    {
        m_pAnchorFrame->Hide();
    }
}

void SwFramePage::EnableVerticalPositioning( bool bEnable )
{
    m_bAllowVertPositioning = bEnable;
    m_pVerticalFT->Enable( bEnable );
    m_pVerticalDLB->Enable( bEnable );
    m_pAtVertPosFT->Enable( bEnable );
    m_pAtVertPosED->Enable( bEnable );
    m_pVertRelationFT->Enable( bEnable );
    m_pVertRelationLB->Enable( bEnable );
}

SwGrfExtPage::SwGrfExtPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "PicturePage",
        "modules/swriter/ui/picturepage.ui", &rSet)
    , pGrfDlg(nullptr)
    , bHtmlMode(false)
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
    disposeOnce();
}

void SwGrfExtPage::dispose()
{
    delete pGrfDlg;
    m_pMirror.clear();
    m_pMirrorVertBox.clear();
    m_pMirrorHorzBox.clear();
    m_pAllPagesRB.clear();
    m_pLeftPagesRB.clear();
    m_pRightPagesRB.clear();
    m_pBmpWin.clear();
    m_pConnectED.clear();
    m_pBrowseBT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwGrfExtPage::Create( vcl::Window *pParent, const SfxItemSet *rSet )
{
    return VclPtr<SwGrfExtPage>::Create( pParent, *rSet );
}

void SwGrfExtPage::Reset(const SfxItemSet *rSet)
{
    const SfxPoolItem* pItem;
    const sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    bHtmlMode = (nHtmlMode & HTMLMODE_ON) != 0;

    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_GRF_CONNECT, true, &pItem)
        && static_cast<const SfxBoolItem *>(pItem)->GetValue() )
    {
        m_pBrowseBT->Enable();
        m_pConnectED->SetReadOnly(false);
    }

    ActivatePage(*rSet);
}

void SwGrfExtPage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxProtectItem& rProt = static_cast<const SvxProtectItem& >(rSet.Get(RES_PROTECT));
    bool bProtContent = rProt.IsContentProtected();

    const SfxPoolItem* pItem = nullptr;
    bool bEnable = false;
    bool bEnableMirrorRB = false;

    SfxItemState eState = rSet.GetItemState(RES_GRFATR_MIRRORGRF, true, &pItem);
    if( SfxItemState::UNKNOWN != eState && !bProtContent && !bHtmlMode )
    {
        if( SfxItemState::SET != eState )
            pItem = &rSet.Get( RES_GRFATR_MIRRORGRF );

        bEnable = true;

        MirrorGraph eMirror = static_cast< MirrorGraph >(static_cast<const SwMirrorGrf* >(pItem)->GetValue());
        switch( eMirror )
        {
        case RES_MIRROR_GRAPH_DONT: break;
        case RES_MIRROR_GRAPH_VERT: m_pMirrorHorzBox->Check(); break;
        case RES_MIRROR_GRAPH_HOR:  m_pMirrorVertBox->Check(); break;
        case RES_MIRROR_GRAPH_BOTH: m_pMirrorHorzBox->Check();
                                    m_pMirrorVertBox->Check();
                                    break;
        default:
            ;
        }

        const int nPos = (static_cast<const SwMirrorGrf* >(pItem)->IsGrfToggle() ? 1 : 0)
            + ((eMirror == RES_MIRROR_GRAPH_VERT || eMirror == RES_MIRROR_GRAPH_BOTH) ? 2 : 0);

        bEnableMirrorRB = nPos != 0;

        switch (nPos)
        {
            case 1: // mirror at left / even pages
                m_pLeftPagesRB->Check();
                m_pMirrorHorzBox->Check();
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

    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, false, &pItem ) )
    {
        const SvxBrushItem& rBrush = *static_cast<const SvxBrushItem*>(pItem);
        if( !rBrush.GetGraphicLink().isEmpty() )
        {
            aGrfName = aNewGrfName = rBrush.GetGraphicLink();
            m_pConnectED->SetText( aNewGrfName );
        }
        OUString referer;
        SfxStringItem const * it = static_cast<SfxStringItem const *>(
            rSet.GetItem(SID_REFERER));
        if (it != nullptr) {
            referer = it->GetValue();
        }
        const Graphic* pGrf = rBrush.GetGraphic(referer);
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

bool SwGrfExtPage::FillItemSet( SfxItemSet *rSet )
{
    bool bModified = false;
    if ( m_pMirrorHorzBox->IsValueChangedFromSaved() ||
         m_pMirrorVertBox->IsValueChangedFromSaved() ||
         m_pAllPagesRB->IsValueChangedFromSaved() ||
         m_pLeftPagesRB->IsValueChangedFromSaved() ||
         m_pRightPagesRB->IsValueChangedFromSaved() )
    {
        bModified = true;

        bool bHori = false;

        if (m_pMirrorHorzBox->IsChecked() &&
                !m_pLeftPagesRB->IsChecked())
            bHori = true;

        MirrorGraph eMirror;
        eMirror = m_pMirrorVertBox->IsChecked() && bHori ?
                    RES_MIRROR_GRAPH_BOTH : bHori ?
                    RES_MIRROR_GRAPH_VERT : m_pMirrorVertBox->IsChecked() ?
                    RES_MIRROR_GRAPH_HOR  : RES_MIRROR_GRAPH_DONT;

        bool bMirror = !m_pAllPagesRB->IsChecked();
        SwMirrorGrf aMirror( eMirror );
        aMirror.SetGrfToggle(bMirror );
        rSet->Put( aMirror );
    }

    if( aGrfName != aNewGrfName || m_pConnectED->IsModified() )
    {
        bModified = true;
        aGrfName = m_pConnectED->GetText();
        rSet->Put( SvxBrushItem( aGrfName, aFilterName, GPOS_LT,
                                SID_ATTR_GRAF_GRAPHIC ));
    }
    return bModified;
}

SfxTabPage::sfxpg SwGrfExtPage::DeactivatePage(SfxItemSet *_pSet)
{
    if( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}

IMPL_LINK_NOARG_TYPED(SwGrfExtPage, BrowseHdl, Button*, void)
{
    if(!pGrfDlg)
    {
        pGrfDlg = new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
                SFXWB_GRAPHIC );
        pGrfDlg->SetTitle(get<VclFrame>("linkframe")->get_label());
    }
    pGrfDlg->SetDisplayDirectory( m_pConnectED->GetText() );
    uno::Reference < ui::dialogs::XFilePicker2 > xFP = pGrfDlg->GetFilePicker();
    uno::Reference < ui::dialogs::XFilePickerControlAccess > xCtrlAcc(xFP, uno::UNO_QUERY);
    xCtrlAcc->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, uno::makeAny(true) );

    if ( pGrfDlg->Execute() == ERRCODE_NONE )
    {   // remember selected filter
        aFilterName = pGrfDlg->GetCurrentFilter();
        aNewGrfName = INetURLObject::decode( pGrfDlg->GetPath(),
                                           INetURLObject::DECODE_UNAMBIGUOUS );
        m_pConnectED->SetModifyFlag();
        m_pConnectED->SetText( aNewGrfName );
        //reset mirrors because maybe a Bitmap was swapped with
        //another type of graphic that cannot be mirrored.
        m_pMirrorVertBox->Check(false);
        m_pMirrorHorzBox->Check(false);
        m_pAllPagesRB->Enable(false);
        m_pLeftPagesRB->Enable(false);
        m_pRightPagesRB->Enable(false);
        m_pBmpWin->MirrorHorz(false);
        m_pBmpWin->MirrorVert(false);

        Graphic aGraphic;
        (void)GraphicFilter::LoadGraphic(pGrfDlg->GetPath(), OUString(), aGraphic);
        m_pBmpWin->SetGraphic(aGraphic);

        bool bEnable = GRAPHIC_BITMAP      == aGraphic.GetType() ||
                            GRAPHIC_GDIMETAFILE == aGraphic.GetType();
        m_pMirrorVertBox->Enable(bEnable);
        m_pMirrorHorzBox->Enable(bEnable);
        m_pAllPagesRB->Enable(bEnable);
        m_pLeftPagesRB->Enable(bEnable);
        m_pRightPagesRB->Enable(bEnable);
    }
}

IMPL_LINK_NOARG_TYPED(SwGrfExtPage, MirrorHdl, Button*, void)
{
    bool bEnable = m_pMirrorHorzBox->IsChecked();

    m_pBmpWin->MirrorHorz( m_pMirrorVertBox->IsChecked() );
    m_pBmpWin->MirrorVert( bEnable );

    m_pAllPagesRB->Enable(bEnable);
    m_pLeftPagesRB->Enable(bEnable);
    m_pRightPagesRB->Enable(bEnable);

    if (!m_pAllPagesRB->IsChecked() && !m_pLeftPagesRB->IsChecked() && !m_pRightPagesRB->IsChecked())
        m_pAllPagesRB->Check();
}

// example window
BmpWindow::BmpWindow(vcl::Window* pPar, WinBits nStyle)
    : Window(pPar, nStyle)
    , bHorz(false)
    , bVert(false)
    , bGraphic(false)
    , bLeftAlign(false)
{
}

Size BmpWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(127 , 66), MapMode(MAP_APPFONT));
}

VCL_BUILDER_FACTORY_ARGS(BmpWindow, 0)

void BmpWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    // Setup
    rRenderContext.SetBackground();
    SetPaintTransparent(true);
    // #i119307# the graphic might have transparency, set up white as the color
    // to use when drawing a rectangle under the image
    rRenderContext.SetLineColor(COL_WHITE);
    rRenderContext.SetFillColor(COL_WHITE);

    // Paint
    Point aPntPos;
    Size aPntSz(GetSizePixel());
    Size aGrfSize;
    if (bGraphic)
        aGrfSize = ::GetGraphicSizeTwip(aGraphic, &rRenderContext);
    //it should show the default bitmap also if no graphic can be found
    if (!aGrfSize.Width() && !aGrfSize.Height())
        aGrfSize = rRenderContext.PixelToLogic(aBmp.GetSizePixel());

    long nRelGrf = aGrfSize.Width() * 100L / aGrfSize.Height();
    long nRelWin = aPntSz.Width() * 100L / aPntSz.Height();
    if (nRelGrf < nRelWin)
    {
        const long nWidth = aPntSz.Width();
        // if we use a replacement preview, try to draw at original size
        if (!bGraphic && (aGrfSize.Width() <= aPntSz.Width())
                      && (aGrfSize.Height() <= aPntSz.Height()))
        {
            const long nHeight = aPntSz.Height();
            aPntSz.Width() = aGrfSize.Width();
            aPntSz.Height() = aGrfSize.Height();
            aPntPos.Y() += (nHeight - aPntSz.Height()) / 2;
        }
        else
            aPntSz.Width() = aPntSz.Height() * nRelGrf /100;

        if (!bLeftAlign)
            aPntPos.X() += nWidth - aPntSz.Width() ;
    }

    // #i119307# clear window background, the graphic might have transparency
    rRenderContext.DrawRect(Rectangle(aPntPos, aPntSz));

    if (bHorz || bVert)
    {
        BitmapEx aTmpBmp(bGraphic ? aGraphic.GetBitmapEx() : aBmp);
        BmpMirrorFlags nMirrorFlags(BmpMirrorFlags::NONE);
        if (bHorz)
            nMirrorFlags |= BmpMirrorFlags::Vertical;
        if (bVert)
            nMirrorFlags |= BmpMirrorFlags::Horizontal;
        aTmpBmp.Mirror(nMirrorFlags);
        rRenderContext.DrawBitmapEx(aPntPos, aPntSz, aTmpBmp);
    }
    else if (bGraphic)  //draw unmirrored preview graphic
    {
        aGraphic.Draw(&rRenderContext, aPntPos, aPntSz);
    }
    else    //draw unmirrored stock sample image
    {
        rRenderContext.DrawBitmapEx(aPntPos, aPntSz, aBmp);
    }
}

BmpWindow::~BmpWindow()
{
}

void BmpWindow::SetGraphic(const Graphic& rGraphic)
{
    aGraphic = rGraphic;
    Size aSize = aGraphic.GetPrefSize();
    bGraphic = aSize.Width() && aSize.Height();
    Invalidate();
}

void BmpWindow::SetBitmapEx(const BitmapEx& rBmp)
{
    aBmp = rBmp;
    Invalidate();
}

// set URL and ImageMap at frames
SwFrameURLPage::SwFrameURLPage( vcl::Window *pParent, const SfxItemSet &rSet ) :
    SfxTabPage(pParent, "FrameURLPage" , "modules/swriter/ui/frmurlpage.ui", &rSet)
{
    get(pURLED,"url");
    get(pSearchPB,"search");
    get(pNameED,"name");
    get(pFrameCB,"frame");

    get(pServerCB,"server");
    get(pClientCB,"client");

    pSearchPB->SetClickHdl(LINK(this, SwFrameURLPage, InsertFileHdl));
}

SwFrameURLPage::~SwFrameURLPage()
{
    disposeOnce();
}

void SwFrameURLPage::dispose()
{
    pURLED.clear();
    pSearchPB.clear();
    pNameED.clear();
    pFrameCB.clear();
    pServerCB.clear();
    pClientCB.clear();
    SfxTabPage::dispose();
}

void SwFrameURLPage::Reset( const SfxItemSet *rSet )
{
    const SfxPoolItem* pItem;
    if ( SfxItemState::SET == rSet->GetItemState( SID_DOCFRAME, true, &pItem))
    {
        std::unique_ptr<TargetList> pList(new TargetList);
        static_cast<const SfxFrameItem*>(pItem)->GetFrame()->GetTargetList(*pList);
        if( !pList->empty() )
        {
            size_t nCount = pList->size();
            for ( size_t i = 0; i < nCount; i++ )
            {
                pFrameCB->InsertEntry( pList->at( i ) );
            }
        }
    }

    if ( SfxItemState::SET == rSet->GetItemState( RES_URL, true, &pItem ) )
    {
        const SwFormatURL* pFormatURL = static_cast<const SwFormatURL*>(pItem);
        pURLED->SetText( INetURLObject::decode( pFormatURL->GetURL(),
                                           INetURLObject::DECODE_UNAMBIGUOUS ));
        pNameED->SetText( pFormatURL->GetName());

        pClientCB->Enable( pFormatURL->GetMap() != nullptr );
        pClientCB->Check ( pFormatURL->GetMap() != nullptr );
        pServerCB->Check ( pFormatURL->IsServerMap() );

        pFrameCB->SetText(pFormatURL->GetTargetFrameName());
        pFrameCB->SaveValue();
    }
    else
        pClientCB->Enable( false );

    pServerCB->SaveValue();
    pClientCB->SaveValue();
}

bool SwFrameURLPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = false;
    const SwFormatURL* pOldURL = static_cast<const SwFormatURL*>(GetOldItem(*rSet, RES_URL));
    std::unique_ptr<SwFormatURL> pFormatURL;
    if(pOldURL)
        pFormatURL.reset(static_cast<SwFormatURL*>(pOldURL->Clone()));
    else
        pFormatURL.reset(new SwFormatURL());

    {
        const OUString sText = pURLED->GetText();

        if( pFormatURL->GetURL() != sText ||
            pFormatURL->GetName() != pNameED->GetText() ||
            pServerCB->IsChecked() != pFormatURL->IsServerMap() )
        {
            pFormatURL->SetURL( sText, pServerCB->IsChecked() );
            pFormatURL->SetName( pNameED->GetText() );
            bModified = true;
        }
    }

    if(!pClientCB->IsChecked() && pFormatURL->GetMap() != nullptr)
    {
        pFormatURL->SetMap(nullptr);
        bModified = true;
    }

    if(pFormatURL->GetTargetFrameName() != pFrameCB->GetText())
    {
        pFormatURL->SetTargetFrameName(pFrameCB->GetText());
        bModified = true;
    }
    rSet->Put(*pFormatURL);
    return bModified;
}

VclPtr<SfxTabPage> SwFrameURLPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFrameURLPage>::Create( pParent, *rSet );
}

IMPL_LINK_NOARG_TYPED(SwFrameURLPage, InsertFileHdl, Button*, void)
{
    FileDialogHelper aDlgHelper( ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    uno::Reference < ui::dialogs::XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

    try
    {
        const OUString sTemp(pURLED->GetText());
        if(!sTemp.isEmpty())
            xFP->setDisplayDirectory(sTemp);
    }
    catch( const uno::Exception& rEx )
    {
        (void) rEx;
    }
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        pURLED->SetText( xFP->getSelectedFiles().getConstArray()[0] );
    }
}

SwFrameAddPage::SwFrameAddPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "FrameAddPage" , "modules/swriter/ui/frmaddpage.ui", &rSet)
    , m_pWrtSh(nullptr)
    , m_bHtmlMode(false)
    , m_bFormat(false)
    , m_bNew(false)
{
    get(m_pNameFrame, "nameframe");
    get(m_pNameFT,"name_label");
    get(m_pNameED,"name");
    get(m_pAltNameFT,"altname_label");
    get(m_pAltNameED,"altname");
    get(m_pPrevFT,"prev_label");
    get(m_pPrevLB,"prev");
    get(m_pNextFT,"next_label");
    get(m_pNextLB,"next");

    get(m_pProtectFrame,"protect");
    get(m_pProtectContentCB,"protectcontent");
    get(m_pProtectFrameCB,"protectframe");
    get(m_pProtectSizeCB,"protectsize");

    get(m_pContentAlignFrame, "contentalign");
    get(m_pVertAlignLB,"vertalign");

    get(m_pPropertiesFrame,"properties");
    get(m_pEditInReadonlyCB,"editinreadonly");
    get(m_pPrintFrameCB,"printframe");
    get(m_pTextFlowFT,"textflow_label");
    get(m_pTextFlowLB,"textflow");

}

SwFrameAddPage::~SwFrameAddPage()
{
    disposeOnce();
}

void SwFrameAddPage::dispose()
{
    m_pNameFrame.clear();
    m_pNameFT.clear();
    m_pNameED.clear();
    m_pAltNameFT.clear();
    m_pAltNameED.clear();
    m_pPrevFT.clear();
    m_pPrevLB.clear();
    m_pNextFT.clear();
    m_pNextLB.clear();
    m_pProtectFrame.clear();
    m_pProtectContentCB.clear();
    m_pProtectFrameCB.clear();
    m_pProtectSizeCB.clear();
    m_pContentAlignFrame.clear();
    m_pVertAlignLB.clear();
    m_pPropertiesFrame.clear();
    m_pEditInReadonlyCB.clear();
    m_pPrintFrameCB.clear();
    m_pTextFlowFT.clear();
    m_pTextFlowLB.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SwFrameAddPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFrameAddPage>::Create(pParent, *rSet);
}

void SwFrameAddPage::Reset(const SfxItemSet *rSet )
{
    const SfxPoolItem* pItem;
    sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    m_bHtmlMode = (nHtmlMode & HTMLMODE_ON) != 0;
    if (m_bHtmlMode)
    {
        m_pProtectFrame->Hide();
        m_pEditInReadonlyCB->Hide();
        m_pPrintFrameCB->Hide();
    }
    if (m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog")
    {
        m_pEditInReadonlyCB->Hide();
        if (m_bHtmlMode)
        {
            m_pPropertiesFrame->Hide();
        }
        m_pContentAlignFrame->Hide();
    }

    if(SfxItemState::SET == rSet->GetItemState(FN_SET_FRM_ALT_NAME, false, &pItem))
    {
        m_pAltNameED->SetText(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_pAltNameED->SaveValue();
    }

    if(!m_bFormat)
    {
        // insert graphic - properties
        // bNew is not set, so recognise by selection
        OUString aTmpName1;
        if(SfxItemState::SET == rSet->GetItemState(FN_SET_FRM_NAME, false, &pItem))
        {
            aTmpName1 = static_cast<const SfxStringItem*>(pItem)->GetValue();
        }

        OSL_ENSURE(m_pWrtSh, "keine Shell?");
        if( m_bNew || aTmpName1.isEmpty() )
        {
            if (m_sDlgType == "PictureDialog")
                aTmpName1 = m_pWrtSh->GetUniqueGrfName();
            else if (m_sDlgType == "ObjectDialog")
                aTmpName1 = m_pWrtSh->GetUniqueOLEName();
            else
                aTmpName1 = m_pWrtSh->GetUniqueFrameName();

            m_pWrtSh->SetFlyName(aTmpName1);
        }

        m_pNameED->SetText( aTmpName1 );
        m_pNameED->SaveValue();
    }
    else
    {
        m_pNameED->Enable( false );
        m_pAltNameED->Enable(false);
        m_pNameFT->Enable( false );
        m_pAltNameFT->Enable(false);
    }
    if (m_sDlgType == "FrameDialog" && m_pAltNameFT->IsVisible())
    {
        m_pAltNameFT->Hide();
        m_pAltNameED->Hide();
    }
    else
    {
        m_pNameED->SetModifyHdl(LINK(this, SwFrameAddPage, EditModifyHdl));
    }

    if (!m_bNew)
    {
        SwFrameFormat* pFormat = m_pWrtSh->GetFlyFrameFormat();

        if (pFormat)
        {
            const SwFormatChain &rChain = pFormat->GetChain();
            const SwFlyFrameFormat* pFlyFormat;
            OUString sNextChain, sPrevChain;
            if ((pFlyFormat = rChain.GetPrev()) != nullptr)
            {
                sPrevChain = pFlyFormat->GetName();
            }

            if ((pFlyFormat = rChain.GetNext()) != nullptr)
            {
                sNextChain = pFlyFormat->GetName();
            }
            //determine chainable frames
            ::std::vector< OUString > aPrevPageFrames;
            ::std::vector< OUString > aThisPageFrames;
            ::std::vector< OUString > aNextPageFrames;
            ::std::vector< OUString > aRemainFrames;
            m_pWrtSh->GetConnectableFrameFormats(*pFormat, sNextChain, false,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            lcl_InsertVectors(*m_pPrevLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(!sPrevChain.isEmpty())
            {
                if(LISTBOX_ENTRY_NOTFOUND == m_pPrevLB->GetEntryPos(sPrevChain))
                    m_pPrevLB->InsertEntry(sPrevChain, 1);
                m_pPrevLB->SelectEntry(sPrevChain);
            }
            else
                m_pPrevLB->SelectEntryPos(0);
            aPrevPageFrames.erase(aPrevPageFrames.begin(), aPrevPageFrames.end());
            aNextPageFrames.erase(aNextPageFrames.begin(), aNextPageFrames.end());
            aThisPageFrames.erase(aThisPageFrames.begin(), aThisPageFrames.end());
            aRemainFrames.erase(aRemainFrames.begin(), aRemainFrames.end());

            m_pWrtSh->GetConnectableFrameFormats(*pFormat, sPrevChain, true,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            lcl_InsertVectors(*m_pNextLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(!sNextChain.isEmpty())
            {
                if(LISTBOX_ENTRY_NOTFOUND == m_pNextLB->GetEntryPos(sNextChain))
                    m_pNextLB->InsertEntry(sNextChain, 1);
                m_pNextLB->SelectEntry(sNextChain);
            }
            else
                m_pNextLB->SelectEntryPos(0);
            Link<ListBox&,void> aLink(LINK(this, SwFrameAddPage, ChainModifyHdl));
            m_pPrevLB->SetSelectHdl(aLink);
            m_pNextLB->SetSelectHdl(aLink);
        }
    }
    // Pos Protected
    const SvxProtectItem& rProt = static_cast<const SvxProtectItem& >(rSet->Get(RES_PROTECT));
    m_pProtectFrameCB->Check(rProt.IsPosProtected());
    m_pProtectContentCB->Check(rProt.IsContentProtected());
    m_pProtectSizeCB->Check(rProt.IsSizeProtected());

    const SwFormatEditInReadonly& rEdit = static_cast<const SwFormatEditInReadonly& >(rSet->Get(RES_EDIT_IN_READONLY));
    m_pEditInReadonlyCB->Check(rEdit.GetValue());          m_pEditInReadonlyCB->SaveValue();

    // print
    const SvxPrintItem& rPrt = static_cast<const SvxPrintItem&>(rSet->Get(RES_PRINT));
    m_pPrintFrameCB->Check(rPrt.GetValue());               m_pPrintFrameCB->SaveValue();

    // textflow
    SfxItemState eState;
    if( (!m_bHtmlMode || (0 != (nHtmlMode&HTMLMODE_SOME_STYLES)))
            && m_sDlgType != "PictureDialog" && m_sDlgType != "ObjectDialog" &&
        SfxItemState::UNKNOWN != ( eState = rSet->GetItemState(
                                        RES_FRAMEDIR )) )
    {
        m_pTextFlowFT->Show();
        m_pTextFlowLB->Show();

        //vertical text flow is not possible in HTML
        if(m_bHtmlMode)
        {
            sal_uLong nData = FRMDIR_VERT_TOP_RIGHT;
            m_pTextFlowLB->RemoveEntry(m_pTextFlowLB->GetEntryPos(reinterpret_cast<void*>(nData)));
        }
        sal_uInt16 nVal = static_cast<const SvxFrameDirectionItem&>(rSet->Get(RES_FRAMEDIR)).GetValue();
        sal_Int32 nPos;
        for( nPos = m_pTextFlowLB->GetEntryCount(); nPos; )
            if( (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pTextFlowLB->GetEntryData( --nPos )) == nVal )
                break;
        m_pTextFlowLB->SelectEntryPos( nPos );
        m_pTextFlowLB->SaveValue();
    }
    else
    {
        m_pTextFlowFT->Hide();
        m_pTextFlowLB->Hide();
    }

    // Content alignment
    if ( rSet->GetItemState(RES_TEXT_VERT_ADJUST) > SfxItemState::DEFAULT )
    {
        SdrTextVertAdjust nAdjust = static_cast<const SdrTextVertAdjustItem&>(rSet->Get(RES_TEXT_VERT_ADJUST)).GetValue();
        sal_Int32 nPos = 0;
        switch(nAdjust)
        {
            case SDRTEXTVERTADJUST_TOP:      nPos = 0;   break;
            case SDRTEXTVERTADJUST_CENTER:
            case SDRTEXTVERTADJUST_BLOCK:    nPos = 1;   break;
            case SDRTEXTVERTADJUST_BOTTOM:   nPos = 2;   break;
        }
        m_pVertAlignLB->SelectEntryPos(nPos);
    }
    m_pVertAlignLB->SaveValue();
}

bool SwFrameAddPage::FillItemSet(SfxItemSet *rSet)
{
    bool bRet = false;
    if (m_pNameED->IsValueChangedFromSaved())
        bRet |= nullptr != rSet->Put(SfxStringItem(FN_SET_FRM_NAME, m_pNameED->GetText()));
    if (m_pAltNameED->IsValueChangedFromSaved())
        bRet |= nullptr != rSet->Put(SfxStringItem(FN_SET_FRM_ALT_NAME, m_pAltNameED->GetText()));

    const SfxPoolItem* pOldItem;
    SvxProtectItem aProt ( static_cast<const SvxProtectItem& >(GetItemSet().Get(RES_PROTECT)) );
    aProt.SetContentProtect( m_pProtectContentCB->IsChecked() );
    aProt.SetSizeProtect ( m_pProtectSizeCB->IsChecked() );
    aProt.SetPosProtect  ( m_pProtectFrameCB->IsChecked() );
    if ( nullptr == (pOldItem = GetOldItem(*rSet, FN_SET_PROTECT)) ||
                aProt != *pOldItem )
        bRet |= nullptr != rSet->Put( aProt);

    if ( m_pEditInReadonlyCB->IsValueChangedFromSaved() )
        bRet |= nullptr != rSet->Put( SwFormatEditInReadonly( RES_EDIT_IN_READONLY, m_pEditInReadonlyCB->IsChecked()));

    if ( m_pPrintFrameCB->IsValueChangedFromSaved() )
        bRet |= nullptr != rSet->Put( SvxPrintItem( RES_PRINT, m_pPrintFrameCB->IsChecked()));

    // textflow
    if( m_pTextFlowLB->IsVisible() )
    {
        sal_Int32 nPos = m_pTextFlowLB->GetSelectEntryPos();
        if( m_pTextFlowLB->IsValueChangedFromSaved() )
        {
            sal_uInt16 nData = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pTextFlowLB->GetEntryData( nPos ));
            bRet |= nullptr != rSet->Put( SvxFrameDirectionItem(
                                    (SvxFrameDirection)nData, RES_FRAMEDIR ));
        }
    }
    if(m_pWrtSh)
    {
        const SwFrameFormat* pFormat = m_pWrtSh->GetFlyFrameFormat();
        if (pFormat)
        {
            OUString sCurrentPrevChain, sCurrentNextChain;
            if(m_pPrevLB->GetSelectEntryPos())
                sCurrentPrevChain = m_pPrevLB->GetSelectEntry();
            if(m_pNextLB->GetSelectEntryPos())
                sCurrentNextChain = m_pNextLB->GetSelectEntry();
            const SwFormatChain &rChain = pFormat->GetChain();
            const SwFlyFrameFormat* pFlyFormat;
            OUString sNextChain, sPrevChain;
            if ((pFlyFormat = rChain.GetPrev()) != nullptr)
                sPrevChain = pFlyFormat->GetName();

            if ((pFlyFormat = rChain.GetNext()) != nullptr)
                sNextChain = pFlyFormat->GetName();
            if(sPrevChain != sCurrentPrevChain)
                bRet |= nullptr != rSet->Put(SfxStringItem(FN_PARAM_CHAIN_PREVIOUS, sCurrentPrevChain));
            if(sNextChain != sCurrentNextChain)
                bRet |= nullptr != rSet->Put(SfxStringItem(FN_PARAM_CHAIN_NEXT, sCurrentNextChain));
        }
    }

    if(m_pVertAlignLB->IsValueChangedFromSaved())
    {
        SdrTextVertAdjust nAdjust;
        switch(m_pVertAlignLB->GetSelectEntryPos())
        {
            default:
            case 0 : nAdjust = SDRTEXTVERTADJUST_TOP; break;
            case 1 : nAdjust = SDRTEXTVERTADJUST_CENTER; break;
            case 2 : nAdjust = SDRTEXTVERTADJUST_BOTTOM; break;
        }
        bRet |= nullptr != rSet->Put(SdrTextVertAdjustItem(nAdjust, RES_TEXT_VERT_ADJUST));
    }

    return bRet;
}

IMPL_LINK_NOARG_TYPED(SwFrameAddPage, EditModifyHdl, Edit&, void)
{
    bool bEnable = !m_pNameED->GetText().isEmpty();
    m_pAltNameED->Enable(bEnable);
    m_pAltNameFT->Enable(bEnable);
}

void SwFrameAddPage::SetFormatUsed(bool bFormatUsed)
{
    m_bFormat = bFormatUsed;
    if (m_bFormat)
    {
        m_pNameFrame->Hide();
    }
}

IMPL_LINK_TYPED(SwFrameAddPage, ChainModifyHdl, ListBox&, rBox, void)
{
    OUString sCurrentPrevChain, sCurrentNextChain;
    if(m_pPrevLB->GetSelectEntryPos())
        sCurrentPrevChain = m_pPrevLB->GetSelectEntry();
    if(m_pNextLB->GetSelectEntryPos())
        sCurrentNextChain = m_pNextLB->GetSelectEntry();
    SwFrameFormat* pFormat = m_pWrtSh->GetFlyFrameFormat();
    if (pFormat)
    {
        bool bNextBox = m_pNextLB == &rBox;
        ListBox& rChangeLB = bNextBox ? *m_pPrevLB : *m_pNextLB;
        for(sal_Int32 nEntry = rChangeLB.GetEntryCount(); nEntry > 1; nEntry--)
            rChangeLB.RemoveEntry(nEntry - 1);
        //determine chainable frames
        ::std::vector< OUString > aPrevPageFrames;
        ::std::vector< OUString > aThisPageFrames;
        ::std::vector< OUString > aNextPageFrames;
        ::std::vector< OUString > aRemainFrames;
        m_pWrtSh->GetConnectableFrameFormats(*pFormat, bNextBox ? sCurrentNextChain : sCurrentPrevChain, !bNextBox,
                        aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
        lcl_InsertVectors(rChangeLB,
                aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
        const OUString sToSelect = bNextBox ? sCurrentPrevChain : sCurrentNextChain;
        if(rChangeLB.GetEntryPos(sToSelect) != LISTBOX_ENTRY_NOTFOUND)
            rChangeLB.SelectEntry(sToSelect);
        else
            rChangeLB.SelectEntryPos(0);

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
