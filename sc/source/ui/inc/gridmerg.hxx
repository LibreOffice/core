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

#ifndef SC_GRIDMERG_HXX
#define SC_GRIDMERG_HXX

#include <tools/solar.h>

class OutputDevice;

class ScGridMerger
{
private:
    OutputDevice*   pDev;
    long            nOneX;
    long            nOneY;
    long            nFixStart;
    long            nFixEnd;
    long            nVarStart;
    long            nVarDiff;
    long            nCount;
    sal_Bool            bVertical;
    sal_Bool            bOptimize;

    void        AddLine( long nStart, long nEnd, long nPos );

public:
                ScGridMerger( OutputDevice* pOutDev, long nOnePixelX, long nOnePixelY );
                ~ScGridMerger();

    void        AddHorLine( long nX1, long nX2, long nY );
    void        AddVerLine( long nX, long nY1, long nY2 );
    void        Flush();
};



#endif


