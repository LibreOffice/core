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

#ifndef INCLUDED_SVX_SOURCE_INC_FMURL_HXX
#define INCLUDED_SVX_SOURCE_INC_FMURL_HXX

constexpr OUStringLiteral FMURL_FORM_POSITION = u".uno:FormController/positionForm";
constexpr OUStringLiteral FMURL_FORM_RECORDCOUNT = u".uno:FormController/RecordCount";
constexpr OUStringLiteral FMURL_RECORD_MOVEFIRST = u".uno:FormController/moveToFirst";
constexpr OUStringLiteral FMURL_RECORD_MOVEPREV = u".uno:FormController/moveToPrev";
constexpr OUStringLiteral FMURL_RECORD_MOVENEXT = u".uno:FormController/moveToNext";
constexpr OUStringLiteral FMURL_RECORD_MOVELAST = u".uno:FormController/moveToLast";
constexpr OUStringLiteral FMURL_RECORD_MOVETONEW = u".uno:FormController/moveToNew";
constexpr OUStringLiteral FMURL_RECORD_UNDO = u".uno:FormController/undoRecord";
constexpr OUStringLiteral FMURL_RECORD_SAVE = u".uno:FormController/saveRecord";
constexpr OUStringLiteral FMURL_RECORD_DELETE = u".uno:FormController/deleteRecord";
constexpr OUStringLiteral FMURL_FORM_REFRESH = u".uno:FormController/refreshForm";
constexpr OUStringLiteral FMURL_FORM_REFRESH_CURRENT_CONTROL = u".uno:FormController/refreshCurrentControl";
constexpr OUStringLiteral FMURL_FORM_SORT_UP  = u".uno:FormController/sortUp";
constexpr OUStringLiteral FMURL_FORM_SORT_DOWN = u".uno:FormController/sortDown";
constexpr OUStringLiteral FMURL_FORM_SORT = u".uno:FormController/sort";
constexpr OUStringLiteral FMURL_FORM_AUTO_FILTER = u".uno:FormController/autoFilter";
constexpr OUStringLiteral FMURL_FORM_FILTER = u".uno:FormController/filter";
constexpr OUStringLiteral FMURL_FORM_APPLY_FILTER = u".uno:FormController/applyFilter";
constexpr OUStringLiteral FMURL_FORM_REMOVE_FILTER = u".uno:FormController/removeFilterOrder";
#define FMURL_CONFIRM_DELETION             ".uno:FormSlots/ConfirmDeletion"
#define FMURL_COMPONENT_FORMGRIDVIEW       ".component:DB/FormGridView"
#define FMURL_GRIDVIEW_CLEARVIEW           ".uno:FormSlots/ClearView"
#define FMURL_GRIDVIEW_ADDCOLUMN           ".uno:FormSlots/AddGridColumn"
#define FMURL_GRIDVIEW_ATTACHTOFORM        ".uno:FormSlots/AttachToForm"
#define FMARG_ATTACHTO_MASTERFORM          "MasterForm"
#define FMARG_ADDCOL_COLUMNTYPE            "ColumnType"
#define FMARG_ADDCOL_COLUMNPOS             "ColumnPosition"

#endif // INCLUDED_SVX_SOURCE_INC_FMURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
