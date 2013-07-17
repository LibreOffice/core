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

#include <cstddef>
#include <swpossizetabpage.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/anchorid.hxx>
#include <svl/aeitem.hxx>
#include <svx/swframevalidation.hxx>
#include <cuires.hrc>
#include <swpossizetabpage.hrc>
#include <sfx2/htmlmode.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/rectenum.hxx>
#include <sal/macros.h>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::text;
#define SwFPos SvxSwFramePosString

struct FrmMap
{
    SvxSwFramePosString::StringId   eStrId;
    SvxSwFramePosString::StringId   eMirrorStrId;
    short                           nAlign;
    sal_uLong                           nLBRelations;
};

struct RelationMap
{
    SvxSwFramePosString::StringId   eStrId;
    SvxSwFramePosString::StringId   eMirrorStrId;
    sal_uLong  nLBRelation;
    short nRelation;
};
struct StringIdPair_Impl
{
    SvxSwFramePosString::StringId eHori;
    SvxSwFramePosString::StringId eVert;
};

#define LB_FRAME                0x00000001L // paragraph text area
#define LB_PRTAREA              0x00000002L // paragraph text area + indents
#define LB_VERT_FRAME           0x00000004L // vertical paragraph text area
#define LB_VERT_PRTAREA         0x00000008L // vertical paragraph text area + indents
#define LB_REL_FRM_LEFT         0x00000010L // left paragraph margin
#define LB_REL_FRM_RIGHT        0x00000020L // right paragraph margin

#define LB_REL_PG_LEFT          0x00000040L // left page margin
#define LB_REL_PG_RIGHT         0x00000080L // right page margin
#define LB_REL_PG_FRAME         0x00000100L // complete page
#define LB_REL_PG_PRTAREA       0x00000200L // text area of page

#define LB_FLY_REL_PG_LEFT      0x00000400L // left frame margin
#define LB_FLY_REL_PG_RIGHT     0x00000800L // right frame margin
#define LB_FLY_REL_PG_FRAME     0x00001000L // complete frame
#define LB_FLY_REL_PG_PRTAREA   0x00002000L // frame interior

#define LB_REL_BASE             0x00010000L // as char, relative to baseline
#define LB_REL_CHAR             0x00020000L // as char, relative to character
#define LB_REL_ROW              0x00040000L // as char, relative to line

// #i22305#
#define LB_FLY_VERT_FRAME       0x00100000L // vertical entire frame
#define LB_FLY_VERT_PRTAREA     0x00200000L // vertical frame text area

// #i22341#
#define LB_VERT_LINE            0x00400000L // vertical text line

static RelationMap aRelationMap[] =
{
    {SwFPos::FRAME,         SwFPos::FRAME,             LB_FRAME,           RelOrientation::FRAME},
    {SwFPos::PRTAREA,       SwFPos::PRTAREA,           LB_PRTAREA,         RelOrientation::PRINT_AREA},
    {SwFPos::REL_PG_LEFT,   SwFPos::MIR_REL_PG_LEFT,   LB_REL_PG_LEFT,     RelOrientation::PAGE_LEFT},
    {SwFPos::REL_PG_RIGHT,  SwFPos::MIR_REL_PG_RIGHT,  LB_REL_PG_RIGHT,    RelOrientation::PAGE_RIGHT},
    {SwFPos::REL_FRM_LEFT,  SwFPos::MIR_REL_FRM_LEFT,  LB_REL_FRM_LEFT,    RelOrientation::FRAME_LEFT},
    {SwFPos::REL_FRM_RIGHT, SwFPos::MIR_REL_FRM_RIGHT, LB_REL_FRM_RIGHT,   RelOrientation::FRAME_RIGHT},
    {SwFPos::REL_PG_FRAME,  SwFPos::REL_PG_FRAME,      LB_REL_PG_FRAME,    RelOrientation::PAGE_FRAME},
    {SwFPos::REL_PG_PRTAREA,SwFPos::REL_PG_PRTAREA,    LB_REL_PG_PRTAREA,  RelOrientation::PAGE_PRINT_AREA},
    {SwFPos::REL_CHAR,      SwFPos::REL_CHAR,          LB_REL_CHAR,        RelOrientation::CHAR},

    {SwFPos::FLY_REL_PG_LEFT,       SwFPos::FLY_MIR_REL_PG_LEFT,    LB_FLY_REL_PG_LEFT,     RelOrientation::PAGE_LEFT},
    {SwFPos::FLY_REL_PG_RIGHT,      SwFPos::FLY_MIR_REL_PG_RIGHT,   LB_FLY_REL_PG_RIGHT,    RelOrientation::PAGE_RIGHT},
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB_FLY_REL_PG_FRAME,    RelOrientation::PAGE_FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB_FLY_REL_PG_PRTAREA,  RelOrientation::PAGE_PRINT_AREA},

    {SwFPos::REL_BORDER,        SwFPos::REL_BORDER,             LB_VERT_FRAME,          RelOrientation::FRAME},
    {SwFPos::REL_PRTAREA,       SwFPos::REL_PRTAREA,            LB_VERT_PRTAREA,        RelOrientation::PRINT_AREA},

    // #i22305#
    {SwFPos::FLY_REL_PG_FRAME,      SwFPos::FLY_REL_PG_FRAME,       LB_FLY_VERT_FRAME,      RelOrientation::FRAME},
    {SwFPos::FLY_REL_PG_PRTAREA,    SwFPos::FLY_REL_PG_PRTAREA,     LB_FLY_VERT_PRTAREA,    RelOrientation::PRINT_AREA},

    // #i22341#
    {SwFPos::REL_LINE,  SwFPos::REL_LINE,   LB_VERT_LINE,   RelOrientation::TEXT_LINE}
};

static RelationMap aAsCharRelationMap[] =
{
    {SwFPos::REL_BASE,  SwFPos::REL_BASE,   LB_REL_BASE,   RelOrientation::FRAME},
    {SwFPos::REL_CHAR,  SwFPos::REL_CHAR,   LB_REL_CHAR,   RelOrientation::FRAME},
    {SwFPos::REL_ROW,   SwFPos::REL_ROW,   LB_REL_ROW,     RelOrientation::FRAME}
};

/*--------------------------------------------------------------------
    Anchored at page
 --------------------------------------------------------------------*/

#define HORI_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_PG_LEFT| \
                        LB_REL_PG_RIGHT)

static FrmMap aHPageMap[] =
{
    {SwFPos::LEFT,       SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PAGE_REL},
    {SwFPos::RIGHT,      SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PAGE_REL},
    {SwFPos::CENTER_HORI,SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PAGE_REL},
    {SwFPos::FROMLEFT,   SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PAGE_REL}
};

static FrmMap aHPageHtmlMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

#define VERT_PAGE_REL   (LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVPageMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PAGE_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PAGE_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PAGE_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PAGE_REL}
};

static FrmMap aVPageHtmlMap[] =
{
    {SwFPos::FROMTOP, SwFPos::FROMTOP,        VertOrientation::NONE,      LB_REL_PG_FRAME}
};

/*--------------------------------------------------------------------
    Anchored at frame
 --------------------------------------------------------------------*/

#define HORI_FRAME_REL  (LB_FLY_REL_PG_FRAME|LB_FLY_REL_PG_PRTAREA| \
                        LB_FLY_REL_PG_LEFT|LB_FLY_REL_PG_RIGHT)

static FrmMap aHFrameMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,  HORI_FRAME_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_FRAME_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_FRAME_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_FRAME_REL}
};

static FrmMap aHFlyHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      LB_FLY_REL_PG_FRAME},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB_FLY_REL_PG_FRAME}
};

// #i18732# - own vertical alignment map for to frame anchored objects
// #i22305#
#define VERT_FRAME_REL   (LB_VERT_FRAME|LB_FLY_VERT_PRTAREA)

static FrmMap aVFrameMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_FRAME_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_FRAME_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_FRAME_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_FRAME_REL}
};

static FrmMap aVFlyHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB_FLY_VERT_FRAME},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      LB_FLY_VERT_FRAME}
};

static FrmMap aVMultiSelectionMap[] =
{
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      0}
};
static FrmMap aHMultiSelectionMap[] =
{
    {SwFPos::FROMLEFT,      SwFPos::FROMLEFT,       HoriOrientation::NONE,      0}
};

/*--------------------------------------------------------------------
    Anchored at paragraph
 --------------------------------------------------------------------*/

#define HORI_PARA_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT)

static FrmMap aHParaMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_PARA_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_PARA_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_PARA_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_PARA_REL}
};

#define HTML_HORI_PARA_REL  (LB_FRAME|LB_PRTAREA)

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


#define VERT_PARA_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVParaMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       VERT_PARA_REL},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,    VERT_PARA_REL},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,    VERT_PARA_REL},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,      VERT_PARA_REL}
};

static FrmMap aVParaHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,       LB_VERT_PRTAREA}
};

/*--------------------------------------------------------------------
    Anchored at character
 --------------------------------------------------------------------*/

#define HORI_CHAR_REL   (LB_FRAME|LB_PRTAREA|LB_REL_PG_LEFT|LB_REL_PG_RIGHT| \
                        LB_REL_PG_FRAME|LB_REL_PG_PRTAREA|LB_REL_FRM_LEFT| \
                        LB_REL_FRM_RIGHT|LB_REL_CHAR)

static FrmMap aHCharMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,      HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     HORI_CHAR_REL},
    {SwFPos::CENTER_HORI,   SwFPos::CENTER_HORI,    HoriOrientation::CENTER,    HORI_CHAR_REL},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      HORI_CHAR_REL}
};

#define HTML_HORI_CHAR_REL  (LB_FRAME|LB_PRTAREA|LB_REL_CHAR)

static FrmMap aHCharHtmlMap[] =
{
    {SwFPos::LEFT,          SwFPos::LEFT,           HoriOrientation::LEFT,      HTML_HORI_CHAR_REL},
    {SwFPos::RIGHT,         SwFPos::RIGHT,          HoriOrientation::RIGHT,     HTML_HORI_CHAR_REL}
};

static FrmMap aHCharHtmlAbsMap[] =
{
    {SwFPos::LEFT,          SwFPos::MIR_LEFT,       HoriOrientation::LEFT,          LB_PRTAREA|LB_REL_CHAR},
    {SwFPos::RIGHT,         SwFPos::MIR_RIGHT,      HoriOrientation::RIGHT,     LB_PRTAREA},
    {SwFPos::FROMLEFT,      SwFPos::MIR_FROMLEFT,   HoriOrientation::NONE,      LB_REL_PG_FRAME}
};

// #i18732# - allow vertical alignment at page areas
// #i22341# - handle <LB_REL_CHAR> on its own
#define VERT_CHAR_REL   (LB_VERT_FRAME|LB_VERT_PRTAREA| \
                         LB_REL_PG_FRAME|LB_REL_PG_PRTAREA)

static FrmMap aVCharMap[] =
{
    // #i22341#
    // introduce mappings for new vertical alignment at top of line <LB_VERT_LINE>
    // and correct mapping for vertical alignment at character for position <FROM_BOTTOM>
    // Note: Because of these adjustments the map becomes ambigous in its values
    //       <eStrId>/<eMirrorStrId> and <nAlign>. These ambiguities are considered
    //       in the methods <SwFrmPage::FillRelLB(..)>, <SwFrmPage::GetAlignment(..)>
    //       and <SwFrmPage::FillPosLB(..)>
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,        VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::BELOW,         SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        VERT_CHAR_REL|LB_REL_CHAR},
    {SwFPos::FROMTOP,       SwFPos::FROMTOP,        VertOrientation::NONE,          VERT_CHAR_REL},
    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     VertOrientation::NONE,          LB_REL_CHAR|LB_VERT_LINE},
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB_VERT_LINE},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB_VERT_LINE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB_VERT_LINE}
};


static FrmMap aVCharHtmlMap[] =
{
    {SwFPos::BELOW,         SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};

static FrmMap aVCharHtmlAbsMap[] =
{
    {SwFPos::TOP,    SwFPos::TOP,            VertOrientation::TOP,           LB_REL_CHAR},
    {SwFPos::BELOW,  SwFPos::BELOW,          VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR}
};
/*--------------------------------------------------------------------
    anchored as character
 --------------------------------------------------------------------*/

static FrmMap aVAsCharMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           LB_REL_BASE},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::BOTTOM,        LB_REL_BASE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        LB_REL_BASE},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::CHAR_TOP,      LB_REL_CHAR},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::CHAR_BOTTOM,   LB_REL_CHAR},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CHAR_CENTER,   LB_REL_CHAR},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB_REL_ROW},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB_REL_ROW},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB_REL_ROW},

    {SwFPos::FROMBOTTOM,    SwFPos::FROMBOTTOM,     VertOrientation::NONE,          LB_REL_BASE}
};

static FrmMap aVAsCharHtmlMap[] =
{
    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::TOP,           LB_REL_BASE},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::CENTER,        LB_REL_BASE},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::CHAR_TOP,      LB_REL_CHAR},

    {SwFPos::TOP,           SwFPos::TOP,            VertOrientation::LINE_TOP,      LB_REL_ROW},
    {SwFPos::BOTTOM,        SwFPos::BOTTOM,         VertOrientation::LINE_BOTTOM,   LB_REL_ROW},
    {SwFPos::CENTER_VERT,   SwFPos::CENTER_VERT,    VertOrientation::LINE_CENTER,   LB_REL_ROW}
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
            SvxSwFramePosString::StringId eStringId, sal_Bool bVertical, sal_Bool bRTL)
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
        nIndex = 0;
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
static sal_uLong lcl_GetLBRelationsForRelations( const sal_uInt16 _nRel )
{
    sal_uLong nLBRelations = 0L;

    for ( sal_uInt16 nRelMapPos = 0; nRelMapPos < SAL_N_ELEMENTS(aRelationMap); ++nRelMapPos )
    {
        if ( aRelationMap[nRelMapPos].nRelation == _nRel )
        {
            nLBRelations |= aRelationMap[nRelMapPos].nLBRelation;
        }
    }

    return nLBRelations;
}

// #i22341# - helper method on order to determine all possible
// listbox relations in a relation map for a given string ID
static sal_uLong lcl_GetLBRelationsForStrID( const FrmMap* _pMap,
                                  const SvxSwFramePosString::StringId _eStrId,
                                  const bool _bUseMirrorStr )
{
    sal_uLong nLBRelations = 0L;

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

SvxSwPosSizeTabPage::SvxSwPosSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_SWPOSSIZE ), rInAttrs ),
#ifdef _MSC_VER
#pragma warning (disable : 4355)
#endif
    m_aSizeFL(    this, CUI_RES( FL_SIZE   ) ),
    m_aWidthFT(   this, CUI_RES( FT_WIDTH  ) ),
    m_aWidthMF(   this, CUI_RES( MF_WIDTH  ) ),
    m_aHeightFT(  this, CUI_RES( FT_HEIGHT ) ),
    m_aHeightMF(  this, CUI_RES( MF_HEIGHT ) ),
    m_aKeepRatioCB(this,CUI_RES( CB_KEEPRATIO) ),
    m_aSeparatorFL(this,CUI_RES( FL_SEPARATOR) ),

    m_aAnchorFL(  this, CUI_RES( FL_ANCHOR ) ),
    m_aToPageRB(  this, CUI_RES( RB_TOPAGE ) ),
    m_aToParaRB(  this, CUI_RES( RB_TOPARA ) ),
    m_aToCharRB(  this, CUI_RES( RB_TOCHAR ) ),
    m_aAsCharRB(  this, CUI_RES( RB_ASCHAR ) ),
    m_aToFrameRB( this, CUI_RES( RB_TOFRAME) ),

    m_aProtectionFL(  this, CUI_RES( FL_PROTECTION  ) ),
    m_aPositionCB(    this, CUI_RES( CB_POSITION    ) ),
    m_aSizeCB(        this, CUI_RES( CB_SIZE        ) ),

    m_aPositionFL(    this, CUI_RES( FL_POSITION    ) ),
    m_aHoriFT(        this, CUI_RES( FT_HORI        ) ),
    m_aHoriLB(        this, CUI_RES( LB_HORI        ) ),
    m_aHoriByFT(      this, CUI_RES( FT_HORIBY      ) ),
    m_aHoriByMF(      this, CUI_RES( MF_HORIBY      ) ),
    m_aHoriToFT(      this, CUI_RES( FT_HORITO      ) ),
    m_aHoriToLB(      this, CUI_RES( LB_HORITO      ) ),

    m_aHoriMirrorCB(  this, CUI_RES( CB_HORIMIRROR  ) ),

    m_aVertFT(        this, CUI_RES( FT_VERT        ) ),
    m_aVertLB(        this, CUI_RES( LB_VERT        ) ),
    m_aVertByFT(      this, CUI_RES( FT_VERTBY      ) ),
    m_aVertByMF(      this, CUI_RES( MF_VERTBY      ) ),
    m_aVertToFT(      this, CUI_RES( FT_VERTTO      ) ),
    m_aVertToLB(      this, CUI_RES( LB_VERTTO      ) ),

    m_aFollowCB(      this, CUI_RES( CB_FOLLOW      ) ),
    m_aExampleWN( this,   CUI_RES( WN_EXAMPLE ) ),
#ifdef _MSC_VER
#pragma warning (default : 4355)
#endif
    m_pVMap( 0 ),
    m_pHMap( 0 ),
    m_pSdrView( 0 ),
    m_nOldH(HoriOrientation::CENTER),
    m_nOldHRel(RelOrientation::FRAME),
    m_nOldV(VertOrientation::TOP),
    m_nOldVRel(RelOrientation::PRINT_AREA),
    m_fWidthHeightRatio(1.0),
    m_nHtmlMode(0),
    m_bHtmlMode(false),
    m_bAtHoriPosModified(false),
    m_bAtVertPosModified(false),
    m_bIsVerticalFrame(false),
    m_bPositioningDisabled(false),
    m_bIsMultiSelection(false),
    m_bIsInRightToLeft(false)
{
    FreeResource();
    FieldUnit eDlgUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( m_aHoriByMF, eDlgUnit, sal_True );
    SetFieldUnit( m_aVertByMF, eDlgUnit, sal_True );
    SetFieldUnit( m_aWidthMF , eDlgUnit, sal_True );
    SetFieldUnit( m_aHeightMF, eDlgUnit, sal_True );

    SetExchangeSupport();

    Link aLk = LINK(this, SvxSwPosSizeTabPage, RangeModifyHdl);
    m_aWidthMF.    SetLoseFocusHdl( aLk );
    m_aHeightMF.   SetLoseFocusHdl( aLk );
    m_aHoriByMF.SetLoseFocusHdl( aLk );
    m_aVertByMF.SetLoseFocusHdl( aLk );
    m_aFollowCB.SetClickHdl( aLk );

    aLk = LINK(this, SvxSwPosSizeTabPage, ModifyHdl);
    m_aWidthMF.    SetModifyHdl( aLk );
    m_aHeightMF.   SetModifyHdl( aLk );
    m_aHoriByMF.SetModifyHdl( aLk );
    m_aVertByMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxSwPosSizeTabPage, AnchorTypeHdl);
    m_aToPageRB.SetClickHdl( aLk );
    m_aToParaRB.SetClickHdl( aLk );
    m_aToCharRB.SetClickHdl( aLk );
    m_aAsCharRB.SetClickHdl( aLk );
    m_aToFrameRB.SetClickHdl( aLk );

    m_aHoriLB.SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, PosHdl));
    m_aVertLB.  SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, PosHdl));

    m_aHoriToLB.SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, RelHdl));
    m_aVertToLB.SetSelectHdl(LINK(this, SvxSwPosSizeTabPage, RelHdl));

    m_aHoriMirrorCB.SetClickHdl(LINK(this, SvxSwPosSizeTabPage, MirrorHdl));
    m_aPositionCB.SetClickHdl(LINK(this, SvxSwPosSizeTabPage, ProtectHdl));
}

SvxSwPosSizeTabPage::~SvxSwPosSizeTabPage()
{
}

SfxTabPage* SvxSwPosSizeTabPage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new SvxSwPosSizeTabPage(pParent, rSet);
}

sal_uInt16* SvxSwPosSizeTabPage::GetRanges()
{
    static sal_uInt16 pSwPosRanges[] =
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

sal_Bool SvxSwPosSizeTabPage::FillItemSet( SfxItemSet& rSet)
{
    bool bAnchorChanged = false;
    short nAnchor = GetAnchorType(&bAnchorChanged);
    sal_Bool bModified = sal_False;
    if(bAnchorChanged)
    {
        rSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, nAnchor));
        bModified |= sal_True;
    }
    if ( m_aPositionCB.GetState() != m_aPositionCB.GetSavedValue() )
    {
        if( m_aPositionCB.GetState() == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
        else
            rSet.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                m_aPositionCB.GetState() == STATE_CHECK ? sal_True : sal_False ) );
        bModified |= sal_True;
    }

    if ( m_aSizeCB.GetState() != m_aSizeCB.GetSavedValue() )
    {
        if ( m_aSizeCB.GetState() == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rSet.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_aSizeCB.GetState() == STATE_CHECK ? sal_True : sal_False ) );
        bModified |= sal_True;
    }

    const SfxItemSet& rOldSet = GetItemSet();

    if(!m_bPositioningDisabled)
    {
        //on multiple selections the positioning is set via SdrView
        if(m_bIsMultiSelection)
        {
            if( m_aHoriByMF.IsValueModified() || m_aVertByMF.IsValueModified() )
            {
                long nHoriByPos =
                            static_cast<long>(m_aHoriByMF.Denormalize(m_aHoriByMF.GetValue(FUNIT_TWIP)));
                long nVertByPos =
                            static_cast<long>(m_aVertByMF.Denormalize(m_aVertByMF.GetValue(FUNIT_TWIP)));

                // old rectangle with CoreUnit
                m_aRect = m_pSdrView->GetAllMarkedRect();
                m_pSdrView->GetSdrPageView()->LogicToPagePos( m_aRect );

                nHoriByPos += m_aAnchorPos.X();
                nVertByPos += m_aAnchorPos.Y();

                rSet.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_X ), nHoriByPos ) );
                rSet.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_Y ), nVertByPos ) );

                bModified |= sal_True;
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

                sal_uInt16 nMapPos = GetMapPos(m_pHMap, m_aHoriLB);
                short nAlign = GetAlignment(m_pHMap, nMapPos, m_aHoriLB, m_aHoriToLB);
                short nRel = GetRelation(m_pHMap, m_aHoriToLB);
                const long nHoriByPos =
                            static_cast<long>(m_aHoriByMF.Denormalize(m_aHoriByMF.GetValue(FUNIT_TWIP)));
                if (
                    nAlign != rHoriOrient.GetValue() ||
                    nRel != rHoriRelation.GetValue() ||
                    (m_aHoriByMF.IsEnabled() && nHoriByPos != rHoriPosition.GetValue())
                   )
                {
                    rSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_ORIENT, nAlign));
                    rSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_RELATION, nRel));
                    if(m_aHoriByMF.IsEnabled())
                        rSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_HORI_POSITION, nHoriByPos));
                    bModified |= sal_True;
                }
            }
            if(m_aHoriMirrorCB.IsEnabled() && m_aHoriMirrorCB.IsChecked() != m_aHoriMirrorCB.GetSavedValue())
                bModified |= 0 != rSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_HORI_MIRROR, m_aHoriMirrorCB.IsChecked()));

            if ( m_pVMap )
            {
                const SfxInt16Item& rVertOrient =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_ORIENT));
                const SfxInt16Item& rVertRelation =
                        static_cast<const SfxInt16Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_RELATION));
                const SfxInt32Item& rVertPosition =
                        static_cast<const SfxInt32Item&>(rOldSet.Get( SID_ATTR_TRANSFORM_VERT_POSITION));

                sal_uInt16 nMapPos = GetMapPos(m_pVMap, m_aVertLB);
                short nAlign = GetAlignment(m_pVMap, nMapPos, m_aVertLB, m_aVertToLB);
                short nRel = GetRelation(m_pVMap, m_aVertToLB);
                // #i34055# - convert vertical position for
                // as-character anchored objects
                long nVertByPos =
                        static_cast<long>(m_aVertByMF.Denormalize(m_aVertByMF.GetValue(FUNIT_TWIP)));
                if ( GetAnchorType() == TextContentAnchorType_AS_CHARACTER )
                {
                    nVertByPos *= -1;
                }
                if ( nAlign != rVertOrient.GetValue() ||
                     nRel != rVertRelation.GetValue() ||
                     ( m_aVertByMF.IsEnabled() &&
                       nVertByPos != rVertPosition.GetValue() ) )
                {
                    rSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, nAlign));
                    rSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, nRel));
                    if(m_aVertByMF.IsEnabled())
                        rSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, nVertByPos));
                    bModified |= sal_True;
                }
            }

            // #i18732#
            if(m_aFollowCB.IsChecked() != m_aFollowCB.GetSavedValue())
            {
                //Writer internal type - based on SfxBoolItem
                const SfxPoolItem* pItem = GetItem( rOldSet, SID_SW_FOLLOW_TEXT_FLOW);
                if(pItem)
                {
                    SfxBoolItem* pFollow = static_cast<SfxBoolItem*>(pItem->Clone());
                    pFollow->SetValue(m_aFollowCB.IsChecked());
                    bModified |= 0 != rSet.Put(*pFollow);
                    delete pFollow;
                }
            }
        }
    }
    if ( m_aWidthMF.IsValueModified() || m_aHeightMF.IsValueModified() )
    {
        sal_uInt32 nWidth = static_cast<sal_uInt32>(m_aWidthMF.Denormalize(m_aWidthMF.GetValue(FUNIT_TWIP)));
        sal_uInt32 nHeight = static_cast<sal_uInt32>(m_aHeightMF.Denormalize(m_aHeightMF.GetValue(FUNIT_TWIP)));
        rSet.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ),
                        (sal_uInt32) nWidth ) );
        rSet.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ),
                        (sal_uInt32) nHeight ) );
        //this item is required by SdrEditView::SetGeoAttrToMarked()
        rSet.Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), RP_LT ) );

        bModified |= sal_True;
    }

    return bModified;
}

void SvxSwPosSizeTabPage::Reset( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_TRANSFORM_ANCHOR );
    bool bInvalidateAnchor = false;
    short nAnchorType = TextContentAnchorType_AT_PARAGRAPH;
    if(pItem)
    {
        nAnchorType = static_cast<const SfxInt16Item*>(pItem)->GetValue();
        switch(nAnchorType)
        {
            case  TextContentAnchorType_AT_PAGE:        m_aToPageRB.Check();  break;
            case  TextContentAnchorType_AT_PARAGRAPH:   m_aToParaRB.Check();  break;
            case  TextContentAnchorType_AT_CHARACTER:   m_aToCharRB.Check();  break;
            case  TextContentAnchorType_AS_CHARACTER:   m_aAsCharRB.Check();  break;
            case  TextContentAnchorType_AT_FRAME:       m_aToFrameRB.Check(); break;
            default : bInvalidateAnchor = true;
        }
        m_aToPageRB.SaveValue();
        m_aToParaRB.SaveValue();
        m_aToCharRB.SaveValue();
        m_aAsCharRB.SaveValue();
        m_aToFrameRB.SaveValue();
    }
    if(bInvalidateAnchor)
    {
        m_aToPageRB.Enable( sal_False );
        m_aToParaRB.Enable( sal_False );
        m_aToCharRB.Enable( sal_False );
        m_aAsCharRB.Enable( sal_False );
        m_aToFrameRB.Enable( sal_False );
    }

    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_PROTECT_POS );
    if ( pItem )
    {
        sal_Bool bProtected = ( ( const SfxBoolItem* )pItem )->GetValue();
        m_aPositionCB.SetState( bProtected ? STATE_CHECK : STATE_NOCHECK );
        m_aPositionCB.EnableTriState( sal_False );
        m_aSizeCB.Enable( !bProtected );
    }
    else
    {
        m_aPositionCB.SetState( STATE_DONTKNOW );
    }

    m_aPositionCB.SaveValue();

    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_PROTECT_SIZE );

    if ( pItem )
    {
        m_aSizeCB.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                              ? STATE_CHECK : STATE_NOCHECK );
        m_aSizeCB.EnableTriState( sal_False );
    }
    else
        m_aSizeCB.SetState( STATE_DONTKNOW );
    m_aSizeCB.SaveValue();

    pItem = GetItem( rSet, SID_HTML_MODE );
    if(pItem)
        m_nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    m_bHtmlMode = 0 != (m_bHtmlMode & HTMLMODE_ON);

    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_IN_VERTICAL_TEXT );
    if(pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue())
    {
        String sHLabel = m_aHoriFT.GetText();
        m_aHoriFT.SetText(m_aVertFT.GetText());
        m_aVertFT.SetText(sHLabel);
        m_bIsVerticalFrame = true;
    }
    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_IN_RTL_TEXT);
    if(pItem)
        m_bIsInRightToLeft = static_cast<const SfxBoolItem*>(pItem)->GetValue();

    pItem = GetItem( rSet, SID_SW_FOLLOW_TEXT_FLOW);
    if(pItem)
    {
        const bool bFollowTextFlow =
            static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_aFollowCB.Check( bFollowTextFlow );
    }
    m_aFollowCB.SaveValue();

    if(m_bHtmlMode)
    {
        m_aHoriMirrorCB.Show(sal_False);
        m_aKeepRatioCB.Enable(sal_False);
        // #i18732# - hide checkbox in HTML mode
        m_aFollowCB.Show(sal_False);
    }
    else
    {
        // #i18732# correct enable/disable of check box 'Mirror on..'
        m_aHoriMirrorCB.Enable(!m_aAsCharRB.IsChecked() && !m_bIsMultiSelection);

        // #i18732# - enable/disable check box 'Follow text flow'.
        m_aFollowCB.Enable( m_aToParaRB.IsChecked() ||
                                  m_aToCharRB.IsChecked() );
    }

    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_WIDTH );
    sal_Int32 nWidth = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, (sal_uInt32)1 );

    m_aWidthMF.SetValue(m_aWidthMF.Normalize(nWidth), FUNIT_TWIP);

    pItem = GetItem( rSet, SID_ATTR_TRANSFORM_HEIGHT );
    sal_Int32 nHeight = std::max( pItem ? ( static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0, (sal_uInt32)1 );
    m_aHeightMF.SetValue(m_aHeightMF.Normalize(nHeight), FUNIT_TWIP);
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;

    if(!m_bPositioningDisabled)
    {
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_HORI_ORIENT);
        if(pItem)
        {
            short nHoriOrientation = static_cast< const SfxInt16Item*>(pItem)->GetValue();
            m_nOldH = nHoriOrientation;
        }
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_VERT_ORIENT);
        if(pItem)
        {
            short nVertOrientation = static_cast< const SfxInt16Item*>(pItem)->GetValue();
            m_nOldV = nVertOrientation;
        }
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_HORI_RELATION);
        if(pItem)
        {
            m_nOldHRel = static_cast< const SfxInt16Item*>(pItem)->GetValue();
        }

        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_VERT_RELATION);
        if(pItem)
        {
            m_nOldVRel = static_cast< const SfxInt16Item*>(pItem)->GetValue();
        }
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_HORI_MIRROR);
        if(pItem)
            m_aHoriMirrorCB.Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
        m_aHoriMirrorCB.SaveValue();

        sal_Int32 nHoriPos = 0;
        sal_Int32 nVertPos = 0;
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_HORI_POSITION);
        if(pItem)
            nHoriPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        pItem = GetItem( rSet, SID_ATTR_TRANSFORM_VERT_POSITION);
        if(pItem)
            nVertPos = static_cast<const SfxInt32Item*>(pItem)->GetValue();

        InitPos(nAnchorType, m_nOldH, m_nOldHRel, m_nOldV, m_nOldVRel, nHoriPos, nVertPos);

        m_aVertByMF.SaveValue();
        m_aHoriByMF.SaveValue();
        // #i18732#
        m_aFollowCB.SaveValue();

        RangeModifyHdl(&m_aWidthMF);  // initially set maximum values
    }
}

int  SvxSwPosSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );
    return( LEAVE_PAGE );
}

void SvxSwPosSizeTabPage::EnableAnchorTypes(sal_uInt16 nAnchorEnable)
{
    if((nAnchorEnable & SVX_OBJ_AT_FLY))
        m_aToFrameRB.Show();
    if(!(nAnchorEnable & SVX_OBJ_PAGE))
        m_aToPageRB.Enable(sal_False);
}

short SvxSwPosSizeTabPage::GetAnchorType(bool* pbHasChanged)
{
    short nRet = -1;
    RadioButton* pCheckedButton = 0;
    if(m_aToParaRB.IsEnabled())
    {
        if(m_aToPageRB.IsChecked())
        {
            nRet = TextContentAnchorType_AT_PAGE;
            pCheckedButton = &m_aToPageRB;
        }
        else if(m_aToParaRB.IsChecked())
        {
            nRet = TextContentAnchorType_AT_PARAGRAPH;
            pCheckedButton = &m_aToParaRB;
        }
        else if(m_aToCharRB.IsChecked())
        {
            nRet = TextContentAnchorType_AT_CHARACTER;
            pCheckedButton = &m_aToCharRB;
        }
        else if(m_aAsCharRB.IsChecked())
        {
            nRet = TextContentAnchorType_AS_CHARACTER;
            pCheckedButton = &m_aAsCharRB;
        }
        else if(m_aToFrameRB.IsChecked())
        {
            nRet = TextContentAnchorType_AT_FRAME;
            pCheckedButton = &m_aToFrameRB;
        }
    }
    if(pbHasChanged)
    {
         if(pCheckedButton)
             *pbHasChanged = pCheckedButton->IsChecked() != pCheckedButton->GetSavedValue();
         else
             *pbHasChanged = false;
    }
   return nRet;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, RangeModifyHdl)
{
    if(m_bPositioningDisabled)
        return 0;
    SvxSwFrameValidation        aVal;

    aVal.nAnchorType = GetAnchorType();
    aVal.bAutoHeight = false;
    aVal.bAutoWidth = false;
    aVal.bMirror = m_aHoriMirrorCB.IsChecked();
    // #i18732#
    aVal.bFollowTextFlow = m_aFollowCB.IsChecked();

    if ( m_pHMap )
    {
        // horizontal alignment
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, m_aHoriToLB);
        sal_uInt16 nAlign = GetAlignment(m_pHMap, nMapPos, m_aHoriLB, m_aHoriToLB);
        sal_uInt16 nRel = GetRelation(m_pHMap, m_aHoriToLB);

        aVal.nHoriOrient = (short)nAlign;
        aVal.nHRelOrient = (short)nRel;
    }
    else
        aVal.nHoriOrient = HoriOrientation::NONE;

    if ( m_pVMap )
    {
        // vertical alignment
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, m_aVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, m_aVertLB, m_aVertToLB);
        sal_uInt16 nRel = GetRelation(m_pVMap, m_aVertToLB);

        aVal.nVertOrient = (short)nAlign;
        aVal.nVRelOrient = (short)nRel;
    }
    else
        aVal.nVertOrient = VertOrientation::NONE;

    const long nAtHorzPosVal =
                    static_cast<long>(m_aHoriByMF.Denormalize(m_aHoriByMF.GetValue(FUNIT_TWIP)));
    const long nAtVertPosVal =
                    static_cast<long>(m_aVertByMF.Denormalize(m_aVertByMF.GetValue(FUNIT_TWIP)));

    aVal.nHPos = nAtHorzPosVal;
    aVal.nVPos = nAtVertPosVal;

    sal_Int32 nWidth = static_cast<sal_uInt32>(m_aWidthMF. Denormalize(m_aWidthMF.GetValue(FUNIT_TWIP)));
    sal_Int32 nHeight = static_cast<sal_uInt32>(m_aHeightMF.Denormalize(m_aHeightMF.GetValue(FUNIT_TWIP)));
    aVal.nWidth  = nWidth;
    aVal.nHeight = nHeight;

    if(m_aValidateLink.IsSet())
        m_aValidateLink.Call(&aVal);

    nWidth = aVal.nWidth;
    nHeight = aVal.nHeight;

    // minimum width also for style
    m_aHeightMF.SetMin(m_aHeightMF.Normalize(aVal.nMinHeight), FUNIT_TWIP);
    m_aWidthMF. SetMin(m_aWidthMF.Normalize(aVal.nMinWidth), FUNIT_TWIP);

    sal_Int32 nMaxWidth(aVal.nMaxWidth);
    sal_Int32 nMaxHeight(aVal.nMaxHeight);

    sal_Int64 nTmp = m_aHeightMF.Normalize(nMaxHeight);
    m_aHeightMF.SetMax(nTmp, FUNIT_TWIP);

    nTmp = m_aWidthMF.Normalize(nMaxWidth);
    m_aWidthMF.SetMax(nTmp, FUNIT_TWIP);

    m_aHoriByMF.SetMin(m_aHoriByMF.Normalize(aVal.nMinHPos), FUNIT_TWIP);
    m_aHoriByMF.SetMax(m_aHoriByMF.Normalize(aVal.nMaxHPos), FUNIT_TWIP);
    if ( aVal.nHPos != nAtHorzPosVal )
        m_aHoriByMF.SetValue(m_aHoriByMF.Normalize(aVal.nHPos), FUNIT_TWIP);

    m_aVertByMF.SetMin(m_aVertByMF.Normalize(aVal.nMinVPos ), FUNIT_TWIP);
    m_aVertByMF.SetMax(m_aVertByMF.Normalize(aVal.nMaxVPos), FUNIT_TWIP);
    if ( aVal.nVPos != nAtVertPosVal )
        m_aVertByMF.SetValue(m_aVertByMF.Normalize(aVal.nVPos), FUNIT_TWIP);

    return 0;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, AnchorTypeHdl)
{
    m_aHoriMirrorCB.Enable(!m_aAsCharRB.IsChecked() && !m_bIsMultiSelection);

    // #i18732# - enable check box 'Follow text flow' for anchor
    // type to-paragraph' and to-character
    m_aFollowCB.Enable( m_aToParaRB.IsChecked() || m_aToCharRB.IsChecked() );

    short nId = GetAnchorType();

    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);
    RangeModifyHdl(0);

    if(m_bHtmlMode)
    {
        PosHdl(&m_aHoriLB);
        PosHdl(&m_aVertLB);
    }
    return 0;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, MirrorHdl)
{
    short nId = GetAnchorType();
    InitPos( nId, USHRT_MAX, 0, USHRT_MAX, 0, LONG_MAX, LONG_MAX);

    return 0;
}

IMPL_LINK( SvxSwPosSizeTabPage, RelHdl, ListBox *, pLB )
{
    sal_Bool bHori = pLB == &m_aHoriToLB;

    UpdateExample();

    if (bHori)
        m_bAtHoriPosModified = sal_True;
    else
        m_bAtVertPosModified = sal_True;

    if(m_bHtmlMode  && TextContentAnchorType_AT_CHARACTER == GetAnchorType()) // again special treatment
    {
        if(bHori)
        {
            sal_uInt16 nRel = GetRelation(m_pHMap, m_aHoriToLB);
            if(RelOrientation::PRINT_AREA == nRel && 0 == m_aVertLB.GetSelectEntryPos())
            {
                m_aVertLB.SelectEntryPos(1);
            }
            else if(RelOrientation::CHAR == nRel && 1 == m_aVertLB.GetSelectEntryPos())
            {
                m_aVertLB.SelectEntryPos(0);
            }
        }
    }
    if (pLB)    // only if the hanlder has been called by a change of the controller
        RangeModifyHdl(0);

    return 0;

}

IMPL_LINK( SvxSwPosSizeTabPage, PosHdl, ListBox *, pLB )
{
    sal_Bool bHori = pLB == &m_aHoriLB;
    ListBox *pRelLB = bHori ? &m_aHoriToLB : &m_aVertToLB;
    FixedText *pRelFT = bHori ? &m_aHoriToFT : &m_aVertToFT;
    FrmMap *pMap = bHori ? m_pHMap : m_pVMap;


    sal_uInt16 nMapPos = GetMapPos(pMap, *pLB);
    sal_uInt16 nAlign = GetAlignment(pMap, nMapPos, *pLB, *pRelLB);

    if (bHori)
    {
        sal_Bool bEnable = HoriOrientation::NONE == nAlign;
        m_aHoriByMF.Enable( bEnable );
        m_aHoriByFT.Enable( bEnable );
    }
    else
    {
        sal_Bool bEnable = VertOrientation::NONE == nAlign;
        m_aVertByMF.Enable( bEnable );
        m_aVertByFT.Enable( bEnable );
    }

    if (pLB)    // only if the hanlder has been called by a change of the controller
        RangeModifyHdl( 0 );

    short nRel = 0;
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
        m_bAtHoriPosModified = sal_True;
    else
        m_bAtVertPosModified = sal_True;

    // special treatment for HTML-Mode with horz-vert-dependencies
    if(m_bHtmlMode && TextContentAnchorType_AT_CHARACTER == GetAnchorType())
    {
        sal_Bool bSet = sal_False;
        if(bHori)
        {
            // on the right only below is allowed - from the left only at the top
            // from the left at the character -> below
            if((HoriOrientation::LEFT == nAlign || HoriOrientation::RIGHT == nAlign) &&
                    0 == m_aVertLB.GetSelectEntryPos())
            {
                if(RelOrientation::FRAME == nRel)
                    m_aVertLB.SelectEntryPos(1);
                else
                    m_aVertLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            else if(HoriOrientation::LEFT == nAlign && 1 == m_aVertLB.GetSelectEntryPos())
            {
                m_aVertLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            else if(HoriOrientation::NONE == nAlign && 1 == m_aVertLB.GetSelectEntryPos())
            {
                m_aVertLB.SelectEntryPos(0);
                bSet = sal_True;
            }
            if(bSet)
                PosHdl(&m_aVertLB);
        }
        else
        {
            if(VertOrientation::TOP == nAlign)
            {
                if(1 == m_aHoriLB.GetSelectEntryPos())
                {
                    m_aHoriLB.SelectEntryPos(0);
                    bSet = sal_True;
                }
                m_aHoriToLB.SelectEntryPos(1);
            }
            else if(VertOrientation::CHAR_BOTTOM == nAlign)
            {
                if(2 == m_aHoriLB.GetSelectEntryPos())
                {
                    m_aHoriLB.SelectEntryPos(0);
                    bSet = sal_True;
                }
                m_aHoriToLB.SelectEntryPos(0) ;
            }
            if(bSet)
                PosHdl(&m_aHoriLB);
        }

    }
    return 0;
}

IMPL_LINK( SvxSwPosSizeTabPage, ModifyHdl, Edit *, pEdit )
{
    sal_Int64 nWidth = m_aWidthMF.Denormalize(m_aWidthMF.GetValue(FUNIT_TWIP));
    sal_Int64 nHeight = m_aHeightMF.Denormalize(m_aHeightMF.GetValue(FUNIT_TWIP));
    if ( m_aKeepRatioCB.IsChecked() )
    {
        if ( pEdit == &m_aWidthMF )
        {
            nHeight = sal_Int64((double)nWidth / m_fWidthHeightRatio);
            m_aHeightMF.SetValue(m_aHeightMF.Normalize(nHeight), FUNIT_TWIP);
        }
        else if(pEdit == &m_aHeightMF)
        {
            nWidth = sal_Int64((double)nHeight * m_fWidthHeightRatio);
            m_aWidthMF.SetValue(m_aWidthMF.Normalize(nWidth), FUNIT_TWIP);
        }
    }
    m_fWidthHeightRatio = nHeight ? double(nWidth) / double(nHeight) : 1.0;
    UpdateExample();
    return 0;
}

IMPL_LINK_NOARG(SvxSwPosSizeTabPage, ProtectHdl)
{
    m_aSizeCB.Enable(m_aPositionCB.IsEnabled() && !m_aPositionCB.IsChecked());
    return 0;
}

short SvxSwPosSizeTabPage::GetRelation(FrmMap *, ListBox &rRelationLB)
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

short SvxSwPosSizeTabPage::GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos, ListBox &/*rAlignLB*/, ListBox &rRelationLB)
{
    short nAlign = 0;

    // #i22341# - special handling also for map <aVCharMap>,
    // because it contains ambigous items for alignment
    if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap ||
            pMap == aVCharMap )
    {
        if (rRelationLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            sal_uLong  nRel = ((RelationMap *)rRelationLB.GetEntryData(rRelationLB.GetSelectEntryPos()))->nLBRelation;
            std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (std::size_t i = 0; i < nMapCount; i++)
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

sal_uInt16 SvxSwPosSizeTabPage::GetMapPos(FrmMap *pMap, ListBox &rAlignLB)
{
    sal_uInt16 nMapPos = 0;
    sal_uInt16 nLBSelPos = rAlignLB.GetSelectEntryPos();

    if (nLBSelPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);
            String sSelEntry(rAlignLB.GetSelectEntry());

            for (std::size_t i = 0; i < nMapCount; i++)
            {
                SvxSwFramePosString::StringId eResId = pMap[i].eStrId;

                String sEntry = m_aFramePosString.GetString(eResId);

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

void SvxSwPosSizeTabPage::InitPos(short nAnchor,
                                sal_uInt16 nH,
                                sal_uInt16 nHRel,
                                sal_uInt16 nV,
                                sal_uInt16 nVRel,
                                long   nX,
                                long   nY)
{
    sal_uInt16 nPos = m_aVertLB.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pVMap )
    {
        m_nOldV    = m_pVMap[nPos].nAlign;
        nPos = m_aVertToLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldVRel = ((RelationMap *)m_aVertToLB.GetEntryData(nPos))->nRelation;
    }

    nPos = m_aHoriLB.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND && m_pHMap )
    {
        m_nOldH    = m_pHMap[nPos].nAlign;

        nPos = m_aHoriToLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            m_nOldHRel = ((RelationMap *)m_aHoriToLB.GetEntryData(nPos))->nRelation;
    }

    sal_Bool bEnable = sal_True;
    if( m_bIsMultiSelection )
    {
        m_pVMap = aVMultiSelectionMap;
        m_pHMap = aHMultiSelectionMap;
    }
    else if( nAnchor == TextContentAnchorType_AT_PAGE )
    {
        m_pVMap = m_bHtmlMode ? aVPageHtmlMap : aVPageMap;
        m_pHMap = m_bHtmlMode ? aHPageHtmlMap : aHPageMap;
    }
    else if ( nAnchor == TextContentAnchorType_AT_FRAME )
    {
        // #i18732# - own vertical alignment map for to frame
        // anchored objects.
        m_pVMap = m_bHtmlMode ? aVFlyHtmlMap : aVFrameMap;
        m_pHMap = m_bHtmlMode ? aHFlyHtmlMap : aHFrameMap;
    }
    else if ( nAnchor == TextContentAnchorType_AT_PARAGRAPH )
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
    else if ( nAnchor == TextContentAnchorType_AT_CHARACTER )
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
    else if ( nAnchor == TextContentAnchorType_AS_CHARACTER )
    {
        m_pVMap = m_bHtmlMode ? aVAsCharHtmlMap     : aVAsCharMap;
        m_pHMap = 0;
        bEnable = sal_False;
    }
    m_aHoriLB.Enable( bEnable );
    m_aHoriFT.Enable( bEnable );

    // select current Pos
    // horizontal
    if ( nH == USHRT_MAX )
    {
        nH    = m_nOldH;
        nHRel = m_nOldHRel;
    }
    // #i22341# - pass <nHRel> as 3rd parameter to method <FillPosLB>
    sal_uInt16 nMapPos = FillPosLB(m_pHMap, nH, nHRel, m_aHoriLB);
    FillRelLB(m_pHMap, nMapPos, nH, nHRel, m_aHoriToLB, m_aHoriToFT);

    // vertical
    if ( nV == USHRT_MAX )
    {
        nV    = m_nOldV;
        nVRel = m_nOldVRel;
    }
    // #i22341# - pass <nVRel> as 3rd parameter to method <FillPosLB>
    nMapPos = FillPosLB(m_pVMap, nV, nVRel, m_aVertLB);
    FillRelLB(m_pVMap, nMapPos, nV, nVRel, m_aVertToLB, m_aVertToFT);

    // Edits init
    bEnable = nH == HoriOrientation::NONE &&
            nAnchor != TextContentAnchorType_AS_CHARACTER;//#61359# why not in formats&& !bFormat;
    if (!bEnable)
    {
        m_aHoriByMF.SetValue( 0, FUNIT_TWIP );
        if (nX != LONG_MAX && m_bHtmlMode)
            m_aHoriByMF.SetModifyFlag();
    }
    else if(m_bIsMultiSelection)
    {
         m_aHoriByMF.SetValue( m_aHoriByMF.Normalize(m_aRect.Left()), FUNIT_TWIP );
    }
    else
    {
        if (nX != LONG_MAX)
            m_aHoriByMF.SetValue( m_aHoriByMF.Normalize(nX), FUNIT_TWIP );
    }
    m_aHoriByFT.Enable( bEnable );
    m_aHoriByMF.Enable( bEnable );

    bEnable = nV == VertOrientation::NONE;
    if ( !bEnable )
    {
        m_aVertByMF.SetValue( 0, FUNIT_TWIP );
        if(nY != LONG_MAX && m_bHtmlMode)
            m_aVertByMF.SetModifyFlag();
    }
    else if(m_bIsMultiSelection)
    {
         m_aVertByMF.SetValue( m_aVertByMF.Normalize(m_aRect.Top()), FUNIT_TWIP );
    }
    else
    {
        if ( nAnchor == TextContentAnchorType_AS_CHARACTER )
        {
            if ( nY == LONG_MAX )
                nY = 0;
            else
                nY *= -1;
        }
        if ( nY != LONG_MAX )
            m_aVertByMF.SetValue( m_aVertByMF.Normalize(nY), FUNIT_TWIP );
    }
    m_aVertByFT.Enable( bEnable );
    m_aVertByMF.Enable( bEnable );
    UpdateExample();
}

void SvxSwPosSizeTabPage::UpdateExample()
{
    sal_uInt16 nPos = m_aHoriLB.GetSelectEntryPos();
    if ( m_pHMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(m_pHMap, m_aHoriLB);
        short nAlign = GetAlignment(m_pHMap, nMapPos, m_aHoriLB, m_aHoriToLB);
        short nRel = GetRelation(m_pHMap, m_aHoriToLB);

        m_aExampleWN.SetHAlign(nAlign);
        m_aExampleWN.SetHoriRel(nRel);
    }

    nPos = m_aVertLB.GetSelectEntryPos();
    if ( m_pVMap && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nMapPos = GetMapPos(m_pVMap, m_aVertLB);
        sal_uInt16 nAlign = GetAlignment(m_pVMap, nMapPos, m_aVertLB, m_aVertToLB);
        sal_uInt16 nRel = GetRelation(m_pVMap, m_aVertToLB);

        m_aExampleWN.SetVAlign(nAlign);
        m_aExampleWN.SetVertRel(nRel);
    }

    // Size
    long nXPos = static_cast<long>(m_aHoriByMF.Denormalize(m_aHoriByMF.GetValue(FUNIT_TWIP)));
    long nYPos = static_cast<long>(m_aVertByMF.Denormalize(m_aVertByMF.GetValue(FUNIT_TWIP)));
    m_aExampleWN.SetRelPos(Point(nXPos, nYPos));

    m_aExampleWN.SetAnchor( GetAnchorType() );
    m_aExampleWN.Invalidate();
}

sal_uLong SvxSwPosSizeTabPage::FillRelLB(FrmMap *pMap, sal_uInt16 nMapPos, sal_uInt16 nAlign,
        sal_uInt16 nRel, ListBox &rLB, FixedText &rFT)
{
    String sSelEntry;
    sal_uLong  nLBRelations = 0;
    std::size_t nMapCount = ::lcl_GetFrmMapCount(pMap);

    rLB.Clear();

    if (nMapPos < nMapCount)
    {
        if (pMap == aVAsCharHtmlMap || pMap == aVAsCharMap)
        {
            String sOldEntry(rLB.GetSelectEntry());
            sal_uInt16 nRelCount = SAL_N_ELEMENTS(aAsCharRelationMap);
            SvxSwFramePosString::StringId eStrId = pMap[nMapPos].eStrId;

            for (std::size_t _nMapPos = 0; _nMapPos < nMapCount; _nMapPos++)
            {
                if (pMap[_nMapPos].eStrId == eStrId)
                {
                    nLBRelations = pMap[_nMapPos].nLBRelations;
                    for (sal_uInt16 nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (nLBRelations & aAsCharRelationMap[nRelPos].nLBRelation)
                        {
                            SvxSwFramePosString::StringId sStrId1 = aAsCharRelationMap[nRelPos].eStrId;

                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            String sEntry = m_aFramePosString.GetString(sStrId1);
                            sal_uInt16 nPos = rLB.InsertEntry(sEntry);
                            rLB.SetEntryData(nPos, &aAsCharRelationMap[nRelPos]);
                            if (pMap[_nMapPos].nAlign == nAlign)
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
                    for (sal_uInt16 i = 0; i < rLB.GetEntryCount(); i++)
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
            sal_uInt16 nRelCount = SAL_N_ELEMENTS(aRelationMap);

            // #i22341# - special handling for map <aVCharMap>,
            // because its ambigous in its <eStrId>/<eMirrorStrId>.
            if ( pMap == aVCharMap )
            {
                nLBRelations = ::lcl_GetLBRelationsForStrID( pMap,
                                             ( m_aHoriMirrorCB.IsChecked()
                                               ? pMap[nMapPos].eMirrorStrId
                                               : pMap[nMapPos].eStrId ),
                                             m_aHoriMirrorCB.IsChecked() );
            }
            else
            {
                nLBRelations = pMap[nMapPos].nLBRelations;
            }

            for (sal_uLong nBit = 1; nBit < 0x80000000; nBit <<= 1)
            {
                if (nLBRelations & nBit)
                {
                    for (sal_uInt16 nRelPos = 0; nRelPos < nRelCount; nRelPos++)
                    {
                        if (aRelationMap[nRelPos].nLBRelation == nBit)
                        {
                            SvxSwFramePosString::StringId sStrId1 = m_aHoriMirrorCB.IsChecked() ? aRelationMap[nRelPos].eMirrorStrId : aRelationMap[nRelPos].eStrId;
                            sStrId1 = lcl_ChangeResIdToVerticalOrRTL(sStrId1, m_bIsVerticalFrame, m_bIsInRightToLeft);
                            String sEntry = m_aFramePosString.GetString(sStrId1);
                            sal_uInt16 nPos = rLB.InsertEntry(sEntry);
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
                // Probably anchor change. So look for a similar relation.
                switch (nRel)
                {
                    case RelOrientation::FRAME:             nRel = RelOrientation::PAGE_FRAME;    break;
                    case RelOrientation::PRINT_AREA:        nRel = RelOrientation::PAGE_PRINT_AREA;  break;
                    case RelOrientation::PAGE_LEFT:       nRel = RelOrientation::FRAME_LEFT;    break;
                    case RelOrientation::PAGE_RIGHT:      nRel = RelOrientation::FRAME_RIGHT;   break;
                    case RelOrientation::FRAME_LEFT:      nRel = RelOrientation::PAGE_LEFT;     break;
                    case RelOrientation::FRAME_RIGHT:     nRel = RelOrientation::PAGE_RIGHT;    break;
                    case RelOrientation::PAGE_FRAME:      nRel = RelOrientation::FRAME;           break;
                    case RelOrientation::PAGE_PRINT_AREA: nRel = RelOrientation::PRINT_AREA;         break;

                    default:
                        if (rLB.GetEntryCount())
                        {
                            RelationMap *pEntry = (RelationMap *)rLB.GetEntryData(rLB.GetEntryCount() - 1);
                            nRel = pEntry->nRelation;
                        }
                        break;
                }

                for (sal_uInt16 i = 0; i < rLB.GetEntryCount(); i++)
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
    rFT.Enable(rLB.GetEntryCount() != 0);

    RelHdl(&rLB);

    return nLBRelations;
}

sal_uInt16 SvxSwPosSizeTabPage::FillPosLB(FrmMap *_pMap,
                                      sal_uInt16 _nAlign,
                                      const sal_uInt16 _nRel,
                                      ListBox &_rLB)
{
    String sSelEntry, sOldEntry;
    sOldEntry = _rLB.GetSelectEntry();

    _rLB.Clear();

    // #i22341# - determine all possible listbox relations for
    // given relation for map <aVCharMap>
    const sal_uLong nLBRelations = (_pMap != aVCharMap)
                               ? 0L
                               : ::lcl_GetLBRelationsForRelations( _nRel );

    // fill listbox
    std::size_t nCount = ::lcl_GetFrmMapCount(_pMap);
    for (std::size_t i = 0; _pMap && i < nCount; ++i)
    {
//      #61359# why not from the left/from inside or from the top?
//      if (!bFormat || (pMap[i].eStrId != SwFPos::FROMLEFT && pMap[i].eStrId != SwFPos::FROMTOP))
        {
            SvxSwFramePosString::StringId eStrId = m_aHoriMirrorCB.IsChecked() ? _pMap[i].eMirrorStrId : _pMap[i].eStrId;
            eStrId = lcl_ChangeResIdToVerticalOrRTL(eStrId, m_bIsVerticalFrame, m_bIsInRightToLeft);
            String sEntry(m_aFramePosString.GetString(eStrId));
            if (_rLB.GetEntryPos(sEntry) == LISTBOX_ENTRY_NOTFOUND)
            {
                // don't insert duplicate entries at character wrapped borders
                _rLB.InsertEntry(sEntry);
            }
            // #i22341# - add condition to handle map <aVCharMap>
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
    if( rMarkList.GetMarkCount() >= 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        m_aAnchorPos = pObj->GetAnchorPos();

        if( m_aAnchorPos != Point(0,0) ) // -> Writer
        {
            for( sal_uInt16 i = 1; i < rMarkList.GetMarkCount(); i++ )
            {
                pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                if( m_aAnchorPos != pObj->GetAnchorPos() )
                {
                    // different anchor positions -> disable positioning
                    m_aPositionFL.Enable(sal_False);
                    m_aHoriFT.Enable(sal_False);
                    m_aHoriLB.Enable(sal_False);
                    m_aHoriByFT.Enable(sal_False);
                    m_aHoriByMF.Enable(sal_False);
                    m_aHoriToFT.Enable(sal_False);
                    m_aHoriToLB.Enable(sal_False);
                    m_aHoriMirrorCB.Enable(sal_False);
                    m_aVertFT.Enable(sal_False);
                    m_aVertLB.Enable(sal_False);
                    m_aVertByFT.Enable(sal_False);
                    m_aVertByMF.Enable(sal_False);
                    m_aVertToFT.Enable(sal_False);
                    m_aVertToLB.Enable(sal_False);
                    m_aFollowCB.Enable(sal_False);
                    m_aHoriByMF.SetText(String());
                    m_aVertByMF.SetText(String());

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
        if( ( pObj->GetObjInventor() == SdrInventor ) &&
            ( eKind==OBJ_TEXT || eKind==OBJ_TITLETEXT || eKind==OBJ_OUTLINETEXT) &&
            pObj->HasText() )
        {
            OSL_FAIL("AutoWidth/AutoHeight should be enabled");
        }
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
