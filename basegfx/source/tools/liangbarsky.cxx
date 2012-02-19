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

#include "basegfx/tools/tools.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/range/b2drange.hxx"


namespace basegfx
{
    namespace tools
    {
        namespace
        {
            // see Foley/vanDam, pp. 122 for the Liang-Barsky line
            // clipping algorithm
            inline bool liangBarskyClipT( double  nDenom,
                                          double  nNumerator,
                                          double& io_rTE,
                                          double& io_rTL )
            {
                double t;
                if( nDenom > 0 )
                {
                    t = nNumerator / nDenom;
                    if( t > io_rTL )
                        return false;
                    else if( t > io_rTE )
                        io_rTE = t;
                }
                else if( nDenom < 0 )
                {
                    t = nNumerator / nDenom;
                    if( t < io_rTE )
                        return false;
                    else
                        io_rTL = t;
                }
                else if( nNumerator > 0 )
                {
                    return false;
                }

                return true;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
