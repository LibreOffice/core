/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <o3tl/typed_flags_set.hxx>
#include <editeng/editdata.hxx>
#include <tools/gen.hxx>

/** Information about the bullet in the paragraph*/
struct BulletInfo
{
public:
    OUString maText;
    Size maSize = Size(-1, -1);
    ScalingParameters maScalingParameters;
};

// internal use only!
enum class ParaFlag
{
    NONE               = 0x0000,
    HOLDDEPTH          = 0x4000,
    ISPAGE             = 0x0100,
};
namespace o3tl
{
    template<> struct typed_flags<ParaFlag> : is_typed_flags<ParaFlag, 0xc100> {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
