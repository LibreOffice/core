/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <tools/gen.hxx>

#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace vcl
{
struct ControlLayoutData;
}

// #i75163#
namespace basegfx
{
class B2DHomMatrix;
}

struct ImplOutDevData
{
    VclPtr<VirtualDevice> mpRotateDev;
    vcl::ControlLayoutData* mpRecordLayout;
    tools::Rectangle maRecordRect;

    // #i75163#
    basegfx::B2DHomMatrix* mpViewTransform;
    basegfx::B2DHomMatrix* mpInverseViewTransform;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
