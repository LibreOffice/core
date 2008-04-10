/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: invader1.cxx,v $
 * $Revision: 1.8 $
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
#include <stdlib.h>
#include <vcl/msgbox.hxx>
#include <tools/datetime.hxx>

#include "invader.hxx"
#include "monst.hxx"
#include "shapes.hxx"
#include "expl.hxx"
#include "waves.hxx"
#include "score.hxx"
#include "strings.hrc"


void MyWindow::Kollision()
{
    Rectangle aRect;
    long nStart   = nScore;
    long nRockets = pMunition->Count();

    long nWert;

    unsigned long i;
    for(i=0; i<pBombe->Count(); i++)
    {
        //Kollision Bombe->Mauer prfen
        aRect = pBombe->GetKoll(i);
        if(pWall->Kollision(aRect,FALSE))
            pBombe->SetDelete(i);
        //Kollision Bombe->Fighter prfen
        if(pFighter->Kollision(pBombe->GetKoll(i),pExplosion))
        {
            bFightDest = TRUE;
            pBombe->SetDelete(i);
        }
    }

    for(i=0; i<pMunition->Count(); i++)
    {
        //Kollision Muniton->Gegner prfen
        aRect = pMunition->GetKoll(i);
        nWert = pGegner->Kollision(aRect,pExplosion);
        if(nWert != -1)
        {
            nScore += nWert;
            pMunition->SetDelete(i);
        }
        //Kollision Muniton->Mauer prfen
        aRect = pMunition->GetKoll(i);
        if(pWall->Kollision(aRect,FALSE))
            pMunition->SetDelete(i);

        //Kollision Munition->Bombe prfen
        aRect = pMunition->GetKoll(i);
        if(pBombe->Kollision(aRect,pExplosion))
            pMunition->SetDelete(i);
    }

    for(i=0; i<pGegner->Count();i++)
    {

        //Kollision Gegner->Mauer prfen
        aRect = pGegner->GetKoll(i);
        pWall->Kollision(aRect,TRUE);
        //Kollision Gegner->Fighter prfen
        aRect = pGegner->GetKoll(i);
        if(pFighter->Kollision(aRect,pExplosion))
        {
            bFightDest = TRUE;
            pGegner->ClearAll();
            pBombe->ClearAll();
        }

    }

    pBombe->RemoveBomben();
    nWert = pMunition->RemoveMunition();
    if( nWert != nRockets)
        pScoreWindow->SetRockets(nWert);

    if(nScore > nStart)
        pScoreWindow->SetScore(nScore);

    if(pGegner->RemoveGegner())
    {
        bEndLevel = TRUE;
        pBombe->ClearAll();
        pMunition->ClearAll();
    }

    if(bFightDest)
    {
        pBombe->ClearAll();
        pMunition->ClearAll();
        pGegner->ClearAll();
    }

    if(pExplosion->RemoveExpl() && (bEndLevel || bFightDest))
    {
        if(!bWaitDlg)
            aWaitDlgTimer.Start();

        /*String    aString;
        if(!bFightDest)
        {
            aString = String(ResId(STR_GRAT1,GetResMgr()));
            aString += String(nLevel);
            aString += String(ResId(STR_GRAT2,GetResMgr()));

            if(!pBox)
            {
                bWaitDlg = TRUE;
                pBox = new MessBox(GetParent(),WinBits(WB_OK),String(ResId(STR_APP_TITLE,GetResMgr())),aString);
                ProgStatus=FALSE;
                SetBackground(Wallpaper( Color( COL_WHITE ) ) );

                aWaitDlgTimer.Start();
                return;
            }

            SetBackground();
            ProgStatus=TRUE;
            delete pBox;
            pBox = 0L;
            nLevel++;
            GrabFocus();
        }
        else
        {
            nFighter--;
            pScoreWindow->SetLives(nFighter);
            if(nFighter > 0)
            {
                aString = String(ResId(STR_FIGHTDEST1,GetResMgr()));
                aString += String(nFighter);
                aString += String(ResId(STR_FIGHTDEST2,GetResMgr()));
            }
            else
            {
                aString = String(ResId(STR_GAMEOVER,GetResMgr()));
            }

            BOOL aDummyStatus;

            if(!pBox)
            {
                pBox = new MessBox(this,WinBits(WB_OK),String(ResId(STR_APP_TITLE,GetResMgr())),aString);
                aDummyStatus = ProgStatus;
                //ProgStatus=FALSE;
                SetBackground(Wallpaper( Color( COL_WHITE ) ) );

                aWaitDlgTimer.Start();

                return;
            }

            if(nFighter == 0)
            {
                ProgStatus = FALSE;
                nLevel     = 1;
                nFighter   = 3;
                pGegner->SetRandWert(200);
                pGegner->SetDown(10);

                if(bTimeHigh)
                    nTimeOut = TIMEHIGH;
                else
                    nTimeOut = TIMELOW;
            }

            SetBackground();
            //ProgStatus=aDummyStatus;
            delete pBox;
            pBox = 0L;
            GrabFocus();

            if(!ProgStatus)
            {
                StartPhase = 1;
                if (nScore > nHighScore)
                    {
                        String  aString;
                        aString = String(ResId(STR_HIGHSCORE1,GetResMgr()));
                        aString += String(nScore);
                        aString += String(ResId(STR_HIGHSCORE2,GetResMgr()));
                        aString += String(nHighScore);
                        aString += String(ResId(STR_HIGHSCORE3,GetResMgr()));
                        nHighScore = nScore;
                        if(!pBox)
                        {
                            pBox = new MessBox(this,WinBits(WB_OK),String(ResId(STR_APP_TITLE,GetResMgr())),aString);
                            Paint(Rectangle(Point(0,0),Point(640,480)));
                            SetBackground(Wallpaper( Color( COL_WHITE ) ) );

                            aWaitDlgTimer.Start();
                        }

                        SetBackground();
                        delete pBox;
                        pBox = 0L;
                        GrabFocus();
                    }
                nScore     = 0;
            }

        }

        InitLevel();*/
    }
}

void MyWindow::InitLevel()
{
    bStartLevel = TRUE;
    long nWert;

    aPaintTimer.SetTimeout(1000);
    nStartLevel = 0;
    pFighter->Init();
    pMunition->ClearAll();
    pBombe->ClearAll();
    pWall->ClearAll();
    pGegner->ClearAll();
    bAuseModus = FALSE;
    nAuseCount = 0;
    pGegner->SetAuseMode(FALSE);

    pScoreWindow->SetRockets(5);
    pScoreWindow->SetLives(nFighter);
    pScoreWindow->SetScore(nScore);
    pScoreWindow->SetLevel(nLevel);

    if(nLevel > 1)
    {
        pGegner->SetRandWert(pGegner->GetRandWert()+10);
        nTimeOut -= 2;
        if(nTimeOut <= 0)
            nTimeOut = 1;
    }

    if(nLevel == 10 || nLevel == 20 || nLevel == 30 || nLevel >= 40)
    {
        pGegner->SetDown(pGegner->GetDown()+2);
        if(pGegner->GetDown() > 30)
            pGegner->SetDown(30);
    }

    nWert = nLevel;
    while( nWert > WAVES )
        nWert -= WAVES;

    if(!( nLevel % 10 ))
    {
        if(!bGetFighter)
        {
            nFighter++;
            pScoreWindow->SetLives(nFighter);
            bGetFighter = TRUE;
        }
    }
    else
        bGetFighter = FALSE;

    USHORT nGegner;
    USHORT nDummy;

    if(nLevel < WAVES*2)
    {
        nGegner = waves[nWert-1][0];
        nDummy = nGegner;
        for(long i=0; i<nGegner; i++)
            if ( nLevel > 10 ) pGegner->InsertGegner((USHORT)waves[nWert-1][i*3+1],
                                (USHORT)waves[nWert-1][i*3+2],
                                (USHORT)waves[nWert-1][i*3+3]);
            else if ( waves[nWert-1][i*3+1] < 4 ) pGegner->InsertGegner((USHORT)waves[nWert-1][i*3+1],
                                (USHORT)waves[nWert-1][i*3+2],
                                (USHORT)waves[nWert-1][i*3+3]);
            else nDummy--;


    }
    else
    {
        if(levelpuf[0] != nLevel)
            RandomLevel();
        else
        {
            for(long i=0; i<levelpuf[1]; i++)
                pGegner->InsertGegner(levelpuf[i*3+2],levelpuf[i*3+3],levelpuf[i*3+4]);
        }
        nGegner = levelpuf[1];
        nDummy  = nGegner;
    }

    BYTE nMauer;
    Point aPoint;

    if(nLevel < WAVES*2)
        nMauer = (BYTE)waves[nWert-1][nGegner*3+1];
    else
        nMauer = 0xff;
    if(nMauer & 0x01)
    {
        aPoint = Point(0,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x02)
    {
        aPoint = Point(32,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x04)
    {
        aPoint = Point(64,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x08)
    {
        aPoint = Point(96,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x10)
    {
        aPoint = Point(128,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x20)
    {
        aPoint = Point(160,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x40)
    {
        aPoint = Point(192,340);
        pWall->InsertWall(aPoint);
    }
    if(nMauer & 0x80)
    {
        aPoint = Point(224,340);
        pWall->InsertWall(aPoint);
    }
    if(nLevel < WAVES*2)
        nMauer = (BYTE)waves[nWert-1][nGegner*3+2];
    else
        nMauer = 0xff;
    if(nMauer & 0x01)
        pWall->InsertWall(Point(256,340));
    if(nMauer & 0x02)
        pWall->InsertWall(Point(288,340));
    if(nMauer & 0x04)
        pWall->InsertWall(Point(320,340));
    if(nMauer & 0x08)
        pWall->InsertWall(Point(352,340));
    if(nMauer & 0x10)
        pWall->InsertWall(Point(384,340));
    if(nMauer & 0x20)
        pWall->InsertWall(Point(416,340));
    if(nMauer & 0x40)
        pWall->InsertWall(Point(448,340));
    if(nMauer & 0x80)
        pWall->InsertWall(Point(480,340));
    if(nLevel < WAVES*2)
        nMauer = (BYTE)waves[nWert-1][nGegner*3+3];
    else
        nMauer = 0xff;
    if(nMauer & 0x01)
        pWall->InsertWall(Point(512,340));
    if(nMauer & 0x02)
        pWall->InsertWall(Point(544,340));
    if(nMauer & 0x04)
        pWall->InsertWall(Point(576,340));
    if(nMauer & 0x08)
        pWall->InsertWall(Point(608,340));

    nGegner = nDummy;

}

void MyWindow::RandomLevel()
{
    Time aTime;

    srand(aTime.GetTime() % 1000);

    long nPos = ranpos[0];
    while(!pGegner->Count())
    {
        for(long i=0; i<nPos ;i++)
        {

            int nRan = rand() % 40;

            if(nRan < 3)
                pGegner->InsertGegner(GEGNER5,ranpos[i*2+1],ranpos[i*2+2]);
            if(nRan > 2 && nRan < 9)
                pGegner->InsertGegner(GEGNER4,ranpos[i*2+1],ranpos[i*2+2]);
            if(nRan > 8 && nRan < 16)
                pGegner->InsertGegner(GEGNER3,ranpos[i*2+1],ranpos[i*2+2]);
            if(nRan > 15 && nRan < 27)
                pGegner->InsertGegner(GEGNER2,ranpos[i*2+1],ranpos[i*2+2]);
            if(nRan > 26 && nRan < 41)
                pGegner->InsertGegner(GEGNER1,ranpos[i*2+1],ranpos[i*2+2]);
        }
    }

    levelpuf[0] = nLevel;
    levelpuf[1] = pGegner->Count();
    unsigned long i;
    for(i=0; i<pGegner->Count(); i++)
    {
        levelpuf[i*3+2] = pGegner->GegType(i);
        levelpuf[i*3+3] = pGegner->GegnerX(i);
        levelpuf[i*3+4] = pGegner->GegnerY(i);
    }
}

