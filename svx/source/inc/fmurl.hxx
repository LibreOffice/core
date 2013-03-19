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

#ifndef _SVX_FMURL_HXX
#define _SVX_FMURL_HXX

#define FMURL_FORM_POSITION                rtl::OUString( ".uno:FormController/positionForm" )
#define FMURL_FORM_RECORDCOUNT             rtl::OUString( ".uno:FormController/RecordCount" )
#define FMURL_RECORD_MOVEFIRST             rtl::OUString( ".uno:FormController/moveToFirst" )
#define FMURL_RECORD_MOVEPREV              rtl::OUString( ".uno:FormController/moveToPrev" )
#define FMURL_RECORD_MOVENEXT              rtl::OUString( ".uno:FormController/moveToNext" )
#define FMURL_RECORD_MOVELAST              rtl::OUString( ".uno:FormController/moveToLast" )
#define FMURL_RECORD_MOVETONEW             rtl::OUString( ".uno:FormController/moveToNew" )
#define FMURL_RECORD_UNDO                  rtl::OUString( ".uno:FormController/undoRecord" )
#define FMURL_RECORD_SAVE                  rtl::OUString( ".uno:FormController/saveRecord" )
#define FMURL_RECORD_DELETE                rtl::OUString( ".uno:FormController/deleteRecord" )
#define FMURL_FORM_REFRESH                 rtl::OUString( ".uno:FormController/refreshForm" )
#define FMURL_FORM_REFRESH_CURRENT_CONTROL rtl::OUString( ".uno:FormController/refreshCurrentControl" )
#define FMURL_FORM_SORT_UP                 rtl::OUString( ".uno:FormController/sortUp" )
#define FMURL_FORM_SORT_DOWN               rtl::OUString( ".uno:FormController/sortDown" )
#define FMURL_FORM_SORT                    rtl::OUString( ".uno:FormController/sort" )
#define FMURL_FORM_AUTO_FILTER             rtl::OUString( ".uno:FormController/autoFilter" )
#define FMURL_FORM_FILTER                  rtl::OUString( ".uno:FormController/filter" )
#define FMURL_FORM_APPLY_FILTER            rtl::OUString( ".uno:FormController/applyFilter" )
#define FMURL_FORM_REMOVE_FILTER           rtl::OUString( ".uno:FormController/removeFilterOrder" )
#define FMURL_CONFIRM_DELETION             rtl::OUString( ".uno:FormSlots/ConfirmDeletion" )
#define FMURL_COMPONENT_FORMGRIDVIEW       rtl::OUString( ".component:DB/FormGridView" )
#define FMURL_GRIDVIEW_CLEARVIEW           rtl::OUString( ".uno:FormSlots/ClearView" )
#define FMURL_GRIDVIEW_ADDCOLUMN           rtl::OUString( ".uno:FormSlots/AddGridColumn" )
#define FMURL_GRIDVIEW_ATTACHTOFORM        rtl::OUString( ".uno:FormSlots/AttachToForm" )
#define FMARG_ATTACHTO_MASTERFORM          rtl::OUString( "MasterForm" )
#define FMARG_ADDCOL_COLUMNTYPE            rtl::OUString( "ColumnType" )
#define FMARG_ADDCOL_COLUMNPOS             rtl::OUString( "ColumnPosition" )

#endif // _SVX_FMURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
