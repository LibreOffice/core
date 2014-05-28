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
#include <smmod.hxx>
#include <config.hxx>
#include <view.hxx>
#include <visitors.hxx>

#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/settings.hxx>

SV_DECL_REF(SmDocShell)
SV_IMPL_REF(SmDocShell)

SmElement::SmElement(SmNodePointer pNode, OUString aText, OUString aHelpText) :
    mpNode(pNode),
    maText(aText),
    maHelpText(aHelpText)
{}

SmElement::~SmElement()
{}

SmNodePointer SmElement::getNode()
{
    return mpNode;
}

SmElementSeparator::SmElementSeparator() :
    SmElement(SmNodePointer(), OUString(), OUString())
{}

const sal_uInt16 SmElementsControl::aUnaryBinaryOperatorsList[][2] =
{
    {RID_PLUSX, RID_PLUSX_HELP}, {RID_MINUSX, RID_MINUSX_HELP},
    {RID_PLUSMINUSX, RID_PLUSMINUSX_HELP}, {RID_MINUSPLUSX, RID_MINUSPLUSX_HELP},
    {0xFFFF, 0},
    {RID_XPLUSY, RID_XPLUSY_HELP}, {RID_XMINUSY, RID_XMINUSY_HELP},
    {RID_XCDOTY, RID_XCDOTY_HELP}, {RID_XTIMESY, RID_XTIMESY_HELP},
    {RID_XSYMTIMESY, RID_XSYMTIMESY_HELP}, {RID_XOVERY, RID_XOVERY_HELP},
    {RID_XDIVY, RID_XDIVY_HELP}, {RID_XSYMDIVIDEY, RID_XSYMDIVIDEY_HELP},
    {RID_XCIRCY, RID_XCIRCY_HELP}, {RID_XWIDESLASHY, RID_XWIDESLASHY_HELP},
    {RID_XWIDEBSLASHY, RID_XWIDEBSLASHY_HELP},
    {0xFFFF, 0},
    {RID_NEGX, RID_NEGX_HELP}, {RID_XANDY, RID_XANDY_HELP}, {RID_XORY, RID_XORY_HELP},
};

const sal_uInt16 SmElementsControl::aRelationsList[][2] =
{
    {RID_XEQY, RID_XEQY_HELP}, {RID_XNEQY, RID_XNEQY_HELP}, {RID_XLTY, RID_XLTY_HELP},
    {RID_XLEY, RID_XLEY_HELP}, {RID_XLESLANTY, RID_XLESLANTY_HELP}, {RID_XGTY, RID_XGTY_HELP},
    {RID_XGEY, RID_XGEY_HELP}, {RID_XGESLANTY, RID_XGESLANTY_HELP},
    {RID_XLLY, RID_XLLY_HELP}, {RID_XGGY, RID_XGGY_HELP},
    {0xFFFF, 0},
    {RID_XAPPROXY, RID_XAPPROXY_HELP}, {RID_XSIMY, RID_XSIMY_HELP}, {RID_XSIMEQY, RID_XSIMEQY_HELP},
    {RID_XEQUIVY, RID_XEQUIVY_HELP}, {RID_XPROPY, RID_XPROPY_HELP}, {RID_XPARALLELY, RID_XPARALLELY_HELP},
    {RID_XORTHOY, RID_XORTHOY_HELP}, {RID_XDIVIDESY, RID_XDIVIDESY_HELP}, {RID_XNDIVIDESY, RID_XNDIVIDESY_HELP},
    {RID_XTOWARDY, RID_XTOWARDY_HELP},
    {0xFFFF, 0},
    {RID_DLARROW, RID_DLARROW_HELP}, {RID_DLRARROW, RID_DLRARROW_HELP}, {RID_DRARROW, RID_DRARROW_HELP},
    {0xFFFF, 0},
    {RID_XPRECEDESY, RID_XPRECEDESY_HELP}, {RID_XSUCCEEDSY, RID_XSUCCEEDSY_HELP},
    {RID_XPRECEDESEQUALY, RID_XPRECEDESEQUALY_HELP}, {RID_XSUCCEEDSEQUALY, RID_XSUCCEEDSEQUALY_HELP},
    {RID_XPRECEDESEQUIVY, RID_XPRECEDESEQUIVY_HELP}, {RID_XSUCCEEDSEQUIVY, RID_XSUCCEEDSEQUIVY_HELP},
    {RID_XNOTPRECEDESY, RID_XNOTPRECEDESY_HELP}, {RID_XNOTSUCCEEDSY, RID_XNOTSUCCEEDSY_HELP},
};

const sal_uInt16 SmElementsControl::aSetOperations[][2] =
{
    {RID_XINY, RID_XINY_HELP}, {RID_XNOTINY, RID_XNOTINY_HELP}, {RID_XOWNSY, RID_XOWNSY_HELP},
    {0xFFFF, 0},
    {RID_XINTERSECTIONY, RID_XINTERSECTIONY_HELP}, {RID_XUNIONY, RID_XUNIONY_HELP},
    {RID_XSETMINUSY, RID_XSETMINUSY_HELP}, {RID_XSLASHY, RID_XSLASHY_HELP},
    {RID_XSUBSETY, RID_XSUBSETY_HELP}, {RID_XSUBSETEQY, RID_XSUBSETEQY_HELP},
    {RID_XSUPSETY, RID_XSUPSETY_HELP}, {RID_XSUPSETEQY, RID_XSUPSETEQY_HELP},
    {RID_XNSUBSETY, RID_XNSUBSETY_HELP}, {RID_XNSUBSETEQY, RID_XNSUBSETEQY_HELP},
    {RID_XNSUPSETY, RID_XNSUPSETY_HELP}, {RID_XNSUPSETEQY, RID_XNSUPSETEQY_HELP},
    {0xFFFF, 0},
    {RID_EMPTYSET, RID_EMPTYSET_HELP}, {RID_ALEPH, RID_ALEPH_HELP}, {RID_SETN, RID_SETN_HELP},
    {RID_SETZ, RID_SETZ_HELP}, {RID_SETQ, RID_SETQ_HELP}, {RID_SETR, RID_SETR_HELP}, {RID_SETC, RID_SETC_HELP}
};

const sal_uInt16 SmElementsControl::aFunctions[][2] =
{
    {RID_ABSX, RID_ABSX_HELP}, {RID_FACTX, RID_FACTX_HELP}, {RID_SQRTX, RID_SQRTX_HELP},
    {RID_NROOTXY, RID_NROOTXY_HELP}, {RID_RSUPX, RID_RSUPX_HELP}, {RID_EX, RID_EX_HELP},
    {RID_LNX, RID_LNX_HELP}, {RID_EXPX, RID_EXPX_HELP}, {RID_LOGX, RID_LOGX_HELP},
    {0xFFFF, 0},
    {RID_SINX, RID_SINX_HELP}, {RID_COSX, RID_COSX_HELP}, {RID_TANX, RID_TANX_HELP}, {RID_COTX, RID_COTX_HELP},
    {RID_SINHX, RID_SINHX_HELP}, {RID_COSHX, RID_COSHX_HELP}, {RID_TANHX, RID_TANHX_HELP},
    {RID_COTHX, RID_COTHX_HELP},
    {0xFFFF, 0},
    {RID_ARCSINX, RID_ARCSINX_HELP}, {RID_ARCCOSX, RID_ARCCOSX_HELP}, {RID_ARCTANX, RID_ARCTANX_HELP},
    {RID_ARCCOTX, RID_ARCCOTX_HELP}, {RID_ARSINHX, RID_ARSINHX_HELP}, {RID_ARCOSHX, RID_ARCOSHX_HELP},
    {RID_ARTANHX, RID_ARTANHX_HELP}, {RID_ARCOTHX, RID_ARCOTHX_HELP}
};

const sal_uInt16 SmElementsControl::aOperators[][2] =
{
    {RID_LIMX, RID_LIMX_HELP}, {RID_SUMX, RID_SUMX_HELP}, {RID_PRODX, RID_PRODX_HELP},
    {RID_COPRODX, RID_COPRODX_HELP}, {RID_INTX, RID_INTX_HELP}, {RID_IINTX, RID_IINTX_HELP},
    {RID_IIINTX, RID_IIINTX_HELP}, {RID_LINTX, RID_LINTX_HELP}, {RID_LLINTX, RID_LLINTX_HELP},
    {RID_LLLINTX, RID_LLLINTX_HELP},
    {0xFFFF, 0},
    {RID_LIM_FROMX, RID_LIM_FROMX_HELP}, {RID_SUM_FROMX, RID_SUM_FROMX_HELP}, {RID_PROD_FROMX, RID_PROD_FROMX_HELP},
    {RID_COPROD_FROMX, RID_COPROD_FROMX_HELP}, {RID_INT_FROMX, RID_INT_FROMX_HELP},
    {RID_IINT_FROMX, RID_IINT_FROMX_HELP}, {RID_IIINT_FROMX, RID_IIINT_FROMX_HELP},
    {RID_LINT_FROMX, RID_LINT_FROMX_HELP}, {RID_LLINT_FROMX, RID_LLINT_FROMX_HELP}, {RID_LLLINT_FROMX, RID_LLLINT_FROMX_HELP},
    {0xFFFF, 0},
    {RID_LIM_TOX, RID_LIM_TOX_HELP}, {RID_SUM_TOX, RID_SUM_TOX_HELP}, {RID_PROD_TOX, RID_PROD_TOX_HELP},
    {RID_COPROD_TOX, RID_COPROD_TOX_HELP}, {RID_INT_TOX, RID_INT_TOX_HELP}, {RID_IINT_TOX, RID_IINT_TOX_HELP},
    {RID_IIINT_TOX, RID_IIINT_TOX_HELP}, {RID_LINT_TOX, RID_LINT_TOX_HELP}, {RID_LLINT_TOX, RID_LLINT_TOX_HELP},
    {RID_LLLINT_TOX, RID_LLLINT_TOX_HELP},
    {0xFFFF, 0},
    {RID_LIM_FROMTOX, RID_LIM_FROMTOX_HELP}, {RID_SUM_FROMTOX, RID_SUM_FROMTOX_HELP},
    {RID_PROD_FROMTOX, RID_PROD_FROMTOX_HELP}, {RID_COPROD_FROMTOX, RID_COPROD_FROMTOX_HELP},
    {RID_INT_FROMTOX, RID_INT_FROMTOX_HELP}, {RID_IINT_FROMTOX, RID_IINT_FROMTOX_HELP},
    {RID_IIINT_FROMTOX, RID_IIINT_FROMTOX_HELP}, {RID_LINT_FROMTOX, RID_LINT_FROMTOX_HELP},
    {RID_LLINT_FROMTOX, RID_LLINT_FROMTOX_HELP}, {RID_LLLINT_FROMTOX, RID_LLLINT_FROMTOX_HELP}
};

const sal_uInt16 SmElementsControl::aAttributes[][2] =
{
    {RID_ACUTEX, RID_ACUTEX_HELP}, {RID_GRAVEX, RID_GRAVEX_HELP}, {RID_BREVEX, RID_BREVEX_HELP},
    {RID_CIRCLEX, RID_CIRCLEX_HELP}, {RID_DOTX, RID_DOTX_HELP}, {RID_DDOTX, RID_DDOTX_HELP},
    {RID_DDDOTX, RID_DDDOTX_HELP}, {RID_BARX, RID_BARX_HELP}, {RID_VECX, RID_VECX_HELP},
    {RID_TILDEX, RID_TILDEX_HELP}, {RID_HATX, RID_HATX_HELP}, {RID_CHECKX, RID_CHECKX_HELP},
    {0xFFFF, 0},
    {RID_WIDEVECX, RID_WIDEVECX_HELP}, {RID_WIDETILDEX, RID_WIDETILDEX_HELP}, {RID_WIDEHATX, RID_WIDEHATX_HELP},
    {RID_OVERLINEX, RID_OVERLINEX_HELP}, {RID_UNDERLINEX, RID_UNDERLINEX_HELP}, {RID_OVERSTRIKEX, RID_OVERSTRIKEX_HELP},
    {0xFFFF, 0},
    {RID_PHANTOMX, RID_PHANTOMX_HELP}, {RID_BOLDX, RID_BOLDX_HELP}, {RID_ITALX, RID_ITALX_HELP},
    {RID_SIZEXY, RID_SIZEXY_HELP}, {RID_FONTXY, RID_FONTXY_HELP},
    {0xFFFF, 0},
    {RID_COLORX_BLACK, RID_COLORX_BLACK_HELP}, {RID_COLORX_BLUE, RID_COLORX_BLUE_HELP},
    {RID_COLORX_GREEN, RID_COLORX_GREEN_HELP}, {RID_COLORX_RED, RID_COLORX_RED_HELP},
    {RID_COLORX_CYAN, RID_COLORX_CYAN_HELP}, {RID_COLORX_MAGENTA, RID_COLORX_MAGENTA_HELP},
    {RID_COLORX_YELLOW, RID_COLORX_YELLOW_HELP}
};

const sal_uInt16 SmElementsControl::aBrackets[][2] =
{
    {RID_LRGROUPX, RID_LRGROUPX_HELP},
    {0xFFFF, 0},
    {RID_LRPARENTX, RID_LRPARENTX_HELP}, {RID_LRBRACKETX, RID_LRBRACKETX_HELP},
    {RID_LRDBRACKETX, RID_LRDBRACKETX_HELP}, {RID_LRBRACEX, RID_LRBRACEX_HELP},
    {RID_LRANGLEX, RID_LRANGLEX_HELP}, {RID_LMRANGLEXY, RID_LMRANGLEXY_HELP},
    {RID_LRCEILX, RID_LRCEILX_HELP}, {RID_LRFLOORX, RID_LRFLOORX_HELP},
    {RID_LRLINEX, RID_LRLINEX_HELP}, {RID_LRDLINEX, RID_LRDLINEX_HELP},
    {0xFFFF, 0},
    {RID_SLRPARENTX, RID_SLRPARENTX_HELP}, {RID_SLRBRACKETX, RID_SLRBRACKETX_HELP},
    {RID_SLRDBRACKETX, RID_SLRDBRACKETX_HELP}, {RID_SLRBRACEX, RID_SLRBRACEX_HELP},
    {RID_SLRANGLEX, RID_SLRANGLEX_HELP}, {RID_SLMRANGLEXY, RID_SLMRANGLEXY_HELP},
    {RID_SLRCEILX, RID_SLRCEILX_HELP}, {RID_SLRFLOORX, RID_SLRFLOORX_HELP},
    {RID_SLRLINEX, RID_SLRLINEX_HELP}, {RID_SLRDLINEX, RID_SLRDLINEX_HELP},
    {0XFFFF, 0},
    {RID_XOVERBRACEY, RID_XOVERBRACEY_HELP}, {RID_XUNDERBRACEY, RID_XUNDERBRACEY_HELP},
};

const sal_uInt16 SmElementsControl::aFormats[][2] =
{
    {RID_RSUPX, RID_RSUPX_HELP}, {RID_RSUBX, RID_RSUBX_HELP}, {RID_LSUPX, RID_LSUPX_HELP},
    {RID_LSUBX, RID_LSUBX_HELP}, {RID_CSUPX, RID_CSUPX_HELP}, {RID_CSUBX, RID_CSUBX_HELP},
    {0xFFFF, 0},
    {RID_NEWLINE, RID_NEWLINE_HELP}, {RID_SBLANK, RID_SBLANK_HELP}, {RID_BLANK, RID_BLANK_HELP},
    {RID_NOSPACE, RID_NOSPACE_HELP},
    {RID_ALIGNLX, RID_ALIGNLX_HELP}, {RID_ALIGNCX, RID_ALIGNCX_HELP}, {RID_ALIGNRX, RID_ALIGNRX_HELP},
    {0xFFFF, 0},
    {RID_BINOMXY, RID_BINOMXY_HELP}, {RID_STACK, RID_STACK_HELP},
    {RID_MATRIX, RID_MATRIX_HELP},
};

const sal_uInt16 SmElementsControl::aOthers[][2] =
{
    {RID_INFINITY, RID_INFINITY_HELP}, {RID_PARTIAL, RID_PARTIAL_HELP}, {RID_NABLA, RID_NABLA_HELP},
    {RID_EXISTS, RID_EXISTS_HELP}, {RID_NOTEXISTS, RID_NOTEXISTS_HELP}, {RID_FORALL, RID_FORALL_HELP},
    {RID_HBAR, RID_HBAR_HELP}, {RID_LAMBDABAR, RID_LAMBDABAR_HELP}, {RID_RE, RID_RE_HELP},
    {RID_IM, RID_IM_HELP}, {RID_WP, RID_WP_HELP},
    {0xFFFF, 0},
    {RID_LEFTARROW, RID_LEFTARROW_HELP}, {RID_RIGHTARROW, RID_RIGHTARROW_HELP}, {RID_UPARROW, RID_UPARROW_HELP},
    {RID_DOWNARROW, RID_DOWNARROW_HELP},
    {0xFFFF, 0},
    {RID_DOTSLOW, RID_DOTSLOW_HELP}, {RID_DOTSAXIS, RID_DOTSAXIS_HELP}, {RID_DOTSVERT, RID_DOTSVERT_HELP},
    {RID_DOTSUP, RID_DOTSUP_HELP}, {RID_DOTSDOWN, RID_DOTSDOWN_HELP}
};

SmElementsControl::SmElementsControl(Window *pParent, const ResId& rResId) :
    Control(pParent, rResId),
    mpDocShell(new SmDocShell(SFXOBJECTSHELL_STD_NORMAL)),
    mpCurrentElement(NULL),
    mbVerticalMode(true),
    mpScroll(new ScrollBar(this, WB_VERT))
{
    maFormat.SetBaseSize(PixelToLogic(Size(0, 24)));

    mpScroll->SetScrollHdl( LINK(this, SmElementsControl, ScrollHdl) );
    mpScroll->Show();
}

SmElementsControl::~SmElementsControl()
{
    mpDocShell->DoClose();
}

void SmElementsControl::setVerticalMode(bool bVerticalMode)
{
    mbVerticalMode = bVerticalMode;
}

void SmElementsControl::Paint(const Rectangle&)
{
    Push();

    SetMapMode( MapMode(MAP_100TH_MM) );
    SetDrawMode( DRAWMODE_DEFAULT );
    SetLayoutMode( TEXT_LAYOUT_BIDI_LTR );
    SetDigitLanguage( LANGUAGE_ENGLISH );

    bool bOldVisibleState = mpScroll->IsVisible();

    sal_Int32 nScrollbarWidth = bOldVisibleState ? GetSettings().GetStyleSettings().GetScrollBarSize() : 0;

    sal_Int32 nControlWidth = GetOutputSizePixel().Width() - nScrollbarWidth;
    sal_Int32 nControlHeight = GetOutputSizePixel().Height();

    sal_Int32 boxX = maMaxElementDimensions.Width()  + 10;
    sal_Int32 boxY = maMaxElementDimensions.Height() + 10;

    sal_Int32 x = 0;
    sal_Int32 y = -mpScroll->GetThumbPos();

    sal_Int32 perLine = 0;

    if (mbVerticalMode)
        perLine = nControlHeight / boxY;
    else
        perLine = nControlWidth / boxX;

    if(perLine <= 0) {
        perLine = 1;
    }

    if (mbVerticalMode)
        boxY = nControlHeight / perLine;
    else
        boxX = nControlWidth / perLine;

    for (sal_uInt16 i = 0; i < maElementList.size() ; i++)
    {
        SmElement* element = maElementList[i].get();
        if (element->isSeparator())
        {
            if (mbVerticalMode)
            {
                x += boxX;
                y = 0;

                Rectangle aSelectionRectangle(
                    x+5-1, y+5,
                    x+5+1, nControlHeight - 5);

                DrawRect(PixelToLogic(aSelectionRectangle));
                x += 10;
            }
            else
            {
                x = 0;
                y += boxY;

                Rectangle aSelectionRectangle(
                    x+5,                              y+5-1,
                    nControlWidth - 5, y+5+1);

                DrawRect(PixelToLogic(aSelectionRectangle));
                y += 10;
            }
        }
        else
        {
            Size aSizePixel = LogicToPixel(Size(element->getNode()->GetWidth(), element->getNode()->GetHeight()));
            if(mbVerticalMode)
            {
                if ( y + boxY > nControlHeight)
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

            if (mpCurrentElement == element)
            {
                Push();
                SetFillColor( Color(230, 230, 230) );
                SetLineColor( Color(230, 230, 230) );

                DrawRect(PixelToLogic(Rectangle(x+2, y+2, x+boxX-2, y+boxY-2)));
                Pop();
            }

            Point location(x + ((boxX-aSizePixel.Width())/2), y + ((boxY-aSizePixel.Height())/2));
            SmDrawingVisitor(*this, PixelToLogic(location), element->getNode().get());

            element->mBoxLocation = Point(x,y);
            element->mBoxSize     = Size(boxX, boxY);

            if(mbVerticalMode)
                y += boxY;
            else
                x += boxX;
        }
    }

    sal_Int32 nTotalControlHeight = y + boxY + mpScroll->GetThumbPos();

    if (nTotalControlHeight > GetOutputSizePixel().Height())
    {
        mpScroll->SetRangeMax(nTotalControlHeight);
        mpScroll->SetPosSizePixel(Point(nControlWidth, 0), Size(nScrollbarWidth, nControlHeight));
        mpScroll->SetVisibleSize(nControlHeight);
        mpScroll->Show();
    }
    else
    {
        mpScroll->SetThumbPos(0);
        mpScroll->Hide();
    }

    // If scrollbar visibility changed, we have to go through the
    // calculation once more, see nScrollbarWidth
    if (bOldVisibleState != mpScroll->IsVisible())
        Invalidate();

    Pop();
}

void SmElementsControl::MouseMove( const MouseEvent& rMouseEvent )
{
    mpCurrentElement = NULL;
    OUString tooltip;
    if (Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()))
    {
        for (sal_uInt16 i = 0; i < maElementList.size() ; i++)
        {
            SmElement* element = maElementList[i].get();
            Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                if (mpCurrentElement != element)
                {
                    mpCurrentElement = element;
                    Invalidate();
                    tooltip = element->getHelpText();
                }
            }
        }
    }
    else
    {
        Control::MouseMove (rMouseEvent);
    }

    SetQuickHelpText(tooltip);
}

void SmElementsControl::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    GrabFocus();

    if (rMouseEvent.IsLeft() && Rectangle(Point(0, 0), GetOutputSizePixel()).IsInside(rMouseEvent.GetPosPixel()))
    {
        for (sal_uInt16 i = 0; i < maElementList.size() ; i++)
        {
            SmElement* element = maElementList[i].get();
            Rectangle rect(element->mBoxLocation, element->mBoxSize);
            if (rect.IsInside(rMouseEvent.GetPosPixel()))
            {
                aSelectHdlLink.Call(element);
                return;
            }
        }
    }
    else
    {
        Control::MouseButtonDown (rMouseEvent);
    }
}

IMPL_LINK_NOARG( SmElementsControl, ScrollHdl )
{
    DoScroll(mpScroll->GetDelta());
    return 0;
}

void SmElementsControl::DoScroll(long nDelta)
{
    Point aNewPoint = mpScroll->GetPosPixel();
    Rectangle aRect(Point(), GetOutputSize());
    aRect.Right() -= mpScroll->GetSizePixel().Width();
    Scroll( 0, -nDelta, aRect );
    mpScroll->SetPosPixel(aNewPoint);
    Invalidate();
}

void SmElementsControl::addSeparator()
{
    SmElementPointer pElement(new SmElementSeparator());
    maElementList.push_back(pElement);
}

void SmElementsControl::addElement(OUString aElementVisual, OUString aElementSource, OUString aHelpText)
{
    SmNodePointer pNode(SmParser().ParseExpression(aElementVisual));

    pNode->Prepare(maFormat, *mpDocShell);
    pNode->SetSize(Fraction(10,8));
    pNode->Arrange(*this, maFormat);

    Size aSizePixel = LogicToPixel(Size(pNode->GetWidth(), pNode->GetHeight()), MAP_100TH_MM);
    if (aSizePixel.Width() > maMaxElementDimensions.Width()) {
        maMaxElementDimensions.Width() = aSizePixel.Width();
    }

    if (aSizePixel.Height() > maMaxElementDimensions.Height()) {
        maMaxElementDimensions.Height() = aSizePixel.Height();
    }

    SmElementPointer pElement(new SmElement(pNode, aElementSource, aHelpText));
    maElementList.push_back(pElement);
}

void SmElementsControl::setElementSetId(sal_uInt16 aSetId)
{
    maCurrentSetId = aSetId;
    maMaxElementDimensions = Size();
    build();
}

void SmElementsControl::addElements(const sal_uInt16 aElementsArray[][2], sal_uInt16 aElementsArraySize)
{
    for (sal_uInt16 i = 0; i < aElementsArraySize ; i++)
    {
        sal_uInt16 aElementId = aElementsArray[i][0];
        sal_uInt16 aElementIdHelp = aElementsArray[i][1];
        if (aElementId == 0xFFFF) {
            addSeparator();
        } else {
            if (aElementId == RID_NEWLINE)
                addElement(OStringToOUString( "\xe2\x86\xb5", RTL_TEXTENCODING_UTF8 ), SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_SBLANK)
                addElement("\"`\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_BLANK)
                addElement("\"~\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_PHANTOMX)
                addElement("\"" + SM_RESSTR(STR_HIDE) +"\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_BOLDX)
                addElement("bold B", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_ITALX)
                addElement("ital I", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_SIZEXY)
                addElement("\"" + SM_RESSTR(STR_SIZE) + "\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_FONTXY)
                addElement("\"" + SM_RESSTR(STR_FONT) + "\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_BLACK)
                addElement("color black { \"" + SM_RESSTR(STR_BLACK) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_BLUE)
                addElement("color blue { \"" + SM_RESSTR(STR_BLUE) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_GREEN)
                addElement("color green { \"" + SM_RESSTR(STR_GREEN) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_RED)
                addElement("color red { \"" + SM_RESSTR(STR_RED) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_CYAN)
                addElement("color cyan { \"" + SM_RESSTR(STR_CYAN) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_MAGENTA)
                addElement("color magenta { \"" + SM_RESSTR(STR_MAGENTA) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_COLORX_YELLOW)
                addElement("color yellow { \"" + SM_RESSTR(STR_YELLOW) + "\" }", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_ALIGNLX)
                addElement("\"" + SM_RESSTR(STR_ALIGN_LEFT) + "\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_ALIGNCX)
                addElement("\"" + SM_RESSTR(STR_ALIGN_CENTER) + "\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else if (aElementId == RID_ALIGNRX)
                addElement("\"" + SM_RESSTR(STR_ALIGN_RIGHT) + "\"", SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
            else
                addElement(SM_RESSTR(aElementId), SM_RESSTR(aElementId), SM_RESSTR(aElementIdHelp));
        }
    }
}

void SmElementsControl::build()
{
    maElementList.clear();

    switch(maCurrentSetId)
    {
        // we need to divide by 2 because of the matrix of two dimensions
        case RID_CATEGORY_UNARY_BINARY_OPERATORS:
            addElements(aUnaryBinaryOperatorsList, SAL_N_ELEMENTS(aUnaryBinaryOperatorsList));
        break;
        case RID_CATEGORY_RELATIONS:
            addElements(aRelationsList, SAL_N_ELEMENTS(aRelationsList));
        break;
        case RID_CATEGORY_SET_OPERATIONS:
            addElements(aSetOperations, SAL_N_ELEMENTS(aSetOperations));
        break;
        case RID_CATEGORY_FUNCTIONS:
            addElements(aFunctions, SAL_N_ELEMENTS(aFunctions));
        break;
        case RID_CATEGORY_OPERATORS:
            addElements(aOperators, SAL_N_ELEMENTS(aOperators));
        break;
        case RID_CATEGORY_ATTRIBUTES:
            addElements(aAttributes, SAL_N_ELEMENTS(aAttributes));
        break;
        case RID_CATEGORY_BRACKETS:
            addElements(aBrackets, SAL_N_ELEMENTS(aBrackets));
        break;
        case RID_CATEGORY_FORMATS:
            addElements(aFormats, SAL_N_ELEMENTS(aFormats));
        break;
        case RID_CATEGORY_OTHERS:
            addElements(aOthers, SAL_N_ELEMENTS(aOthers));
        break;
        case RID_CATEGORY_EXAMPLES:
        {
            OUString aEquation = "C=%pi cdot d = 2 cdot %pi cdot r";
            addElement(aEquation, aEquation, "");
            aEquation = "E=mc^2";
            addElement(aEquation, aEquation, "");
            aEquation = "a^2 + b^2 = c^2";
            addElement(aEquation, aEquation, "");
            aEquation = "f ( x ) = sum from { { i = 0 } } to { infinity } { {f^{(i)}(0)} over {i!} x^i}";
            addElement(aEquation, aEquation, "");
            aEquation = "f ( x ) = {1} over {%sigma sqrt{2%pi} }e^-{{(x-%mu)^2} over {2%sigma^2}}";
            addElement(aEquation, aEquation, "");
        }
        break;
    }
    Invalidate();
}

const sal_uInt16 SmElementsDockingWindow::aCategories[] = {
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

SmElementsDockingWindow::SmElementsDockingWindow(SfxBindings* pInputBindings, SfxChildWindow* pChildWindow, Window* pParent) :
    SfxDockingWindow(pInputBindings, pChildWindow, pParent, SmResId(RID_ELEMENTSDOCKINGWINDOW)),
    maElementsControl   (this, SmResId(1)),
    maElementListBox    (this, SmResId(1))
{
    maElementsControl.SetBorderStyle( WINDOW_BORDER_MONO );

    maElementListBox.SetDropDownLineCount( 10 );

    for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aCategories) ; i++)
    {
        maElementListBox.InsertEntry(SM_RESSTR(aCategories[i]));
    }

    maElementListBox.SetSelectHdl(LINK(this, SmElementsDockingWindow, ElementSelectedHandle));

    Wallpaper aWallpaper( Color( COL_WHITE ) );

    maElementsControl.SetBackground( aWallpaper );
    maElementsControl.SetTextColor( Color( COL_BLACK ) );
    maElementsControl.SetSelectHdl(LINK(this, SmElementsDockingWindow, SelectClickHdl));

    maElementListBox.SelectEntry(SM_RESSTR(RID_CATEGORY_UNARY_BINARY_OPERATORS));
    maElementsControl.setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);

    FreeResource();
}

SmElementsDockingWindow::~SmElementsDockingWindow ()
{
}

void SmElementsDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel( Size(100, 100) );

    Invalidate();
}

void SmElementsDockingWindow::EndDocking( const Rectangle& rReactangle, sal_Bool bFloatMode)
{
    SfxDockingWindow::EndDocking(rReactangle, bFloatMode);
    bool bVertical = ( GetAlignment() == SFX_ALIGN_TOP || GetAlignment() == SFX_ALIGN_BOTTOM );
    maElementsControl.setVerticalMode(bVertical);
}

IMPL_LINK( SmElementsDockingWindow, SelectClickHdl, SmElement*, pElement)
{
    SmViewShell* pViewSh = GetView();

    if (pViewSh)
    {
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
            SID_INSERTCOMMANDTEXT, SFX_CALLMODE_STANDARD,
            new SfxStringItem(SID_INSERTCOMMANDTEXT, pElement->getText()), 0L);
    }
    return 0;
}

IMPL_LINK( SmElementsDockingWindow, ElementSelectedHandle, ListBox*, pList)
{
    for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aCategories) ; i++)
    {
        sal_uInt16 aCurrentCategory = aCategories[i];
        OUString aCurrentCategoryString = SM_RESSTR(aCurrentCategory);
        if (aCurrentCategoryString == pList->GetSelectEntry())
        {
            maElementsControl.setElementSetId(aCurrentCategory);
            return 0;
        }
    }
    return 0;
}

SmViewShell* SmElementsDockingWindow::GetView()
{
    SfxViewShell *pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    return PTR_CAST(SmViewShell, pView);
}

void SmElementsDockingWindow::Resize()
{
    bool bVertical = ( GetAlignment() == SFX_ALIGN_TOP || GetAlignment() == SFX_ALIGN_BOTTOM );
    maElementsControl.setVerticalMode(bVertical);

    sal_uInt32 aWidth  = GetOutputSizePixel().Width();
    sal_uInt32 aHeight = GetOutputSizePixel().Height();

    sal_uInt32 aElementsSetsHeight = 23;
    sal_uInt32 aPadding = 5;

    Rectangle aRect1 = Rectangle(aPadding, aPadding, aWidth - aPadding, aElementsSetsHeight + aPadding);

    maElementListBox.SetPosSizePixel(aRect1.TopLeft(), aRect1.GetSize());

    Rectangle aRect = Rectangle(aPadding, aElementsSetsHeight + aPadding + aPadding, aWidth - aPadding, aHeight - aPadding);

    maElementsControl.SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());

    SfxDockingWindow::Resize();
    Invalidate();
}

SFX_IMPL_DOCKINGWINDOW_WITHID(SmElementsDockingWindowWrapper, SID_ELEMENTSDOCKINGWINDOW);

SmElementsDockingWindowWrapper::SmElementsDockingWindowWrapper(
                            Window *pParentWindow, sal_uInt16 nId,
                            SfxBindings *pBindings, SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    pWindow = new SmElementsDockingWindow(pBindings, this, pParentWindow);
    SmElementsDockingWindow* pDialog = (SmElementsDockingWindow*) pWindow;
    pDialog->SetPosSizePixel(Point(0, 0), Size(300, 0));
    pDialog->Show();

    eChildAlignment = SFX_ALIGN_LEFT;

    pDialog->Initialize( pInfo );
}

SmElementsDockingWindowWrapper::~SmElementsDockingWindowWrapper()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
