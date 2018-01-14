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
#ifndef INCLUDED_SVX_INC_SWFRAMETYPES_HXX
#define INCLUDED_SVX_INC_SWFRAMETYPES_HXX

#include <com/sun/star/text/TextContentAnchorType.hpp>

// The former Rendezvous-IDs live on:
// There are IDs for the anchors (SwFormatAnchor) and some others
// that are only of importance for interfaces (SwDoc).
enum class RndStdIds
{
    UNKNOWN = -1,       // return value used by SwFEShell::GetAnchorId

    // the following 5 values are deliberately the same as the values in css::text::TextContentAnchorType
    FLY_AT_PARA = int(css::text::TextContentAnchorType_AT_PARAGRAPH),  // Anchored at paragraph.
    FLY_AS_CHAR = int(css::text::TextContentAnchorType_AS_CHARACTER),  // Anchored as character.
    FLY_AT_PAGE = int(css::text::TextContentAnchorType_AT_PAGE),       // Anchored at page.
    FLY_AT_FLY  = int(css::text::TextContentAnchorType_AT_FRAME),      // Anchored at frame.
    FLY_AT_CHAR = int(css::text::TextContentAnchorType_AT_CHARACTER),  // Anchored at character.

    HEADER,
    FOOTER,
    HEADERL,
    HEADERR,

    DRAW_OBJECT     // A draw-Object! For the SwDoc-interface only!
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
