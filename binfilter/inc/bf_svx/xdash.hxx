/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SVX_XDASH_HXX
#define _SVX_XDASH_HXX

//-------------
// class XDash
//-------------

#include <bf_svx/xenum.hxx>
namespace binfilter {

class XDash
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

    int operator==(const XDash& rDash) const;

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

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
