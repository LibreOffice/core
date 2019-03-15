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

#include <memory>
#include <cstddef>
#include <swpossizetabpage.hxx>
#include <svx/dlgutil.hxx>
#include <svx/anchorid.hxx>
#include <svl/aeitem.hxx>
#include <svx/swframevalidation.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/rectenum.hxx>
#include <sal/macros.h>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::text;

#define SwFPos SvxSwFramePosString

enum class LB;

struct FrmMap
{
    SvxSwFramePosString::StringId   eStrId;
    SvxSwFramePosString::StringId   eMirrorStrId;
    short                           nAlign;
    LB                              nLBRelations;
};

struct RelationMap
{
    SvxSwFramePosString::StringId   eStrId;
    SvxSwFramePosString::StringId   eMirrorStrId;
    LB                              nLBRelation;
    short nRelation;
};
struct StringIdPair_Impl
{
    SvxSwFramePosString::StringId eHori;
    SvxSwFramePosString::StringId eVert;
};

enum class LB {
    NONE                = 0x000000,
    Frame               = 0x000001, // paragraph text area
    PrintArea           = 0x000002, // paragraph text area + indents
    VertFrame           = 0x000004, // vertical paragraph text area
    VertPrintArea       = 0x000008, // vertical paragraph text area + indents
    RelFrameLeft        = 0x000010, // left paragraph margin
    RelFrameRight       = 0x000020, // right paragraph margin

    RelPageLeft         = 0x000040, // left page margin
    RelPageRight        = 0x000080, // right page margin
    RelPageFrame        = 0x000100, // complete page
    RelPagePrintArea    = 0x000200, // text area of page

    FlyRelPageLeft      = 0x000400, // left frame margin
    FlyRelPageRight     = 0x000800, // right frame margin
    FlyRelPageFrame     = 0x001000, // complete frame
    FlyRelPagePrintArea = 0x002000, // frame interior

    RelBase             = 0x004000, // as char, relative to baseline
    RelChar             = 0x008000, // as char, relative to character
    RelRow              = 0x010000, // as char, relative to line

// #i22305#
    FlyVertFrame        = 0x020000, // vertical entire frame
    FlyVertPrintArea    = 0x040000, // vertical frame text area

// #i22341#
    VertLine            = 0x080000, // vertical text line

    LAST = VertLine
};
namespace o3tl {
    template<> struct typed_flags<LB> : is_typed_flags<LB, 0x0fffff> {};
}

static RelationMap const aRelationMap[] =
{
    {SwFPos::FRAME,         SwFPos::FRAME,             LB::Frame,           RelOrientation::FRAME},
    {SwFPos::PRTAREA,       SwFPos::PRTAREA,           LB::PrintArea,         RelOrientation::PRINT_AREA},
    {SwFPos::REL_PG_LEFT,   SwFPos::MIR_REL_PG_LEFT,   LB::RelPageLeft,     RelOrientation::PAGE_LEFT},
    {SwFPos::REL_PG_RIGHT,  SwFPos::MIR_REL_PG_RIGHT,  LB::RelPageRight,    RelOrientation::PAGE_RIGHT},
    {SwFPos::REL_FRM_LEFT,  SwFPos::MIR_REL_FRM_LEFT,  LB::RelFrameLeft,    RelOrientation::FRAME_LEFT},
    {SwFPos::REL_FRM_RIGHT, SwFPos::MIR_REL_FRM_RIGHT, LB::RelFrameRight,   RelOrientation::FRAME_RIGHT},
    {SwFPos::REL_PG_FRAME,  SwFPos::REL_PG_FRAME,      LB::RelPageFrame,    RelOrientation::PAGE_FRAME},
    {SwFPos::REL_PG_PRTAREA,SwFPos::REL_PG_PRTAREA,    LB::RelPagePrintArea,  RelOrientation::PAGE_PRINT_AREA},
    {SwFPos::REL_CHAR,      SwFPos::REL_CHAR,          LB::RelChar,        RelOrientation::CHAR},

    {SwFPos::FLY_REL_PG_LEFT,       SwFPos::FLY_MIR_REL_PG_LEFT,    LB::FlyRelPageLeft,     RelOrientation::PAGE_LEFT},
    {SwFPos::FLY_REL_PG_RIGHT,      SwFPos::FLY_MIR_REL_PG_RIGHT,   LB::FlyRelPageRight,    RelOrientation::PAGE_RIGHT},
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB::FlyRelPageFrame,    RelOrientation::PAGE_FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB::FlyRelPagePrintArea,  RelOrientation::PAGE_PRINT_AREA},

    {SwFPos::REL_BORDER,        SwFPos::REL_BORDER,             LB::VertFrame,          RelOrientation::FRAME},
    {SwFPos::REL_PRTAREA,       SwFPos::REL_PRTAREA,            LB::VertPrintArea,        RelOrientation::PRINT_AREA},

    // #i22305#
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB::FlyVertFrame,      RelOrientation::FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB::FlyVertPrintArea,    RelOrientation::PRINT_AREA},

    // #i22341#
    {SwFPos::REL_LINE,  SwFPos::REL_LINE,   LB::VertLine,   RelOrientation::TEXT_LINE}
};

static RelationMap const aAsCharRelationMap[] =
{
    {SwFPos::REL_BASE,  SwFPos::REL_BASE,   LB::RelBase,   RelOrientation::FRAME},
    {SwFPos::REL_CHAR,  SwFPos::REL_CHAR,   LB::RelChar,   RelOrientation::FRAME},
    {SwFPos::REL_ROW,   SwFPos::REL_ROW,   LB::RelRow,     RelOrientation::FRAME}
};

/*--------------------------------------------------------------------
    Anchored at page
 --------------------------------------------------------------------*/

static constexpr auto HORI_PAGE_REL = LB::RelPageFrame|LB::RelPagePrintArea|LB::RelPageLeft|
                                      LB::RelPageRight;

static FrmMap const aHPageMap[] =
{
    {SwFPos::LEFT,       SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,      SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,   SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrmMap const aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB::RelPageFrame}
};

#define VERT_PAGE_REL   (LB::RelPageFrame|LB::RelPagePrintArea)

static FrmMap const aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrmMap const aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP, SwFPos::FROMTOP,        VertOrientation::NONE,      LB::RelPageFrame}
};

/*--------------------------------------------------------------------
    Anchored at frame
 --------------------------------------------------------------------*/

static constexpr auto HORI_FRAME_REL = LB::FlyRelPageFrame|LB::FlyRelPagePrintArea|
                                       LB::FlyRelPageLeft|LB::FlyRelPageRight;

static FrmMap const aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrmMap const aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      LB::FlyRelPageFrame},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB::FlyRelPageFrame}
};

// #i18732# - own vertical alignment map for to frame anchored objects
// #i22305#
#define VERT_FRAME_REL   (LB::VertFrame|LB::FlyVertPrintArea)

static FrmMap const aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrmMap const aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB::FlyVertFrame},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      LB::FlyVertFrame}
};

static FrmMap const aVMultiSelectionMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      LB::NONE}
};
static FrmMap const aHMultiSelectionMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::FROMLEFT,       HoriOrientation::NONE,      LB::NONE}
};

/*--------------------------------------------------------------------
    Anchored at paragraph
 --------------------------------------------------------------------*/

static constexpr auto HORI_PARA_REL = LB::Frame|LB::PrintArea|LB::RelPageLeft|LB::RelPageRight|
                                      LB::RelPageFrame|LB::RelPagePrintArea|LB::RelFrameLeft|
                                      LB::RelFrameRight;

static FrmMap const aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB::Frame|LB::PrintArea)

static FrmMap const aHParaHtmlMap[] =
{
    {SwFPos::LEFT,  SwFPos::LEFT,   HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT, SwFPos::RIGHT,  HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

static FrmMap const aHParaHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};


static constexpr auto VERT_PARA_REL = LB::VertFrame|LB::VertPrintArea|
                                      LB::RelPageFrame|LB::RelPagePrintArea;

static FrmMap const aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PARA_REL}
};

static FrmMap const aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB::VertPrintArea}
};

/*--------------------------------------------------------------------
    Anchored at character
 --------------------------------------------------------------------*/

static constexpr auto HORI_CHAR_REL = LB::Frame|LB::PrintArea|LB::RelPageLeft|LB::RelPageRight|
                                      LB::RelPageFrame|LB::RelPagePrintArea|LB::RelFrameLeft|
                                      LB::RelFrameRight|LB::RelChar;

static FrmMap aHCharMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_CHAR_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_CHAR_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_CHAR_REL}
};

#define HTML_HORI_CHAR_REL  (LB::Frame|LB::PrintArea|LB::RelChar)

static FrmMap aHCharHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::LEFT,           HoriOrientation::LEFT,      HTML_HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::RIGHT,          HoriOrientation::RIGHT,     HTML_HORI_CHAR_REL}
};

static FrmMap aHCharHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,          LB::PrintArea|LB::RelChar},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     LB::PrintArea},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB::RelPageFrame}
};

// #i18732# - allow vertical alignment at page areas
// #i22341# - handle <LB::RelChar> on its own
static constexpr auto VERT_CHAR_REL = LB::VertFrame|LB::VertPrintArea|
                                      LB::RelPageFrame|LB::RelPagePrintArea;

static FrmMap aVCharMap[] =
{
    // #i22341#
    // introduce mappings for new vertical alignment at top of line <LB::VertLine>
    // and correct mapping for vertical alignment at character for position <FROM_BOTTOM>
    // Note: because of these adjustments the map becomes ambiguous in its values
    //       <eStrId>/<eMirrorStrId> and <nAlign>. These ambiguities are considered
    //       in the methods <SwFrmPage::FillRelLB(..)>, <SwFrmPage::GetAlignment(..)>
    //       and <SwFrmPage::FillPosLB(..)>
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           VERT_CHAR_REL|LB::RelChar},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,        VERT_CHAR_REL|LB::RelChar},
    {SwFPos::BELOW,         SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB::RelChar},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        VERT_CHAR_REL|LB::RelChar},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,          VERT_CHAR_REL},
    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     VertOrientation::NONE,          LB::RelChar|LB::VertLine},
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB::VertLine},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB::VertLine},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB::VertLine}
};


static FrmMap const aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};

static FrmMap const aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,    SwFPos::TOP,            VertOrientation::TOP,           LB::RelChar},
    {SwFPos::BELOW,  SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};
/*--------------------------------------------------------------------
    anchored as character
 --------------------------------------------------------------------*/

static FrmMap const aVAsCharMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           LB::RelBase},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,        LB::RelBase},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        LB::RelBase},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::CHAR_TOP,      LB::RelChar},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::CHAR_BOTTOM,   LB::RelChar},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CHAR_CENTER,   LB::RelChar},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB::RelRow},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB::RelRow},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB::RelRow},

    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     VertOrientation::NONE,          LB::RelBase}
};

static FrmMap const aVAsCharHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           LB::RelBase},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        LB::RelBase},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::CHAR_TOP,      LB::RelChar},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB::RelRow},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB::RelRow},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB::RelRow}
};

static std::size_t lcl_GetFrmMapCount(const FrmMap* pMap)
{
    if( pMap )
    {
        if( pMap == aVParaHtmlMap )
            return SAL_N_ELEMENTS(aVParaHtmlMap);
        if( pMap == aVAsCharHtmlMap )
            return SAL_N_ELEMENTS( aVAsCharHtmlMap );
        if( pMap == aHParaHtmlMap )
            return SAL_N_ELEMENTS( aHParaHtmlMap );
        if( pMap == aHParaHtmlAbsMap )
            return SAL_N_ELEMENTS( aHParaHtmlAbsMap );
        if( pMap == aVPageMap )
            return SAL_N_ELEMENTS( aVPageMap );
        if( pMap == aVPageHtmlMap )
            return SAL_N_ELEMENTS( aVPageHtmlMap );
        if( pMap == aVAsCharMap )
            return SAL_N_ELEMENTS( aVAsCharMap );
        if( pMap == aVParaMap )
            return SAL_N_ELEMENTS( aVParaMap );
        if( pMap == aHParaMap )
            return SAL_N_ELEMENTS( aHParaMap );
        if( pMap == aHFrameMap )
            return SAL_N_ELEMENTS( aHFrameMap );
        if( pMap == aVFrameMap )
            return SAL_N_ELEMENTS( aVFrameMap );
        if( pMap == aHCharMap )
            return SAL_N_ELEMENTS( aHCharMap );
        if( pMap == aHCharHtmlMap )
            return SAL_N_ELEMENTS( aHCharHtmlMap );
        if( pMap == aHCharHtmlAbsMap )
            return SAL_N_ELEMENTS( aHCharHtmlAbsMap );
        if( pMap == aVCharMap )
            return SAL_N_ELEMENTS( aVCharMap );
        if( pMap == aVCharHtmlMap )
            return SAL_N_ELEMENTS( aVCharHtmlMap );
        if( pMap == aVCharHtmlAbsMap )
            return SAL_N_ELEMENTS( aVCharHtmlAbsMap );
        if( pMap == aHPageHtmlMap )
            return SAL_N_ELEMENTS( aHPageHtmlMap );
        if( pMap == aHFlyHtmlMap )
            return SAL_N_ELEMENTS( aHFlyHtmlMap );
        if( pMap == aVFlyHtmlMap )
            return SAL_N_ELEMENTS( aVFlyHtmlMap );
        if( pMap == aVMultiSelectionMap )
            return SAL_N_ELEMENTS( aVMultiSelectionMap );
        if( pMap == aHMultiSelectionMap )
            return SAL_N_ELEMENTS( aHMultiSelectionMap );
        return SAL_N_ELEMENTS(aHPageMap);
    }
    return 0;
}

static SvxSwFramePosString::StringId lcl_ChangeResIdToVerticalOrRTL(
            SvxSwFramePosString::StringId eStringId, bool bVertical, bool bRTL)
{
    //special handling of STR_FROMLEFT
    if(SwFPos::FROMLEFT == eStringId)
    {
        eStringId = bVertical ?
            bRTL ? SwFPos::FROMBOTTOM : SwFPos::FROMTOP :
            bRTL ? SwFPos::FROMRIGHT : SwFPos::FROMLEFT;
        return eStringId;
    }
    if(bVertical)
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
            if(aVertIds[nIndex].eHori == eStringId)
            {
                eStringId = aVertIds[nIndex].eVert;
                break;
            }
        }
    }
    return eStringId;
}
// #i22341# - helper method in order to determine all possible
// listbox relations in a relation map for a given relation
static LB lcl_GetLBRelationsForRelations( const sal_uInt16 _nRel )
{
    LB nLBRelations = LB::NONE;

    for (RelationMap const & nRelMapPos : aRelationMap)
    {
        if ( nRelMapPos.nRelation == _nRel )
        {
            nLBRelations |= nRelMapPos.nLBRelation;
        }
    }

    return nLBRelations;
}

// #i22341# - helper method on order to determine all possible
// listbox relations in a relation map for a given string ID
static LB lcl_GetLBRelationsForStrID(const FrmMap* _pMap,
                                     const SvxSwFramePosString::StringId _eStrId,
                                     const bool _bUseMirrorStr )
{
    LB nLBRelations = LB::NONE;

    std::size_t nRelMapSize = lcl_GetFrmMapCount( _pMap );
    for ( std::size_t nRelMapPos = 0; nRelMapPos < nRelMapSize; ++nRelMapPos )
    {
        if ( ( !_bUseMirrorStr && _pMap[nRelMapPos].eStrId == _eStrId ) ||
             ( _bUseMirrorStr && _pMap[nRelMapPos].eMirrorStrId == _eStrId ) )
        {
            nLBRelations |= _pMap[nRelMapPos].nLBRelations;
        }
    }

    return nLBRelations;
}

SvxSwPosSizeTabPage::SvxSwPosSizeTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "cui/ui/swpossizepage.ui", "SwPosSizePage", &rInAttrs)
    , m_pVMap(nullptr)
    , m_pHMap(nullptr)
    , m_pSdrView(nullptr)
    , m_nOldH(HoriOrientation::CENTER)
    , m_nOldHRel(RelOrientation::FRAME)
    , m_nOldV(VertOrientation::TOP)
    , m_nOldVRel(RelOrientation::PRINT_AREA)
    , m_fWidthHeightRatio(1.0)
    , m_bHtmlMode(false)
    , m_bIsVerticalFrame(false)
    , m_bPositioningDisabled(false)
    , m_bIsMultiSelection(false)
    , m_bIsInRightToLeft(false)
    , m_nProtectSizeState(TRISTATE_FALSE)
    , m_xWidthMF(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
    , m_xHeightMF(m_xBuilder->weld_metric_spin_button("height", FieldUnit::CM))
    , m_xKeepRatioCB(m_xBuilder->weld_check_button("ratio"))
    , m_xToPageRB(m_xBuilder->weld_radio_button("topage"))
    , m_xToParaRB(m_xBuilder->weld_radio_button("topara"))
    , m_xToCharRB(m_xBuilder->weld_radio_button("tochar"))
    , m_xAsCharRB(m_xBuilder->weld_radio_button("aschar"))
    , m_xToFrameRB(m_xBuilder->weld_radio_button("toframe"))
    , m_xPositionCB(m_xBuilder->weld_check_button("pos"))
    , m_xSizeCB(m_xBuilder->weld_check_button("size"))
    , m_xPosFrame(m_xBuilder->weld_widget("posframe"))
    , m_xHoriFT(m_xBuilder->weld_label("horiposft"))
    , m_xHoriLB(m_xBuilder->weld_combo_box("horipos"))
    , m_xHoriByFT(m_xBuilder->weld_label("horibyft"))
    , m_xHoriByMF(m_xBuilder->weld_metric_spin_button("byhori", FieldUnit::CM))
    , m_xHoriToFT(m_xBuilder->weld_label("horitoft"))
    , m_xHoriToLB(m_xBuilder->weld_combo_box("horianchor"))
    , m_xHoriMirrorCB(m_xBuilder->weld_check_button("mirror"))
    , m_xVertFT(m_xBuilder->weld_label("vertposft"))
    , m_xVertLB(m_xBuilder->weld_combo_box("vertpos"))
    , m_xVertByFT(m_xBuilder->weld_label("vertbyft"))
    , m_xVertByMF(m_xBuilder->weld_metric_spin_button("byvert", FieldUnit::CM))
    , m_xVertToFT(m_xBuilder->weld_label("verttoft"))
    , m_xVertToLB(m_xBuilder->weld_combo_box("vertanchor"))
    , m_xFollowCB(m_xBuilder->weld_check_button("followtextflow"))
    , m_xExampleWN(new weld::CustomWeld(*m_xBuilder, "preview", m_aExampleWN))
{
    setOptimalFrmWidth();
    setOptimalRelWidth();

    FieldUnit eDlgUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit(*m_xHoriByMF, eDlgUnit, true);
    SetFieldUnit(*m_xVertByMF, eDlgUnit, true);
    SetFieldUnit(*m_xWidthMF , eDlgUnit, true);
    SetFieldUnit(*m_xHeightMF, eDlgUnit, true);

    SetExchangeSupport();

    Link<weld::Widget&,void> aLk3 = LINK(this, SvxSwPosSizeTabPage, RangeModifyHdl);
    m_xWidthMF->connect_focus_out(aLk3);
    m_xHeightMF->connect_focus_out(aLk3);
    m_xHoriByMF->connect_focus_out(aLk3);
    m_xVertByMF->connect_focus_out(aLk3);
    m_xFollowCB->connect_toggled(LINK(this, SvxSwPosSizeTabPage, RangeModifyClickHdl));

    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SvxSwPosSizeTabPage, ModifyHdl);
    m_xWidthMF->connect_value_changed( aLk );
    m_xHeightMF->connect_value_changed( aLk );
    m_xHoriByMF->connect_value_changed( aLk );
    m_xVertByMF->connect_value_changed( aLk );

    Link<weld::ToggleButton&,void> aLk2 = LINK(this, SvxSwPosSizeTabPage, AnchorTypeHdl);
    m_xToPageRB->connect_toggled( aLk2 );
    m_xToParaRB->connect_toggled( aLk2 );
    m_xToCharRB->connect_toggled( aLk2 );
    m_xAsCharRB->connect_toggled( aLk2 );
    m_xToFrameRB->connect_toggled( aLk2 );

    m_xHoriLB->connect_changed(LINK(this, SvxSwPosSizeTabPage, PosHdl));
    m_xVertLB->connect_changed(LINK(this, SvxSwPosSizeTabPage, PosHdl));

    m_xHoriToLB->connect_changed(LINK(this, SvxSwPosSizeTabPage, RelHdl));
    m_xVertToLB->connect_changed(LINK(this, SvxSwPosSizeTabPage, RelHdl));

    m_xHoriMirrorCB->connect_toggled(LINK(this, SvxSwPosSizeTabPage, MirrorHdl));
    m_xPositionCB->connect_toggled(LINK(this, SvxSwPosSizeTabPage, ProtectHdl));
}

SvxSwPosSizeTabPage::~SvxSwPosSizeTabPage()
{
    disposeOnce();
}

void SvxSwPosSizeTabPage::dispose()
{
    m_xWidthMF.reset();
    m_xHeightMF.reset();
    m_xHoriByMF.reset();
    m_xVertByMF.reset();
    SfxTabPage::dispose();
}

namespace
{
    struct FrmMaps
    {
        FrmMap const *pMap;
        size_t nCount;
    };
}

void SvxSwPosSizeTabPage::setOptimalFrmWidth()
{
    static const FrmMaps aMaps[] = {
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
    for (const FrmMaps& aMap : aMaps)
    {
        for (size_t j = 0; j < aMap.nCount; ++j)
        {
            aFrames.push_back(aMap.pMap[j].eStrId);
            aFrames.push_back(aMap.pMap[j].eMirrorStrId);
        }
    }

    std::sort(aFrames.begin(), aFrames.end());
    aFrames.erase(std::unique(aFrames.begin(), aFrames.end()), aFrames.end());

    for (auto const& frame : aFrames)
    {
        m_xHoriLB->append_text(SvxSwFramePosString::GetString(frame));
    }

    Size aBiggest(m_xHoriLB->get_preferred_size());
    m_xHoriLB->set_size_request(aBiggest.Width(), -1);
    m_xVertLB->set_size_request(aBiggest.Width(), -1);
    m_xHoriLB->clear();
}

namespace
{
    struct RelationMaps
    {
        RelationMap const *pMap;
        size_t nCount;
    };
}

void SvxSwPosSizeTabPage::setOptimalRelWidth()
{
    static const RelationMaps aMaps[] = {
        { aRelationMap, SAL_N_ELEMENTS(aRelationMap) },
        { aAsCharRelationMap, SAL_N_ELEMENTS(aAsCharRelationMap) }
    };

    std::vector<SvxSwFramePosString::StringId> aRels;
    for (const RelationMaps& aMap : aMaps)
    {
        for (size_t j = 0; j < aMap.nCount; ++j)
        {
            aRels.push_back(aMap.pMap[j].eStrId);
            aRels.push_back(aMap.pMap[j].eMirrorStrId);
        }
    }

    std::sort(aRels.begin(), aRels.end());
    aRels.erase(std::unique(aRels.begin(), aRels.end()), aRels.end());

    for (auto const& elem : aRels)
    {
        m_xHoriLB->append_text(SvxSwFramePosString::GetString(elem));
    }

    Size aBiggest(m_xHoriLB->get_preferred_size());
    m_xHoriLB->set_size_request(aBiggest.Width(), -1);
    m_xVertLB->set_size_request(aBiggest.Width(), -1);
    m_xHoriLB->clear();
}

VclPtr<SfxTabPage> SvxSwPosSizeTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxSwPosSizeTabPage>::Create(pParent, *rSet);
}

const sal_uInt16* SvxSwPosSizeTabPage::GetRanges()
{
    static const sal_uInt16 pSwPosRanges[] =
    {
        SID_ATTR_TRANSFORM_POS_X,
        SID_ATTR_TRANSFORM_POS_Y,
        SID_ATTR_TRANSFORM_PROTECT_POS,
        SID_ATTR_TRANSFORM_PROTECT_POS,
        SID_ATTR_TRANSFORM_INTERN,
        SID_ATTR_TRANSFORM_INTERN,
        SID_ATTR_TRANSFORM_ANCHOR,
        SID_ATTR_TRANSFORM_VERT_ORIENT,
        SID_ATTR_TRANSFORM_WIDTH,
        SID_ATTR_TRANSFORM_SIZE_POINT,
        SID_ATTR_TRANSFORM_PROTECT_POS,
        SID_ATTR_TRANSFORM_INTERN,
        SID_ATTR_TRANSFORM_AUTOWIDTH,
        SID_ATTR_TRANSFORM_VERT_ORIENT,
        SID_HTML_MODE,
        SID_HTML_MODE,
        SID_SW_FOLLOW_TEXT_FLOW,
        SID_SW_FOLLOW_TEXT_FLOW,
        SID_ATTR_TRANSFORM_HORI_POSITION,
        SID_ATTR_TRANSFORM_VERT_POSITION,
        0
    };
    return pSwPosRanges;
}

bool SvxSwPosSizeTabPage::FillItemSet( SfxItemSet* rSet)
{
    bool bAnchorChanged = false;
    RndStdIds nAnchor = GetAnchorType(&bAnchorChanged);
    bool bModified = false;
    if(bAnchorChanged)
    {
        rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, static_cast<sal_Int16>(nAnchor)));
        bModified = true;
    }
    if (m_xPositionCB->get_state_changed_from_saved())
    {
        if (m_xPositionCB->get_inconsistent())
            rSet->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
        else
            rSet->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                m_xPositionCB->get_state() == TRISTATE_TRUE ) );
        bModified = true;
    }

    if (m_xSizeCB->get_state_changed_from_saved())
    {
        if (m_xSizeCB->get_inconsistent())
            rSet->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rSet->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_xSizeCB->get_state() == TRISTATE_TRUE ) );
        bModified = true;
    }

    const SfxItemSet& rOldSet = GetItemSet();

    if(!m_bPositioningDisabled)
    {
        //on multiple selections the positioning is set via SdrView
        if (m_bIsMultiSelection)
        {
            if (m_xHoriByMF->get_value_changed_from_saved() || m_xVertByMF->get_value_changed_from_saved())
            {
                auto nHoriByPos = m_xHoriByMF->denormalize(m_xHoriByMF->get_value(FieldUnit::TWIP));
                auto nVertByPos = m_xVertByMF->denormalize(m_xVertByMF->get_value(FieldUnit::TWIP));

                // old rectangle with CoreUnit
                m_aRect = m_pSdrView->GetAllMarkedRect();
                m_pSdrView->GetSdrPageView()->LogicToPagePos( m_aRect );

                nHoriByPos += m_aAnchorPos.X();
                nVertByPos += m_aAnchorPos.Y();

                rSet->Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_X ), nHoriByPos ) );
                rSet->Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_Y ), nVertByPos ) );

                bModified = true;
            }
        }
        else
        {
            if ( m_pHMap )
            {
                const SfxInt16Item& rHoriOrient =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_HORI_ORIENT));
                const SfxInt16Item& rHoriRelation =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_HORI_RELATION));
                const SfxInt32Item& rHoriPosition =
                        static_cast<const SfxInt32Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_HORI_POSITION)) ;

                sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_xHoriLB);
                short nAlign = GetAlignment(m_pHMap, nMapPos, *m_xHoriToLB);
                short nRel = GetRelation(*m_xHoriToLB);
                const auto nHoriByPos = m_xHoriByMF->denormalize(m_xHoriByMF->get_value(FieldUnit::TWIP));
                if (
                    nAlign != rHoriOrient.GetValue() ||
                    nRel != rHoriRelation.GetValue() ||
                    (m_xHoriByMF->get_sensitive() && nHoriByPos != rHoriPosition.GetValue())
                   )
                {
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_ORIENT, nAlign));
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_RELATION, nRel));
                    if(m_xHoriByMF->get_sensitive())
                        rSet->Put(SfxInt32Item(SID_ATTR_TRANSFORM_HORI_POSITION, nHoriByPos));
                    bModified = true;
                }
            }
            if (m_xHoriMirrorCB->get_sensitive() && m_xHoriMirrorCB->get_state_changed_from_saved())
                bModified |= nullptr != rSet->Put(SfxBoolItem(SID_ATTR_TRANSFORM_HORI_MIRROR, m_xHoriMirrorCB->get_active()));

            if ( m_pVMap )
            {
                const SfxInt16Item& rVertOrient =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_ORIENT));
                const SfxInt16Item& rVertRelation =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_RELATION));
                const SfxInt32Item& rVertPosition =
                        static_cast<const SfxInt32Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_POSITION));

                sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_xVertLB);
                short nAlign = GetAlignment(m_pVMap, nMapPos, *m_xVertToLB);
                short nRel = GetRelation(*m_xVertToLB);
                // #i34055# - convert vertical position for
                // as-character anchored objects
                auto nVertByPos = m_xVertByMF->denormalize(m_xVertByMF->get_value(FieldUnit::TWIP));
                if (GetAnchorType() == RndStdIds::FLY_AS_CHAR)
                {
                    nVertByPos *= -1;
                }
                if ( nAlign != rVertOrient.GetValue() ||
                     nRel != rVertRelation.GetValue() ||
                     ( m_xVertByMF->get_sensitive() &&
                       nVertByPos != rVertPosition.GetValue() ) )
                {
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, nAlign));
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, nRel));
                    if(m_xVertByMF->get_sensitive())
                        rSet->Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, nVertByPos));
                    bModified = true;
                }
            }

            // #i18732#
            if (m_xFollowCB->get_state_changed_from_saved())
            {
                //Writer internal type - based on SfxBoolItem
                const SfxPoolItem* pItem = GetItem( rOldSet, SID_SW_FOLLOW_TEXT_FLOW);
                if(pItem)
                {
                    std::unique_ptr<SfxBoolItem> pFollow(static_cast<SfxBoolItem*>(pItem->Clone()));
                    pFollow->SetValue(m_xFollowCB->get_active());
                    bModified |= nullptr != rSet->Put(*pFollow);
                }
            }
        }
    }
    if (m_xWidthMF->get_value_changed_from_saved() || m_xHeightMF->get_value_changed_from_saved())
    {
        sal_uInt32 nWidth = static_cast<sal_uInt32>(m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::TWIP)));
        sal_uInt32 nHeight = static_cast<sal_uInt32>(m_xHeightMF->denormalize(m_xHeightMF->get_value(FieldUnit::TWIP)));
        rSet->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ), nWidth ) );
        rSet->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ), nHeight ) );
        //this item is required by SdrEditView::SetGeoAttrToMarked()
        rSet->Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), sal_uInt16(RectPoint::LT) ) );

        bModified = true;
    }

    return bModified;
}

void SvxSwPosSizeTabPage::Reset( const SfxItemSet* rSet)
{
    const SfxPoolItem* pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_ANCHOR );
    bool bInvalidateAnchor = false;
    RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA;
    if(pItem)
    {
        nAnchorType = static_cast<RndStdIds>(static_cast<const SfxInt16Item*>(pItem)->GetValue());
        switch(nAnchorType)
        {
            case RndStdIds::FLY_AT_PAGE:   m_xToPageRB->set_active(true);  break;
            case RndStdIds::FLY_AT_PARA:   m_xToParaRB->set_active(true);  break;
            case RndStdIds::FLY_AT_CHAR:   m_xToCharRB->set_active(true);  break;
            case RndStdIds::FLY_AS_CHAR:   m_xAsCharRB->set_active(true);  break;
            case RndStdIds::FLY_AT_FLY:    m_xToFrameRB->set_active(true); break;
            default : bInvalidateAnchor = true;
        }
        m_xToPageRB->save_state();
        m_xToParaRB->save_state();
        m_xToCharRB->save_state();
        m_xAsCharRB->save_state();
        m_xToFrameRB->save_state();
    }
    if (bInvalidateAnchor)
    {
        m_xToPageRB->set_sensitive( false );
        m_xToParaRB->set_sensitive( false );
        m_xToCharRB->set_sensitive( false );
        m_xAsCharRB->set_sensitive( false );
        m_xToFrameRB->set_sensitive( false );
    }

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_PROTECT_POS );
    if (pItem)
    {
        bool bProtected = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_xPositionCB->set_active(bProtected);
        m_xSizeCB->set_sensitive(!bProtected);
    }
    else
    {
        m_xPositionCB->set_inconsistent(true);
    }

    m_xPositionCB->save_state();

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_PROTECT_SIZE );

    if (pItem)
    {
        m_xSizeCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    }
    else
        m_xSizeCB->set_inconsistent(true);
    m_xSizeCB->save_state();

    pItem = GetItem( *rSet, SID_HTML_MODE );
    if(pItem)
    {
        m_bHtmlMode =
            (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
            != 0;
    }

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_IN_VERTICAL_TEXT );
    if(pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue())
    {
        OUString sHLabel = m_xHoriFT->get_label();
        m_xHoriFT->set_label(m_xVertFT->get_label());
        m_xVertFT->set_label(sHLabel);
        m_bIsVerticalFrame = true;
    }
    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_IN_RTL_TEXT);
    if(pItem)
        m_bIsInRightToLeft = static_cast<const SfxBoolItem*>(pItem)->GetValue();

    pItem = GetItem( *rSet, SID_SW_FOLLOW_TEXT_FLOW);
    if(pItem)
    {
        const bool bFollowTextFlow =
            static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_xFollowCB->set_active(bFollowTextFlow);
    }
    m_xFollowCB->save_state();

    if(m_bHtmlMode)
    {
        m_xHoriMirrorCB->hide();
        m_xKeepRatioCB->set_sensitive(false);
        // #i18732# - hide checkbox in HTML mode
        m_xFollowCB->hide();
    }
    else
    {
        // #i18732# correct enable/disable of check box 'Mirror on..'
        m_xHoriMirrorCB->set_sensitive(!m_xAsCharRB->get_active() && !m_bIsMultiSelection);

        // #i18732# - enable/disable check box 'Follow text flow'.
        m_xFollowCB->set_sensitive(m_xToParaRB->get_active() ||
                                   m_xToCharRB->get_active());
    }

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_WIDTH );
    sal_Int32 nWidth = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, sal_uInt32(1) );

    m_xWidthMF->set_value(m_xWidthMF->normalize(nWidth), FieldUnit::TWIP);

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HEIGHT );
    sal_Int32 nHeight = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, sal_uInt32(1) );
    m_xHeightMF->set_value(m_xHeightMF->normalize(nHeight), FieldUnit::TWIP);
    m_fWidthHeightRatio = double(nWidth) / double(nHeight);

    if(!m_bPositioningDisabled)
    {
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HORI_ORIENT);
        if(pItem)
        {
            short nHoriOrientation = static_cast< const SfxInt16Item*>(pItem)->GetValue();
            m_nOldH = nHoriOrientation;
        }
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_VERT_ORIENT);
        if(pItem)
        {
            short nVertOrientation = static_cast< const SfxInt16Item*>(pItem)->GetValue();
            m_nOldV = nVertOrientation;
        }
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HORI_RELATION);
        if(pItem)
        {
            m_nOldHRel = static_cast< const SfxInt16Item*>(pItem)->GetValue();
        }

        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_VERT_RELATION);
        if(pItem)
        {
            m_nOldVRel = static_cast< const SfxInt16Item*>(pItem)->GetValue();
        }
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HORI_MIRROR);
        if(pItem)
            m_xHoriMirrorCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());
        m_xHoriMirrorCB->save_state();

        sal_Int32 nHoriPos = 0;
        sal_Int32 nVertPos = 0;
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HORI_POSITION);
        if(pItem)
            nHoriPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_VERT_POSITION);
        if(pItem)
            nVertPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();

        InitPos(nAnchorType, m_nOldH, m_nOldHRel, m_nOldV, m_nOldVRel, nHoriPos, nVertPos);

        m_xVertByMF->save_value();
        m_xHoriByMF->save_value();
        // #i18732#
        m_xFollowCB->save_state();

        RangeModifyHdl(m_xWidthMF->get_widget());  // initially set maximum values
    }
}

DeactivateRC SvxSwPosSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
    {
        _pSet->Put(SfxBoolItem(GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                m_xPositionCB->get_active()));
        _pSet->Put(SfxBoolItem(GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_xSizeCB->get_active()));
        FillItemSet( _pSet );
    }
    return DeactivateRC::LeavePage;
}

void SvxSwPosSizeTabPage::EnableAnchorTypes(SvxAnchorIds nAnchorEnable)
{
    if (nAnchorEnable & SvxAnchorIds::Fly)
        m_xToFrameRB->show();
    if (!(nAnchorEnable & SvxAnchorIds::Page))
        m_xToPageRB->set_sensitive(false);
}

RndStdIds SvxSwPosSizeTabPage::GetAnchorType(bool* pbHasChanged)
{
    RndStdIds nRet = RndStdIds::UNKNOWN;
    weld::RadioButton* pCheckedButton = nullptr;
    if(m_xToParaRB->get_sensitive())
    {
        if(m_xToPageRB->get_active())
        {
            nRet = RndStdIds::FLY_AT_PAGE;
            pCheckedButton = m_xToPageRB.get();
        }
        else if(m_xToParaRB->get_active())
        {
            nRet = RndStdIds::FLY_AT_PARA;
            pCheckedButton = m_xToParaRB.get();
        }
        else if(m_xToCharRB->get_active())
        {
            nRet = RndStdIds::FLY_AT_CHAR;
            pCheckedButton = m_xToCharRB.get();
        }
        else if(m_xAsCharRB->get_active())
        {
            nRet = RndStdIds::FLY_AS_CHAR;
            pCheckedButton = m_xAsCharRB.get();
        }
        else if(m_xToFrameRB->get_active())
        {
            nRet = RndStdIds::FLY_AT_FLY;
            pCheckedButton = m_xToFrameRB.get();
        }
    }
    if(pbHasChanged)
    {
         if(pCheckedButton)
             *pbHasChanged = pCheckedButton->get_state_changed_from_saved();
         else
             *pbHasChanged = false;
    }
    return nRet;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, RangeModifyClickHdl, weld::ToggleButton&, void)
{
    RangeModifyHdl(m_xWidthMF->get_widget());
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, RangeModifyHdl, weld::Widget&, void)
{
    if (m_bPositioningDisabled)
        return;
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = GetAnchorType();
    aVal.bAutoHeight = false;
    aVal.bMirror = m_xHoriMirrorCB->get_active();
    // #i18732#
    aVal.bFollowTextFlow = m_xFollowCB->get_active();

    if ( m_pHMap )
    {
        // horizontal alignment
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_xHoriToLB);
        sal_uInt16 nAlign = GetAlignment(m_pHMap, nMapPos, *m_xHoriToLB);
        sal_uInt16 nRel = GetRelation(*m_xHoriToLB);

        aVal.nHoriOrient = static_cast<short>(nAlign);
        aVal.nHRelOrient = static_cast<short>(nRel);
    }
    else
        aVal.nHoriOrient = HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // vertical alignment
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_xVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, *m_xVertToLB);
        sal_uInt16 nRel = GetRelation(*m_xVertToLB);

        aVal.nVertOrient = static_cast<short>(nAlign);
        aVal.nVRelOrient = static_cast<short>(nRel);
    }
    else
        aVal.nVertOrient = VertOrientation::NONE;

    const auto nAtHorzPosVal = m_xHoriByMF->denormalize(m_xHoriByMF->get_value(FieldUnit::TWIP));
    const auto nAtVertPosVal = m_xVertByMF->denormalize(m_xVertByMF->get_value(FieldUnit::TWIP));

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    sal_Int32 nWidth = static_cast<sal_uInt32>(m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::TWIP)));
    sal_Int32 nHeight = static_cast<sal_uInt32>(m_xHeightMF->denormalize(m_xHeightMF->get_value(FieldUnit::TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    m_aValidateLink.Call(aVal);

    // minimum width also for style
    m_xHeightMF->set_min(m_xHeightMF->normalize(aVal.nMinHeight), FieldUnit::TWIP);
    m_xWidthMF->set_min(m_xWidthMF->normalize(aVal.nMinWidth), FieldUnit::TWIP);

    sal_Int32 nMaxWidth(aVal.nMaxWidth);
    sal_Int32 nMaxHeight(aVal.nMaxHeight);

    sal_Int64 nTmp = m_xHeightMF->normalize(nMaxHeight);
    m_xHeightMF->set_max(nTmp, FieldUnit::TWIP);

    nTmp = m_xWidthMF->normalize(nMaxWidth);
    m_xWidthMF->set_max(nTmp, FieldUnit::TWIP);

    m_xHoriByMF->set_range(m_xHoriByMF->normalize(aVal.nMinHPos),
                           m_xHoriByMF->normalize(aVal.nMaxHPos), FieldUnit::TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        m_xHoriByMF->set_value(m_xHoriByMF->normalize(aVal.nHPos), FieldUnit::TWIP);

    m_xVertByMF->set_range(m_xVertByMF->normalize(aVal.nMinVPos),
                           m_xVertByMF->normalize(aVal.nMaxVPos), FieldUnit::TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_xVertByMF->set_value(m_xVertByMF->normalize(aVal.nVPos), FieldUnit::TWIP);
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, AnchorTypeHdl, weld::ToggleButton&, void)
{
    m_xHoriMirrorCB->set_sensitive(!m_xAsCharRB->get_active() && !m_bIsMultiSelection);

    // #i18732# - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    m_xFollowCB->set_sensitive(m_xToParaRB->get_active() || m_xToCharRB->get_active());

    RndStdIds nId = GetAnchorType();

    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl(m_xWidthMF->get_widget());

    if(m_bHtmlMode)
    {
        PosHdl(*m_xHoriLB);
        PosHdl(*m_xVertLB);
    }
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, MirrorHdl, weld::ToggleButton&, void)
{
    RndStdIds nId = GetAnchorType();
    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
}

IMPL_LINK( SvxSwPosSizeTabPage, RelHdl, weld::ComboBox&, rLB, void )
{
    bool bHori = &rLB == m_xHoriToLB.get();

    UpdateExample();

    if (m_bHtmlMode && RndStdIds::FLY_AT_CHAR == GetAnchorType()) // again special treatment
    {
        if(bHori)
        {
            sal_uInt16 nRel = GetRelation(*m_xHoriToLB);
            if(RelOrientation::PRINT_AREA == nRel && 0 == m_xVertLB->get_active())
            {
                m_xVertLB->set_active(1);
            }
            else if(RelOrientation::CHAR == nRel && 1 == m_xVertLB->get_active())
            {
                m_xVertLB->set_active(0);
            }
        }
    }
    RangeModifyHdl(m_xWidthMF->get_widget());
}

IMPL_LINK(SvxSwPosSizeTabPage, PosHdl, weld::ComboBox&, rLB, void)
{
    bool bHori = &rLB == m_xHoriLB.get();
    weld::ComboBox* pRelLB = bHori ? m_xHoriToLB.get() : m_xVertToLB.get();
    weld::Label* pRelFT = bHori ? m_xHoriToFT.get() : m_xVertToFT.get();
    FrmMap const *pMap = bHori ? m_pHMap : m_pVMap;


    sal_uInt16 nMapPos = GetMapPos(pMap, rLB);
    sal_uInt16 nAlign = GetAlignment(pMap, nMapPos, *pRelLB);

    if (bHori)
    {
        bool bEnable = HoriOrientation::NONE == nAlign;
        m_xHoriByMF->set_sensitive( bEnable );
        m_xHoriByFT->set_sensitive( bEnable );
    }
    else
    {
        bool bEnable = VertOrientation::NONE == nAlign;
        m_xVertByMF->set_sensitive( bEnable );
        m_xVertByFT->set_sensitive( bEnable );
    }

    RangeModifyHdl(m_xWidthMF->get_widget());

    short nRel = 0;
    if (rLB.get_active() != -1)
    {
        if (pRelLB->get_active() != -1)
            nRel = reinterpret_cast<RelationMap*>(pRelLB->get_active_id().toUInt64())->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->clear();

    UpdateExample();

    // special treatment for HTML-Mode with horz-vert-dependencies
    if (m_bHtmlMode && RndStdIds::FLY_AT_CHAR == GetAnchorType())
    {
        bool bSet = false;
        if(bHori)
        {
            // on the right only below is allowed - from the left only at the top
            // from the left at the character -> below
            if((HoriOrientation::LEFT == nAlign || HoriOrientation::RIGHT == nAlign) &&
                    0 == m_xVertLB->get_active())
            {
                if(RelOrientation::FRAME == nRel)
                    m_xVertLB->set_active(1);
                else
                    m_xVertLB->set_active(0);
                bSet = true;
            }
            else if(HoriOrientation::LEFT == nAlign && 1 == m_xVertLB->get_active())
            {
                m_xVertLB->set_active(0);
                bSet = true;
            }
            else if(HoriOrientation::NONE == nAlign && 1 == m_xVertLB->get_active())
            {
                m_xVertLB->set_active(0);
                bSet = true;
            }
            if(bSet)
                PosHdl(*m_xVertLB);
        }
        else
        {
            if(VertOrientation::TOP == nAlign)
            {
                if(1 == m_xHoriLB->get_active())
                {
                    m_xHoriLB->set_active(0);
                    bSet = true;
                }
                m_xHoriToLB->set_active(1);
            }
            else if(VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == m_xHoriLB->get_active())
                {
                    m_xHoriLB->set_active(0);
                    bSet = true;
                }
                m_xHoriToLB->set_active(0) ;
            }
            if(bSet)
                PosHdl(*m_xHoriLB);
        }

    }
}

IMPL_LINK( SvxSwPosSizeTabPage, ModifyHdl, weld::MetricSpinButton&, rEdit, void )
{
    auto nWidth = m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::TWIP));
    auto nHeight = m_xHeightMF->denormalize(m_xHeightMF->get_value(FieldUnit::TWIP));
    if (m_xKeepRatioCB->get_active())
    {
        if ( &rEdit == m_xWidthMF.get() )
        {
            nHeight = int(static_cast<double>(nWidth) / m_fWidthHeightRatio);
            m_xHeightMF->set_value(m_xHeightMF->normalize(nHeight), FieldUnit::TWIP);
        }
        else if(&rEdit == m_xHeightMF.get())
        {
            nWidth = int(static_cast<double>(nHeight) * m_fWidthHeightRatio);
            m_xWidthMF->set_value(m_xWidthMF->normalize(nWidth), FieldUnit::TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, ProtectHdl, weld::ToggleButton&, void)
{
    if (m_xSizeCB->get_sensitive())
    {
        m_nProtectSizeState = m_xSizeCB->get_state();
    }

    m_xSizeCB->set_state(m_xPositionCB->get_state() == TRISTATE_TRUE ?  TRISTATE_TRUE : m_nProtectSizeState);
    m_xSizeCB->set_sensitive(m_xPositionCB->get_sensitive() && !m_xPositionCB->get_active());
}

short SvxSwPosSizeTabPage::GetRelation(const weld::ComboBox& rRelationLB)
{
    short nRel = 0;
    int nPos = rRelationLB.get_active();
    if (nPos != -1)
    {
        RelationMap *pEntry = reinterpret_cast<RelationMap*>(rRelationLB.get_id(nPos).toUInt64());
        nRel = pEntry->nRelation;
    }

    return nRel;
}

short SvxSwPosSizeTabPage::GetAlignment(FrmMap const *pMap, sal_uInt16 nMapPos, const weld::ComboBox& rRelationLB)
{
    short nAlign = 0;

    // #i22341# - special handling also for map <aVCharMap>,
    // because it contains ambiguous items for alignment
    if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap ||
            pMap == aVCharMap )
    {
        if (rRelationLB.get_active() != -1)
        {
            LB  nRel = reinterpret_cast<RelationMap*>(rRelationLB.get_active_id().toUInt64())->nLBRelation;
            std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (std::size_t i = 0; i < nMapCount; i++)
            {
                if (pMap[i].eStrId == eStrId)
                {
                    LB nLBRelations = pMap[i].nLBRelations;
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

sal_uInt16 SvxSwPosSizeTabPage::GetMapPos(FrmMap const *pMap, const weld::ComboBox& rAlignLB)
{
    sal_uInt16 nMapPos = 0;
    int nLBSelPos = rAlignLB.get_active();

    if (nLBSelPos != -1)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            OUString sSelEntry(rAlignLB.get_active_text());

            for (std::size_t i = 0; i < nMapCount; i++)
            {
                SvxSwFramePosString::StringId eResId = pMap[i].eStrId;

                OUString sEntry = SvxSwFramePosString::GetString(eResId);

                if (sEntry == sSelEntry)
                {
                    nMapPos = sal::static_int_cast< sal_uInt16 >(i);
                    break;
                }
            }
        }
        else
            nMapPos = nLBSelPos;
    }

    return nMapPos;
}

void SvxSwPosSizeTabPage::InitPos(RndStdIds nAnchor,
                                sal_uInt16 nH,
                                sal_uInt16 nHRel,
                                sal_uInt16 nV,
                                sal_uInt16 nVRel,
                                long   nX,
                                long   nY)
{
    int nPos = m_xVertLB->get_active();
    if (nPos != -1 && m_pVMap)
    {
        m_nOldV    = m_pVMap[nPos].nAlign;
        nPos = m_xVertToLB->get_active();
        if (nPos != -1)
            m_nOldVRel = reinterpret_cast<RelationMap*>(m_xVertToLB->get_id(nPos).toUInt64())->nRelation;
    }

    nPos = m_xHoriLB->get_active();
    if (nPos != -1 && m_pHMap)
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_xHoriToLB->get_active();
        if (nPos != -1)
            m_nOldHRel = reinterpret_cast<RelationMap*>(m_xHoriToLB->get_id(nPos).toUInt64())->nRelation;
    }

    bool bEnable = true;
    if( m_bIsMultiSelection )
    {
        m_pVMap = aVMultiSelectionMap;
        m_pHMap = aHMultiSelectionMap;
    }
    else if (nAnchor == RndStdIds::FLY_AT_PAGE)
    {
        m_pVMap = m_bHtmlMode ? aVPageHtmlMap : aVPageMap;
        m_pHMap = m_bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if (nAnchor == RndStdIds::FLY_AT_FLY)
    {
        // #i18732# - own vertical alignment map for to frame
        // anchored objects.
        m_pVMap = m_bHtmlMode ? aVFlyHtmlMap : aVFrameMap;
        m_pHMap = m_bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if (nAnchor == RndStdIds::FLY_AT_PARA)
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
    else if (nAnchor == RndStdIds::FLY_AT_CHAR)
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
    else if (nAnchor == RndStdIds::FLY_AS_CHAR)
    {
        m_pVMap = m_bHtmlMode ? aVAsCharHtmlMap     : aVAsCharMap;
        m_pHMap = nullptr;
        bEnable = false;
    }
    m_xHoriLB->set_sensitive(bEnable);
    m_xHoriFT->set_sensitive(bEnable);

    // select current Pos
    // horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = m_nOldH;
        nHRel = m_nOldHRel;
    }
    // #i22341# - pass <nHRel> as 3rd parameter to method <FillPosLB>
    sal_uInt16 nMapPos = FillPosLB(m_pHMap, nH, nHRel, *m_xHoriLB);
    FillRelLB(m_pHMap, nMapPos, nH, nHRel, *m_xHoriToLB, *m_xHoriToFT);

    // vertical
    if ( nV == USHRT_MAX )
    {
        nV    = m_nOldV;
        nVRel = m_nOldVRel;
    }
    // #i22341# - pass <nVRel> as 3rd parameter to method <FillPosLB>
    nMapPos = FillPosLB(m_pVMap, nV, nVRel, *m_xVertLB);
    FillRelLB(m_pVMap, nMapPos, nV, nVRel, *m_xVertToLB, *m_xVertToFT);

    // Edits init
    bEnable = nH == HoriOrientation::NONE && nAnchor != RndStdIds::FLY_AS_CHAR; //#61359# why not in formats&& !bFormat;
    if (!bEnable)
    {
        m_xHoriByMF->set_value(0, FieldUnit::TWIP);
    }
    else if(m_bIsMultiSelection)
    {
         m_xHoriByMF->set_value(m_xHoriByMF->normalize(m_aRect.Left()), FieldUnit::TWIP);
    }
    else
    {
        if (nX != LONG_MAX)
            m_xHoriByMF->set_value(m_xHoriByMF->normalize(nX), FieldUnit::TWIP);
    }
    m_xHoriByFT->set_sensitive(bEnable);
    m_xHoriByMF->set_sensitive(bEnable);

    bEnable = nV == VertOrientation::NONE;
    if ( !bEnable )
    {
        m_xVertByMF->set_value( 0, FieldUnit::TWIP );
    }
    else if(m_bIsMultiSelection)
    {
         m_xVertByMF->set_value(m_xVertByMF->normalize(m_aRect.Top()), FieldUnit::TWIP);
    }
    else
    {
        if (nAnchor == RndStdIds::FLY_AS_CHAR)
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            m_xVertByMF->set_value( m_xVertByMF->normalize(nY), FieldUnit::TWIP );
    }
    m_xVertByFT->set_sensitive( bEnable );
    m_xVertByMF->set_sensitive( bEnable );
    UpdateExample();
}

void SvxSwPosSizeTabPage::UpdateExample()
{
    int nPos = m_xHoriLB->get_active();
    if (m_pHMap && nPos != -1)
    {
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_xHoriLB);
        short nAlign = GetAlignment(m_pHMap, nMapPos, *m_xHoriToLB);
        short nRel = GetRelation(*m_xHoriToLB);

        m_aExampleWN.SetHAlign(nAlign);
        m_aExampleWN.SetHoriRel(nRel);
    }

    nPos = m_xVertLB->get_active();
    if (m_pVMap && nPos != -1)
    {
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_xVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, *m_xVertToLB);
        sal_uInt16 nRel = GetRelation(*m_xVertToLB);

        m_aExampleWN.SetVAlign(nAlign);
        m_aExampleWN.SetVertRel(nRel);
    }

    // Size
    auto nXPos = m_xHoriByMF->denormalize(m_xHoriByMF->get_value(FieldUnit::TWIP));
    auto nYPos = m_xVertByMF->denormalize(m_xVertByMF->get_value(FieldUnit::TWIP));
    m_aExampleWN.SetRelPos(Point(nXPos, nYPos));

    m_aExampleWN.SetAnchor( GetAnchorType() );
    m_aExampleWN.Invalidate();
}

void SvxSwPosSizeTabPage::FillRelLB(FrmMap const *pMap, sal_uInt16 nMapPos, sal_uInt16 nAlign,
                                    sal_uInt16 nRel, weld::ComboBox& rLB, weld::Label& rFT)
{
    OUString sSelEntry;
    LB  nLBRelations = LB::NONE;
    std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);

    rLB.clear();

    if (nMapPos < nMapCount)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            OUString sOldEntry(rLB.get_active_text());
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (std::size_t _nMapPos = 0; _nMapPos < nMapCount; _nMapPos++)
            {
                if (pMap[_nMapPos].eStrId == eStrId)
                {
                    nLBRelations = pMap[_nMapPos].nLBRelations;
                    for (size_t nRelPos = 0; nRelPos < SAL_N_ELEMENTS(aAsCharRelationMap); nRelPos++)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = aAsCharRelationMap[nRelPos].eStrId;

                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            OUString sEntry = SvxSwFramePosString::GetString(sStrId1);
                            rLB.append(OUString::number(reinterpret_cast<sal_uInt64>(&aAsCharRelationMap[nRelPos])), sEntry);
                            if (pMap[_nMapPos].nAlign == nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                rLB.set_active_text(sSelEntry);
            else
            {
                rLB.set_active_text(sOldEntry);
                if (rLB.get_active() == -1)
                {
                    for (int i = 0; i < rLB.get_count(); i++)
                    {
                        RelationMap *pEntry = reinterpret_cast<RelationMap*>(rLB.get_id(i).toUInt64());
                        if (pEntry->nLBRelation == LB::RelChar) // Default
                        {
                            rLB.set_active(i);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            // #i22341# - special handling for map <aVCharMap>,
            // because its ambiguous in its <eStrId>/<eMirrorStrId>.
            if ( pMap == aVCharMap )
            {
                nLBRelations = ::lcl_GetLBRelationsForStrID( pMap,
                                             ( m_xHoriMirrorCB->get_active()
                                               ? pMap[nMapPos].eMirrorStrId
                                               : pMap[nMapPos].eStrId ),
                                             m_xHoriMirrorCB->get_active() );
            }
            else
            {
                nLBRelations = pMap[nMapPos].nLBRelations;
            }

            for (sal_uLong nBit = 1; nBit < sal_uLong(LB::LAST); nBit <<= 1)
            {
                if (nLBRelations & static_cast<LB>(nBit))
                {
                    for (size_t nRelPos = 0; nRelPos < SAL_N_ELEMENTS(aRelationMap); nRelPos++)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == static_cast<LB>(nBit))
                        {
                            SvxSwFramePosString::StringId sStrId1 = m_xHoriMirrorCB->get_active() ? aRelationMap[nRelPos].eMirrorStrId : aRelationMap[nRelPos].eStrId;
                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            OUString sEntry = SvxSwFramePosString::GetString(sStrId1);
                            rLB.append(OUString::number(reinterpret_cast<sal_uInt64>(&aRelationMap[nRelPos])), sEntry);
                            if (sSelEntry.isEmpty() && aRelationMap[nRelPos].nRelation == nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                rLB.set_active_text(sSelEntry);
            else
            {
                // Probably anchor change. So look for a similar relation.
                switch (nRel)
                {
                    case RelOrientation::FRAME:           nRel = RelOrientation::PAGE_FRAME;    break;
                    case RelOrientation::PRINT_AREA:      nRel = RelOrientation::PAGE_PRINT_AREA;  break;
                    case RelOrientation::PAGE_LEFT:       nRel = RelOrientation::FRAME_LEFT;    break;
                    case RelOrientation::PAGE_RIGHT:      nRel = RelOrientation::FRAME_RIGHT;   break;
                    case RelOrientation::FRAME_LEFT:      nRel = RelOrientation::PAGE_LEFT;     break;
                    case RelOrientation::FRAME_RIGHT:     nRel = RelOrientation::PAGE_RIGHT;    break;
                    case RelOrientation::PAGE_FRAME:      nRel = RelOrientation::FRAME;           break;
                    case RelOrientation::PAGE_PRINT_AREA: nRel = RelOrientation::PRINT_AREA;         break;

                    default:
                        if (rLB.get_count())
                        {
                            RelationMap *pEntry = reinterpret_cast<RelationMap*>(rLB.get_id(rLB.get_count() - 1).toUInt64());
                            nRel = pEntry->nRelation;
                        }
                        break;
                }

                for (int i = 0; i < rLB.get_count(); ++i)
                {
                    RelationMap *pEntry = reinterpret_cast<RelationMap*>(rLB.get_id(i).toUInt64());
                    if (pEntry->nRelation == nRel)
                    {
                        rLB.set_active(i);
                        break;
                    }
                }

                if (rLB.get_active() == -1)
                    rLB.set_active(0);
            }
        }
    }

    rLB.set_sensitive(rLB.get_count() != 0);
    rFT.set_sensitive(rLB.get_count() != 0);

    RelHdl(rLB);
}

sal_uInt16 SvxSwPosSizeTabPage::FillPosLB(FrmMap const *_pMap,
                                      sal_uInt16 _nAlign,
                                      const sal_uInt16 _nRel,
                                      weld::ComboBox& _rLB)
{
    OUString sSelEntry, sOldEntry;
    sOldEntry = _rLB.get_active_text();

    _rLB.clear();

    // #i22341# - determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const LB nLBRelations = (_pMap != aVCharMap)
                               ? LB::NONE
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill listbox
    std::size_t nCount = ::lcl_GetFrmMapCount(_pMap);
    for (std::size_t i = 0; _pMap && i < nCount; ++i)
    {
        SvxSwFramePosString::StringId eStrId = m_xHoriMirrorCB->get_active() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
        eStrId = lcl_ChangeResIdToVerticalOrRTL(eStrId, m_bIsVerticalFrame, m_bIsInRightToLeft);
        OUString sEntry(SvxSwFramePosString::GetString(eStrId));
        if (_rLB.find_text(sEntry) == -1)
        {
            // don't insert duplicate entries at character wrapped borders
            _rLB.append_text(sEntry);
        }
        // #i22341# - add condition to handle map <aVCharMap>
        // that is ambiguous in the alignment.
        if ( _pMap[i].nAlign == _nAlign &&
             ( _pMap != aVCharMap || _pMap[i].nLBRelations & nLBRelations ) )
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

void SvxSwPosSizeTabPage::SetView( const SdrView* pSdrView )
{
    m_pSdrView = pSdrView;
    if(!m_pSdrView)
    {
        OSL_FAIL("No SdrView* set");
        return;
    }

    // setting of the rectangle and the working area
    m_aRect = m_pSdrView->GetAllMarkedRect();
    m_pSdrView->GetSdrPageView()->LogicToPagePos( m_aRect );

    // get WorkArea
    m_aWorkArea = m_pSdrView->GetWorkArea();

    // consider anchor position (for Writer)
    const SdrMarkList& rMarkList = m_pSdrView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() > 0 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        m_aAnchorPos = pObj->GetAnchorPos();

        if( m_aAnchorPos != Point(0,0) ) // -> Writer
        {
            for( size_t i = 1; i < rMarkList.GetMarkCount(); ++i )
            {
                pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                if( m_aAnchorPos != pObj->GetAnchorPos() )
                {
                    // different anchor positions -> disable positioning
                    m_xPosFrame->set_sensitive(false);
                    m_bPositioningDisabled = true;
                    return;
                }
            }
        }
        Point aPt = m_aAnchorPos * -1;
        Point aPt2 = aPt;

        aPt += m_aWorkArea.TopLeft();
        m_aWorkArea.SetPos( aPt );

        aPt2 += m_aRect.TopLeft();
        m_aRect.SetPos( aPt2 );
    }

    // this should happen via SID_ATTR_TRANSFORM_AUTOSIZE
    if( rMarkList.GetMarkCount() != 1 )
        m_bIsMultiSelection = true;
#if OSL_DEBUG_LEVEL > 1
    else
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( ( pObj->GetObjInventor() == SdrInventor::Default ) &&
            ( eKind==OBJ_TEXT || eKind==OBJ_TITLETEXT || eKind==OBJ_OUTLINETEXT) &&
            pObj->HasText() )
        {
            OSL_FAIL("AutoWidth/AutoHeight should be enabled");
        }
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
