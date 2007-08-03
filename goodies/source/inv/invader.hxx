/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: invader.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:29:20 $
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
#ifndef _INVADER_HXX
#define _INVADER_HXX

#define WINWIDTH 640
#define WINHEIGHT 480
#define LEVELTEXTX 280
#define LEVELTEXTY 200

#define TIMEHIGH    100
#define TIMELOW     30
#define AUSEMODE    50

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX //autogen
#include <tools/resmgr.hxx>
#endif
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif


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
