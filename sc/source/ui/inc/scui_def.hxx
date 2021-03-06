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

#include <o3tl/typed_flags_set.hxx>

#define SCRET_COLS 0x42
#define SCRET_ROWS 0x43

#define FDS_OPT_NONE        0  // from filldlg.hxx
#define FDS_OPT_HORZ        1   // from filldlg.hxx
#define FDS_OPT_VERT        2  // from filldlg.hxx

enum class InsertContentsFlags {
    NONE       = 0x00,
    NoEmpty    = 0x01, //from inscodlg.hxx
    Trans      = 0x02, //from inscodlg.hxx
    Link       = 0x04  //from inscodlg.hxx
};
namespace o3tl {
    template<> struct typed_flags<InsertContentsFlags> : is_typed_flags<InsertContentsFlags, 0x07> {};
}

enum class CellShiftDisabledFlags {
    NONE  = 0x00,
    Down  = 0x01, //from inscodlg.hxx
    Right = 0x02  //from inscodlg.hxx
};
namespace o3tl {
    template<> struct typed_flags<CellShiftDisabledFlags> : is_typed_flags<CellShiftDisabledFlags, 0x3> {};
}

enum class CreateNameFlags {
    NONE       = 0,
    Top        = 1, //from namecrea.hxx
    Left       = 2, //from namecrea.hxx
    Bottom     = 4, //from namecrea.hxx
    Right      = 8, //from namecrea.hxx
};
namespace o3tl {
    template<> struct typed_flags<CreateNameFlags> : is_typed_flags<CreateNameFlags, 0xf> {};
}

#define BTN_PASTE_NAME  100  // from namepast.hxx
#define BTN_PASTE_LIST  101  // from namepast.hxx
#define BTN_PASTE_CLOSE 102  // from namepast.hxx

#define BTN_EXTEND_RANGE       150
#define BTN_CURRENT_SELECTION  151
#define SCRET_REMOVE    0x42 //from subtdlg.hxx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
