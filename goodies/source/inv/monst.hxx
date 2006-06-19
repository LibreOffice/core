/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: monst.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 21:51:27 $
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
#ifndef _MONST_HXX
#define _MONST_HXX

enum GegnerType { GEGNER1, GEGNER2, GEGNER3, GEGNER4, GEGNER5 };
enum GegnerMode { MOVE1, MOVE2, MOVE3, MOVE4, MOVE5, DELETED, HIDE };

#define DIFFX   20
#define DIFFY   20
#define MOVEX   10
#define MOVEY   10
#define KOLLXY  2

#include <tools/list.hxx>
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

struct Gegner_Impl
{
    Point       aXY;
    long        nRow;
    long        aX;
    long        nDiffX;
    long        nPoints;
    long        nMaxHits;
    long        nHits;
    long        nDelay;
    GegnerType  aType;
    GegnerMode  aMode;
    Rectangle   aKoll;
};

DECLARE_LIST(GegnerListe,Gegner_Impl*)

class Explosion;
class Bombe;
class Fighter;

class Gegner : public GegnerListe
{
    private:
        long        nRows;
        long        nGegnerRow;
        long        nMaxX;
        Size        aOutSize;
        Image*      pBitMonst1;
        Image*      pBitMonst2;
        Image*      pBitMonst3;
        Image*      pBitMonst4;
        Image*      pBitMonst1b;
        Image*      pBitMonst2b;
        Image*      pBitMonst3b;
        Image*      pBitMonst4b;
        Image*      pBitMonst5;
        Image*      pBitMonst5a;
        Image*      pBitMonst5b;
        Bombe*      pBombe;
        Fighter*    pFighter;

        BOOL        bDown;
        BOOL        bLeft;
        BOOL        bAuseMode;
        long        nRandWert;
        long        nDown;

    public:
                    Gegner(Fighter* pFighter, Bombe* pBombe, ResMgr* pRes);
                    ~Gegner();

        void        InsertGegner(USHORT nType, USHORT x, USHORT y);
        void        DrawGegner(OutputDevice* pDev, Point* pStart);
        void        Move();
        void        SetRandWert(long nWert) { nRandWert = nWert; }
        long        GetRandWert() { return nRandWert; }
        long        GegnerX(long nWert) { return GetObject(nWert)->aXY.X(); }
        long        GegnerXStart(long nWert) { return GetObject(nWert)->aX; }
        long        GegnerY(long nWert) { return GetObject(nWert)->aXY.Y(); }
        GegnerType  GegType(long nWert)
                        { return GetObject(nWert)->aType; }
        GegnerMode  GegMode(long nWert)
                        { return GetObject(nWert)->aMode; }
        void        SetMode(long nWert, enum GegnerMode nMode)
                        { GetObject(nWert)->aMode = nMode; }
        void        SetGegnerPos(long nWert, const Point& rPoint)
                        { GetObject(nWert)->aXY = rPoint;}
        long        GegnerDiffX(long nWert) { return GetObject(nWert)->nDiffX; }
        Rectangle&  GegnerKoll(long nWert) { return GetObject(nWert)->aKoll; }
        Rectangle   GetKoll(long nWert);
        void        SetKoll(long nWert, const Rectangle& rRect)
                        { GetObject(nWert)->aKoll = rRect; }
        long        Kollision(Rectangle& rRect, Explosion* pExplosion);
        Point&      GegnerPos(long nWert) { return GetObject(nWert)->aXY; }
        BOOL        RemoveGegner();
        void        ClearAll();
        long        GegnerPoints(long nWert) { return GetObject(nWert)->nPoints; }
        void        GegnerHit(long nWert) { GetObject(nWert)->nHits++; }
        BOOL        GegnerDest(long nWert);
        void        SetDown(long nWert) { nDown = nWert; }
        long        GetDown() { return nDown; }
        void        SetAuseMode(BOOL bFlag) { bAuseMode = bFlag; }
        void        SetDelay(long nWert) { GetObject(nWert)->nDelay = 3; }
        long        GetDelay(long nWert) { return GetObject(nWert)->nDelay; }
        void        DecDelay(long nWert) { GetObject(nWert)->nDelay--; }
};

#endif
