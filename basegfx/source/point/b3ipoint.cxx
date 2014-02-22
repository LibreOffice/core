/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/point/b3ipoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B3IPoint& B3IPoint::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        double fTempX(
            rMat.get(0, 0) * mnX +
            rMat.get(0, 1) * mnY +
            rMat.get(0, 2) * mnZ +
            rMat.get(0, 3));
        double fTempY(
            rMat.get(1, 0) * mnX +
            rMat.get(1, 1) * mnY +
            rMat.get(1, 2) * mnZ +
            rMat.get(1, 3));
        double fTempZ(
            rMat.get(2, 0) * mnX +
            rMat.get(2, 1) * mnY +
            rMat.get(2, 2) * mnZ +
            rMat.get(2, 3));

        if(!rMat.isLastLineDefault())
        {
            const double fOne(1.0);
            const double fTempM(
                rMat.get(3, 0) * mnX +
                rMat.get(3, 1) * mnY +
                rMat.get(3, 2) * mnZ +
                rMat.get(3, 3));

            if(!fTools::equalZero(fTempM) && !fTools::equal(fOne, fTempM))
            {
                fTempX /= fTempM;
                fTempY /= fTempM;
                fTempZ /= fTempM;
            }
        }

        mnX = fround(fTempX);
        mnY = fround(fTempY);
        mnZ = fround(fTempZ);

        return *this;
    }
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
