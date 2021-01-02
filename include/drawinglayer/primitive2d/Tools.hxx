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

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/CommonTypes.hxx>
#include <basegfx/range/b2drange.hxx>

namespace drawinglayer::primitive2d
{
class VisitingParameters;
}

namespace drawinglayer::primitive2d
{
/// get B2DRange from a given Primitive2DReference
basegfx::B2DRange DRAWINGLAYERCORE_DLLPUBLIC getB2DRangeFromPrimitive2DReference(
    const Primitive2DReference& rCandidate, VisitingParameters const& rParameters);

/** compare two Primitive2DReferences for equality, including trying to get implementations (BasePrimitive2D)
    and using compare operator
 */
bool DRAWINGLAYERCORE_DLLPUBLIC arePrimitive2DReferencesEqual(const Primitive2DReference& rA,
                                                              const Primitive2DReference& rB);

OUString DRAWINGLAYERCORE_DLLPUBLIC idToString(sal_uInt32 nId);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
