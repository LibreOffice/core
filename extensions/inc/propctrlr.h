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

#ifndef EXTENSIONS_PROPCTRLR_H
#define EXTENSIONS_PROPCTRLR_H

inline constexpr OUStringLiteral UID_PROP_DLG_FONT_TYPE = u"EXTENSIONS_UID_PROP_DLG_FONT_TYPE";
inline constexpr OUStringLiteral UID_PROP_DLG_IMAGE_URL = u"EXTENSIONS_UID_PROP_DLG_IMAGE_URL";
inline constexpr OUStringLiteral UID_PROP_DLG_BACKGROUNDCOLOR = u"EXTENSIONS_UID_PROP_DLG_BACKGROUNDCOLOR";
inline constexpr OUStringLiteral UID_PROP_DLG_SYMBOLCOLOR = u"EXTENSIONS_UID_PROP_DLG_SYMBOLCOLOR";
inline constexpr OUStringLiteral UID_PROP_DLG_ATTR_DATASOURCE = u"EXTENSIONS_UID_PROP_DLG_ATTR_DATASOURCE";
inline constexpr OUStringLiteral UID_PROP_DLG_ATTR_TARGET_URL = u"EXTENSIONS_UID_PROP_DLG_ATTR_TARGET_URL";
inline constexpr OUStringLiteral UID_PROP_DLG_NUMBER_FORMAT = u"EXTENSIONS_UID_PROP_DLG_NUMBER_FORMAT";
inline constexpr OUStringLiteral UID_PROP_DLG_CONTROLLABEL = u"EXTENSIONS_UID_PROP_DLG_CONTROLLABEL";
inline constexpr OUStringLiteral UID_PROP_DLG_FILLCOLOR = u"EXTENSIONS_UID_PROP_DLG_FILLCOLOR";
inline constexpr OUStringLiteral UID_PROP_DLG_TABINDEX = u"EXTENSIONS_UID_PROP_DLG_TABINDEX";
inline constexpr OUStringLiteral UID_PROP_DLG_SQLCOMMAND = u"EXTENSIONS_UID_PROP_DLG_SQLCOMMAND";
inline constexpr OUStringLiteral UID_PROP_DLG_FORMLINKFIELDS = u"EXTENSIONS_UID_PROP_DLG_FORMLINKFIELDS";
inline constexpr OUStringLiteral UID_PROP_DLG_FILTER = u"EXTENSIONS_UID_PROP_DLG_FILTER";
inline constexpr OUStringLiteral UID_PROP_DLG_ORDER = u"EXTENSIONS_UID_PROP_DLG_ORDER";
inline constexpr OUStringLiteral UID_PROP_DLG_SELECTION = u"EXTENSIONS_UID_PROP_DLG_SELECTION";
inline constexpr OUStringLiteral UID_PROP_DLG_BIND_EXPRESSION = u"EXTENSIONS_UID_PROP_DLG_BIND_EXPRESSION";
inline constexpr OUStringLiteral UID_PROP_DLG_XSD_REQUIRED = u"EXTENSIONS_UID_PROP_DLG_XSD_REQUIRED";
inline constexpr OUStringLiteral UID_PROP_DLG_XSD_RELEVANT = u"EXTENSIONS_UID_PROP_DLG_XSD_RELEVANT";
inline constexpr OUStringLiteral UID_PROP_DLG_XSD_READONLY = u"EXTENSIONS_UID_PROP_DLG_XSD_READONLY";
inline constexpr OUStringLiteral UID_PROP_DLG_XSD_CONSTRAINT = u"EXTENSIONS_UID_PROP_DLG_XSD_CONSTRAINT";
inline constexpr OUStringLiteral UID_PROP_DLG_XSD_CALCULATION = u"EXTENSIONS_UID_PROP_DLG_XSD_CALCULATION";
inline constexpr OUStringLiteral UID_PROP_ADD_DATA_TYPE = u"EXTENSIONS_UID_PROP_ADD_DATA_TYPE";
inline constexpr OUStringLiteral UID_PROP_REMOVE_DATA_TYPE = u"EXTENSIONS_UID_PROP_REMOVE_DATA_TYPE";
inline constexpr OUStringLiteral UID_PROP_DLG_BORDERCOLOR = u"EXTENSIONS_UID_PROP_DLG_BORDERCOLOR";

inline constexpr OStringLiteral UID_BRWEVT_APPROVEACTIONPERFORMED = "EXTENSIONS_UID_BRWEVT_APPROVEACTIONPERFORMED";
inline constexpr OStringLiteral UID_BRWEVT_ACTIONPERFORMED = "EXTENSIONS_UID_BRWEVT_ACTIONPERFORMED";
inline constexpr OStringLiteral UID_BRWEVT_CHANGED = "EXTENSIONS_UID_BRWEVT_CHANGED";
inline constexpr OStringLiteral UID_BRWEVT_TEXTCHANGED = "EXTENSIONS_UID_BRWEVT_TEXTCHANGED";
inline constexpr OStringLiteral UID_BRWEVT_ITEMSTATECHANGED = "EXTENSIONS_UID_BRWEVT_ITEMSTATECHANGED";
inline constexpr OStringLiteral UID_BRWEVT_FOCUSGAINED = "EXTENSIONS_UID_BRWEVT_FOCUSGAINED";
inline constexpr OStringLiteral UID_BRWEVT_FOCUSLOST = "EXTENSIONS_UID_BRWEVT_FOCUSLOST";
inline constexpr OStringLiteral UID_BRWEVT_KEYTYPED = "EXTENSIONS_UID_BRWEVT_KEYTYPED";
inline constexpr OStringLiteral UID_BRWEVT_KEYUP = "EXTENSIONS_UID_BRWEVT_KEYUP";
inline constexpr OStringLiteral UID_BRWEVT_MOUSEENTERED = "EXTENSIONS_UID_BRWEVT_MOUSEENTERED";
inline constexpr OStringLiteral UID_BRWEVT_MOUSEDRAGGED = "EXTENSIONS_UID_BRWEVT_MOUSEDRAGGED";
inline constexpr OStringLiteral UID_BRWEVT_MOUSEMOVED = "EXTENSIONS_UID_BRWEVT_MOUSEMOVED";
inline constexpr OStringLiteral UID_BRWEVT_MOUSEPRESSED = "EXTENSIONS_UID_BRWEVT_MOUSEPRESSED";
inline constexpr OStringLiteral UID_BRWEVT_MOUSERELEASED = "EXTENSIONS_UID_BRWEVT_MOUSERELEASED";
inline constexpr OStringLiteral UID_BRWEVT_MOUSEEXITED = "EXTENSIONS_UID_BRWEVT_MOUSEEXITED";
inline constexpr OStringLiteral UID_BRWEVT_APPROVERESETTED = "EXTENSIONS_UID_BRWEVT_APPROVERESETTED";
inline constexpr OStringLiteral UID_BRWEVT_RESETTED = "EXTENSIONS_UID_BRWEVT_RESETTED";
inline constexpr OStringLiteral UID_BRWEVT_SUBMITTED = "EXTENSIONS_UID_BRWEVT_SUBMITTED";
inline constexpr OStringLiteral UID_BRWEVT_BEFOREUPDATE = "EXTENSIONS_UID_BRWEVT_BEFOREUPDATE";
inline constexpr OStringLiteral UID_BRWEVT_AFTERUPDATE = "EXTENSIONS_UID_BRWEVT_AFTERUPDATE";
inline constexpr OStringLiteral UID_BRWEVT_LOADED = "EXTENSIONS_UID_BRWEVT_LOADED";
inline constexpr OStringLiteral UID_BRWEVT_RELOADING = "EXTENSIONS_UID_BRWEVT_RELOADING";
inline constexpr OStringLiteral UID_BRWEVT_RELOADED = "EXTENSIONS_UID_BRWEVT_RELOADED";
inline constexpr OStringLiteral UID_BRWEVT_UNLOADING = "EXTENSIONS_UID_BRWEVT_UNLOADING";
inline constexpr OStringLiteral UID_BRWEVT_UNLOADED = "EXTENSIONS_UID_BRWEVT_UNLOADED";
inline constexpr OStringLiteral UID_BRWEVT_CONFIRMDELETE = "EXTENSIONS_UID_BRWEVT_CONFIRMDELETE";
inline constexpr OStringLiteral UID_BRWEVT_APPROVEROWCHANGE = "EXTENSIONS_UID_BRWEVT_APPROVEROWCHANGE";
inline constexpr OStringLiteral UID_BRWEVT_ROWCHANGE = "EXTENSIONS_UID_BRWEVT_ROWCHANGE";
inline constexpr OStringLiteral UID_BRWEVT_POSITIONING = "EXTENSIONS_UID_BRWEVT_POSITIONING";
inline constexpr OStringLiteral UID_BRWEVT_POSITIONED = "EXTENSIONS_UID_BRWEVT_POSITIONED";
inline constexpr OStringLiteral UID_BRWEVT_APPROVEPARAMETER = "EXTENSIONS_UID_BRWEVT_APPROVEPARAMETER";
inline constexpr OStringLiteral UID_BRWEVT_ERROROCCURRED = "EXTENSIONS_UID_BRWEVT_ERROROCCURRED";
inline constexpr OStringLiteral UID_BRWEVT_ADJUSTMENTVALUECHANGED = "EXTENSIONS_UID_BRWEVT_ADJUSTMENTVALUECHANGED";

#endif // EXTENSIONS_PROPCTRLR_HRC

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
