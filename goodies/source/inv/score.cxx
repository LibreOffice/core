/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: score.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:55:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        DrawText(Point(0,0),String(ResId(nHero, pRes)));

    DrawText(Point(180,0),String(ResId(STR_ROCKETS, pRes)));

    DrawText(Point(300,0),String(ResId(STR_FIGHTER, pRes)));
    DrawText(Point(370,0),String::CreateFromInt32(nLives));

    DrawText(Point(400,0),String(ResId(STR_LEVEL, pRes)));
    DrawText(Point(460,0),String::CreateFromInt32(nLevel));

    DrawText(Point(500,0),String(ResId(STR_SCORE, pRes)));
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
