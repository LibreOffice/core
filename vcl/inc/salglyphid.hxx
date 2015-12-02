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
#ifndef INCLUDED_VCL_INC_SALGLYPHID_HXX
#define INCLUDED_VCL_INC_SALGLYPHID_HXX

// TODO: sal_GlyphId should become a class...
typedef sal_uInt32 sal_GlyphId;

// Glyph Flags
#define GF_NONE     0x00000000
#define GF_FLAGMASK 0xFF800000
#define GF_IDXMASK  ~GF_FLAGMASK
#define GF_ISCHAR   0x00800000
#define GF_ROTL     0x01000000
// caution !!!
#define GF_VERT     0x02000000
// GF_VERT is only for windows implementation
// (win/gdi/salgdi3.cxx, win/gdi/winlayout.cxx)
// don't use this elsewhere !!!
#define GF_ROTR     0x03000000
#define GF_ROTMASK  0x03000000
#define GF_UNHINTED 0x04000000
#define GF_GSUB     0x08000000
#define GF_FONTMASK 0xF0000000
#define GF_FONTSHIFT 28

#define GF_DROPPED  0xFFFFFFFF

#endif // INCLUDED_VCL_INC_SALGLYPHID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
