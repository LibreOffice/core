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
#ifndef _SVX_PAPERINF_HXX
#define _SVX_PAPERINF_HXX

// INCLUDE ---------------------------------------------------------------

#include <vcl/print.hxx>
#include <vcl/mapmod.hxx>
#include <editeng/svxenum.hxx>
#include "editeng/editengdllapi.h"

// forward ---------------------------------------------------------------

class Printer;
class Size;
class String;

// class SvxPaperInfo -----------------------------------------------------

class EDITENG_DLLPUBLIC SvxPaperInfo
{
public:
    static Size     GetDefaultPaperSize( MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( Paper ePaper, MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( const Printer* pPrinter );
    static Paper    GetSvxPaper( const Size &rSize, MapUnit eUnit = MAP_TWIP, bool bSloppy = sal_False );
    static long     GetSloppyPaperDimension( long nSize, MapUnit eUnit = MAP_TWIP );
    static String   GetName( Paper ePaper );
};

// INLINE -----------------------------------------------------------------

inline Size &Swap(Size &rSize)
{
    const long lVal = rSize.Width();
    rSize.Width() = rSize.Height();
    rSize.Height() = lVal;
    return rSize;
}

inline Size &LandscapeSwap(Size &rSize)
{
    if ( rSize.Height() > rSize.Width() )
        Swap( rSize );
    return rSize;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
