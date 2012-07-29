/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "tabsplit.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"

//==================================================================

ScTabSplitter::ScTabSplitter( Window* pParent, WinBits nWinStyle, ScViewData* pData ) :
    Splitter( pParent, nWinStyle ),
    pViewData(pData)
{
    SetFixed(false);
    EnableRTL( false );
}


ScTabSplitter::~ScTabSplitter()
{
}

void ScTabSplitter::MouseMove( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseMove( rMEvt );
    else
        Splitter::MouseMove( rMEvt );
}

void ScTabSplitter::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonUp( rMEvt );
    else
        Splitter::MouseButtonUp( rMEvt );
}

void ScTabSplitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonDown( rMEvt );
    else
        Splitter::MouseButtonDown( rMEvt );
}

void ScTabSplitter::Splitting( Point& rSplitPos )
{
    Window* pParent = GetParent();
    Point aScreenPos = pParent->OutputToNormalizedScreenPixel( rSplitPos );
    pViewData->GetView()->SnapSplitPos( aScreenPos );
    Point aNew = pParent->NormalizedScreenToOutputPixel( aScreenPos );
    if ( IsHorizontal() )
        rSplitPos.X() = aNew.X();
    else
        rSplitPos.Y() = aNew.Y();
}


void ScTabSplitter::SetFixed(bool bSet)
{
    bFixed = bSet;
    if (bSet)
        SetPointer(POINTER_ARROW);
    else if (IsHorizontal())
        SetPointer(POINTER_HSPLIT);
    else
        SetPointer(POINTER_VSPLIT);
}

void ScTabSplitter::Paint( const Rectangle& rRect )
{
    const Color oldFillCol = GetFillColor();
    const Color oldLineCol = GetLineColor();

    if (IsHorizontal())
    {
        switch (pViewData->GetHSplitMode())
        {
            case SC_SPLIT_NONE:
            {
                // Draw 3D border
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.TopRight(), rRect.BottomRight());
                DrawLine(rRect.BottomLeft(), rRect.BottomRight());
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.TopRight());
                DrawLine(rRect.TopLeft(), rRect.BottomLeft());
                // Fill internal rectangle
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(rRect.Left()+1, rRect.Top()+1, rRect.Right()-1, rRect.Bottom()-1));
                // Draw handle
                SetLineColor(Color(COL_BLACK));
                SetFillColor(Color(COL_BLACK));
                const long xc = rRect.Right()+rRect.Left();
                const long h4 = rRect.GetHeight()/4;
                // First xc fraction is truncated, second one is rounded. This will draw a centered line
                // in handlers with odd width and a centered rectangle in those with even width.
                DrawRect(Rectangle(Point(xc/2, rRect.Top()+h4), Point((xc+1)/2, rRect.Bottom()-h4)));
                break;
            }
            case SC_SPLIT_NORMAL:
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.BottomLeft());
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.TopRight(), rRect.BottomRight());
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(Point(rRect.Left()+1, rRect.Top()), Point(rRect.Right()-1, rRect.Bottom())));
                break;
            case SC_SPLIT_FIX:
                // Nothing to draw
                break;
        }
    }
    else
    {
        switch (pViewData->GetVSplitMode())
        {
            case SC_SPLIT_NONE:
            {
                // Draw 3D border
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.TopRight(), rRect.BottomRight());
                DrawLine(rRect.BottomLeft(), rRect.BottomRight());
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.TopRight());
                DrawLine(rRect.TopLeft(), rRect.BottomLeft());
                // Fill internal rectangle
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(rRect.Left()+1, rRect.Top()+1, rRect.Right()-1, rRect.Bottom()-1));
                // Draw handle
                SetLineColor(Color(COL_BLACK));
                SetFillColor(Color(COL_BLACK));
                const long yc = rRect.Top()+rRect.Bottom();
                const long w4 = rRect.GetWidth()/4;
                // First yc fraction is truncated, second one is rounded. This will draw a centered line
                // in handlers with odd height and a centered rectangle in those with even height.
                DrawRect(Rectangle(Point(rRect.Left()+w4, yc/2), Point(rRect.Right()-w4, (yc+1)/2)));
                break;
            }
            case SC_SPLIT_NORMAL:
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.TopRight());
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.BottomLeft(), rRect.BottomRight());
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(Point(rRect.Left(), rRect.Top()+1), Point(rRect.Right(), rRect.Bottom()-1)));
                break;
            case SC_SPLIT_FIX:
                // Nothing to draw
                break;
        }
    }

    SetFillColor(oldFillCol);
    SetLineColor(oldLineCol);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
