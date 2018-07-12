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
#include <ElementsDockingWindow.hxx>

#include <starmath.hrc>
#include <strings.hrc>
#include <smmod.hxx>
#include <view.hxx>
#include <visitors.hxx>
#include <document.hxx>
#include <node.hxx>
#include "uiobject.hxx"
#include <strings.hxx>

#include <o3tl/make_unique.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

SmElement::SmElement(std::unique_ptr<SmNode>&& pNode, const OUString& aText, const OUString& aHelpText) :
    mpNode(std::move(pNode)),
    maText(aText),
    maHelpText(aHelpText)
{}

SmElement::~SmElement()
{}

const std::unique_ptr<SmNode>& SmElement::getNode()
{
    return mpNode;
}

SmElementSeparator::SmElementSeparator() :
    SmElement(std::unique_ptr<SmNode>(), OUString(), OUString())
{}

const std::pair<const char*, const char*> SmElementsControl::aUnaryBinaryOperatorsList[] =
{
    {RID_PLUSX, RID_PLUSX_HELP}, {RID_MINUSX, RID_MINUSX_HELP},
    {RID_PLUSMINUSX, RID_PLUSMINUSX_HELP}, {RID_MINUSPLUSX, RID_MINUSPLUSX_HELP},
    {nullptr, nullptr},
    {RID_XPLUSY, RID_XPLUSY_HELP}, {RID_XMINUSY, RID_XMINUSY_HELP},
    {RID_XCDOTY, RID_XCDOTY_HELP}, {RID_XTIMESY, RID_XTIMESY_HELP},
    {RID_XSYMTIMESY, RID_XSYMTIMESY_HELP}, {RID_XOVERY, RID_XOVERY_HELP},
    {RID_XDIVY, RID_XDIVY_HELP}, {RID_XSYMDIVIDEY, RID_XSYMDIVIDEY_HELP},
    {RID_XOPLUSY, RID_XOPLUSY_HELP}, {RID_XOMINUSY, RID_XOMINUSY_HELP},
    {RID_XODOTY, RID_XODOTY_HELP}, {RID_XOTIMESY, RID_XOTIMESY_HELP},
    {RID_XODIVIDEY, RID_XODIVIDEY_HELP}, {RID_XCIRCY, RID_XCIRCY_HELP},
    {RID_XWIDESLASHY, RID_XWIDESLASHY_HELP}, {RID_XWIDEBSLASHY, RID_XWIDEBSLASHY_HELP},
    {nullptr, nullptr},
    {RID_NEGX, RID_NEGX_HELP}, {RID_XANDY, RID_XANDY_HELP}, {RID_XORY, RID_XORY_HELP},
};

const std::pair<const char*, const char*> SmElementsControl::aRelationsList[] =
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

const std::pair<const char*, const char*> SmElementsControl::aSetOperations[] =
{
    {RID_XINY, RID_XINY_HELP}, {RID_XNOTINY, RID_XNOTINY_HELP}, {RID_XOWNSY, RID_XOWNSY_HELP},
    {nullptr, nullptr},
    {RID_XINTERSECTIONY, RID_XINTERSECTIONY_HELP}, {RID_XUNIONY, RID_XUNIONY_HELP},
    {RID_XSETMINUSY, RID_XSETMINUSY_HELP}, {RID_XSLASHY, RID_XSLASHY_HELP},
    {RID_XSUBSETY, RID_XSUBSETY_HELP}, {RID_XSUBSETEQY, RID_XSUBSETEQY_HELP},
    {RID_XSUPSETY, RID_XSUPSETY_HELP}, {RID_XSUPSETEQY, RID_XSUPSETEQY_HELP},
    {RID_XNSUBSETY, RID_XNSUBSETY_HELP}, {RID_XNSUBSETEQY, RID_XNSUBSETEQY_HELP},
    {RID_XNSUPSETY, RID_XNSUPSETY_HELP}, {RID_XNSUPSETEQY, RID_XNSUPSETEQY_HELP},
    {nullptr, nullptr},
    {RID_EMPTYSET, RID_EMPTYSET_HELP}, {RID_ALEPH, RID_ALEPH_HELP}, {RID_SETN, RID_SETN_HELP},
    {RID_SETZ, RID_SETZ_HELP}, {RID_SETQ, RID_SETQ_HELP}, {RID_SETR, RID_SETR_HELP}, {RID_SETC, RID_SETC_HELP}
};

const std::pair<const char*, const char*> SmElementsControl::aFunctions[] =
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
    {RID_ARTANHX, RID_ARTANHX_HELP}, {RID_ARCOTHX, RID_ARCOTHX_HELP}
};

const std::pair<const char*, const char*> SmElementsControl::aOperators[] =
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
};

const std::pair<const char*, const char*> SmElementsControl::aAttributes[] =
{
    {RID_ACUTEX, RID_ACUTEX_HELP}, {RID_GRAVEX, RID_GRAVEX_HELP}, {RID_BREVEX, RID_BREVEX_HELP},
    {RID_CIRCLEX, RID_CIRCLEX_HELP}, {RID_DOTX, RID_DOTX_HELP}, {RID_DDOTX, RID_DDOTX_HELP},
    {RID_DDDOTX, RID_DDDOTX_HELP}, {RID_BARX, RID_BARX_HELP}, {RID_VECX, RID_VECX_HELP},
    {RID_TILDEX, RID_TILDEX_HELP}, {RID_HATX, RID_HATX_HELP}, {RID_CHECKX, RID_CHECKX_HELP},
    {nullptr, nullptr},
    {RID_WIDEVECX, RID_WIDEVECX_HELP}, {RID_WIDETILDEX, RID_WIDETILDEX_HELP}, {RID_WIDEHATX, RID_WIDEHATX_HELP},
    {RID_OVERLINEX, RID_OVERLINEX_HELP}, {RID_UNDERLINEX, RID_UNDERLINEX_HELP}, {RID_OVERSTRIKEX, RID_OVERSTRIKEX_HELP},
    {nullptr, nullptr},
    {RID_PHANTOMX, RID_PHANTOMX_HELP}, {RID_BOLDX, RID_BOLDX_HELP}, {RID_ITALX, RID_ITALX_HELP},
    {RID_SIZEXY, RID_SIZEXY_HELP}, {RID_FONTXY, RID_FONTXY_HELP},
    {nullptr, nullptr},
    {RID_COLORX_BLACK, RID_COLORX_BLACK_HELP}, {RID_COLORX_BLUE, RID_COLORX_BLUE_HELP},
    {RID_COLORX_GREEN, RID_COLORX_GREEN_HELP}, {RID_COLORX_RED, RID_COLORX_RED_HELP},
    {RID_COLORX_CYAN, RID_COLORX_CYAN_HELP}, {RID_COLORX_MAGENTA, RID_COLORX_MAGENTA_HELP},
    {RID_COLORX_YELLOW, RID_COLORX_YELLOW_HELP}, {RID_COLORX_GRAY, RID_COLORX_GRAY_HELP},
    {RID_COLORX_LIME, RID_COLORX_LIME_HELP}, {RID_COLORX_MAROON, RID_COLORX_MAROON_HELP},
    {RID_COLORX_NAVY, RID_COLORX_NAVY_HELP}, {RID_COLORX_OLIVE, RID_COLORX_OLIVE_HELP},
    {RID_COLORX_PURPLE, RID_COLORX_PURPLE_HELP}, {RID_COLORX_SILVER, RID_COLORX_SILVER_HELP},
    {RID_COLORX_TEAL, RID_COLORX_TEAL_HELP}
};

const std::pair<const char*, const char*> SmElementsControl::aBrackets[] =
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
    {RID_XEVALUATEDATY, RID_XEVALUATEDATY_HELP},
    {nullptr, nullptr},
    {RID_XOVERBRACEY, RID_XOVERBRACEY_HELP}, {RID_XUNDERBRACEY, RID_XUNDERBRACEY_HELP},
};

const std::pair<const char*, const char*> SmElementsControl::aFormats[] =
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

const std::pair<const char*, const char*> SmElementsControl::aOthers[] =
{
    {RID_INFINITY, RID_INFINITY_HELP}, {RID_PARTIAL, RID_PARTIAL_HELP}, {RID_NABLA, RID_NABLA_HELP},
    {RID_EXISTS, RID_EXISTS_HELP}, {RID_NOTEXISTS, RID_NOTEXISTS_HELP}, {RID_FORALL, RID_FORALL_HELP},
    {RID_HBAR, RID_HBAR_HELP}, {RID_LAMBDABAR, RID_LAMBDABAR_HELP}, {RID_RE, RID_RE_HELP},
    {RID_IM, RID_IM_HELP}, {RID_WP, RID_WP_HELP},
    {nullptr, nullptr},
    {RID_LEFTARROW, RID_LEFTARROW_HELP}, {RID_RIGHTARROW, RID_RIGHTARROW_HELP}, {RID_UPARROW, RID_UPARROW_HELP},
    {RID_DOWNARROW, RID_DOWNARROW_HELP},
    {nullptr, nullptr},
    {RID_DOTSLOW, RID_DOTSLOW_HELP}, {RID_DOTSAXIS, RID_DOTSAXIS_HELP}, {RID_DOTSVERT, RID_DOTSVERT_HELP},
    {RID_DOTSUP, RID_DOTSUP_HELP}, {RID_DOTSDOWN, RID_DOTSDOWN_HELP}
};

SmElementsControl::SmElementsControl(vcl::Window *pParent)
    : Control(pParent, WB_TABSTOP)
    , mpDocShell(new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT))
    , mpCurrentElement(nullptr)
    , mbVerticalMode(true)
    , mxScroll(VclPtr<ScrollBar>::Create(this, WB_VERT))
{
    set_id("element_selector");
    SetMapMode( MapMode(MapUnit::Map100thMM) );
    SetDrawMode( DrawModeFlags::Default );
    SetLayoutMode( ComplexTextLayoutFlags::Default );
    SetDigitLanguage( LANGUAGE_ENGLISH );

    maFormat.SetBaseSize(PixelToLogic(Size(0, SmPtsTo100th_mm(12))));

    mxScroll->SetScrollHdl( LINK(this, SmElementsControl, ScrollHdl) );
    mxScroll->Show();
}

SmElementsControl::~SmElementsControl()
{
    disposeOnce();
}

void SmElementsControl::dispose()
{
    mpDocShell->DoClose();
    mxScroll.disposeAndClear();
    Control::dispose();
}

void SmElementsControl::setVerticalMode(bool bVerticalMode)
{
    mbVerticalMode = bVerticalMode;
}

void SmElementsControl::LayoutOrPaintContents(vcl::RenderContext *pContext)
{
    bool bOldVisibleState = mxScroll->IsVisible();

    sal_Int32 nScrollbarWidth = bOldVisibleState ? GetSettings().GetStyleSettings().GetScrollBarSize() : 0;

    sal_Int32 nControlWidth = GetOutputSizePixel().Width() - nScrollbarWidth;
    sal_Int32 nControlHeight = GetOutputSizePixel().Height();

    sal_Int32 boxX = maMaxElementDimensions.Width()  + 10;
    sal_Int32 boxY = maMaxElementDimensions.Height() + 10;

    sal_Int32 x = 0;
    sal_Int32 y = -mxScroll->GetThumbPos();

    sal_Int32 perLine = 0;

    if (mbVerticalMode)
        perLine = nControlHeight / boxY;
    else
        perLine = nControlWidth / boxX;

    if (perLine <= 0)
    {
        perLine = 1;
    }

    if (mbVerticalMode)
        boxY = nControlHeight / perLine;
    else
        boxX = nControlWidth / perLine;

    for (std::unique_ptr<SmElement> & i : maElementList)
    {
        SmElement* element = i.get();
        if (element->isSeparator())
        {
            if (mbVerticalMode)
            {
                x += boxX;
                y = 0;

                tools::Rectangle aSelectionRectangle(x + 5 - 1, y + 5,
                                              x + 5 + 1, nControlHeight - 5);

                if (pContext)
                    pContext->DrawRect(PixelToLogic(aSelectionRectangle));
                x += 10;
            }
            else
            {
                x = 0;
                y += boxY;

                tools::Rectangle aSelectionRectangle(x + 5, y + 5 - 1,
                                              nControlWidth - 5, y + 5 + 1);

                if (pContext)
                    pContext->DrawRect(PixelToLogic(aSelectionRectangle));
                y += 10;
            }
        }
        else
        {
            Size aSizePixel = LogicToPixel(Size(element->getNode()->GetWidth(),
                                                element->getNode()->GetHeight()));
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

            if (mpCurrentElement == element && pContext)
            {
                pContext->Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
                pContext->SetFillColor(Color(230, 230, 230));
                pContext->SetLineColor(Color(230, 230, 230));

                pContext->DrawRect(PixelToLogic(tools::Rectangle(x + 2, y + 2, x + boxX - 2, y + boxY - 2)));
                pContext->Pop();
            }

            Point location(x + ((boxX - aSizePixel.Width()) / 2),
                           y + ((boxY - aSizePixel.Height()) / 2));

            if (pContext)
                SmDrawingVisitor(*pContext, PixelToLogic(location), element->getNode().get());

            element->mBoxLocation = Point(x,y);
            element->mBoxSize = Size(boxX, boxY);

            if (mbVerticalMode)
                y += boxY;
            else
                x += boxX;
        }
    }

    if (!pContext)
    {
        sal_Int32 nTotalControlHeight = y + boxY + mxScroll->GetThumbPos();

        if (nTotalControlHeight > GetOutputSizePixel().Height())
        {
            mxScroll->SetRangeMax(nTotalControlHeight);
            mxScroll->SetPosSizePixel(Point(nControlWidth, 0), Size(nScrollbarWidth, nControlHeight));
            mxScroll->SetVisibleSize(nControlHeight);
            mxScroll->Show();
        }
        else
        {
            mxScroll->SetThumbPos(0);
            mxScroll->Hide();
        }
    }
}

void SmElementsControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push();
    LayoutOrPaintContents(&rRenderContext);
    rRenderContext.Pop();
}

void SmElementsControl::RequestHelp(const HelpEvent& rHEvt)
{
    if (rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK))
    {
        SmElement* pHelpElement = mpCurrentElement;

        if (!rHEvt.KeyboardActivated())
        {
            Point aHelpEventPos(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
            for (std::unique_ptr<SmElement> & i : maElementList)
            {
                SmElement* pElement = i.get();
                tools::Rectangle aRect(pElement->mBoxLocation, pElement->mBoxSize);
                if (aRect.IsInside(aHelpEventPos))
                {
                    pHelpElement = pElement;
                    break;
                }
            }
        }

        if (!pHelpElement)
            return;

        tools::Rectangle aHelpRect(pHelpElement->mBoxLocation, pHelpElement->mBoxSize);
        Point aPt = OutputToScreenPixel( aHelpRect.TopLeft() );
        aHelpRect.SetLeft( aPt.X() );
        aHelpRect.SetTop( aPt.Y() );
        aPt = OutputToScreenPixel( aHelpRect.BottomRight() );
        aHelpRect.SetRight( aPt.X() );
        aHelpRect.SetBottom( aPt.Y() );

        // get text and display it
        OUString aStr = pHelpElement->getHelpText();
        if (rHEvt.GetMode() & HelpEventMode::BALLOON)
        {
            Help::ShowBalloon(this, aHelpRect.Center(), aHelpRect, aStr);
        }
        else
            Help::ShowQuickHelp(this, aHelpRect, aStr, aStr, QuickHelpFlags::CtrlText);
        return;
    }

    Control::RequestHelp(rHEvt);
}

void SmElementsControl::MouseMove( const MouseEvent& rMouseEvent )
{
    SmElement* pPrevElement = mpCurrentElement;
    mpCurrentElement = nullptr;
    if (rMouseEvent.IsLeaveWindow())
    {
        LayoutOrPaintContents();
        Invalidate();
        return;
    }
    if (tools::Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()))
    {
        for (std::unique_ptr<SmElement> & i : maElementList)
        {
            SmElement* element = i.get();
            tools::Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                if (pPrevElement != element)
                {
                    mpCurrentElement = element;
                    LayoutOrPaintContents();
                    Invalidate();
                    return;
                }
            }
        }
        return;
    }

    Control::MouseMove(rMouseEvent);
}

namespace {

void collectUIInformation(OUString aID)
{
    EventDescription aDescription;
    aDescription.aID = aID;
    aDescription.aParent = "element_selector";
    aDescription.aAction = "SELECT";
    UITestLogger::getInstance().logEvent(aDescription);
}

}

void SmElementsControl::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    GrabFocus();

    if (rMouseEvent.IsLeft() && tools::Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()) && maSelectHdlLink.IsSet())
    {
        sal_uInt16 nElementCount = maElementList.size();

        for (sal_uInt16 n = 0; n < nElementCount; n++)
        {
            std::unique_ptr<SmElement> & i = maElementList[n];
            SmElement* element = i.get();
            tools::Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                maSelectHdlLink.Call(*element);
                collectUIInformation(OUString::number(n));
                return;
            }
        }
    }
    else
    {
        Control::MouseButtonDown (rMouseEvent);
    }
}

IMPL_LINK_NOARG( SmElementsControl, ScrollHdl, ScrollBar*, void )
{
    DoScroll(mxScroll->GetDelta());
}

void SmElementsControl::DoScroll(long nDelta)
{
    Point aNewPoint = mxScroll->GetPosPixel();
    tools::Rectangle aRect(Point(), GetOutputSize());
    aRect.AdjustRight( -(mxScroll->GetSizePixel().Width()) );
    Scroll( 0, -nDelta, aRect );
    mxScroll->SetPosPixel(aNewPoint);
    LayoutOrPaintContents();
    Invalidate();
}

void SmElementsControl::addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText)
{
    auto pNode = SmParser().ParseExpression(aElementVisual);

    pNode->Prepare(maFormat, *mpDocShell, 0);
    pNode->SetSize(Fraction(10,8));
    pNode->Arrange(*this, maFormat);

    Size aSizePixel = LogicToPixel(Size(pNode->GetWidth(), pNode->GetHeight()), MapMode(MapUnit::Map100thMM));
    if (aSizePixel.Width() > maMaxElementDimensions.Width()) {
        maMaxElementDimensions.setWidth( aSizePixel.Width() );
    }

    if (aSizePixel.Height() > maMaxElementDimensions.Height()) {
        maMaxElementDimensions.setHeight( aSizePixel.Height() );
    }

    maElementList.push_back(o3tl::make_unique<SmElement>(std::move(pNode), aElementSource, aHelpText));
}

void SmElementsControl::setElementSetId(const char* pSetId)
{
    msCurrentSetId = pSetId;
    maMaxElementDimensions = Size();
    build();
}

void SmElementsControl::addElements(const std::pair<const char*, const char*> aElementsArray[], sal_uInt16 aElementsArraySize)
{
    for (sal_uInt16 i = 0; i < aElementsArraySize ; i++)
    {
        const char* pElement = aElementsArray[i].first;
        const char* pElementHelp = aElementsArray[i].second;
        if (!pElement) {
            maElementList.push_back(o3tl::make_unique<SmElementSeparator>());
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
            else if (aElement == RID_COLORX_CYAN)
                addElement("color cyan { \"" + SmResId(STR_CYAN) + "\" }", aElement, SmResId(pElementHelp));
            else if (aElement == RID_COLORX_MAGENTA)
                addElement("color magenta { \"" + SmResId(STR_MAGENTA) + "\" }", aElement, SmResId(pElementHelp));
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

            else if (aElement == RID_XOVERBRACEY)
                addElement("{<?><?><?>} overbrace {<?>} ", aElement, SmResId(pElementHelp));
            else if (aElement == RID_XUNDERBRACEY)
                addElement("{<?><?><?>} underbrace {<?>} ", aElement, SmResId(pElementHelp));
            else
                addElement(aElement, aElement, SmResId(pElementHelp));
        }
    }
}

void SmElementsControl::build()
{
    maElementList.clear();

    if (msCurrentSetId == RID_CATEGORY_UNARY_BINARY_OPERATORS)
        addElements(aUnaryBinaryOperatorsList, SAL_N_ELEMENTS(aUnaryBinaryOperatorsList));
    else if (msCurrentSetId == RID_CATEGORY_RELATIONS)
        addElements(aRelationsList, SAL_N_ELEMENTS(aRelationsList));
    else if (msCurrentSetId == RID_CATEGORY_SET_OPERATIONS)
        addElements(aSetOperations, SAL_N_ELEMENTS(aSetOperations));
    else if (msCurrentSetId == RID_CATEGORY_FUNCTIONS)
        addElements(aFunctions, SAL_N_ELEMENTS(aFunctions));
    else if (msCurrentSetId == RID_CATEGORY_OPERATORS)
        addElements(aOperators, SAL_N_ELEMENTS(aOperators));
    else if (msCurrentSetId == RID_CATEGORY_ATTRIBUTES)
        addElements(aAttributes, SAL_N_ELEMENTS(aAttributes));
    else if (msCurrentSetId ==  RID_CATEGORY_BRACKETS)
        addElements(aBrackets, SAL_N_ELEMENTS(aBrackets));
    else if (msCurrentSetId == RID_CATEGORY_FORMATS)
        addElements(aFormats, SAL_N_ELEMENTS(aFormats));
    else if (msCurrentSetId == RID_CATEGORY_OTHERS)
        addElements(aOthers, SAL_N_ELEMENTS(aOthers));
    else if (msCurrentSetId == RID_CATEGORY_EXAMPLES)
    {
        OUString aEquation = "C=%pi cdot d = 2 cdot %pi cdot r";
        addElement(aEquation, aEquation, "");
        aEquation = "E=mc^2";
        addElement(aEquation, aEquation, "");
        aEquation = "a^2 + b^2 = c^2";
        addElement(aEquation, aEquation, "");
        aEquation = "f ( x ) = sum from { { i = 0 } } to { infinity } { {f^{(i)}(0)} over {i!} x^i}";
        addElement(aEquation, aEquation, "");
        aEquation = "f ( x ) = {1} over {%sigma sqrt{2%pi} }func e^-{{(x-%mu)^2} over {2%sigma^2}}";
        addElement(aEquation, aEquation, "");
    }
    LayoutOrPaintContents();
    Invalidate();
}

Size SmElementsControl::GetOptimalSize() const
{
    return LogicToPixel(Size(100, 100), MapMode(MapUnit::MapAppFont));
}

FactoryFunction SmElementsControl::GetUITestFactory() const
{
    return ElementSelectorUIObject::create;
}

const char* SmElementsDockingWindow::aCategories[] = {
    RID_CATEGORY_UNARY_BINARY_OPERATORS,
    RID_CATEGORY_RELATIONS,
    RID_CATEGORY_SET_OPERATIONS,
    RID_CATEGORY_FUNCTIONS,
    RID_CATEGORY_OPERATORS,
    RID_CATEGORY_ATTRIBUTES,
    RID_CATEGORY_BRACKETS,
    RID_CATEGORY_FORMATS,
    RID_CATEGORY_OTHERS,
    RID_CATEGORY_EXAMPLES
};

SmElementsDockingWindow::SmElementsDockingWindow(SfxBindings* pInputBindings, SfxChildWindow* pChildWindow, vcl::Window* pParent) :
    SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DockingElements",
        "modules/smath/ui/dockingelements.ui")
{
    mpElementsControl = VclPtr<SmElementsControl>::Create(get<vcl::Window>("box"));
    mpElementsControl->set_hexpand(true);
    mpElementsControl->set_vexpand(true);
    mpElementsControl->Show();
    get(mpElementListBox, "listbox");

    mpElementsControl->SetBorderStyle( WindowBorderStyle::MONO );

    mpElementListBox->SetDropDownLineCount( SAL_N_ELEMENTS(aCategories) );

    for (const char* pCategory : aCategories)
    {
        mpElementListBox->InsertEntry(SmResId(pCategory));
    }

    mpElementListBox->SetSelectHdl(LINK(this, SmElementsDockingWindow, ElementSelectedHandle));
    mpElementListBox->SelectEntry(SmResId(RID_CATEGORY_UNARY_BINARY_OPERATORS));

    mpElementsControl->SetBackground( COL_WHITE );
    mpElementsControl->SetTextColor( COL_BLACK );
    mpElementsControl->setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);
    mpElementsControl->SetSelectHdl(LINK(this, SmElementsDockingWindow, SelectClickHandler));
}

SmElementsDockingWindow::~SmElementsDockingWindow ()
{
    disposeOnce();
}

void SmElementsDockingWindow::dispose()
{
    mpElementsControl.disposeAndClear();
    mpElementListBox.clear();
    SfxDockingWindow::dispose();
}

void SmElementsDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel( Size(100, 100) );

    Invalidate();
}

void SmElementsDockingWindow::EndDocking( const tools::Rectangle& rReactangle, bool bFloatMode)
{
    SfxDockingWindow::EndDocking(rReactangle, bFloatMode);
    bool bVertical = ( GetAlignment() == SfxChildAlignment::TOP || GetAlignment() == SfxChildAlignment::BOTTOM );
    mpElementsControl->setVerticalMode(bVertical);
}

IMPL_LINK(SmElementsDockingWindow, SelectClickHandler, SmElement&, rElement, void)
{
    SmViewShell* pViewSh = GetView();

    if (pViewSh)
    {
        std::unique_ptr<SfxStringItem> pInsertCommand = o3tl::make_unique<SfxStringItem>(SID_INSERTCOMMANDTEXT, rElement.getText());
        pViewSh->GetViewFrame()->GetDispatcher()->ExecuteList(
            SID_INSERTCOMMANDTEXT, SfxCallMode::RECORD,
            { pInsertCommand.get() });
    }
}

IMPL_LINK( SmElementsDockingWindow, ElementSelectedHandle, ListBox&, rList, void)
{
    for (const char* pCurrentCategory : aCategories)
    {
        OUString aCurrentCategoryString = SmResId(pCurrentCategory);
        if (aCurrentCategoryString == rList.GetSelectedEntry())
        {
            mpElementsControl->setElementSetId(pCurrentCategory);
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
    mpElementsControl->setVerticalMode(bVertical);

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
