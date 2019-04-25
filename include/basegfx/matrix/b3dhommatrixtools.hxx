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

#include <basegfx/basegfxdllapi.h>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace drawing
{
struct HomogenMatrix;
}
}
}
}

namespace basegfx
{
namespace utils
{
/* tooling methods for converting API matrices (drawing::HomogenMatrix) to
    B3DHomMatrix. drawing::HomogenMatrix4 is not used by OOo
 */
BASEGFX_DLLPUBLIC B3DHomMatrix
UnoHomogenMatrixToB3DHomMatrix(const com::sun::star::drawing::HomogenMatrix& rMatrixIn);

BASEGFX_DLLPUBLIC void
B3DHomMatrixToUnoHomogenMatrix(const B3DHomMatrix& rMatrixIn,
                               com::sun::star::drawing::HomogenMatrix& rMatrixOut);

} // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
