/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "tabsplit.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"

#include <vcl/settings.hxx>



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
                
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.TopRight(), rRect.BottomRight());
                DrawLine(rRect.BottomLeft(), rRect.BottomRight());
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.TopRight());
                DrawLine(rRect.TopLeft(), rRect.BottomLeft());
                
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(rRect.Left()+1, rRect.Top()+1, rRect.Right()-1, rRect.Bottom()-1));
                
                SetLineColor(Color(COL_BLACK));
                SetFillColor(Color(COL_BLACK));
                const long xc = rRect.Right()+rRect.Left();
                const long h4 = rRect.GetHeight()/4;
                
                
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
                
                break;
        }
    }
    else
    {
        switch (pViewData->GetVSplitMode())
        {
            case SC_SPLIT_NONE:
            {
                
                SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
                DrawLine(rRect.TopRight(), rRect.BottomRight());
                DrawLine(rRect.BottomLeft(), rRect.BottomRight());
                SetLineColor(GetSettings().GetStyleSettings().GetLightColor());
                DrawLine(rRect.TopLeft(), rRect.TopRight());
                DrawLine(rRect.TopLeft(), rRect.BottomLeft());
                
                SetLineColor();
                SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
                DrawRect(Rectangle(rRect.Left()+1, rRect.Top()+1, rRect.Right()-1, rRect.Bottom()-1));
                
                SetLineColor(Color(COL_BLACK));
                SetFillColor(Color(COL_BLACK));
                const long yc = rRect.Top()+rRect.Bottom();
                const long w4 = rRect.GetWidth()/4;
                
                
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
                
                break;
        }
    }

    SetFillColor(oldFillCol);
    SetLineColor(oldLineCol);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
