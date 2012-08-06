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

#ifndef _SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX
#define _SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SfxItemSet;

namespace drawinglayer { namespace attribute {
    class Sdr3DObjectAttribute;
}}

namespace basegfx {
    class B2DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // SdrAttribute creators
        attribute::Sdr3DObjectAttribute* createNewSdr3DObjectAttribute(const SfxItemSet& rSet);

        // helpers

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
