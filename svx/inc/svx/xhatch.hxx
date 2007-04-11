/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xhatch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:56:07 $
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

#ifndef _SVX_XHATCH_HXX
#define _SVX_XHATCH_HXX

#include <svx/xenum.hxx>

#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//--------------
// class XHatch
//--------------

class SVX_DLLPUBLIC XHatch
{
protected:
    XHatchStyle     eStyle;
    Color           aColor;
    long            nDistance;
    long            nAngle;

public:
                    XHatch() : eStyle(XHATCH_SINGLE), nDistance(0), nAngle(0) {}
                    XHatch(const Color& rCol, XHatchStyle eStyle = XHATCH_SINGLE,
                           long nDistance = 20, long nAngle = 0);

    bool operator==(const XHatch& rHatch) const;

    void            SetHatchStyle(XHatchStyle eNewStyle) { eStyle = eNewStyle; }
    void            SetColor(const Color& rColor) { aColor = rColor; }
    void            SetDistance(long nNewDistance) { nDistance = nNewDistance; }
    void            SetAngle(long nNewAngle) { nAngle = nNewAngle; }

    XHatchStyle     GetHatchStyle() const { return eStyle; }
    Color           GetColor() const { return aColor; }
    long            GetDistance() const { return nDistance; }
    long            GetAngle() const { return nAngle; }
};

#endif
