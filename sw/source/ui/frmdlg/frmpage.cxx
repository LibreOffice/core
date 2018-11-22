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
#include <hintids.hxx>
#include <bitmaps.hlst>
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
#include <tools/globname.hxx>
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

#include <strings.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <vcl/graphicfilter.hxx>
#include <vcl/builderfactory.hxx>
#include <svtools/embedhlp.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::sfx2;

#define SwFPos SvxSwFramePosString

struct StringIdPair_Impl
{
    SvxSwFramePosString::StringId const eHori;
    SvxSwFramePosString::StringId const eVert;
};

#define MAX_PERCENT_WIDTH   254
#define MAX_PERCENT_HEIGHT  254

enum class LB {
    NONE                = 0x00000000L,
    Frame               = 0x00000001L,  // text region of the paragraph
    PrintArea           = 0x00000002L,  // text region of the paragraph + indentions
    VertFrame           = 0x00000004L,  // vertical text region of the paragraph
    VertPrintArea       = 0x00000008L,  // vertical text region of the paragraph + indentions
    RelFrameLeft        = 0x00000010L,  // left paragraph edge
    RelFrameRight       = 0x00000020L,  // right paragraph edge

    RelPageLeft         = 0x00000040L,  // left page edge
    RelPageRight        = 0x00000080L,  // right page edge
    RelPageFrame        = 0x00000100L,  // whole page
    RelPagePrintArea    = 0x00000200L,  // text region of the page

    FlyRelPageLeft      = 0x00000400L,  // left frame edge
    FlyRelPageRight     = 0x00000800L,   // right frame edge
    FlyRelPageFrame     = 0x00001000L,  // whole frame
    FlyRelPagePrintArea = 0x00002000L,  // inside of the frame

    RelBase             = 0x00010000L,  // character alignment Base
    RelChar             = 0x00020000L,  // character alignment Character
    RelRow              = 0x00040000L,  // character alignment Row

    FlyVertFrame        = 0x00100000L,  // vertical entire frame
    FlyVertPrintArea    = 0x00200000L,  // vertical frame text area

    VertLine            = 0x00400000L,  // vertical text line
};
namespace o3tl {
    template<> struct typed_flags<LB> : is_typed_flags<LB, 0x00773fffL> {};
}

struct RelationMap
{
    SvxSwFramePosString::StringId const eStrId;
    SvxSwFramePosString::StringId const eMirrorStrId;
    LB const         nLBRelation;
    sal_Int16 const  nRelation;
};

struct FrameMap
{
    SvxSwFramePosString::StringId const eStrId;
    SvxSwFramePosString::StringId const eMirrorStrId;
    sal_Int16 const  nAlign;
    LB const         nLBRelations;
};


static RelationMap const aRelationMap[] =
{
    {SwFPos::FRAME,  SwFPos::FRAME, LB::Frame, text::RelOrientation::FRAME},
    {SwFPos::PRTAREA,           SwFPos::PRTAREA,                LB::PrintArea,             text::RelOrientation::PRINT_AREA},
    {SwFPos::REL_PG_LEFT,       SwFPos::MIR_REL_PG_LEFT,        LB::RelPageLeft,         text::RelOrientation::PAGE_LEFT},
    {SwFPos::REL_PG_RIGHT,      SwFPos::MIR_REL_PG_RIGHT,       LB::RelPageRight,        text::RelOrientation::PAGE_RIGHT},
    {SwFPos::REL_FRM_LEFT,      SwFPos::MIR_REL_FRM_LEFT,       LB::RelFrameLeft,        text::RelOrientation::FRAME_LEFT},
    {SwFPos::REL_FRM_RIGHT,     SwFPos::MIR_REL_FRM_RIGHT,      LB::RelFrameRight,       text::RelOrientation::FRAME_RIGHT},
    {SwFPos::REL_PG_FRAME,      SwFPos::REL_PG_FRAME,           LB::RelPageFrame,        text::RelOrientation::PAGE_FRAME},
    {SwFPos::REL_PG_PRTAREA,    SwFPos::REL_PG_PRTAREA,         LB::RelPagePrintArea,      text::RelOrientation::PAGE_PRINT_AREA},
    {SwFPos::REL_CHAR,          SwFPos::REL_CHAR,               LB::RelChar,            text::RelOrientation::CHAR},

    {SwFPos::FLY_REL_PG_LEFT,       SwFPos::FLY_MIR_REL_PG_LEFT,    LB::FlyRelPageLeft,     text::RelOrientation::PAGE_LEFT},
    {SwFPos::FLY_REL_PG_RIGHT,      SwFPos::FLY_MIR_REL_PG_RIGHT,   LB::FlyRelPageRight,    text::RelOrientation::PAGE_RIGHT},
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB::FlyRelPageFrame,    text::RelOrientation::PAGE_FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB::FlyRelPagePrintArea,  text::RelOrientation::PAGE_PRINT_AREA},

    {SwFPos::REL_BORDER,        SwFPos::REL_BORDER,             LB::VertFrame,          text::RelOrientation::FRAME},
    {SwFPos::REL_PRTAREA,       SwFPos::REL_PRTAREA,            LB::VertPrintArea,        text::RelOrientation::PRINT_AREA},

    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,   LB::FlyVertFrame,      text::RelOrientation::FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB::FlyVertPrintArea,    text::RelOrientation::PRINT_AREA},

    {SwFPos::REL_LINE,  SwFPos::REL_LINE,   LB::VertLine,   text::RelOrientation::TEXT_LINE}
};

static RelationMap const aAsCharRelationMap[] =
{
    {SwFPos::REL_BASE,  SwFPos::REL_BASE,   LB::RelBase,    text::RelOrientation::FRAME},
    {SwFPos::REL_CHAR,   SwFPos::REL_CHAR,   LB::RelChar,   text::RelOrientation::FRAME},
    {SwFPos::REL_ROW,    SwFPos::REL_ROW,   LB::RelRow,     text::RelOrientation::FRAME}
};

// site anchored
static constexpr auto HORI_PAGE_REL = LB::RelPageFrame | LB::RelPagePrintArea | LB::RelPageLeft |
                        LB::RelPageRight;

static FrameMap const aHPageMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrameMap const aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB::RelPageFrame}
};

#define VERT_PAGE_REL   (LB::RelPageFrame|LB::RelPagePrintArea)

static FrameMap const aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrameMap const aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB::RelPageFrame}
};

// frame anchored
static constexpr auto HORI_FRAME_REL = LB::FlyRelPageFrame | LB::FlyRelPagePrintArea |
                                       LB::FlyRelPageLeft | LB::FlyRelPageRight;

static FrameMap const aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrameMap const aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB::FlyRelPageFrame},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB::FlyRelPageFrame}
};

// own vertical alignment map for objects anchored to frame
#define VERT_FRAME_REL   (LB::FlyVertFrame|LB::FlyVertPrintArea)

static FrameMap const aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrameMap const aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB::FlyVertFrame},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      LB::FlyVertFrame}
};

// paragraph anchored
static constexpr auto HORI_PARA_REL = LB::Frame | LB::PrintArea | LB::RelPageLeft | LB::RelPageRight |
                        LB::RelPageFrame | LB::RelPagePrintArea | LB::RelFrameLeft |
                        LB::RelFrameRight;

static FrameMap const aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB::Frame|LB::PrintArea)

static FrameMap const aHParaHtmlMap[] =
{
    {SwFPos::LEFT,  SwFPos::LEFT,   text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT, SwFPos::RIGHT,  text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

static FrameMap const aHParaHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

// allow vertical alignment at page areas
static constexpr auto VERT_PARA_REL = LB::VertFrame | LB::VertPrintArea |
                                      LB::RelPageFrame | LB::RelPagePrintArea;

static FrameMap const aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,      VERT_PARA_REL}
};

static FrameMap const aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,       LB::VertPrintArea}
};

// anchored relative to the character
static constexpr auto HORI_CHAR_REL = LB::Frame|LB::PrintArea | LB::RelPageLeft | LB::RelPageRight |
                                      LB::RelPageFrame | LB::RelPagePrintArea | LB::RelFrameLeft |
                                      LB::RelFrameRight | LB::RelChar;

static FrameMap const aHCharMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     HORI_CHAR_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    text::HoriOrientation::CENTER,    HORI_CHAR_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      HORI_CHAR_REL}
};

#define HTML_HORI_CHAR_REL  (LB::Frame|LB::PrintArea|LB::RelChar)

static FrameMap const aHCharHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::LEFT,           text::HoriOrientation::LEFT,      HTML_HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::RIGHT,          text::HoriOrientation::RIGHT,     HTML_HORI_CHAR_REL}
};

static FrameMap const aHCharHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       text::HoriOrientation::LEFT,      LB::PrintArea|LB::RelChar},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      text::HoriOrientation::RIGHT,     LB::PrintArea},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   text::HoriOrientation::NONE,      LB::RelPageFrame}
};

// allow vertical alignment at page areas
static constexpr auto VERT_CHAR_REL = LB::VertFrame | LB::VertPrintArea |
                                      LB::RelPageFrame | LB::RelPagePrintArea;

static FrameMap const aVCharMap[] =
{
    // introduce mappings for new vertical alignment at top of line <LB::VertLine>
    // and correct mapping for vertical alignment at character for position <FROM_BOTTOM>
    // Note: Because of these adjustments the map becomes ambiguous in its values
    //       <eStrId>/<eMirrorStrId> and <nAlign>. These ambiguities are considered
    //       in the methods <SwFramePage::FillRelLB(..)>, <SwFramePage::GetAlignment(..)>
    //       and <SwFramePage::FillPosLB(..)>
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,           VERT_CHAR_REL|LB::RelChar},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,        VERT_CHAR_REL|LB::RelChar},
    {SwFPos::BELOW,         SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB::RelChar},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        VERT_CHAR_REL|LB::RelChar},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        text::VertOrientation::NONE,          VERT_CHAR_REL},
    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     text::VertOrientation::NONE,          LB::RelChar|LB::VertLine},
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB::VertLine},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB::VertLine},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB::VertLine}
};

static FrameMap const aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};

static FrameMap const aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            text::VertOrientation::TOP,           LB::RelChar},
    {SwFPos::BELOW,             SwFPos::BELOW,          text::VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};

// anchored as character
static FrameMap const aVAsCharMap[] =
{
    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::TOP,           LB::RelBase},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::BOTTOM,        LB::RelBase},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        LB::RelBase},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::CHAR_TOP,      LB::RelChar},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::CHAR_BOTTOM,   LB::RelChar},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CHAR_CENTER,   LB::RelChar},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB::RelRow},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB::RelRow},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB::RelRow},

    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     text::VertOrientation::NONE,          LB::RelBase}
};

static FrameMap const aVAsCharHtmlMap[] =
{
    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::TOP,           LB::RelBase},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::CENTER,        LB::RelBase},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::CHAR_TOP,      LB::RelChar},

    {SwFPos::TOP,               SwFPos::TOP,            text::VertOrientation::LINE_TOP,      LB::RelRow},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         text::VertOrientation::LINE_BOTTOM,   LB::RelRow},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    text::VertOrientation::LINE_CENTER,   LB::RelRow}
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
    FN_UNO_DESCRIPTION,     FN_UNO_DESCRIPTION,
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

static void lcl_InsertVectors(weld::ComboBox& rBox,
    const std::vector< OUString >& rPrev, const std::vector< OUString >& rThis,
    const std::vector< OUString >& rNext, const std::vector< OUString >& rRemain)
{
    for(const auto& rItem : rPrev)
        rBox.append_text(rItem);
    for(const auto& rItem : rThis)
        rBox.append_text(rItem);
    for(const auto& rItem : rNext)
        rBox.append_text(rItem);
    rBox.append_separator();
    //now insert all strings sorted
    const auto nStartPos = rBox.get_count();

    for(const auto& rItem : rPrev)
        ::InsertStringSorted("", rItem, rBox, nStartPos );
    for(const auto& rItem : rThis)
        ::InsertStringSorted("", rItem, rBox, nStartPos );
    for(const auto& rItem : rNext)
        ::InsertStringSorted("", rItem, rBox, nStartPos );
    for(const auto& rItem : rRemain)
        ::InsertStringSorted("", rItem, rBox, nStartPos );
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
        for(const StringIdPair_Impl & rHoriId : aHoriIds)
        {
            if(rHoriId.eHori == eStringId)
            {
                eStringId = rHoriId.eVert;
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
static LB lcl_GetLBRelationsForRelations( const sal_Int16 _nRel )
{
    LB nLBRelations = LB::NONE;

    for (RelationMap const & i : aRelationMap)
    {
        if ( i.nRelation == _nRel )
        {
            nLBRelations |= i.nLBRelation;
        }
    }

    return nLBRelations;
}

// helper method on order to determine all possible
// listbox relations in a relation map for a given string ID
static LB lcl_GetLBRelationsForStrID( const FrameMap* _pMap,
                                             const SvxSwFramePosString::StringId _eStrId,
                                             const bool _bUseMirrorStr )
{
    LB nLBRelations = LB::NONE;

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
        FrameMap const * pMap;
        size_t nCount;
    };
}

void SwFramePage::setOptimalFrameWidth()
{
    static FrameMaps const aMaps[] = {
        { aHPageMap, SAL_N_ELEMENTS(aHPageMap) },
        { aHPageHtmlMap, SAL_N_ELEMENTS(aHPageHtmlMap) },
        { aVPageMap, SAL_N_ELEMENTS(aVPageMap) },
        { aVPageHtmlMap, SAL_N_ELEMENTS(aVPageHtmlMap) },
        { aHFrameMap, SAL_N_ELEMENTS(aHFrameMap) },
        { aHFlyHtmlMap, SAL_N_ELEMENTS(aHFlyHtmlMap) },
        { aVFrameMap, SAL_N_ELEMENTS(aVFrameMap) },
        { aVFlyHtmlMap, SAL_N_ELEMENTS(aVFlyHtmlMap) },
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
    for (const FrameMaps & rMap : aMaps)
    {
        for (size_t j = 0; j < rMap.nCount; ++j)
        {
            aFrames.push_back(rMap.pMap[j].eStrId);
            aFrames.push_back(rMap.pMap[j].eMirrorStrId);
        }
    }

    std::sort(aFrames.begin(), aFrames.end());
    aFrames.erase(std::unique(aFrames.begin(), aFrames.end()), aFrames.end());

    for (const auto& rFrame : aFrames)
    {
        m_pHorizontalDLB->InsertEntry(SvxSwFramePosString::GetString(rFrame));
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
        RelationMap const * pMap;
        size_t nCount;
    };
}

void SwFramePage::setOptimalRelWidth()
{
    static const RelationMaps aMaps[] = {
        { aRelationMap, SAL_N_ELEMENTS(aRelationMap) },
        { aAsCharRelationMap, SAL_N_ELEMENTS(aAsCharRelationMap) }
    };

    std::vector<SvxSwFramePosString::StringId> aRels;
    for (const RelationMaps & rMap : aMaps)
    {
        for (size_t j = 0; j < rMap.nCount; ++j)
        {
            aRels.push_back(rMap.pMap[j].eStrId);
            aRels.push_back(rMap.pMap[j].eMirrorStrId);
        }
    }

    std::sort(aRels.begin(), aRels.end());
    aRels.erase(std::unique(aRels.begin(), aRels.end()), aRels.end());

    for (const auto& rRel : aRels)
    {
        m_pHoriRelationLB->InsertEntry(SvxSwFramePosString::GetString(rRel));
    }

    Size aBiggest(m_pHoriRelationLB->GetOptimalSize());
    m_pHoriRelationLB->set_width_request(aBiggest.Width());
    m_pVertRelationLB->set_width_request(aBiggest.Width());
    m_pRelWidthRelationLB->set_width_request(aBiggest.Width());
    m_pRelHeightRelationLB->set_width_request(aBiggest.Width());
    m_pHoriRelationLB->Clear();
}

VclPtr<SfxTabPage> SwFramePage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFramePage>::Create( pParent.pParent, *rSet );
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
    const SwFormatAnchor& rAnchor = rSet->Get(RES_ANCHOR);

    if (SfxItemState::SET == rSet->GetItemState(FN_OLE_IS_MATH, false, &pItem))
        m_bIsMathOLE = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    if (SfxItemState::SET == rSet->GetItemState(FN_MATH_BASELINE_ALIGNMENT, false, &pItem))
        m_bIsMathBaselineAlignment = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()) );

    if (m_bFormat)
    {
        // at formats no anchor editing
        m_pAnchorFrame->Enable(false);
        m_pFixedRatioCB->Enable(false);
    }
    else
    {
        if (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_FLY && !pSh->IsFlyInFly())
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
                SetText(SwResId(STR_FRMUI_OLE_INSERT));
            else
                SetText(SwResId(STR_FRMUI_OLE_EDIT));
        }
    }
    else
    {
        m_aGrfSize = rSet->Get(RES_FRM_SIZE).GetSize();
    }

    // entering procent value made possible
    m_aWidthED.SetBaseValue( m_aWidthED.Normalize(m_aGrfSize.Width()), FieldUnit::TWIP );
    m_aHeightED.SetBaseValue( m_aHeightED.Normalize(m_aGrfSize.Height()), FieldUnit::TWIP );
    //the available space is not yet known so the RefValue has to be calculated from size and relative size values
    //this is needed only if relative values are already set

    const SwFormatFrameSize& rFrameSize = rSet->Get(RES_FRM_SIZE);

    m_pRelWidthRelationLB->InsertEntry(SvxSwFramePosString::GetString(SwFPos::FRAME));
    m_pRelWidthRelationLB->InsertEntry(SvxSwFramePosString::GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetWidthPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetWidthPercent() != 0)
    {
        //calculate the reference value from the with and relative width values
        sal_Int32 nSpace = rFrameSize.GetWidth() * 100 / rFrameSize.GetWidthPercent();
        m_aWidthED.SetRefValue( nSpace );

        m_pRelWidthRelationLB->Enable();
    }
    else
        m_pRelWidthRelationLB->Disable();

    m_pRelHeightRelationLB->InsertEntry(SvxSwFramePosString::GetString(SwFPos::FRAME));
    m_pRelHeightRelationLB->InsertEntry(SvxSwFramePosString::GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetHeightPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetHeightPercent() != 0)
    {
        //calculate the reference value from the with and relative width values
        sal_Int32 nSpace = rFrameSize.GetHeight() * 100 / rFrameSize.GetHeightPercent();
        m_aHeightED.SetRefValue( nSpace );

        m_pRelHeightRelationLB->Enable();
    }
    else
        m_pRelHeightRelationLB->Disable();

    // general initialisation part
    switch(rAnchor.GetAnchorId())
    {
        case RndStdIds::FLY_AT_PAGE: m_pAnchorAtPageRB->Check(); break;
        case RndStdIds::FLY_AT_PARA: m_pAnchorAtParaRB->Check(); break;
        case RndStdIds::FLY_AT_CHAR: m_pAnchorAtCharRB->Check(); break;
        case RndStdIds::FLY_AS_CHAR: m_pAnchorAsCharRB->Check(); break;
        case RndStdIds::FLY_AT_FLY: m_pAnchorAtFrameRB->Check();break;
        default:; //prevent warning
    }

    // i#22341 - determine content position of character
    // Note: content position can be NULL
    mpToCharContentPos = rAnchor.GetContentAnchor();

    // i#18732 - init checkbox value
    {
        const bool bFollowTextFlow =
            rSet->Get(RES_FOLLOW_TEXT_FLOW).GetValue();
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

    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FieldUnit::TWIP)));
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

    RndStdIds eAnchorId = GetAnchor();

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
        SwFormatHoriOrient aHoriOrient( rOldSet.Get(RES_HORI_ORIENT) );

        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        const sal_Int16 eHOri = GetAlignment(m_pHMap, nMapPos, *m_pHoriRelationLB);
        const sal_Int16 eRel = GetRelation(*m_pHoriRelationLB);

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(m_pMirrorPagesCB->IsChecked());

        bool bMod = m_pAtHorzPosED->IsValueChangedFromSaved();
        bMod |= m_pMirrorPagesCB->IsValueChangedFromSaved();

        if ( eHOri == text::HoriOrientation::NONE &&
             (m_bNew || (m_bAtHorzPosModified || bMod) || m_nOldH != eHOri ) )
        {
            SwTwips nX = static_cast< SwTwips >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FieldUnit::TWIP)));
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
        SwFormatVertOrient aVertOrient( rOldSet.Get(RES_VERT_ORIENT) );

        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        const sal_Int16 eVOri = GetAlignment(m_pVMap, nMapPos, *m_pVertRelationLB);
        const sal_Int16 eRel = GetRelation(*m_pVertRelationLB);

        aVertOrient.SetVertOrient    ( eVOri);
        aVertOrient.SetRelationOrient( eRel );

        bool bMod = m_pAtVertPosED->IsValueChangedFromSaved();

        if ( eVOri == text::VertOrientation::NONE &&
             ( m_bNew || (m_bAtVertPosModified || bMod) || m_nOldV != eVOri) )
        {
            // vertical position
            // recalculate offset for character bound frames
            SwTwips nY = static_cast< SwTwips >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FieldUnit::TWIP)));
            if (eAnchorId == RndStdIds::FLY_AS_CHAR)
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
    const SwFormatFrameSize& rOldSize = rOldSet.Get(RES_FRM_SIZE);
    SwFormatFrameSize aSz( rOldSize );

    sal_Int32 nRelWidthRelation = m_pRelWidthRelationLB->GetSelectedEntryPos();
    if (nRelWidthRelation != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nRelWidthRelation == 0)
            aSz.SetWidthPercentRelation(text::RelOrientation::FRAME);
        else if (nRelWidthRelation == 1)
            aSz.SetWidthPercentRelation(text::RelOrientation::PAGE_FRAME);
    }
    sal_Int32 nRelHeightRelation = m_pRelHeightRelationLB->GetSelectedEntryPos();
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
        sal_Int64 nNewWidth  = m_aWidthED.DenormalizePercent(m_aWidthED.GetRealValue(FieldUnit::TWIP));
        sal_Int64 nNewHeight = m_aHeightED.DenormalizePercent(m_aHeightED.GetRealValue(FieldUnit::TWIP));
        aSz.SetWidth (static_cast< SwTwips >(nNewWidth));
        aSz.SetHeight(static_cast< SwTwips >(nNewHeight));

        if (m_pRelWidthCB->IsChecked())
        {
            aSz.SetWidthPercent(static_cast<sal_uInt8>(std::min( static_cast< sal_Int64 >(MAX_PERCENT_WIDTH), m_aWidthED.Convert(m_aWidthED.NormalizePercent(nNewWidth), FieldUnit::TWIP, FieldUnit::CUSTOM))));
        }
        else
            aSz.SetWidthPercent(0);
        if (m_pRelHeightCB->IsChecked())
            aSz.SetHeightPercent(static_cast<sal_uInt8>(std::min(static_cast< sal_Int64 >(MAX_PERCENT_HEIGHT), m_aHeightED.Convert(m_aHeightED.NormalizePercent(nNewHeight), FieldUnit::TWIP, FieldUnit::CUSTOM))));
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

// initialise horizontal and vertical Pos
void SwFramePage::InitPos(RndStdIds eId,
                                sal_Int16 nH,
                                sal_Int16 nHRel,
                                sal_Int16 nV,
                                sal_Int16 nVRel,
                                long   nX,
                                long   nY)
{
    sal_Int32 nPos = m_pVerticalDLB->GetSelectedEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pVMap )
    {
        m_nOldV    = m_pVMap[nPos].nAlign;

        nPos = m_pVertRelationLB->GetSelectedEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldVRel = static_cast<RelationMap *>(m_pVertRelationLB->GetEntryData(nPos))->nRelation;
    }

    nPos = m_pHorizontalDLB->GetSelectedEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pHMap )
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_pHoriRelationLB->GetSelectedEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldHRel = static_cast<RelationMap *>(m_pHoriRelationLB->GetEntryData(nPos))->nRelation;
    }

    bool bEnable = true;
    if ( eId == RndStdIds::FLY_AT_PAGE )
    {
        m_pVMap = m_bHtmlMode ? aVPageHtmlMap : aVPageMap;
        m_pHMap = m_bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if ( eId == RndStdIds::FLY_AT_FLY )
    {
        // own vertical alignment map for to frame
        // anchored objects.
        m_pVMap = m_bHtmlMode ? aVFlyHtmlMap : aVFrameMap;
        m_pHMap = m_bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if ( eId == RndStdIds::FLY_AT_PARA )
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
    else if ( eId == RndStdIds::FLY_AT_CHAR )
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
    else if ( eId == RndStdIds::FLY_AS_CHAR )
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

    bEnable = nH == text::HoriOrientation::NONE && eId != RndStdIds::FLY_AS_CHAR;
    if (!bEnable)
    {
        m_pAtHorzPosED->SetValue( 0, FieldUnit::TWIP );
        if (nX != LONG_MAX && m_bHtmlMode)
            m_pAtHorzPosED->SetModifyFlag();
    }
    else
    {
        if (nX != LONG_MAX)
            m_pAtHorzPosED->SetValue( m_pAtHorzPosED->Normalize(nX), FieldUnit::TWIP );
    }
    m_pAtHorzPosFT->Enable( bEnable );
    m_pAtHorzPosED->Enable( bEnable );

    bEnable = nV == text::VertOrientation::NONE;
    if ( !bEnable )
    {
        m_pAtVertPosED->SetValue( 0, FieldUnit::TWIP );
        if(nY != LONG_MAX && m_bHtmlMode)
            m_pAtVertPosED->SetModifyFlag();
    }
    else
    {
        if ( eId == RndStdIds::FLY_AS_CHAR )
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            m_pAtVertPosED->SetValue( m_pAtVertPosED->Normalize(nY), FieldUnit::TWIP );
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
    const OUString sOldEntry = _rLB.GetSelectedEntry();

    _rLB.Clear();

    // i#22341 determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const LB nLBRelations = (_pMap != aVCharMap)
                               ? LB::NONE
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill Listbox
    size_t nCount = ::lcl_GetFrameMapCount(_pMap);
    for (size_t i = 0; _pMap && i < nCount; ++i)
    {
//      Why not from the left/from inside or from above?
        SvxSwFramePosString::StringId eStrId = m_pMirrorPagesCB->IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
        // --> OD 2009-08-31 #mongolianlayout#
        eStrId = lcl_ChangeResIdToVerticalOrRTL( eStrId,
                                                 m_bIsVerticalFrame,
                                                 m_bIsVerticalL2R,
                                                 m_bIsInRightToLeft);
        OUString sEntry(SvxSwFramePosString::GetString(eStrId));
        if (_rLB.GetEntryPos(sEntry) == LISTBOX_ENTRY_NOTFOUND)
        {
            // don't insert entries when frames are character bound
            _rLB.InsertEntry(sEntry);
        }
        // i#22341 - add condition to handle map <aVCharMap>
        // that is ambiguous in the alignment.
        if ( _pMap[i].nAlign == _nAlign &&
             ( (_pMap != aVCharMap) || _pMap[i].nLBRelations & nLBRelations ) )
        {
            sSelEntry = sEntry;
        }
    }

    _rLB.SelectEntry(sSelEntry);
    if (!_rLB.GetSelectedEntryCount())
        _rLB.SelectEntry(sOldEntry);

    if (!_rLB.GetSelectedEntryCount())
        _rLB.SelectEntryPos(0);

    PosHdl(_rLB);

    return GetMapPos(_pMap, _rLB);
}

void SwFramePage::FillRelLB( const FrameMap* _pMap,
                            const sal_uInt16 _nLBSelPos,
                            const sal_Int16 _nAlign,
                            const sal_Int16 _nRel,
                            ListBox& _rLB,
                            FixedText& _rFT )
{
    OUString sSelEntry;
    LB       nLBRelations = LB::NONE;
    size_t   nMapCount = ::lcl_GetFrameMapCount(_pMap);

    _rLB.Clear();

    if (_nLBSelPos < nMapCount)
    {
        if (_pMap == aVAsCharHtmlMap || _pMap == aVAsCharMap)
        {
            const OUString sOldEntry(_rLB.GetSelectedEntry());
            SvxSwFramePosString::StringId eStrId = _pMap[_nLBSelPos].eStrId;

            for (size_t nMapPos = 0; nMapPos < nMapCount; nMapPos++)
            {
                if (_pMap[nMapPos].eStrId == eStrId)
                {
                    nLBRelations = _pMap[nMapPos].nLBRelations;
                    for (RelationMap const & rCharMap : aAsCharRelationMap)
                    {
                        if (nLBRelations & rCharMap.nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = rCharMap.eStrId;

                            // --> OD 2009-08-31 #mongolianlayout#
                            sStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( sStrId1,
                                                                m_bIsVerticalFrame,
                                                                m_bIsVerticalL2R,
                                                                m_bIsInRightToLeft);
                            const OUString sEntry = SvxSwFramePosString::GetString(sStrId1);
                            sal_Int32 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, const_cast<RelationMap*>(&rCharMap));
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

                if (!_rLB.GetSelectedEntryCount())
                {
                    for (sal_Int32 i = 0; i < _rLB.GetEntryCount(); i++)
                    {
                        RelationMap *pEntry = static_cast<RelationMap *>(_rLB.GetEntryData(i));
                        if (pEntry->nLBRelation == LB::RelChar) // default
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
            // because its ambiguous in its <eStrId>/<eMirrorStrId>.
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
                if (nLBRelations & static_cast<LB>(nBit))
                {
                    for (RelationMap const & rMap : aRelationMap)
                    {
                        if (rMap.nLBRelation == static_cast<LB>(nBit))
                        {
                            SvxSwFramePosString::StringId eStrId1 = m_pMirrorPagesCB->IsChecked() ?
                                            rMap.eMirrorStrId : rMap.eStrId;
                            // --> OD 2009-08-31 #mongolianlayout#
                            eStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( eStrId1,
                                                                m_bIsVerticalFrame,
                                                                m_bIsVerticalL2R,
                                                                m_bIsInRightToLeft);
                            const OUString sEntry = SvxSwFramePosString::GetString(eStrId1);
                            sal_Int32 nPos = _rLB.InsertEntry(sEntry);
                            _rLB.SetEntryData(nPos, const_cast<RelationMap*>(&rMap));
                            if (sSelEntry.isEmpty() && rMap.nRelation == _nRel)
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

                if (!_rLB.GetSelectedEntryCount())
                    _rLB.SelectEntryPos(0);
            }
        }
    }

    const bool bEnable = _rLB.GetEntryCount() != 0
            && (&_rLB != m_pVertRelationLB || m_bAllowVertPositioning);
    _rLB.Enable( bEnable );
    _rFT.Enable( bEnable );

    RelHdl(_rLB);
}

sal_Int16 SwFramePage::GetRelation(ListBox const &rRelationLB)
{
    const sal_Int32 nPos = rRelationLB.GetSelectedEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        RelationMap *pEntry = static_cast<RelationMap *>(rRelationLB.GetEntryData(nPos));
        return pEntry->nRelation;
    }

    return 0;
}

sal_Int16 SwFramePage::GetAlignment(FrameMap const *pMap, sal_Int32 nMapPos,
        ListBox const &rRelationLB)
{
    if (!pMap || nMapPos < 0)
        return 0;

    const size_t nMapCount = ::lcl_GetFrameMapCount(pMap);

    if (static_cast<size_t>(nMapPos) >= nMapCount)
        return 0;

    // i#22341 special handling also for map <aVCharMap>,
    // because it contains ambiguous items for alignment
    if ( pMap != aVAsCharHtmlMap && pMap != aVAsCharMap && pMap != aVCharMap )
        return pMap[nMapPos].nAlign;

    if (rRelationLB.GetSelectedEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        return 0;

    const RelationMap *const pRelationMap = static_cast<const RelationMap *>(
        rRelationLB.GetSelectedEntryData());
    const LB nRel = pRelationMap->nLBRelation;
    const SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

    for (size_t i = 0; i < nMapCount; ++i)
    {
        if (pMap[i].eStrId == eStrId && (pMap[i].nLBRelations & nRel))
            return pMap[i].nAlign;
    }

    return 0;
}

sal_Int32 SwFramePage::GetMapPos( const FrameMap *pMap, ListBox const &rAlignLB )
{
    sal_Int32 nMapPos = 0;
    sal_Int32 nLBSelPos = rAlignLB.GetSelectedEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            const size_t nMapCount = ::lcl_GetFrameMapCount(pMap);
            const OUString sSelEntry(rAlignLB.GetSelectedEntry());

            for (size_t i = 0; i < nMapCount; i++)
            {
                SvxSwFramePosString::StringId eResId = pMap[i].eStrId;

                OUString sEntry = SvxSwFramePosString::GetString(eResId);
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
    RndStdIds nRet = RndStdIds::FLY_AT_PAGE;
    if(m_pAnchorAtParaRB->IsChecked())
    {
        nRet = RndStdIds::FLY_AT_PARA;
    }
    else if(m_pAnchorAtCharRB->IsChecked())
    {
        nRet = RndStdIds::FLY_AT_CHAR;
    }
    else if(m_pAnchorAsCharRB->IsChecked())
    {
        nRet = RndStdIds::FLY_AS_CHAR;
    }
    else if(m_pAnchorAtFrameRB->IsChecked())
    {
        nRet = RndStdIds::FLY_AT_FLY;
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

DeactivateRC SwFramePage::DeactivatePage(SfxItemSet * _pSet)
{
    if ( _pSet )
    {
        FillItemSet( _pSet );

        if (!m_bFormat) // tdf#112574 no anchor in styles
        {
            //FillItemSet doesn't set the anchor into the set when it matches
            //the original. But for the other pages we need the current anchor.
            SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell()
                                : getFrameDlgParentShell();
            RndStdIds eAnchorId = GetAnchor();
            SwFormatAnchor aAnc( eAnchorId, pSh->GetPhyPageNum() );
            _pSet->Put( aAnc );
        }
    }

    return DeactivateRC::LeavePage;
}

// swap left/right with inside/outside
IMPL_LINK_NOARG(SwFramePage, MirrorHdl, Button*, void)
{
    RndStdIds eId = GetAnchor();
    InitPos( eId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
}

IMPL_LINK( SwFramePage, RelSizeClickHdl, Button *, p, void )
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
IMPL_LINK_NOARG(SwFramePage, RangeModifyClickHdl, Button*, void)
{
    RangeModifyHdl();
}
IMPL_LINK_NOARG(SwFramePage, RangeModifyLoseFocusHdl, Control&, void)
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
    SwFlyFrameAttrMgr aMgr( m_bNew, pSh, GetItemSet() );
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = GetAnchor();
    aVal.bAutoHeight = m_pAutoHeightCB->IsChecked();
    aVal.bMirror = m_pMirrorPagesCB->IsChecked();
    aVal.bFollowTextFlow = m_pFollowTextFlowCB->IsChecked();

    if ( m_pHMap )
    {
        // alignment horizontal
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        aVal.nHoriOrient = GetAlignment(m_pHMap, nMapPos, *m_pHoriRelationLB);
        aVal.nHRelOrient = GetRelation(*m_pHoriRelationLB);
    }
    else
        aVal.nHoriOrient = text::HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // alignment vertical
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        aVal.nVertOrient = GetAlignment(m_pVMap, nMapPos, *m_pVertRelationLB);
        aVal.nVRelOrient = GetRelation(*m_pVertRelationLB);
    }
    else
        aVal.nVertOrient = text::VertOrientation::NONE;

    const long nAtHorzPosVal = static_cast< long >(
                    m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FieldUnit::TWIP)) );
    const long nAtVertPosVal = static_cast< long >(
                    m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FieldUnit::TWIP)) );

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, mpToCharContentPos, true);   // one time, to get reference values for percental values

    // set reference values for percental values (100%) ...
    m_aWidthED.SetRefValue(aVal.aPercentSize.Width());
    m_aHeightED.SetRefValue(aVal.aPercentSize.Height());

    // ... and correctly convert width and height with it
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED. DenormalizePercent(m_aWidthED.GetValue(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FieldUnit::TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    aMgr.ValidateMetrics(aVal, mpToCharContentPos);    // one more time, to determine all remaining values with correct width and height.

    // all columns have to be correct
    const SfxItemSet* pExampleSet = GetDialogExampleSet();
    if (pExampleSet && SfxItemState::DEFAULT <= pExampleSet->GetItemState(RES_COL))
    {
        const SwFormatCol& rCol = pExampleSet->Get(RES_COL);
        if ( rCol.GetColumns().size() > 1 )
        {
            for (const SwColumn & i : rCol.GetColumns())
            {
                aVal.nMinWidth += i.GetLeft() +
                                  i.GetRight() +
                                  MINFLY;
            }
            aVal.nMinWidth -= MINFLY;//one was already in there!
        }
    }

    nWidth = aVal.nWidth;
    nHeight = aVal.nHeight;

    // minimum range also for template
    m_aHeightED.SetMin(m_aHeightED.NormalizePercent(aVal.nMinHeight), FieldUnit::TWIP);
    m_aWidthED. SetMin(m_aWidthED.NormalizePercent(aVal.nMinWidth), FieldUnit::TWIP);

    SwTwips nMaxWidth(aVal.nMaxWidth);
    SwTwips nMaxHeight(aVal.nMaxHeight);

    if (aVal.bAutoHeight && (m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"))
    {
        SwTwips nTmp = std::min(nWidth * nMaxHeight / std::max(nHeight, 1L), nMaxHeight);
        m_aWidthED.SetMax(m_aWidthED.NormalizePercent(nTmp), FieldUnit::TWIP);

        nTmp = std::min(nHeight * nMaxWidth / std::max(nWidth, 1L), nMaxWidth);
        m_aHeightED.SetMax(m_aWidthED.NormalizePercent(nTmp), FieldUnit::TWIP);
    }
    else
    {
        SwTwips nTmp = static_cast< SwTwips >(m_aHeightED.NormalizePercent(nMaxHeight));
        m_aHeightED.SetMax(nTmp, FieldUnit::TWIP);

        nTmp = static_cast< SwTwips >(m_aWidthED.NormalizePercent(nMaxWidth));
        m_aWidthED.SetMax(nTmp, FieldUnit::TWIP);
    }

    m_pAtHorzPosED->SetMin(m_pAtHorzPosED->Normalize(aVal.nMinHPos), FieldUnit::TWIP);
    m_pAtHorzPosED->SetMax(m_pAtHorzPosED->Normalize(aVal.nMaxHPos), FieldUnit::TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        m_pAtHorzPosED->SetValue(m_pAtHorzPosED->Normalize(aVal.nHPos), FieldUnit::TWIP);

    const SwTwips nUpperOffset = (aVal.nAnchorType == RndStdIds::FLY_AS_CHAR)
        ? m_nUpperBorder : 0;
    const SwTwips nLowerOffset = (aVal.nAnchorType == RndStdIds::FLY_AS_CHAR)
        ? m_nLowerBorder : 0;

    m_pAtVertPosED->SetMin(m_pAtVertPosED->Normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset), FieldUnit::TWIP);
    m_pAtVertPosED->SetMax(m_pAtVertPosED->Normalize(aVal.nMaxVPos), FieldUnit::TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_pAtVertPosED->SetValue(m_pAtVertPosED->Normalize(aVal.nVPos), FieldUnit::TWIP);
}

IMPL_LINK_NOARG(SwFramePage, AnchorTypeHdl, Button*, void)
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
            && RndStdIds::FLY_AS_CHAR == eId) );
}

IMPL_LINK( SwFramePage, PosHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHorizontalDLB;
    ListBox *pRelLB = bHori ? m_pHoriRelationLB.get() : m_pVertRelationLB.get();
    FixedText *pRelFT = bHori ? m_pHoriRelationFT.get() : m_pVertRelationFT.get();
    FrameMap const *pMap = bHori ? m_pHMap : m_pVMap;

    const sal_Int32 nMapPos = GetMapPos(pMap, rLB);
    const sal_Int16 nAlign = GetAlignment(pMap, nMapPos, *pRelLB);

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
    if (rLB.GetSelectedEntryCount())
    {

        if (pRelLB->GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND)
            nRel = static_cast<RelationMap *>(pRelLB->GetSelectedEntryData())->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->Clear();

    UpdateExample();

    if (bHori)
        m_bAtHorzPosModified = true;
    else
        m_bAtVertPosModified = true;

    // special treatment for HTML-Mode with horizontal-vertical-dependencies
    if(m_bHtmlMode && (RndStdIds::FLY_AT_CHAR == GetAnchor()))
    {
        bool bSet = false;
        if(bHori)
        {
            // right is allowed only above - from the left only above
            // from the left at character -> below
            if((text::HoriOrientation::LEFT == nAlign || text::HoriOrientation::RIGHT == nAlign) &&
                    0 == m_pVerticalDLB->GetSelectedEntryPos())
            {
                if(text::RelOrientation::FRAME == nRel)
                    m_pVerticalDLB->SelectEntryPos(1);
                else
                    m_pVerticalDLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(text::HoriOrientation::LEFT == nAlign && 1 == m_pVerticalDLB->GetSelectedEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(text::HoriOrientation::NONE == nAlign && 1 == m_pVerticalDLB->GetSelectedEntryPos())
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
                if(1 == m_pHorizontalDLB->GetSelectedEntryPos())
                {
                    m_pHorizontalDLB->SelectEntryPos(0);
                    bSet = true;
                }
                m_pHoriRelationLB->SelectEntryPos(1);
            }
            else if(text::VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == m_pHorizontalDLB->GetSelectedEntryPos())
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
IMPL_LINK( SwFramePage, RelHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHoriRelationLB;

    UpdateExample();

    if (bHori)
        m_bAtHorzPosModified = true;
    else
        m_bAtVertPosModified = true;

    if (m_bHtmlMode && (RndStdIds::FLY_AT_CHAR == GetAnchor()))
    {
        if(bHori)
        {
            const sal_Int16 nRel = GetRelation(*m_pHoriRelationLB);
            if(text::RelOrientation::PRINT_AREA == nRel && 0 == m_pVerticalDLB->GetSelectedEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(1);
            }
            else if(text::RelOrientation::CHAR == nRel && 1 == m_pVerticalDLB->GetSelectedEntryPos())
            {
                m_pVerticalDLB->SelectEntryPos(0);
            }
        }
    }
    RangeModifyHdl();
}

IMPL_LINK_NOARG(SwFramePage, RealSizeHdl, Button*, void)
{
    m_aWidthED.SetUserValue( m_aWidthED. NormalizePercent(m_aGrfSize.Width() ), FieldUnit::TWIP);
    m_aHeightED.SetUserValue(m_aHeightED.NormalizePercent(m_aGrfSize.Height()), FieldUnit::TWIP);
    m_fWidthHeightRatio = m_aGrfSize.Height() ? double(m_aGrfSize.Width()) / double(m_aGrfSize.Height()) : 1.0;
    UpdateExample();
}

IMPL_LINK_NOARG(SwFramePage, AutoWidthClickHdl, Button*, void)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoWidthCB->IsChecked(), *m_pWidthFT, *m_pWidthAutoFT, *m_aWidthED.get() );
}

IMPL_LINK_NOARG(SwFramePage, AutoHeightClickHdl, Button*, void)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_pAutoHeightCB->IsChecked(), *m_pHeightFT, *m_pHeightAutoFT, *m_aWidthED.get() );
}

IMPL_LINK( SwFramePage, ModifyHdl, Edit&, rEdit, void )
{
    SwTwips nWidth  = static_cast< SwTwips >(m_aWidthED.DenormalizePercent(m_aWidthED.GetValue(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_aHeightED.DenormalizePercent(m_aHeightED.GetValue(FieldUnit::TWIP)));
    if ( m_pFixedRatioCB->IsChecked() )
    {
        if (&rEdit == m_aWidthED.get())
        {
            nHeight = SwTwips(static_cast<double>(nWidth) / m_fWidthHeightRatio);
            m_aHeightED.SetPrcntValue(m_aHeightED.NormalizePercent(nHeight), FieldUnit::TWIP);
        }
        else if (&rEdit == m_aHeightED.get())
        {
            nWidth = SwTwips(static_cast<double>(nHeight) * m_fWidthHeightRatio);
            m_aWidthED.SetPrcntValue(m_aWidthED.NormalizePercent(nWidth), FieldUnit::TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
}

void SwFramePage::UpdateExample()
{
    sal_Int32 nPos = m_pHorizontalDLB->GetSelectedEntryPos();
    if ( m_pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_pHorizontalDLB);
        m_pExampleWN->SetHAlign(GetAlignment(m_pHMap, nMapPos, *m_pHoriRelationLB));
        m_pExampleWN->SetHoriRel(GetRelation(*m_pHoriRelationLB));
    }

    nPos = m_pVerticalDLB->GetSelectedEntryPos();
    if ( m_pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_pVerticalDLB);
        m_pExampleWN->SetVAlign(GetAlignment(m_pVMap, nMapPos, *m_pVertRelationLB));
        m_pExampleWN->SetVertRel(GetRelation(*m_pVertRelationLB));
    }

    // size
    long nXPos = static_cast< long >(m_pAtHorzPosED->Denormalize(m_pAtHorzPosED->GetValue(FieldUnit::TWIP)));
    long nYPos = static_cast< long >(m_pAtVertPosED->Denormalize(m_pAtVertPosED->GetValue(FieldUnit::TWIP)));
    m_pExampleWN->SetRelPos(Point(nXPos, nYPos));

    m_pExampleWN->SetAnchor(GetAnchor());
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

            static struct GlobalNameId {
                sal_uInt32 const n1;
                sal_uInt16 n2, n3;
                sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
            } const aGlbNmIds[] = { { SO3_SM_CLASSID_60 }, { SO3_SM_CLASSID_50 },
                                    { SO3_SM_CLASSID_40 }, { SO3_SM_CLASSID_30 } };

            for (const GlobalNameId & rId : aGlbNmIds) {
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

    const SwFormatFrameSize& rSize = rSet.Get(RES_FRM_SIZE);
    sal_Int64 nWidth  = m_aWidthED.NormalizePercent(rSize.GetWidth());
    sal_Int64 nHeight = m_aHeightED.NormalizePercent(rSize.GetHeight());

    if (nWidth != m_aWidthED.GetValue(FieldUnit::TWIP))
    {
        if(!bReset)
        {
            // value was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            m_aWidthED.SetUserValue(nWidth, FieldUnit::TWIP);
        }
        else
            m_aWidthED.SetPrcntValue(nWidth, FieldUnit::TWIP);
    }

    if (nHeight != m_aHeightED.GetValue(FieldUnit::TWIP))
    {
        if (!bReset)
        {
            // values was changed by circulation-Tabpage and
            // has to be set with Modify-Flag
            m_aHeightED.SetUserValue(nHeight, FieldUnit::TWIP);
        }
        else
            m_aHeightED.SetPrcntValue(nHeight, FieldUnit::TWIP);
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
    const SvxULSpaceItem &rUL = rSet.Get(RES_UL_SPACE);
    m_nUpperBorder = rUL.GetUpper();
    m_nLowerBorder = rUL.GetLower();

    if(SfxItemState::SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        m_pFixedRatioCB->Check(static_cast<const SfxBoolItem&>(rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        m_pFixedRatioCB->SaveValue();
    }

    // columns
    SwFormatCol aCol( rSet.Get(RES_COL) );
    ::FitToActualSize( aCol, static_cast<sal_uInt16>(rSize.GetWidth()) );

    RndStdIds eAnchorId = GetAnchor();

    if ( m_bNew && !m_bFormat )
        InitPos(eAnchorId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
    else
    {
        const SwFormatHoriOrient& rHori = rSet.Get(RES_HORI_ORIENT);
        const SwFormatVertOrient& rVert = rSet.Get(RES_VERT_ORIENT);
        m_nOldH    = rHori.GetHoriOrient();
        m_nOldHRel = rHori.GetRelationOrient();
        m_nOldV    = rVert.GetVertOrient();
        m_nOldVRel = rVert.GetRelationOrient();

        if (eAnchorId == RndStdIds::FLY_AT_PAGE)
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
    const SwFormatSurround& rSurround = rSet.Get(RES_SURROUND);
    m_pExampleWN->SetWrap( rSurround.GetSurround() );

    if ( rSurround.GetSurround() == css::text::WrapTextMode_THROUGH )
    {
        const SvxOpaqueItem& rOpaque = rSet.Get(RES_OPAQUE);
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
        m_aWidthED.SetPrcntValue(rSize.GetWidthPercent(), FieldUnit::CUSTOM);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != SwFormatFrameSize::SYNCED &&
        !m_pRelHeightCB->IsChecked())
    {
        m_pRelHeightCB->Check();
        RelSizeClickHdl(m_pRelHeightCB);
        m_aHeightED.SetPrcntValue(rSize.GetHeightPercent(), FieldUnit::CUSTOM);
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

SwGrfExtPage::SwGrfExtPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/picturepage.ui", "PicturePage", &rSet)
    , m_bHtmlMode(false)
    , m_xMirror(m_xBuilder->weld_widget("flipframe"))
    , m_xMirrorVertBox(m_xBuilder->weld_check_button("vert"))
    , m_xMirrorHorzBox(m_xBuilder->weld_check_button("hori"))
    , m_xAllPagesRB(m_xBuilder->weld_radio_button("allpages"))
    , m_xLeftPagesRB(m_xBuilder->weld_radio_button("leftpages"))
    , m_xRightPagesRB(m_xBuilder->weld_radio_button("rightpages"))
    , m_xConnectED(m_xBuilder->weld_entry("entry"))
    , m_xBrowseBT(m_xBuilder->weld_button("browse"))
    , m_xLinkFrame(m_xBuilder->weld_frame("linkframe"))
    // RotGrfFlyFrame: Need Angle and RotateControls now
    , m_xFlAngle(m_xBuilder->weld_frame("FL_ANGLE"))
    , m_xNfAngle(m_xBuilder->weld_spin_button("NF_ANGLE"))
    , m_xCtlAngle(new weld::CustomWeld(*m_xBuilder, "CTL_ANGLE", m_aCtlAngle))
    , m_xBmpWin(new weld::CustomWeld(*m_xBuilder, "preview", m_aBmpWin))
{
    m_aBmpWin.SetBitmapEx(BitmapEx(RID_BMP_PREVIEW_FALLBACK));

    m_aCtlAngle.SetLinkedField(m_xNfAngle.get(), 2);

    SetExchangeSupport();
    m_xMirrorHorzBox->connect_toggled(LINK(this, SwGrfExtPage, MirrorHdl));
    m_xMirrorVertBox->connect_toggled(LINK(this, SwGrfExtPage, MirrorHdl));
    m_xBrowseBT->connect_clicked(LINK(this, SwGrfExtPage, BrowseHdl));
}

SwGrfExtPage::~SwGrfExtPage()
{
    disposeOnce();
}

void SwGrfExtPage::dispose()
{
    m_xBmpWin.reset();
    m_xCtlAngle.reset();
    m_xGrfDlg.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwGrfExtPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwGrfExtPage>::Create(pParent, *rSet);
}

void SwGrfExtPage::Reset(const SfxItemSet *rSet)
{
    const SfxPoolItem* pItem;
    const sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    m_bHtmlMode = (nHtmlMode & HTMLMODE_ON) != 0;

    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_GRF_CONNECT, true, &pItem)
        && static_cast<const SfxBoolItem *>(pItem)->GetValue() )
    {
        m_xBrowseBT->set_sensitive(true);
        m_xConnectED->set_editable(true);
    }

    // RotGrfFlyFrame: Get RotationAngle and set at control
    if(SfxItemState::SET == rSet->GetItemState( SID_ATTR_TRANSFORM_ANGLE, false, &pItem))
    {
        m_aCtlAngle.SetRotation(static_cast<const SfxInt32Item*>(pItem)->GetValue());
    }
    else
    {
        m_aCtlAngle.SetRotation(0);
    }
    m_aCtlAngle.SaveValue();

    ActivatePage(*rSet);
}

void SwGrfExtPage::ActivatePage(const SfxItemSet& rSet)
{
    const SvxProtectItem& rProt = rSet.Get(RES_PROTECT);
    bool bProtContent = rProt.IsContentProtected();

    const SfxPoolItem* pItem = nullptr;
    bool bEnable = false;
    bool bEnableMirrorRB = false;

    SfxItemState eState = rSet.GetItemState(RES_GRFATR_MIRRORGRF, true, &pItem);
    if (SfxItemState::UNKNOWN != eState && !bProtContent && !m_bHtmlMode)
    {
        if( SfxItemState::SET != eState )
            pItem = &rSet.Get( RES_GRFATR_MIRRORGRF );

        bEnable = true;

        MirrorGraph eMirror = static_cast<const SwMirrorGrf* >(pItem)->GetValue();
        switch( eMirror )
        {
        case MirrorGraph::Dont: break;
        case MirrorGraph::Vertical:    m_xMirrorHorzBox->set_active(true); break;
        case MirrorGraph::Horizontal:  m_xMirrorVertBox->set_active(true); break;
        case MirrorGraph::Both:        m_xMirrorHorzBox->set_active(true);
                                       m_xMirrorVertBox->set_active(true);
                                       break;
        default:
            ;
        }

        const int nPos = (static_cast<const SwMirrorGrf* >(pItem)->IsGrfToggle() ? 1 : 0)
            + ((eMirror == MirrorGraph::Vertical || eMirror == MirrorGraph::Both) ? 2 : 0);

        bEnableMirrorRB = nPos != 0;

        switch (nPos)
        {
            case 1: // mirror at left / even pages
                m_xLeftPagesRB->set_active(true);
                m_xMirrorHorzBox->set_active(true);
                break;
            case 2: // mirror on all pages
                m_xAllPagesRB->set_active(true);
                break;
            case 3: // mirror on right / odd pages
                m_xRightPagesRB->set_active(true);
                break;
            default:
                m_xAllPagesRB->set_active(true);
                break;
        }
    }

    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, false, &pItem ) )
    {
        const SvxBrushItem& rBrush = *static_cast<const SvxBrushItem*>(pItem);
        if( !rBrush.GetGraphicLink().isEmpty() )
        {
            aGrfName = aNewGrfName = rBrush.GetGraphicLink();
            m_xConnectED->set_text(aNewGrfName);
        }
        OUString referer;
        SfxStringItem const * it = static_cast<SfxStringItem const *>(
            rSet.GetItem(SID_REFERER));
        if (it != nullptr) {
            referer = it->GetValue();
        }
        const Graphic* pGrf = rBrush.GetGraphic(referer);
        if( pGrf )
            m_aBmpWin.SetGraphic( *pGrf );
    }

    m_xConnectED->save_value();

    m_xMirror->set_sensitive(bEnable);
    m_xAllPagesRB->set_sensitive(bEnableMirrorRB);
    m_xLeftPagesRB->set_sensitive(bEnableMirrorRB);
    m_xRightPagesRB->set_sensitive(bEnableMirrorRB);

    m_xAllPagesRB->save_state();
    m_xLeftPagesRB->save_state();
    m_xRightPagesRB->save_state();
    m_xMirrorHorzBox->save_state();
    m_xMirrorVertBox->save_state();

    m_aBmpWin.MirrorHorz( m_xMirrorVertBox->get_active() );
    m_aBmpWin.MirrorVert( m_xMirrorHorzBox->get_active() );
    m_aBmpWin.Invalidate();
}

bool SwGrfExtPage::FillItemSet( SfxItemSet *rSet )
{
    bool bModified = false;
    if ( m_xMirrorHorzBox->get_state_changed_from_saved() ||
         m_xMirrorVertBox->get_state_changed_from_saved() ||
         m_xAllPagesRB->get_state_changed_from_saved() ||
         m_xLeftPagesRB->get_state_changed_from_saved() ||
         m_xRightPagesRB->get_state_changed_from_saved() )
    {
        bModified = true;

        bool bHori = false;

        if (m_xMirrorHorzBox->get_active() &&
                !m_xLeftPagesRB->get_active())
            bHori = true;

        MirrorGraph eMirror;
        eMirror = m_xMirrorVertBox->get_active() && bHori ?
                    MirrorGraph::Both : bHori ?
                    MirrorGraph::Vertical : m_xMirrorVertBox->get_active() ?
                    MirrorGraph::Horizontal  : MirrorGraph::Dont;

        bool bMirror = !m_xAllPagesRB->get_active();
        SwMirrorGrf aMirror( eMirror );
        aMirror.SetGrfToggle(bMirror );
        rSet->Put( aMirror );
    }

    if (aGrfName != aNewGrfName || m_xConnectED->get_value_changed_from_saved())
    {
        bModified = true;
        aGrfName = m_xConnectED->get_text();
        rSet->Put( SvxBrushItem( aGrfName, aFilterName, GPOS_LT,
                                SID_ATTR_GRAF_GRAPHIC ));
    }

    // RotGrfFlyFrame: Safe rotation if modified
    if(m_aCtlAngle.IsValueModified())
    {
        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ANGLE), m_aCtlAngle.GetRotation()));
        bModified = true;
    }

    return bModified;
}

DeactivateRC SwGrfExtPage::DeactivatePage(SfxItemSet *_pSet)
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(SwGrfExtPage, BrowseHdl, weld::Button&, void)
{
    if(!m_xGrfDlg)
    {
        m_xGrfDlg.reset(new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
                FileDialogFlags::Graphic, GetFrameWeld()));
        m_xGrfDlg->SetTitle(m_xLinkFrame->get_label());
    }
    m_xGrfDlg->SetDisplayDirectory(m_xConnectED->get_text());
    uno::Reference < ui::dialogs::XFilePicker3 > xFP = m_xGrfDlg->GetFilePicker();
    uno::Reference < ui::dialogs::XFilePickerControlAccess > xCtrlAcc(xFP, uno::UNO_QUERY);
    xCtrlAcc->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, uno::makeAny(true) );

    if ( m_xGrfDlg->Execute() != ERRCODE_NONE )
        return;

// remember selected filter
    aFilterName = m_xGrfDlg->GetCurrentFilter();
    aNewGrfName = INetURLObject::decode( m_xGrfDlg->GetPath(),
                                       INetURLObject::DecodeMechanism::Unambiguous );
    m_xConnectED->set_text(aNewGrfName);
    //reset mirrors because maybe a Bitmap was swapped with
    //another type of graphic that cannot be mirrored.
    m_xMirrorVertBox->set_active(false);
    m_xMirrorHorzBox->set_active(false);
    m_xAllPagesRB->set_sensitive(false);
    m_xLeftPagesRB->set_sensitive(false);
    m_xRightPagesRB->set_sensitive(false);
    m_aBmpWin.MirrorHorz(false);
    m_aBmpWin.MirrorVert(false);

    Graphic aGraphic;
    (void)GraphicFilter::LoadGraphic(m_xGrfDlg->GetPath(), OUString(), aGraphic);
    m_aBmpWin.SetGraphic(aGraphic);

    bool bEnable = GraphicType::Bitmap      == aGraphic.GetType() ||
                        GraphicType::GdiMetafile == aGraphic.GetType();
    m_xMirrorVertBox->set_sensitive(bEnable);
    m_xMirrorHorzBox->set_sensitive(bEnable);
    m_xAllPagesRB->set_sensitive(bEnable);
    m_xLeftPagesRB->set_sensitive(bEnable);
    m_xRightPagesRB->set_sensitive(bEnable);

}

IMPL_LINK_NOARG(SwGrfExtPage, MirrorHdl, weld::ToggleButton&, void)
{
    bool bEnable = m_xMirrorHorzBox->get_active();

    m_aBmpWin.MirrorHorz( m_xMirrorVertBox->get_active() );
    m_aBmpWin.MirrorVert( bEnable );

    m_xAllPagesRB->set_sensitive(bEnable);
    m_xLeftPagesRB->set_sensitive(bEnable);
    m_xRightPagesRB->set_sensitive(bEnable);

    if (!m_xAllPagesRB->get_active() && !m_xLeftPagesRB->get_active() && !m_xRightPagesRB->get_active())
        m_xAllPagesRB->set_active(true);
}

// example window
BmpWindow::BmpWindow()
    : bHorz(false)
    , bVert(false)
    , bGraphic(false)
{
}

void BmpWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize = pDrawingArea->get_ref_device().LogicToPixel(Size(127 , 66), MapMode(MapUnit::MapAppFont));
    set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);
}

void BmpWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    // Setup
    rRenderContext.SetBackground();
    // #i119307# the graphic might have transparency, set up white as the color
    // to use when drawing a rectangle under the image
    rRenderContext.SetLineColor(COL_WHITE);
    rRenderContext.SetFillColor(COL_WHITE);

    // Paint
    Point aPntPos;
    Size aPntSz(GetOutputSizePixel());
    Size aGrfSize;
    if (bGraphic)
        aGrfSize = ::GetGraphicSizeTwip(aGraphic, &rRenderContext);
    //it should show the default bitmap also if no graphic can be found
    if (!aGrfSize.Width() && !aGrfSize.Height())
        aGrfSize = rRenderContext.PixelToLogic(aBmp.GetSizePixel());

    long nRelGrf = aGrfSize.Width() * 100 / aGrfSize.Height();
    long nRelWin = aPntSz.Width() * 100 / aPntSz.Height();
    if (nRelGrf < nRelWin)
    {
        const long nWidth = aPntSz.Width();
        // if we use a replacement preview, try to draw at original size
        if (!bGraphic && (aGrfSize.Width() <= aPntSz.Width())
                      && (aGrfSize.Height() <= aPntSz.Height()))
        {
            const long nHeight = aPntSz.Height();
            aPntSz.setWidth( aGrfSize.Width() );
            aPntSz.setHeight( aGrfSize.Height() );
            aPntPos.AdjustY((nHeight - aPntSz.Height()) / 2 );
        }
        else
            aPntSz.setWidth( aPntSz.Height() * nRelGrf /100 );

        aPntPos.AdjustX(nWidth - aPntSz.Width() ) ;
    }

    // #i119307# clear window background, the graphic might have transparency
    rRenderContext.DrawRect(tools::Rectangle(aPntPos, aPntSz));

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
SwFrameURLPage::SwFrameURLPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/frmurlpage.ui", "FrameURLPage", &rSet)
    , m_xURLED(m_xBuilder->weld_entry("url"))
    , m_xSearchPB(m_xBuilder->weld_button("search"))
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xFrameCB(m_xBuilder->weld_combo_box("frame"))
    , m_xServerCB(m_xBuilder->weld_check_button("server"))
    , m_xClientCB(m_xBuilder->weld_check_button("client"))
{
    m_xSearchPB->connect_clicked(LINK(this, SwFrameURLPage, InsertFileHdl));
}

SwFrameURLPage::~SwFrameURLPage()
{
    disposeOnce();
}

void SwFrameURLPage::Reset( const SfxItemSet *rSet )
{
    const SfxPoolItem* pItem;
    if ( SfxItemState::SET == rSet->GetItemState( SID_DOCFRAME, true, &pItem))
    {
        std::unique_ptr<TargetList> pList(new TargetList);
        SfxFrame::GetDefaultTargetList(*pList);
        if( !pList->empty() )
        {
            size_t nCount = pList->size();
            for (size_t i = 0; i < nCount; ++i)
            {
                m_xFrameCB->append_text(pList->at(i));
            }
        }
    }

    if ( SfxItemState::SET == rSet->GetItemState( RES_URL, true, &pItem ) )
    {
        const SwFormatURL* pFormatURL = static_cast<const SwFormatURL*>(pItem);
        m_xURLED->set_text(INetURLObject::decode(pFormatURL->GetURL(),
                                           INetURLObject::DecodeMechanism::Unambiguous));
        m_xNameED->set_text(pFormatURL->GetName());

        m_xClientCB->set_sensitive(pFormatURL->GetMap() != nullptr);
        m_xClientCB->set_active(pFormatURL->GetMap() != nullptr);
        m_xServerCB->set_active(pFormatURL->IsServerMap());

        m_xFrameCB->set_entry_text(pFormatURL->GetTargetFrameName());
        m_xFrameCB->save_value();
    }
    else
        m_xClientCB->set_sensitive(false);

    m_xServerCB->save_state();
    m_xClientCB->save_state();
}

bool SwFrameURLPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = false;
    const SwFormatURL* pOldURL = GetOldItem(*rSet, RES_URL);
    std::unique_ptr<SwFormatURL> pFormatURL;
    if(pOldURL)
        pFormatURL.reset(static_cast<SwFormatURL*>(pOldURL->Clone()));
    else
        pFormatURL.reset(new SwFormatURL());

    {
        const OUString sText = m_xURLED->get_text();

        if( pFormatURL->GetURL() != sText ||
            pFormatURL->GetName() != m_xNameED->get_text() ||
            m_xServerCB->get_active() != pFormatURL->IsServerMap() )
        {
            pFormatURL->SetURL(sText, m_xServerCB->get_active());
            pFormatURL->SetName(m_xNameED->get_text());
            bModified = true;
        }
    }

    if (!m_xClientCB->get_active() && pFormatURL->GetMap() != nullptr)
    {
        pFormatURL->SetMap(nullptr);
        bModified = true;
    }

    if(pFormatURL->GetTargetFrameName() != m_xFrameCB->get_active_text())
    {
        pFormatURL->SetTargetFrameName(m_xFrameCB->get_active_text());
        bModified = true;
    }
    rSet->Put(*pFormatURL);
    return bModified;
}

VclPtr<SfxTabPage> SwFrameURLPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFrameURLPage>::Create(pParent, *rSet);
}

IMPL_LINK_NOARG(SwFrameURLPage, InsertFileHdl, weld::Button&, void)
{
    FileDialogHelper aDlgHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                FileDialogFlags::NONE, GetFrameWeld());
    uno::Reference < ui::dialogs::XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

    try
    {
        const OUString sTemp(m_xURLED->get_text());
        if(!sTemp.isEmpty())
            xFP->setDisplayDirectory(sTemp);
    }
    catch( const uno::Exception& ) {}
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        m_xURLED->set_text(xFP->getSelectedFiles().getConstArray()[0]);
    }
}

SwFrameAddPage::SwFrameAddPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/frmaddpage.ui", "FrameAddPage", &rSet)
    , m_pWrtSh(nullptr)
    , m_bHtmlMode(false)
    , m_bFormat(false)
    , m_bNew(false)
    , m_xNameFrame(m_xBuilder->weld_widget("nameframe"))
    , m_xNameFT(m_xBuilder->weld_label("name_label"))
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xAltNameFT(m_xBuilder->weld_label("altname_label"))
    , m_xAltNameED(m_xBuilder->weld_entry("altname"))
    , m_xDescriptionFT(m_xBuilder->weld_label("description_label"))
    , m_xDescriptionED(m_xBuilder->weld_entry("description"))
    , m_xPrevFT(m_xBuilder->weld_label("prev_label"))
    , m_xPrevLB(m_xBuilder->weld_combo_box("prev"))
    , m_xNextFT(m_xBuilder->weld_label("next_label"))
    , m_xNextLB(m_xBuilder->weld_combo_box("next"))
    , m_xProtectFrame(m_xBuilder->weld_widget("protect"))
    , m_xProtectContentCB(m_xBuilder->weld_check_button("protectcontent"))
    , m_xProtectFrameCB(m_xBuilder->weld_check_button("protectframe"))
    , m_xProtectSizeCB(m_xBuilder->weld_check_button("protectsize"))
    , m_xContentAlignFrame(m_xBuilder->weld_widget("contentalign"))
    , m_xVertAlignLB(m_xBuilder->weld_combo_box("vertalign"))
    , m_xPropertiesFrame(m_xBuilder->weld_widget("properties"))
    , m_xEditInReadonlyCB(m_xBuilder->weld_check_button("editinreadonly"))
    , m_xPrintFrameCB(m_xBuilder->weld_check_button("printframe"))
    , m_xTextFlowFT(m_xBuilder->weld_label("textflow_label"))
    , m_xTextFlowLB(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box("textflow")))
{
    m_xTextFlowLB->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xTextFlowLB->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xTextFlowLB->append(SvxFrameDirection::Vertical_RL_TB, SvxResId(RID_SVXSTR_PAGEDIR_RTL_VERT));
    m_xTextFlowLB->append(SvxFrameDirection::Vertical_LR_TB, SvxResId(RID_SVXSTR_PAGEDIR_LTR_VERT));
    m_xTextFlowLB->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));
    m_xDescriptionED->set_size_request(-1, m_xDescriptionED->get_preferred_size().Height());
}

SwFrameAddPage::~SwFrameAddPage()
{
    disposeOnce();
}

void SwFrameAddPage::dispose()
{
    m_xTextFlowLB.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwFrameAddPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
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
        m_xProtectFrame->hide();
        m_xEditInReadonlyCB->hide();
        m_xPrintFrameCB->hide();
    }
    if (m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog")
    {
        m_xEditInReadonlyCB->hide();
        if (m_bHtmlMode)
        {
            m_xPropertiesFrame->hide();
        }
        m_xContentAlignFrame->hide();
    }

    if(SfxItemState::SET == rSet->GetItemState(FN_SET_FRM_ALT_NAME, false, &pItem))
    {
        m_xAltNameED->set_text(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_xAltNameED->save_value();
    }

    if(SfxItemState::SET == rSet->GetItemState(FN_UNO_DESCRIPTION, false, &pItem))
    {
        m_xDescriptionED->set_text(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_xDescriptionED->save_value();
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

        OSL_ENSURE(m_pWrtSh, "no Shell?");
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

        m_xNameED->set_text( aTmpName1 );
        m_xNameED->save_value();
    }
    else
    {
        m_xNameED->set_sensitive( false );
        m_xAltNameED->set_sensitive(false);
        m_xNameFT->set_sensitive( false );
        m_xAltNameFT->set_sensitive(false);
    }
    if (m_sDlgType == "FrameDialog" && m_xAltNameFT->get_visible())
    {
        m_xAltNameFT->hide();
        m_xAltNameED->hide();
    }
    else
    {
        m_xNameED->connect_changed(LINK(this, SwFrameAddPage, EditModifyHdl));
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
            std::vector< OUString > aPrevPageFrames;
            std::vector< OUString > aThisPageFrames;
            std::vector< OUString > aNextPageFrames;
            std::vector< OUString > aRemainFrames;
            m_pWrtSh->GetConnectableFrameFormats(*pFormat, sNextChain, false,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            for (sal_Int32 nEntry = m_xPrevLB->get_count(); nEntry > 1; nEntry--)
                m_xPrevLB->remove(nEntry - 1);
            lcl_InsertVectors(*m_xPrevLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(!sPrevChain.isEmpty())
            {
                if (m_xPrevLB->find_text(sPrevChain) == -1)
                    m_xPrevLB->insert_text(1, sPrevChain);
                m_xPrevLB->set_active_text(sPrevChain);
            }
            else
                m_xPrevLB->set_active(0);
            aPrevPageFrames.erase(aPrevPageFrames.begin(), aPrevPageFrames.end());
            aNextPageFrames.erase(aNextPageFrames.begin(), aNextPageFrames.end());
            aThisPageFrames.erase(aThisPageFrames.begin(), aThisPageFrames.end());
            aRemainFrames.erase(aRemainFrames.begin(), aRemainFrames.end());

            m_pWrtSh->GetConnectableFrameFormats(*pFormat, sPrevChain, true,
                            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
            for (sal_Int32 nEntry = m_xNextLB->get_count(); nEntry > 1; nEntry--)
                m_xNextLB->remove(nEntry - 1);
            lcl_InsertVectors(*m_xNextLB, aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
            if(!sNextChain.isEmpty())
            {
                if (m_xNextLB->find_text(sNextChain) == -1)
                    m_xNextLB->insert_text(1, sNextChain);
                m_xNextLB->set_active_text(sNextChain);
            }
            else
                m_xNextLB->set_active(0);
            Link<weld::ComboBox&,void> aLink(LINK(this, SwFrameAddPage, ChainModifyHdl));
            m_xPrevLB->connect_changed(aLink);
            m_xNextLB->connect_changed(aLink);
        }
    }
    // Pos Protected
    const SvxProtectItem& rProt = rSet->Get(RES_PROTECT);
    m_xProtectFrameCB->set_active(rProt.IsPosProtected());
    m_xProtectContentCB->set_active(rProt.IsContentProtected());
    m_xProtectSizeCB->set_active(rProt.IsSizeProtected());

    const SwFormatEditInReadonly& rEdit = rSet->Get(RES_EDIT_IN_READONLY);
    m_xEditInReadonlyCB->set_active(rEdit.GetValue());
    m_xEditInReadonlyCB->save_state();

    // print
    const SvxPrintItem& rPrt = rSet->Get(RES_PRINT);
    m_xPrintFrameCB->set_active(rPrt.GetValue());
    m_xPrintFrameCB->save_state();

    // textflow
    if( (!m_bHtmlMode || (0 != (nHtmlMode&HTMLMODE_SOME_STYLES)))
        && m_sDlgType != "PictureDialog" && m_sDlgType != "ObjectDialog"
        && SfxItemState::UNKNOWN != rSet->GetItemState( RES_FRAMEDIR ) )
    {
        m_xTextFlowFT->show();
        m_xTextFlowLB->show();

        //vertical text flow is not possible in HTML
        if(m_bHtmlMode)
        {
            m_xTextFlowLB->remove_id(SvxFrameDirection::Vertical_RL_TB);
        }
        SvxFrameDirection nVal = rSet->Get(RES_FRAMEDIR).GetValue();
        m_xTextFlowLB->set_active_id(nVal);
        m_xTextFlowLB->save_value();
    }
    else
    {
        m_xTextFlowFT->hide();
        m_xTextFlowLB->hide();
    }

    // Content alignment
    if ( rSet->GetItemState(RES_TEXT_VERT_ADJUST) > SfxItemState::DEFAULT )
    {
        SdrTextVertAdjust nAdjust = rSet->Get(RES_TEXT_VERT_ADJUST).GetValue();
        sal_Int32 nPos = 0;
        switch(nAdjust)
        {
            case SDRTEXTVERTADJUST_TOP:      nPos = 0;   break;
            case SDRTEXTVERTADJUST_CENTER:
            case SDRTEXTVERTADJUST_BLOCK:    nPos = 1;   break;
            case SDRTEXTVERTADJUST_BOTTOM:   nPos = 2;   break;
        }
        m_xVertAlignLB->set_active(nPos);
    }
    m_xVertAlignLB->save_value();
}

bool SwFrameAddPage::FillItemSet(SfxItemSet *rSet)
{
    bool bRet = false;
    if (m_xNameED->get_value_changed_from_saved())
        bRet |= nullptr != rSet->Put(SfxStringItem(FN_SET_FRM_NAME, m_xNameED->get_text()));
    if (m_xAltNameED->get_value_changed_from_saved())
        bRet |= nullptr != rSet->Put(SfxStringItem(FN_SET_FRM_ALT_NAME, m_xAltNameED->get_text()));
    if (m_xDescriptionED->get_value_changed_from_saved())
        bRet |= nullptr != rSet->Put(SfxStringItem(FN_UNO_DESCRIPTION, m_xDescriptionED->get_text()));

    const SfxPoolItem* pOldItem;
    SvxProtectItem aProt ( GetItemSet().Get(RES_PROTECT) );
    aProt.SetContentProtect( m_xProtectContentCB->get_active() );
    aProt.SetSizeProtect ( m_xProtectSizeCB->get_active() );
    aProt.SetPosProtect  ( m_xProtectFrameCB->get_active() );
    if ( nullptr == (pOldItem = GetOldItem(*rSet, FN_SET_PROTECT)) ||
                aProt != *pOldItem )
        bRet |= nullptr != rSet->Put( aProt);

    if ( m_xEditInReadonlyCB->get_state_changed_from_saved() )
        bRet |= nullptr != rSet->Put( SwFormatEditInReadonly( RES_EDIT_IN_READONLY, m_xEditInReadonlyCB->get_active()));

    if ( m_xPrintFrameCB->get_state_changed_from_saved() )
        bRet |= nullptr != rSet->Put( SvxPrintItem( RES_PRINT, m_xPrintFrameCB->get_active()));

    // textflow
    if (m_xTextFlowLB->get_visible() && m_xTextFlowLB->get_value_changed_from_saved())
    {
        SvxFrameDirection eDirection = m_xTextFlowLB->get_active_id();
        bRet |= nullptr != rSet->Put( SvxFrameDirectionItem(eDirection, RES_FRAMEDIR ));
    }
    if(m_pWrtSh)
    {
        const SwFrameFormat* pFormat = m_pWrtSh->GetFlyFrameFormat();
        if (pFormat)
        {
            OUString sCurrentPrevChain, sCurrentNextChain;
            if (m_xPrevLB->get_active())
                sCurrentPrevChain = m_xPrevLB->get_active_text();
            if (m_xNextLB->get_active())
                sCurrentNextChain = m_xNextLB->get_active_text();
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

    if (m_xVertAlignLB->get_value_changed_from_saved())
    {
        SdrTextVertAdjust nAdjust;
        switch (m_xVertAlignLB->get_active())
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

IMPL_LINK_NOARG(SwFrameAddPage, EditModifyHdl, weld::Entry&, void)
{
    bool bEnable = !m_xNameED->get_text().isEmpty();
    m_xAltNameED->set_sensitive(bEnable);
    m_xAltNameFT->set_sensitive(bEnable);
}

void SwFrameAddPage::SetFormatUsed(bool bFormatUsed)
{
    m_bFormat = bFormatUsed;
    if (m_bFormat)
    {
        m_xNameFrame->hide();
    }
}

IMPL_LINK(SwFrameAddPage, ChainModifyHdl, weld::ComboBox&, rBox, void)
{
    OUString sCurrentPrevChain, sCurrentNextChain;
    if (m_xPrevLB->get_active())
        sCurrentPrevChain = m_xPrevLB->get_active_text();
    if (m_xNextLB->get_active())
        sCurrentNextChain = m_xNextLB->get_active_text();
    SwFrameFormat* pFormat = m_pWrtSh->GetFlyFrameFormat();
    if (!pFormat)
        return;

    bool bNextBox = m_xNextLB.get() == &rBox;
    weld::ComboBox& rChangeLB = bNextBox ? *m_xPrevLB : *m_xNextLB;
    for (sal_Int32 nEntry = rChangeLB.get_count(); nEntry > 1; nEntry--)
        rChangeLB.remove(nEntry - 1);
    //determine chainable frames
    std::vector< OUString > aPrevPageFrames;
    std::vector< OUString > aThisPageFrames;
    std::vector< OUString > aNextPageFrames;
    std::vector< OUString > aRemainFrames;
    m_pWrtSh->GetConnectableFrameFormats(*pFormat, bNextBox ? sCurrentNextChain : sCurrentPrevChain, !bNextBox,
                    aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames );
    lcl_InsertVectors(rChangeLB,
            aPrevPageFrames, aThisPageFrames, aNextPageFrames, aRemainFrames);
    const OUString sToSelect = bNextBox ? sCurrentPrevChain : sCurrentNextChain;
    if (rChangeLB.find_text(sToSelect) != -1)
        rChangeLB.set_active_text(sToSelect);
    else
        rChangeLB.set_active(0);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
