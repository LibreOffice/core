/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: whiter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:16:02 $
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
#ifndef _SFX_WHITER_HXX
#define _SFX_WHITER_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SfxItemSet;


// INCLUDE ---------------------------------------------------------------

class SVL_DLLPUBLIC SfxWhichIter
{
    const USHORT *pRanges, *pStart;
    USHORT nOfst, nFrom, nTo;

public:
    SfxWhichIter( const SfxItemSet& rSet, USHORT nFrom = 0, USHORT nTo = USHRT_MAX );
    ~SfxWhichIter();

    USHORT  GetCurWhich() const { return *pRanges + nOfst; }
    USHORT  NextWhich();
    USHORT  PrevWhich();

    USHORT  FirstWhich();
    USHORT  LastWhich();
};

#endif
