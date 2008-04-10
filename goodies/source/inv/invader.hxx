/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: invader.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _INVADER_HXX
#define _INVADER_HXX

#define WINWIDTH 640
#define WINHEIGHT 480
#define LEVELTEXTX 280
#define LEVELTEXTY 200

#define TIMEHIGH    100
#define TIMELOW     30
#define AUSEMODE    50

#include <vcl/timer.hxx>
#include <vcl/floatwin.hxx>
#include <tools/resmgr.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>


class Gegner;
class Fighter;
class Munition;
class Explosion;
class Bombe;
class Wall;
class ScoreWindow;
class MessBox;

class MyApp : public Application
{
    public:
        virtual void        Main();
};

class MyWindow : public FloatingWindow
{
    private:
        ResMgr*         pRes;
        BOOL            ProgStatus;
        BOOL            bStartLevel;
        BOOL            bEndLevel;
        BOOL            bFightDest;
        BOOL            bTimeHigh;
        BOOL            bPause;
        BOOL            bAuseModus;
        BOOL            bGetFighter;
        BOOL            bWaitDlg;

        long            nStartLevel;
        long            StartPhase;
        Pointer         aPointer;
        long            nLevel;
        long            nScore;
        long            nHighScore;
        long            TheHero;
        long            nFighter;
        long            nTimeOut;
        long            nAuseCount;

        Image*          pBitWelcome2;
        Image*          pBitHeros;
        Image*          pBitStarWars;
        Gegner*         pGegner;
        Fighter*        pFighter;
        Munition*       pMunition;
        Explosion*      pExplosion;
        Bombe*          pBombe;
        Wall*           pWall;
        Timer           aPaintTimer;
        Timer           aWaitDlgTimer;
        ScoreWindow*    pScoreWindow;
        VirtualDevice*  pVirtualDevice;
        Window*         pPauseWindow;
        MessBox*        pBox;

        void            Init();
        DECL_LINK( PaintTimer, Timer*);
        DECL_LINK( StartDlgTimer, Timer* );

        void            InitLevel();
        void            RandomLevel();

        long            nDirection;
        BOOL            bMouseMooving;
        void            PlaceDialog(MessBox* pBox);

    public:
                        MyWindow(Window* pParent, ResMgr* pRes);
                        ~MyWindow();

        virtual void    Paint(const Rectangle& rRect);
        virtual void    KeyInput( const KeyEvent& rKEvent);
        virtual void    MouseButtonDown(const MouseEvent& rMEvt);
//          virtual void    MouseButtonUp(const MouseEvent& rMEvt);
        virtual void    MouseMove(const MouseEvent& rMEvt);
        virtual BOOL    Close();

                void    EndLevel();
                void    Kollision();
        ResMgr*         GetResMgr(){return pRes;}

        void    setApplication(MyApp *rApplication) { pApplication = rApplication; };
        MyApp *pApplication;
};

#endif
