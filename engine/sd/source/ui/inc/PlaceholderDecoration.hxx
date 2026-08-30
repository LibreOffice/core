/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>

class SdrObject;

namespace sd
{
/// Build the aids that mark out a presentation placeholder: a dashed boundary
/// around the area it occupies, and, for a placeholder on a master page, the
/// name of the area in the corner opposite its text. The boundary follows the
/// Impress option for showing text boundaries, so an object that takes no aids,
/// or a turned-off option, yields nothing.
///
/// bSubContentProcessing tells whether the object is being drawn as part of a
/// master behind another page rather than on the page it lives on. Such a
/// drawing takes the boundary only for a header or footer placeholder, and
/// never the area name.
drawinglayer::primitive2d::Primitive2DContainer
createPlaceholderDecoration(SdrObject& rObject, bool bSubContentProcessing);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
