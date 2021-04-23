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
#include <view.hxx>
#include <visitors.hxx>
#include <document.hxx>
#include "uiobject.hxx"
#include <strings.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <vcl/uitest/logger.hxx>

SmElement::SmElement(std::unique_ptr<SmNode>&& pNode, const OUString& aText, const OUString& aHelpText) :
    mpNode(std::move(pNode)),
    maText(aText),
    maHelpText(aHelpText)
{}

SmElement::~SmElement()
{}

const std::unique_ptr<SmNode>& SmElement::getNode() const { return mpNode; }

SmElementSeparator::SmElementSeparator() :
    SmElement(std::unique_ptr<SmNode>(), OUString(), OUString())
{}

const SmElementDescr SmElementsControl::m_a5UnaryBinaryOperatorsList[] =
{
    {RID_PLUSX, RID_PLUSX_HELP}, {RID_MINUSX, RID_MINUSX_HELP},
    {RID_PLUSMINUSX, RID_PLUSMINUSX_HELP}, {RID_MINUSPLUSX, RID_MINUSPLUSX_HELP},
    {nullptr, nullptr},
    {RID_XPLUSY, RID_XPLUSY_HELP}, {RID_XMINUSY, RID_XMINUSY_HELP},
    {RID_XCDOTY, RID_XCDOTY_HELP}, {RID_XTIMESY, RID_XTIMESY_HELP},
    {RID_XSYMTIMESY, RID_XSYMTIMESY_HELP}, {RID_XOVERY, RID_XOVERY_HELP},
    {RID_FRACXY, RID_FRACXY_HELP},
    {RID_XDIVY, RID_XDIVY_HELP}, {RID_XSYMDIVIDEY, RID_XSYMDIVIDEY_HELP},
    {RID_XOPLUSY, RID_XOPLUSY_HELP}, {RID_XOMINUSY, RID_XOMINUSY_HELP},
    {RID_XODOTY, RID_XODOTY_HELP}, {RID_XOTIMESY, RID_XOTIMESY_HELP},
    {RID_XODIVIDEY, RID_XODIVIDEY_HELP}, {RID_XCIRCY, RID_XCIRCY_HELP},
    {RID_XWIDESLASHY, RID_XWIDESLASHY_HELP}, {RID_XWIDEBSLASHY, RID_XWIDEBSLASHY_HELP},
    {nullptr, nullptr},
    {RID_NEGX, RID_NEGX_HELP}, {RID_XANDY, RID_XANDY_HELP}, {RID_XORY, RID_XORY_HELP}
};

const SmElementDescr SmElementsControl::m_a5RelationsList[] =
{
    {RID_XEQY, RID_XEQY_HELP}, {RID_XNEQY, RID_XNEQY_HELP}, {RID_XLTY, RID_XLTY_HELP},
    {RID_XLEY, RID_XLEY_HELP}, {RID_XLESLANTY, RID_XLESLANTY_HELP}, {RID_XGTY, RID_XGTY_HELP},
    {RID_XGEY, RID_XGEY_HELP}, {RID_XGESLANTY, RID_XGESLANTY_HELP},
    {RID_XLLY, RID_XLLY_HELP}, {RID_XGGY, RID_XGGY_HELP},
    {nullptr, nullptr},
    {RID_XAPPROXY, RID_XAPPROXY_HELP}, {RID_XSIMY, RID_XSIMY_HELP}, {RID_XSIMEQY, RID_XSIMEQY_HELP},
    {RID_XEQUIVY, RID_XEQUIVY_HELP}, {RID_XPROPY, RID_XPROPY_HELP}, {RID_XPARALLELY, RID_XPARALLELY_HELP},
    {RID_XORTHOY, RID_XORTHOY_HELP}, {RID_XDIVIDESY, RID_XDIVIDESY_HELP}, {RID_XNDIVIDESY, RID_XNDIVIDESY_HELP},
    {RID_XTOWARDY, RID_XTOWARDY_HELP}, {RID_XTRANSLY, RID_XTRANSLY_HELP}, {RID_XTRANSRY, RID_XTRANSRY_HELP},
    {RID_XDEFY, RID_XDEFY_HELP},
    {nullptr, nullptr},
    {RID_DLARROW, RID_DLARROW_HELP}, {RID_DLRARROW, RID_DLRARROW_HELP}, {RID_DRARROW, RID_DRARROW_HELP},
    {nullptr, nullptr},
    {RID_XPRECEDESY, RID_XPRECEDESY_HELP}, {RID_XSUCCEEDSY, RID_XSUCCEEDSY_HELP},
    {RID_XPRECEDESEQUALY, RID_XPRECEDESEQUALY_HELP}, {RID_XSUCCEEDSEQUALY, RID_XSUCCEEDSEQUALY_HELP},
    {RID_XPRECEDESEQUIVY, RID_XPRECEDESEQUIVY_HELP}, {RID_XSUCCEEDSEQUIVY, RID_XSUCCEEDSEQUIVY_HELP},
    {RID_XNOTPRECEDESY, RID_XNOTPRECEDESY_HELP}, {RID_XNOTSUCCEEDSY, RID_XNOTSUCCEEDSY_HELP},
};

const SmElementDescr SmElementsControl::m_a5SetOperationsList[] =
{
    {RID_XINY, RID_XINY_HELP}, {RID_XNOTINY, RID_XNOTINY_HELP}, {RID_XOWNSY, RID_XOWNSY_HELP},
    {nullptr, nullptr},
    {RID_XINTERSECTIONY, RID_XINTERSECTIONY_HELP}, {RID_XUNIONY, RID_XUNIONY_HELP},
    {RID_XSETMINUSY, RID_XSETMINUSY_HELP}, {RID_XSETQUOTIENTY, RID_XSETQUOTIENTY_HELP},
    {RID_XSUBSETY, RID_XSUBSETY_HELP}, {RID_XSUBSETEQY, RID_XSUBSETEQY_HELP},
    {RID_XSUPSETY, RID_XSUPSETY_HELP}, {RID_XSUPSETEQY, RID_XSUPSETEQY_HELP},
    {RID_XNSUBSETY, RID_XNSUBSETY_HELP}, {RID_XNSUBSETEQY, RID_XNSUBSETEQY_HELP},
    {RID_XNSUPSETY, RID_XNSUPSETY_HELP}, {RID_XNSUPSETEQY, RID_XNSUPSETEQY_HELP},
    {nullptr, nullptr},
    {RID_EMPTYSET, RID_EMPTYSET_HELP}, {RID_ALEPH, RID_ALEPH_HELP}, {RID_SETN, RID_SETN_HELP},
    {RID_SETZ, RID_SETZ_HELP}, {RID_SETQ, RID_SETQ_HELP}, {RID_SETR, RID_SETR_HELP}, {RID_SETC, RID_SETC_HELP}
};

const SmElementDescr SmElementsControl::m_a5FunctionsList[] =
{
    {RID_ABSX, RID_ABSX_HELP}, {RID_FACTX, RID_FACTX_HELP}, {RID_SQRTX, RID_SQRTX_HELP},
    {RID_NROOTXY, RID_NROOTXY_HELP}, {RID_RSUPX, RID_RSUPX_HELP}, {RID_EX, RID_EX_HELP},
    {RID_LNX, RID_LNX_HELP}, {RID_EXPX, RID_EXPX_HELP}, {RID_LOGX, RID_LOGX_HELP},
    {nullptr, nullptr},
    {RID_SINX, RID_SINX_HELP}, {RID_COSX, RID_COSX_HELP}, {RID_TANX, RID_TANX_HELP}, {RID_COTX, RID_COTX_HELP},
    {RID_SINHX, RID_SINHX_HELP}, {RID_COSHX, RID_COSHX_HELP}, {RID_TANHX, RID_TANHX_HELP},
    {RID_COTHX, RID_COTHX_HELP},
    {nullptr, nullptr},
    {RID_ARCSINX, RID_ARCSINX_HELP}, {RID_ARCCOSX, RID_ARCCOSX_HELP}, {RID_ARCTANX, RID_ARCTANX_HELP},
    {RID_ARCCOTX, RID_ARCCOTX_HELP}, {RID_ARSINHX, RID_ARSINHX_HELP}, {RID_ARCOSHX, RID_ARCOSHX_HELP},
    {RID_ARTANHX, RID_ARTANHX_HELP}, {RID_ARCOTHX, RID_ARCOTHX_HELP},
    {nullptr, nullptr},
    {RID_FUNCX, RID_FUNCX_HELP}
};

const SmElementDescr SmElementsControl::m_a5OperatorsList[] =
{
    {RID_LIMX, RID_LIMX_HELP}, {RID_LIM_FROMX, RID_LIM_FROMX_HELP},
    {RID_LIM_TOX, RID_LIM_TOX_HELP}, {RID_LIM_FROMTOX, RID_LIM_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_LIMINFX, RID_LIMINFX_HELP}, {RID_LIMINF_FROMX, RID_LIMINF_FROMX_HELP},
    {RID_LIMINF_TOX, RID_LIMINF_TOX_HELP}, {RID_LIMINF_FROMTOX, RID_LIMINF_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_LIMSUPX, RID_LIMSUPX_HELP}, {RID_LIMSUP_FROMX, RID_LIMSUP_FROMX_HELP},
    {RID_LIMSUP_TOX, RID_LIMSUP_TOX_HELP}, {RID_LIMSUP_FROMTOX, RID_LIMSUP_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_SUMX, RID_SUMX_HELP}, {RID_SUM_FROMX, RID_SUM_FROMX_HELP},
    {RID_SUM_TOX, RID_SUM_TOX_HELP}, {RID_SUM_FROMTOX, RID_SUM_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_PRODX, RID_PRODX_HELP}, {RID_PROD_FROMX, RID_PROD_FROMX_HELP},
    {RID_PROD_TOX, RID_PROD_TOX_HELP}, {RID_PROD_FROMTOX, RID_PROD_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_COPRODX, RID_COPRODX_HELP}, {RID_COPROD_FROMX, RID_COPROD_FROMX_HELP},
    {RID_COPROD_TOX, RID_COPROD_TOX_HELP}, {RID_COPROD_FROMTOX, RID_COPROD_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_INTX, RID_INTX_HELP}, {RID_INT_FROMX, RID_INT_FROMX_HELP},
    {RID_INT_TOX, RID_INT_TOX_HELP}, {RID_INT_FROMTOX, RID_INT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_IINTX, RID_IINTX_HELP}, {RID_IINT_FROMX, RID_IINT_FROMX_HELP},
    {RID_IINT_TOX, RID_IINT_TOX_HELP}, {RID_IINT_FROMTOX, RID_IINT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_IIINTX, RID_IIINTX_HELP}, {RID_IIINT_FROMX, RID_IIINT_FROMX_HELP},
    {RID_IIINT_TOX, RID_IIINT_TOX_HELP}, {RID_IIINT_FROMTOX, RID_IIINT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_LINTX, RID_LINTX_HELP}, {RID_LINT_FROMX, RID_LINT_FROMX_HELP},
    {RID_LINT_TOX, RID_LINT_TOX_HELP}, {RID_LINT_FROMTOX, RID_LINT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_LLINTX, RID_LLINTX_HELP}, {RID_LLINT_FROMX, RID_LLINT_FROMX_HELP},
    {RID_LLINT_TOX, RID_LLINT_TOX_HELP}, {RID_LLINT_FROMTOX, RID_LLINT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_LLLINTX, RID_LLLINTX_HELP}, {RID_LLLINT_FROMX, RID_LLLINT_FROMX_HELP},
    {RID_LLLINT_TOX, RID_LLLINT_TOX_HELP}, {RID_LLLINT_FROMTOX, RID_LLLINT_FROMTOX_HELP},
    {nullptr, nullptr},
    {RID_OPERX, RID_OPERX_HELP}, {RID_OPER_FROMX, RID_OPER_FROMX_HELP},
    {RID_OPER_TOX, RID_OPER_TOX_HELP}, {RID_OPER_FROMTOX, RID_OPER_FROMTOX_HELP}
};

const SmElementDescr SmElementsControl::m_a5AttributesList[] =
{
    {RID_ACUTEX, RID_ACUTEX_HELP}, {RID_GRAVEX, RID_GRAVEX_HELP}, {RID_BREVEX, RID_BREVEX_HELP},
    {RID_CIRCLEX, RID_CIRCLEX_HELP}, {RID_DOTX, RID_DOTX_HELP}, {RID_DDOTX, RID_DDOTX_HELP},
    {RID_DDDOTX, RID_DDDOTX_HELP}, {RID_BARX, RID_BARX_HELP}, {RID_VECX, RID_VECX_HELP},
    {RID_HARPOONX, RID_HARPOONX_HELP},
    {RID_TILDEX, RID_TILDEX_HELP}, {RID_HATX, RID_HATX_HELP}, {RID_CHECKX, RID_CHECKX_HELP},
    {nullptr, nullptr},
    {RID_WIDEVECX, RID_WIDEVECX_HELP}, {RID_WIDEHARPOONX, RID_WIDEHARPOONX_HELP},
    {RID_WIDETILDEX, RID_WIDETILDEX_HELP}, {RID_WIDEHATX, RID_WIDEHATX_HELP},
    {RID_OVERLINEX, RID_OVERLINEX_HELP}, {RID_UNDERLINEX, RID_UNDERLINEX_HELP}, {RID_OVERSTRIKEX, RID_OVERSTRIKEX_HELP},
    {nullptr, nullptr},
    {RID_PHANTOMX, RID_PHANTOMX_HELP}, {RID_BOLDX, RID_BOLDX_HELP}, {RID_ITALX, RID_ITALX_HELP},
    {RID_SIZEXY, RID_SIZEXY_HELP}, {RID_FONTXY, RID_FONTXY_HELP},
    {nullptr, nullptr},
    {RID_COLORX_BLACK, RID_COLORX_BLACK_HELP}, {RID_COLORX_BLUE, RID_COLORX_BLUE_HELP},
    {RID_COLORX_GREEN, RID_COLORX_GREEN_HELP}, {RID_COLORX_RED, RID_COLORX_RED_HELP},
    {RID_COLORX_AQUA, RID_COLORX_AQUA_HELP}, {RID_COLORX_FUCHSIA, RID_COLORX_FUCHSIA_HELP},
    {RID_COLORX_YELLOW, RID_COLORX_YELLOW_HELP}, {RID_COLORX_GRAY, RID_COLORX_GRAY_HELP},
    {RID_COLORX_LIME, RID_COLORX_LIME_HELP}, {RID_COLORX_MAROON, RID_COLORX_MAROON_HELP},
    {RID_COLORX_NAVY, RID_COLORX_NAVY_HELP}, {RID_COLORX_OLIVE, RID_COLORX_OLIVE_HELP},
    {RID_COLORX_PURPLE, RID_COLORX_PURPLE_HELP}, {RID_COLORX_SILVER, RID_COLORX_SILVER_HELP},
    {RID_COLORX_TEAL, RID_COLORX_TEAL_HELP},{RID_COLORX_RGB, RID_COLORX_RGB_HELP},
    {RID_COLORX_HEX, RID_COLORX_HEX_HELP},
    {nullptr, nullptr},
    {RID_COLORX_CORAL, RID_COLORX_CORAL_HELP}, {RID_COLORX_MIDNIGHT, RID_COLORX_MIDNIGHT_HELP},
    {RID_COLORX_CRIMSON, RID_COLORX_CRIMSON_HELP}, {RID_COLORX_VIOLET, RID_COLORX_VIOLET_HELP},
    {RID_COLORX_ORANGE, RID_COLORX_ORANGE_HELP}, {RID_COLORX_ORANGERED, RID_COLORX_ORANGERED_HELP},
    {RID_COLORX_SEAGREEN, RID_COLORX_SEAGREEN_HELP}, {RID_COLORX_INDIGO, RID_COLORX_INDIGO_HELP},
    {RID_COLORX_HOTPINK, RID_COLORX_HOTPINK_HELP}, {RID_COLORX_LAVENDER, RID_COLORX_LAVENDER_HELP},
    {RID_COLORX_SNOW, RID_COLORX_SNOW_HELP},
};

const SmElementDescr SmElementsControl::m_a5BracketsList[] =
{
    {RID_LRGROUPX, RID_LRGROUPX_HELP},
    {nullptr, nullptr},
    {RID_LRPARENTX, RID_LRPARENTX_HELP}, {RID_LRBRACKETX, RID_LRBRACKETX_HELP},
    {RID_LRDBRACKETX, RID_LRDBRACKETX_HELP}, {RID_LRBRACEX, RID_LRBRACEX_HELP},
    {RID_LRANGLEX, RID_LRANGLEX_HELP}, {RID_LMRANGLEXY, RID_LMRANGLEXY_HELP},
    {RID_LRCEILX, RID_LRCEILX_HELP}, {RID_LRFLOORX, RID_LRFLOORX_HELP},
    {RID_LRLINEX, RID_LRLINEX_HELP}, {RID_LRDLINEX, RID_LRDLINEX_HELP},
    {nullptr, nullptr},
    {RID_SLRPARENTX, RID_SLRPARENTX_HELP}, {RID_SLRBRACKETX, RID_SLRBRACKETX_HELP},
    {RID_SLRDBRACKETX, RID_SLRDBRACKETX_HELP}, {RID_SLRBRACEX, RID_SLRBRACEX_HELP},
    {RID_SLRANGLEX, RID_SLRANGLEX_HELP}, {RID_SLMRANGLEXY, RID_SLMRANGLEXY_HELP},
    {RID_SLRCEILX, RID_SLRCEILX_HELP}, {RID_SLRFLOORX, RID_SLRFLOORX_HELP},
    {RID_SLRLINEX, RID_SLRLINEX_HELP}, {RID_SLRDLINEX, RID_SLRDLINEX_HELP},
    {nullptr, nullptr},
    {RID_XOVERBRACEY, RID_XOVERBRACEY_HELP}, {RID_XUNDERBRACEY, RID_XUNDERBRACEY_HELP},
    {nullptr, nullptr},
    {RID_EVALX, RID_EVALUATEX_HELP}, {RID_EVAL_FROMX, RID_EVALUATE_FROMX_HELP},
    {RID_EVAL_TOX, RID_EVALUATE_TOX_HELP}, {RID_EVAL_FROMTOX, RID_EVALUATE_FROMTOX_HELP},
};

const SmElementDescr SmElementsControl::m_a5FormatsList[] =
{
    {RID_RSUPX, RID_RSUPX_HELP}, {RID_RSUBX, RID_RSUBX_HELP}, {RID_LSUPX, RID_LSUPX_HELP},
    {RID_LSUBX, RID_LSUBX_HELP}, {RID_CSUPX, RID_CSUPX_HELP}, {RID_CSUBX, RID_CSUBX_HELP},
    {nullptr, nullptr},
    {RID_NEWLINE, RID_NEWLINE_HELP}, {RID_SBLANK, RID_SBLANK_HELP}, {RID_BLANK, RID_BLANK_HELP},
    {RID_NOSPACE, RID_NOSPACE_HELP},
    {RID_ALIGNLX, RID_ALIGNLX_HELP}, {RID_ALIGNCX, RID_ALIGNCX_HELP}, {RID_ALIGNRX, RID_ALIGNRX_HELP},
    {nullptr, nullptr},
    {RID_BINOMXY, RID_BINOMXY_HELP}, {RID_STACK, RID_STACK_HELP},
    {RID_MATRIX, RID_MATRIX_HELP},
};

const SmElementDescr SmElementsControl::m_a5OthersList[] =
{
    {RID_INFINITY, RID_INFINITY_HELP}, {RID_PARTIAL, RID_PARTIAL_HELP}, {RID_NABLA, RID_NABLA_HELP},
    {RID_EXISTS, RID_EXISTS_HELP}, {RID_NOTEXISTS, RID_NOTEXISTS_HELP}, {RID_FORALL, RID_FORALL_HELP},
    {RID_HBAR, RID_HBAR_HELP}, {RID_LAMBDABAR, RID_LAMBDABAR_HELP}, {RID_RE, RID_RE_HELP},
    {RID_IM, RID_IM_HELP}, {RID_WP, RID_WP_HELP}, {RID_LAPLACE, RID_LAPLACE_HELP},
    {RID_FOURIER, RID_FOURIER_HELP}, {RID_BACKEPSILON, RID_BACKEPSILON_HELP},
    {nullptr, nullptr},
    {RID_LEFTARROW, RID_LEFTARROW_HELP}, {RID_RIGHTARROW, RID_RIGHTARROW_HELP}, {RID_UPARROW, RID_UPARROW_HELP},
    {RID_DOWNARROW, RID_DOWNARROW_HELP},
    {nullptr, nullptr},
    {RID_DOTSLOW, RID_DOTSLOW_HELP}, {RID_DOTSAXIS, RID_DOTSAXIS_HELP}, {RID_DOTSVERT, RID_DOTSVERT_HELP},
    {RID_DOTSUP, RID_DOTSUP_HELP}, {RID_DOTSDOWN, RID_DOTSDOWN_HELP}
};

const SmElementDescr SmElementsControl::m_a5ExamplesList[] =
{
    {"{func e}^{i %pi} + 1 = 0", RID_EXAMPLE_EULER_IDENTITY_HELP},
    {"C = %pi cdot d = 2 cdot %pi cdot r", RID_EXAMPLE_CIRCUMFERENCE_HELP},
    {"c = sqrt{ a^2 + b^2 }", RID_EXAMPLE_PYTHAGOREAN_THEO_HELP},
    {"vec F = m times vec a", RID_EXAMPLE_2NEWTON},
    {"E = m c^2", RID_EXAMPLE_MASS_ENERGY_EQUIV_HELP},
    {"G_{%mu %nu} + %LAMBDA g_{%mu %nu}= frac{8 %pi G}{c^4} T_{%mu %nu}", RID_EXAMPLE_GENERAL_RELATIVITY_HELP},
    {"%DELTA t' = { %DELTA t } over sqrt{ 1 - v^2 over c^2 }", RID_EXAMPLE_SPECIAL_RELATIVITY_HELP },
    {"d over dt left( {partial L}over{partial dot q} right) = {partial L}over{partial q}", RID_EXAMPLE_EULER_LAGRANGE_HELP },
    {"int from a to b f'(x) dx = f(b) - f(a)", RID_EXAMPLE_FTC_HELP},
    {"ldline %delta bold{r}(t) rdline approx e^{%lambda t} ldline %delta { bold{r} }_0 rdline", RID_EXAMPLE_CHAOS_HELP},
    {"f(x) = sum from { n=0 } to { infinity } { {f^{(n)}(x_0) } over { fact{n} } (x-x_0)^n }", RID_EXAMPLE_A_TAYLOR_SERIES_HELP},
    {"f(x) = {1} over { %sigma sqrt{2 %pi} } func e^-{ {(x-%mu)^2} over {2 %sigma^2} }", RID_EXAMPLE_GAUSS_DISTRIBUTION_HELP},
};

#define AS_PAIR(a) a, SAL_N_ELEMENTS(a)
const std::tuple<const char*, const SmElementDescr*, size_t> SmElementsControl::m_a5Categories[] =
{
    {RID_CATEGORY_UNARY_BINARY_OPERATORS, AS_PAIR(m_a5UnaryBinaryOperatorsList)},
    {RID_CATEGORY_RELATIONS, AS_PAIR(m_a5RelationsList)},
    {RID_CATEGORY_SET_OPERATIONS, AS_PAIR(m_a5SetOperationsList)},
    {RID_CATEGORY_FUNCTIONS, AS_PAIR(m_a5FunctionsList)},
    {RID_CATEGORY_OPERATORS, AS_PAIR(m_a5OperatorsList)},
    {RID_CATEGORY_ATTRIBUTES, AS_PAIR(m_a5AttributesList)},
    {RID_CATEGORY_BRACKETS, AS_PAIR(m_a5BracketsList)},
    {RID_CATEGORY_FORMATS, AS_PAIR(m_a5FormatsList)},
    {RID_CATEGORY_OTHERS, AS_PAIR(m_a5OthersList)},
    {RID_CATEGORY_EXAMPLES, AS_PAIR(m_a5ExamplesList)},
};

const size_t SmElementsControl::m_a5CategoriesSize = SAL_N_ELEMENTS(m_a5Categories);

SmElementsControl::SmElementsControl(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
    : mpDocShell(new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT))
    , m_nCurrentElement(SAL_MAX_UINT16)
    , m_nCurrentRolloverElement(SAL_MAX_UINT16)
    , m_nCurrentOffset(0)
    , m_nSmSyntaxVersion(SM_MOD()->GetConfig()->GetDefaultSmSyntaxVersion())
    , mbVerticalMode(true)
    , mxScroll(std::move(xScrolledWindow))
    , m_bFirstPaintAfterLayout(false)
{
    mxScroll->connect_hadjustment_changed( LINK(this, SmElementsControl, ScrollHdl) );
    mxScroll->connect_vadjustment_changed( LINK(this, SmElementsControl, ScrollHdl) );
    maParser.reset(starmathdatabase::GetVersionSmParser(m_nSmSyntaxVersion));
    maParser->SetImportSymbolNames(true);
}

SmElementsControl::~SmElementsControl()
{
    mpDocShell->DoClose();
}

void SmElementsControl::setVerticalMode(bool bVerticalMode)
{
    if (mbVerticalMode == bVerticalMode)
        return;
    mbVerticalMode = bVerticalMode;
    // turn off scrollbars, LayoutOrPaintContents will enable whichever one
    // might be needed
    mxScroll->set_vpolicy(VclPolicyType::NEVER);
    mxScroll->set_hpolicy(VclPolicyType::NEVER);
    LayoutOrPaintContents(GetDrawingArea()->get_ref_device(), false);
    Invalidate();
}

SmElement* SmElementsControl::current() const
{
    sal_uInt16 nCur = (m_nCurrentRolloverElement != SAL_MAX_UINT16)
            ? m_nCurrentRolloverElement
            : (HasFocus() ? m_nCurrentElement : SAL_MAX_UINT16);
    return (nCur < maElementList.size()) ? maElementList[nCur].get() : nullptr;
}

void SmElementsControl::setCurrentElement(sal_uInt16 nPos)
{
    if (m_nCurrentElement == nPos)
        return;
    if (nPos != SAL_MAX_UINT16 && nPos >= maElementList.size())
        return;
    if (m_xAccessible.is() && m_nCurrentElement != SAL_MAX_UINT16)
        m_xAccessible->ReleaseFocus(m_nCurrentElement);
    m_nCurrentElement = nPos;
    if (m_xAccessible.is() && m_nCurrentElement != SAL_MAX_UINT16)
        m_xAccessible->AcquireFocus();
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

/**
 * !bDraw => layout only
 *
 * Layouting is always done without a scrollbar and will show or hide it.
 * The first paint (m_bFirstPaintAfterLayout) therefore needs to update a
 * visible scrollbar, because the layouting was wrong.
 **/
void SmElementsControl::LayoutOrPaintContents(vcl::RenderContext& rContext, bool bDraw)
{
    rContext.Push();

    rContext.SetMapMode( MapMode(MapUnit::Map100thMM) );
    rContext.SetDrawMode( DrawModeFlags::Default );
    rContext.SetLayoutMode( ComplexTextLayoutFlags::Default );
    rContext.SetDigitLanguage( LANGUAGE_ENGLISH );
    if (bDraw)
    {
        rContext.SetBackground(GetControlBackground());
        rContext.SetTextColor(GetTextColor());
        rContext.Erase();
    }

    const sal_Int32 nControlHeight = GetOutputSizePixel().Height();
    const sal_Int32 nControlWidth = GetOutputSizePixel().Width();

    sal_Int32 boxX = maMaxElementDimensions.Width()  + 10;
    sal_Int32 boxY = maMaxElementDimensions.Height() + 10;

    sal_Int32 x = mbVerticalMode ? -mxScroll->hadjustment_get_value() : 0;
    sal_Int32 y = !mbVerticalMode ? -mxScroll->vadjustment_get_value() : 0;

    sal_Int32 perLine = 0;

    if (mbVerticalMode)
        perLine = nControlHeight / boxY;
    else
        perLine = nControlWidth / boxX;
    if (perLine <= 0)
        perLine = 1;

    if (mbVerticalMode)
        boxY = nControlHeight / perLine;
    else
        boxX = nControlWidth / perLine;

    const SmElement* pCurrentElement = current();
    for (const std::unique_ptr<SmElement> & i : maElementList)
    {
        SmElement* element = i.get();
        if (element->isSeparator())
        {
            if (mbVerticalMode)
            {
                x += boxX;
                y = 0;

                element->mBoxLocation = Point(x, y);
                element->mBoxSize = Size(10, nControlHeight);

                tools::Rectangle aSelectionRectangle(x + 5 - 1, y + 5,
                                              x + 5 + 1, nControlHeight - 5);

                if (bDraw)
                    rContext.DrawRect(rContext.PixelToLogic(aSelectionRectangle));
                x += 10;
            }
            else
            {
                x = 0;
                y += boxY;

                element->mBoxLocation = Point(x, y);
                element->mBoxSize = Size(nControlWidth, 10);

                tools::Rectangle aSelectionRectangle(x + 5, y + 5 - 1,
                                              nControlWidth - 5, y + 5 + 1);

                if (bDraw)
                    rContext.DrawRect(rContext.PixelToLogic(aSelectionRectangle));
                y += 10;
            }
        }
        else
        {
            if (mbVerticalMode)
            {
                if (y + boxY > nControlHeight)
                {
                    x += boxX;
                    y = 0;
                }
            }
            else
            {
                if ( x + boxX > nControlWidth)
                {
                    x = 0;
                    y += boxY;
                }
            }

            element->mBoxLocation = Point(x,y);
            element->mBoxSize = Size(boxX, boxY);

            if (bDraw)
            {
                if (pCurrentElement == element)
                {
                    rContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
                    const StyleSettings& rStyleSettings = rContext.GetSettings().GetStyleSettings();
                    rContext.SetLineColor(rStyleSettings.GetHighlightColor());
                    rContext.SetFillColor(COL_TRANSPARENT);
                    rContext.DrawRect(rContext.PixelToLogic(tools::Rectangle(x + 1, y + 1, x + boxX - 1, y + boxY - 1)));
                    rContext.DrawRect(rContext.PixelToLogic(tools::Rectangle(x + 2, y + 2, x + boxX - 2, y + boxY - 2)));
                    rContext.Pop();
                }

                Size aSizePixel = rContext.LogicToPixel(Size(element->getNode()->GetWidth(),
                                                    element->getNode()->GetHeight()));
                Point location(x + ((boxX - aSizePixel.Width()) / 2),
                               y + ((boxY - aSizePixel.Height()) / 2));
                SmDrawingVisitor(rContext, rContext.PixelToLogic(location), element->getNode().get());
            }

            if (mbVerticalMode)
                y += boxY;
            else
                x += boxX;
        }
    }

    if (bDraw)
    {
        if (!m_bFirstPaintAfterLayout)
        {
            rContext.Pop();
            return;
        }
        m_bFirstPaintAfterLayout = false;
    }
    else
        m_bFirstPaintAfterLayout = true;

    if (mbVerticalMode)
    {
        sal_Int32 nTotalControlWidth = x + boxX + mxScroll->hadjustment_get_value();
        if (nTotalControlWidth > GetOutputSizePixel().Width())
        {
            mxScroll->hadjustment_set_upper(nTotalControlWidth);
            mxScroll->hadjustment_set_page_size(nControlWidth);
            mxScroll->hadjustment_set_page_increment(nControlWidth);
            mxScroll->set_hpolicy(VclPolicyType::ALWAYS);
        }
        else
        {
            mxScroll->hadjustment_set_value(0);
            mxScroll->set_hpolicy(VclPolicyType::NEVER);
        }
    }
    else
    {
        sal_Int32 nTotalControlHeight = y + boxY + mxScroll->vadjustment_get_value();
        if (nTotalControlHeight > GetOutputSizePixel().Height())
        {
            mxScroll->vadjustment_set_upper(nTotalControlHeight);
            mxScroll->vadjustment_set_page_size(nControlHeight);
            mxScroll->vadjustment_set_page_increment(nControlHeight);
            mxScroll->set_vpolicy(VclPolicyType::ALWAYS);
        }
        else
        {
            mxScroll->vadjustment_set_value(0);
            mxScroll->set_vpolicy(VclPolicyType::NEVER);
        }
    }
    rContext.Pop();
}

void SmElementsControl::Resize()
{
    CustomWidgetController::Resize();
    LayoutOrPaintContents(GetDrawingArea()->get_ref_device(), false);
}

void SmElementsControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    LayoutOrPaintContents(rRenderContext, true);
}

OUString SmElementsControl::RequestHelp(tools::Rectangle& rRect)
{
    if (!hasRollover())
        return OUString();

    const SmElement* pHelpElement = current();
    if (!pHelpElement)
        return OUString();

    rRect = tools::Rectangle(pHelpElement->mBoxLocation, pHelpElement->mBoxSize);

    // get text and display it
    return pHelpElement->getHelpText();
}

bool SmElementsControl::MouseMove( const MouseEvent& rMouseEvent )
{
    if (rMouseEvent.IsLeaveWindow())
    {
        m_nCurrentRolloverElement = SAL_MAX_UINT16;
        Invalidate();
        return false;
    }

    if (tools::Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()))
    {
        const SmElement* pPrevElement = current();
        if (pPrevElement)
        {
            const tools::Rectangle rect(pPrevElement->mBoxLocation, pPrevElement->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
                return true;
        }

        const sal_uInt16 nElementCount = maElementList.size();
        for (sal_uInt16 n = 0; n < nElementCount; n++)
        {
            const SmElement* element = maElementList[n].get();
            if (pPrevElement == element)
                continue;

            const tools::Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                m_nCurrentRolloverElement = n;
                Invalidate();
                return true;
            }
        }
        if (pPrevElement && hasRollover())
            Invalidate();
        m_nCurrentRolloverElement = SAL_MAX_UINT16;
        return true;
    }

    return false;
}

namespace {

void collectUIInformation(const OUString& aID)
{
    EventDescription aDescription;
    aDescription.aID = aID;
    aDescription.aParent = "element_selector";
    aDescription.aAction = "SELECT";
    aDescription.aKeyWord = "ElementUIObject";
    UITestLogger::getInstance().logEvent(aDescription);
}

}

bool SmElementsControl::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    GrabFocus();

    if (rMouseEvent.IsLeft() && tools::Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()) && maSelectHdlLink.IsSet())
    {
        const SmElement* pPrevElement = hasRollover() ? current() : nullptr;
        if (pPrevElement)
        {
            tools::Rectangle rect(pPrevElement->mBoxLocation, pPrevElement->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                setCurrentElement(m_nCurrentRolloverElement);
                maSelectHdlLink.Call(*const_cast<SmElement*>(pPrevElement));
                collectUIInformation(OUString::number(m_nCurrentRolloverElement));
                return true;
            }
        }

        const sal_uInt16 nElementCount = maElementList.size();
        for (sal_uInt16 n = 0; n < nElementCount; n++)
        {
            SmElement* element = maElementList[n].get();
            tools::Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                setCurrentElement(n);
                maSelectHdlLink.Call(*element);
                collectUIInformation(OUString::number(n));
                return true;
            }
        }

        return true;
    }
    return false;
}

void SmElementsControl::GetFocus()
{
    CustomWidgetController::GetFocus();
    Invalidate();
    if (m_xAccessible.is() && m_nCurrentElement != SAL_MAX_UINT16)
        m_xAccessible->AcquireFocus();
}

void SmElementsControl::LoseFocus()
{
    if (m_xAccessible.is() && m_nCurrentElement != SAL_MAX_UINT16)
        m_xAccessible->ReleaseFocus(m_nCurrentElement);
    CustomWidgetController::LoseFocus();
    Invalidate();
}

sal_uInt16 SmElementsControl::nextElement(const bool bBackward, const sal_uInt16 nStartPos, const sal_uInt16 nLastElement)
{
    sal_uInt16 nPos = nStartPos;

    while (true)
    {
        if (bBackward)
        {
            if (nPos == 0)
                break;
            nPos--;
        }
        else
        {
            if (nPos == nLastElement)
                break;
            nPos++;
        }

        if (nStartPos == nPos)
            break;
        if (!maElementList[nPos]->isSeparator())
            break;
    }

    return nPos;
}

void SmElementsControl::scrollToElement(const bool bBackward, const SmElement *pCur)
{
    if (mbVerticalMode)
    {
        auto nScrollPos = mxScroll->hadjustment_get_value();
        nScrollPos += pCur->mBoxLocation.X();
        if (!bBackward)
            nScrollPos += pCur->mBoxSize.Width() - GetOutputSizePixel().Width();
        mxScroll->hadjustment_set_value(nScrollPos);
    }
    else
    {
        auto nScrollPos = mxScroll->vadjustment_get_value();
        nScrollPos += pCur->mBoxLocation.Y();
        if (!bBackward)
            nScrollPos += pCur->mBoxSize.Height() - GetOutputSizePixel().Height();
        mxScroll->vadjustment_set_value(nScrollPos);
    }
}

void SmElementsControl::stepFocus(const bool bBackward)
{
    const sal_uInt16 nStartPos = m_nCurrentElement;
    const sal_uInt16 nLastElement = (maElementList.size() ? maElementList.size() - 1 : 0);
    assert(nStartPos <= nLastElement);

    sal_uInt16 nPos = nextElement(bBackward, nStartPos, nLastElement);
    if (nStartPos != nPos)
    {
        m_nCurrentRolloverElement = SAL_MAX_UINT16;
        setCurrentElement(nPos);

        const tools::Rectangle outputRect(Point(0,0), GetOutputSizePixel());
        const SmElement *pCur = maElementList[nPos].get();
        tools::Rectangle elementRect(pCur->mBoxLocation, pCur->mBoxSize);
        if (!outputRect.IsInside(elementRect))
            scrollToElement(bBackward, pCur);
        Invalidate();
    }
}

void SmElementsControl::pageFocus(const bool bBackward)
{
    const sal_uInt16 nStartPos = m_nCurrentElement;
    const sal_uInt16 nLastElement = (maElementList.size() ? maElementList.size() - 1 : 0);
    assert(nStartPos <= nLastElement);
    tools::Rectangle outputRect(Point(0,0), GetOutputSizePixel());
    sal_uInt16 nPrevPos = nStartPos;
    sal_uInt16 nPos = nPrevPos;

    bool bMoved = false;
    while (true)
    {
        nPrevPos = nPos;
        nPos = nextElement(bBackward, nPrevPos, nLastElement);
        if (nPrevPos == nPos)
            break;

        m_nCurrentRolloverElement = SAL_MAX_UINT16;

        SmElement *pCur = maElementList[nPos].get();
        tools::Rectangle elementRect(pCur->mBoxLocation, pCur->mBoxSize);
        if (!outputRect.IsInside(elementRect))
        {
            if (nPrevPos != nStartPos)
            {
                nPos = nPrevPos;
                break;
            }
            if (bMoved)
                break;
            pCur = maElementList[nPrevPos].get();

            elementRect = tools::Rectangle(pCur->mBoxLocation, pCur->mBoxSize);
            if (mbVerticalMode)
                outputRect.Move(bBackward ? -outputRect.GetWidth() + elementRect.Right() : elementRect.Left(), 0);
            else
                outputRect.Move(0, bBackward ? -outputRect.GetHeight() + elementRect.Bottom() : elementRect.Top());
            bMoved = true;
        }
    }

    if (nStartPos != nPos)
    {
        setCurrentElement(nPos);
        if (bMoved)
            scrollToElement(bBackward, maElementList[nPos].get());
        Invalidate();
    }
}

bool SmElementsControl::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if (aKeyCode.GetModifier())
    {
        return false;
    }

    switch(aKeyCode.GetCode())
    {
        case KEY_RETURN:
            [[fallthrough]];
        case KEY_SPACE:
            assert(m_nCurrentElement < maElementList.size());
            assert(maSelectHdlLink.IsSet());
            maSelectHdlLink.Call(*maElementList[m_nCurrentElement]);
            collectUIInformation(OUString::number(m_nCurrentElement));
            break;

        case KEY_DOWN:
            [[fallthrough]];
        case KEY_RIGHT:
            stepFocus(false);
            break;

        case KEY_LEFT:
            [[fallthrough]];
        case KEY_UP:
            stepFocus(true);
            break;

        case KEY_HOME:
            if (!maElementList.empty())
            {
                setCurrentElement(0);
                mxScroll->vadjustment_set_value(0);
            }
            break;
        case KEY_END:
            if (!maElementList.empty())
            {
                setCurrentElement(maElementList.size() - 1);
                mxScroll->vadjustment_set_value(mxScroll->vadjustment_get_upper());
            }
            break;

        case KEY_PAGEUP:
            pageFocus(true);
            break;
        case KEY_PAGEDOWN:
            pageFocus(false);
            break;

        default:
            return false;
    }
    return true;
}

IMPL_LINK_NOARG( SmElementsControl, ScrollHdl, weld::ScrolledWindow&, void )
{
    Invalidate();
}

void SmElementsControl::addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText)
{
    // SAL_MAX_UINT16 is invalid, zero is the scrollbar
    assert(maElementList.size() < SAL_MAX_UINT16 - 2);
    auto pNode = maParser->ParseExpression(aElementVisual);

    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    rDevice.Push(PushFlags::MAPMODE);
    rDevice.SetMapMode( MapMode(MapUnit::Map100thMM) );

    pNode->Prepare(maFormat, *mpDocShell, 0);
    pNode->SetSize(Fraction(10,8));
    pNode->Arrange(rDevice, maFormat);

    Size aSizePixel = rDevice.LogicToPixel(Size(pNode->GetWidth(), pNode->GetHeight()), MapMode(MapUnit::Map100thMM));
    if (aSizePixel.Width() > maMaxElementDimensions.Width()) {
        maMaxElementDimensions.setWidth( aSizePixel.Width() );
    }

    if (aSizePixel.Height() > maMaxElementDimensions.Height()) {
        maMaxElementDimensions.setHeight( aSizePixel.Height() );
    }

    maElementList.push_back(std::make_unique<SmElement>(std::move(pNode), aElementSource, aHelpText));

    rDevice.Pop();
}

void SmElementsControl::setElementSetId(const char* pSetId)
{
    if (msCurrentSetId == pSetId)
        return;
    msCurrentSetId = pSetId;
    maMaxElementDimensions = Size();
    build();
}

void SmElementsControl::addElements(const SmElementDescr aElementsArray[], sal_uInt16 aElementsArraySize)
{
    for (sal_uInt16 i = 0; i < aElementsArraySize ; i++)
    {
        const char* pElement = aElementsArray[i].first;
        const char* pElementHelp = aElementsArray[i].second;
        if (!pElement) {
            maElementList.push_back(std::make_unique<SmElementSeparator>());
        } else {
            OUString aElement(OUString::createFromAscii(pElement));
            if (aElement == RID_NEWLINE)
                addElement(OUString(u"\u21B5"), aElement, SmResId(pElementHelp));
            else if (aElement == RID_SBLANK)
                addElement("\"`\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_BLANK)
                addElement("\"~\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_PHANTOMX)
                addElement("\"" + SmResId(STR_HIDE) +"\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_BOLDX)
                addElement("bold B", aElement, SmResId(pElementHelp));
            else if (aElement == RID_ITALX)
                addElement("ital I", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SIZEXY)
                addElement("\"" + SmResId(STR_SIZE) + "\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_FONTXY)
                addElement("\"" + SmResId(STR_FONT) + "\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_BLACK)
                addElement("color black { \"" + SmResId(STR_BLACK) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_BLUE)
                addElement("color blue { \"" + SmResId(STR_BLUE) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_GREEN)
                addElement("color green { \"" + SmResId(STR_GREEN) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_RED)
                addElement("color red { \"" + SmResId(STR_RED) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_AQUA)
                addElement("color aqua { \"" + SmResId(STR_AQUA) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_FUCHSIA)
                addElement("color fuchsia { \"" + SmResId(STR_FUCHSIA) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_YELLOW)
                addElement("color yellow { \"" + SmResId(STR_YELLOW) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_GRAY)
                addElement("color gray { \"" + SmResId(STR_GRAY) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_LIME)
                addElement("color lime { \"" + SmResId(STR_LIME) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_MAROON)
                addElement("color maroon { \"" + SmResId(STR_MAROON) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_NAVY)
                addElement("color navy { \"" + SmResId(STR_NAVY) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_OLIVE)
                addElement("color olive { \"" + SmResId(STR_OLIVE) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_PURPLE)
                addElement("color purple { \"" + SmResId(STR_PURPLE) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_SILVER)
                addElement("color silver { \"" + SmResId(STR_SILVER) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_TEAL)
                addElement("color teal { \"" + SmResId(STR_TEAL) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_CORAL)
                addElement("color coral { \"" + SmResId(STR_CORAL) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_CRIMSON)
                addElement("color crimson { \"" + SmResId(STR_CRIMSON) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_MIDNIGHT)
                addElement("color midnightblue { \"" + SmResId(STR_MIDNIGHT) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_VIOLET)
                addElement("color violet { \"" + SmResId(STR_VIOLET) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_ORANGE)
                addElement("color orange { \"" + SmResId(STR_ORANGE) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_ORANGERED)
                addElement("color orangered { \"" + SmResId(STR_ORANGERED) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_SEAGREEN)
                addElement("color seagreen { \"" + SmResId(STR_SEAGREEN) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_INDIGO)
                addElement("color indigo { \"" + SmResId(STR_INDIGO) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_HOTPINK)
                addElement("color hotpink { \"" + SmResId(STR_HOTPINK) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_SNOW)
                addElement("color snow { \"" + SmResId(STR_SNOW) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_LAVENDER)
                addElement("color lavender { \"" + SmResId(STR_LAVENDER) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_RGB)
                addElement("color rgb 0 0 0 { \"" + SmResId(STR_RGB) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_RGBA)
                addElement("color rgba 0 0 0 0 { \"" + SmResId(STR_RGBA) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_HEX)
                addElement("color hex 000000 { \"" + SmResId(STR_HEX) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_ALIGNLX)
                addElement("\"" + SmResId(STR_ALIGN_LEFT) + "\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_ALIGNCX)
                addElement("\"" + SmResId(STR_ALIGN_CENTER) + "\"", aElement, SmResId(pElementHelp));
            else if (aElement == RID_ALIGNRX)
                addElement("\"" + SmResId(STR_ALIGN_RIGHT) + "\"", aElement, SmResId(pElementHelp));

            else if (aElement == RID_SLRPARENTX)
                addElement("left ( binom{<?>}{<?>} right ) ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRBRACKETX)
                addElement("left [ binom{<?>}{<?>} right ] ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRDBRACKETX)
                addElement("left ldbracket binom{<?>}{<?>} right rdbracket ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRBRACEX)
                addElement("left lbrace binom{<?>}{<?>} right rbrace ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRANGLEX)
                addElement("left langle binom{<?>}{<?>} right rangle ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRCEILX)
                addElement("left lceil binom{<?>}{<?>} right rceil ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRFLOORX)
                addElement("left lfloor binom{<?>}{<?>} right rfloor ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRLINEX)
                addElement("left lline binom{<?>}{<?>} right rline ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLRDLINEX)
                addElement("left ldline binom{<?>}{<?>} right rdline ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_SLMRANGLEXY)
                addElement("left langle binom{<?>}{<?>} mline binom{<?>}{<?>} right rangle ", aElement, SmResId(pElementHelp));

            else if (aElement == RID_OPERX)
                addElement(u"oper \xE22B <?>", aElement, SmResId(pElementHelp));
            else if (aElement == RID_OPER_FROMX)
                addElement(u"oper \xE22B from <?> <?>", aElement, SmResId(pElementHelp));
            else if (aElement == RID_OPER_TOX)
                addElement(u"oper \xE22B to <?> <?>", aElement, SmResId(pElementHelp));
            else if (aElement == RID_OPER_FROMTOX)
                addElement(u"oper \xE22B from <?> to <?> <?>", aElement, SmResId(pElementHelp));

            else if (aElement == RID_XOVERBRACEY)
                addElement("{<?><?><?>} overbrace {<?>} ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_XUNDERBRACEY)
                addElement("{<?><?><?>} underbrace {<?>} ", aElement, SmResId(pElementHelp));
            else
                addElement(aElement, aElement, pElementHelp ? SmResId(pElementHelp) : "");
        }
    }
}

void SmElementsControl::build()
{
    // The order is important!
    // 1. Ensure there are no items left
    // 2. Release all the current accessible items.
    //    This will check for new items after releasing them!
    // 3. Set the cursor element
    maElementList.clear();
    mxScroll->hadjustment_set_value(0);
    mxScroll->vadjustment_set_value(0);
    mxScroll->set_hpolicy(VclPolicyType::NEVER);
    mxScroll->set_vpolicy(VclPolicyType::NEVER);

    if (m_xAccessible.is())
        m_xAccessible->ReleaseAllItems();

    setCurrentElement(SAL_MAX_UINT16);

    m_nCurrentOffset = 0;

    switch(m_nSmSyntaxVersion)
    {
        case 5:
        {
            for (sal_uInt16 n = 0; n < m_a5CategoriesSize; ++n)
            {
                if (msCurrentSetId == std::get<0>(m_a5Categories[n]))
                {
                    addElements(std::get<1>(m_a5Categories[n]), std::get<2>(m_a5Categories[n]));
                    break;
                }
                else
                    m_nCurrentOffset += std::get<2>(m_a5Categories[n]);
            }
            break;
        }
        case 6:
        default:
            throw std::range_error("parser version limit");
    }

    m_nCurrentRolloverElement = SAL_MAX_UINT16;
    LayoutOrPaintContents(GetDrawingArea()->get_ref_device(), false);

    if (m_xAccessible.is())
        m_xAccessible->AddAllItems();

    setCurrentElement(0);
    Invalidate();
}

void SmElementsControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    maFormat.SetBaseSize(rDevice.PixelToLogic(Size(0, SmPtsTo100th_mm(12))));
    Size aSize(rDevice.LogicToPixel(Size(10, 100), MapMode(MapUnit::MapAppFont)));
    // give it an arbitrary small width request so it can shrink in the sidebar
    pDrawingArea->set_size_request(42, aSize.Height());
    SetOutputSizePixel(aSize);
}

FactoryFunction SmElementsControl::GetUITestFactory() const
{
    return ElementSelectorUIObject::create;
}

bool SmElementsControl::itemIsSeparator(sal_uInt16 nPos) const
{
    if (nPos < m_nCurrentOffset)
        return true;
    nPos -= m_nCurrentOffset;
    if (nPos >= maElementList.size())
        return true;
    return maElementList[nPos]->isSeparator();
}

css::uno::Reference<css::accessibility::XAccessible> SmElementsControl::CreateAccessible()
{
    if (!m_xAccessible.is())
    {
        m_xAccessible = new AccessibleSmElementsControl(*this);
        m_xAccessible->AddAllItems();
    }
    return m_xAccessible;
}

bool SmElementsControl::itemTrigger(sal_uInt16 nPos)
{
    if (nPos < m_nCurrentOffset)
        return false;
    nPos -= m_nCurrentOffset;
    if (nPos >= maElementList.size())
        return false;

    maSelectHdlLink.Call(*maElementList[nPos]);
    collectUIInformation(OUString::number(nPos));
    return true;
}

tools::Rectangle SmElementsControl::itemPosRect(sal_uInt16 nPos) const
{
    if (nPos < m_nCurrentOffset)
        return tools::Rectangle();
    nPos -= m_nCurrentOffset;
    if (nPos >= maElementList.size())
        return tools::Rectangle();

    SmElement* pItem = maElementList[nPos].get();
    return tools::Rectangle(pItem->mBoxLocation, pItem->mBoxSize);
}

bool SmElementsControl::itemIsVisible(sal_uInt16 nPos) const
{
    tools::Rectangle elementRect = itemPosRect(nPos);
    if (elementRect.IsEmpty())
        return false;

    tools::Rectangle outputRect(Point(0, 0), GetOutputSizePixel());
    return outputRect.IsInside(elementRect);
}

sal_uInt16 SmElementsControl::itemCount() const { return maElementList.size(); }

sal_uInt16 SmElementsControl::itemHighlighted() const { return m_nCurrentElement; }

void SmElementsControl::setItemHighlighted(sal_uInt16 nPos)
{
    if (m_nCurrentRolloverElement == nPos)
        return;
    if (nPos != SAL_MAX_UINT16 && nPos >= maElementList.size())
        return;

    if (maElementList[nPos]->isSeparator())
        m_nCurrentRolloverElement = SAL_MAX_UINT16;
    else
        m_nCurrentRolloverElement = nPos;
    Invalidate();
}

void SmElementsControl::setSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion)
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

OUString SmElementsControl::itemName(sal_uInt16 nPos) const
{
    if (nPos < m_nCurrentOffset)
        return OUString();
    nPos -= m_nCurrentOffset;
    if (nPos >= maElementList.size())
        return OUString();

    return maElementList[nPos]->getHelpText();
}

sal_uInt16 SmElementsControl::itemAtPos(const Point& rPoint) const
{
    sal_uInt16 nElementCount = maElementList.size();
    for (sal_uInt16 n = 0; n < nElementCount; n++)
    {
        const SmElement* pItem = maElementList[n].get();
        tools::Rectangle elementRect(pItem->mBoxLocation, pItem->mBoxSize);
        if (elementRect.IsInside(rPoint))
            return n;
    }
    return SAL_MAX_UINT16;
}

SmElementsDockingWindow::SmElementsDockingWindow(SfxBindings* pInputBindings, SfxChildWindow* pChildWindow, vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DockingElements",
        "modules/smath/ui/dockingelements.ui")
    , mxElementsControl(new SmElementsControl(m_xBuilder->weld_scrolled_window("scrolledwindow", true)))
    , mxElementsControlWin(new weld::CustomWeld(*m_xBuilder, "element_selector", *mxElementsControl))
    , mxElementListBox(m_xBuilder->weld_combo_box("listbox"))
{
    // give it an arbitrary small width request so it can shrink in the sidebar
    mxElementListBox->set_size_request(42, -1);

    for (size_t i = 0; i < SmElementsControl::categoriesSize(); ++i)
        mxElementListBox->append_text(SmResId(std::get<0>(SmElementsControl::categories()[i])));

    mxElementListBox->connect_changed(LINK(this, SmElementsDockingWindow, ElementSelectedHandle));
    mxElementListBox->set_active_text(SmResId(RID_CATEGORY_UNARY_BINARY_OPERATORS));

    mxElementsControl->setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);
    mxElementsControl->SetSelectHdl(LINK(this, SmElementsDockingWindow, SelectClickHandler));
}

void SmElementsDockingWindow::GetFocus()
{
    SfxDockingWindow::GetFocus();
    if (mxElementListBox)
        mxElementListBox->grab_focus();
}

SmElementsDockingWindow::~SmElementsDockingWindow ()
{
    disposeOnce();
}

void SmElementsDockingWindow::dispose()
{
    mxElementsControlWin.reset();
    mxElementsControl.reset();
    mxElementListBox.reset();
    SfxDockingWindow::dispose();
}

void SmElementsDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel( Size(100, 100) );

    Invalidate();
}

void SmElementsDockingWindow::setSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion)
{
    mxElementsControl->setSmSyntaxVersion(nSmSyntaxVersion);
}

void SmElementsDockingWindow::EndDocking( const tools::Rectangle& rReactangle, bool bFloatMode)
{
    SfxDockingWindow::EndDocking(rReactangle, bFloatMode);
    bool bVertical = ( GetAlignment() == SfxChildAlignment::TOP || GetAlignment() == SfxChildAlignment::BOTTOM );
    mxElementsControl->setVerticalMode(bVertical);
}

IMPL_LINK(SmElementsDockingWindow, SelectClickHandler, SmElement&, rElement, void)
{
    SmViewShell* pViewSh = GetView();

    if (pViewSh)
    {
        std::unique_ptr<SfxStringItem> pInsertCommand = std::make_unique<SfxStringItem>(SID_INSERTCOMMANDTEXT, rElement.getText());
        pViewSh->GetViewFrame()->GetDispatcher()->ExecuteList(
            SID_INSERTCOMMANDTEXT, SfxCallMode::RECORD,
            { pInsertCommand.get() });
    }
}

IMPL_LINK( SmElementsDockingWindow, ElementSelectedHandle, weld::ComboBox&, rList, void)
{
    for (size_t i = 0; i < SmElementsControl::categoriesSize(); ++i)
    {
        const char *pCurrentCategory = std::get<0>(SmElementsControl::categories()[i]);
        OUString aCurrentCategoryString = SmResId(pCurrentCategory);
        if (aCurrentCategoryString == rList.get_active_text())
        {
            mxElementsControl->setElementSetId(pCurrentCategory);
            setSmSyntaxVersion(GetView()->GetDoc()->GetSmSyntaxVersion());
            return;
        }
    }
}

SmViewShell* SmElementsDockingWindow::GetView()
{
    SfxViewShell* pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    return  dynamic_cast<SmViewShell*>( pView);
}

void SmElementsDockingWindow::Resize()
{
    bool bVertical = ( GetAlignment() == SfxChildAlignment::TOP || GetAlignment() == SfxChildAlignment::BOTTOM );
    mxElementsControl->setVerticalMode(bVertical);

    SfxDockingWindow::Resize();
    Invalidate();
}

SFX_IMPL_DOCKINGWINDOW_WITHID(SmElementsDockingWindowWrapper, SID_ELEMENTSDOCKINGWINDOW);

SmElementsDockingWindowWrapper::SmElementsDockingWindowWrapper(
                            vcl::Window *pParentWindow, sal_uInt16 nId,
                            SfxBindings *pBindings, SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    VclPtrInstance<SmElementsDockingWindow> pDialog(pBindings, this, pParentWindow);
    SetWindow(pDialog);
    pDialog->setDeferredProperties();
    pDialog->SetPosSizePixel(Point(0, 0), Size(300, 0));
    pDialog->Show();

    SetAlignment(SfxChildAlignment::LEFT);

    pDialog->Initialize( pInfo );
}

SmElementsDockingWindowWrapper::~SmElementsDockingWindowWrapper()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
