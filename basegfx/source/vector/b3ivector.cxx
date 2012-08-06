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

#include <basegfx/vector/b3ivector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
    B3IVector& B3IVector::operator*=( const B3DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX + rMat.get(0,1)*mnY + rMat.get(0,2)*mnZ );
        mnY = fround( rMat.get(1,0)*mnX + rMat.get(1,1)*mnY + rMat.get(1,2)*mnZ );
        mnZ = fround( rMat.get(2,0)*mnX + rMat.get(2,1)*mnY + rMat.get(2,2)*mnZ );

        return *this;
    }

    B3IVector operator*( const B3DHomMatrix& rMat, const B3IVector& rVec )
    {
        B3IVector aRes( rVec );
        return aRes*=rMat;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
