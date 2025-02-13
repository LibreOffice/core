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

#include <ElementsDockingWindow.hxx>

#include <starmath.hrc>
#include <strings.hrc>
#include <smmod.hxx>
#include <cfgitem.hxx>
#include <parse.hxx>
#include <utility>
#include <view.hxx>
#include <visitors.hxx>
#include <document.hxx>
#include <strings.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/virdev.hxx>

#include <unordered_map>

namespace
{
// element, element help, element visual, element visual's translatable
typedef std::tuple<std::u16string_view, TranslateId, std::u16string_view, TranslateId> SmElementDescr;

// SmParser 5 elements

const SmElementDescr s_a5UnaryBinaryOperatorsList[] =
{
    {RID_PLUSX, RID_PLUSX_HELP, {}, {}},
    {RID_MINUSX, RID_MINUSX_HELP, {}, {}},
    {RID_PLUSMINUSX, RID_PLUSMINUSX_HELP, {}, {}},
    {RID_MINUSPLUSX, RID_MINUSPLUSX_HELP, {}, {}},
    {},
    {RID_XPLUSY, RID_XPLUSY_HELP, {}, {}},
    {RID_XMINUSY, RID_XMINUSY_HELP, {}, {}},
    {RID_XCDOTY, RID_XCDOTY_HELP, {}, {}},
    {RID_XTIMESY, RID_XTIMESY_HELP, {}, {}},
    {RID_XSYMTIMESY, RID_XSYMTIMESY_HELP, {}, {}},
    {RID_XOVERY, RID_XOVERY_HELP, {}, {}},
    {RID_FRACXY, RID_FRACXY_HELP, {}, {}},
    {RID_XDIVY, RID_XDIVY_HELP, {}, {}},
    {RID_XSYMDIVIDEY, RID_XSYMDIVIDEY_HELP, {}, {}},
    {RID_XOPLUSY, RID_XOPLUSY_HELP, {}, {}},
    {RID_XOMINUSY, RID_XOMINUSY_HELP, {}, {}},
    {RID_XODOTY, RID_XODOTY_HELP, {}, {}},
    {RID_XOTIMESY, RID_XOTIMESY_HELP, {}, {}},
    {RID_XODIVIDEY, RID_XODIVIDEY_HELP, {}, {}},
    {RID_XCIRCY, RID_XCIRCY_HELP, {}, {}},
    {RID_XWIDESLASHY, RID_XWIDESLASHY_HELP, {}, {}},
    {RID_XWIDEBSLASHY, RID_XWIDEBSLASHY_HELP, {}, {}},
    {},
    {RID_NEGX, RID_NEGX_HELP, {}, {}},
    {RID_XANDY, RID_XANDY_HELP, {}, {}},
    {RID_XORY, RID_XORY_HELP, {}, {}}
};

const SmElementDescr s_a5RelationsList[] =
{
    {RID_XEQY, RID_XEQY_HELP, {}, {}},
    {RID_XNEQY, RID_XNEQY_HELP, {}, {}},
    {RID_XLTY, RID_XLTY_HELP, {}, {}},
    {RID_XLEY, RID_XLEY_HELP, {}, {}},
    {RID_XLESLANTY, RID_XLESLANTY_HELP, {}, {}},
    {RID_XGTY, RID_XGTY_HELP, {}, {}},
    {RID_XGEY, RID_XGEY_HELP, {}, {}},
    {RID_XGESLANTY, RID_XGESLANTY_HELP, {}, {}},
    {RID_XLLY, RID_XLLY_HELP, {}, {}},
    {RID_XGGY, RID_XGGY_HELP, {}, {}},
    {},
    {RID_XAPPROXY, RID_XAPPROXY_HELP, {}, {}},
    {RID_XSIMY, RID_XSIMY_HELP, {}, {}},
    {RID_XSIMEQY, RID_XSIMEQY_HELP, {}, {}},
    {RID_XEQUIVY, RID_XEQUIVY_HELP, {}, {}},
    {RID_XPROPY, RID_XPROPY_HELP, {}, {}},
    {RID_XPARALLELY, RID_XPARALLELY_HELP, {}, {}},
    {RID_XORTHOY, RID_XORTHOY_HELP, {}, {}},
    {RID_XDIVIDESY, RID_XDIVIDESY_HELP, {}, {}},
    {RID_XNDIVIDESY, RID_XNDIVIDESY_HELP, {}, {}},
    {RID_XTOWARDY, RID_XTOWARDY_HELP, {}, {}},
    {RID_XTRANSLY, RID_XTRANSLY_HELP, {}, {}},
    {RID_XTRANSRY, RID_XTRANSRY_HELP, {}, {}},
    {RID_XDEFY, RID_XDEFY_HELP, {}, {}},
    {},
    {RID_DLARROW, RID_DLARROW_HELP, {}, {}},
    {RID_DLRARROW, RID_DLRARROW_HELP, {}, {}},
    {RID_DRARROW, RID_DRARROW_HELP, {}, {}},
    {},
    {RID_XPRECEDESY, RID_XPRECEDESY_HELP, {}, {}},
    {RID_XSUCCEEDSY, RID_XSUCCEEDSY_HELP, {}, {}},
    {RID_XPRECEDESEQUALY, RID_XPRECEDESEQUALY_HELP, {}, {}},
    {RID_XSUCCEEDSEQUALY, RID_XSUCCEEDSEQUALY_HELP, {}, {}},
    {RID_XPRECEDESEQUIVY, RID_XPRECEDESEQUIVY_HELP, {}, {}},
    {RID_XSUCCEEDSEQUIVY, RID_XSUCCEEDSEQUIVY_HELP, {}, {}},
    {RID_XNOTPRECEDESY, RID_XNOTPRECEDESY_HELP, {}, {}},
    {RID_XNOTSUCCEEDSY, RID_XNOTSUCCEEDSY_HELP, {}, {}},
};

const SmElementDescr s_a5SetOperationsList[] =
{
    {RID_XINY, RID_XINY_HELP, {}, {}},
    {RID_XNOTINY, RID_XNOTINY_HELP, {}, {}},
    {RID_XOWNSY, RID_XOWNSY_HELP, {}, {}},
    {},
    {RID_XINTERSECTIONY, RID_XINTERSECTIONY_HELP, {}, {}},
    {RID_XUNIONY, RID_XUNIONY_HELP, {}, {}},
    {RID_XSETMINUSY, RID_XSETMINUSY_HELP, {}, {}},
    {RID_XSETQUOTIENTY, RID_XSETQUOTIENTY_HELP, {}, {}},
    {RID_XSUBSETY, RID_XSUBSETY_HELP, {}, {}},
    {RID_XSUBSETEQY, RID_XSUBSETEQY_HELP, {}, {}},
    {RID_XSUPSETY, RID_XSUPSETY_HELP, {}, {}},
    {RID_XSUPSETEQY, RID_XSUPSETEQY_HELP, {}, {}},
    {RID_XNSUBSETY, RID_XNSUBSETY_HELP, {}, {}},
    {RID_XNSUBSETEQY, RID_XNSUBSETEQY_HELP, {}, {}},
    {RID_XNSUPSETY, RID_XNSUPSETY_HELP, {}, {}},
    {RID_XNSUPSETEQY, RID_XNSUPSETEQY_HELP, {}, {}},
    {},
    {RID_EMPTYSET, RID_EMPTYSET_HELP, {}, {}},
    {RID_ALEPH, RID_ALEPH_HELP, {}, {}},
    {RID_SETN, RID_SETN_HELP, {}, {}},
    {RID_SETZ, RID_SETZ_HELP, {}, {}},
    {RID_SETQ, RID_SETQ_HELP, {}, {}},
    {RID_SETR, RID_SETR_HELP, {}, {}},
    {RID_SETC, RID_SETC_HELP, {}, {}}
};

const SmElementDescr s_a5FunctionsList[] =
{
    {RID_ABSX, RID_ABSX_HELP, {}, {}},
    {RID_FACTX, RID_FACTX_HELP, {}, {}},
    {RID_SQRTX, RID_SQRTX_HELP, {}, {}},
    {RID_NROOTXY, RID_NROOTXY_HELP, {}, {}},
    {RID_RSUPX, RID_RSUPX_HELP, {}, {}},
    {RID_EX, RID_EX_HELP, {}, {}},
    {RID_LNX, RID_LNX_HELP, {}, {}},
    {RID_EXPX, RID_EXPX_HELP, {}, {}},
    {RID_LOGX, RID_LOGX_HELP, {}, {}},
    {RID_ARALOGX, RID_SINX_HELP, {}, {}},
    {},
    {RID_SINX, RID_SINX_HELP, {}, {}},
    {RID_COSX, RID_COSX_HELP, {}, {}},
    {RID_TANX, RID_TANX_HELP, {}, {}},
    {RID_COTX, RID_COTX_HELP, {}, {}},
    {RID_SINHX, RID_SINHX_HELP, {}, {}},
    {RID_COSHX, RID_COSHX_HELP, {}, {}},
    {RID_TANHX, RID_TANHX_HELP, {}, {}},
    {RID_COTHX, RID_COTHX_HELP, {}, {}},
    {},
    {RID_ARASINX, RID_SINX_HELP, {}, {}},
    {RID_ARACOSX, RID_COSX_HELP, {}, {}},
    {RID_ARATANX, RID_TANX_HELP, {}, {}},
    {RID_ARACOTX, RID_COTX_HELP, {}, {}},
    {RID_ARASECX, RID_COTX_HELP, {}, {}},
    {RID_ARACSCX, RID_COTX_HELP, {}, {}},
    {RID_ARASINHX, RID_SINHX_HELP, {}, {}},
    {RID_ARACOSHX, RID_COSHX_HELP, {}, {}},
    {RID_ARATANHX, RID_TANHX_HELP, {}, {}},
    {RID_ARACOTHX, RID_COTHX_HELP, {}, {}},
    {RID_ARASECHX, RID_COTX_HELP, {}, {}},
    {RID_ARACSCHX, RID_COTX_HELP, {}, {}},
    {},
    {RID_ARASIN2X, RID_SINX_HELP, {}, {}},
    {RID_ARACOS2X, RID_COSX_HELP, {}, {}},
    {RID_ARATAN2X, RID_TANX_HELP, {}, {}},
    {RID_ARACOT2X, RID_COTX_HELP, {}, {}},
    {RID_ARASEC2X, RID_COTX_HELP, {}, {}},
    {RID_ARACSC2X, RID_COTX_HELP, {}, {}},
    {RID_ARASINH2X, RID_SINHX_HELP, {}, {}},
    {RID_ARACOSH2X, RID_COSHX_HELP, {}, {}},
    {RID_ARATANH2X, RID_TANHX_HELP, {}, {}},
    {RID_ARACOTH2X, RID_COTHX_HELP, {}, {}},
    {RID_ARASECH2X, RID_COTX_HELP, {}, {}},
    {RID_ARACSCH2X, RID_COTX_HELP, {}, {}},
    {},
    {RID_ARCSINX, RID_ARCSINX_HELP, {}, {}},
    {RID_ARCCOSX, RID_ARCCOSX_HELP, {}, {}},
    {RID_ARCTANX, RID_ARCTANX_HELP, {}, {}},
    {RID_ARCCOTX, RID_ARCCOTX_HELP, {}, {}},
    {RID_ARSINHX, RID_ARSINHX_HELP, {}, {}},
    {RID_ARCOSHX, RID_ARCOSHX_HELP, {}, {}},
    {RID_ARTANHX, RID_ARTANHX_HELP, {}, {}},
    {RID_ARCOTHX, RID_ARCOTHX_HELP, {}, {}},
    {},
    {RID_FUNCX, RID_FUNCX_HELP, {}, {}},
};

const SmElementDescr s_a5OperatorsList[] =
{
    {RID_LIMX, RID_LIMX_HELP, {}, {}},
    {RID_LIM_FROMX, RID_LIM_FROMX_HELP, {}, {}},
    {RID_LIM_TOX, RID_LIM_TOX_HELP, {}, {}},
    {RID_LIM_FROMTOX, RID_LIM_FROMTOX_HELP, {}, {}},
    {},
    {RID_LIMINFX, RID_LIMINFX_HELP, {}, {}},
    {RID_LIMINF_FROMX, RID_LIMINF_FROMX_HELP, {}, {}},
    {RID_LIMINF_TOX, RID_LIMINF_TOX_HELP, {}, {}},
    {RID_LIMINF_FROMTOX, RID_LIMINF_FROMTOX_HELP, {}, {}},
    {},
    {RID_LIMSUPX, RID_LIMSUPX_HELP, {}, {}},
    {RID_LIMSUP_FROMX, RID_LIMSUP_FROMX_HELP, {}, {}},
    {RID_LIMSUP_TOX, RID_LIMSUP_TOX_HELP, {}, {}},
    {RID_LIMSUP_FROMTOX, RID_LIMSUP_FROMTOX_HELP, {}, {}},
    {},
    {RID_HADDX, RID_HADDX_HELP, {}, {}},
    {RID_HADD_FROMX, RID_HADD_FROMX_HELP, {}, {}},
    {RID_HADD_TOX, RID_HADD_TOX_HELP, {}, {}},
    {RID_HADD_FROMTOX, RID_HADD_FROMTOX_HELP, {}, {}},
    {},
    {RID_SUMX, RID_SUMX_HELP, {}, {}},
    {RID_SUM_FROMX, RID_SUM_FROMX_HELP, {}, {}},
    {RID_SUM_TOX, RID_SUM_TOX_HELP, {}, {}},
    {RID_SUM_FROMTOX, RID_SUM_FROMTOX_HELP, {}, {}},
    {},
    {RID_MAJX, RID_MAJX_HELP, {}, {}},
    {RID_MAJ_FROMX, RID_MAJ_FROMX_HELP, {}, {}},
    {RID_MAJ_TOX, RID_MAJ_TOX_HELP, {}, {}},
    {RID_MAJ_FROMTOX, RID_MAJ_FROMTOX_HELP, {}, {}},
    {},
    {RID_PRODX, RID_PRODX_HELP, {}, {}},
    {RID_PROD_FROMX, RID_PROD_FROMX_HELP, {}, {}},
    {RID_PROD_TOX, RID_PROD_TOX_HELP, {}, {}},
    {RID_PROD_FROMTOX, RID_PROD_FROMTOX_HELP, {}, {}},
    {},
    {RID_COPRODX, RID_COPRODX_HELP, {}, {}},
    {RID_COPROD_FROMX, RID_COPROD_FROMX_HELP, {}, {}},
    {RID_COPROD_TOX, RID_COPROD_TOX_HELP, {}, {}},
    {RID_COPROD_FROMTOX, RID_COPROD_FROMTOX_HELP, {}, {}},
    {},
    {RID_INTX, RID_INTX_HELP, {}, {}},
    {RID_INT_FROMX, RID_INT_FROMX_HELP, {}, {}},
    {RID_INT_TOX, RID_INT_TOX_HELP, {}, {}},
    {RID_INT_FROMTOX, RID_INT_FROMTOX_HELP, {}, {}},
    {},
    {RID_IINTX, RID_IINTX_HELP, {}, {}},
    {RID_IINT_FROMX, RID_IINT_FROMX_HELP, {}, {}},
    {RID_IINT_TOX, RID_IINT_TOX_HELP, {}, {}},
    {RID_IINT_FROMTOX, RID_IINT_FROMTOX_HELP, {}, {}},
    {},
    {RID_IIINTX, RID_IIINTX_HELP, {}, {}},
    {RID_IIINT_FROMX, RID_IIINT_FROMX_HELP, {}, {}},
    {RID_IIINT_TOX, RID_IIINT_TOX_HELP, {}, {}},
    {RID_IIINT_FROMTOX, RID_IIINT_FROMTOX_HELP, {}, {}},
    {},
    {RID_LINTX, RID_LINTX_HELP, {}, {}},
    {RID_LINT_FROMX, RID_LINT_FROMX_HELP, {}, {}},
    {RID_LINT_TOX, RID_LINT_TOX_HELP, {}, {}},
    {RID_LINT_FROMTOX, RID_LINT_FROMTOX_HELP, {}, {}},
    {},
    {RID_LLINTX, RID_LLINTX_HELP, {}, {}},
    {RID_LLINT_FROMX, RID_LLINT_FROMX_HELP, {}, {}},
    {RID_LLINT_TOX, RID_LLINT_TOX_HELP, {}, {}},
    {RID_LLINT_FROMTOX, RID_LLINT_FROMTOX_HELP, {}, {}},
    {},
    {RID_LLLINTX, RID_LLLINTX_HELP, {}, {}},
    {RID_LLLINT_FROMX, RID_LLLINT_FROMX_HELP, {}, {}},
    {RID_LLLINT_TOX, RID_LLLINT_TOX_HELP, {}, {}},
    {RID_LLLINT_FROMTOX, RID_LLLINT_FROMTOX_HELP, {}, {}},
    {},
    {RID_OPERX, RID_OPERX_HELP, u"oper \xE22B <?>", {}},
    {RID_OPER_FROMX, RID_OPER_FROMX_HELP, u"oper \xE22B from <?> <?>", {}},
    {RID_OPER_TOX, RID_OPER_TOX_HELP, u"oper \xE22B to <?> <?>", {}},
    {RID_OPER_FROMTOX, RID_OPER_FROMTOX_HELP, u"oper \xE22B from <?> to <?> <?>", {}},
};

const SmElementDescr s_a5AttributesList[] =
{
    {RID_ACUTEX, RID_ACUTEX_HELP, {}, {}},
    {RID_GRAVEX, RID_GRAVEX_HELP, {}, {}},
    {RID_BREVEX, RID_BREVEX_HELP, {}, {}},
    {RID_CIRCLEX, RID_CIRCLEX_HELP, {}, {}},
    {RID_DOTX, RID_DOTX_HELP, {}, {}},
    {RID_DDOTX, RID_DDOTX_HELP, {}, {}},
    {RID_DDDOTX, RID_DDDOTX_HELP, {}, {}},
    {RID_BARX, RID_BARX_HELP, {}, {}},
    {RID_VECX, RID_VECX_HELP, {}, {}},
    {RID_HARPOONX, RID_HARPOONX_HELP, {}, {}},
    {RID_TILDEX, RID_TILDEX_HELP, {}, {}},
    {RID_HATX, RID_HATX_HELP, {}, {}},
    {RID_CHECKX, RID_CHECKX_HELP, {}, {}},
    {},
    {RID_WIDEVECX, RID_WIDEVECX_HELP, {}, {}},
    {RID_WIDEHARPOONX, RID_WIDEHARPOONX_HELP, {}, {}},
    {RID_WIDETILDEX, RID_WIDETILDEX_HELP, {}, {}},
    {RID_WIDEHATX, RID_WIDEHATX_HELP, {}, {}},
    {RID_OVERLINEX, RID_OVERLINEX_HELP, {}, {}},
    {RID_UNDERLINEX, RID_UNDERLINEX_HELP, {}, {}},
    {RID_OVERSTRIKEX, RID_OVERSTRIKEX_HELP, {}, {}},
    {},
    {RID_PHANTOMX, RID_PHANTOMX_HELP, u"\"$1\"", STR_HIDE},
    {RID_BOLDX, RID_BOLDX_HELP, u"bold B", {}},
    {RID_ITALX, RID_ITALX_HELP, u"ital I", {}},
    {RID_SIZEXY, RID_SIZEXY_HELP, u"\"$1\"", STR_SIZE},
    {RID_FONTXY, RID_FONTXY_HELP, u"\"$1\"", STR_FONT},
    {},
    {RID_COLORX_BLACK, RID_COLORX_BLACK_HELP, u"color black { \"$1\" }", STR_BLACK},
    {RID_COLORX_BLUE, RID_COLORX_BLUE_HELP, u"color blue { \"$1\" }", STR_BLUE},
    {RID_COLORX_GREEN, RID_COLORX_GREEN_HELP, u"color green { \"$1\" }", STR_GREEN},
    {RID_COLORX_RED, RID_COLORX_RED_HELP, u"color red { \"$1\" }", STR_RED},
    {RID_COLORX_AQUA, RID_COLORX_AQUA_HELP, u"color aqua { \"$1\" }", STR_AQUA},
    {RID_COLORX_FUCHSIA, RID_COLORX_FUCHSIA_HELP, u"color fuchsia { \"$1\" }", STR_FUCHSIA},
    {RID_COLORX_YELLOW, RID_COLORX_YELLOW_HELP, u"color yellow { \"$1\" }", STR_YELLOW},
    {RID_COLORX_GRAY, RID_COLORX_GRAY_HELP, u"color gray { \"$1\" }", STR_GRAY},
    {RID_COLORX_LIME, RID_COLORX_LIME_HELP, u"color lime { \"$1\" }", STR_LIME},
    {RID_COLORX_MAROON, RID_COLORX_MAROON_HELP, u"color maroon { \"$1\" }", STR_MAROON},
    {RID_COLORX_NAVY, RID_COLORX_NAVY_HELP, u"color navy { \"$1\" }", STR_NAVY},
    {RID_COLORX_OLIVE, RID_COLORX_OLIVE_HELP, u"color olive { \"$1\" }", STR_OLIVE},
    {RID_COLORX_PURPLE, RID_COLORX_PURPLE_HELP, u"color purple { \"$1\" }", STR_PURPLE},
    {RID_COLORX_SILVER, RID_COLORX_SILVER_HELP, u"color silver { \"$1\" }", STR_SILVER},
    {RID_COLORX_TEAL, RID_COLORX_TEAL_HELP, u"color teal { \"$1\" }", STR_TEAL},
    {RID_COLORX_RGB, RID_COLORX_RGB_HELP, u"color rgb 0 0 0 { \"$1\" }", STR_RGB},
    //{RID_COLORX_RGBA, RID_COLORX_RGBA_HELP, u"color rgba 0 0 0 0 { \"$1\" }", STR_RGBA},
    {RID_COLORX_HEX, RID_COLORX_HEX_HELP, u"color hex 000000 { \"$1\" }", STR_HEX},
    {},
    {RID_COLORX_CORAL, RID_COLORX_CORAL_HELP, u"color coral { \"$1\" }", STR_CORAL},
    {RID_COLORX_MIDNIGHT, RID_COLORX_MIDNIGHT_HELP, u"color midnightblue { \"$1\" }", STR_MIDNIGHT},
    {RID_COLORX_CRIMSON, RID_COLORX_CRIMSON_HELP, u"color crimson { \"$1\" }", STR_CRIMSON},
    {RID_COLORX_VIOLET, RID_COLORX_VIOLET_HELP, u"color violet { \"$1\" }", STR_VIOLET},
    {RID_COLORX_ORANGE, RID_COLORX_ORANGE_HELP, u"color orange { \"$1\" }", STR_ORANGE},
    {RID_COLORX_ORANGERED, RID_COLORX_ORANGERED_HELP, u"color orangered { \"$1\" }", STR_ORANGERED},
    {RID_COLORX_SEAGREEN, RID_COLORX_SEAGREEN_HELP, u"color seagreen { \"$1\" }", STR_SEAGREEN},
    {RID_COLORX_INDIGO, RID_COLORX_INDIGO_HELP, u"color indigo { \"$1\" }", STR_INDIGO},
    {RID_COLORX_HOTPINK, RID_COLORX_HOTPINK_HELP, u"color hotpink { \"$1\" }", STR_HOTPINK},
    {RID_COLORX_LAVENDER, RID_COLORX_LAVENDER_HELP, u"color lavender { \"$1\" }", STR_LAVENDER},
    {RID_COLORX_SNOW, RID_COLORX_SNOW_HELP, u"color snow { \"$1\" }", STR_SNOW},
};

const SmElementDescr s_a5BracketsList[] =
{
    {RID_LRGROUPX, RID_LRGROUPX_HELP, {}, {}},
    {},
    {RID_LRPARENTX, RID_LRPARENTX_HELP, {}, {}},
    {RID_LRBRACKETX, RID_LRBRACKETX_HELP, {}, {}},
    {RID_LRDBRACKETX, RID_LRDBRACKETX_HELP, {}, {}},
    {RID_LRBRACEX, RID_LRBRACEX_HELP, {}, {}},
    {RID_LRANGLEX, RID_LRANGLEX_HELP, {}, {}},
    {RID_LMRANGLEXY, RID_LMRANGLEXY_HELP, {}, {}},
    {RID_LRCEILX, RID_LRCEILX_HELP, {}, {}},
    {RID_LRFLOORX, RID_LRFLOORX_HELP, {}, {}},
    {RID_LRLINEX, RID_LRLINEX_HELP, {}, {}},
    {RID_LRDLINEX, RID_LRDLINEX_HELP, {}, {}},
    {},
    {RID_SLRPARENTX, RID_SLRPARENTX_HELP, u"left ( binom{<?>}{<?>} right )", {}},
    {RID_SLRBRACKETX, RID_SLRBRACKETX_HELP, u"left [ binom{<?>}{<?>} right ]", {}},
    {RID_SLRDBRACKETX, RID_SLRDBRACKETX_HELP, u"left ldbracket binom{<?>}{<?>} right rdbracket", {}},
    {RID_SLRBRACEX, RID_SLRBRACEX_HELP, u"left lbrace binom{<?>}{<?>} right rbrace", {}},
    {RID_SLRANGLEX, RID_SLRANGLEX_HELP, u"left langle binom{<?>}{<?>} right rangle", {}},
    {RID_SLMRANGLEXY, RID_SLMRANGLEXY_HELP, u"left langle binom{<?>}{<?>} mline binom{<?>}{<?>} right rangle", {}},
    {RID_SLRCEILX, RID_SLRCEILX_HELP, u"left lceil binom{<?>}{<?>} right rceil", {}},
    {RID_SLRFLOORX, RID_SLRFLOORX_HELP, u"left lfloor binom{<?>}{<?>} right rfloor", {}},
    {RID_SLRLINEX, RID_SLRLINEX_HELP, u"left lline binom{<?>}{<?>} right rline", {}},
    {RID_SLRDLINEX, RID_SLRDLINEX_HELP, u"left ldline binom{<?>}{<?>} right rdline", {}},
    {},
    {RID_XOVERBRACEY, RID_XOVERBRACEY_HELP, u"{<?><?><?>} overbrace {<?>}", {}},
    {RID_XUNDERBRACEY, RID_XUNDERBRACEY_HELP, u"{<?><?><?>} underbrace {<?>} ", {}},
    {},
    {RID_EVALX, RID_EVALUATEX_HELP, {}, {}},
    {RID_EVAL_FROMX, RID_EVALUATE_FROMX_HELP, {}, {}},
    {RID_EVAL_TOX, RID_EVALUATE_TOX_HELP, {}, {}},
    {RID_EVAL_FROMTOX, RID_EVALUATE_FROMTOX_HELP, {}, {}},
};

const SmElementDescr s_a5FormatsList[] =
{
    {RID_RSUPX, RID_RSUPX_HELP, {}, {}},
    {RID_RSUBX, RID_RSUBX_HELP, {}, {}},
    {RID_LSUPX, RID_LSUPX_HELP, {}, {}},
    {RID_LSUBX, RID_LSUBX_HELP, {}, {}},
    {RID_CSUPX, RID_CSUPX_HELP, {}, {}},
    {RID_CSUBX, RID_CSUBX_HELP, {}, {}},
    {},
    {RID_NEWLINE, RID_NEWLINE_HELP, u"\u21B5", {}},
    {RID_SBLANK, RID_SBLANK_HELP, u"\"`\"", {}},
    {RID_BLANK, RID_BLANK_HELP, u"\"~\"", {}},
    {RID_NOSPACE, RID_NOSPACE_HELP, {}, {}},
    {RID_ALIGNLX, RID_ALIGNLX_HELP, u"\"$1\"", STR_ALIGN_LEFT},
    {RID_ALIGNCX, RID_ALIGNCX_HELP, u"\"$1\"", STR_ALIGN_CENTER},
    {RID_ALIGNRX, RID_ALIGNRX_HELP, u"\"$1\"", STR_ALIGN_RIGHT},
    {},
    {RID_BINOMXY, RID_BINOMXY_HELP, {}, {}},
    {RID_STACK, RID_STACK_HELP, {}, {}},
    {RID_MATRIX, RID_MATRIX_HELP, {}, {}},
};

const SmElementDescr s_a5OthersList[] =
{
    {RID_INFINITY, RID_INFINITY_HELP, {}, {}},
    {RID_PARTIAL, RID_PARTIAL_HELP, {}, {}},
    {RID_NABLA, RID_NABLA_HELP, {}, {}},
    {RID_EXISTS, RID_EXISTS_HELP, {}, {}},
    {RID_NOTEXISTS, RID_NOTEXISTS_HELP, {}, {}},
    {RID_FORALL, RID_FORALL_HELP, {}, {}},
    {RID_HBAR, RID_HBAR_HELP, {}, {}},
    {RID_LAMBDABAR, RID_LAMBDABAR_HELP, {}, {}},
    {RID_RE, RID_RE_HELP, {}, {}},
    {RID_IM, RID_IM_HELP, {}, {}},
    {RID_WP, RID_WP_HELP, {}, {}},
    {RID_LAPLACE, RID_LAPLACE_HELP, {}, {}},
    {RID_FOURIER, RID_FOURIER_HELP, {}, {}},
    {RID_BACKEPSILON, RID_BACKEPSILON_HELP, {}, {}},
    {},
    {RID_LEFTARROW, RID_LEFTARROW_HELP, {}, {}},
    {RID_RIGHTARROW, RID_RIGHTARROW_HELP, {}, {}},
    {RID_UPARROW, RID_UPARROW_HELP, {}, {}},
    {RID_DOWNARROW, RID_DOWNARROW_HELP, {}, {}},
    {},
    {RID_DOTSLOW, RID_DOTSLOW_HELP, {}, {}},
    {RID_DOTSAXIS, RID_DOTSAXIS_HELP, {}, {}},
    {RID_DOTSVERT, RID_DOTSVERT_HELP, {}, {}},
    {RID_DOTSUP, RID_DOTSUP_HELP, {}, {}},
    {RID_DOTSDOWN, RID_DOTSDOWN_HELP, {}, {}},
};

const SmElementDescr s_a5ExamplesList[] =
{
    {u"{func e}^{i %pi} + 1 = 0", RID_EXAMPLE_EULER_IDENTITY_HELP, {}, {}},
    {u"C = %pi cdot d = 2 cdot %pi cdot r", RID_EXAMPLE_CIRCUMFERENCE_HELP, {}, {}},
    {u"c = sqrt{ a^2 + b^2 }", RID_EXAMPLE_PYTHAGOREAN_THEO_HELP, {}, {}},
    {u"vec F = m times vec a", RID_EXAMPLE_2NEWTON, {}, {}},
    {u"E = m c^2", RID_EXAMPLE_MASS_ENERGY_EQUIV_HELP, {}, {}},
    {u"G_{%mu %nu} + %LAMBDA g_{%mu %nu}= frac{8 %pi G}{c^4} T_{%mu %nu}", RID_EXAMPLE_GENERAL_RELATIVITY_HELP, {}, {}},
    {u"%DELTA t' = { %DELTA t } over sqrt{ 1 - v^2 over c^2 }", RID_EXAMPLE_SPECIAL_RELATIVITY_HELP, {}, {}},
    {u"d over dt left( {partial L}over{partial dot q} right) = {partial L}over{partial q}", RID_EXAMPLE_EULER_LAGRANGE_HELP, {}, {}},
    {u"int from a to b f'(x) dx = f(b) - f(a)", RID_EXAMPLE_FTC_HELP, {}, {}},
    {u"ldline %delta bold{r}(t) rdline approx e^{%lambda t} ldline %delta { bold{r} }_0 rdline", RID_EXAMPLE_CHAOS_HELP, {}, {}},
    {u"f(x) = sum from { n=0 } to { infinity } { {f^{(n)}(x_0) } over { fact{n} } (x-x_0)^n }", RID_EXAMPLE_A_TAYLOR_SERIES_HELP, {}, {}},
    {u"f(x) = {1} over { %sigma sqrt{2 %pi} } func e^-{ {(x-%mu)^2} over {2 %sigma^2} }", RID_EXAMPLE_GAUSS_DISTRIBUTION_HELP, {}, {}},
};

const std::vector<TranslateId> s_a5Categories{
    RID_CATEGORY_UNARY_BINARY_OPERATORS,
    RID_CATEGORY_RELATIONS,
    RID_CATEGORY_SET_OPERATIONS,
    RID_CATEGORY_FUNCTIONS,
    RID_CATEGORY_OPERATORS,
    RID_CATEGORY_ATTRIBUTES,
    RID_CATEGORY_BRACKETS,
    RID_CATEGORY_FORMATS,
    RID_CATEGORY_OTHERS,
    RID_CATEGORY_EXAMPLES,
    RID_CATEGORY_USERDEFINED,
};

template <size_t N>
constexpr std::pair<const SmElementDescr*, size_t> asPair(const SmElementDescr (&category)[N])
{
    return { category, N };
}

const std::vector<std::pair<const SmElementDescr*, size_t>> s_a5CategoryDescriptions{
    { asPair(s_a5UnaryBinaryOperatorsList) },
    { asPair(s_a5RelationsList) },
    { asPair(s_a5SetOperationsList) },
    { asPair(s_a5FunctionsList) },
    { asPair(s_a5OperatorsList) },
    { asPair(s_a5AttributesList) },
    { asPair(s_a5BracketsList) },
    { asPair(s_a5FormatsList) },
    { asPair(s_a5OthersList) },
    { asPair(s_a5ExamplesList) },
};

} // namespace

// static
const std::vector<TranslateId>& SmElementsControl::categories()
{
    return s_a5Categories;
}

struct ElementData
{
    OUString maElementSource;
    OUString maHelpText;
    int maPos;
    ElementData(const OUString& aElementSource, const OUString& aHelpText, const int& aPos)
        : maElementSource(aElementSource)
        , maHelpText(aHelpText)
        , maPos(aPos)
    {
    }
};

SmElementsControl::SmElementsControl(std::unique_ptr<weld::IconView> pIconView,
                                     std::unique_ptr<weld::Menu> pMenu)
    : mpDocShell(new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT))
    , mnCurrentSetIndex(-1)
    , m_nSmSyntaxVersion(SmModule::get()->GetConfig()->GetDefaultSmSyntaxVersion())
    , m_bAllowDelete(false)
    , mpIconView(std::move(pIconView))
    , mxPopup(std::move(pMenu))
{
    maParser.reset(starmathdatabase::GetVersionSmParser(m_nSmSyntaxVersion));
    maParser->SetImportSymbolNames(true);

    mpIconView->connect_query_tooltip(LINK(this, SmElementsControl, QueryTooltipHandler));
    mpIconView->connect_item_activated(LINK(this, SmElementsControl, ElementActivatedHandler));
    mpIconView->connect_mouse_press(LINK(this, SmElementsControl, MousePressHdl));
}

SmElementsControl::~SmElementsControl()
{
    mpDocShell->DoClose();
}

Color SmElementsControl::GetTextColor()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return rStyleSettings.GetFieldTextColor();
}

Color SmElementsControl::GetControlBackground()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return rStyleSettings.GetFieldColor();
}

namespace
{
    // SmTmpDevice::GetTextColor assumes a fg/bg of svtools::FONTCOLOR/svtools::DOCCOLOR
    // here replace COL_AUTO with the desired fg color, alternatively could add something
    // to SmTmpDevice to override its defaults
    class AutoColorVisitor : public SmDefaultingVisitor
    {
    private:
        Color m_aAutoColor;
    public:
        AutoColorVisitor(SmNode* pNode, Color aAutoColor)
            : m_aAutoColor(aAutoColor)
        {
            DefaultVisit(pNode);
        }
        virtual void DefaultVisit(SmNode* pNode) override
        {
            if (pNode->GetFont().GetColor() == COL_AUTO)
                pNode->GetFont().SetColor(m_aAutoColor);
            size_t nNodes = pNode->GetNumSubNodes();
            for (size_t i = 0; i < nNodes; ++i)
            {
                SmNode* pChild = pNode->GetSubNode(i);
                if (!pChild)
                    continue;
                DefaultVisit(pChild);
            }
        }
    };
}

void SmElementsControl::addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText)
{
    std::unique_ptr<SmNode> pNode = maParser->ParseExpression(aElementVisual);
    ScopedVclPtr<VirtualDevice> pDevice(mpIconView->create_virtual_device());
    pDevice->SetMapMode(MapMode(SmMapUnit()));
    pDevice->SetDrawMode(DrawModeFlags::Default);
    pDevice->SetLayoutMode(vcl::text::ComplexTextLayoutFlags::Default);
    pDevice->SetDigitLanguage(LANGUAGE_ENGLISH);
    pDevice->EnableRTL(false);

    pDevice->SetBackground(GetControlBackground());
    pDevice->SetTextColor(GetTextColor());

    pNode->Prepare(maFormat, *mpDocShell, 0);
    pNode->SetSize(Fraction(10,8));
    pNode->Arrange(*pDevice, maFormat);

    AutoColorVisitor(pNode.get(), GetTextColor());

    Size aSize = pDevice->LogicToPixel(Size(pNode->GetWidth(), pNode->GetHeight()));
    aSize.extendBy(10, 0); // Add 5 pixels from both sides to accommodate extending parts of italics
    pDevice->SetOutputSizePixel(aSize);
    SmDrawingVisitor(*pDevice, pDevice->PixelToLogic(Point(5, 0)), pNode.get(), maFormat);

    maItemDatas.push_back(std::make_unique<ElementData>(aElementSource, aHelpText, maItemDatas.size()));
    const OUString aId(weld::toId(maItemDatas.back().get()));
    mpIconView->insert(-1, nullptr, &aId, pDevice, nullptr);
    mpIconView->set_item_accessible_name(mpIconView->n_children() - 1, GetElementHelpText(aId));
    if (mpIconView->get_item_width() < aSize.Width())
        mpIconView->set_item_width(aSize.Width());
}

OUString SmElementsControl::GetElementSource(const OUString& itemId)
{
    return weld::fromId<ElementData*>(itemId)->maElementSource;
}

OUString SmElementsControl::GetElementHelpText(const OUString& itemId)
{
    return weld::fromId<ElementData*>(itemId)->maHelpText;
}

int SmElementsControl::GetElementPos(const OUString& itemId)
{
    return weld::fromId<ElementData*>(itemId)->maPos;
}

void SmElementsControl::setElementSetIndex(int nSetIndex, bool bForceBuild)
{
    if (!bForceBuild && mnCurrentSetIndex == nSetIndex)
        return;
    mnCurrentSetIndex = nSetIndex;
    build();
}

void SmElementsControl::addElements(int nCategory)
{
    mpIconView->freeze();
    mpIconView->clear();
    mpIconView->set_item_width(0);
    maItemDatas.clear();

    if (o3tl::make_unsigned(nCategory) < s_a5CategoryDescriptions.size())
    {
        const auto& [aElementsArray, aElementsArraySize] = s_a5CategoryDescriptions[nCategory];

        for (size_t i = 0; i < aElementsArraySize; i++)
        {
            const auto& [element, elementHelp, elementVisual, visualTranslatable] = aElementsArray[i];
            if (element.empty())
            {
                mpIconView->append_separator({});
            }
            else
            {
                OUString aElement(element);
                OUString aVisual(elementVisual.empty() ? aElement : OUString(elementVisual));
                if (visualTranslatable)
                    aVisual = aVisual.replaceFirst("$1", SmResId(visualTranslatable));
                OUString aHelp(elementHelp ? SmResId(elementHelp) : OUString());
                addElement(aVisual, aElement, aHelp);
            }
        }
    }
    else
    {
        css::uno::Sequence<OUString> sNames = SmModule::get()->GetConfig()->LoadUserDefinedNames();
        OUString sFormula;
        for (int i = 0; i < sNames.getLength(); i++)
        {
            SmModule::get()->GetConfig()->GetUserDefinedFormula(sNames[i], sFormula);
            addElement(sFormula, sFormula, sNames[i]);
        }
    }

    mpIconView->set_size_request(0, 0);
    mpIconView->thaw();
}

void SmElementsControl::build()
{
    switch(m_nSmSyntaxVersion)
    {
        case 5:
            addElements(mnCurrentSetIndex);
            m_sHoveredItem = "nil"; // if list is empty we must not use the previously hovered item
            break;
        case 6:
        default:
            throw std::range_error("parser version limit");
    }
}

void SmElementsControl::setSmSyntaxVersion(sal_Int16 nSmSyntaxVersion)
{
    if( m_nSmSyntaxVersion != nSmSyntaxVersion )
    {
        m_nSmSyntaxVersion = nSmSyntaxVersion;
        maParser.reset(starmathdatabase::GetVersionSmParser(nSmSyntaxVersion));
        maParser->SetImportSymbolNames(true);
        // Be careful, we need the parser in order to build !!!
        build();
    }
}

IMPL_LINK(SmElementsControl, QueryTooltipHandler, const weld::TreeIter&, iter, OUString)
{
    if (const OUString id = mpIconView->get_id(iter); !id.isEmpty())
    {
        m_sHoveredItem = id;
        return GetElementHelpText(id);
    }
    return {};
}

IMPL_LINK_NOARG(SmElementsControl, ElementActivatedHandler, weld::IconView&, bool)
{
    if (const OUString id = mpIconView->get_selected_id(); !id.isEmpty())
        maSelectHdlLink.Call(GetElementSource(id));

    mpIconView->unselect_all();
    return true;
}

IMPL_LINK(SmElementsControl, MousePressHdl, const MouseEvent&, rEvt, bool)
{
    if (rEvt.IsRight() && m_bAllowDelete && (m_sHoveredItem != "nil"))
    {
        mpIconView->select( GetElementPos(m_sHoveredItem) );
        OUString sElementId = mpIconView->get_selected_id();
        if (!sElementId.isEmpty())
        {
            OUString sResponse = mxPopup->popup_at_rect(
                mpIconView.get(), tools::Rectangle(rEvt.GetPosPixel(), Size(1, 1)));
            if (sResponse == "delete")
            {
                SmModule::get()->GetConfig()->DeleteUserDefinedFormula( GetElementHelpText(m_sHoveredItem) );
                build(); //refresh view
            }
            mpIconView->unselect_all();
        }
    }
    return true;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
