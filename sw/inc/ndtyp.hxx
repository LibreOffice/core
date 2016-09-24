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

// IDs for different nodes. The member indicating the type of node is
// in base class.
const sal_uInt8 ND_ENDNODE      = 0x01;
const sal_uInt8 ND_STARTNODE    = 0x02;
const sal_uInt8 ND_TABLENODE    = 0x04 | ND_STARTNODE; ///< SwTableNode is derived from SwStartNode.
const sal_uInt8 ND_TEXTNODE     = 0x08;
const sal_uInt8 ND_GRFNODE      = 0x10;
const sal_uInt8 ND_OLENODE      = 0x20;
const sal_uInt8 ND_SECTIONNODE  = 0x40 | ND_STARTNODE; ///< SwSectionNode is derived from SwStartNode.
const sal_uInt8 ND_PLACEHOLDER  = 0x80;

// NoTextNode (if any of the 2 bits are set).
const sal_uInt8 ND_NOTXTNODE    = ND_GRFNODE | ND_OLENODE;
// ContentNode (if any of the 3 bits are set).
const sal_uInt8 ND_CONTENTNODE  = ND_TEXTNODE | ND_NOTXTNODE;

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
