/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmurl.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "fmurl.hxx"


namespace svxform
{

    IMPLEMENT_CONSTASCII_USTRING(FMURL_FORMSLOTS_PREFIX,   ".uno:FormController/");

    IMPLEMENT_CONSTASCII_USTRING(FMURL_FORM_POSITION,      ".uno:FormController/positionForm"  );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_FORM_RECORDCOUNT,   ".uno:FormController/RecordCount"  );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_MOVEFIRST,   ".uno:FormController/moveToFirst"  );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_MOVEPREV,    ".uno:FormController/moveToPrev"   );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_MOVENEXT,    ".uno:FormController/moveToNext"   );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_MOVELAST,    ".uno:FormController/moveToLast"   );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_MOVETONEW,   ".uno:FormController/moveToNew"    );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_UNDO,        ".uno:FormController/undoRecord"   );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_SAVE,        ".uno:FormController/saveRecord"   );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_RECORD_DELETE,      ".uno:FormController/deleteRecord" );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_FORM_REFRESH,       ".uno:FormController/refreshForm"  );
    IMPLEMENT_CONSTASCII_USTRING(FMURL_FORM_REFRESH_CURRENT_CONTROL,
                                                            ".uno:FormController/refreshCurrentControl"  );

    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_SORT_UP,      ".uno:FormController/sortUp"       );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_SORT_DOWN,    ".uno:FormController/sortDown"     );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_SORT,         ".uno:FormController/sort"         );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_AUTO_FILTER,  ".uno:FormController/autoFilter"   );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_FILTER,       ".uno:FormController/filter"       );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_APPLY_FILTER, ".uno:FormController/applyFilter"  );
    IMPLEMENT_CONSTASCII_USTRING( FMURL_FORM_REMOVE_FILTER,".uno:FormController/removeFilterOrder" );

    IMPLEMENT_CONSTASCII_USTRING(FMURL_CONFIRM_DELETION,".uno:FormSlots/ConfirmDeletion");

    IMPLEMENT_CONSTASCII_USTRING(FMURL_COMPONENT_FORMGRIDVIEW,".component:DB/FormGridView");
    IMPLEMENT_CONSTASCII_USTRING(FMURL_GRIDVIEW_CLEARVIEW,".uno:FormSlots/ClearView");
    IMPLEMENT_CONSTASCII_USTRING(FMURL_GRIDVIEW_ADDCOLUMN,".uno:FormSlots/AddGridColumn");
    IMPLEMENT_CONSTASCII_USTRING(FMURL_GRIDVIEW_ATTACHTOFORM,".uno:FormSlots/AttachToForm");

    IMPLEMENT_CONSTASCII_USTRING(FMARG_ATTACHTO_MASTERFORM,"MasterForm");
    IMPLEMENT_CONSTASCII_USTRING(FMARG_ADDCOL_COLUMNTYPE,"ColumnType");
    IMPLEMENT_CONSTASCII_USTRING(FMARG_ADDCOL_COLUMNPOS,"ColumnPosition");

}   // namespace svxform
