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

#include <svx/PaletteManager.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/objsh.hxx>
#include <svx/drawitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <stack>
#include <set>
#include <cppu/unotype.hxx>
#include <officecfg/Office/Common.hxx>

#include "palettes.hxx"

static const struct
{
    Color mColor;
    const char* mName;
} aLibreColors[] =
{
    { Color(0x1b, 0x1b, 0x1b), "HLC 000 10 00" },
    { Color(0x30, 0x30, 0x30), "HLC 000 20 00" },
    { Color(0x47, 0x47, 0x47), "HLC 000 30 00" },
    { Color(0x5e, 0x5e, 0x5e), "HLC 000 40 00" },
    { Color(0x77, 0x77, 0x77), "HLC 000 50 00" },
    { Color(0x91, 0x91, 0x91), "HLC 000 60 00" },
    { Color(0xab, 0xab, 0xab), "HLC 000 70 00" },
    { Color(0xc6, 0xc6, 0xc6), "HLC 000 80 00" },
    { Color(0xe2, 0xe2, 0xe2), "HLC 000 90 00" },
    { Color(0x3d, 0x2b, 0x2f), "HLC 010 20 10" },
    { Color(0x4a, 0x24, 0x2d), "HLC 010 20 20" },
    { Color(0x56, 0x1a, 0x2a), "HLC 010 20 30" },
    { Color(0x55, 0x42, 0x45), "HLC 010 30 10" },
    { Color(0x63, 0x3b, 0x43), "HLC 010 30 20" },
    { Color(0x70, 0x32, 0x40), "HLC 010 30 30" },
    { Color(0x7c, 0x27, 0x3e), "HLC 010 30 40" },
    { Color(0x88, 0x13, 0x3d), "HLC 010 30 50" },
    { Color(0x6d, 0x59, 0x5d), "HLC 010 40 10" },
    { Color(0x7d, 0x52, 0x5a), "HLC 010 40 20" },
    { Color(0x8b, 0x4a, 0x57), "HLC 010 40 30" },
    { Color(0x98, 0x40, 0x55), "HLC 010 40 40" },
    { Color(0xa5, 0x33, 0x54), "HLC 010 40 50" },
    { Color(0xb1, 0x1e, 0x51), "HLC 010 40 60" },
    { Color(0x87, 0x72, 0x76), "HLC 010 50 10" },
    { Color(0x97, 0x6b, 0x73), "HLC 010 50 20" },
    { Color(0xa6, 0x63, 0x70), "HLC 010 50 30" },
    { Color(0xb4, 0x5a, 0x6d), "HLC 010 50 40" },
    { Color(0xc2, 0x4e, 0x6c), "HLC 010 50 50" },
    { Color(0xcf, 0x40, 0x69), "HLC 010 50 60" },
    { Color(0xdb, 0x2d, 0x67), "HLC 010 50 70" },
    { Color(0xa1, 0x8b, 0x8f), "HLC 010 60 10" },
    { Color(0xb2, 0x84, 0x8c), "HLC 010 60 20" },
    { Color(0xc2, 0x7d, 0x89), "HLC 010 60 30" },
    { Color(0xd1, 0x74, 0x86), "HLC 010 60 40" },
    { Color(0xe0, 0x69, 0x85), "HLC 010 60 50" },
    { Color(0xbc, 0xa5, 0xaa), "HLC 010 70 10" },
    { Color(0xce, 0x9f, 0xa7), "HLC 010 70 20" },
    { Color(0xdf, 0x97, 0xa3), "HLC 010 70 30" },
    { Color(0xef, 0x8e, 0xa0), "HLC 010 70 40" },
    { Color(0xd8, 0xc1, 0xc5), "HLC 010 80 10" },
    { Color(0xea, 0xba, 0xc2), "HLC 010 80 20" },
    { Color(0xf4, 0xdc, 0xe1), "HLC 010 90 10" },
    { Color(0x3e, 0x2b, 0x2c), "HLC 020 20 10" },
    { Color(0x4a, 0x25, 0x28), "HLC 020 20 20" },
    { Color(0x56, 0x1c, 0x23), "HLC 020 20 30" },
    { Color(0x56, 0x42, 0x42), "HLC 020 30 10" },
    { Color(0x63, 0x3b, 0x3e), "HLC 020 30 20" },
    { Color(0x70, 0x33, 0x39), "HLC 020 30 30" },
    { Color(0x7b, 0x29, 0x35), "HLC 020 30 40" },
    { Color(0x87, 0x17, 0x2f), "HLC 020 30 50" },
    { Color(0x6e, 0x59, 0x5a), "HLC 020 40 10" },
    { Color(0x7c, 0x53, 0x55), "HLC 020 40 20" },
    { Color(0x8b, 0x4b, 0x4f), "HLC 020 40 30" },
    { Color(0x97, 0x42, 0x4b), "HLC 020 40 40" },
    { Color(0xa4, 0x36, 0x46), "HLC 020 40 50" },
    { Color(0xaf, 0x25, 0x42), "HLC 020 40 60" },
    { Color(0x88, 0x72, 0x72), "HLC 020 50 10" },
    { Color(0x97, 0x6b, 0x6e), "HLC 020 50 20" },
    { Color(0xa6, 0x64, 0x67), "HLC 020 50 30" },
    { Color(0xb4, 0x5b, 0x63), "HLC 020 50 40" },
    { Color(0xc2, 0x50, 0x5d), "HLC 020 50 50" },
    { Color(0xce, 0x44, 0x59), "HLC 020 50 60" },
    { Color(0xd9, 0x33, 0x55), "HLC 020 50 70" },
    { Color(0xa2, 0x8b, 0x8c), "HLC 020 60 10" },
    { Color(0xb2, 0x85, 0x87), "HLC 020 60 20" },
    { Color(0xc2, 0x7d, 0x80), "HLC 020 60 30" },
    { Color(0xd0, 0x75, 0x7c), "HLC 020 60 40" },
    { Color(0xdf, 0x6b, 0x76), "HLC 020 60 50" },
    { Color(0xbd, 0xa5, 0xa6), "HLC 020 70 10" },
    { Color(0xce, 0x9f, 0xa1), "HLC 020 70 20" },
    { Color(0xdf, 0x97, 0x9a), "HLC 020 70 30" },
    { Color(0xd9, 0xc1, 0xc1), "HLC 020 80 10" },
    { Color(0xea, 0xba, 0xbc), "HLC 020 80 20" },
    { Color(0xf5, 0xdc, 0xdd), "HLC 020 90 10" },
    { Color(0x3d, 0x2c, 0x29), "HLC 030 20 10" },
    { Color(0x4a, 0x26, 0x23), "HLC 030 20 20" },
    { Color(0x55, 0x1e, 0x1c), "HLC 030 20 30" },
    { Color(0x55, 0x42, 0x3f), "HLC 030 30 10" },
    { Color(0x63, 0x3c, 0x38), "HLC 030 30 20" },
    { Color(0x6f, 0x35, 0x31), "HLC 030 30 30" },
    { Color(0x79, 0x2c, 0x2a), "HLC 030 30 40" },
    { Color(0x84, 0x1f, 0x23), "HLC 030 30 50" },
    { Color(0x6d, 0x5a, 0x56), "HLC 030 40 10" },
    { Color(0x7c, 0x54, 0x4f), "HLC 030 40 20" },
    { Color(0x8a, 0x4c, 0x47), "HLC 030 40 30" },
    { Color(0x95, 0x45, 0x40), "HLC 030 40 40" },
    { Color(0xa1, 0x3b, 0x38), "HLC 030 40 50" },
    { Color(0xac, 0x2d, 0x31), "HLC 030 40 60" },
    { Color(0xb6, 0x19, 0x2a), "HLC 030 40 70" },
    { Color(0x87, 0x72, 0x6f), "HLC 030 50 10" },
    { Color(0x97, 0x6c, 0x67), "HLC 030 50 20" },
    { Color(0xa5, 0x65, 0x5f), "HLC 030 50 30" },
    { Color(0xb1, 0x5e, 0x57), "HLC 030 50 40" },
    { Color(0xbe, 0x55, 0x4f), "HLC 030 50 50" },
    { Color(0xca, 0x49, 0x48), "HLC 030 50 60" },
    { Color(0xd5, 0x3d, 0x40), "HLC 030 50 70" },
    { Color(0xe1, 0x28, 0x39), "HLC 030 50 80" },
    { Color(0xa1, 0x8c, 0x88), "HLC 030 60 10" },
    { Color(0xb2, 0x85, 0x80), "HLC 030 60 20" },
    { Color(0xc1, 0x7f, 0x78), "HLC 030 60 30" },
    { Color(0xce, 0x78, 0x6f), "HLC 030 60 40" },
    { Color(0xdc, 0x6f, 0x67), "HLC 030 60 50" },
    { Color(0xe9, 0x65, 0x5f), "HLC 030 60 60" },
    { Color(0xbc, 0xa6, 0xa2), "HLC 030 70 10" },
    { Color(0xce, 0xa0, 0x9a), "HLC 030 70 20" },
    { Color(0xde, 0x99, 0x91), "HLC 030 70 30" },
    { Color(0xec, 0x92, 0x89), "HLC 030 70 40" },
    { Color(0xd8, 0xc1, 0xbd), "HLC 030 80 10" },
    { Color(0xea, 0xbb, 0xb5), "HLC 030 80 20" },
    { Color(0xf5, 0xdd, 0xd9), "HLC 030 90 10" },
    { Color(0x3c, 0x2c, 0x28), "HLC 040 20 10" },
    { Color(0x48, 0x27, 0x1f), "HLC 040 20 20" },
    { Color(0x54, 0x43, 0x3e), "HLC 040 30 10" },
    { Color(0x61, 0x3d, 0x35), "HLC 040 30 20" },
    { Color(0x6c, 0x37, 0x2a), "HLC 040 30 30" },
    { Color(0x75, 0x31, 0x21), "HLC 040 30 40" },
    { Color(0x6c, 0x5a, 0x55), "HLC 040 40 10" },
    { Color(0x7a, 0x55, 0x4b), "HLC 040 40 20" },
    { Color(0x87, 0x4f, 0x41), "HLC 040 40 30" },
    { Color(0x91, 0x49, 0x37), "HLC 040 40 40" },
    { Color(0x9c, 0x40, 0x2c), "HLC 040 40 50" },
    { Color(0xa6, 0x36, 0x23), "HLC 040 40 60" },
    { Color(0x86, 0x73, 0x6d), "HLC 040 50 10" },
    { Color(0x94, 0x6d, 0x63), "HLC 040 50 20" },
    { Color(0xa2, 0x67, 0x58), "HLC 040 50 30" },
    { Color(0xad, 0x61, 0x4e), "HLC 040 50 40" },
    { Color(0xb9, 0x5a, 0x43), "HLC 040 50 50" },
    { Color(0xc4, 0x51, 0x39), "HLC 040 50 60" },
    { Color(0xce, 0x47, 0x2d), "HLC 040 50 70" },
    { Color(0xd8, 0x3a, 0x23), "HLC 040 50 80" },
    { Color(0xa0, 0x8c, 0x86), "HLC 040 60 10" },
    { Color(0xb0, 0x87, 0x7c), "HLC 040 60 20" },
    { Color(0xbe, 0x81, 0x70), "HLC 040 60 30" },
    { Color(0xca, 0x7b, 0x66), "HLC 040 60 40" },
    { Color(0xd7, 0x74, 0x5b), "HLC 040 60 50" },
    { Color(0xe3, 0x6b, 0x51), "HLC 040 60 60" },
    { Color(0xbb, 0xa7, 0xa1), "HLC 040 70 10" },
    { Color(0xcb, 0xa1, 0x96), "HLC 040 70 20" },
    { Color(0xdb, 0x9b, 0x8a), "HLC 040 70 30" },
    { Color(0xe7, 0x95, 0x7f), "HLC 040 70 40" },
    { Color(0xd7, 0xc2, 0xbc), "HLC 040 80 10" },
    { Color(0xe8, 0xbc, 0xb1), "HLC 040 80 20" },
    { Color(0xf3, 0xde, 0xd7), "HLC 040 90 10" },
    { Color(0x3b, 0x2d, 0x26), "HLC 050 20 10" },
    { Color(0x45, 0x29, 0x1b), "HLC 050 20 20" },
    { Color(0x53, 0x43, 0x3c), "HLC 050 30 10" },
    { Color(0x5d, 0x3f, 0x30), "HLC 050 30 20" },
    { Color(0x68, 0x3a, 0x24), "HLC 050 30 30" },
    { Color(0x6b, 0x5b, 0x53), "HLC 050 40 10" },
    { Color(0x77, 0x57, 0x46), "HLC 050 40 20" },
    { Color(0x82, 0x52, 0x3a), "HLC 050 40 30" },
    { Color(0x8b, 0x4d, 0x2e), "HLC 050 40 40" },
    { Color(0x95, 0x46, 0x20), "HLC 050 40 50" },
    { Color(0x85, 0x73, 0x6b), "HLC 050 50 10" },
    { Color(0x91, 0x6f, 0x5e), "HLC 050 50 20" },
    { Color(0x9e, 0x6a, 0x51), "HLC 050 50 30" },
    { Color(0xa8, 0x65, 0x45), "HLC 050 50 40" },
    { Color(0xb2, 0x5f, 0x37), "HLC 050 50 50" },
    { Color(0xbb, 0x59, 0x28), "HLC 050 50 60" },
    { Color(0x9f, 0x8d, 0x85), "HLC 050 60 10" },
    { Color(0xac, 0x89, 0x77), "HLC 050 60 20" },
    { Color(0xb9, 0x84, 0x69), "HLC 050 60 30" },
    { Color(0xc4, 0x7f, 0x5d), "HLC 050 60 40" },
    { Color(0xd0, 0x79, 0x4f), "HLC 050 60 50" },
    { Color(0xd9, 0x73, 0x40), "HLC 050 60 60" },
    { Color(0xe3, 0x6c, 0x33), "HLC 050 60 70" },
    { Color(0xba, 0xa7, 0x9f), "HLC 050 70 10" },
    { Color(0xc8, 0xa3, 0x90), "HLC 050 70 20" },
    { Color(0xd6, 0x9e, 0x82), "HLC 050 70 30" },
    { Color(0xe1, 0x99, 0x76), "HLC 050 70 40" },
    { Color(0xee, 0x93, 0x68), "HLC 050 70 50" },
    { Color(0xd6, 0xc2, 0xba), "HLC 050 80 10" },
    { Color(0xe4, 0xbe, 0xab), "HLC 050 80 20" },
    { Color(0xf3, 0xb9, 0x9d), "HLC 050 80 30" },
    { Color(0xf2, 0xde, 0xd5), "HLC 050 90 10" },
    { Color(0x3b, 0x2d, 0x25), "HLC 060 20 10" },
    { Color(0x52, 0x44, 0x3a), "HLC 060 30 10" },
    { Color(0x5b, 0x41, 0x2d), "HLC 060 30 20" },
    { Color(0x63, 0x3d, 0x1e), "HLC 060 30 30" },
    { Color(0x6a, 0x5b, 0x51), "HLC 060 40 10" },
    { Color(0x75, 0x58, 0x43), "HLC 060 40 20" },
    { Color(0x7e, 0x54, 0x34), "HLC 060 40 30" },
    { Color(0x86, 0x51, 0x26), "HLC 060 40 40" },
    { Color(0x83, 0x74, 0x6a), "HLC 060 50 10" },
    { Color(0x8f, 0x70, 0x5b), "HLC 060 50 20" },
    { Color(0x99, 0x6d, 0x4b), "HLC 060 50 30" },
    { Color(0xa2, 0x69, 0x3d), "HLC 060 50 40" },
    { Color(0xaa, 0x65, 0x2c), "HLC 060 50 50" },
    { Color(0xb1, 0x61, 0x18), "HLC 060 50 60" },
    { Color(0x9e, 0x8d, 0x83), "HLC 060 60 10" },
    { Color(0xaa, 0x8a, 0x73), "HLC 060 60 20" },
    { Color(0xb4, 0x86, 0x63), "HLC 060 60 30" },
    { Color(0xbe, 0x82, 0x55), "HLC 060 60 40" },
    { Color(0xc7, 0x7e, 0x45), "HLC 060 60 50" },
    { Color(0xcf, 0x7a, 0x33), "HLC 060 60 60" },
    { Color(0xd7, 0x76, 0x1f), "HLC 060 60 70" },
    { Color(0xb9, 0xa8, 0x9d), "HLC 060 70 10" },
    { Color(0xc5, 0xa4, 0x8d), "HLC 060 70 20" },
    { Color(0xd1, 0xa1, 0x7d), "HLC 060 70 30" },
    { Color(0xdb, 0x9d, 0x6e), "HLC 060 70 40" },
    { Color(0xe5, 0x99, 0x5d), "HLC 060 70 50" },
    { Color(0xee, 0x94, 0x4c), "HLC 060 70 60" },
    { Color(0xd4, 0xc3, 0xb8), "HLC 060 80 10" },
    { Color(0xe2, 0xbf, 0xa7), "HLC 060 80 20" },
    { Color(0xee, 0xbc, 0x97), "HLC 060 80 30" },
    { Color(0xf1, 0xdf, 0xd3), "HLC 060 90 10" },
    { Color(0x38, 0x2f, 0x23), "HLC 070 20 10" },
    { Color(0x4f, 0x45, 0x39), "HLC 070 30 10" },
    { Color(0x56, 0x43, 0x2b), "HLC 070 30 20" },
    { Color(0x5e, 0x40, 0x1a), "HLC 070 30 30" },
    { Color(0x67, 0x5c, 0x50), "HLC 070 40 10" },
    { Color(0x6f, 0x5a, 0x41), "HLC 070 40 20" },
    { Color(0x77, 0x58, 0x30), "HLC 070 40 30" },
    { Color(0x7d, 0x56, 0x1f), "HLC 070 40 40" },
    { Color(0x81, 0x75, 0x68), "HLC 070 50 10" },
    { Color(0x89, 0x73, 0x59), "HLC 070 50 20" },
    { Color(0x92, 0x70, 0x47), "HLC 070 50 30" },
    { Color(0x98, 0x6e, 0x37), "HLC 070 50 40" },
    { Color(0xa0, 0x6b, 0x22), "HLC 070 50 50" },
    { Color(0x9b, 0x8e, 0x81), "HLC 070 60 10" },
    { Color(0xa4, 0x8c, 0x71), "HLC 070 60 20" },
    { Color(0xad, 0x8a, 0x5f), "HLC 070 60 30" },
    { Color(0xb4, 0x87, 0x4f), "HLC 070 60 40" },
    { Color(0xbc, 0x84, 0x3b), "HLC 070 60 50" },
    { Color(0xc2, 0x82, 0x27), "HLC 070 60 60" },
    { Color(0xb6, 0xa9, 0x9b), "HLC 070 70 10" },
    { Color(0xbf, 0xa7, 0x8b), "HLC 070 70 20" },
    { Color(0xc9, 0xa4, 0x79), "HLC 070 70 30" },
    { Color(0xd1, 0xa2, 0x68), "HLC 070 70 40" },
    { Color(0xd9, 0x9f, 0x54), "HLC 070 70 50" },
    { Color(0xe0, 0x9c, 0x41), "HLC 070 70 60" },
    { Color(0xe5, 0x9a, 0x2b), "HLC 070 70 70" },
    { Color(0xd1, 0xc4, 0xb6), "HLC 070 80 10" },
    { Color(0xdb, 0xc2, 0xa5), "HLC 070 80 20" },
    { Color(0xe6, 0xbf, 0x92), "HLC 070 80 30" },
    { Color(0xee, 0xbd, 0x81), "HLC 070 80 40" },
    { Color(0xf7, 0xba, 0x6e), "HLC 070 80 50" },
    { Color(0xee, 0xe0, 0xd1), "HLC 070 90 10" },
    { Color(0xf8, 0xde, 0xc0), "HLC 070 90 20" },
    { Color(0x35, 0x2f, 0x23), "HLC 080 20 10" },
    { Color(0x4c, 0x46, 0x39), "HLC 080 30 10" },
    { Color(0x52, 0x44, 0x29), "HLC 080 30 20" },
    { Color(0x64, 0x5d, 0x50), "HLC 080 40 10" },
    { Color(0x6b, 0x5c, 0x3f), "HLC 080 40 20" },
    { Color(0x71, 0x5b, 0x2e), "HLC 080 40 30" },
    { Color(0x75, 0x59, 0x1a), "HLC 080 40 40" },
    { Color(0x7d, 0x76, 0x68), "HLC 080 50 10" },
    { Color(0x85, 0x75, 0x57), "HLC 080 50 20" },
    { Color(0x8b, 0x73, 0x45), "HLC 080 50 30" },
    { Color(0x90, 0x72, 0x32), "HLC 080 50 40" },
    { Color(0x93, 0x71, 0x1b), "HLC 080 50 50" },
    { Color(0x97, 0x90, 0x81), "HLC 080 60 10" },
    { Color(0x9f, 0x8e, 0x6f), "HLC 080 60 20" },
    { Color(0xa6, 0x8d, 0x5d), "HLC 080 60 30" },
    { Color(0xac, 0x8b, 0x4a), "HLC 080 60 40" },
    { Color(0xaf, 0x8a, 0x36), "HLC 080 60 50" },
    { Color(0xb4, 0x89, 0x1b), "HLC 080 60 60" },
    { Color(0xb2, 0xaa, 0x9b), "HLC 080 70 10" },
    { Color(0xba, 0xa9, 0x89), "HLC 080 70 20" },
    { Color(0xc2, 0xa7, 0x76), "HLC 080 70 30" },
    { Color(0xc8, 0xa5, 0x63), "HLC 080 70 40" },
    { Color(0xcc, 0xa5, 0x4f), "HLC 080 70 50" },
    { Color(0xd1, 0xa3, 0x39), "HLC 080 70 60" },
    { Color(0xd6, 0xa2, 0x1e), "HLC 080 70 70" },
    { Color(0xce, 0xc5, 0xb6), "HLC 080 80 10" },
    { Color(0xd6, 0xc4, 0xa3), "HLC 080 80 20" },
    { Color(0xde, 0xc2, 0x90), "HLC 080 80 30" },
    { Color(0xe5, 0xc1, 0x7d), "HLC 080 80 40" },
    { Color(0xe9, 0xc0, 0x69), "HLC 080 80 50" },
    { Color(0xef, 0xbe, 0x54), "HLC 080 80 60" },
    { Color(0xf4, 0xbd, 0x3e), "HLC 080 80 70" },
    { Color(0xea, 0xe1, 0xd1), "HLC 080 90 10" },
    { Color(0xf3, 0xe0, 0xbe), "HLC 080 90 20" },
    { Color(0xfb, 0xde, 0xab), "HLC 080 90 30" },
    { Color(0x34, 0x30, 0x22), "HLC 090 20 10" },
    { Color(0x4b, 0x46, 0x37), "HLC 090 30 10" },
    { Color(0x4e, 0x46, 0x27), "HLC 090 30 20" },
    { Color(0x63, 0x5e, 0x4e), "HLC 090 40 10" },
    { Color(0x67, 0x5e, 0x3d), "HLC 090 40 20" },
    { Color(0x6a, 0x5d, 0x2c), "HLC 090 40 30" },
    { Color(0x7c, 0x77, 0x66), "HLC 090 50 10" },
    { Color(0x80, 0x76, 0x55), "HLC 090 50 20" },
    { Color(0x84, 0x76, 0x43), "HLC 090 50 30" },
    { Color(0x86, 0x76, 0x2f), "HLC 090 50 40" },
    { Color(0x88, 0x76, 0x16), "HLC 090 50 50" },
    { Color(0x96, 0x90, 0x7f), "HLC 090 60 10" },
    { Color(0x9b, 0x90, 0x6d), "HLC 090 60 20" },
    { Color(0x9e, 0x90, 0x5b), "HLC 090 60 30" },
    { Color(0xa1, 0x8f, 0x48), "HLC 090 60 40" },
    { Color(0xa3, 0x8f, 0x32), "HLC 090 60 50" },
    { Color(0xa5, 0x8f, 0x14), "HLC 090 60 60" },
    { Color(0xb1, 0xab, 0x99), "HLC 090 70 10" },
    { Color(0xb6, 0xaa, 0x87), "HLC 090 70 20" },
    { Color(0xba, 0xaa, 0x74), "HLC 090 70 30" },
    { Color(0xbd, 0xaa, 0x61), "HLC 090 70 40" },
    { Color(0xc0, 0xaa, 0x4c), "HLC 090 70 50" },
    { Color(0xc2, 0xa9, 0x34), "HLC 090 70 60" },
    { Color(0xc3, 0xa9, 0x0e), "HLC 090 70 70" },
    { Color(0xcc, 0xc6, 0xb4), "HLC 090 80 10" },
    { Color(0xd1, 0xc6, 0xa1), "HLC 090 80 20" },
    { Color(0xd6, 0xc5, 0x8e), "HLC 090 80 30" },
    { Color(0xd9, 0xc5, 0x7a), "HLC 090 80 40" },
    { Color(0xdc, 0xc5, 0x66), "HLC 090 80 50" },
    { Color(0xdf, 0xc5, 0x50), "HLC 090 80 60" },
    { Color(0xe1, 0xc4, 0x35), "HLC 090 80 70" },
    { Color(0xe3, 0xc4, 0x00), "HLC 090 80 80" },
    { Color(0xe9, 0xe2, 0xcf), "HLC 090 90 10" },
    { Color(0xee, 0xe2, 0xbc), "HLC 090 90 20" },
    { Color(0xf2, 0xe1, 0xa9), "HLC 090 90 30" },
    { Color(0xf6, 0xe1, 0x95), "HLC 090 90 40" },
    { Color(0xfa, 0xe1, 0x80), "HLC 090 90 50" },
    { Color(0xfd, 0xe0, 0x6b), "HLC 090 90 60" },
    { Color(0x33, 0x30, 0x23), "HLC 100 20 10" },
    { Color(0x49, 0x47, 0x39), "HLC 100 30 10" },
    { Color(0x4a, 0x48, 0x28), "HLC 100 30 20" },
    { Color(0x61, 0x5e, 0x50), "HLC 100 40 10" },
    { Color(0x62, 0x5f, 0x3f), "HLC 100 40 20" },
    { Color(0x62, 0x60, 0x2d), "HLC 100 40 30" },
    { Color(0x7a, 0x77, 0x68), "HLC 100 50 10" },
    { Color(0x7b, 0x78, 0x56), "HLC 100 50 20" },
    { Color(0x7b, 0x79, 0x44), "HLC 100 50 30" },
    { Color(0x7b, 0x7a, 0x31), "HLC 100 50 40" },
    { Color(0x94, 0x91, 0x81), "HLC 100 60 10" },
    { Color(0x95, 0x92, 0x6f), "HLC 100 60 20" },
    { Color(0x96, 0x92, 0x5c), "HLC 100 60 30" },
    { Color(0x95, 0x93, 0x49), "HLC 100 60 40" },
    { Color(0x96, 0x94, 0x33), "HLC 100 60 50" },
    { Color(0xae, 0xab, 0x9b), "HLC 100 70 10" },
    { Color(0xb0, 0xac, 0x88), "HLC 100 70 20" },
    { Color(0xb1, 0xad, 0x76), "HLC 100 70 30" },
    { Color(0xb1, 0xae, 0x62), "HLC 100 70 40" },
    { Color(0xb2, 0xae, 0x4d), "HLC 100 70 50" },
    { Color(0xb1, 0xaf, 0x35), "HLC 100 70 60" },
    { Color(0xaf, 0xb0, 0x14), "HLC 100 70 70" },
    { Color(0xca, 0xc7, 0xb6), "HLC 100 80 10" },
    { Color(0xcb, 0xc7, 0xa3), "HLC 100 80 20" },
    { Color(0xcc, 0xc8, 0x90), "HLC 100 80 30" },
    { Color(0xcd, 0xc9, 0x7c), "HLC 100 80 40" },
    { Color(0xce, 0xc9, 0x67), "HLC 100 80 50" },
    { Color(0xce, 0xca, 0x51), "HLC 100 80 60" },
    { Color(0xcc, 0xcb, 0x39), "HLC 100 80 70" },
    { Color(0xcc, 0xcc, 0x0a), "HLC 100 80 80" },
    { Color(0xe6, 0xe3, 0xd1), "HLC 100 90 10" },
    { Color(0xe8, 0xe3, 0xbe), "HLC 100 90 20" },
    { Color(0xe9, 0xe4, 0xaa), "HLC 100 90 30" },
    { Color(0xe9, 0xe5, 0x96), "HLC 100 90 40" },
    { Color(0xeb, 0xe5, 0x82), "HLC 100 90 50" },
    { Color(0x30, 0x31, 0x23), "HLC 110 20 10" },
    { Color(0x46, 0x48, 0x38), "HLC 110 30 10" },
    { Color(0x45, 0x49, 0x2a), "HLC 110 30 20" },
    { Color(0x5e, 0x5f, 0x4f), "HLC 110 40 10" },
    { Color(0x5d, 0x61, 0x40), "HLC 110 40 20" },
    { Color(0x5a, 0x62, 0x2e), "HLC 110 40 30" },
    { Color(0x77, 0x78, 0x67), "HLC 110 50 10" },
    { Color(0x76, 0x79, 0x58), "HLC 110 50 20" },
    { Color(0x73, 0x7b, 0x46), "HLC 110 50 30" },
    { Color(0x70, 0x7d, 0x34), "HLC 110 50 40" },
    { Color(0x90, 0x92, 0x81), "HLC 110 60 10" },
    { Color(0x8f, 0x93, 0x70), "HLC 110 60 20" },
    { Color(0x8c, 0x95, 0x5e), "HLC 110 60 30" },
    { Color(0x8a, 0x96, 0x4d), "HLC 110 60 40" },
    { Color(0x86, 0x98, 0x37), "HLC 110 60 50" },
    { Color(0xab, 0xac, 0x9b), "HLC 110 70 10" },
    { Color(0xaa, 0xae, 0x8a), "HLC 110 70 20" },
    { Color(0xa7, 0xb0, 0x77), "HLC 110 70 30" },
    { Color(0xa5, 0xb1, 0x66), "HLC 110 70 40" },
    { Color(0xa1, 0xb3, 0x51), "HLC 110 70 50" },
    { Color(0x9e, 0xb4, 0x3c), "HLC 110 70 60" },
    { Color(0x9b, 0xb6, 0x1f), "HLC 110 70 70" },
    { Color(0xc6, 0xc8, 0xb5), "HLC 110 80 10" },
    { Color(0xc5, 0xc9, 0xa4), "HLC 110 80 20" },
    { Color(0xc3, 0xcb, 0x91), "HLC 110 80 30" },
    { Color(0xc1, 0xcd, 0x7f), "HLC 110 80 40" },
    { Color(0xbd, 0xce, 0x6b), "HLC 110 80 50" },
    { Color(0xe2, 0xe4, 0xd1), "HLC 110 90 10" },
    { Color(0xe1, 0xe5, 0xc0), "HLC 110 90 20" },
    { Color(0xdf, 0xe7, 0xac), "HLC 110 90 30" },
    { Color(0x2c, 0x32, 0x24), "HLC 120 20 10" },
    { Color(0x43, 0x49, 0x3a), "HLC 120 30 10" },
    { Color(0x3e, 0x4b, 0x2b), "HLC 120 30 20" },
    { Color(0x5a, 0x61, 0x51), "HLC 120 40 10" },
    { Color(0x56, 0x63, 0x42), "HLC 120 40 20" },
    { Color(0x50, 0x65, 0x32), "HLC 120 40 30" },
    { Color(0x73, 0x79, 0x69), "HLC 120 50 10" },
    { Color(0x6e, 0x7c, 0x59), "HLC 120 50 20" },
    { Color(0x69, 0x7e, 0x49), "HLC 120 50 30" },
    { Color(0x63, 0x80, 0x3a), "HLC 120 50 40" },
    { Color(0x8c, 0x93, 0x82), "HLC 120 60 10" },
    { Color(0x88, 0x95, 0x72), "HLC 120 60 20" },
    { Color(0x82, 0x98, 0x61), "HLC 120 60 30" },
    { Color(0x7c, 0x9a, 0x52), "HLC 120 60 40" },
    { Color(0x76, 0x9c, 0x3f), "HLC 120 60 50" },
    { Color(0x6e, 0x9e, 0x29), "HLC 120 60 60" },
    { Color(0xa6, 0xae, 0x9c), "HLC 120 70 10" },
    { Color(0xa2, 0xb0, 0x8c), "HLC 120 70 20" },
    { Color(0x9d, 0xb2, 0x7b), "HLC 120 70 30" },
    { Color(0x97, 0xb5, 0x6b), "HLC 120 70 40" },
    { Color(0x90, 0xb7, 0x58), "HLC 120 70 50" },
    { Color(0x89, 0xb9, 0x44), "HLC 120 70 60" },
    { Color(0xc2, 0xc9, 0xb7), "HLC 120 80 10" },
    { Color(0xbd, 0xcb, 0xa6), "HLC 120 80 20" },
    { Color(0xb8, 0xce, 0x95), "HLC 120 80 30" },
    { Color(0xb2, 0xd0, 0x85), "HLC 120 80 40" },
    { Color(0xde, 0xe5, 0xd3), "HLC 120 90 10" },
    { Color(0xd9, 0xe8, 0xc1), "HLC 120 90 20" },
    { Color(0x2a, 0x33, 0x26), "HLC 130 20 10" },
    { Color(0x40, 0x49, 0x3c), "HLC 130 30 10" },
    { Color(0x3a, 0x4c, 0x2e), "HLC 130 30 20" },
    { Color(0x58, 0x61, 0x53), "HLC 130 40 10" },
    { Color(0x51, 0x64, 0x45), "HLC 130 40 20" },
    { Color(0x47, 0x66, 0x37), "HLC 130 40 30" },
    { Color(0x70, 0x7a, 0x6b), "HLC 130 50 10" },
    { Color(0x69, 0x7d, 0x5d), "HLC 130 50 20" },
    { Color(0x60, 0x80, 0x4e), "HLC 130 50 30" },
    { Color(0x57, 0x82, 0x41), "HLC 130 50 40" },
    { Color(0x4a, 0x85, 0x30), "HLC 130 50 50" },
    { Color(0x8a, 0x94, 0x84), "HLC 130 60 10" },
    { Color(0x83, 0x96, 0x75), "HLC 130 60 20" },
    { Color(0x79, 0x9a, 0x67), "HLC 130 60 30" },
    { Color(0x70, 0x9c, 0x59), "HLC 130 60 40" },
    { Color(0x64, 0x9f, 0x49), "HLC 130 60 50" },
    { Color(0x5a, 0xa1, 0x37), "HLC 130 60 60" },
    { Color(0xa4, 0xae, 0x9e), "HLC 130 70 10" },
    { Color(0x9d, 0xb1, 0x8f), "HLC 130 70 20" },
    { Color(0x93, 0xb4, 0x80), "HLC 130 70 30" },
    { Color(0x8a, 0xb7, 0x72), "HLC 130 70 40" },
    { Color(0x7f, 0xba, 0x62), "HLC 130 70 50" },
    { Color(0xbf, 0xca, 0xb9), "HLC 130 80 10" },
    { Color(0xb8, 0xcd, 0xaa), "HLC 130 80 20" },
    { Color(0xae, 0xd0, 0x9a), "HLC 130 80 30" },
    { Color(0xdb, 0xe6, 0xd5), "HLC 130 90 10" },
    { Color(0x28, 0x33, 0x27), "HLC 140 20 10" },
    { Color(0x3e, 0x4a, 0x3d), "HLC 140 30 10" },
    { Color(0x33, 0x4d, 0x33), "HLC 140 30 20" },
    { Color(0x25, 0x50, 0x27), "HLC 140 30 30" },
    { Color(0x55, 0x62, 0x54), "HLC 140 40 10" },
    { Color(0x4a, 0x65, 0x4a), "HLC 140 40 20" },
    { Color(0x3d, 0x68, 0x3e), "HLC 140 40 30" },
    { Color(0x2e, 0x6a, 0x33), "HLC 140 40 40" },
    { Color(0x6e, 0x7a, 0x6c), "HLC 140 50 10" },
    { Color(0x62, 0x7e, 0x62), "HLC 140 50 20" },
    { Color(0x55, 0x81, 0x55), "HLC 140 50 30" },
    { Color(0x48, 0x84, 0x4a), "HLC 140 50 40" },
    { Color(0x36, 0x87, 0x3c), "HLC 140 50 50" },
    { Color(0x87, 0x94, 0x86), "HLC 140 60 10" },
    { Color(0x7b, 0x98, 0x7b), "HLC 140 60 20" },
    { Color(0x6e, 0x9c, 0x6e), "HLC 140 60 30" },
    { Color(0x62, 0x9e, 0x62), "HLC 140 60 40" },
    { Color(0x51, 0xa2, 0x55), "HLC 140 60 50" },
    { Color(0x3b, 0xa4, 0x48), "HLC 140 60 60" },
    { Color(0xa1, 0xaf, 0xa0), "HLC 140 70 10" },
    { Color(0x95, 0xb3, 0x95), "HLC 140 70 20" },
    { Color(0x88, 0xb7, 0x87), "HLC 140 70 30" },
    { Color(0x7c, 0xba, 0x7c), "HLC 140 70 40" },
    { Color(0xbc, 0xca, 0xbb), "HLC 140 80 10" },
    { Color(0xb0, 0xce, 0xaf), "HLC 140 80 20" },
    { Color(0xd8, 0xe6, 0xd7), "HLC 140 90 10" },
    { Color(0x26, 0x34, 0x29), "HLC 150 20 10" },
    { Color(0x3c, 0x4a, 0x3f), "HLC 150 30 10" },
    { Color(0x2d, 0x4e, 0x36), "HLC 150 30 20" },
    { Color(0x18, 0x51, 0x2e), "HLC 150 30 30" },
    { Color(0x53, 0x62, 0x56), "HLC 150 40 10" },
    { Color(0x44, 0x66, 0x4d), "HLC 150 40 20" },
    { Color(0x32, 0x69, 0x44), "HLC 150 40 30" },
    { Color(0x1b, 0x6c, 0x3b), "HLC 150 40 40" },
    { Color(0x00, 0x6f, 0x32), "HLC 150 40 50" },
    { Color(0x6b, 0x7b, 0x6e), "HLC 150 50 10" },
    { Color(0x5c, 0x7f, 0x65), "HLC 150 50 20" },
    { Color(0x4b, 0x83, 0x5c), "HLC 150 50 30" },
    { Color(0x38, 0x86, 0x53), "HLC 150 50 40" },
    { Color(0x14, 0x89, 0x49), "HLC 150 50 50" },
    { Color(0x00, 0x8b, 0x3f), "HLC 150 50 60" },
    { Color(0x85, 0x95, 0x87), "HLC 150 60 10" },
    { Color(0x76, 0x99, 0x7e), "HLC 150 60 20" },
    { Color(0x65, 0x9d, 0x75), "HLC 150 60 30" },
    { Color(0x54, 0xa0, 0x6b), "HLC 150 60 40" },
    { Color(0x3a, 0xa4, 0x62), "HLC 150 60 50" },
    { Color(0x9f, 0xaf, 0xa2), "HLC 150 70 10" },
    { Color(0x90, 0xb4, 0x98), "HLC 150 70 20" },
    { Color(0x7f, 0xb8, 0x8e), "HLC 150 70 30" },
    { Color(0x6e, 0xbc, 0x85), "HLC 150 70 40" },
    { Color(0xba, 0xcb, 0xbd), "HLC 150 80 10" },
    { Color(0xaa, 0xd0, 0xb3), "HLC 150 80 20" },
    { Color(0xd5, 0xe7, 0xd8), "HLC 150 90 10" },
    { Color(0x23, 0x34, 0x2c), "HLC 160 20 10" },
    { Color(0x0f, 0x37, 0x27), "HLC 160 20 20" },
    { Color(0x39, 0x4b, 0x42), "HLC 160 30 10" },
    { Color(0x28, 0x4e, 0x3d), "HLC 160 30 20" },
    { Color(0x05, 0x51, 0x36), "HLC 160 30 30" },
    { Color(0x50, 0x63, 0x59), "HLC 160 40 10" },
    { Color(0x3f, 0x66, 0x54), "HLC 160 40 20" },
    { Color(0x27, 0x6a, 0x4d), "HLC 160 40 30" },
    { Color(0x00, 0x6d, 0x47), "HLC 160 40 40" },
    { Color(0x00, 0x70, 0x40), "HLC 160 40 50" },
    { Color(0x68, 0x7b, 0x71), "HLC 160 50 10" },
    { Color(0x57, 0x80, 0x6c), "HLC 160 50 20" },
    { Color(0x41, 0x84, 0x65), "HLC 160 50 30" },
    { Color(0x23, 0x87, 0x5f), "HLC 160 50 40" },
    { Color(0x00, 0x8a, 0x58), "HLC 160 50 50" },
    { Color(0x00, 0x8d, 0x52), "HLC 160 50 60" },
    { Color(0x00, 0x8f, 0x4c), "HLC 160 50 70" },
    { Color(0x81, 0x95, 0x8b), "HLC 160 60 10" },
    { Color(0x71, 0x9a, 0x85), "HLC 160 60 20" },
    { Color(0x5b, 0x9e, 0x7e), "HLC 160 60 30" },
    { Color(0x42, 0xa2, 0x78), "HLC 160 60 40" },
    { Color(0x0d, 0xa5, 0x70), "HLC 160 60 50" },
    { Color(0x9b, 0xb0, 0xa5), "HLC 160 70 10" },
    { Color(0x8a, 0xb5, 0x9f), "HLC 160 70 20" },
    { Color(0x76, 0xb9, 0x98), "HLC 160 70 30" },
    { Color(0xb6, 0xcb, 0xc0), "HLC 160 80 10" },
    { Color(0xa5, 0xd0, 0xba), "HLC 160 80 20" },
    { Color(0xd2, 0xe8, 0xdc), "HLC 160 90 10" },
    { Color(0x22, 0x34, 0x2f), "HLC 170 20 10" },
    { Color(0x07, 0x38, 0x2b), "HLC 170 20 20" },
    { Color(0x37, 0x4b, 0x45), "HLC 170 30 10" },
    { Color(0x22, 0x4f, 0x41), "HLC 170 30 20" },
    { Color(0x00, 0x52, 0x3e), "HLC 170 30 30" },
    { Color(0x4f, 0x63, 0x5c), "HLC 170 40 10" },
    { Color(0x3a, 0x67, 0x59), "HLC 170 40 20" },
    { Color(0x1a, 0x6b, 0x55), "HLC 170 40 30" },
    { Color(0x00, 0x6e, 0x51), "HLC 170 40 40" },
    { Color(0x00, 0x71, 0x4f), "HLC 170 40 50" },
    { Color(0x67, 0x7c, 0x75), "HLC 170 50 10" },
    { Color(0x53, 0x80, 0x71), "HLC 170 50 20" },
    { Color(0x38, 0x84, 0x6d), "HLC 170 50 30" },
    { Color(0x00, 0x88, 0x69), "HLC 170 50 40" },
    { Color(0x00, 0x8b, 0x67), "HLC 170 50 50" },
    { Color(0x00, 0x8e, 0x64), "HLC 170 50 60" },
    { Color(0x80, 0x95, 0x8e), "HLC 170 60 10" },
    { Color(0x6c, 0x9a, 0x8b), "HLC 170 60 20" },
    { Color(0x53, 0x9f, 0x87), "HLC 170 60 30" },
    { Color(0x30, 0xa3, 0x83), "HLC 170 60 40" },
    { Color(0x00, 0xa6, 0x80), "HLC 170 60 50" },
    { Color(0x9a, 0xb0, 0xa9), "HLC 170 70 10" },
    { Color(0x86, 0xb5, 0xa5), "HLC 170 70 20" },
    { Color(0x6e, 0xba, 0xa1), "HLC 170 70 30" },
    { Color(0xb5, 0xcc, 0xc4), "HLC 170 80 10" },
    { Color(0xa0, 0xd1, 0xc0), "HLC 170 80 20" },
    { Color(0xd1, 0xe8, 0xe0), "HLC 170 90 10" },
    { Color(0x1f, 0x35, 0x30), "HLC 180 20 10" },
    { Color(0x00, 0x38, 0x30), "HLC 180 20 20" },
    { Color(0x35, 0x4b, 0x46), "HLC 180 30 10" },
    { Color(0x1b, 0x4f, 0x46), "HLC 180 30 20" },
    { Color(0x00, 0x52, 0x46), "HLC 180 30 30" },
    { Color(0x4c, 0x63, 0x5e), "HLC 180 40 10" },
    { Color(0x34, 0x67, 0x5e), "HLC 180 40 20" },
    { Color(0x02, 0x6b, 0x5d), "HLC 180 40 30" },
    { Color(0x00, 0x6e, 0x5d), "HLC 180 40 40" },
    { Color(0x64, 0x7c, 0x77), "HLC 180 50 10" },
    { Color(0x4d, 0x81, 0x76), "HLC 180 50 20" },
    { Color(0x2d, 0x85, 0x76), "HLC 180 50 30" },
    { Color(0x00, 0x88, 0x76), "HLC 180 50 40" },
    { Color(0x00, 0x8c, 0x75), "HLC 180 50 50" },
    { Color(0x00, 0x8e, 0x75), "HLC 180 50 60" },
    { Color(0x7d, 0x96, 0x90), "HLC 180 60 10" },
    { Color(0x67, 0x9b, 0x90), "HLC 180 60 20" },
    { Color(0x4a, 0x9f, 0x8f), "HLC 180 60 30" },
    { Color(0x12, 0xa3, 0x8f), "HLC 180 60 40" },
    { Color(0x97, 0xb1, 0xab), "HLC 180 70 10" },
    { Color(0x81, 0xb6, 0xaa), "HLC 180 70 20" },
    { Color(0x66, 0xba, 0xaa), "HLC 180 70 30" },
    { Color(0xb2, 0xcc, 0xc6), "HLC 180 80 10" },
    { Color(0x9c, 0xd1, 0xc5), "HLC 180 80 20" },
    { Color(0xce, 0xe8, 0xe2), "HLC 180 90 10" },
    { Color(0x20, 0x34, 0x31), "HLC 190 20 10" },
    { Color(0x00, 0x38, 0x34), "HLC 190 20 20" },
    { Color(0x36, 0x4b, 0x48), "HLC 190 30 10" },
    { Color(0x1a, 0x4f, 0x4b), "HLC 190 30 20" },
    { Color(0x00, 0x52, 0x4e), "HLC 190 30 30" },
    { Color(0x4d, 0x63, 0x60), "HLC 190 40 10" },
    { Color(0x34, 0x67, 0x62), "HLC 190 40 20" },
    { Color(0x00, 0x6b, 0x65), "HLC 190 40 30" },
    { Color(0x00, 0x6e, 0x68), "HLC 190 40 40" },
    { Color(0x65, 0x7c, 0x78), "HLC 190 50 10" },
    { Color(0x4d, 0x80, 0x7b), "HLC 190 50 20" },
    { Color(0x27, 0x85, 0x7e), "HLC 190 50 30" },
    { Color(0x00, 0x88, 0x82), "HLC 190 50 40" },
    { Color(0x00, 0x8c, 0x83), "HLC 190 50 50" },
    { Color(0x00, 0x8f, 0x86), "HLC 190 50 60" },
    { Color(0x7f, 0x95, 0x92), "HLC 190 60 10" },
    { Color(0x66, 0x9a, 0x95), "HLC 190 60 20" },
    { Color(0x46, 0x9f, 0x98), "HLC 190 60 30" },
    { Color(0x00, 0xa3, 0x9c), "HLC 190 60 40" },
    { Color(0x99, 0xb0, 0xac), "HLC 190 70 10" },
    { Color(0x80, 0xb5, 0xb0), "HLC 190 70 20" },
    { Color(0x62, 0xba, 0xb3), "HLC 190 70 30" },
    { Color(0xb3, 0xcc, 0xc8), "HLC 190 80 10" },
    { Color(0x9b, 0xd1, 0xcb), "HLC 190 80 20" },
    { Color(0xcf, 0xe8, 0xe4), "HLC 190 90 10" },
    { Color(0x1e, 0x34, 0x34), "HLC 200 20 10" },
    { Color(0x00, 0x37, 0x39), "HLC 200 20 20" },
    { Color(0x34, 0x4b, 0x4b), "HLC 200 30 10" },
    { Color(0x18, 0x4f, 0x4f), "HLC 200 30 20" },
    { Color(0x00, 0x52, 0x55), "HLC 200 30 30" },
    { Color(0x4c, 0x63, 0x63), "HLC 200 40 10" },
    { Color(0x33, 0x67, 0x67), "HLC 200 40 20" },
    { Color(0x00, 0x6b, 0x6e), "HLC 200 40 30" },
    { Color(0x00, 0x6e, 0x72), "HLC 200 40 40" },
    { Color(0x64, 0x7c, 0x7c), "HLC 200 50 10" },
    { Color(0x4c, 0x80, 0x80), "HLC 200 50 20" },
    { Color(0x1f, 0x84, 0x87), "HLC 200 50 30" },
    { Color(0x00, 0x88, 0x8c), "HLC 200 50 40" },
    { Color(0x00, 0x8b, 0x92), "HLC 200 50 50" },
    { Color(0x00, 0x8e, 0x97), "HLC 200 50 60" },
    { Color(0x7d, 0x96, 0x95), "HLC 200 60 10" },
    { Color(0x66, 0x9a, 0x9a), "HLC 200 60 20" },
    { Color(0x41, 0x9f, 0xa1), "HLC 200 60 30" },
    { Color(0x00, 0xa3, 0xa6), "HLC 200 60 40" },
    { Color(0x00, 0xa6, 0xad), "HLC 200 60 50" },
    { Color(0x97, 0xb0, 0xb0), "HLC 200 70 10" },
    { Color(0x80, 0xb5, 0xb5), "HLC 200 70 20" },
    { Color(0x5e, 0xba, 0xbc), "HLC 200 70 30" },
    { Color(0xb2, 0xcc, 0xcc), "HLC 200 80 10" },
    { Color(0x9b, 0xd1, 0xd1), "HLC 200 80 20" },
    { Color(0xce, 0xe8, 0xe8), "HLC 200 90 10" },
    { Color(0x1f, 0x34, 0x37), "HLC 210 20 10" },
    { Color(0x00, 0x37, 0x3e), "HLC 210 20 20" },
    { Color(0x35, 0x4b, 0x4e), "HLC 210 30 10" },
    { Color(0x14, 0x4e, 0x56), "HLC 210 30 20" },
    { Color(0x00, 0x52, 0x5d), "HLC 210 30 30" },
    { Color(0x4c, 0x62, 0x66), "HLC 210 40 10" },
    { Color(0x30, 0x67, 0x6e), "HLC 210 40 20" },
    { Color(0x00, 0x6a, 0x76), "HLC 210 40 30" },
    { Color(0x00, 0x6d, 0x7e), "HLC 210 40 40" },
    { Color(0x64, 0x7b, 0x7f), "HLC 210 50 10" },
    { Color(0x4a, 0x80, 0x87), "HLC 210 50 20" },
    { Color(0x1c, 0x84, 0x90), "HLC 210 50 30" },
    { Color(0x00, 0x87, 0x98), "HLC 210 50 40" },
    { Color(0x00, 0x8a, 0xa0), "HLC 210 50 50" },
    { Color(0x7e, 0x95, 0x99), "HLC 210 60 10" },
    { Color(0x64, 0x9a, 0xa2), "HLC 210 60 20" },
    { Color(0x40, 0x9e, 0xaa), "HLC 210 60 30" },
    { Color(0x00, 0xa2, 0xb3), "HLC 210 60 40" },
    { Color(0x00, 0xa5, 0xbb), "HLC 210 60 50" },
    { Color(0x98, 0xb0, 0xb4), "HLC 210 70 10" },
    { Color(0x7e, 0xb5, 0xbd), "HLC 210 70 20" },
    { Color(0x5d, 0xb9, 0xc5), "HLC 210 70 30" },
    { Color(0xb2, 0xcb, 0xcf), "HLC 210 80 10" },
    { Color(0x99, 0xd0, 0xd8), "HLC 210 80 20" },
    { Color(0xce, 0xe7, 0xec), "HLC 210 90 10" },
    { Color(0x20, 0x34, 0x39), "HLC 220 20 10" },
    { Color(0x00, 0x37, 0x41), "HLC 220 20 20" },
    { Color(0x36, 0x4a, 0x50), "HLC 220 30 10" },
    { Color(0x18, 0x4e, 0x59), "HLC 220 30 20" },
    { Color(0x00, 0x51, 0x64), "HLC 220 30 30" },
    { Color(0x4d, 0x62, 0x68), "HLC 220 40 10" },
    { Color(0x33, 0x66, 0x71), "HLC 220 40 20" },
    { Color(0x00, 0x69, 0x7d), "HLC 220 40 30" },
    { Color(0x00, 0x6c, 0x86), "HLC 220 40 40" },
    { Color(0x66, 0x7b, 0x81), "HLC 220 50 10" },
    { Color(0x4d, 0x7f, 0x8b), "HLC 220 50 20" },
    { Color(0x22, 0x83, 0x96), "HLC 220 50 30" },
    { Color(0x00, 0x86, 0xa1), "HLC 220 50 40" },
    { Color(0x00, 0x89, 0xad), "HLC 220 50 50" },
    { Color(0x7f, 0x95, 0x9b), "HLC 220 60 10" },
    { Color(0x67, 0x99, 0xa5), "HLC 220 60 20" },
    { Color(0x44, 0x9d, 0xb1), "HLC 220 60 30" },
    { Color(0x00, 0xa0, 0xbc), "HLC 220 60 40" },
    { Color(0x00, 0xa4, 0xc8), "HLC 220 60 50" },
    { Color(0x99, 0xaf, 0xb6), "HLC 220 70 10" },
    { Color(0x81, 0xb4, 0xc0), "HLC 220 70 20" },
    { Color(0x61, 0xb8, 0xcd), "HLC 220 70 30" },
    { Color(0xb4, 0xcb, 0xd1), "HLC 220 80 10" },
    { Color(0x9c, 0xcf, 0xdc), "HLC 220 80 20" },
    { Color(0xcf, 0xe7, 0xed), "HLC 220 90 10" },
    { Color(0x21, 0x33, 0x3a), "HLC 230 20 10" },
    { Color(0x37, 0x4a, 0x51), "HLC 230 30 10" },
    { Color(0x1e, 0x4d, 0x5e), "HLC 230 30 20" },
    { Color(0x00, 0x50, 0x6a), "HLC 230 30 30" },
    { Color(0x4e, 0x62, 0x69), "HLC 230 40 10" },
    { Color(0x38, 0x65, 0x76), "HLC 230 40 20" },
    { Color(0x00, 0x68, 0x83), "HLC 230 40 30" },
    { Color(0x00, 0x6b, 0x8f), "HLC 230 40 40" },
    { Color(0x67, 0x7a, 0x83), "HLC 230 50 10" },
    { Color(0x51, 0x7e, 0x90), "HLC 230 50 20" },
    { Color(0x2c, 0x81, 0x9d), "HLC 230 50 30" },
    { Color(0x00, 0x84, 0xa9), "HLC 230 50 40" },
    { Color(0x00, 0x87, 0xb7), "HLC 230 50 50" },
    { Color(0x80, 0x94, 0x9d), "HLC 230 60 10" },
    { Color(0x6b, 0x98, 0xaa), "HLC 230 60 20" },
    { Color(0x4b, 0x9b, 0xb8), "HLC 230 60 30" },
    { Color(0x0c, 0x9e, 0xc5), "HLC 230 60 40" },
    { Color(0x00, 0xa2, 0xd3), "HLC 230 60 50" },
    { Color(0x00, 0xa5, 0xe1), "HLC 230 60 60" },
    { Color(0x9a, 0xaf, 0xb7), "HLC 230 70 10" },
    { Color(0x85, 0xb2, 0xc6), "HLC 230 70 20" },
    { Color(0x68, 0xb6, 0xd4), "HLC 230 70 30" },
    { Color(0x41, 0xba, 0xe1), "HLC 230 70 40" },
    { Color(0xb5, 0xca, 0xd3), "HLC 230 80 10" },
    { Color(0xa0, 0xce, 0xe2), "HLC 230 80 20" },
    { Color(0x84, 0xd2, 0xf1), "HLC 230 80 30" },
    { Color(0xd1, 0xe6, 0xef), "HLC 230 90 10" },
    { Color(0x22, 0x33, 0x3c), "HLC 240 20 10" },
    { Color(0x02, 0x35, 0x49), "HLC 240 20 20" },
    { Color(0x38, 0x49, 0x53), "HLC 240 30 10" },
    { Color(0x21, 0x4c, 0x61), "HLC 240 30 20" },
    { Color(0x00, 0x4e, 0x6f), "HLC 240 30 30" },
    { Color(0x50, 0x61, 0x6b), "HLC 240 40 10" },
    { Color(0x3b, 0x64, 0x7a), "HLC 240 40 20" },
    { Color(0x10, 0x67, 0x88), "HLC 240 40 30" },
    { Color(0x00, 0x69, 0x96), "HLC 240 40 40" },
    { Color(0x68, 0x7a, 0x84), "HLC 240 50 10" },
    { Color(0x54, 0x7d, 0x93), "HLC 240 50 20" },
    { Color(0x36, 0x80, 0xa3), "HLC 240 50 30" },
    { Color(0x00, 0x82, 0xb0), "HLC 240 50 40" },
    { Color(0x00, 0x85, 0xc0), "HLC 240 50 50" },
    { Color(0x00, 0x87, 0xd0), "HLC 240 50 60" },
    { Color(0x81, 0x94, 0x9e), "HLC 240 60 10" },
    { Color(0x6e, 0x97, 0xae), "HLC 240 60 20" },
    { Color(0x54, 0x9a, 0xbe), "HLC 240 60 30" },
    { Color(0x28, 0x9c, 0xcc), "HLC 240 60 40" },
    { Color(0x00, 0x9f, 0xdc), "HLC 240 60 50" },
    { Color(0x9c, 0xae, 0xb9), "HLC 240 70 10" },
    { Color(0x88, 0xb1, 0xc9), "HLC 240 70 20" },
    { Color(0x70, 0xb5, 0xda), "HLC 240 70 30" },
    { Color(0x4e, 0xb7, 0xe8), "HLC 240 70 40" },
    { Color(0xb6, 0xca, 0xd5), "HLC 240 80 10" },
    { Color(0xa3, 0xcd, 0xe6), "HLC 240 80 20" },
    { Color(0x8c, 0xd0, 0xf6), "HLC 240 80 30" },
    { Color(0xd2, 0xe6, 0xf1), "HLC 240 90 10" },
    { Color(0x25, 0x32, 0x3d), "HLC 250 20 10" },
    { Color(0x11, 0x34, 0x4a), "HLC 250 20 20" },
    { Color(0x3b, 0x48, 0x54), "HLC 250 30 10" },
    { Color(0x2b, 0x4a, 0x62), "HLC 250 30 20" },
    { Color(0x00, 0x4d, 0x72), "HLC 250 30 30" },
    { Color(0x53, 0x60, 0x6d), "HLC 250 40 10" },
    { Color(0x44, 0x62, 0x7b), "HLC 250 40 20" },
    { Color(0x26, 0x64, 0x8c), "HLC 250 40 30" },
    { Color(0x00, 0x66, 0x9b), "HLC 250 40 40" },
    { Color(0x00, 0x69, 0xab), "HLC 250 40 50" },
    { Color(0x6b, 0x79, 0x86), "HLC 250 50 10" },
    { Color(0x5d, 0x7b, 0x95), "HLC 250 50 20" },
    { Color(0x44, 0x7d, 0xa6), "HLC 250 50 30" },
    { Color(0x1d, 0x7f, 0xb6), "HLC 250 50 40" },
    { Color(0x00, 0x82, 0xc7), "HLC 250 50 50" },
    { Color(0x85, 0x93, 0xa0), "HLC 250 60 10" },
    { Color(0x76, 0x95, 0xb0), "HLC 250 60 20" },
    { Color(0x60, 0x97, 0xc2), "HLC 250 60 30" },
    { Color(0x44, 0x99, 0xd1), "HLC 250 60 40" },
    { Color(0x9f, 0xad, 0xbb), "HLC 250 70 10" },
    { Color(0x90, 0xaf, 0xcb), "HLC 250 70 20" },
    { Color(0x7b, 0xb2, 0xdd), "HLC 250 70 30" },
    { Color(0xba, 0xc9, 0xd7), "HLC 250 80 10" },
    { Color(0xac, 0xcb, 0xe8), "HLC 250 80 20" },
    { Color(0xd6, 0xe5, 0xf3), "HLC 250 90 10" },
    { Color(0x29, 0x31, 0x3d), "HLC 260 20 10" },
    { Color(0x19, 0x32, 0x4c), "HLC 260 20 20" },
    { Color(0x3f, 0x47, 0x55), "HLC 260 30 10" },
    { Color(0x31, 0x49, 0x64), "HLC 260 30 20" },
    { Color(0x18, 0x4a, 0x74), "HLC 260 30 30" },
    { Color(0x57, 0x5f, 0x6d), "HLC 260 40 10" },
    { Color(0x49, 0x61, 0x7d), "HLC 260 40 20" },
    { Color(0x36, 0x62, 0x8e), "HLC 260 40 30" },
    { Color(0x0a, 0x64, 0x9e), "HLC 260 40 40" },
    { Color(0x00, 0x65, 0xaf), "HLC 260 40 50" },
    { Color(0x6f, 0x78, 0x86), "HLC 260 50 10" },
    { Color(0x62, 0x79, 0x97), "HLC 260 50 20" },
    { Color(0x51, 0x7b, 0xa8), "HLC 260 50 30" },
    { Color(0x37, 0x7d, 0xb9), "HLC 260 50 40" },
    { Color(0x89, 0x92, 0xa0), "HLC 260 60 10" },
    { Color(0x7c, 0x93, 0xb2), "HLC 260 60 20" },
    { Color(0x6c, 0x95, 0xc3), "HLC 260 60 30" },
    { Color(0x56, 0x96, 0xd5), "HLC 260 60 40" },
    { Color(0xa3, 0xac, 0xbb), "HLC 260 70 10" },
    { Color(0x96, 0xae, 0xcd), "HLC 260 70 20" },
    { Color(0x87, 0xaf, 0xdf), "HLC 260 70 30" },
    { Color(0xbe, 0xc7, 0xd7), "HLC 260 80 10" },
    { Color(0xb1, 0xc9, 0xea), "HLC 260 80 20" },
    { Color(0xda, 0xe3, 0xf3), "HLC 260 90 10" },
    { Color(0x2a, 0x31, 0x3f), "HLC 270 20 10" },
    { Color(0x1f, 0x31, 0x4e), "HLC 270 20 20" },
    { Color(0x40, 0x47, 0x56), "HLC 270 30 10" },
    { Color(0x37, 0x48, 0x66), "HLC 270 30 20" },
    { Color(0x28, 0x48, 0x75), "HLC 270 30 30" },
    { Color(0x05, 0x49, 0x85), "HLC 270 30 40" },
    { Color(0x58, 0x5f, 0x6e), "HLC 270 40 10" },
    { Color(0x4f, 0x5f, 0x7f), "HLC 270 40 20" },
    { Color(0x43, 0x60, 0x8f), "HLC 270 40 30" },
    { Color(0x30, 0x60, 0xa0), "HLC 270 40 40" },
    { Color(0x00, 0x61, 0xb1), "HLC 270 40 50" },
    { Color(0x70, 0x77, 0x88), "HLC 270 50 10" },
    { Color(0x68, 0x78, 0x99), "HLC 270 50 20" },
    { Color(0x5d, 0x78, 0xaa), "HLC 270 50 30" },
    { Color(0x4d, 0x79, 0xbb), "HLC 270 50 40" },
    { Color(0x8a, 0x91, 0xa2), "HLC 270 60 10" },
    { Color(0x81, 0x92, 0xb4), "HLC 270 60 20" },
    { Color(0x77, 0x92, 0xc5), "HLC 270 60 30" },
    { Color(0xa4, 0xac, 0xbd), "HLC 270 70 10" },
    { Color(0x9c, 0xac, 0xcf), "HLC 270 70 20" },
    { Color(0xbf, 0xc7, 0xd9), "HLC 270 80 10" },
    { Color(0xb7, 0xc7, 0xec), "HLC 270 80 20" },
    { Color(0xdb, 0xe3, 0xf5), "HLC 270 90 10" },
    { Color(0x2c, 0x30, 0x3d), "HLC 280 20 10" },
    { Color(0x27, 0x30, 0x4c), "HLC 280 20 20" },
    { Color(0x43, 0x46, 0x55), "HLC 280 30 10" },
    { Color(0x3e, 0x46, 0x64), "HLC 280 30 20" },
    { Color(0x37, 0x46, 0x74), "HLC 280 30 30" },
    { Color(0x2b, 0x45, 0x84), "HLC 280 30 40" },
    { Color(0x0e, 0x46, 0x94), "HLC 280 30 50" },
    { Color(0x5a, 0x5e, 0x6d), "HLC 280 40 10" },
    { Color(0x56, 0x5e, 0x7d), "HLC 280 40 20" },
    { Color(0x50, 0x5d, 0x8e), "HLC 280 40 30" },
    { Color(0x47, 0x5d, 0x9e), "HLC 280 40 40" },
    { Color(0x73, 0x77, 0x86), "HLC 280 50 10" },
    { Color(0x6f, 0x76, 0x97), "HLC 280 50 20" },
    { Color(0x69, 0x76, 0xa8), "HLC 280 50 30" },
    { Color(0x61, 0x75, 0xba), "HLC 280 50 40" },
    { Color(0x8c, 0x90, 0xa0), "HLC 280 60 10" },
    { Color(0x88, 0x90, 0xb2), "HLC 280 60 20" },
    { Color(0x83, 0x8f, 0xc4), "HLC 280 60 30" },
    { Color(0xa7, 0xab, 0xbb), "HLC 280 70 10" },
    { Color(0xa3, 0xaa, 0xce), "HLC 280 70 20" },
    { Color(0xc2, 0xc6, 0xd7), "HLC 280 80 10" },
    { Color(0xbe, 0xc5, 0xea), "HLC 280 80 20" },
    { Color(0xde, 0xe2, 0xf4), "HLC 280 90 10" },
    { Color(0x30, 0x2f, 0x3d), "HLC 290 20 10" },
    { Color(0x2e, 0x2e, 0x4b), "HLC 290 20 20" },
    { Color(0x2a, 0x2c, 0x5a), "HLC 290 20 30" },
    { Color(0x46, 0x45, 0x55), "HLC 290 30 10" },
    { Color(0x45, 0x44, 0x63), "HLC 290 30 20" },
    { Color(0x42, 0x43, 0x73), "HLC 290 30 30" },
    { Color(0x3e, 0x41, 0x81), "HLC 290 30 40" },
    { Color(0x37, 0x40, 0x91), "HLC 290 30 50" },
    { Color(0x5e, 0x5d, 0x6d), "HLC 290 40 10" },
    { Color(0x5c, 0x5c, 0x7c), "HLC 290 40 20" },
    { Color(0x5b, 0x5a, 0x8c), "HLC 290 40 30" },
    { Color(0x57, 0x59, 0x9b), "HLC 290 40 40" },
    { Color(0x77, 0x76, 0x86), "HLC 290 50 10" },
    { Color(0x75, 0x74, 0x96), "HLC 290 50 20" },
    { Color(0x74, 0x73, 0xa7), "HLC 290 50 30" },
    { Color(0x90, 0x8f, 0xa0), "HLC 290 60 10" },
    { Color(0x8f, 0x8e, 0xb0), "HLC 290 60 20" },
    { Color(0x8e, 0x8c, 0xc2), "HLC 290 60 30" },
    { Color(0xab, 0xaa, 0xbb), "HLC 290 70 10" },
    { Color(0xaa, 0xa8, 0xcc), "HLC 290 70 20" },
    { Color(0xc6, 0xc5, 0xd7), "HLC 290 80 10" },
    { Color(0xe2, 0xe1, 0xf4), "HLC 290 90 10" },
    { Color(0x33, 0x2e, 0x3c), "HLC 300 20 10" },
    { Color(0x35, 0x2c, 0x49), "HLC 300 20 20" },
    { Color(0x36, 0x29, 0x57), "HLC 300 20 30" },
    { Color(0x4a, 0x44, 0x53), "HLC 300 30 10" },
    { Color(0x4c, 0x42, 0x61), "HLC 300 30 20" },
    { Color(0x4d, 0x3f, 0x6f), "HLC 300 30 30" },
    { Color(0x4e, 0x3d, 0x7c), "HLC 300 30 40" },
    { Color(0x4e, 0x3a, 0x8b), "HLC 300 30 50" },
    { Color(0x62, 0x5c, 0x6b), "HLC 300 40 10" },
    { Color(0x64, 0x59, 0x7a), "HLC 300 40 20" },
    { Color(0x66, 0x57, 0x89), "HLC 300 40 30" },
    { Color(0x68, 0x54, 0x96), "HLC 300 40 40" },
    { Color(0x7b, 0x74, 0x85), "HLC 300 50 10" },
    { Color(0x7d, 0x72, 0x94), "HLC 300 50 20" },
    { Color(0x7f, 0x6f, 0xa3), "HLC 300 50 30" },
    { Color(0x81, 0x6c, 0xb1), "HLC 300 50 40" },
    { Color(0x95, 0x8e, 0x9f), "HLC 300 60 10" },
    { Color(0x97, 0x8b, 0xaf), "HLC 300 60 20" },
    { Color(0x9a, 0x89, 0xbf), "HLC 300 60 30" },
    { Color(0xaf, 0xa8, 0xba), "HLC 300 70 10" },
    { Color(0xb2, 0xa6, 0xca), "HLC 300 70 20" },
    { Color(0xcb, 0xc4, 0xd5), "HLC 300 80 10" },
    { Color(0xce, 0xc1, 0xe6), "HLC 300 80 20" },
    { Color(0xe7, 0xe0, 0xf2), "HLC 300 90 10" },
    { Color(0x35, 0x2d, 0x3b), "HLC 310 20 10" },
    { Color(0x39, 0x2a, 0x46), "HLC 310 20 20" },
    { Color(0x3e, 0x26, 0x52), "HLC 310 20 30" },
    { Color(0x4c, 0x44, 0x52), "HLC 310 30 10" },
    { Color(0x51, 0x41, 0x5e), "HLC 310 30 20" },
    { Color(0x56, 0x3c, 0x6b), "HLC 310 30 30" },
    { Color(0x5a, 0x38, 0x76), "HLC 310 30 40" },
    { Color(0x5e, 0x33, 0x83), "HLC 310 30 50" },
    { Color(0x64, 0x5b, 0x6a), "HLC 310 40 10" },
    { Color(0x69, 0x58, 0x77), "HLC 310 40 20" },
    { Color(0x6f, 0x54, 0x84), "HLC 310 40 30" },
    { Color(0x73, 0x50, 0x90), "HLC 310 40 40" },
    { Color(0x7d, 0x74, 0x83), "HLC 310 50 10" },
    { Color(0x82, 0x70, 0x91), "HLC 310 50 20" },
    { Color(0x89, 0x6c, 0x9f), "HLC 310 50 30" },
    { Color(0x8d, 0x68, 0xab), "HLC 310 50 40" },
    { Color(0x97, 0x8d, 0x9d), "HLC 310 60 10" },
    { Color(0x9c, 0x8a, 0xab), "HLC 310 60 20" },
    { Color(0xa3, 0x86, 0xba), "HLC 310 60 30" },
    { Color(0xb2, 0xa8, 0xb8), "HLC 310 70 10" },
    { Color(0xb7, 0xa4, 0xc7), "HLC 310 70 20" },
    { Color(0xcd, 0xc3, 0xd4), "HLC 310 80 10" },
    { Color(0xd3, 0xbf, 0xe3), "HLC 310 80 20" },
    { Color(0xe9, 0xdf, 0xf0), "HLC 310 90 10" },
    { Color(0x37, 0x2d, 0x39), "HLC 320 20 10" },
    { Color(0x3f, 0x28, 0x42), "HLC 320 20 20" },
    { Color(0x46, 0x22, 0x4d), "HLC 320 20 30" },
    { Color(0x4e, 0x43, 0x50), "HLC 320 30 10" },
    { Color(0x57, 0x3e, 0x5a), "HLC 320 30 20" },
    { Color(0x5e, 0x39, 0x65), "HLC 320 30 30" },
    { Color(0x65, 0x33, 0x6e), "HLC 320 30 40" },
    { Color(0x6b, 0x2b, 0x7a), "HLC 320 30 50" },
    { Color(0x66, 0x5b, 0x68), "HLC 320 40 10" },
    { Color(0x70, 0x56, 0x72), "HLC 320 40 20" },
    { Color(0x78, 0x51, 0x7e), "HLC 320 40 30" },
    { Color(0x7f, 0x4b, 0x88), "HLC 320 40 40" },
    { Color(0x86, 0x44, 0x94), "HLC 320 40 50" },
    { Color(0x7f, 0x73, 0x81), "HLC 320 50 10" },
    { Color(0x89, 0x6e, 0x8c), "HLC 320 50 20" },
    { Color(0x92, 0x69, 0x98), "HLC 320 50 30" },
    { Color(0x99, 0x64, 0xa2), "HLC 320 50 40" },
    { Color(0x99, 0x8d, 0x9b), "HLC 320 60 10" },
    { Color(0xa4, 0x88, 0xa6), "HLC 320 60 20" },
    { Color(0xad, 0x83, 0xb3), "HLC 320 60 30" },
    { Color(0xb4, 0xa7, 0xb6), "HLC 320 70 10" },
    { Color(0xbf, 0xa2, 0xc1), "HLC 320 70 20" },
    { Color(0xc8, 0x9d, 0xce), "HLC 320 70 30" },
    { Color(0xd0, 0xc2, 0xd2), "HLC 320 80 10" },
    { Color(0xdb, 0xbd, 0xdd), "HLC 320 80 20" },
    { Color(0xec, 0xde, 0xee), "HLC 320 90 10" },
    { Color(0x39, 0x2c, 0x38), "HLC 330 20 10" },
    { Color(0x43, 0x27, 0x3f), "HLC 330 20 20" },
    { Color(0x4c, 0x1f, 0x47), "HLC 330 20 30" },
    { Color(0x50, 0x42, 0x4f), "HLC 330 30 10" },
    { Color(0x5b, 0x3d, 0x57), "HLC 330 30 20" },
    { Color(0x65, 0x36, 0x5f), "HLC 330 30 30" },
    { Color(0x6d, 0x2f, 0x67), "HLC 330 30 40" },
    { Color(0x76, 0x24, 0x6f), "HLC 330 30 50" },
    { Color(0x69, 0x5a, 0x67), "HLC 330 40 10" },
    { Color(0x74, 0x54, 0x6f), "HLC 330 40 20" },
    { Color(0x7e, 0x4e, 0x77), "HLC 330 40 30" },
    { Color(0x87, 0x47, 0x80), "HLC 330 40 40" },
    { Color(0x91, 0x3e, 0x88), "HLC 330 40 50" },
    { Color(0x82, 0x72, 0x80), "HLC 330 50 10" },
    { Color(0x8e, 0x6d, 0x88), "HLC 330 50 20" },
    { Color(0x99, 0x67, 0x91), "HLC 330 50 30" },
    { Color(0xa2, 0x60, 0x9a), "HLC 330 50 40" },
    { Color(0x9c, 0x8c, 0x9a), "HLC 330 60 10" },
    { Color(0xa8, 0x86, 0xa3), "HLC 330 60 20" },
    { Color(0xb4, 0x80, 0xac), "HLC 330 60 30" },
    { Color(0xbe, 0x7a, 0xb5), "HLC 330 60 40" },
    { Color(0xb7, 0xa6, 0xb4), "HLC 330 70 10" },
    { Color(0xc4, 0xa1, 0xbe), "HLC 330 70 20" },
    { Color(0xd0, 0x9a, 0xc7), "HLC 330 70 30" },
    { Color(0xd2, 0xc2, 0xd0), "HLC 330 80 10" },
    { Color(0xe0, 0xbc, 0xda), "HLC 330 80 20" },
    { Color(0xef, 0xdd, 0xec), "HLC 330 90 10" },
    { Color(0x3c, 0x2c, 0x35), "HLC 340 20 10" },
    { Color(0x46, 0x26, 0x3a), "HLC 340 20 20" },
    { Color(0x51, 0x1d, 0x40), "HLC 340 20 30" },
    { Color(0x53, 0x42, 0x4c), "HLC 340 30 10" },
    { Color(0x5e, 0x3c, 0x51), "HLC 340 30 20" },
    { Color(0x6a, 0x34, 0x57), "HLC 340 30 30" },
    { Color(0x74, 0x2b, 0x5c), "HLC 340 30 40" },
    { Color(0x7f, 0x1b, 0x62), "HLC 340 30 50" },
    { Color(0x6b, 0x59, 0x63), "HLC 340 40 10" },
    { Color(0x77, 0x53, 0x69), "HLC 340 40 20" },
    { Color(0x84, 0x4c, 0x6f), "HLC 340 40 30" },
    { Color(0x8f, 0x44, 0x75), "HLC 340 40 40" },
    { Color(0x9a, 0x38, 0x7b), "HLC 340 40 50" },
    { Color(0xa5, 0x29, 0x81), "HLC 340 40 60" },
    { Color(0x85, 0x72, 0x7c), "HLC 340 50 10" },
    { Color(0x91, 0x6c, 0x82), "HLC 340 50 20" },
    { Color(0x9e, 0x65, 0x89), "HLC 340 50 30" },
    { Color(0xaa, 0x5d, 0x8e), "HLC 340 50 40" },
    { Color(0xb6, 0x53, 0x95), "HLC 340 50 50" },
    { Color(0x9f, 0x8b, 0x96), "HLC 340 60 10" },
    { Color(0xac, 0x85, 0x9c), "HLC 340 60 20" },
    { Color(0xba, 0x7e, 0xa3), "HLC 340 60 30" },
    { Color(0xc6, 0x77, 0xa9), "HLC 340 60 40" },
    { Color(0xba, 0xa6, 0xb1), "HLC 340 70 10" },
    { Color(0xc8, 0xa0, 0xb7), "HLC 340 70 20" },
    { Color(0xd6, 0x98, 0xbe), "HLC 340 70 30" },
    { Color(0xd5, 0xc1, 0xcc), "HLC 340 80 10" },
    { Color(0xe4, 0xbb, 0xd2), "HLC 340 80 20" },
    { Color(0xf2, 0xdd, 0xe8), "HLC 340 90 10" },
    { Color(0x3c, 0x2c, 0x32), "HLC 350 20 10" },
    { Color(0x48, 0x25, 0x35), "HLC 350 20 20" },
    { Color(0x53, 0x1b, 0x39), "HLC 350 20 30" },
    { Color(0x54, 0x42, 0x49), "HLC 350 30 10" },
    { Color(0x61, 0x3b, 0x4c), "HLC 350 30 20" },
    { Color(0x6d, 0x33, 0x4f), "HLC 350 30 30" },
    { Color(0x79, 0x28, 0x53), "HLC 350 30 40" },
    { Color(0x84, 0x15, 0x55), "HLC 350 30 50" },
    { Color(0x6c, 0x59, 0x60), "HLC 350 40 10" },
    { Color(0x7a, 0x53, 0x64), "HLC 350 40 20" },
    { Color(0x87, 0x4b, 0x68), "HLC 350 40 30" },
    { Color(0x94, 0x41, 0x6b), "HLC 350 40 40" },
    { Color(0xa0, 0x34, 0x6d), "HLC 350 40 50" },
    { Color(0xac, 0x21, 0x71), "HLC 350 40 60" },
    { Color(0x86, 0x72, 0x79), "HLC 350 50 10" },
    { Color(0x94, 0x6b, 0x7d), "HLC 350 50 20" },
    { Color(0xa2, 0x63, 0x81), "HLC 350 50 30" },
    { Color(0xb0, 0x5a, 0x84), "HLC 350 50 40" },
    { Color(0xbd, 0x4f, 0x86), "HLC 350 50 50" },
    { Color(0xc9, 0x41, 0x8a), "HLC 350 50 60" },
    { Color(0xa0, 0x8b, 0x93), "HLC 350 60 10" },
    { Color(0xaf, 0x85, 0x97), "HLC 350 60 20" },
    { Color(0xbe, 0x7d, 0x9b), "HLC 350 60 30" },
    { Color(0xcc, 0x74, 0x9e), "HLC 350 60 40" },
    { Color(0xda, 0x6a, 0xa1), "HLC 350 60 50" },
    { Color(0xbb, 0xa6, 0xad), "HLC 350 70 10" },
    { Color(0xcb, 0x9f, 0xb1), "HLC 350 70 20" },
    { Color(0xda, 0x97, 0xb5), "HLC 350 70 30" },
    { Color(0xe9, 0x8f, 0xb9), "HLC 350 70 40" },
    { Color(0xd7, 0xc1, 0xc9), "HLC 350 80 10" },
    { Color(0xe7, 0xba, 0xcd), "HLC 350 80 20" },
    { Color(0xf3, 0xdd, 0xe5), "HLC 350 90 10" },
    { Color(0x3e, 0x2b, 0x31), "HLC 360 20 10" },
    { Color(0x4b, 0x24, 0x31), "HLC 360 20 20" },
    { Color(0x56, 0x19, 0x32), "HLC 360 20 30" },
    { Color(0x56, 0x41, 0x47), "HLC 360 30 10" },
    { Color(0x63, 0x3a, 0x47), "HLC 360 30 20" },
    { Color(0x70, 0x32, 0x48), "HLC 360 30 30" },
    { Color(0x7c, 0x26, 0x48), "HLC 360 30 40" },
    { Color(0x87, 0x11, 0x49), "HLC 360 30 50" },
    { Color(0x6e, 0x59, 0x5f), "HLC 360 40 10" },
    { Color(0x7d, 0x52, 0x5f), "HLC 360 40 20" },
    { Color(0x8a, 0x4a, 0x60), "HLC 360 40 30" },
    { Color(0x97, 0x40, 0x60), "HLC 360 40 40" },
    { Color(0xa4, 0x32, 0x61), "HLC 360 40 50" },
    { Color(0xb0, 0x1d, 0x61), "HLC 360 40 60" },
    { Color(0x88, 0x71, 0x77), "HLC 360 50 10" },
    { Color(0x97, 0x6a, 0x78), "HLC 360 50 20" },
    { Color(0xa6, 0x62, 0x78), "HLC 360 50 30" },
    { Color(0xb4, 0x59, 0x79), "HLC 360 50 40" },
    { Color(0xc1, 0x4e, 0x79), "HLC 360 50 50" },
    { Color(0xce, 0x3f, 0x7a), "HLC 360 50 60" },
    { Color(0xdb, 0x28, 0x7a), "HLC 360 50 70" },
    { Color(0xa2, 0x8b, 0x91), "HLC 360 60 10" },
    { Color(0xb2, 0x84, 0x91), "HLC 360 60 20" },
    { Color(0xc2, 0x7c, 0x92), "HLC 360 60 30" },
    { Color(0xd0, 0x73, 0x92), "HLC 360 60 40" },
    { Color(0xde, 0x69, 0x93), "HLC 360 60 50" },
    { Color(0xbd, 0xa5, 0xab), "HLC 360 70 10" },
    { Color(0xce, 0x9e, 0xac), "HLC 360 70 20" },
    { Color(0xde, 0x96, 0xac), "HLC 360 70 30" },
    { Color(0xed, 0x8e, 0xad), "HLC 360 70 40" },
    { Color(0xd9, 0xc0, 0xc7), "HLC 360 80 10" },
    { Color(0xeb, 0xb9, 0xc7), "HLC 360 80 20" },
    { Color(0xf6, 0xdc, 0xe3), "HLC 360 90 10" },
};

class PaletteBuiltinLibreColors : public Palette
{
public:
    virtual ~PaletteBuiltinLibreColors() override
    {
    }

    virtual const OUString& GetName() override
    {
        static const OUString aName("LibreColors");
        return aName;
    }

    virtual const OUString& GetPath() override
    {
        static const OUString aPath;
        return aPath;
    }

    virtual void LoadColorSet( SvxColorValueSet& rColorSet ) override
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(aLibreColors); i++)
        {
            rColorSet.InsertItem(i, aLibreColors[i].mColor, OUString::createFromAscii(aLibreColors[i].mName));
        }
    }

    virtual bool IsValid() override
    {
        return true;
    }
};

PaletteManager::PaletteManager() :
    mnMaxRecentColors(Application::GetSettings().GetStyleSettings().GetColorValueSetColumnCount()),
    mnNumOfPalettes(2),
    mnCurrentPalette(0),
    mnColorCount(0),
    mpBtnUpdater(nullptr),
    mLastColor(COL_AUTO),
    maColorSelectFunction(PaletteManager::DispatchColorCommand),
    m_context(comphelper::getProcessComponentContext())
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if(pDocSh)
    {
        const SfxPoolItem* pItem = nullptr;
        if( nullptr != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) )
            pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }
    if(!pColorList.is())
        pColorList = XColorList::CreateStdColorList();
    LoadPalettes();
    mnNumOfPalettes += m_Palettes.size();
}

PaletteManager::~PaletteManager()
{
}

void PaletteManager::LoadPalettes()
{
    m_Palettes.clear();
    OUString aPalPaths = SvtPathOptions().GetPalettePath();

    std::stack<OUString> aDirs;
    sal_Int32 nIndex = 0;
    do
    {
        aDirs.push(aPalPaths.getToken(0, ';', nIndex));
    }
    while (nIndex >= 0);

    std::unique_ptr<Palette> pPalette;
    std::set<OUString> aNames;
    //try all entries palette path list user first, then
    //system, ignoring duplicate file names
    while (!aDirs.empty())
    {
        OUString aPalPath = aDirs.top();
        aDirs.pop();

        osl::Directory aDir(aPalPath);
        osl::DirectoryItem aDirItem;
        osl::FileStatus aFileStat( osl_FileStatus_Mask_FileName |
                                   osl_FileStatus_Mask_FileURL  |
                                   osl_FileStatus_Mask_Type     );
        if( aDir.open() == osl::FileBase::E_None )
        {
            while( aDir.getNextItem(aDirItem) == osl::FileBase::E_None )
            {
                aDirItem.getFileStatus(aFileStat);
                if(aFileStat.isRegular() || aFileStat.isLink())
                {
                    OUString aFName = aFileStat.getFileName();
                    INetURLObject aURLObj( aFileStat.getFileURL() );
                    OUString aFNameWithoutExt = aURLObj.GetBase();
                    if (aNames.find(aFName) == aNames.end())
                    {
                        if( aFName.endsWithIgnoreAsciiCase(".gpl") )
                            pPalette.reset(new PaletteGPL(aFileStat.getFileURL(), aFNameWithoutExt));
                        else if( aFName.endsWithIgnoreAsciiCase(".soc") )
                            pPalette.reset(new PaletteSOC(aFileStat.getFileURL(), aFNameWithoutExt));
                        else if ( aFName.endsWithIgnoreAsciiCase(".ase") )
                            pPalette.reset(new PaletteASE(aFileStat.getFileURL(), aFNameWithoutExt));

                        if( pPalette && pPalette->IsValid() )
                            m_Palettes.push_back( std::move(pPalette) );
                        aNames.insert(aFNameWithoutExt);
                    }
                }
            }
        }
    }
    pPalette.reset(new PaletteBuiltinLibreColors());
    assert(pPalette && pPalette->IsValid());
    m_Palettes.push_back( std::move(pPalette) );
}

void PaletteManager::ReloadColorSet(SvxColorValueSet &rColorSet)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    if( mnCurrentPalette == 0)
    {
        rColorSet.Clear();
        css::uno::Sequence< sal_Int32 > CustomColorList( officecfg::Office::Common::UserColors::CustomColor::get() );
        css::uno::Sequence< OUString > CustomColorNameList( officecfg::Office::Common::UserColors::CustomColorName::get() );
        int nIx = 1;
        for (int i = 0; i < CustomColorList.getLength(); ++i)
        {
            Color aColor(CustomColorList[i]);
            rColorSet.InsertItem(nIx, aColor, CustomColorNameList[i]);
            ++nIx;
        }
    }
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
    {
        // Add doc colors to palette
        std::set<Color> aColors = pDocSh->GetDocColors();
        mnColorCount = aColors.size();
        rColorSet.Clear();
        rColorSet.addEntriesForColorSet(aColors, SVX_RESSTR( RID_SVXSTR_DOC_COLOR_PREFIX ) + " " );
    }
    else
    {
        m_Palettes[mnCurrentPalette - 1]->LoadColorSet( rColorSet );
        mnColorCount = rColorSet.GetItemCount();
    }
}

void PaletteManager::ReloadRecentColorSet(SvxColorValueSet& rColorSet)
{
    maRecentColors.clear();
    rColorSet.Clear();
    css::uno::Sequence< sal_Int32 > Colorlist(officecfg::Office::Common::UserColors::RecentColor::get());
    css::uno::Sequence< OUString > ColorNamelist(officecfg::Office::Common::UserColors::RecentColorName::get());
    int nIx = 1;
    const bool bHasColorNames = Colorlist.getLength() == ColorNamelist.getLength();
    for (int i = 0; i < Colorlist.getLength(); ++i)
    {
        Color aColor(Colorlist[i]);
        OUString sColorName = bHasColorNames ? ColorNamelist[i] : ("#" + aColor.AsRGBHexString().toAsciiUpperCase());
        maRecentColors.push_back(std::make_pair(aColor, sColorName));
        rColorSet.InsertItem(nIx, aColor, sColorName);
        ++nIx;
    }
}

std::vector<OUString> PaletteManager::GetPaletteList()
{
    std::vector<OUString> aPaletteNames;

    aPaletteNames.push_back( SVX_RESSTR( RID_SVXSTR_CUSTOM_PAL ) );
    for (auto const& it : m_Palettes)
    {
        aPaletteNames.push_back( (*it).GetName() );
    }
    aPaletteNames.push_back( SVX_RESSTR ( RID_SVXSTR_DOC_COLORS ) );

    return aPaletteNames;
}

void PaletteManager::SetPalette( sal_Int32 nPos )
{
    mnCurrentPalette = nPos;
    if( nPos != mnNumOfPalettes - 1 && nPos != 0)
    {
        pColorList = XPropertyList::AsColorList(
                            XPropertyList::CreatePropertyListFromURL(
                            XPropertyListType::Color, GetSelectedPalettePath()));
        auto name = GetPaletteName(); // may change pColorList
        pColorList->SetName(name);
        if(pColorList->Load())
        {
            SfxObjectShell* pShell = SfxObjectShell::Current();
            if (pShell != nullptr)
            {
                SvxColorListItem aColorItem(pColorList, SID_COLOR_TABLE);
                pShell->PutItem( aColorItem );
            }
        }
    }
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
    officecfg::Office::Common::UserColors::PaletteName::set(GetPaletteName(), batch);
    batch->commit();
}

sal_Int32 PaletteManager::GetPalette()
{
    return mnCurrentPalette;
}

OUString PaletteManager::GetPaletteName()
{
    std::vector<OUString> aNames(GetPaletteList());
    if(mnCurrentPalette != mnNumOfPalettes - 1 && mnCurrentPalette != 0)
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if(pDocSh)
        {
            const SfxPoolItem* pItem = nullptr;
            if( nullptr != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) )
                pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
        }
    }
    return aNames[mnCurrentPalette];
}

OUString PaletteManager::GetSelectedPalettePath()
{
    if(mnCurrentPalette != mnNumOfPalettes - 1 && mnCurrentPalette != 0)
        return m_Palettes[mnCurrentPalette - 1]->GetPath();
    else
        return OUString();
}

long PaletteManager::GetColorCount()
{
    return mnColorCount;
}

long PaletteManager::GetRecentColorCount()
{
    return maRecentColors.size();
}

const Color& PaletteManager::GetLastColor()
{
    return mLastColor;
}

void PaletteManager::SetLastColor(const Color& rLastColor)
{
    mLastColor = rLastColor;
}

void PaletteManager::AddRecentColor(const Color& rRecentColor, const OUString& rName, bool bFront)
{
    auto itColor = std::find_if(maRecentColors.begin(),
                                maRecentColors.end(),
                                [rRecentColor] (const NamedColor &a) { return a.first == rRecentColor; });
    // if recent color to be added is already in list, remove it
    if( itColor != maRecentColors.end() )
        maRecentColors.erase( itColor );

    if (maRecentColors.size() == mnMaxRecentColors)
        maRecentColors.pop_back();
    if (bFront)
        maRecentColors.push_front(std::make_pair(rRecentColor, rName));
    else
        maRecentColors.push_back(std::make_pair(rRecentColor, rName));
    css::uno::Sequence< sal_Int32 > aColorList(maRecentColors.size());
    css::uno::Sequence< OUString > aColorNameList(maRecentColors.size());
    for (size_t i = 0; i < maRecentColors.size(); ++i)
    {
        aColorList[i] = static_cast<sal_Int32>(maRecentColors[i].first.GetColor());
        aColorNameList[i] = maRecentColors[i].second;
    }
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
    officecfg::Office::Common::UserColors::RecentColor::set(aColorList, batch);
    officecfg::Office::Common::UserColors::RecentColorName::set(aColorNameList, batch);
    batch->commit();
}

void PaletteManager::SetBtnUpdater(svx::ToolboxButtonColorUpdater* pBtnUpdater)
{
    mpBtnUpdater = pBtnUpdater;
    mLastColor = mpBtnUpdater->GetCurrentColor();
}

void PaletteManager::SetColorSelectFunction(const std::function<void(const OUString&, const NamedColor&)>& aColorSelectFunction)
{
    maColorSelectFunction = aColorSelectFunction;
}

void PaletteManager::PopupColorPicker(const OUString& aCommand)
{
    // The calling object goes away during aColorDlg.Execute(), so we must copy this
    OUString aCommandCopy = aCommand;
    SvColorDialog aColorDlg( nullptr );
    aColorDlg.SetColor ( mLastColor );
    aColorDlg.SetMode( svtools::ColorPickerMode_MODIFY );
    if( aColorDlg.Execute() == RET_OK )
    {
        if (mpBtnUpdater)
            mpBtnUpdater->Update( aColorDlg.GetColor() );
        mLastColor = aColorDlg.GetColor();
        OUString sColorName = ("#" + mLastColor.AsRGBHexString().toAsciiUpperCase());
        NamedColor aNamedColor = std::make_pair(mLastColor, sColorName);
        AddRecentColor(mLastColor, sColorName);
        maColorSelectFunction(aCommandCopy, aNamedColor);
    }
}

void PaletteManager::DispatchColorCommand(const OUString& aCommand, const NamedColor& rColor)
{
    using namespace css::uno;
    using namespace css::frame;
    using namespace css::beans;
    using namespace css::util;

    Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<XDesktop2> xDesktop = Desktop::create(xContext);
    Reference<XDispatchProvider> xDispatchProvider(xDesktop->getCurrentFrame(), UNO_QUERY );
    if (xDispatchProvider.is())
    {
        INetURLObject aObj( aCommand );

        Sequence<PropertyValue> aArgs(1);
        aArgs[0].Name = aObj.GetURLPath();
        aArgs[0].Value = makeAny(sal_Int32(rColor.first.GetColor()));

        URL aTargetURL;
        aTargetURL.Complete = aCommand;
        Reference<XURLTransformer> xURLTransformer(URLTransformer::create(comphelper::getProcessComponentContext()));
        xURLTransformer->parseStrict(aTargetURL);

        Reference<XDispatch> xDispatch = xDispatchProvider->queryDispatch(aTargetURL, OUString(), 0);
        if (xDispatch.is())
            xDispatch->dispatch(aTargetURL, aArgs);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
