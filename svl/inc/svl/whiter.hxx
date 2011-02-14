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
#ifndef _SFX_WHITER_HXX
#define _SFX_WHITER_HXX

#include "svl/svldllapi.h"

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif
#include <tools/solar.h>

class SfxItemSet;


// INCLUDE ---------------------------------------------------------------

class SVL_DLLPUBLIC SfxWhichIter
{
    const sal_uInt16 *pRanges, *pStart;
    sal_uInt16 nOfst, nFrom, nTo;

public:
    SfxWhichIter( const SfxItemSet& rSet, sal_uInt16 nFrom = 0, sal_uInt16 nTo = USHRT_MAX );
    ~SfxWhichIter();

    sal_uInt16  GetCurWhich() const { return *pRanges + nOfst; }
    sal_uInt16  NextWhich();
    sal_uInt16  PrevWhich();

    sal_uInt16  FirstWhich();
    sal_uInt16  LastWhich();
};

#endif
