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

SV_DECL_REF(SmDocShell)
SV_IMPL_REF(SmDocShell)

SmElement::SmElement(SmNodePointer pNode, OUString aText) :
    mpNode(pNode),
    maText(aText)
{}

SmElement::~SmElement()
{}

SmNodePointer SmElement::getNode()
{
    return mpNode;
}

SmElementSeparator::SmElementSeparator() :
    SmElement(SmNodePointer(), OUString())
{}

//////////////////////////////////

const sal_uInt16 SmElementsControl::aUnaryBinaryOperatorsList[] =
{
    RID_PLUSX, RID_MINUSX, RID_PLUSMINUSX, RID_MINUSPLUSX,
    0xFFFF,
    RID_XPLUSY, RID_XMINUSY, RID_XCDOTY, RID_XTIMESY,
    RID_XSYMTIMESY, RID_XOVERY,  RID_XDIVY, RID_XSYMDIVIDEY,
    RID_XCIRCY, RID_XWIDESLASHY, RID_XWIDEBSLASHY,
    0xFFFF,
    RID_NEGX, RID_XANDY, RID_XORY,
};

const sal_uInt16 SmElementsControl::aRelationsList[] =
{
    RID_XEQY, RID_XNEQY, RID_XLTY, RID_XLEY,
    RID_XLESLANTY, RID_XGTY, RID_XGEY, RID_XGESLANTY,
    0xFFFF,
    RID_XAPPROXY, RID_XSIMY, RID_XSIMEQY, RID_XEQUIVY,
    RID_XPROPY, RID_XPARALLELY, RID_XORTHOY, RID_XDIVIDESY,
    RID_XNDIVIDESY, RID_XTOWARDY,
    0xFFFF,
    RID_DLARROW, RID_DLRARROW, RID_DRARROW,
    0xFFFF,
    RID_XPRECEDESY, RID_XSUCCEEDSY, RID_XPRECEDESEQUALY, RID_XSUCCEEDSEQUALY,
    RID_XPRECEDESEQUIVY, RID_XSUCCEEDSEQUIVY, RID_XNOTPRECEDESY, RID_XNOTSUCCEEDSY
};

const sal_uInt16 SmElementsControl::aSetOperations[] =
{
    RID_XINY, RID_XNOTINY, RID_XOWNSY,
    0xFFFF,
    RID_XINTERSECTIONY, RID_XUNIONY, RID_XSETMINUSY, RID_XSLASHY,
    RID_XSUBSETY, RID_XSUBSETEQY, RID_XSUPSETEQY, RID_XNSUBSETY,
    RID_XNSUBSETEQY, RID_XNSUPSETY, RID_XNSUPSETEQY,
    0xFFFF,
    RID_EMPTYSET, RID_ALEPH, RID_SETN, RID_SETZ,
    RID_SETQ, RID_SETR, RID_SETC
};

const sal_uInt16 SmElementsControl::aFunctions[] =
{
    RID_ABSX, RID_FACTX, RID_SQRTX, RID_NROOTXY,
    RID_RSUPX, RID_EX, RID_LNX, RID_EXPX,
    RID_LOGX,
    0xFFFF,
    RID_SINX, RID_COSX, RID_TANX, RID_COTX,
    RID_SINHX, RID_COSHX, RID_TANHX, RID_COTHX,
    0xFFFF,
    RID_ARCSINX, RID_ARCCOSX, RID_ARCTANX, RID_ARCCOTX,
    RID_ARSINHX, RID_ARCOSHX, RID_ARTANHX, RID_ARCOTHX
};

const sal_uInt16 SmElementsControl::aOperators[] =
{
    RID_LIMX, RID_SUMX, RID_PRODX, RID_COPRODX,
    RID_INTX, RID_IINTX, RID_IIINTX, RID_LINTX,
    RID_LLINTX, RID_LLLINTX,
    0xFFFF,
    RID_LIM_FROMX, RID_SUM_FROMX, RID_PROD_FROMX, RID_COPROD_FROMX,
    RID_INT_FROMX, RID_IINT_FROMX, RID_IIINT_FROMX, RID_LINT_FROMX,
    RID_LLINT_FROMX, RID_LLLINT_FROMX,
    0xFFFF,
    RID_LIM_TOX, RID_SUM_TOX, RID_PROD_TOX, RID_COPROD_TOX,
    RID_INT_TOX, RID_IINT_TOX, RID_IIINT_TOX, RID_LINT_TOX,
    RID_LLINT_TOX, RID_LLLINT_TOX,
    0xFFFF,
    RID_LIM_FROMTOX, RID_SUM_FROMTOX, RID_PROD_FROMTOX, RID_COPROD_FROMTOX,
    RID_INT_FROMTOX, RID_IINT_FROMTOX, RID_IIINT_FROMTOX, RID_LINT_FROMTOX,
    RID_LLINT_FROMTOX, RID_LLLINT_FROMTOX,

};

const sal_uInt16 SmElementsControl::aAttributes[] =
{
    RID_ACUTEX, RID_GRAVEX, RID_BREVEX, RID_CIRCLEX,
    RID_DOTX, RID_DDOTX, RID_DDDOTX, RID_BARX,
    RID_VECX, RID_TILDEX, RID_HATX,
    0xFFFF,
    RID_WIDEVECX, RID_WIDETILDEX, RID_WIDEHATX, RID_OVERLINEX,
    RID_UNDERLINEX, RID_OVERSTRIKEX,
    0xFFFF,
    RID_PHANTOMX, RID_BOLDX, RID_ITALX, RID_SIZEXY, RID_FONTXY
};

const sal_uInt16 SmElementsControl::aBrackets[] =
{
    RID_LRGROUPX,
    0xFFFF,
    RID_LRPARENTX, RID_LRBRACKETX, RID_LRDBRACKETX, RID_LRBRACEX,
    RID_LRANGLEX, RID_LMRANGLEXY, RID_LRCEILX, RID_LRFLOORX,
    RID_LRLINEX, RID_LRDLINEX,
    0xFFFF,
    RID_SLRPARENTX, RID_SLRBRACKETX, RID_SLRDBRACKETX, RID_SLRANGLEX,
    RID_SLMRANGLEXY, RID_SLRCEILX, RID_SLRFLOORX, RID_SLRLINEX,
    RID_SLRDLINEX,
    0XFFFF,
    RID_XOVERBRACEY, RID_XUNDERBRACEY
};

const sal_uInt16 SmElementsControl::aFormats[] =
{
    RID_RSUPX, RID_RSUBX, RID_LSUPX, RID_LSUBX,
    RID_CSUPX, RID_CSUBX,
    0xFFFF,
    RID_NEWLINE, RID_SBLANK, RID_BLANK, RID_NOSPACE,
    RID_BINOMXY, RID_STACK, RID_MATRIX,
};

const sal_uInt16 SmElementsControl::aOthers[] =
{
    RID_INFINITY, RID_PARTIAL, RID_NABLA, RID_EXISTS,
    RID_NOTEXISTS, RID_FORALL, RID_HBAR, RID_LAMBDABAR,
    RID_RE, RID_IM, RID_WP,
    0xFFFF,
    RID_LEFTARROW, RID_RIGHTARROW, RID_UPARROW, RID_DOWNARROW,
    0xFFFF,
    RID_DOTSLOW, RID_DOTSAXIS, RID_DOTSVERT, RID_DOTSUP,
    RID_DOTSDOWN
};

SmElementsControl::SmElementsControl(Window *pParent, const ResId& rResId) :
    Control(pParent, rResId),
    mpDocShell(new SmDocShell(SFXOBJECTSHELL_STD_NORMAL)),
    mpCurrentElement(NULL),
    mbVerticalMode(true)
{
    maFormat.SetBaseSize(PixelToLogic(Size(0, 24)));
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

    sal_Int32 boxX = maMaxElementDimensions.Width()  + 10;
    sal_Int32 boxY = maMaxElementDimensions.Height() + 10;

    sal_Int32 x = 0;
    sal_Int32 y = 0;

    sal_Int32 perLine = 0;

    if (mbVerticalMode)
        perLine = GetOutputSizePixel().Height() / boxY;
    else
        perLine = GetOutputSizePixel().Width()  / boxX;

    if(perLine <= 0) {
        perLine = 1;
    }

    if (mbVerticalMode)
        boxY = GetOutputSizePixel().Height() / perLine;
    else
        boxX = GetOutputSizePixel().Width() / perLine;

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
                    x+5+1, GetOutputSizePixel().Height() - 5);

                DrawRect(PixelToLogic(aSelectionRectangle));
                x += 10;
            }
            else
            {
                x = 0;
                y += boxY;

                Rectangle aSelectionRectangle(
                    x+5,                              y+5-1,
                    GetOutputSizePixel().Width() - 5, y+5+1);

                DrawRect(PixelToLogic(aSelectionRectangle));
                y += 10;
            }
        }
        else
        {
            Size aSizePixel = LogicToPixel(Size(element->getNode()->GetWidth(), element->getNode()->GetHeight()));
            if(mbVerticalMode)
            {
                if ( y + boxY > GetOutputSizePixel().Height())
                {
                    x += boxX;
                    y = 0;
                }
            }
            else
            {
                if ( x + boxX > GetOutputSizePixel().Width())
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

    Pop();
}

void SmElementsControl::MouseMove( const MouseEvent& rMouseEvent )
{
    mpCurrentElement = NULL;
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
                }
            }
        }
    }
    else
    {
        Control::MouseMove (rMouseEvent);
    }
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

void SmElementsControl::addSeparator()
{
    SmElementPointer pElement(new SmElementSeparator());
    maElementList.push_back(pElement);
}

void SmElementsControl::addElement(OUString aElementVisual, OUString aElementSource)
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

    SmElementPointer pElement(new SmElement(pNode, aElementSource));
    maElementList.push_back(pElement);
}

void SmElementsControl::setElementSetId(sal_uInt16 aSetId)
{
    maCurrentSetId = aSetId;
    maMaxElementDimensions = Size();
    build();
}

void SmElementsControl::addElements(const sal_uInt16 aElementsArray[], sal_uInt16 aElementsArraySize)
{
    for (sal_uInt16 i = 0; i < aElementsArraySize ; i++)
    {
        sal_uInt16 aElementId = aElementsArray[i];
        if (aElementId == 0xFFFF) {
            addSeparator();
        } else {
            if (aElementId == RID_NEWLINE)
                addElement(OStringToOUString( "\xe2\x86\xb5", RTL_TEXTENCODING_UTF8 ), SmResId(aElementId));
            else if (aElementId == RID_SBLANK)
                addElement(OUString("\"`\""), SmResId(aElementId));
            else if (aElementId == RID_BLANK)
                addElement(OUString("\"~\""), SmResId(aElementId));
            else if (aElementId == RID_PHANTOMX)
                addElement(OUString("\"hide\""), SmResId(aElementId));
            else if (aElementId == RID_BOLDX)
                addElement(OUString("bold B"), SmResId(aElementId));
            else if (aElementId == RID_ITALX)
                addElement(OUString("ital I"), SmResId(aElementId));
            else if (aElementId == RID_SIZEXY)
                addElement(OUString("\"size\""), SmResId(aElementId));
            else if (aElementId == RID_FONTXY)
                addElement(OUString("\"font\""), SmResId(aElementId));
            else
                addElement(SmResId(aElementId), SmResId(aElementId));
        }
    }
}

void SmElementsControl::build()
{
    maElementList.clear();

    switch(maCurrentSetId)
    {
        case RID_CATEGORY_UNARY_BINARY_OPERATORS:
            addElements(aUnaryBinaryOperatorsList, sizeof(aUnaryBinaryOperatorsList) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_RELATIONS:
            addElements(aRelationsList, sizeof(aRelationsList) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_SET_OPERATIONS:
            addElements(aSetOperations, sizeof(aSetOperations) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_FUNCTIONS:
            addElements(aFunctions, sizeof(aFunctions) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_OPERATORS:
            addElements(aOperators, sizeof(aOperators) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_ATTRIBUTES:
            addElements(aAttributes, sizeof(aAttributes) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_BRACKETS:
            addElements(aBrackets, sizeof(aBrackets) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_FORMATS:
            addElements(aFormats, sizeof(aFormats) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_OTHERS:
            addElements(aOthers, sizeof(aOthers) / sizeof(sal_uInt16));
        break;
        case RID_CATEGORY_EXAMPLES:
        {
            OUString aEquation = OUString("C=%pi cdot d = 2 cdot %pi cdot r");
            addElement(aEquation, aEquation);
            aEquation = OUString("E=mc^2");
            addElement(aEquation, aEquation);
            aEquation = OUString("a^2 + b^2 = c^2");
            addElement(aEquation, aEquation);
            aEquation = OUString("f ( x ) = sum from { { i = 0 } } to { infinity } { {f^{(i)}(0)} over {i!} x^i}");
            addElement(aEquation, aEquation);
            aEquation = OUString("f ( x ) = {1} over {%sigma sqrt{2%pi} }e^-{{(x-%mu)^2} over {2%sigma^2}}");
            addElement(aEquation, aEquation);
        }
        break;
    }
    Invalidate();
}

//*******************

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

    for (sal_uInt16 i = 0; i < sizeof(aCategories) / sizeof(sal_uInt16) ; i++)
    {
        maElementListBox.InsertEntry(OUString(SmResId(aCategories[i])));
    }

    maElementListBox.SetSelectHdl(LINK(this, SmElementsDockingWindow, ElementSelectedHandle));

    Wallpaper aWallpaper( Color( COL_WHITE ) );

    maElementsControl.SetBackground( aWallpaper );
    maElementsControl.SetTextColor( Color( COL_BLACK ) );
    maElementsControl.SetSelectHdl(LINK(this, SmElementsDockingWindow, SelectClickHdl));

    maElementListBox.SelectEntry(OUString(SmResId(RID_CATEGORY_UNARY_BINARY_OPERATORS)));
    maElementsControl.setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);

    FreeResource();
}

SmElementsDockingWindow::~SmElementsDockingWindow ()
{
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
    for (sal_uInt16 i = 0; i < sizeof(aCategories) / sizeof(sal_uInt16) ; i++)
    {
        sal_uInt16 aCurrentCategory = aCategories[i];
        OUString aCurrentCategoryString = OUString(SmResId(aCurrentCategory));
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
