/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

// class SvxPaperInfo -----------------------------------------------------

class EDITENG_DLLPUBLIC SvxPaperInfo
{
public:
    static Size     GetDefaultPaperSize( MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( Paper ePaper, MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( const Printer* pPrinter );
    static Paper    GetSvxPaper( const Size &rSize, MapUnit eUnit = MAP_TWIP, bool bSloppy = sal_False );
    static long     GetSloppyPaperDimension( long nSize, MapUnit eUnit = MAP_TWIP );
    static OUString GetName( Paper ePaper );
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
