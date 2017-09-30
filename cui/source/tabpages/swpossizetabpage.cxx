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
#include <dialmgr.hxx>
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

static RelationMap aRelationMap[] =
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

static RelationMap aAsCharRelationMap[] =
{
    {SwFPos::REL_BASE,  SwFPos::REL_BASE,   LB::RelBase,   RelOrientation::FRAME},
    {SwFPos::REL_CHAR,  SwFPos::REL_CHAR,   LB::RelChar,   RelOrientation::FRAME},
    {SwFPos::REL_ROW,   SwFPos::REL_ROW,   LB::RelRow,     RelOrientation::FRAME}
};

/*--------------------------------------------------------------------
    Anchored at page
 --------------------------------------------------------------------*/

#define HORI_PAGE_REL   (LB::RelPageFrame|LB::RelPagePrintArea|LB::RelPageLeft| \
                        LB::RelPageRight)

static FrmMap aHPageMap[] =
{
    {SwFPos::LEFT,       SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,      SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,   SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrmMap aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB::RelPageFrame}
};

#define VERT_PAGE_REL   (LB::RelPageFrame|LB::RelPagePrintArea)

static FrmMap aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrmMap aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP, SwFPos::FROMTOP,        VertOrientation::NONE,      LB::RelPageFrame}
};

/*--------------------------------------------------------------------
    Anchored at frame
 --------------------------------------------------------------------*/

#define HORI_FRAME_REL  (LB::FlyRelPageFrame|LB::FlyRelPagePrintArea| \
                        LB::FlyRelPageLeft|LB::FlyRelPageRight)

static FrmMap aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrmMap aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      LB::FlyRelPageFrame},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB::FlyRelPageFrame}
};

// #i18732# - own vertical alignment map for to frame anchored objects
// #i22305#
#define VERT_FRAME_REL   (LB::VertFrame|LB::FlyVertPrintArea)

static FrmMap aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrmMap aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB::FlyVertFrame},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      LB::FlyVertFrame}
};

static FrmMap aVMultiSelectionMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      LB::NONE}
};
static FrmMap aHMultiSelectionMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::FROMLEFT,       HoriOrientation::NONE,      LB::NONE}
};

/*--------------------------------------------------------------------
    Anchored at paragraph
 --------------------------------------------------------------------*/

#define HORI_PARA_REL   (LB::Frame|LB::PrintArea|LB::RelPageLeft|LB::RelPageRight| \
                        LB::RelPageFrame|LB::RelPagePrintArea|LB::RelFrameLeft| \
                        LB::RelFrameRight)

static FrmMap aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB::Frame|LB::PrintArea)

static FrmMap aHParaHtmlMap[] =
{
    {SwFPos::LEFT,  SwFPos::LEFT,   HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT, SwFPos::RIGHT,  HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};

static FrmMap aHParaHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HTML_HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HTML_HORI_PARA_REL}
};


#define VERT_PARA_REL   (LB::VertFrame|LB::VertPrintArea| \
                         LB::RelPageFrame|LB::RelPagePrintArea)

static FrmMap aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PARA_REL}
};

static FrmMap aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB::VertPrintArea}
};

/*--------------------------------------------------------------------
    Anchored at character
 --------------------------------------------------------------------*/

#define HORI_CHAR_REL   (LB::Frame|LB::PrintArea|LB::RelPageLeft|LB::RelPageRight| \
                        LB::RelPageFrame|LB::RelPagePrintArea|LB::RelFrameLeft| \
                        LB::RelFrameRight|LB::RelChar)

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
#define VERT_CHAR_REL   (LB::VertFrame|LB::VertPrintArea| \
                         LB::RelPageFrame|LB::RelPagePrintArea)

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


static FrmMap aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};

static FrmMap aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,    SwFPos::TOP,            VertOrientation::TOP,           LB::RelChar},
    {SwFPos::BELOW,  SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB::RelChar}
};
/*--------------------------------------------------------------------
    anchored as character
 --------------------------------------------------------------------*/

static FrmMap aVAsCharMap[] =
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

static FrmMap aVAsCharHtmlMap[] =
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
        sal_uInt16 nIndex;
        for(nIndex = 0; nIndex < SAL_N_ELEMENTS(aHoriIds); ++nIndex)
        {
            if(aHoriIds[nIndex].eHori == eStringId)
            {
                eStringId = aHoriIds[nIndex].eVert;
                return eStringId;
            }
        }
        for(nIndex = 0; nIndex < SAL_N_ELEMENTS(aVertIds); ++nIndex)
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

    for (RelationMap & nRelMapPos : aRelationMap)
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
static LB lcl_GetLBRelationsForStrID( const FrmMap* _pMap,
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

SvxSwPosSizeTabPage::SvxSwPosSizeTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "SwPosSizePage", "cui/ui/swpossizepage.ui", &rInAttrs)
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
{
    get(m_pWidthMF, "width");
    get(m_pHeightMF, "height");
    get(m_pKeepRatioCB, "ratio");
    get(m_pToPageRB, "topage");
    get(m_pToParaRB, "topara");
    get(m_pToCharRB, "tochar");
    get(m_pAsCharRB, "aschar");
    get(m_pToFrameRB, "toframe");
    get(m_pPositionCB, "pos");
    get(m_pSizeCB, "size");
    get(m_pPosFrame, "posframe");
    get(m_pHoriFT, "horiposft");
    get(m_pHoriLB, "horipos");
    get(m_pHoriByFT, "horibyft");
    get(m_pHoriByMF, "byhori");
    get(m_pHoriToFT, "horitoft");
    get(m_pHoriToLB, "horianchor");
    get(m_pHoriMirrorCB, "mirror");
    get(m_pVertFT, "vertposft");
    get(m_pVertLB, "vertpos");
    get(m_pVertByFT, "vertbyft");
    get(m_pVertByMF, "byvert");
    get(m_pVertToFT, "verttoft");
    get(m_pVertToLB, "vertanchor");
    get(m_pFollowCB, "followtextflow");
    get(m_pExampleWN, "preview");

    setOptimalFrmWidth();
    setOptimalRelWidth();

    FieldUnit eDlgUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_pHoriByMF, eDlgUnit, true );
    SetFieldUnit( *m_pVertByMF, eDlgUnit, true );
    SetFieldUnit( *m_pWidthMF , eDlgUnit, true );
    SetFieldUnit( *m_pHeightMF, eDlgUnit, true );

    SetExchangeSupport();

    Link<Control&,void> aLk3 = LINK(this, SvxSwPosSizeTabPage, RangeModifyHdl);
    m_pWidthMF->SetLoseFocusHdl( aLk3 );
    m_pHeightMF->SetLoseFocusHdl( aLk3 );
    m_pHoriByMF->SetLoseFocusHdl( aLk3 );
    m_pVertByMF->SetLoseFocusHdl( aLk3 );
    m_pFollowCB->SetClickHdl( LINK(this, SvxSwPosSizeTabPage, RangeModifyClickHdl) );

    Link<Edit&,void> aLk = LINK(this, SvxSwPosSizeTabPage, ModifyHdl);
    m_pWidthMF->SetModifyHdl( aLk );
    m_pHeightMF->SetModifyHdl( aLk );
    m_pHoriByMF->SetModifyHdl( aLk );
    m_pVertByMF->SetModifyHdl( aLk );

    Link<Button*,void> aLk2 = LINK(this, SvxSwPosSizeTabPage, AnchorTypeHdl);
    m_pToPageRB->SetClickHdl( aLk2 );
    m_pToParaRB->SetClickHdl( aLk2 );
    m_pToCharRB->SetClickHdl( aLk2 );
    m_pAsCharRB->SetClickHdl( aLk2 );
    m_pToFrameRB->SetClickHdl( aLk2 );

    m_pHoriLB->SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, PosHdl));
    m_pVertLB->SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, PosHdl));

    m_pHoriToLB->SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, RelHdl));
    m_pVertToLB->SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, RelHdl));

    m_pHoriMirrorCB->SetClickHdl(LINK(this, SvxSwPosSizeTabPage, MirrorHdl));
    m_pPositionCB->SetClickHdl(LINK(this, SvxSwPosSizeTabPage, ProtectHdl));
}

SvxSwPosSizeTabPage::~SvxSwPosSizeTabPage()
{
    disposeOnce();
}

void SvxSwPosSizeTabPage::dispose()
{
    m_pWidthMF.clear();
    m_pHeightMF.clear();
    m_pKeepRatioCB.clear();
    m_pToPageRB.clear();
    m_pToParaRB.clear();
    m_pToCharRB.clear();
    m_pAsCharRB.clear();
    m_pToFrameRB.clear();
    m_pPositionCB.clear();
    m_pSizeCB.clear();
    m_pPosFrame.clear();
    m_pHoriFT.clear();
    m_pHoriLB.clear();
    m_pHoriByFT.clear();
    m_pHoriByMF.clear();
    m_pHoriToFT.clear();
    m_pHoriToLB.clear();
    m_pHoriMirrorCB.clear();
    m_pVertFT.clear();
    m_pVertLB.clear();
    m_pVertByFT.clear();
    m_pVertByMF.clear();
    m_pVertToFT.clear();
    m_pVertToLB.clear();
    m_pFollowCB.clear();
    m_pExampleWN.clear();
    SfxTabPage::dispose();
}


namespace
{
    struct FrmMaps
    {
        FrmMap *pMap;
        size_t nCount;
    };
}

void SvxSwPosSizeTabPage::setOptimalFrmWidth()
{
    const FrmMaps aMaps[] = {
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

    for (std::vector<SvxSwFramePosString::StringId>::const_iterator aI = aFrames.begin(), aEnd = aFrames.end();
        aI != aEnd; ++aI)
    {
        m_pHoriLB->InsertEntry(SvxSwFramePosString::GetString(*aI));
    }

    Size aBiggest(m_pHoriLB->GetOptimalSize());
    m_pHoriLB->set_width_request(aBiggest.Width());
    m_pVertLB->set_width_request(aBiggest.Width());
    m_pHoriLB->Clear();
}

namespace
{
    struct RelationMaps
    {
        RelationMap *pMap;
        size_t nCount;
    };
}

void SvxSwPosSizeTabPage::setOptimalRelWidth()
{
    const RelationMaps aMaps[] = {
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

    for (std::vector<SvxSwFramePosString::StringId>::const_iterator aI = aRels.begin(), aEnd = aRels.end();
        aI != aEnd; ++aI)
    {
        m_pHoriLB->InsertEntry(SvxSwFramePosString::GetString(*aI));
    }

    Size aBiggest(m_pHoriLB->GetOptimalSize());
    m_pHoriLB->set_width_request(aBiggest.Width());
    m_pVertLB->set_width_request(aBiggest.Width());
    m_pHoriLB->Clear();
}

VclPtr<SfxTabPage> SvxSwPosSizeTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet)
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
        rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, (sal_Int16)nAnchor));
        bModified = true;
    }
    if ( m_pPositionCB->IsValueChangedFromSaved() )
    {
        if( m_pPositionCB->GetState() == TRISTATE_INDET )
            rSet->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
        else
            rSet->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                m_pPositionCB->GetState() == TRISTATE_TRUE ) );
        bModified = true;
    }

    if ( m_pSizeCB->IsValueChangedFromSaved() )
    {
        if ( m_pSizeCB->GetState() == TRISTATE_INDET )
            rSet->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rSet->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_pSizeCB->GetState() == TRISTATE_TRUE ) );
        bModified = true;
    }

    const SfxItemSet& rOldSet = GetItemSet();

    if(!m_bPositioningDisabled)
    {
        //on multiple selections the positioning is set via SdrView
        if(m_bIsMultiSelection)
        {
            if( m_pHoriByMF->IsValueModified() || m_pVertByMF->IsValueModified() )
            {
                long nHoriByPos =
                            static_cast<long>(m_pHoriByMF->Denormalize(m_pHoriByMF->GetValue(FUNIT_TWIP)));
                long nVertByPos =
                            static_cast<long>(m_pVertByMF->Denormalize(m_pVertByMF->GetValue(FUNIT_TWIP)));

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

                sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_pHoriLB);
                short nAlign = GetAlignment(m_pHMap, nMapPos, *m_pHoriToLB);
                short nRel = GetRelation(*m_pHoriToLB);
                const long nHoriByPos =
                            static_cast<long>(m_pHoriByMF->Denormalize(m_pHoriByMF->GetValue(FUNIT_TWIP)));
                if (
                    nAlign != rHoriOrient.GetValue() ||
                    nRel != rHoriRelation.GetValue() ||
                    (m_pHoriByMF->IsEnabled() && nHoriByPos != rHoriPosition.GetValue())
                   )
                {
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_ORIENT, nAlign));
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_RELATION, nRel));
                    if(m_pHoriByMF->IsEnabled())
                        rSet->Put(SfxInt32Item(SID_ATTR_TRANSFORM_HORI_POSITION, nHoriByPos));
                    bModified = true;
                }
            }
            if(m_pHoriMirrorCB->IsEnabled() && m_pHoriMirrorCB->IsValueChangedFromSaved())
                bModified |= nullptr != rSet->Put(SfxBoolItem(SID_ATTR_TRANSFORM_HORI_MIRROR, m_pHoriMirrorCB->IsChecked()));

            if ( m_pVMap )
            {
                const SfxInt16Item& rVertOrient =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_ORIENT));
                const SfxInt16Item& rVertRelation =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_RELATION));
                const SfxInt32Item& rVertPosition =
                        static_cast<const SfxInt32Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_POSITION));

                sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_pVertLB);
                short nAlign = GetAlignment(m_pVMap, nMapPos, *m_pVertToLB);
                short nRel = GetRelation(*m_pVertToLB);
                // #i34055# - convert vertical position for
                // as-character anchored objects
                long nVertByPos =
                        static_cast<long>(m_pVertByMF->Denormalize(m_pVertByMF->GetValue(FUNIT_TWIP)));
                if (GetAnchorType() == RndStdIds::FLY_AS_CHAR)
                {
                    nVertByPos *= -1;
                }
                if ( nAlign != rVertOrient.GetValue() ||
                     nRel != rVertRelation.GetValue() ||
                     ( m_pVertByMF->IsEnabled() &&
                       nVertByPos != rVertPosition.GetValue() ) )
                {
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, nAlign));
                    rSet->Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, nRel));
                    if(m_pVertByMF->IsEnabled())
                        rSet->Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, nVertByPos));
                    bModified = true;
                }
            }

            // #i18732#
            if(m_pFollowCB->IsValueChangedFromSaved())
            {
                //Writer internal type - based on SfxBoolItem
                const SfxPoolItem* pItem = GetItem( rOldSet, SID_SW_FOLLOW_TEXT_FLOW);
                if(pItem)
                {
                    std::unique_ptr<SfxBoolItem> pFollow(static_cast<SfxBoolItem*>(pItem->Clone()));
                    pFollow->SetValue(m_pFollowCB->IsChecked());
                    bModified |= nullptr != rSet->Put(*pFollow);
                }
            }
        }
    }
    if ( m_pWidthMF->IsValueModified() || m_pHeightMF->IsValueModified() )
    {
        sal_uInt32 nWidth = static_cast<sal_uInt32>(m_pWidthMF->Denormalize(m_pWidthMF->GetValue(FUNIT_TWIP)));
        sal_uInt32 nHeight = static_cast<sal_uInt32>(m_pHeightMF->Denormalize(m_pHeightMF->GetValue(FUNIT_TWIP)));
        rSet->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ), nWidth ) );
        rSet->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ), nHeight ) );
        //this item is required by SdrEditView::SetGeoAttrToMarked()
        rSet->Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), (sal_uInt16)RectPoint::LT ) );

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
        nAnchorType = (RndStdIds) static_cast<const SfxInt16Item*>(pItem)->GetValue();
        switch(nAnchorType)
        {
            case RndStdIds::FLY_AT_PAGE:   m_pToPageRB->Check();  break;
            case RndStdIds::FLY_AT_PARA:   m_pToParaRB->Check();  break;
            case RndStdIds::FLY_AT_CHAR:   m_pToCharRB->Check();  break;
            case RndStdIds::FLY_AS_CHAR:   m_pAsCharRB->Check();  break;
            case RndStdIds::FLY_AT_FLY:    m_pToFrameRB->Check(); break;
            default : bInvalidateAnchor = true;
        }
        m_pToPageRB->SaveValue();
        m_pToParaRB->SaveValue();
        m_pToCharRB->SaveValue();
        m_pAsCharRB->SaveValue();
        m_pToFrameRB->SaveValue();
    }
    if(bInvalidateAnchor)
    {
        m_pToPageRB->Enable( false );
        m_pToParaRB->Enable( false );
        m_pToCharRB->Enable( false );
        m_pAsCharRB->Enable( false );
        m_pToFrameRB->Enable( false );
    }

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_PROTECT_POS );
    if ( pItem )
    {
        bool bProtected = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_pPositionCB->SetState( bProtected ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pPositionCB->EnableTriState( false );
        m_pSizeCB->Enable( !bProtected );
    }
    else
    {
        m_pPositionCB->SetState( TRISTATE_INDET );
    }

    m_pPositionCB->SaveValue();

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_PROTECT_SIZE );

    if ( pItem )
    {
        m_pSizeCB->SetState( static_cast<const SfxBoolItem*>(pItem)->GetValue()
                              ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pSizeCB->EnableTriState( false );
    }
    else
        m_pSizeCB->SetState( TRISTATE_INDET );
    m_pSizeCB->SaveValue();

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
        OUString sHLabel = m_pHoriFT->GetText();
        m_pHoriFT->SetText(m_pVertFT->GetText());
        m_pVertFT->SetText(sHLabel);
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
        m_pFollowCB->Check( bFollowTextFlow );
    }
    m_pFollowCB->SaveValue();

    if(m_bHtmlMode)
    {
        m_pHoriMirrorCB->Show(false);
        m_pKeepRatioCB->Enable(false);
        // #i18732# - hide checkbox in HTML mode
        m_pFollowCB->Show(false);
    }
    else
    {
        // #i18732# correct enable/disable of check box 'Mirror on..'
        m_pHoriMirrorCB->Enable(!m_pAsCharRB->IsChecked() && !m_bIsMultiSelection);

        // #i18732# - enable/disable check box 'Follow text flow'.
        m_pFollowCB->Enable( m_pToParaRB->IsChecked() ||
                                  m_pToCharRB->IsChecked() );
    }

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_WIDTH );
    sal_Int32 nWidth = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, (sal_uInt32)1 );

    m_pWidthMF->SetValue(m_pWidthMF->Normalize(nWidth), FUNIT_TWIP);

    pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HEIGHT );
    sal_Int32 nHeight = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, (sal_uInt32)1 );
    m_pHeightMF->SetValue(m_pHeightMF->Normalize(nHeight), FUNIT_TWIP);
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
            m_pHoriMirrorCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
        m_pHoriMirrorCB->SaveValue();

        sal_Int32 nHoriPos = 0;
        sal_Int32 nVertPos = 0;
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_HORI_POSITION);
        if(pItem)
            nHoriPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        pItem = GetItem( *rSet, SID_ATTR_TRANSFORM_VERT_POSITION);
        if(pItem)
            nVertPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();

        InitPos(nAnchorType, m_nOldH, m_nOldHRel, m_nOldV, m_nOldVRel, nHoriPos, nVertPos);

        m_pVertByMF->SaveValue();
        m_pHoriByMF->SaveValue();
        // #i18732#
        m_pFollowCB->SaveValue();

        RangeModifyHdl(*m_pWidthMF);  // initially set maximum values
    }
}

DeactivateRC SvxSwPosSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxSwPosSizeTabPage::EnableAnchorTypes(SvxAnchorIds nAnchorEnable)
{
    if((nAnchorEnable & SvxAnchorIds::Fly))
        m_pToFrameRB->Show();
    if(!(nAnchorEnable & SvxAnchorIds::Page))
        m_pToPageRB->Enable(false);
}

RndStdIds SvxSwPosSizeTabPage::GetAnchorType(bool* pbHasChanged)
{
    RndStdIds nRet = RndStdIds::UNKNOWN;
    RadioButton* pCheckedButton = nullptr;
    if(m_pToParaRB->IsEnabled())
    {
        if(m_pToPageRB->IsChecked())
        {
            nRet = RndStdIds::FLY_AT_PAGE;
            pCheckedButton = m_pToPageRB;
        }
        else if(m_pToParaRB->IsChecked())
        {
            nRet = RndStdIds::FLY_AT_PARA;
            pCheckedButton = m_pToParaRB;
        }
        else if(m_pToCharRB->IsChecked())
        {
            nRet = RndStdIds::FLY_AT_CHAR;
            pCheckedButton = m_pToCharRB;
        }
        else if(m_pAsCharRB->IsChecked())
        {
            nRet = RndStdIds::FLY_AS_CHAR;
            pCheckedButton = m_pAsCharRB;
        }
        else if(m_pToFrameRB->IsChecked())
        {
            nRet = RndStdIds::FLY_AT_FLY;
            pCheckedButton = m_pToFrameRB;
        }
    }
    if(pbHasChanged)
    {
         if(pCheckedButton)
             *pbHasChanged = pCheckedButton->IsValueChangedFromSaved();
         else
             *pbHasChanged = false;
    }
   return nRet;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, RangeModifyClickHdl, Button*, void)
{
    RangeModifyHdl(*m_pWidthMF);
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, RangeModifyHdl, Control&, void)
{
    if(m_bPositioningDisabled)
        return;
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = GetAnchorType();
    aVal.bAutoHeight = false;
    aVal.bMirror = m_pHoriMirrorCB->IsChecked();
    // #i18732#
    aVal.bFollowTextFlow = m_pFollowCB->IsChecked();

    if ( m_pHMap )
    {
        // horizontal alignment
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_pHoriToLB);
        sal_uInt16 nAlign = GetAlignment(m_pHMap, nMapPos, *m_pHoriToLB);
        sal_uInt16 nRel = GetRelation(*m_pHoriToLB);

        aVal.nHoriOrient = (short)nAlign;
        aVal.nHRelOrient = (short)nRel;
    }
    else
        aVal.nHoriOrient = HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // vertical alignment
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_pVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, *m_pVertToLB);
        sal_uInt16 nRel = GetRelation(*m_pVertToLB);

        aVal.nVertOrient = (short)nAlign;
        aVal.nVRelOrient = (short)nRel;
    }
    else
        aVal.nVertOrient = VertOrientation::NONE;

    const long nAtHorzPosVal =
                    static_cast<long>(m_pHoriByMF->Denormalize(m_pHoriByMF->GetValue(FUNIT_TWIP)));
    const long nAtVertPosVal =
                    static_cast<long>(m_pVertByMF->Denormalize(m_pVertByMF->GetValue(FUNIT_TWIP)));

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    sal_Int32 nWidth = static_cast<sal_uInt32>(m_pWidthMF-> Denormalize(m_pWidthMF->GetValue(FUNIT_TWIP)));
    sal_Int32 nHeight = static_cast<sal_uInt32>(m_pHeightMF->Denormalize(m_pHeightMF->GetValue(FUNIT_TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    m_aValidateLink.Call(aVal);

    // minimum width also for style
    m_pHeightMF->SetMin(m_pHeightMF->Normalize(aVal.nMinHeight), FUNIT_TWIP);
    m_pWidthMF-> SetMin(m_pWidthMF->Normalize(aVal.nMinWidth), FUNIT_TWIP);

    sal_Int32 nMaxWidth(aVal.nMaxWidth);
    sal_Int32 nMaxHeight(aVal.nMaxHeight);

    sal_Int64 nTmp = m_pHeightMF->Normalize(nMaxHeight);
    m_pHeightMF->SetMax(nTmp, FUNIT_TWIP);

    nTmp = m_pWidthMF->Normalize(nMaxWidth);
    m_pWidthMF->SetMax(nTmp, FUNIT_TWIP);

    m_pHoriByMF->SetMin(m_pHoriByMF->Normalize(aVal.nMinHPos), FUNIT_TWIP);
    m_pHoriByMF->SetMax(m_pHoriByMF->Normalize(aVal.nMaxHPos), FUNIT_TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        m_pHoriByMF->SetValue(m_pHoriByMF->Normalize(aVal.nHPos), FUNIT_TWIP);

    m_pVertByMF->SetMin(m_pVertByMF->Normalize(aVal.nMinVPos ), FUNIT_TWIP);
    m_pVertByMF->SetMax(m_pVertByMF->Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_pVertByMF->SetValue(m_pVertByMF->Normalize(aVal.nVPos), FUNIT_TWIP);
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, AnchorTypeHdl, Button*, void)
{
    m_pHoriMirrorCB->Enable(!m_pAsCharRB->IsChecked() && !m_bIsMultiSelection);

    // #i18732# - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    m_pFollowCB->Enable( m_pToParaRB->IsChecked() || m_pToCharRB->IsChecked() );

    RndStdIds nId = GetAnchorType();

    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl(*m_pWidthMF);

    if(m_bHtmlMode)
    {
        PosHdl(*m_pHoriLB);
        PosHdl(*m_pVertLB);
    }
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, MirrorHdl, Button*, void)
{
    RndStdIds nId = GetAnchorType();
    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
}

IMPL_LINK( SvxSwPosSizeTabPage, RelHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHoriToLB;

    UpdateExample();

    if (m_bHtmlMode && RndStdIds::FLY_AT_CHAR == GetAnchorType()) // again special treatment
    {
        if(bHori)
        {
            sal_uInt16 nRel = GetRelation(*m_pHoriToLB);
            if(RelOrientation::PRINT_AREA == nRel && 0 == m_pVertLB->GetSelectedEntryPos())
            {
                m_pVertLB->SelectEntryPos(1);
            }
            else if(RelOrientation::CHAR == nRel && 1 == m_pVertLB->GetSelectedEntryPos())
            {
                m_pVertLB->SelectEntryPos(0);
            }
        }
    }
    RangeModifyHdl(*m_pWidthMF);
}

IMPL_LINK( SvxSwPosSizeTabPage, PosHdl, ListBox&, rLB, void )
{
    bool bHori = &rLB == m_pHoriLB;
    ListBox *pRelLB = bHori ? m_pHoriToLB.get() : m_pVertToLB.get();
    FixedText *pRelFT = bHori ? m_pHoriToFT.get() : m_pVertToFT.get();
    FrmMap *pMap = bHori ? m_pHMap : m_pVMap;


    sal_uInt16 nMapPos = GetMapPos(pMap, rLB);
    sal_uInt16 nAlign = GetAlignment(pMap, nMapPos, *pRelLB);

    if (bHori)
    {
        bool bEnable = HoriOrientation::NONE == nAlign;
        m_pHoriByMF->Enable( bEnable );
        m_pHoriByFT->Enable( bEnable );
    }
    else
    {
        bool bEnable = VertOrientation::NONE == nAlign;
        m_pVertByMF->Enable( bEnable );
        m_pVertByFT->Enable( bEnable );
    }

    RangeModifyHdl( *m_pWidthMF );

    short nRel = 0;
    if (rLB.GetSelectedEntryCount())
    {

        if (pRelLB->GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND)
            nRel = static_cast<RelationMap *>(pRelLB->GetSelectedEntryData())->nRelation;

        FillRelLB(pMap, nMapPos, nAlign, nRel, *pRelLB, *pRelFT);
    }
    else
        pRelLB->Clear();

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
                    0 == m_pVertLB->GetSelectedEntryPos())
            {
                if(RelOrientation::FRAME == nRel)
                    m_pVertLB->SelectEntryPos(1);
                else
                    m_pVertLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(HoriOrientation::LEFT == nAlign && 1 == m_pVertLB->GetSelectedEntryPos())
            {
                m_pVertLB->SelectEntryPos(0);
                bSet = true;
            }
            else if(HoriOrientation::NONE == nAlign && 1 == m_pVertLB->GetSelectedEntryPos())
            {
                m_pVertLB->SelectEntryPos(0);
                bSet = true;
            }
            if(bSet)
                PosHdl(*m_pVertLB);
        }
        else
        {
            if(VertOrientation::TOP == nAlign)
            {
                if(1 == m_pHoriLB->GetSelectedEntryPos())
                {
                    m_pHoriLB->SelectEntryPos(0);
                    bSet = true;
                }
                m_pHoriToLB->SelectEntryPos(1);
            }
            else if(VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == m_pHoriLB->GetSelectedEntryPos())
                {
                    m_pHoriLB->SelectEntryPos(0);
                    bSet = true;
                }
                m_pHoriToLB->SelectEntryPos(0) ;
            }
            if(bSet)
                PosHdl(*m_pHoriLB);
        }

    }
}

IMPL_LINK( SvxSwPosSizeTabPage, ModifyHdl, Edit&, rEdit, void )
{
    sal_Int64 nWidth = m_pWidthMF->Denormalize(m_pWidthMF->GetValue(FUNIT_TWIP));
    sal_Int64 nHeight = m_pHeightMF->Denormalize(m_pHeightMF->GetValue(FUNIT_TWIP));
    if ( m_pKeepRatioCB->IsChecked() )
    {
        if ( &rEdit == m_pWidthMF )
        {
            nHeight = sal_Int64((double)nWidth / m_fWidthHeightRatio);
            m_pHeightMF->SetValue(m_pHeightMF->Normalize(nHeight), FUNIT_TWIP);
        }
        else if(&rEdit == m_pHeightMF)
        {
            nWidth = sal_Int64((double)nHeight * m_fWidthHeightRatio);
            m_pWidthMF->SetValue(m_pWidthMF->Normalize(nWidth), FUNIT_TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, ProtectHdl, Button*, void)
{
    m_pSizeCB->Enable(m_pPositionCB->IsEnabled() && !m_pPositionCB->IsChecked());
}

short SvxSwPosSizeTabPage::GetRelation(ListBox const &rRelationLB)
{
    short nRel = 0;
    sal_Int32 nPos = rRelationLB.GetSelectedEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        RelationMap *pEntry = static_cast<RelationMap *>(rRelationLB.GetEntryData(nPos));
        nRel = pEntry->nRelation;
    }

    return nRel;
}

short SvxSwPosSizeTabPage::GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos, ListBox const &rRelationLB)
{
    short nAlign = 0;

    // #i22341# - special handling also for map <aVCharMap>,
    // because it contains ambiguous items for alignment
    if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap ||
            pMap == aVCharMap )
    {
        if (rRelationLB.GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            LB  nRel = static_cast<RelationMap *>(rRelationLB.GetSelectedEntryData())->nLBRelation;
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

sal_uInt16 SvxSwPosSizeTabPage::GetMapPos(FrmMap *pMap, ListBox const &rAlignLB)
{
    sal_uInt16 nMapPos = 0;
    sal_Int32 nLBSelPos = rAlignLB.GetSelectedEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            OUString sSelEntry(rAlignLB.GetSelectedEntry());

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
    sal_Int32 nPos = m_pVertLB->GetSelectedEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pVMap )
    {
        m_nOldV    = m_pVMap[nPos].nAlign;
        nPos = m_pVertToLB->GetSelectedEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldVRel = static_cast<RelationMap *>(m_pVertToLB->GetEntryData(nPos))->nRelation;
    }

    nPos = m_pHoriLB->GetSelectedEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pHMap )
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_pHoriToLB->GetSelectedEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldHRel = static_cast<RelationMap *>(m_pHoriToLB->GetEntryData(nPos))->nRelation;
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
    m_pHoriLB->Enable( bEnable );
    m_pHoriFT->Enable( bEnable );

    // select current Pos
    // horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = m_nOldH;
        nHRel = m_nOldHRel;
    }
    // #i22341# - pass <nHRel> as 3rd parameter to method <FillPosLB>
    sal_uInt16 nMapPos = FillPosLB(m_pHMap, nH, nHRel, *m_pHoriLB);
    FillRelLB(m_pHMap, nMapPos, nH, nHRel, *m_pHoriToLB, *m_pHoriToFT);

    // vertical
    if ( nV == USHRT_MAX )
    {
        nV    = m_nOldV;
        nVRel = m_nOldVRel;
    }
    // #i22341# - pass <nVRel> as 3rd parameter to method <FillPosLB>
    nMapPos = FillPosLB(m_pVMap, nV, nVRel, *m_pVertLB);
    FillRelLB(m_pVMap, nMapPos, nV, nVRel, *m_pVertToLB, *m_pVertToFT);

    // Edits init
    bEnable = nH == HoriOrientation::NONE && nAnchor != RndStdIds::FLY_AS_CHAR; //#61359# why not in formats&& !bFormat;
    if (!bEnable)
    {
        m_pHoriByMF->SetValue( 0, FUNIT_TWIP );
        if (nX != LONG_MAX && m_bHtmlMode)
            m_pHoriByMF->SetModifyFlag();
    }
    else if(m_bIsMultiSelection)
    {
         m_pHoriByMF->SetValue( m_pHoriByMF->Normalize(m_aRect.Left()), FUNIT_TWIP );
    }
    else
    {
        if (nX != LONG_MAX)
            m_pHoriByMF->SetValue( m_pHoriByMF->Normalize(nX), FUNIT_TWIP );
    }
    m_pHoriByFT->Enable( bEnable );
    m_pHoriByMF->Enable( bEnable );

    bEnable = nV == VertOrientation::NONE;
    if ( !bEnable )
    {
        m_pVertByMF->SetValue( 0, FUNIT_TWIP );
        if(nY != LONG_MAX && m_bHtmlMode)
            m_pVertByMF->SetModifyFlag();
    }
    else if(m_bIsMultiSelection)
    {
         m_pVertByMF->SetValue( m_pVertByMF->Normalize(m_aRect.Top()), FUNIT_TWIP );
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
            m_pVertByMF->SetValue( m_pVertByMF->Normalize(nY), FUNIT_TWIP );
    }
    m_pVertByFT->Enable( bEnable );
    m_pVertByMF->Enable( bEnable );
    UpdateExample();
}

void SvxSwPosSizeTabPage::UpdateExample()
{
    sal_Int32 nPos = m_pHoriLB->GetSelectedEntryPos();
    if ( m_pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, *m_pHoriLB);
        short nAlign = GetAlignment(m_pHMap, nMapPos, *m_pHoriToLB);
        short nRel = GetRelation(*m_pHoriToLB);

        m_pExampleWN->SetHAlign(nAlign);
        m_pExampleWN->SetHoriRel(nRel);
    }

    nPos = m_pVertLB->GetSelectedEntryPos();
    if ( m_pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, *m_pVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, *m_pVertToLB);
        sal_uInt16 nRel = GetRelation(*m_pVertToLB);

        m_pExampleWN->SetVAlign(nAlign);
        m_pExampleWN->SetVertRel(nRel);
    }

    // Size
    long nXPos = static_cast<long>(m_pHoriByMF->Denormalize(m_pHoriByMF->GetValue(FUNIT_TWIP)));
    long nYPos = static_cast<long>(m_pVertByMF->Denormalize(m_pVertByMF->GetValue(FUNIT_TWIP)));
    m_pExampleWN->SetRelPos(Point(nXPos, nYPos));

    m_pExampleWN->SetAnchor( GetAnchorType() );
    m_pExampleWN->Invalidate();
}

void SvxSwPosSizeTabPage::FillRelLB(FrmMap *pMap, sal_uInt16 nMapPos, sal_uInt16 nAlign,
        sal_uInt16 nRel, ListBox &rLB, FixedText &rFT)
{
    OUString sSelEntry;
    LB  nLBRelations = LB::NONE;
    std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);

    rLB.Clear();

    if (nMapPos < nMapCount)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            OUString sOldEntry(rLB.GetSelectedEntry());
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (std::size_t _nMapPos = 0; _nMapPos < nMapCount; _nMapPos++)
            {
                if (pMap[_nMapPos].eStrId == eStrId)
                {
                    nLBRelations = pMap[_nMapPos].nLBRelations;
                    for (sal_uInt16 nRelPos = 0; nRelPos < SAL_N_ELEMENTS(aAsCharRelationMap); nRelPos++)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = aAsCharRelationMap[nRelPos].eStrId;

                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            OUString sEntry = SvxSwFramePosString::GetString(sStrId1);
                            sal_Int32 nPos = rLB.InsertEntry(sEntry);
                            rLB.SetEntryData(nPos, &aAsCharRelationMap[nRelPos]);
                            if (pMap[_nMapPos].nAlign == nAlign)
                                sSelEntry = sEntry;
                            break;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                rLB.SelectEntry(sSelEntry);
            else
            {
                rLB.SelectEntry(sOldEntry);

                if (!rLB.GetSelectedEntryCount())
                {
                    for (sal_Int32 i = 0; i < rLB.GetEntryCount(); i++)
                    {
                        RelationMap *pEntry = static_cast<RelationMap *>(rLB.GetEntryData(i));
                        if (pEntry->nLBRelation == LB::RelChar) // Default
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
            // #i22341# - special handling for map <aVCharMap>,
            // because its ambiguous in its <eStrId>/<eMirrorStrId>.
            if ( pMap == aVCharMap )
            {
                nLBRelations = ::lcl_GetLBRelationsForStrID( pMap,
                                             ( m_pHoriMirrorCB->IsChecked()
                                               ? pMap[nMapPos].eMirrorStrId
                                               : pMap[nMapPos].eStrId ),
                                             m_pHoriMirrorCB->IsChecked() );
            }
            else
            {
                nLBRelations = pMap[nMapPos].nLBRelations;
            }

            for (sal_uLong nBit = 1; nBit < (sal_uLong)LB::LAST; nBit <<= 1)
            {
                if (nLBRelations & (LB)nBit)
                {
                    for (sal_uInt16 nRelPos = 0; nRelPos < SAL_N_ELEMENTS(aRelationMap); nRelPos++)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == (LB)nBit)
                        {
                            SvxSwFramePosString::StringId sStrId1 = m_pHoriMirrorCB->IsChecked() ? aRelationMap[nRelPos].eMirrorStrId : aRelationMap[nRelPos].eStrId;
                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            OUString sEntry = SvxSwFramePosString::GetString(sStrId1);
                            sal_Int32 nPos = rLB.InsertEntry(sEntry);
                            rLB.SetEntryData(nPos, &aRelationMap[nRelPos]);
                            if (sSelEntry.isEmpty() && aRelationMap[nRelPos].nRelation == nRel)
                                sSelEntry = sEntry;
                        }
                    }
                }
            }
            if (!sSelEntry.isEmpty())
                rLB.SelectEntry(sSelEntry);
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
                        if (rLB.GetEntryCount())
                        {
                            RelationMap *pEntry = static_cast<RelationMap *>(rLB.GetEntryData(rLB.GetEntryCount() - 1));
                            nRel = pEntry->nRelation;
                        }
                        break;
                }

                for (sal_Int32 i = 0; i < rLB.GetEntryCount(); i++)
                {
                    RelationMap *pEntry = static_cast<RelationMap *>(rLB.GetEntryData(i));
                    if (pEntry->nRelation == nRel)
                    {
                        rLB.SelectEntryPos(i);
                        break;
                    }
                }

                if (!rLB.GetSelectedEntryCount())
                    rLB.SelectEntryPos(0);
            }
        }
    }

    rLB.Enable(rLB.GetEntryCount() != 0);
    rFT.Enable(rLB.GetEntryCount() != 0);

    RelHdl(rLB);
}

sal_uInt16 SvxSwPosSizeTabPage::FillPosLB(FrmMap *_pMap,
                                      sal_uInt16 _nAlign,
                                      const sal_uInt16 _nRel,
                                      ListBox &_rLB)
{
    OUString sSelEntry, sOldEntry;
    sOldEntry = _rLB.GetSelectedEntry();

    _rLB.Clear();

    // #i22341# - determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const LB nLBRelations = (_pMap != aVCharMap)
                               ? LB::NONE
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill listbox
    std::size_t nCount = ::lcl_GetFrmMapCount(_pMap);
    for (std::size_t i = 0; _pMap && i < nCount; ++i)
    {
        SvxSwFramePosString::StringId eStrId = m_pHoriMirrorCB->IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
        eStrId = lcl_ChangeResIdToVerticalOrRTL(eStrId, m_bIsVerticalFrame, m_bIsInRightToLeft);
        OUString sEntry(SvxSwFramePosString::GetString(eStrId));
        if (_rLB.GetEntryPos(sEntry) == LISTBOX_ENTRY_NOTFOUND)
        {
            // don't insert duplicate entries at character wrapped borders
            _rLB.InsertEntry(sEntry);
        }
        // #i22341# - add condition to handle map <aVCharMap>
        // that is ambiguous in the alignment.
        if ( _pMap[i].nAlign == _nAlign &&
             ( !(_pMap == aVCharMap) || _pMap[i].nLBRelations & nLBRelations ) )
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
                    m_pPosFrame->Enable(false);
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
