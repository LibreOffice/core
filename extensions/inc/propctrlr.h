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

#define UID_PROP_DLG_FONT_TYPE                          "EXTENSIONS_UID_PROP_DLG_FONT_TYPE"
#define UID_PROP_DLG_IMAGE_URL                          "EXTENSIONS_UID_PROP_DLG_IMAGE_URL"
#define UID_PROP_DLG_BACKGROUNDCOLOR                    "EXTENSIONS_UID_PROP_DLG_BACKGROUNDCOLOR"
#define UID_PROP_DLG_SYMBOLCOLOR                        "EXTENSIONS_UID_PROP_DLG_SYMBOLCOLOR"
#define UID_PROP_DLG_ATTR_DATASOURCE                    "EXTENSIONS_UID_PROP_DLG_ATTR_DATASOURCE"
#define UID_PROP_DLG_ATTR_TARGET_URL                    "EXTENSIONS_UID_PROP_DLG_ATTR_TARGET_URL"
#define UID_PROP_DLG_NUMBER_FORMAT                      "EXTENSIONS_UID_PROP_DLG_NUMBER_FORMAT"
#define UID_PROP_DLG_CONTROLLABEL                       "EXTENSIONS_UID_PROP_DLG_CONTROLLABEL"
#define UID_PROP_DLG_FILLCOLOR                          "EXTENSIONS_UID_PROP_DLG_FILLCOLOR"
#define UID_PROP_DLG_TABINDEX                           "EXTENSIONS_UID_PROP_DLG_TABINDEX"
#define UID_PROP_DLG_SQLCOMMAND                         "EXTENSIONS_UID_PROP_DLG_SQLCOMMAND"
#define UID_PROP_DLG_FORMLINKFIELDS                     "EXTENSIONS_UID_PROP_DLG_FORMLINKFIELDS"
#define UID_PROP_DLG_FILTER                             "EXTENSIONS_UID_PROP_DLG_FILTER"
#define UID_PROP_DLG_ORDER                              "EXTENSIONS_UID_PROP_DLG_ORDER"
#define UID_PROP_DLG_SELECTION                          "EXTENSIONS_UID_PROP_DLG_SELECTION"
#define UID_PROP_DLG_BIND_EXPRESSION                    "EXTENSIONS_UID_PROP_DLG_BIND_EXPRESSION"
#define UID_PROP_DLG_XSD_REQUIRED                       "EXTENSIONS_UID_PROP_DLG_XSD_REQUIRED"
#define UID_PROP_DLG_XSD_RELEVANT                       "EXTENSIONS_UID_PROP_DLG_XSD_RELEVANT"
#define UID_PROP_DLG_XSD_READONLY                       "EXTENSIONS_UID_PROP_DLG_XSD_READONLY"
#define UID_PROP_DLG_XSD_CONSTRAINT                     "EXTENSIONS_UID_PROP_DLG_XSD_CONSTRAINT"
#define UID_PROP_DLG_XSD_CALCULATION                    "EXTENSIONS_UID_PROP_DLG_XSD_CALCULATION"
#define UID_PROP_ADD_DATA_TYPE                          "EXTENSIONS_UID_PROP_ADD_DATA_TYPE"
#define UID_PROP_REMOVE_DATA_TYPE                       "EXTENSIONS_UID_PROP_REMOVE_DATA_TYPE"
#define UID_PROP_DLG_BORDERCOLOR                        "EXTENSIONS_UID_PROP_DLG_BORDERCOLOR"

#define UID_BRWEVT_APPROVEACTIONPERFORMED               "EXTENSIONS_UID_BRWEVT_APPROVEACTIONPERFORMED"
#define UID_BRWEVT_ACTIONPERFORMED                      "EXTENSIONS_UID_BRWEVT_ACTIONPERFORMED"
#define UID_BRWEVT_CHANGED                              "EXTENSIONS_UID_BRWEVT_CHANGED"
#define UID_BRWEVT_TEXTCHANGED                          "EXTENSIONS_UID_BRWEVT_TEXTCHANGED"
#define UID_BRWEVT_ITEMSTATECHANGED                     "EXTENSIONS_UID_BRWEVT_ITEMSTATECHANGED"
#define UID_BRWEVT_FOCUSGAINED                          "EXTENSIONS_UID_BRWEVT_FOCUSGAINED"
#define UID_BRWEVT_FOCUSLOST                            "EXTENSIONS_UID_BRWEVT_FOCUSLOST"
#define UID_BRWEVT_KEYTYPED                             "EXTENSIONS_UID_BRWEVT_KEYTYPED"
#define UID_BRWEVT_KEYUP                                "EXTENSIONS_UID_BRWEVT_KEYUP"
#define UID_BRWEVT_MOUSEENTERED                         "EXTENSIONS_UID_BRWEVT_MOUSEENTERED"
#define UID_BRWEVT_MOUSEDRAGGED                         "EXTENSIONS_UID_BRWEVT_MOUSEDRAGGED"
#define UID_BRWEVT_MOUSEMOVED                           "EXTENSIONS_UID_BRWEVT_MOUSEMOVED"
#define UID_BRWEVT_MOUSEPRESSED                         "EXTENSIONS_UID_BRWEVT_MOUSEPRESSED"
#define UID_BRWEVT_MOUSERELEASED                        "EXTENSIONS_UID_BRWEVT_MOUSERELEASED"
#define UID_BRWEVT_MOUSEEXITED                          "EXTENSIONS_UID_BRWEVT_MOUSEEXITED"
#define UID_BRWEVT_APPROVERESETTED                      "EXTENSIONS_UID_BRWEVT_APPROVERESETTED"
#define UID_BRWEVT_RESETTED                             "EXTENSIONS_UID_BRWEVT_RESETTED"
#define UID_BRWEVT_SUBMITTED                            "EXTENSIONS_UID_BRWEVT_SUBMITTED"
#define UID_BRWEVT_BEFOREUPDATE                         "EXTENSIONS_UID_BRWEVT_BEFOREUPDATE"
#define UID_BRWEVT_AFTERUPDATE                          "EXTENSIONS_UID_BRWEVT_AFTERUPDATE"
#define UID_BRWEVT_LOADED                               "EXTENSIONS_UID_BRWEVT_LOADED"
#define UID_BRWEVT_RELOADING                            "EXTENSIONS_UID_BRWEVT_RELOADING"
#define UID_BRWEVT_RELOADED                             "EXTENSIONS_UID_BRWEVT_RELOADED"
#define UID_BRWEVT_UNLOADING                            "EXTENSIONS_UID_BRWEVT_UNLOADING"
#define UID_BRWEVT_UNLOADED                             "EXTENSIONS_UID_BRWEVT_UNLOADED"
#define UID_BRWEVT_CONFIRMDELETE                        "EXTENSIONS_UID_BRWEVT_CONFIRMDELETE"
#define UID_BRWEVT_APPROVEROWCHANGE                     "EXTENSIONS_UID_BRWEVT_APPROVEROWCHANGE"
#define UID_BRWEVT_ROWCHANGE                            "EXTENSIONS_UID_BRWEVT_ROWCHANGE"
#define UID_BRWEVT_POSITIONING                          "EXTENSIONS_UID_BRWEVT_POSITIONING"
#define UID_BRWEVT_POSITIONED                           "EXTENSIONS_UID_BRWEVT_POSITIONED"
#define UID_BRWEVT_APPROVEPARAMETER                     "EXTENSIONS_UID_BRWEVT_APPROVEPARAMETER"
#define UID_BRWEVT_ERROROCCURRED                        "EXTENSIONS_UID_BRWEVT_ERROROCCURRED"
#define UID_BRWEVT_ADJUSTMENTVALUECHANGED               "EXTENSIONS_UID_BRWEVT_ADJUSTMENTVALUECHANGED"

#endif // EXTENSIONS_PROPCTRLR_HRC

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
