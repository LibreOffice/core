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
        if (pViewData->GetHSplitMode()==SC_SPLIT_NONE)
        {
            // Black border
            SetLineColor(Color(COL_BLACK));
            SetFillColor();
            DrawRect(rRect);
            // Internal frame
            const long xl = rRect.Left()+1;
            const long yt = rRect.Top()+2;
            const long xr = rRect.Right()-1;
            const long yb = rRect.Bottom()-2;
            SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
            for (long x=xl; x<xr; x+=2)
            {
                DrawLine(Point(x,yt), Point(x,yb));
            }
            DrawLine(Point(xl,yt-1), Point(xr,yt-1));
            SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
            for (long x=xl+1; x<=xr; x+=2)
            {
                DrawLine(Point(x,yt), Point(x,yb));
            }
            DrawLine(Point(xl,yb+1), Point(xr,yb+1));
        }
        else
        {
            SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
            DrawLine(rRect.TopLeft(), rRect.BottomLeft());
            SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
            DrawLine(rRect.TopRight(), rRect.BottomRight());
            SetLineColor();
            SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
            DrawRect(Rectangle(Point(rRect.Left()+1, rRect.Top()), Point(rRect.Right()-1, rRect.Bottom())));
        }
    }
    else
    {
        if (pViewData->GetVSplitMode()==SC_SPLIT_NONE)
        {
            // Black border
            SetLineColor(Color(COL_BLACK));
            SetFillColor();
            DrawRect(rRect);
            // Internal frame
            const long xl = rRect.Left()+2;
            const long yt = rRect.Top()+1;
            const long xr = rRect.Right()-2;
            const long yb = rRect.Bottom()-1;
            SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
            for (long y=yt; y<yb; y+=2)
            {
                DrawLine(Point(xl,y), Point(xr,y));
            }
            DrawLine(Point(xl-1,yt), Point(xl-1,yb));
            SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
            for (long y=yt+1; y<=yb; y+=2)
            {
                DrawLine(Point(xl,y), Point(xr,y));
            }
            DrawLine(Point(xr+1,yt), Point(xr+1,yb));
        }
        else
        {
            SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
            DrawLine(rRect.TopLeft(), rRect.TopRight());
            SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
            DrawLine(rRect.BottomLeft(), rRect.BottomRight());
            SetLineColor();
            SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
            DrawRect(Rectangle(Point(rRect.Left(), rRect.Top()+1), Point(rRect.Right(), rRect.Bottom()-1)));
        }
    }

    SetFillColor(oldFillCol);
    SetLineColor(oldLineCol);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
