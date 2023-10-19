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

#include <rtl/ustring.hxx>

inline constexpr OUString FMURL_FORM_POSITION = u".uno:FormController/positionForm"_ustr;
inline constexpr OUString FMURL_FORM_RECORDCOUNT = u".uno:FormController/RecordCount"_ustr;
inline constexpr OUString FMURL_RECORD_MOVEFIRST = u".uno:FormController/moveToFirst"_ustr;
inline constexpr OUString FMURL_RECORD_MOVEPREV = u".uno:FormController/moveToPrev"_ustr;
inline constexpr OUString FMURL_RECORD_MOVENEXT = u".uno:FormController/moveToNext"_ustr;
inline constexpr OUString FMURL_RECORD_MOVELAST = u".uno:FormController/moveToLast"_ustr;
inline constexpr OUString FMURL_RECORD_MOVETONEW = u".uno:FormController/moveToNew"_ustr;
inline constexpr OUString FMURL_RECORD_UNDO = u".uno:FormController/undoRecord"_ustr;
inline constexpr OUString FMURL_RECORD_SAVE = u".uno:FormController/saveRecord"_ustr;
inline constexpr OUString FMURL_RECORD_DELETE = u".uno:FormController/deleteRecord"_ustr;
inline constexpr OUString FMURL_FORM_REFRESH = u".uno:FormController/refreshForm"_ustr;
inline constexpr OUString FMURL_FORM_REFRESH_CURRENT_CONTROL = u".uno:FormController/refreshCurrentControl"_ustr;
inline constexpr OUString FMURL_FORM_SORT_UP = u".uno:FormController/sortUp"_ustr;
inline constexpr OUString FMURL_FORM_SORT_DOWN = u".uno:FormController/sortDown"_ustr;
inline constexpr OUString FMURL_FORM_SORT = u".uno:FormController/sort"_ustr;
inline constexpr OUString FMURL_FORM_AUTO_FILTER = u".uno:FormController/autoFilter"_ustr;
inline constexpr OUString FMURL_FORM_FILTER = u".uno:FormController/filter"_ustr;
inline constexpr OUString FMURL_FORM_APPLY_FILTER = u".uno:FormController/applyFilter"_ustr;
inline constexpr OUString FMURL_FORM_REMOVE_FILTER = u".uno:FormController/removeFilterOrder"_ustr;
inline constexpr OUString FMURL_CONFIRM_DELETION = u".uno:FormSlots/ConfirmDeletion"_ustr;
inline constexpr OUString FMURL_COMPONENT_FORMGRIDVIEW = u".component:DB/FormGridView"_ustr;
inline constexpr OUString FMURL_GRIDVIEW_CLEARVIEW = u".uno:FormSlots/ClearView"_ustr;
inline constexpr OUString FMURL_GRIDVIEW_ADDCOLUMN = u".uno:FormSlots/AddGridColumn"_ustr;
inline constexpr OUString FMURL_GRIDVIEW_ATTACHTOFORM = u".uno:FormSlots/AttachToForm"_ustr;
inline constexpr OUString FMARG_ATTACHTO_MASTERFORM = u"MasterForm"_ustr;
inline constexpr OUString FMARG_ADDCOL_COLUMNTYPE = u"ColumnType"_ustr;
inline constexpr OUString FMARG_ADDCOL_COLUMNPOS = u"ColumnPosition"_ustr;

#endif // INCLUDED_SVX_SOURCE_INC_FMURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
