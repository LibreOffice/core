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

#ifndef INCLUDED_SW_INC_NDTYP_HXX
#define INCLUDED_SW_INC_NDTYP_HXX

#include <tools/solar.h>
#include <o3tl/typed_flags_set.hxx>

// IDs for different nodes. The member indicating the type of node is
// in base class.
enum class SwNodeType : sal_uInt8 {
    NONE         = 0x00,
    End          = 0x01,
    Start        = 0x02,
    Table        = 0x04 | Start, ///< SwTableNode is derived from SwStartNode.
    Text         = 0x08,
    Grf          = 0x10,
    Ole          = 0x20,
    Section      = 0x40 | Start, ///< SwSectionNode is derived from SwStartNode.
    PlaceHolder  = 0x80,

// NoTextNode (if any of the 2 bits are set).
    NoTextMask   = Grf | Ole,
// ContentNode (if any of the 3 bits are set).
    ContentMask  = Text | NoTextMask,
};
namespace o3tl {
    template<> struct typed_flags<SwNodeType> : is_typed_flags<SwNodeType, 0xff> {};
}

// Special types of StartNodes that are not derivations but keep
// "sections" together.
enum SwStartNodeType
{
    SwNormalStartNode = 0,
    SwTableBoxStartNode,
    SwFlyStartNode,
    SwFootnoteStartNode,
    SwHeaderStartNode,
    SwFooterStartNode
};

// is the node the first and/or last node of a section?
// This information is used for the export filters. Our layout never have a
// distance before or after if the node is the first or last in a section.
const sal_uInt8 ND_HAS_PREV_LAYNODE = 0x01;
const sal_uInt8 ND_HAS_NEXT_LAYNODE = 0x02;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
