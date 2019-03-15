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
    void HandleAutoCB(bool _bChecked, weld::Label& _rFT_man, weld::Label& _rFT_auto, weld::MetricSpinButton& _rPF_Edit)
    {
        _rFT_man.set_visible( !_bChecked );
        _rFT_auto.set_visible( _bChecked );
        OUString accName = _bChecked ? _rFT_auto.get_label() : _rFT_man.get_label();
        _rPF_Edit.set_accessible_name(accName);
    }
}

SwFramePage::SwFramePage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/frmtypepage.ui", "FrameTypePage", &rSet)
    , m_bAtHorzPosModified(false)
    , m_bAtVertPosModified(false)
    , m_bFormat(false)
    , m_bNew(true)
    , m_bNoModifyHdl(true)
    , m_bIsVerticalFrame(false)
    , m_bIsVerticalL2R(false)
    , m_bIsInRightToLeft(false)
    , m_bHtmlMode(false)
    , m_nHtmlMode(0)
    , m_nUpperBorder(0)
    , m_nLowerBorder(0)
    , m_fWidthHeightRatio(1.0)
    , mpToCharContentPos(nullptr)
    , m_nOldH(text::HoriOrientation::CENTER)
    , m_nOldHRel(text::RelOrientation::FRAME)
    , m_nOldV(text::VertOrientation::TOP)
    , m_nOldVRel(text::RelOrientation::PRINT_AREA)
    , m_pVMap(nullptr)
    , m_pHMap(nullptr)
    , m_bAllowVertPositioning( true )
    , m_bIsMathOLE(false)
    , m_bIsMathBaselineAlignment(true)
    , m_xWidthFT(m_xBuilder->weld_label("widthft"))
    , m_xWidthAutoFT(m_xBuilder->weld_label("autowidthft"))
    , m_xRelWidthCB(m_xBuilder->weld_check_button("relwidth"))
    , m_xRelWidthRelationLB(m_xBuilder->weld_combo_box("relwidthrelation"))
    , m_xAutoWidthCB(m_xBuilder->weld_check_button("autowidth"))
    , m_xHeightFT(m_xBuilder->weld_label("heightft"))
    , m_xHeightAutoFT(m_xBuilder->weld_label("autoheightft"))
    , m_xRelHeightCB(m_xBuilder->weld_check_button("relheight"))
    , m_xRelHeightRelationLB(m_xBuilder->weld_combo_box("relheightrelation"))
    , m_xAutoHeightCB(m_xBuilder->weld_check_button("autoheight"))
    , m_xFixedRatioCB(m_xBuilder->weld_check_button("ratio"))
    , m_xRealSizeBT(m_xBuilder->weld_button("origsize"))
    , m_xAnchorFrame(m_xBuilder->weld_widget("anchorframe"))
    , m_xAnchorAtPageRB(m_xBuilder->weld_radio_button("topage"))
    , m_xAnchorAtParaRB(m_xBuilder->weld_radio_button("topara"))
    , m_xAnchorAtCharRB(m_xBuilder->weld_radio_button("tochar"))
    , m_xAnchorAsCharRB(m_xBuilder->weld_radio_button("aschar"))
    , m_xAnchorAtFrameRB(m_xBuilder->weld_radio_button("toframe"))
    , m_xHorizontalFT(m_xBuilder->weld_label("horiposft"))
    , m_xHorizontalDLB(m_xBuilder->weld_combo_box("horipos"))
    , m_xAtHorzPosFT(m_xBuilder->weld_label("horibyft"))
    , m_xAtHorzPosED(m_xBuilder->weld_metric_spin_button("byhori", FieldUnit::CM))
    , m_xHoriRelationFT(m_xBuilder->weld_label("horitoft"))
    , m_xHoriRelationLB(m_xBuilder->weld_combo_box("horianchor"))
    , m_xMirrorPagesCB(m_xBuilder->weld_check_button("mirror"))
    , m_xVerticalFT(m_xBuilder->weld_label("vertposft"))
    , m_xVerticalDLB(m_xBuilder->weld_combo_box("vertpos"))
    , m_xAtVertPosFT(m_xBuilder->weld_label("vertbyft"))
    , m_xAtVertPosED(m_xBuilder->weld_metric_spin_button("byvert", FieldUnit::CM))
    , m_xVertRelationFT(m_xBuilder->weld_label("verttoft"))
    , m_xVertRelationLB(m_xBuilder->weld_combo_box("vertanchor"))
    , m_xFollowTextFlowCB(m_xBuilder->weld_check_button("followtextflow"))
    , m_xExampleWN(new weld::CustomWeld(*m_xBuilder, "preview", m_aExampleWN))
    , m_xWidthED(new SwPercentField(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM)))
    , m_xHeightED(new SwPercentField(m_xBuilder->weld_metric_spin_button("height", FieldUnit::CM)))
{
    const auto nWidthRequest = m_xAtHorzPosED->get_preferred_size().Width();
    m_xAtHorzPosED->set_size_request(nWidthRequest, -1);
    m_xAtVertPosED->set_size_request(nWidthRequest, -1);

    setOptimalFrameWidth();
    setOptimalRelWidth();

    SetExchangeSupport();

    Link<weld::MetricSpinButton&,void> aLk3 = LINK(this, SwFramePage, ModifyHdl);
    m_xWidthED->connect_value_changed( aLk3 );
    m_xHeightED->connect_value_changed( aLk3 );
    m_xAtHorzPosED->connect_value_changed( aLk3 );
    m_xAtVertPosED->connect_value_changed( aLk3 );
    m_xFollowTextFlowCB->connect_toggled(LINK(this, SwFramePage, RangeModifyClickHdl));

    Link<weld::ToggleButton&,void> aLk2 = LINK(this, SwFramePage, AnchorTypeHdl);
    m_xAnchorAtPageRB->connect_toggled( aLk2 );
    m_xAnchorAtParaRB->connect_toggled( aLk2 );
    m_xAnchorAtCharRB->connect_toggled( aLk2 );
    m_xAnchorAsCharRB->connect_toggled( aLk2 );
    m_xAnchorAtFrameRB->connect_toggled( aLk2 );

    m_xHorizontalDLB->connect_changed(LINK(this, SwFramePage, PosHdl));
    m_xVerticalDLB->connect_changed(LINK(this, SwFramePage, PosHdl));

    m_xHoriRelationLB->connect_changed(LINK(this, SwFramePage, RelHdl));
    m_xVertRelationLB->connect_changed(LINK(this, SwFramePage, RelHdl));

    m_xMirrorPagesCB->connect_toggled(LINK(this, SwFramePage, MirrorHdl));

    aLk2 = LINK(this, SwFramePage, RelSizeClickHdl);
    m_xRelWidthCB->connect_toggled(aLk2);
    m_xRelHeightCB->connect_toggled(aLk2);

    m_xAutoWidthCB->connect_toggled(LINK(this, SwFramePage, AutoWidthClickHdl));
    m_xAutoHeightCB->connect_toggled(LINK(this, SwFramePage, AutoHeightClickHdl));
}

SwFramePage::~SwFramePage()
{
    disposeOnce();
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
        m_xHorizontalDLB->append_text(SvxSwFramePosString::GetString(rFrame));
    }

    Size aBiggest(m_xHorizontalDLB->get_preferred_size());
    m_xHorizontalDLB->set_size_request(aBiggest.Width(), -1);
    m_xVerticalDLB->set_size_request(aBiggest.Width(), -1);
    m_xHorizontalDLB->clear();
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
        m_xHoriRelationLB->append_text(SvxSwFramePosString::GetString(rRel));
    }

    Size aBiggest(m_xHoriRelationLB->get_preferred_size());
    m_xHoriRelationLB->set_size_request(aBiggest.Width(), -1);
    m_xVertRelationLB->set_size_request(aBiggest.Width(), -1);
    m_xRelWidthRelationLB->set_size_request(aBiggest.Width(), -1);
    m_xRelHeightRelationLB->set_size_request(aBiggest.Width(), -1);
    m_xHoriRelationLB->clear();
}

VclPtr<SfxTabPage> SwFramePage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwFramePage>::Create(pParent, *rSet);
}

void SwFramePage::EnableGraficMode()
{
    // i#39692 - mustn't be called more than once
    if (!m_xRealSizeBT->get_visible())
    {
        m_xWidthFT->show();
        m_xWidthAutoFT->hide();
        m_xAutoHeightCB->hide();

        m_xHeightFT->show();
        m_xHeightAutoFT->hide();
        m_xAutoWidthCB->hide();

        m_xRealSizeBT->show();
    }
}

SwWrtShell *SwFramePage::getFrameDlgParentShell()
{
    return static_cast<SwFrameDlg*>(GetDialogController())->GetWrtShell();
}

void SwFramePage::Reset( const SfxItemSet *rSet )
{
    SwWrtShell* pSh = m_bFormat ? ::GetActiveWrtShell() :
            getFrameDlgParentShell();

    m_nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
    m_bHtmlMode = (m_nHtmlMode & HTMLMODE_ON) != 0;

    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    m_xWidthED->SetMetric(aMetric);
    m_xHeightED->SetMetric(aMetric);
    ::SetFieldUnit(*m_xAtHorzPosED, aMetric);
    ::SetFieldUnit(*m_xAtVertPosED, aMetric);

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
        m_xAnchorFrame->set_sensitive(false);
        m_xFixedRatioCB->set_sensitive(false);
    }
    else
    {
        if (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_FLY && !pSh->IsFlyInFly())
            m_xAnchorAtFrameRB->hide();
        if ( pSh->IsFrameVertical( true, m_bIsInRightToLeft, m_bIsVerticalL2R ) )
        {
            OUString sHLabel = m_xHorizontalFT->get_label();
            m_xHorizontalFT->set_label(m_xVerticalFT->get_label());
            m_xVerticalFT->set_label(sHLabel);
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
            m_xRealSizeBT->connect_clicked(LINK(this, SwFramePage, RealSizeHdl));
            EnableGraficMode();
        }

        if (m_sDlgType == "PictureDialog")
            m_xFixedRatioCB->set_active(false);
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

    // entering percent value made possible

    // the available space is not yet known so the RefValue has to be calculated from size and relative size values
    // this is needed only if relative values are already set
    const SwFormatFrameSize& rFrameSize = rSet->Get(RES_FRM_SIZE);

    m_xRelWidthRelationLB->append_text(SvxSwFramePosString::GetString(SwFPos::FRAME));
    m_xRelWidthRelationLB->append_text(SvxSwFramePosString::GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetWidthPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetWidthPercent() != 0)
    {
        //calculate the reference value from the width and relative width values
        sal_Int32 nSpace = rFrameSize.GetWidth() * 100 / rFrameSize.GetWidthPercent();
        m_xWidthED->SetRefValue( nSpace );

        m_xRelWidthRelationLB->set_sensitive(true);
    }
    else
        m_xRelWidthRelationLB->set_sensitive(false);

    m_xRelHeightRelationLB->append_text(SvxSwFramePosString::GetString(SwFPos::FRAME));
    m_xRelHeightRelationLB->append_text(SvxSwFramePosString::GetString(SwFPos::REL_PG_FRAME));
    if (rFrameSize.GetHeightPercent() != SwFormatFrameSize::SYNCED && rFrameSize.GetHeightPercent() != 0)
    {
        //calculate the reference value from the with and relative width values
        sal_Int32 nSpace = rFrameSize.GetHeight() * 100 / rFrameSize.GetHeightPercent();
        m_xHeightED->SetRefValue( nSpace );

        m_xRelHeightRelationLB->set_sensitive(true);
    }
    else
        m_xRelHeightRelationLB->set_sensitive(false);

    // general initialisation part
    switch(rAnchor.GetAnchorId())
    {
        case RndStdIds::FLY_AT_PAGE: m_xAnchorAtPageRB->set_active(true); break;
        case RndStdIds::FLY_AT_PARA: m_xAnchorAtParaRB->set_active(true); break;
        case RndStdIds::FLY_AT_CHAR: m_xAnchorAtCharRB->set_active(true); break;
        case RndStdIds::FLY_AS_CHAR: m_xAnchorAsCharRB->set_active(true); break;
        case RndStdIds::FLY_AT_FLY: m_xAnchorAtFrameRB->set_active(true);break;
        default:; //prevent warning
    }

    // i#22341 - determine content position of character
    // Note: content position can be NULL
    mpToCharContentPos = rAnchor.GetContentAnchor();

    // i#18732 - init checkbox value
    {
        const bool bFollowTextFlow =
            rSet->Get(RES_FOLLOW_TEXT_FLOW).GetValue();
        m_xFollowTextFlowCB->set_active(bFollowTextFlow);
    }

    if(m_bHtmlMode)
    {
        m_xAutoHeightCB->set_sensitive(false);
        m_xAutoWidthCB->set_sensitive(false);
        m_xMirrorPagesCB->hide();
        if (m_sDlgType == "FrameDialog")
            m_xFixedRatioCB->set_sensitive(false);
        // i#18732 hide checkbox in HTML mode
        m_xFollowTextFlowCB->hide();
    }
    else
    {
        // enable/disable of check box 'Mirror on..'
        m_xMirrorPagesCB->set_sensitive(!m_xAnchorAsCharRB->get_active());

        // enable/disable check box 'Follow text flow'.
        // enable check box 'Follow text
        // flow' also for anchor type to-frame.
        m_xFollowTextFlowCB->set_sensitive(m_xAnchorAtParaRB->get_active() ||
                                           m_xAnchorAtCharRB->get_active() ||
                                           m_xAnchorAtFrameRB->get_active());
    }

    Init(*rSet);
    m_xAtVertPosED->save_value();
    m_xAtHorzPosED->save_value();
    m_xFollowTextFlowCB->save_state();

    m_xWidthED->save_value();
    m_xHeightED->save_value();

    m_bNoModifyHdl = false;
    //lock PercentFields
    m_xWidthED->LockAutoCalculation(true);
    m_xHeightED->LockAutoCalculation(true);
    RangeModifyHdl();  // set all maximum values initially
    m_xHeightED->LockAutoCalculation(false);
    m_xWidthED->LockAutoCalculation(false);

    m_xAutoHeightCB->save_state();
    m_xAutoWidthCB->save_state();

    SwTwips nWidth  = static_cast< SwTwips >(m_xWidthED->DenormalizePercent(m_xWidthED->get_value(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_xHeightED->DenormalizePercent(m_xHeightED->get_value(FieldUnit::TWIP)));
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

        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_xHorizontalDLB);
        const sal_Int16 eHOri = GetAlignment(m_pHMap, nMapPos, *m_xHoriRelationLB);
        const sal_Int16 eRel = GetRelation(*m_xHoriRelationLB);

        aHoriOrient.SetHoriOrient( eHOri );
        aHoriOrient.SetRelationOrient( eRel );
        aHoriOrient.SetPosToggle(m_xMirrorPagesCB->get_active());

        bool bMod = m_xAtHorzPosED->get_value_changed_from_saved();
        bMod |= m_xMirrorPagesCB->get_state_changed_from_saved();

        if ( eHOri == text::HoriOrientation::NONE &&
             (m_bNew || (m_bAtHorzPosModified || bMod) || m_nOldH != eHOri ) )
        {
            SwTwips nX = static_cast< SwTwips >(m_xAtHorzPosED->denormalize(m_xAtHorzPosED->get_value(FieldUnit::TWIP)));
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

        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_xVerticalDLB);
        const sal_Int16 eVOri = GetAlignment(m_pVMap, nMapPos, *m_xVertRelationLB);
        const sal_Int16 eRel = GetRelation(*m_xVertRelationLB);

        aVertOrient.SetVertOrient    ( eVOri);
        aVertOrient.SetRelationOrient( eRel );

        bool bMod = m_xAtVertPosED->get_value_changed_from_saved();

        if ( eVOri == text::VertOrientation::NONE &&
             ( m_bNew || (m_bAtVertPosModified || bMod) || m_nOldV != eVOri) )
        {
            // vertical position
            // recalculate offset for character bound frames
            SwTwips nY = static_cast< SwTwips >(m_xAtVertPosED->denormalize(m_xAtVertPosED->get_value(FieldUnit::TWIP)));
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

    auto nRelWidthRelation = m_xRelWidthRelationLB->get_active();
    if (nRelWidthRelation != -1)
    {
        if (nRelWidthRelation == 0)
            aSz.SetWidthPercentRelation(text::RelOrientation::FRAME);
        else if (nRelWidthRelation == 1)
            aSz.SetWidthPercentRelation(text::RelOrientation::PAGE_FRAME);
    }
    auto nRelHeightRelation = m_xRelHeightRelationLB->get_active();
    if (nRelHeightRelation != -1)
    {
        if (nRelHeightRelation == 0)
            aSz.SetHeightPercentRelation(text::RelOrientation::FRAME);
        else if (nRelHeightRelation == 1)
            aSz.SetHeightPercentRelation(text::RelOrientation::PAGE_FRAME);
    }

    bool bValueModified = m_xWidthED->get_value_changed_from_saved() ||
                          m_xHeightED->get_value_changed_from_saved();
    bool bCheckChanged = m_xRelWidthCB->get_state_changed_from_saved() ||
                         m_xRelHeightCB->get_state_changed_from_saved();

    bool bLegalValue = !(!rOldSize.GetWidth () && !rOldSize.GetHeight() &&
                            m_xWidthED->get_value() == m_xWidthED->get_min() &&
                            m_xHeightED->get_value() == m_xHeightED->get_min());

    if ((m_bNew && !m_bFormat) || ((bValueModified || bCheckChanged) && bLegalValue))
    {
        sal_Int64 nNewWidth  = m_xWidthED->DenormalizePercent(m_xWidthED->GetRealValue(FieldUnit::TWIP));
        sal_Int64 nNewHeight = m_xHeightED->DenormalizePercent(m_xHeightED->GetRealValue(FieldUnit::TWIP));
        aSz.SetWidth (static_cast< SwTwips >(nNewWidth));
        aSz.SetHeight(static_cast< SwTwips >(nNewHeight));

        if (m_xRelWidthCB->get_active())
        {
            aSz.SetWidthPercent(static_cast<sal_uInt8>(std::min(MAX_PERCENT_WIDTH, m_xWidthED->Convert(m_xWidthED->NormalizePercent(nNewWidth), FieldUnit::TWIP, FieldUnit::PERCENT))));
        }
        else
            aSz.SetWidthPercent(0);
        if (m_xRelHeightCB->get_active())
            aSz.SetHeightPercent(static_cast<sal_uInt8>(std::min(MAX_PERCENT_HEIGHT, m_xHeightED->Convert(m_xHeightED->NormalizePercent(nNewHeight), FieldUnit::TWIP, FieldUnit::PERCENT))));
        else
            aSz.SetHeightPercent(0);

        if (m_xFixedRatioCB->get_active() && (m_xRelWidthCB->get_active() != m_xRelHeightCB->get_active()))
        {
            if (m_xRelWidthCB->get_active())
                aSz.SetHeightPercent(SwFormatFrameSize::SYNCED);
            else
                aSz.SetWidthPercent(SwFormatFrameSize::SYNCED);
        }
    }
    if( !IsInGraficMode() )
    {
        if (m_xAutoHeightCB->get_state_changed_from_saved())
        {
            SwFrameSize eFrameSize = m_xAutoHeightCB->get_active()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrameSize != aSz.GetHeightSizeType() )
                aSz.SetHeightSizeType(eFrameSize);
        }
        if (m_xAutoWidthCB->get_state_changed_from_saved())
        {
            SwFrameSize eFrameSize = m_xAutoWidthCB->get_active()? ATT_MIN_SIZE : ATT_FIX_SIZE;
            if( eFrameSize != aSz.GetWidthSizeType() )
                aSz.SetWidthSizeType( eFrameSize );
        }
    }
    if (!m_bFormat && m_xFixedRatioCB->get_state_changed_from_saved())
        bRet |= nullptr != rSet->Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, m_xFixedRatioCB->get_active()));

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
    if (m_xFollowTextFlowCB->get_state_changed_from_saved())
    {
        bRet |= nullptr != rSet->Put(SwFormatFollowTextFlow(m_xFollowTextFlowCB->get_active()));
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
    auto nPos = m_xVerticalDLB->get_active();
    if (nPos != -1 && m_pVMap)
    {
        m_nOldV    = m_pVMap[nPos].nAlign;

        nPos = m_xVertRelationLB->get_active();
        if (nPos != -1)
            m_nOldVRel = reinterpret_cast<RelationMap*>(m_xVertRelationLB->get_id(nPos).toInt64())->nRelation;
    }

    nPos = m_xHorizontalDLB->get_active();
    if (nPos != -1 && m_pHMap)
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_xHoriRelationLB->get_active();
        if (nPos != -1)
            m_nOldHRel = reinterpret_cast<RelationMap*>(m_xHoriRelationLB->get_id(nPos).toInt64())->nRelation;
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
    m_xHorizontalDLB->set_sensitive( bEnable );
    m_xHorizontalFT->set_sensitive( bEnable );

    // select current Pos
    // horizontal
    if ( nH < 0 )
    {
        nH    = m_nOldH;
        nHRel = m_nOldHRel;
    }
    sal_Int32 nMapPos = FillPosLB(m_pHMap, nH, nHRel, *m_xHorizontalDLB);
    FillRelLB(m_pHMap, nMapPos, nH, nHRel, *m_xHoriRelationLB, *m_xHoriRelationFT);

    // vertical
    if ( nV < 0 )
    {
        nV    = m_nOldV;
        nVRel = m_nOldVRel;
    }
    nMapPos = FillPosLB(m_pVMap, nV, nVRel, *m_xVerticalDLB);
    FillRelLB(m_pVMap, nMapPos, nV, nVRel, *m_xVertRelationLB, *m_xVertRelationFT);

    bEnable = nH == text::HoriOrientation::NONE && eId != RndStdIds::FLY_AS_CHAR;
    if (!bEnable)
        m_xAtHorzPosED->set_value(0, FieldUnit::TWIP);
    else
    {
        if (nX != LONG_MAX)
            m_xAtHorzPosED->set_value(m_xAtHorzPosED->normalize(nX), FieldUnit::TWIP);
    }
    m_xAtHorzPosFT->set_sensitive( bEnable );
    m_xAtHorzPosED->set_sensitive( bEnable );

    bEnable = nV == text::VertOrientation::NONE;
    if ( !bEnable )
        m_xAtVertPosED->set_value(0, FieldUnit::TWIP);
    else
    {
        if (eId == RndStdIds::FLY_AS_CHAR)
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            m_xAtVertPosED->set_value(m_xAtVertPosED->normalize(nY), FieldUnit::TWIP);
    }
    m_xAtVertPosFT->set_sensitive( bEnable && m_bAllowVertPositioning );
    m_xAtVertPosED->set_sensitive( bEnable && m_bAllowVertPositioning );
    UpdateExample();
}

sal_Int32 SwFramePage::FillPosLB(const FrameMap* _pMap,
                            const sal_Int16 _nAlign,
                            const sal_Int16 _nRel,
                            weld::ComboBox& _rLB )
{
    OUString sSelEntry;
    const OUString sOldEntry = _rLB.get_active_text();

    _rLB.clear();

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
        SvxSwFramePosString::StringId eStrId = m_xMirrorPagesCB->get_active() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
        // --> OD 2009-08-31 #mongolianlayout#
        eStrId = lcl_ChangeResIdToVerticalOrRTL( eStrId,
                                                 m_bIsVerticalFrame,
                                                 m_bIsVerticalL2R,
                                                 m_bIsInRightToLeft);
        OUString sEntry(SvxSwFramePosString::GetString(eStrId));
        if (_rLB.find_text(sEntry) == -1)
        {
            // don't insert entries when frames are character bound
            _rLB.append_text(sEntry);
        }
        // i#22341 - add condition to handle map <aVCharMap>
        // that is ambiguous in the alignment.
        if ( _pMap[i].nAlign == _nAlign &&
             ( (_pMap != aVCharMap) || _pMap[i].nLBRelations & nLBRelations ) )
        {
            sSelEntry = sEntry;
        }
    }

    _rLB.set_active_text(sSelEntry);
    if (_rLB.get_active() == -1)
        _rLB.set_active_text(sOldEntry);

    if (_rLB.get_active() == -1)
        _rLB.set_active(0);

    PosHdl(_rLB);

    return GetMapPos(_pMap, _rLB);
}

void SwFramePage::FillRelLB(const FrameMap* _pMap,
                            const sal_uInt16 _nLBSelPos,
                            const sal_Int16 _nAlign,
                            const sal_Int16 _nRel,
                            weld::ComboBox& _rLB,
                            weld::Label& _rFT)
{
    OUString sSelEntry;
    LB       nLBRelations = LB::NONE;
    size_t   nMapCount = ::lcl_GetFrameMapCount(_pMap);

    _rLB.clear();

    if (_nLBSelPos < nMapCount)
    {
        if (_pMap == aVAsCharHtmlMap || _pMap == aVAsCharMap)
        {
            const OUString sOldEntry(_rLB.get_active_text());
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
                            _rLB.append(OUString::number(reinterpret_cast<sal_Int64>(&rCharMap)), sEntry);
                            if (_pMap[nMapPos].nAlign == _nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                _rLB.set_active_text(sSelEntry);
            else
            {
                _rLB.set_active_text(sOldEntry);

                if (_rLB.get_active() == -1)
                {
                    for (int i = 0; i < _rLB.get_count(); i++)
                    {
                        RelationMap *pEntry = reinterpret_cast<RelationMap*>(_rLB.get_id(i).toInt64());
                        if (pEntry->nLBRelation == LB::RelChar) // default
                        {
                            _rLB.set_active(i);
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
                                             ( m_xMirrorPagesCB->get_active()
                                               ? _pMap[_nLBSelPos].eMirrorStrId
                                               : _pMap[_nLBSelPos].eStrId),
                                             m_xMirrorPagesCB->get_active() );
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
                            SvxSwFramePosString::StringId eStrId1 = m_xMirrorPagesCB->get_active() ?
                                            rMap.eMirrorStrId : rMap.eStrId;
                            // --> OD 2009-08-31 #mongolianlayout#
                            eStrId1 =
                                lcl_ChangeResIdToVerticalOrRTL( eStrId1,
                                                                m_bIsVerticalFrame,
                                                                m_bIsVerticalL2R,
                                                                m_bIsInRightToLeft);
                            const OUString sEntry = SvxSwFramePosString::GetString(eStrId1);
                            _rLB.append(OUString::number(reinterpret_cast<sal_Int64>(&rMap)), sEntry);
                            if (sSelEntry.isEmpty() && rMap.nRelation == _nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                _rLB.set_active_text(sSelEntry);
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
                        if (_rLB.get_active() != -1)
                        {
                            RelationMap *pEntry = reinterpret_cast<RelationMap*>(_rLB.get_id(_rLB.get_count() - 1).toInt64());
                            nSimRel = pEntry->nRelation;
                        }
                        break;
                }

                for (int i = 0; i < _rLB.get_count(); i++)
                {
                    RelationMap *pEntry = reinterpret_cast<RelationMap*>(_rLB.get_id(i).toInt64());
                    if (pEntry->nRelation == nSimRel)
                    {
                        _rLB.set_active(i);
                        break;
                    }
                }

                if (_rLB.get_active() == -1)
                    _rLB.set_active(0);
            }
        }
    }

    const bool bEnable = _rLB.get_count() != 0
            && (&_rLB != m_xVertRelationLB.get() || m_bAllowVertPositioning);
    _rLB.set_sensitive( bEnable );
    _rFT.set_sensitive( bEnable );

    RelHdl(_rLB);
}

sal_Int16 SwFramePage::GetRelation(const weld::ComboBox& rRelationLB)
{
    const auto nPos = rRelationLB.get_active();
    if (nPos != -1)
    {
        RelationMap *pEntry = reinterpret_cast<RelationMap *>(rRelationLB.get_id(nPos).toInt64());
        return pEntry->nRelation;
    }

    return 0;
}

sal_Int16 SwFramePage::GetAlignment(FrameMap const *pMap, sal_Int32 nMapPos,
                                    const weld::ComboBox& rRelationLB)
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

    if (rRelationLB.get_active() == -1)
        return 0;

    const RelationMap *const pRelationMap = reinterpret_cast<const RelationMap *>(
        rRelationLB.get_active_id().toInt64());
    const LB nRel = pRelationMap->nLBRelation;
    const SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

    for (size_t i = 0; i < nMapCount; ++i)
    {
        if (pMap[i].eStrId == eStrId && (pMap[i].nLBRelations & nRel))
            return pMap[i].nAlign;
    }

    return 0;
}

sal_Int32 SwFramePage::GetMapPos(const FrameMap *pMap, const weld::ComboBox& rAlignLB)
{
    sal_Int32 nMapPos = 0;
    auto nLBSelPos = rAlignLB.get_active();

    if (nLBSelPos != -1)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            const size_t nMapCount = ::lcl_GetFrameMapCount(pMap);
            const OUString sSelEntry(rAlignLB.get_active_text());

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
    if (m_xAnchorAtParaRB->get_active())
    {
        nRet = RndStdIds::FLY_AT_PARA;
    }
    else if (m_xAnchorAtCharRB->get_active())
    {
        nRet = RndStdIds::FLY_AT_CHAR;
    }
    else if (m_xAnchorAsCharRB->get_active())
    {
        nRet = RndStdIds::FLY_AS_CHAR;
    }
    else if (m_xAnchorAtFrameRB->get_active())
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
    m_xWidthED->LockAutoCalculation(true);
    m_xHeightED->LockAutoCalculation(true);
    RangeModifyHdl();  // set all maximum values initially
    m_xHeightED->LockAutoCalculation(false);
    m_xWidthED->LockAutoCalculation(false);
    m_xFollowTextFlowCB->save_state();
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
IMPL_LINK_NOARG(SwFramePage, MirrorHdl, weld::ToggleButton&, void)
{
    RndStdIds eId = GetAnchor();
    InitPos(eId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
}

IMPL_LINK( SwFramePage, RelSizeClickHdl, weld::ToggleButton&, rBtn, void )
{
    if (&rBtn == m_xRelWidthCB.get())
    {
        m_xWidthED->ShowPercent(rBtn.get_active());
        m_xRelWidthRelationLB->set_sensitive(rBtn.get_active());
        if (rBtn.get_active())
            m_xWidthED->get()->set_max(MAX_PERCENT_WIDTH, FieldUnit::NONE);
    }
    else // rBtn == m_xRelHeightCB.get()
    {
        m_xHeightED->ShowPercent(rBtn.get_active());
        m_xRelHeightRelationLB->set_sensitive(rBtn.get_active());
        if (rBtn.get_active())
            m_xHeightED->get()->set_max(MAX_PERCENT_HEIGHT, FieldUnit::NONE);
    }

    RangeModifyHdl();  // correct the values again

    if (&rBtn == m_xRelWidthCB.get())
        ModifyHdl(*m_xWidthED->get());
    else // rBtn == m_xRelHeightCB.get()
        ModifyHdl(*m_xHeightED->get());
}

// range check
IMPL_LINK_NOARG(SwFramePage, RangeModifyClickHdl, weld::ToggleButton&, void)
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
    aVal.bAutoHeight = m_xAutoHeightCB->get_active();
    aVal.bMirror = m_xMirrorPagesCB->get_active();
    aVal.bFollowTextFlow = m_xFollowTextFlowCB->get_active();

    if ( m_pHMap )
    {
        // alignment horizontal
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_xHorizontalDLB);
        aVal.nHoriOrient = GetAlignment(m_pHMap, nMapPos, *m_xHoriRelationLB);
        aVal.nHRelOrient = GetRelation(*m_xHoriRelationLB);
    }
    else
        aVal.nHoriOrient = text::HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // alignment vertical
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_xVerticalDLB);
        aVal.nVertOrient = GetAlignment(m_pVMap, nMapPos, *m_xVertRelationLB);
        aVal.nVRelOrient = GetRelation(*m_xVertRelationLB);
    }
    else
        aVal.nVertOrient = text::VertOrientation::NONE;

    const long nAtHorzPosVal = static_cast< long >(
                    m_xAtHorzPosED->denormalize(m_xAtHorzPosED->get_value(FieldUnit::TWIP)) );
    const long nAtVertPosVal = static_cast< long >(
                    m_xAtVertPosED->denormalize(m_xAtVertPosED->get_value(FieldUnit::TWIP)) );

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    aMgr.ValidateMetrics(aVal, mpToCharContentPos, true);   // one time, to get reference values for percental values

    // set reference values for percental values (100%) ...
    m_xWidthED->SetRefValue(aVal.aPercentSize.Width());
    m_xHeightED->SetRefValue(aVal.aPercentSize.Height());

    // ... and correctly convert width and height with it
    SwTwips nWidth  = static_cast< SwTwips >(m_xWidthED->DenormalizePercent(m_xWidthED->get_value(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_xHeightED->DenormalizePercent(m_xHeightED->get_value(FieldUnit::TWIP)));
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
    m_xHeightED->set_min(m_xHeightED->NormalizePercent(aVal.nMinHeight), FieldUnit::TWIP);
    m_xWidthED->set_min(m_xWidthED->NormalizePercent(aVal.nMinWidth), FieldUnit::TWIP);

    SwTwips nMaxWidth(aVal.nMaxWidth);
    SwTwips nMaxHeight(aVal.nMaxHeight);

    if (aVal.bAutoHeight && (m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"))
    {
        SwTwips nTmp = std::min(nWidth * nMaxHeight / std::max(nHeight, 1L), nMaxHeight);
        m_xWidthED->set_max(m_xWidthED->NormalizePercent(nTmp), FieldUnit::TWIP);

        nTmp = std::min(nHeight * nMaxWidth / std::max(nWidth, 1L), nMaxWidth);
        m_xHeightED->set_max(m_xWidthED->NormalizePercent(nTmp), FieldUnit::TWIP);
    }
    else
    {
        SwTwips nTmp = static_cast< SwTwips >(m_xHeightED->NormalizePercent(nMaxHeight));
        m_xHeightED->set_max(nTmp, FieldUnit::TWIP);

        nTmp = static_cast< SwTwips >(m_xWidthED->NormalizePercent(nMaxWidth));
        m_xWidthED->set_max(nTmp, FieldUnit::TWIP);
    }

    m_xAtHorzPosED->set_range(m_xAtHorzPosED->normalize(aVal.nMinHPos),
                              m_xAtHorzPosED->normalize(aVal.nMaxHPos),
                              FieldUnit::TWIP);
    if (aVal.nHPos != nAtHorzPosVal)
        m_xAtHorzPosED->set_value(m_xAtHorzPosED->normalize(aVal.nHPos), FieldUnit::TWIP);

    const SwTwips nUpperOffset = (aVal.nAnchorType == RndStdIds::FLY_AS_CHAR)
        ? m_nUpperBorder : 0;
    const SwTwips nLowerOffset = (aVal.nAnchorType == RndStdIds::FLY_AS_CHAR)
        ? m_nLowerBorder : 0;

    m_xAtVertPosED->set_range(m_xAtVertPosED->normalize(aVal.nMinVPos + nLowerOffset + nUpperOffset),
                              m_xAtVertPosED->normalize(aVal.nMaxVPos),
                              FieldUnit::TWIP);
    if (aVal.nVPos != nAtVertPosVal)
        m_xAtVertPosED->set_value(m_xAtVertPosED->normalize(aVal.nVPos), FieldUnit::TWIP);
}

IMPL_LINK_NOARG(SwFramePage, AnchorTypeHdl, weld::ToggleButton&, void)
{
    m_xMirrorPagesCB->set_sensitive(!m_xAnchorAsCharRB->get_active());

    // i#18732 - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    // i#22305 - enable check box 'Follow text
    // flow' also for anchor type to-frame.
    m_xFollowTextFlowCB->set_sensitive(m_xAnchorAtParaRB->get_active() ||
                                       m_xAnchorAtCharRB->get_active() ||
                                       m_xAnchorAtFrameRB->get_active());

    RndStdIds eId = GetAnchor();

    InitPos( eId, -1, 0, -1, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl();

    if(m_bHtmlMode)
    {
        PosHdl(*m_xHorizontalDLB);
        PosHdl(*m_xVerticalDLB);
    }

    EnableVerticalPositioning( !(m_bIsMathOLE && m_bIsMathBaselineAlignment
            && RndStdIds::FLY_AS_CHAR == eId) );
}

IMPL_LINK( SwFramePage, PosHdl, weld::ComboBox&, rLB, void )
{
    bool bHori = &rLB == m_xHorizontalDLB.get();
    weld::ComboBox *pRelLB = bHori ? m_xHoriRelationLB.get() : m_xVertRelationLB.get();
    weld::Label *pRelFT = bHori ? m_xHoriRelationFT.get() : m_xVertRelationFT.get();
    FrameMap const *pMap = bHori ? m_pHMap : m_pVMap;

    const sal_Int32 nMapPos = GetMapPos(pMap, rLB);
    const sal_Int16 nAlign = GetAlignment(pMap, nMapPos, *pRelLB);

    if (bHori)
    {
        bool bEnable = text::HoriOrientation::NONE == nAlign;
        m_xAtHorzPosED->set_sensitive( bEnable );
        m_xAtHorzPosFT->set_sensitive( bEnable );
    }
    else
    {
        bool bEnable = text::VertOrientation::NONE == nAlign && m_bAllowVertPositioning;
        m_xAtVertPosED->set_sensitive( bEnable );
        m_xAtVertPosFT->set_sensitive( bEnable );
    }

    RangeModifyHdl();

    sal_Int16 nRel = 0;
    if (rLB.get_active() != -1)
    {
        if (pRelLB->get_active() != -1)
            nRel = reinterpret_cast<RelationMap*>(pRelLB->get_active_id().toInt64())->nRelation;
        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->clear();

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
                    0 == m_xVerticalDLB->get_active())
            {
                if(text::RelOrientation::FRAME == nRel)
                    m_xVerticalDLB->set_active(1);
                else
                    m_xVerticalDLB->set_active(0);
                bSet = true;
            }
            else if(text::HoriOrientation::LEFT == nAlign && 1 == m_xVerticalDLB->get_active())
            {
                m_xVerticalDLB->set_active(0);
                bSet = true;
            }
            else if(text::HoriOrientation::NONE == nAlign && 1 == m_xVerticalDLB->get_active())
            {
                m_xVerticalDLB->set_active(0);
                bSet = true;
            }
            if(bSet)
                PosHdl(*m_xVerticalDLB);
        }
        else
        {
            if(text::VertOrientation::TOP == nAlign)
            {
                if (1 == m_xHorizontalDLB->get_active())
                {
                    m_xHorizontalDLB->set_active(0);
                    bSet = true;
                }
                m_xHoriRelationLB->set_active(1);
            }
            else if(text::VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if (2 == m_xHorizontalDLB->get_active())
                {
                    m_xHorizontalDLB->set_active(0);
                    bSet = true;
                }
                m_xHoriRelationLB->set_active(0) ;
            }
            if(bSet)
                PosHdl(*m_xHorizontalDLB);
        }

    }
}

//  horizontal Pos
IMPL_LINK( SwFramePage, RelHdl, weld::ComboBox&, rLB, void )
{
    bool bHori = &rLB == m_xHoriRelationLB.get();

    UpdateExample();

    if (bHori)
        m_bAtHorzPosModified = true;
    else
        m_bAtVertPosModified = true;

    if (m_bHtmlMode && (RndStdIds::FLY_AT_CHAR == GetAnchor()))
    {
        if(bHori)
        {
            const sal_Int16 nRel = GetRelation(*m_xHoriRelationLB);
            if(text::RelOrientation::PRINT_AREA == nRel && 0 == m_xVerticalDLB->get_active())
            {
                m_xVerticalDLB->set_active(1);
            }
            else if(text::RelOrientation::CHAR == nRel && 1 == m_xVerticalDLB->get_active())
            {
                m_xVerticalDLB->set_active(0);
            }
        }
    }
    RangeModifyHdl();
}

IMPL_LINK_NOARG(SwFramePage, RealSizeHdl, weld::Button&, void)
{
    m_xWidthED->set_value(m_xWidthED->NormalizePercent(m_aGrfSize.Width()), FieldUnit::TWIP);
    m_xHeightED->set_value(m_xHeightED->NormalizePercent(m_aGrfSize.Height()), FieldUnit::TWIP);
    m_fWidthHeightRatio = m_aGrfSize.Height() ? double(m_aGrfSize.Width()) / double(m_aGrfSize.Height()) : 1.0;
    UpdateExample();
}

IMPL_LINK_NOARG(SwFramePage, AutoWidthClickHdl, weld::ToggleButton&, void)
{
    if( !IsInGraficMode() )
        HandleAutoCB( m_xAutoWidthCB->get_active(), *m_xWidthFT, *m_xWidthAutoFT, *m_xWidthED->get() );
}

IMPL_LINK_NOARG(SwFramePage, AutoHeightClickHdl, weld::ToggleButton&, void)
{
    if (!IsInGraficMode())
        HandleAutoCB(m_xAutoHeightCB->get_active(), *m_xHeightFT, *m_xHeightAutoFT, *m_xWidthED->get());
}

IMPL_LINK( SwFramePage, ModifyHdl, weld::MetricSpinButton&, rEdit, void )
{
    SwTwips nWidth  = static_cast< SwTwips >(m_xWidthED->DenormalizePercent(m_xWidthED->get_value(FieldUnit::TWIP)));
    SwTwips nHeight = static_cast< SwTwips >(m_xHeightED->DenormalizePercent(m_xHeightED->get_value(FieldUnit::TWIP)));
    if (m_xFixedRatioCB->get_active())
    {
        if (&rEdit == m_xWidthED->get())
        {
            nHeight = SwTwips(static_cast<double>(nWidth) / m_fWidthHeightRatio);
            m_xHeightED->set_value(m_xHeightED->NormalizePercent(nHeight), FieldUnit::TWIP);
        }
        else if (&rEdit == m_xHeightED->get())
        {
            nWidth = SwTwips(static_cast<double>(nHeight) * m_fWidthHeightRatio);
            m_xWidthED->set_value(m_xWidthED->NormalizePercent(nWidth), FieldUnit::TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
}

void SwFramePage::UpdateExample()
{
    auto nPos = m_xHorizontalDLB->get_active();
    if (m_pHMap && nPos != -1)
    {
        const sal_Int32 nMapPos = GetMapPos(m_pHMap, *m_xHorizontalDLB);
        m_aExampleWN.SetHAlign(GetAlignment(m_pHMap, nMapPos, *m_xHoriRelationLB));
        m_aExampleWN.SetHoriRel(GetRelation(*m_xHoriRelationLB));
    }

    nPos = m_xVerticalDLB->get_active();
    if (m_pVMap && nPos != -1)
    {
        const sal_Int32 nMapPos = GetMapPos(m_pVMap, *m_xVerticalDLB);
        m_aExampleWN.SetVAlign(GetAlignment(m_pVMap, nMapPos, *m_xVertRelationLB));
        m_aExampleWN.SetVertRel(GetRelation(*m_xVertRelationLB));
    }

    // size
    auto nXPos = m_xAtHorzPosED->denormalize(m_xAtHorzPosED->get_value(FieldUnit::TWIP));
    auto nYPos = m_xAtVertPosED->denormalize(m_xAtVertPosED->get_value(FieldUnit::TWIP));
    m_aExampleWN.SetRelPos(Point(nXPos, nYPos));

    m_aExampleWN.SetAnchor(GetAnchor());
    m_aExampleWN.Invalidate();
}

void SwFramePage::Init(const SfxItemSet& rSet)
{
    if(!m_bFormat)
    {
        SwWrtShell* pSh = getFrameDlgParentShell();

        // size
        const bool bSizeFixed = pSh->IsSelObjProtected( FlyProtectFlags::Fixed ) != FlyProtectFlags::NONE;

        m_xWidthED->set_sensitive( !bSizeFixed );
        m_xHeightED->set_sensitive( !bSizeFixed );

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
                    m_xWidthFT->set_sensitive(false);
                    m_xWidthED->set_sensitive(false);
                    m_xRelWidthCB->set_sensitive(false);
                    m_xHeightFT->set_sensitive(false);
                    m_xHeightED->set_sensitive(false);
                    m_xRelHeightCB->set_sensitive(false);
                    m_xFixedRatioCB->set_sensitive(false);
                    m_xRealSizeBT->set_sensitive(false);
                    break;
                }
            }

            // TODO/LATER: get correct aspect
            if(0 != (pSh->GetOLEObject()->getStatus( embed::Aspects::MSOLE_CONTENT ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
                m_xRealSizeBT->set_sensitive(false);
        }
    }

    const SwFormatFrameSize& rSize = rSet.Get(RES_FRM_SIZE);
    sal_Int64 nWidth  = m_xWidthED->NormalizePercent(rSize.GetWidth());
    sal_Int64 nHeight = m_xHeightED->NormalizePercent(rSize.GetHeight());

    if (nWidth != m_xWidthED->get_value(FieldUnit::TWIP))
        m_xWidthED->set_value(nWidth, FieldUnit::TWIP);

    if (nHeight != m_xHeightED->get_value(FieldUnit::TWIP))
        m_xHeightED->set_value(nHeight, FieldUnit::TWIP);

    if (!IsInGraficMode())
    {
        SwFrameSize eSize = rSize.GetHeightSizeType();
        bool bCheck = eSize != ATT_FIX_SIZE;
        m_xAutoHeightCB->set_active(bCheck);
        HandleAutoCB( bCheck, *m_xHeightFT, *m_xHeightAutoFT, *m_xWidthED->get() );
        if( eSize == ATT_VAR_SIZE )
            m_xHeightED->set_value(m_xHeightED->get_min());

        eSize = rSize.GetWidthSizeType();
        bCheck = eSize != ATT_FIX_SIZE;
        m_xAutoWidthCB->set_active(bCheck);
        HandleAutoCB( bCheck, *m_xWidthFT, *m_xWidthAutoFT, *m_xWidthED->get() );
        if( eSize == ATT_VAR_SIZE )
            m_xWidthED->set_value(m_xWidthED->get_min());

        if ( !m_bFormat )
        {
            SwWrtShell* pSh = getFrameDlgParentShell();
            const SwFrameFormat* pFormat = pSh->GetFlyFrameFormat();
            if( pFormat && pFormat->GetChain().GetNext() )
                m_xAutoHeightCB->set_sensitive( false );
        }
    }
    else
        m_xAutoHeightCB->hide();

    // organise circulation-gap for character bound frames
    const SvxULSpaceItem &rUL = rSet.Get(RES_UL_SPACE);
    m_nUpperBorder = rUL.GetUpper();
    m_nLowerBorder = rUL.GetLower();

    if(SfxItemState::SET == rSet.GetItemState(FN_KEEP_ASPECT_RATIO))
    {
        m_xFixedRatioCB->set_active(static_cast<const SfxBoolItem&>(rSet.Get(FN_KEEP_ASPECT_RATIO)).GetValue());
        m_xFixedRatioCB->save_state();
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

        m_xMirrorPagesCB->set_active(rHori.IsPosToggle());
        m_xMirrorPagesCB->save_state();

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
    m_aExampleWN.SetWrap( rSurround.GetSurround() );

    if ( rSurround.GetSurround() == css::text::WrapTextMode_THROUGH )
    {
        const SvxOpaqueItem& rOpaque = rSet.Get(RES_OPAQUE);
        m_aExampleWN.SetTransparent(!rOpaque.GetValue());
    }

    // switch to percent if applicable
    RangeModifyHdl();  // set reference values (for 100%)

    if (rSize.GetWidthPercent() == SwFormatFrameSize::SYNCED || rSize.GetHeightPercent() == SwFormatFrameSize::SYNCED)
        m_xFixedRatioCB->set_active(true);
    if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED &&
        !m_xRelWidthCB->get_active())
    {
        m_xRelWidthCB->set_active(true);
        RelSizeClickHdl(*m_xRelWidthCB);
        m_xWidthED->set_value(rSize.GetWidthPercent(), FieldUnit::PERCENT);
    }
    if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != SwFormatFrameSize::SYNCED &&
        !m_xRelHeightCB->get_active())
    {
        m_xRelHeightCB->set_active(true);
        RelSizeClickHdl(*m_xRelHeightCB);
        m_xHeightED->set_value(rSize.GetHeightPercent(), FieldUnit::PERCENT);
    }
    m_xRelWidthCB->save_state();
    m_xRelHeightCB->save_state();

    if (rSize.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
        m_xRelWidthRelationLB->set_active(1);
    else
        m_xRelWidthRelationLB->set_active(0);

    if (rSize.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
        m_xRelHeightRelationLB->set_active(1);
    else
        m_xRelHeightRelationLB->set_active(0);
}

void SwFramePage::SetFormatUsed(bool bFormatUsed)
{
    m_bFormat = bFormatUsed;
    if (m_bFormat)
        m_xAnchorFrame->hide();
}

void SwFramePage::EnableVerticalPositioning( bool bEnable )
{
    m_bAllowVertPositioning = bEnable;
    m_xVerticalFT->set_sensitive( bEnable );
    m_xVerticalDLB->set_sensitive( bEnable );
    m_xAtVertPosFT->set_sensitive( bEnable );
    m_xAtVertPosED->set_sensitive( bEnable );
    m_xVertRelationFT->set_sensitive( bEnable );
    m_xVertRelationLB->set_sensitive( bEnable );
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
        size_t nCount = pList->size();
        for (size_t i = 0; i < nCount; ++i)
        {
            m_xFrameCB->append_text(pList->at(i));
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
    , m_xDescriptionED(m_xBuilder->weld_text_view("description"))
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
            aPrevPageFrames.clear();
            aNextPageFrames.clear();
            aThisPageFrames.clear();
            aRemainFrames.clear();

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
