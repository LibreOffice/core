/*************************************************************************
 *
 *  $RCSfile: invader.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:09 $
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
        virtual void        Main(int,char*[]);
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
};

#endif
