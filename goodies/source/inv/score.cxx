/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: score.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#include "score.hxx"
#include "strings.hrc"
#include "invader.hxx"

ScoreWindow::ScoreWindow(Window* Parent, WinBits wBits, ResMgr* pP) :
                    Window(Parent, wBits),
                    nHero(-1),
                    nRockets(5),
                    nLives(3),
                    nScore(0L),
                    pRes(pP),
                    bPaintBack(FALSE)
{
    SetBackground(Wallpaper( Color( COL_GRAY ) ) );
}

ScoreWindow::~ScoreWindow()
{
}

void ScoreWindow::Paint(const Rectangle&)
{
    Font aFont = GetFont();
    aFont.SetColor(COL_WHITE);
    aFont.SetFillColor(COL_GRAY);
    SetFont(*&aFont);

    SetFillColor(COL_GRAY);

    if(bPaintBack)
    {
        Push( PUSH_LINECOLOR );
        SetLineColor();
        DrawRect(Rectangle(Point(0,0),GetOutputSizePixel()));
        Pop();
    }

    if(nHero)
        DrawText(Point(0,0),String(ResId(nHero, *pRes)));

    DrawText(Point(180,0),String(ResId(STR_ROCKETS, *pRes)));

    DrawText(Point(300,0),String(ResId(STR_FIGHTER, *pRes)));
    DrawText(Point(370,0),String::CreateFromInt32(nLives));

    DrawText(Point(400,0),String(ResId(STR_LEVEL, *pRes)));
    DrawText(Point(460,0),String::CreateFromInt32(nLevel));

    DrawText(Point(500,0),String(ResId(STR_SCORE, *pRes)));
    String aString = String::CreateFromInt32(nScore);
    if ( aString.Len() < 7 )
    {
        for ( xub_StrLen i = aString.Len(); i < 7; ++i )
            aString.Insert( '0', 0 );
    }
    DrawText(Point(560,0),aString);

    for(long i=0; i<5;i++)
    {
        if(nRockets > i)
        {
            SetLineColor(COL_GREEN);
            SetFillColor(COL_GREEN);
        }
        else
        {
            SetLineColor(COL_RED);
            SetFillColor(COL_RED);
        }

        DrawRect(Rectangle(Point(250+i*8,5),Point(256+i*8,12)));
    }
    SetBackground(Wallpaper( Color( COL_GRAY ) ) );

    bPaintBack = FALSE;
}

void ScoreWindow::SetHero(long nName)
{
    nHero = nName;

    SetBackground();
    Invalidate();
}

void ScoreWindow::SetRockets(USHORT nWert)
{
    nRockets = nWert;

    SetBackground();
    Invalidate();
}

void ScoreWindow::SetLives(USHORT nWert)
{
    nLives = nWert;

//  SetBackground();
    Invalidate();
}

void ScoreWindow::SetScore(long nWert)
{
    nScore = nWert;

    SetBackground();
    Invalidate();
}

void ScoreWindow::SetLevel(long nWert)
{
    nLevel = nWert;

//  SetBackground();
    Invalidate();
}

void ScoreWindow::ShowMe()
{
    bPaintBack = TRUE;

    Window::Show();
}
