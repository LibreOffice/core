/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_FMURL_HXX
#define _SVX_FMURL_HXX

#define FMURL_FORMSLOTS_PREFIX             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/" ) )
#define FMURL_FORM_POSITION                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/positionForm" ) )
#define FMURL_FORM_RECORDCOUNT             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/RecordCount" ) )
#define FMURL_RECORD_MOVEFIRST             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/moveToFirst" ) )
#define FMURL_RECORD_MOVEPREV              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/moveToPrev" ) )
#define FMURL_RECORD_MOVENEXT              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/moveToNext" ) )
#define FMURL_RECORD_MOVELAST              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/moveToLast" ) )
#define FMURL_RECORD_MOVETONEW             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/moveToNew" ) )
#define FMURL_RECORD_UNDO                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/undoRecord" ) )
#define FMURL_RECORD_SAVE                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/saveRecord" ) )
#define FMURL_RECORD_DELETE                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/deleteRecord" ) )
#define FMURL_FORM_REFRESH                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/refreshForm" ) )
#define FMURL_FORM_REFRESH_CURRENT_CONTROL rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/refreshCurrentControl" ) )
#define FMURL_FORM_SORT_UP                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/sortUp" ) )
#define FMURL_FORM_SORT_DOWN               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/sortDown" ) )
#define FMURL_FORM_SORT                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/sort" ) )
#define FMURL_FORM_AUTO_FILTER             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/autoFilter" ) )
#define FMURL_FORM_FILTER                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/filter" ) )
#define FMURL_FORM_APPLY_FILTER            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/applyFilter" ) )
#define FMURL_FORM_REMOVE_FILTER           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormController/removeFilterOrder" ) )
#define FMURL_CONFIRM_DELETION             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/ConfirmDeletion" ) )
#define FMURL_COMPONENT_FORMGRIDVIEW       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".component:DB/FormGridView" ) )
#define FMURL_GRIDVIEW_CLEARVIEW           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/ClearView" ) )
#define FMURL_GRIDVIEW_ADDCOLUMN           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/AddGridColumn" ) )
#define FMURL_GRIDVIEW_ATTACHTOFORM        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/AttachToForm" ) )
#define FMARG_ATTACHTO_MASTERFORM          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MasterForm" ) )
#define FMARG_ADDCOL_COLUMNTYPE            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnType" ) )
#define FMARG_ADDCOL_COLUMNPOS             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnPosition" ) )

#endif // _SVX_FMURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
