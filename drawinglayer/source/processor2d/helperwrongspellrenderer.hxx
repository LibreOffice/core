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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;

namespace drawinglayer { namespace primitive2d {
    class WrongSpellPrimitive2D;
}}

namespace basegfx {
    class B2DHomMatrix;
    class BColorModifierStack;
}

//////////////////////////////////////////////////////////////////////////////
// support WrongSpell rendreing using VCL from primitives due to VCLs nice
// and fast solution with wavelines

namespace drawinglayer
{
    bool renderWrongSpellPrimitive2D(
        const primitive2d::WrongSpellPrimitive2D& rWrongSpellCandidate,
        OutputDevice& rOutputDevice,
        const basegfx::B2DHomMatrix& rObjectToViewTransformation,
        const basegfx::BColorModifierStack& rBColorModifierStack);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
