/*************************************************************************
 *
 *  $RCSfile: svdhlpln.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "svdhlpln.hxx"
#include "svdio.hxx"

#ifndef _GDIOBJ_HXX //autogen
#include <vcl/gdiobj.hxx>
#endif

#ifndef _COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

Pointer SdrHelpLine::GetPointer() const
{
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : return Pointer(POINTER_ESIZE); break;
        case SDRHELPLINE_HORIZONTAL: return Pointer(POINTER_SSIZE); break;
        default                    : return Pointer(POINTER_MOVE);  break;
    } // switch
}

void SdrHelpLine::Draw(OutputDevice& rOut, const Point& rOfs) const
{
    Point aPnt(rOut.LogicToPixel(aPos+rOfs));
    long x=aPnt.X();
    long y=aPnt.Y();
    long nMin=0;
    long nMax=2048;
    FASTBOOL bMap0=rOut.IsMapModeEnabled();
    rOut.EnableMapMode(FALSE);
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  :
        {
            if (rOut.GetOutDevType() == OUTDEV_WINDOW)
            {
                Point aLine[2];
                aLine[0].X() = x; aLine[0].Y() = nMin;
                aLine[1].X() = x; aLine[1].Y() = nMax;
                ((Window&) rOut).InvertTracking(Polygon(2, aLine), SHOWTRACK_WINDOW);
            }
            else
            {
                rOut.DrawLine(Point(x,nMin),Point(x,nMax));
            }
        }
        break;

        case SDRHELPLINE_HORIZONTAL:
        {
            if (rOut.GetOutDevType() == OUTDEV_WINDOW)
            {
                Point aLine[2];
                aLine[0].X() = nMin; aLine[0].Y() = y;
                aLine[1].X() = nMax; aLine[1].Y() = y;
                ((Window&) rOut).InvertTracking(Polygon(2, aLine), SHOWTRACK_WINDOW);
            }
            else
            {
                rOut.DrawLine(Point(nMin,y),Point(nMax,y));
            }
        }
        break;

        case SDRHELPLINE_POINT     :
        {
            int r=SDRHELPLINE_POINT_PIXELSIZE;

            if (rOut.GetOutDevType() == OUTDEV_WINDOW)
            {
                Point aHLine[2];
                aHLine[0].X() = x-r; aHLine[0].Y() = y;
                aHLine[1].X() = x+r; aHLine[1].Y() = y;
                ((Window&) rOut).InvertTracking(Polygon(2, aHLine), SHOWTRACK_WINDOW);

                Point aVLine[2];
                aVLine[0].X() = x; aVLine[0].Y() = y-r;
                aVLine[1].X() = x; aVLine[1].Y() = y+r;
                ((Window&) rOut).InvertTracking(Polygon(2, aVLine), SHOWTRACK_WINDOW);
            }
            else
            {
                rOut.DrawLine(Point(x-r,y),Point(x+r,y));
                rOut.DrawLine(Point(x,y-r),Point(x,y+r));
            }
        }
        break;

    } // switch
    rOut.EnableMapMode(bMap0);
}

FASTBOOL SdrHelpLine::IsHit(const Point& rPnt, USHORT nTolLog, const OutputDevice& rOut) const
{
    Size a1Pix(rOut.PixelToLogic(Size(1,1)));
    FASTBOOL bXHit=rPnt.X()>=aPos.X()-nTolLog && rPnt.X()<=aPos.X()+nTolLog+a1Pix.Width();
    FASTBOOL bYHit=rPnt.Y()>=aPos.Y()-nTolLog && rPnt.Y()<=aPos.Y()+nTolLog+a1Pix.Height();
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : return bXHit; break;
        case SDRHELPLINE_HORIZONTAL: return bYHit; break;
        case SDRHELPLINE_POINT: {
            if (bXHit || bYHit) {
                Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
                return rPnt.X()>=aPos.X()-aRad.Width() && rPnt.X()<=aPos.X()+aRad.Width()+a1Pix.Width() &&
                       rPnt.Y()>=aPos.Y()-aRad.Height() && rPnt.Y()<=aPos.Y()+aRad.Height()+a1Pix.Height();
            }
        } break;
    } // switch
    return FALSE;
}

Rectangle SdrHelpLine::GetBoundRect(const OutputDevice& rOut) const
{
    Rectangle aRet(aPos,aPos);
    Point aOfs(rOut.GetMapMode().GetOrigin());
    Size aSiz(rOut.GetOutputSize());
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : aRet.Top()=-aOfs.Y(); aRet.Bottom()=-aOfs.Y()+aSiz.Height(); break;
        case SDRHELPLINE_HORIZONTAL: aRet.Left()=-aOfs.X(); aRet.Right()=-aOfs.X()+aSiz.Width();  break;
        case SDRHELPLINE_POINT     : {
            Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
            aRet.Left()  -=aRad.Width();
            aRet.Right() +=aRad.Width();
            aRet.Top()   -=aRad.Height();
            aRet.Bottom()+=aRad.Height();
        } break;
    } // switch
    return aRet;
}

SvStream& operator<<(SvStream& rOut, const SdrHelpLine& rHL)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOHlpLID);
    rOut<<UINT16(rHL.eKind);
    rOut<<rHL.aPos;
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrHelpLine& rHL)
{
    SdrIOHeader aHead(rIn,STREAM_READ);
    UINT16 nDum;
    rIn>>nDum;
    rHL.eKind=(SdrHelpLineKind)nDum;
    rIn>>rHL.aPos;
    return rIn;
}

void SdrHelpLineList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

void SdrHelpLineList::operator=(const SdrHelpLineList& rSrcList)
{
    Clear();
    USHORT nAnz=rSrcList.GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        Insert(rSrcList[i]);
    }
}

FASTBOOL SdrHelpLineList::operator==(const SdrHelpLineList& rSrcList) const
{
    FASTBOOL bEqual=FALSE;
    USHORT nAnz=GetCount();
    if (nAnz==rSrcList.GetCount()) {
        bEqual=TRUE;
        for (USHORT i=0; i<nAnz && bEqual; i++) {
            if (*GetObject(i)!=*rSrcList.GetObject(i)) {
                bEqual=FALSE;
            }
        }
    }
    return bEqual;
}

void SdrHelpLineList::DrawAll(OutputDevice& rOut, const Point& rOfs) const
{
    Color aOldLineColor( rOut.GetLineColor() );

    rOut.SetLineColor( Color( COL_GREEN ) );

    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++)
        GetObject(i)->Draw(rOut,rOfs);

    rOut.SetLineColor( aOldLineColor );
}

USHORT SdrHelpLineList::HitTest(const Point& rPnt, USHORT nTolLog, const OutputDevice& rOut) const
{
    USHORT nAnz=GetCount();
    for (USHORT i=nAnz; i>0;) {
        i--;
        if (GetObject(i)->IsHit(rPnt,nTolLog,rOut)) return i;
    }
    return SDRHELPLINE_NOTFOUND;
}

SvStream& operator<<(SvStream& rOut, const SdrHelpLineList& rHLL)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOHLstID);
    USHORT nAnz=rHLL.GetCount();
    rOut<<nAnz;
    for (USHORT i=0; i<nAnz; i++) {
        rOut<<rHLL[i];
    }
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrHelpLineList& rHLL)
{
    SdrIOHeader aHead(rIn,STREAM_READ);
    rHLL.Clear();
    USHORT nAnz;
    rIn>>nAnz;
    for (USHORT i=0; i<nAnz; i++) {
        SdrHelpLine* pHL=new SdrHelpLine;
        rIn>>*pHL;
        rHLL.aList.Insert(pHL,CONTAINER_APPEND);
    }
    return rIn;
}

