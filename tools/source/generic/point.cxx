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

#include <tools/gen.hxx>
#include <basegfx/numeric/ftools.hxx>

void Point::RotateAround( Point& rPoint,
                          Degree10 nOrientation ) const
{
    tools::Long nX = rPoint.X();
    tools::Long nY = rPoint.Y();
    RotateAround(nX, nY, nOrientation);
    rPoint.setX(nX);
    rPoint.setY(nY);
}

void Point::RotateAround( tools::Long& rX, tools::Long& rY,
                          Degree10 nOrientation ) const
{
    const tools::Long nOriginX = X();
    const tools::Long nOriginY = Y();

    if ( (nOrientation >= 0_deg10) && !(nOrientation % 900_deg10) )
    {
        if ( nOrientation >= 3600_deg10 )
            nOrientation %= 3600_deg10;

        if ( nOrientation )
        {
            rX -= nOriginX;
            rY -= nOriginY;

            if ( nOrientation == 900_deg10 )
            {
                tools::Long nTemp = rX;
                rX = rY;
                rY = -nTemp;
            }
            else if ( nOrientation == 1800_deg10 )
            {
                rX = -rX;
                rY = -rY;
            }
            else /* ( nOrientation == 2700 ) */
            {
                tools::Long nTemp = rX;
                rX = -rY;
                rY = nTemp;
            }

            rX += nOriginX;
            rY += nOriginY;
        }
    }
    else
    {
        double nRealOrientation = nOrientation.get() * F_PI1800;
        double nCos = cos( nRealOrientation );
        double nSin = sin( nRealOrientation );

        // Translation...
        tools::Long nX = rX-nOriginX;
        tools::Long nY = rY-nOriginY;

        // Rotation...
        rX = + static_cast<tools::Long>(nCos*nX + nSin*nY) + nOriginX;
        rY = - static_cast<tools::Long>(nSin*nX - nCos*nY) + nOriginY;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
