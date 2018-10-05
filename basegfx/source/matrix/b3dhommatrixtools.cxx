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

#include <basegfx/matrix/b3dhommatrixtools.hxx>
#include <com/sun/star/drawing/HomogenMatrix.hpp>

namespace basegfx
{
namespace utils
{
B3DHomMatrix UnoHomogenMatrixToB3DHomMatrix(const com::sun::star::drawing::HomogenMatrix& rMatrixIn)
{
    B3DHomMatrix aRetval;

    aRetval.set(0, 0, rMatrixIn.Line1.Column1);
    aRetval.set(0, 1, rMatrixIn.Line1.Column2);
    aRetval.set(0, 2, rMatrixIn.Line1.Column3);
    aRetval.set(0, 3, rMatrixIn.Line1.Column4);
    aRetval.set(1, 0, rMatrixIn.Line2.Column1);
    aRetval.set(1, 1, rMatrixIn.Line2.Column2);
    aRetval.set(1, 2, rMatrixIn.Line2.Column3);
    aRetval.set(1, 3, rMatrixIn.Line2.Column4);
    aRetval.set(2, 0, rMatrixIn.Line3.Column1);
    aRetval.set(2, 1, rMatrixIn.Line3.Column2);
    aRetval.set(2, 2, rMatrixIn.Line3.Column3);
    aRetval.set(2, 3, rMatrixIn.Line3.Column4);
    aRetval.set(3, 0, rMatrixIn.Line4.Column1);
    aRetval.set(3, 1, rMatrixIn.Line4.Column2);
    aRetval.set(3, 2, rMatrixIn.Line4.Column3);
    aRetval.set(3, 3, rMatrixIn.Line4.Column4);

    return aRetval;
}

void B3DHomMatrixToUnoHomogenMatrix(const B3DHomMatrix& rMatrixIn,
                                    com::sun::star::drawing::HomogenMatrix& rMatrixOut)
{
    rMatrixOut.Line1.Column1 = rMatrixIn.get(0, 0);
    rMatrixOut.Line1.Column2 = rMatrixIn.get(0, 1);
    rMatrixOut.Line1.Column3 = rMatrixIn.get(0, 2);
    rMatrixOut.Line1.Column4 = rMatrixIn.get(0, 3);
    rMatrixOut.Line2.Column1 = rMatrixIn.get(1, 0);
    rMatrixOut.Line2.Column2 = rMatrixIn.get(1, 1);
    rMatrixOut.Line2.Column3 = rMatrixIn.get(1, 2);
    rMatrixOut.Line2.Column4 = rMatrixIn.get(1, 3);
    rMatrixOut.Line3.Column1 = rMatrixIn.get(2, 0);
    rMatrixOut.Line3.Column2 = rMatrixIn.get(2, 1);
    rMatrixOut.Line3.Column3 = rMatrixIn.get(2, 2);
    rMatrixOut.Line3.Column4 = rMatrixIn.get(2, 3);
    rMatrixOut.Line4.Column1 = rMatrixIn.get(3, 0);
    rMatrixOut.Line4.Column2 = rMatrixIn.get(3, 1);
    rMatrixOut.Line4.Column3 = rMatrixIn.get(3, 2);
    rMatrixOut.Line4.Column4 = rMatrixIn.get(3, 3);
}
} // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
