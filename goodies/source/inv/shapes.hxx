/*************************************************************************
 *
 *  $RCSfile: shapes.hxx,v $
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
#ifndef _SHAPES_HXX
#define _SHAPES_HXX


#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#define KANONEX     10
#define BOMBEY      20
#define MUNIXY      10
#define BOMBXY      5
#define WALLY       400
#define WALLKOLL    8

Image* ImplLoadImage( USHORT nId, ResMgr* pRes );

enum FighterMode { FIGHT_NORM, FIGHT_RIGHT, FIGHT_LEFT, FIGHT_DEST};

class Explosion;

class Fighter
{
    private:
        Image*      pFightr;
        Image*      pFightl;
        Point       aOut;
        Point       aStart;
        long        nMax;
        FighterMode eMode;
        BOOL        bNoKey;
        Size        aSize;
        long        nDelay;

    public:

        Image*      pFight1;

                    Fighter(Size& rOut, ResMgr* pRes);
                    ~Fighter();
        void        Paint(OutputDevice& pDev);
        void        Move(long nDirect);
        Point&      GetPoint() { return aOut; }
        BOOL        Kollision(Rectangle, Explosion* pExpl);
        void        Init();
        void        NoFighter() { eMode = FIGHT_DEST; }
        long        GetHalf() { return aOut.X()+aSize.Width()/2; }
};

enum MuniMode { MUNI_DEL, MUNI_MODE1, MUNI_MODE2 };

#define MUNIY       10
#define MUNMAX      5

struct Munition_Impl
{
    MuniMode    eMode;
    Point       aPoint;
    Rectangle   aKoll;
};

DECLARE_LIST(MunitionListe,Munition_Impl*);

class Munition : public MunitionListe
{
    private:
        Image*      pMunition2;
        Size        aSize;

    public:

        Image*      pMunition1;

                    Munition(ResMgr* pRes);
                    ~Munition();
        void        Paint(OutputDevice& rDev);
        void        Start(Point& rPoint);
        void        SetDelete(long nWert) { GetObject(nWert)->eMode = MUNI_DEL; }
        Rectangle   GetKoll(long nWert) { return GetObject(nWert)->aKoll; }
        void        SetKoll(long nWert, Rectangle aRect)
                        { GetObject(nWert)->aKoll = aRect; }
        MuniMode    GetMode(long nWert) { return GetObject(nWert)->eMode; }
        void        SetMode(long nWert, enum MuniMode mode)
                        { GetObject(nWert)->eMode = mode; }
        void        SetPoint(long nWert, const Point& rPoint)
                        { GetObject(nWert)->aPoint = rPoint; }
        Point       GetPoint(long nWert) { return GetObject(nWert)->aPoint; }
        long        RemoveMunition();
        void        ClearAll();
};

struct Bombe_Impl
{
    Point           aXY;
    Rectangle       aKoll;
    BOOL            bDelete;
};

DECLARE_LIST(BombenListe, Bombe_Impl*);

class Bombe : public BombenListe
{
    private:
        Image*      pBombe;
        Size        aSize;
        long        nSpeed;

    public:
                    Bombe(ResMgr* pRes);
                    ~Bombe();
        void        Paint(OutputDevice& rDev);
        Point&      GetPoint(long nWert) { return GetObject(nWert)->aXY; }
        void        SetPoint(long nWert, const Point& rPoint)
                        { GetObject(nWert)->aXY = rPoint; }
        void        InsertBombe(const Point& rPoint);
        Rectangle   GetKoll(long nWert) { return GetObject(nWert)->aKoll; }
        void        SetKoll(long nWert, Rectangle aRect)
                        { GetObject(nWert)->aKoll = aRect; }
        void        SetDelete(long nWert) { GetObject(nWert)->bDelete = TRUE; }
        void        RemoveBomben();
        void        SetSpeed(long nWert) { nSpeed = nWert; }
        void        ClearAll();
        BOOL        Kollision(Rectangle aRect, Explosion* pExpl);
};

enum WallMode { WALL_MOD1, WALL_MOD2, WALL_MOD3, WALL_MOD4,  WALL_MOD5,
                 WALL_MOD6, WALL_MOD7, WALL_MOD8, WALL_MOD9, WALL_MOD10,
                 WALL_DEL };
struct Wall_Impl
{
    Point           aXY;
    WallMode        eMode;
};

DECLARE_LIST(WallListe, Wall_Impl*);

class Wall : public WallListe
{
    private:
        Image*      pWall1;
        Image*      pWall2;
        Image*      pWall3;
        Image*      pWall4;
        Image*      pWall5;
        Image*      pWall6;
        Image*      pWall7;
        Image*      pWall8;
        Image*      pWall9;
        Image*      pWall10;
        Size        aSize;

    public:
                    Wall(ResMgr* pRes);
                    ~Wall();

        void        Paint(OutputDevice& pDev);
        WallMode    GetMode(long nWert) { return GetObject(nWert)->eMode; }
        void        SetMode(long nWert, enum WallMode Mode)
                        { GetObject(nWert)->eMode = Mode; }
        Point       GetPoint(long nWert) { return GetObject(nWert)->aXY; }
        void        InsertWall(const Point& rPoint);
        void        ClearAll();
        BOOL        Kollision(Rectangle& rRect, BOOL bDel);
};

#endif
