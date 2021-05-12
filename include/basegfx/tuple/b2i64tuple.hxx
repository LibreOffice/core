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

#pragma once

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/tuple/Tuple2D.hxx>

namespace basegfx
{
    /** Base class for all Points/Vectors with two sal_Int64 values

        This class provides all methods common to Point
        and Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two sal_Int64 values
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B2I64Tuple : public Tuple2D<sal_Int64>
    {
    public:
        /** Create a 2D Tuple

            The tuple is initialized to (0, 0)
        */
        B2I64Tuple()
            : Tuple2D(0, 0)
        {}

        /** Create a 2D Tuple

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 2D Tuple.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 2D Tuple.
        */
        B2I64Tuple(sal_Int64 nX, sal_Int64 nY)
            : Tuple2D(nX, nY)
        {}

        /** Create a copy of a 2D Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        B2I64Tuple(const B2I64Tuple& rTup)
            : Tuple2D(rTup.mnX, rTup.mnY)
        {}

        // operators

        B2I64Tuple operator-(void) const
        {
            return B2I64Tuple(-mnX, -mnY);
        }

        B2I64Tuple& operator=( const B2I64Tuple& rTup )
        {
            mnX = rTup.mnX;
            mnY = rTup.mnY;
            return *this;
        }
    };

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
