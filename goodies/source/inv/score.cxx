/*************************************************************************
 *
 *  $RCSfile: score.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "score.hxx"
#include "strings.hrc"
#include "invader.hxx"

ScoreWindow::ScoreWindow(Window* Parent, WinBits wBits, ResMgr* pP) :
                    Window(Parent, wBits),
                    bPaintBack(FALSE),
                    pRes(pP),
                    nHero(-1),
                    nRockets(5),
                    nLives(3),
                    nScore(0L)
{
    SetBackgroundBrush( Brush( COL_GRAY, BRUSH_SOLID ));
}

ScoreWindow::~ScoreWindow()
{
}

void ScoreWindow::Paint(const Rectangle& rRect)
{
    Font aFont = GetFont();
    aFont.SetColor(COL_WHITE);
    aFont.SetFillColor(COL_GRAY);
    SetFont(*&aFont);
    Brush aBrush = GetFillInBrush();
    aBrush.SetColor(COL_GRAY);
    SetFillInBrush(*&aBrush);

    if(bPaintBack)
    {
        Pen aOldPen = GetPen();
        Pen aPen = aOldPen;
        aPen.SetStyle(PEN_NULL);
        SetPen(aPen);

        DrawRect(Rectangle(Point(0,0),GetOutputSizePixel()));
        SetPen(aOldPen);
    }

    if(nHero)
        DrawText(Point(0,0),String(ResId(nHero, pRes)));

    DrawText(Point(180,0),String(ResId(STR_ROCKETS, pRes)));

    DrawText(Point(300,0),String(ResId(STR_FIGHTER, pRes)));
    DrawText(Point(370,0),String(nLives));

    DrawText(Point(400,0),String(ResId(STR_LEVEL, pRes)));
    DrawText(Point(460,0),String(nLevel));

    DrawText(Point(500,0),String(ResId(STR_SCORE, pRes)));
    String aString(nScore);
    if(aString.Len() < 7)
    {
        String bString;
        for(long i=0; i+aString.Len()<7;i++)
            bString += String("0");

        aString = bString + aString;

    }
    DrawText(Point(560,0),aString);

    for(long i=0; i<5;i++)
    {
        Pen aPen = GetPen();
        if(nRockets > i)
        {
            aPen.SetColor(COL_GREEN);
            aPen.SetStyle(PEN_SOLID);
            SetPen(*&aPen);
            aBrush.SetColor(COL_GREEN);
            SetFillInBrush(*&aBrush);
        }
        else
        {
            aPen.SetColor(COL_RED);
            aPen.SetStyle(PEN_SOLID);
            SetPen(*&aPen);
            aBrush.SetColor(COL_RED);
            SetFillInBrush(*&aBrush);
        }

        DrawRect(Rectangle(Point(250+i*8,5),Point(256+i*8,12)));
    }
    SetBackgroundBrush( Brush( COL_GRAY, BRUSH_SOLID ));

    bPaintBack = FALSE;
}

void ScoreWindow::SetHero(long nName)
{
    nHero = nName;

    SetBackgroundBrush(BRUSH_NULL);
    Invalidate();
}

void ScoreWindow::SetRockets(USHORT nWert)
{
    nRockets = nWert;

    SetBackgroundBrush(BRUSH_NULL);
    Invalidate();
}

void ScoreWindow::SetLives(USHORT nWert)
{
    nLives = nWert;

//  SetBackgroundBrush(BRUSH_NULL);
    Invalidate();
}

void ScoreWindow::SetScore(long nWert)
{
    nScore = nWert;

    SetBackgroundBrush(BRUSH_NULL);
    Invalidate();
}

void ScoreWindow::SetLevel(long nWert)
{
    nLevel = nWert;

//  SetBackgroundBrush(BRUSH_NULL);
    Invalidate();
}

void ScoreWindow::Show()
{
    bPaintBack = TRUE;

    Window::Show();
}
