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
#ifndef INCLUDED_SVX_ANCHORID_HXX
#define INCLUDED_SVX_ANCHORID_HXX

#include <o3tl/typed_flags_set.hxx>

enum class SvxAnchorIds
{
    NONE         = 0x0000,
    Paragraph    = 0x0001,     // Frame bound to paragraph
    Character    = 0x0002,     // Frame bound to character
    Page         = 0x0004,     // Frame bound to page
    Fly          = 0x0008,      // Frame bound to other frame
    /** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the size controls */
    NoResize     = 0x0100,
    /** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the protect controls */
    NoProtect    = 0x0200
};
namespace o3tl
{
    template<> struct typed_flags<SvxAnchorIds> : is_typed_flags<SvxAnchorIds, 0x030f> {};
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
