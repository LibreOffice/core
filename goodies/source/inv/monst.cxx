/*************************************************************************
 *
 *  $RCSfile: monst.cxx,v $
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
#include <stdlib.h>
#include "monst.hxx"
#include "invader.hrc"
#include "expl.hxx"
#include "shapes.hxx"

#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _TOOLS_TIME_HXX //autogen
#include <tools/time.hxx>
#endif

Gegner::Gegner(Fighter* pFig, Bombe* pBom, ResMgr* pRes) :
            pFighter(pFig),
            GegnerListe(0,0),
            bDown(FALSE),
            bLeft(TRUE),
            bAuseMode(FALSE),
            pBombe(pBom),
            pBitMonst1(0L),
            pBitMonst2(0L),
            pBitMonst3(0L),
            pBitMonst4(0L),
            pBitMonst1b(0L),
            pBitMonst2b(0L),
            pBitMonst3b(0L),
            pBitMonst4b(0L),
            pBitMonst5(0L),
            pBitMonst5a(0L),
            pBitMonst5b(0L),
            nDown(MOVEY)
{
    pBitMonst1  = ImplLoadImage( MONSTER1, pRes );
    pBitMonst2  = ImplLoadImage( MONSTER2, pRes );
    pBitMonst3  = ImplLoadImage( MONSTER3, pRes );
    pBitMonst4  = ImplLoadImage( MONSTER4, pRes );
    pBitMonst1b = ImplLoadImage( MONSTER1B, pRes );
    pBitMonst2b = ImplLoadImage( MONSTER2B, pRes );
    pBitMonst3b = ImplLoadImage( MONSTER3B, pRes );
    pBitMonst4b = ImplLoadImage( MONSTER4B, pRes );
    pBitMonst5  = ImplLoadImage( MONSTER5, pRes );
    pBitMonst5a = ImplLoadImage( MONSTER5A, pRes );
    pBitMonst5b = ImplLoadImage( MONSTER5B, pRes );

    aOutSize = pBitMonst1->GetSizePixel();

    nRandWert = 200;
}

Gegner::~Gegner()
{
    ClearAll();

    delete pBitMonst1;
    delete pBitMonst2;
    delete pBitMonst3;
    delete pBitMonst4;
    delete pBitMonst1b;
    delete pBitMonst2b;
    delete pBitMonst3b;
    delete pBitMonst4b;
    delete pBitMonst5;
    delete pBitMonst5a;
    delete pBitMonst5b;
}

void Gegner::InsertGegner(USHORT nType, USHORT x, USHORT y)
{
    Gegner_Impl* pWork = new Gegner_Impl();

    pWork->aType = (enum GegnerType)nType;
    pWork->aMode = MOVE1;
    pWork->aXY   = Point(x,y);
    pWork->aX    = x;
    pWork->nHits = 0;
    switch(pWork->aType)
    {
        case GEGNER1:
            pWork->nPoints = 50;
            pWork->nMaxHits = 1;
            break;
        case GEGNER2:
            pWork->nPoints = 75;
            pWork->nMaxHits = 2;
            break;
        case GEGNER3:
            pWork->nPoints = 150;
            pWork->nMaxHits = 3;
            break;
        case GEGNER4:
            pWork->nPoints = 225;
            pWork->nMaxHits = 5;
            break;
        case GEGNER5:
            pWork->nPoints = 500;
            pWork->nMaxHits = 3;
            pWork->aMode = HIDE;
            break;
    }

    Insert(pWork);
}

void Gegner::Move()
{
    BOOL bNextDown = FALSE;

    for(long i=0; i<Count(); i++)
    {
        if(bDown)
        {
            SetGegnerPos(i,Point(GegnerX(i),GegnerY(i)+nDown));

        }
        else if(bLeft)
        {
            SetGegnerPos(i,Point(GegnerX(i)+MOVEX,GegnerY(i)));
            if(GegnerX(i)+MOVEX+aOutSize.Width() > nMaxX)
                bNextDown = TRUE;
        }
        else
        {
            SetGegnerPos(i,Point(GegnerX(i)-MOVEX,GegnerY(i)));
            if(GegnerX(i)-MOVEX <= 0)
                bNextDown = TRUE;
        }

    }

    if(bDown)
    {
        if(bLeft)
            bLeft = FALSE;
        else
            bLeft = TRUE;
    }

    bDown = FALSE;
    if(bNextDown)
        bDown = TRUE;
}

void Gegner::DrawGegner(OutputDevice* pDev,Point* pStart)
{

    Time aTime;
    srand(aTime.GetTime() % 1000);

    nMaxX = pDev->GetOutputSizePixel().Width()-pStart->X();

    for(long i=0; i<Count();i++)
    {
        switch(GegType(i))
        {
            case GEGNER1:
                if(GegMode(i) == MOVE1)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst1);
                    SetMode(i,MOVE2);
                }
                else if(GegMode(i) == MOVE2)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst1b);
                    SetMode(i,MOVE1);
                }
                break;
            case GEGNER2:
                if(GegMode(i) == MOVE1)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst2);
                    SetMode(i,MOVE2);
                }
                else if(GegMode(i) == MOVE2)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst2b);
                    SetMode(i,MOVE1);
                }
                break;
            case GEGNER3:
                if(GegMode(i) == MOVE1)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst3);
                    SetMode(i,MOVE2);
                }
                else if(GegMode(i) == MOVE2)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst3b);
                    SetMode(i,MOVE1);
                }
                break;
            case GEGNER4:
                if(GegMode(i) == MOVE1)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst4);
                    SetMode(i,MOVE2);
                }
                else if(GegMode(i) == MOVE2)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst4b);
                    SetMode(i,MOVE1);
                }
                break;
            case GEGNER5:
                if(GegMode(i) == MOVE1)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst5);
                    DecDelay(i);
                    if(!GetDelay(i))
                    {
                        SetDelay(i);
                        SetMode(i,MOVE2);
                    }
                }
                if(GegMode(i) == MOVE2)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst5a);
                    DecDelay(i);
                    if(!GetDelay(i))
                    {
                        SetDelay(i);
                        SetMode(i,MOVE3);
                    }
                }
                if(GegMode(i) == MOVE3)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst5b);
                    DecDelay(i);
                    if(!GetDelay(i))
                    {
                        pBombe->InsertBombe(Point(GegnerX(i)+aOutSize.Width()/2,
                                                GegnerY(i)+aOutSize.Height()));
                        SetDelay(i);
                        SetMode(i,MOVE4);
                    }
                }
                if(GegMode(i) == MOVE4)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst5a);
                    DecDelay(i);
                    if(!GetDelay(i))
                    {
                        SetDelay(i);
                        SetMode(i,MOVE5);
                    }
                }
                if(GegMode(i) == MOVE5)
                {
                    pDev->DrawImage(Point(pStart->X()+GegnerX(i),
                            pStart->Y()+GegnerY(i)),*pBitMonst5);
                    DecDelay(i);
                    if(!GetDelay(i))
                        SetMode(i,HIDE);
                }
                break;

        }

        SetKoll(i,Rectangle(Point(GegnerX(i)+KOLLXY,GegnerY(i)+KOLLXY),
                        Point(GegnerX(i)+aOutSize.Width()-KOLLXY,
                            GegnerY(i)+aOutSize.Height()-KOLLXY)));

        if(bAuseMode && GegMode(i) == MOVE1)
        {
            if(GegnerX(i) < pFighter->GetHalf() &&
                        GegnerX(i)+aOutSize.Width() > pFighter->GetHalf())
                    pBombe->InsertBombe(Point(pFighter->GetPoint().X(),
                                            GegnerY(i)+aOutSize.Height()));
        }
        else
        {
            int ran = rand();

            if(ran < nRandWert)
            {
                if(GegType(i) != GEGNER5)
                    pBombe->InsertBombe(Point(GegnerX(i)+aOutSize.Width()/2,
                                            GegnerY(i)+aOutSize.Height()));
                else if(GegMode(i) == HIDE)
                {
                    SetMode(i,MOVE1);
                    SetDelay(i);
                }
            }
        }
    }

    Move();
}

long Gegner::Kollision(Rectangle& rRect, Explosion* pExpl)
{
    long nWert = -1;

    Rectangle aWork;

    for(long i=0; i<Count();i++)
    {
        aWork = GegnerKoll(i);
        if((aWork.Left() <= rRect.Left() &&  aWork.Right() >= rRect.Right()) &&
            (aWork.Top() <= rRect.Top() && aWork.Bottom() >= rRect.Bottom()) &&
                GegMode(i) != DELETED)
        {
            nWert = 0;
            if(GegnerDest(i))
            {
                SetMode(i,DELETED);
                if(nWert == -1)
                    nWert = GegnerPoints(i);
                else
                    nWert += GegnerPoints(i);
            }

            pExpl->InsertExpl(GegnerPos(i));

        }
    }

    return nWert;
}

BOOL Gegner::GegnerDest(long nWert)
{
    GegnerHit(nWert);
    if(GetObject(nWert)->nHits >= GetObject(nWert)->nMaxHits)
        return TRUE;

    return FALSE;
}

Rectangle Gegner::GetKoll(long nWert)
{
    return Rectangle(Point(GegnerX(nWert)+aOutSize.Width()/2,
                            GegnerY(nWert)+aOutSize.Height()),
                     Point(GegnerX(nWert)+aOutSize.Width()/2,
                            GegnerY(nWert)+aOutSize.Height()));
}

BOOL Gegner::RemoveGegner()
{
    for(long i=Count()-1; i>=0; i--)
    {
        Gegner_Impl* pWork = GetObject(i);

        if(pWork->aMode == DELETED)
        {
            Remove(pWork);
            delete pWork;
        }
    }
    if(Count())
        return FALSE;
    else
        return TRUE;
}

void Gegner::ClearAll()
{
    for(long i=0; i<Count(); i++)
        delete GetObject(i);

    Clear();
}
