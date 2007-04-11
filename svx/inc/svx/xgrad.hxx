/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xgrad.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:55:46 $
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

#ifndef _SVX__XGRADIENT_HXX
#define _SVX__XGRADIENT_HXX

#include <svx/xenum.hxx>

#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//-----------------
// class XGradient
//-----------------

class SVX_DLLPUBLIC XGradient
{
protected:
    XGradientStyle  eStyle;
    Color           aStartColor;
    Color           aEndColor;
    long            nAngle;
    USHORT          nBorder;
    USHORT          nOfsX;
    USHORT          nOfsY;
    USHORT          nIntensStart;
    USHORT          nIntensEnd;
    USHORT          nStepCount;

public:
    XGradient();
    XGradient( const Color& rStart, const Color& rEnd,
               XGradientStyle eStyle = XGRAD_LINEAR, long nAngle = 0,
               USHORT nXOfs = 50, USHORT nYOfs = 50, USHORT nBorder = 0,
               USHORT nStartIntens = 100, USHORT nEndIntens = 100,
               USHORT nSteps = 0 );

    bool operator==(const XGradient& rGradient) const;

    void SetGradientStyle(XGradientStyle eNewStyle) { eStyle = eNewStyle; }
    void SetStartColor(const Color& rColor)         { aStartColor = rColor; }
    void SetEndColor(const Color& rColor)           { aEndColor = rColor; }
    void SetAngle(long nNewAngle)                   { nAngle = nNewAngle; }
    void SetBorder(USHORT nNewBorder)               { nBorder = nNewBorder; }
    void SetXOffset(USHORT nNewOffset)              { nOfsX = nNewOffset; }
    void SetYOffset(USHORT nNewOffset)              { nOfsY = nNewOffset; }
    void SetStartIntens(USHORT nNewIntens)          { nIntensStart = nNewIntens; }
    void SetEndIntens(USHORT nNewIntens)            { nIntensEnd = nNewIntens; }
    void SetSteps(USHORT nSteps)                    { nStepCount = nSteps; }

    XGradientStyle GetGradientStyle() const         { return eStyle; }
    Color          GetStartColor() const            { return aStartColor; }
    Color          GetEndColor() const              { return aEndColor; }
    long           GetAngle() const                 { return nAngle; }
    USHORT         GetBorder() const                { return nBorder; }
    USHORT         GetXOffset() const               { return nOfsX; }
    USHORT         GetYOffset() const               { return nOfsY; }
    USHORT         GetStartIntens() const           { return nIntensStart; }
    USHORT         GetEndIntens() const             { return nIntensEnd; }
    USHORT         GetSteps() const                 { return nStepCount; }
};

#endif
