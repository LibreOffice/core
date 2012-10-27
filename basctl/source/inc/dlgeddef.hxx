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

#ifndef BASCTL_DLGEDDEF_HXX
#define BASCTL_DLGEDDEF_HXX

#include <sal/types.h>

namespace basctl
{

const sal_uInt32 DlgInventor = sal_uInt32('D')*0x00000001+
                           sal_uInt32('L')*0x00000100+
                           sal_uInt32('G')*0x00010000+
                           sal_uInt32('1')*0x01000000;

enum
{
    OBJ_DLG_CONTROL         =  1,
    OBJ_DLG_DIALOG          =  2,
    OBJ_DLG_PUSHBUTTON      =  3,
    OBJ_DLG_RADIOBUTTON     =  4,
    OBJ_DLG_CHECKBOX        =  5,
    OBJ_DLG_LISTBOX         =  6,
    OBJ_DLG_COMBOBOX        =  7,
    OBJ_DLG_GROUPBOX        =  8,
    OBJ_DLG_EDIT            =  9,
    OBJ_DLG_FIXEDTEXT       = 10,
    OBJ_DLG_IMAGECONTROL    = 11,
    OBJ_DLG_PROGRESSBAR     = 12,
    OBJ_DLG_HSCROLLBAR      = 13,
    OBJ_DLG_VSCROLLBAR      = 14,
    OBJ_DLG_HFIXEDLINE      = 15,
    OBJ_DLG_VFIXEDLINE      = 16,
    OBJ_DLG_DATEFIELD       = 17,
    OBJ_DLG_TIMEFIELD       = 18,
    OBJ_DLG_NUMERICFIELD    = 19,
    OBJ_DLG_CURRENCYFIELD   = 20,
    OBJ_DLG_FORMATTEDFIELD  = 21,
    OBJ_DLG_PATTERNFIELD    = 22,
    OBJ_DLG_FILECONTROL     = 23,
    OBJ_DLG_TREECONTROL     = 24,
    OBJ_DLG_SPINBUTTON      = 25,

    OBJ_DLG_FORMRADIO       = 26,
    OBJ_DLG_FORMCHECK       = 27,
    OBJ_DLG_FORMLIST        = 28,
    OBJ_DLG_FORMCOMBO       = 29,
    OBJ_DLG_FORMSPIN        = 30,
    OBJ_DLG_FORMVSCROLL     = 31,
    OBJ_DLG_FORMHSCROLL     = 32,
};

// control properties
#define DLGED_PROP_BACKGROUNDCOLOR   "BackgroundColor"
#define DLGED_PROP_DROPDOWN          "Dropdown"
#define DLGED_PROP_FORMATSSUPPLIER   "FormatsSupplier"
#define DLGED_PROP_HEIGHT            "Height"
#define DLGED_PROP_LABEL             "Label"
#define DLGED_PROP_NAME              "Name"
#define DLGED_PROP_ORIENTATION       "Orientation"
#define DLGED_PROP_POSITIONX         "PositionX"
#define DLGED_PROP_POSITIONY         "PositionY"
#define DLGED_PROP_STEP              "Step"
#define DLGED_PROP_TABINDEX          "TabIndex"
#define DLGED_PROP_TEXTCOLOR         "TextColor"
#define DLGED_PROP_TEXTLINECOLOR     "TextLineColor"
#define DLGED_PROP_WIDTH             "Width"
#define DLGED_PROP_DECORATION        "Decoration"


} // namespace basctl

#endif // BASCTL_DLGEDDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
