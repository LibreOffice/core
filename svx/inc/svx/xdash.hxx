/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xdash.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:49:21 $
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

#ifndef _SVX_XDASH_HXX
#define _SVX_XDASH_HXX

//-------------
// class XDash
//-------------

#ifndef _XENUM_HXX //autogen
#include <svx/xenum.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <vector>

class SVX_DLLPUBLIC XDash
{
protected:
    XDashStyle  eDash;
    USHORT      nDots;
    ULONG       nDotLen;
    USHORT      nDashes;
    ULONG       nDashLen;
    ULONG       nDistance;

public:
          XDash(XDashStyle eDash = XDASH_RECT,
                USHORT nDots = 1, ULONG nDotLen = 20,
                USHORT nDashes = 1, ULONG nDashLen = 20, ULONG nDistance = 20);

    bool operator==(const XDash& rDash) const;

    void SetDashStyle(XDashStyle eNewStyle) { eDash = eNewStyle; }
    void SetDots(USHORT nNewDots)           { nDots = nNewDots; }
    void SetDotLen(ULONG nNewDotLen)        { nDotLen = nNewDotLen; }
    void SetDashes(USHORT nNewDashes)       { nDashes = nNewDashes; }
    void SetDashLen(ULONG nNewDashLen)      { nDashLen = nNewDashLen; }
    void SetDistance(ULONG nNewDistance)    { nDistance = nNewDistance; }

    XDashStyle  GetDashStyle() const        { return eDash; }
    USHORT      GetDots() const             { return nDots; }
    ULONG       GetDotLen() const           { return nDotLen; }
    USHORT      GetDashes() const           { return nDashes; }
    ULONG       GetDashLen() const          { return nDashLen; }
    ULONG       GetDistance() const         { return nDistance; }

    // XDash is translated into an array of doubles which describe the lengths of the
    // dashes, dots and empty passages. It returns the complete length of the full DashDot
    // sequence and fills the given vetor of doubles accordingly (also resizing, so deleting it).
    double CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const;
};

#endif
