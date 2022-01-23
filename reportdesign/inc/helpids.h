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

#include <rtl/ustring.hxx>

inline constexpr OUStringLiteral UID_RPT_PROP_FORMULA = u"REPORTDESIGN_UID_RPT_PROP_FORMULA";
inline constexpr OUStringLiteral UID_RPT_RPT_PROP_DLG_FONT_TYPE = u"REPORTDESIGN_UID_RPT_RPT_PROP_DLG_FONT_TYPE";
inline constexpr OUStringLiteral UID_RPT_RPT_PROP_DLG_AREA = u"REPORTDESIGN_UID_RPT_RPT_PROP_DLG_AREA";
inline constexpr OUStringLiteral UID_RPT_PROP_CHARTTYPE_DLG = u"REPORTDESIGN_UID_RPT_PROP_CHARTTYPE_DLG";
inline constexpr OUStringLiteral UID_RPT_PROP_DLG_LINKFIELDS = u"REPORTDESIGN_UID_RPT_PROP_DLG_LINKFIELDS";

#define UID_RPT_RPT_APP_VIEW                          "REPORTDESIGN_UID_RPT_RPT_APP_VIEW"
inline constexpr OStringLiteral UID_RPT_REPORTWINDOW = "REPORTDESIGN_UID_RPT_REPORTWINDOW";
inline constexpr OStringLiteral HID_REPORTSECTION = "REPORTDESIGN_HID_REPORTSECTION";
inline constexpr OStringLiteral HID_RPT_FIELDEXPRESSION = "REPORTDESIGN_HID_RPT_FIELDEXPRESSION";
inline constexpr OStringLiteral HID_RPT_FIELD_SEL = "REPORTDESIGN_HID_RPT_FIELD_SEL";
#define HID_RPT_FIELD_SEL_WIN                          "REPORTDESIGN_HID_RPT_FIELD_SEL_WIN"
inline constexpr OStringLiteral HID_RPT_PROPDLG_TAB_GENERAL = "REPORTDESIGN_HID_RPT_PROPDLG_TAB_GENERAL";
inline constexpr OStringLiteral HID_RPT_PROPDLG_TAB_DATA = "REPORTDESIGN_HID_RPT_PROPDLG_TAB_DATA";

inline constexpr OStringLiteral HID_RPT_PROP_FORCENEWPAGE = "REPORTDESIGN_HID_RPT_PROP_FORCENEWPAGE";
inline constexpr OStringLiteral HID_RPT_PROP_NEWROWORCOL = "REPORTDESIGN_HID_RPT_PROP_NEWROWORCOL";
inline constexpr OStringLiteral HID_RPT_PROP_KEEPTOGETHER = "REPORTDESIGN_HID_RPT_PROP_KEEPTOGETHER";
inline constexpr OStringLiteral HID_RPT_PROP_CANGROW = "REPORTDESIGN_HID_RPT_PROP_CANGROW";
inline constexpr OStringLiteral HID_RPT_PROP_CANSHRINK = "REPORTDESIGN_HID_RPT_PROP_CANSHRINK";
inline constexpr OStringLiteral HID_RPT_PROP_REPEATSECTION = "REPORTDESIGN_HID_RPT_PROP_REPEATSECTION";
inline constexpr OStringLiteral HID_RPT_PROP_FORMULALIST = "REPORTDESIGN_HID_RPT_PROP_FORMULALIST";
inline constexpr OStringLiteral HID_RPT_PROP_SCOPE = "REPORTDESIGN_HID_RPT_PROP_SCOPE";
inline constexpr OStringLiteral HID_RPT_PROP_TYPE = "REPORTDESIGN_HID_RPT_PROP_TYPE";
inline constexpr OStringLiteral HID_RPT_PROP_MASTERFIELDS = "REPORTDESIGN_HID_RPT_PROP_MASTERFIELDS";
inline constexpr OStringLiteral HID_RPT_PROP_DETAILFIELDS = "REPORTDESIGN_HID_RPT_PROP_DETAILFIELDS";
inline constexpr OStringLiteral HID_RPT_PROP_AREA = "REPORTDESIGN_HID_RPT_PROP_AREA";
inline constexpr OStringLiteral HID_RPT_PROP_MIMETYPE = "REPORTDESIGN_HID_RPT_PROP_MIMETYPE";
// free
// free
inline constexpr OStringLiteral HID_RPT_PROP_PRINTREPEATEDVALUES = "REPORTDESIGN_HID_RPT_PROP_PRINTREPEATEDVALUES";
inline constexpr OStringLiteral HID_RPT_PROP_CONDITIONALPRINTEXPRESSION = "REPORTDESIGN_HID_RPT_PROP_CONDITIONALPRINTEXPRESSION";
inline constexpr OStringLiteral HID_RPT_PROP_VERTICALALIGN = "REPORTDESIGN_HID_RPT_PROP_VERTICALALIGN";
inline constexpr OStringLiteral HID_RPT_PROP_STARTNEWCOLUMN = "REPORTDESIGN_HID_RPT_PROP_STARTNEWCOLUMN";
inline constexpr OStringLiteral HID_RPT_PROP_RESETPAGENUMBER = "REPORTDESIGN_HID_RPT_PROP_RESETPAGENUMBER";
inline constexpr OStringLiteral HID_RPT_PROP_PARAADJUST = "REPORTDESIGN_HID_RPT_PROP_PARAADJUST";
inline constexpr OStringLiteral HID_RPT_PROP_PRINTWHENGROUPCHANGE = "REPORTDESIGN_HID_RPT_PROP_PRINTWHENGROUPCHANGE";
inline constexpr OStringLiteral HID_RPT_PROP_CHARTTYPE = "REPORTDESIGN_HID_RPT_PROP_CHARTTYPE";
// free
// free
inline constexpr OStringLiteral HID_RPT_PROP_VISIBLE = "REPORTDESIGN_HID_RPT_PROP_VISIBLE";
inline constexpr OStringLiteral HID_RPT_PROP_GROUPKEEPTOGETHER = "REPORTDESIGN_HID_RPT_PROP_GROUPKEEPTOGETHER";
inline constexpr OStringLiteral HID_RPT_PROP_PAGEHEADEROPTION = "REPORTDESIGN_HID_RPT_PROP_PAGEHEADEROPTION";
inline constexpr OStringLiteral HID_RPT_PROP_PAGEFOOTEROPTION = "REPORTDESIGN_HID_RPT_PROP_PAGEFOOTEROPTION";
inline constexpr OStringLiteral HID_RPT_PROP_PREVIEW_COUNT = "REPORTDESIGN_HID_RPT_PROP_PREVIEW_COUNT";
inline constexpr OStringLiteral HID_RPT_PROP_DATAFIELD = "REPORTDESIGN_HID_RPT_PROP_DATAFIELD";
inline constexpr OStringLiteral HID_RPT_PROP_BACKCOLOR = "REPORTDESIGN_HID_RPT_PROP_BACKCOLOR";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_POSITIONX = "REPORTDESIGN_HID_RPT_PROP_RPT_POSITIONX";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_POSITIONY = "REPORTDESIGN_HID_RPT_PROP_RPT_POSITIONY";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_WIDTH = "REPORTDESIGN_HID_RPT_PROP_RPT_WIDTH";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_HEIGHT = "REPORTDESIGN_HID_RPT_PROP_RPT_HEIGHT";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_AUTOGROW = "REPORTDESIGN_HID_RPT_PROP_RPT_AUTOGROW";
inline constexpr OStringLiteral HID_RPT_PROP_DEEPTRAVERSING = "REPORTDESIGN_HID_RPT_PROP_DEEPTRAVERSING";
inline constexpr OStringLiteral HID_RPT_PROP_PREEVALUATED = "REPORTDESIGN_HID_RPT_PROP_PREEVALUATED";
inline constexpr OStringLiteral HID_REPORT_NAVIGATOR_TREE = "REPORTDESIGN_HID_REPORT_NAVIGATOR_TREE";
inline constexpr OStringLiteral HID_RPT_PROP_FORMULA = "REPORTDESIGN_HID_RPT_PROP_FORMULA";
inline constexpr OStringLiteral HID_RPT_PROP_INITIALFORMULA = "REPORTDESIGN_HID_RPT_PROP_INITIALFORMULA";
inline constexpr OStringLiteral HID_RPT_PROP_RPT_FONT = "REPORTDESIGN_HID_RPT_PROP_RPT_FONT";
inline constexpr OStringLiteral HID_RPT_PROP_PRESERVEIRI = "REPORTDESIGN_HID_RPT_PROP_PRESERVEIRI";
inline constexpr OStringLiteral HID_RPT_PROP_BACKTRANSPARENT = "REPORTDESIGN_HID_RPT_PROP_BACKTRANSPARENT";
inline constexpr OStringLiteral HID_RPT_PROP_CONTROLBACKGROUNDTRANSPARENT = "REPORTDESIGN_HID_RPT_PROP_CONTROLBACKGROUNDTRANSPARENT";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
