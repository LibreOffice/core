/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmurl.hxx,v $
 * $Revision: 1.6 $
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

#include "fmstatic.hxx"

namespace svxform
{

    DECLARE_CONSTASCII_USTRING(FMURL_FORMSLOTS_PREFIX);

    DECLARE_CONSTASCII_USTRING(FMURL_FORM_POSITION);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_RECORDCOUNT);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_MOVEFIRST);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_MOVEPREV);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_MOVENEXT);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_MOVELAST);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_MOVETONEW);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_UNDO);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_SAVE);
    DECLARE_CONSTASCII_USTRING(FMURL_RECORD_DELETE);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_REFRESH);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_REFRESH_CURRENT_CONTROL);

    DECLARE_CONSTASCII_USTRING(FMURL_FORM_SORT_UP);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_SORT_DOWN);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_SORT);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_AUTO_FILTER);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_FILTER);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_APPLY_FILTER);
    DECLARE_CONSTASCII_USTRING(FMURL_FORM_REMOVE_FILTER);

    DECLARE_CONSTASCII_USTRING(FMURL_CONFIRM_DELETION);

    DECLARE_CONSTASCII_USTRING(FMURL_COMPONENT_FORMGRIDVIEW);
    DECLARE_CONSTASCII_USTRING(FMURL_GRIDVIEW_CLEARVIEW);
    DECLARE_CONSTASCII_USTRING(FMURL_GRIDVIEW_ADDCOLUMN);
    DECLARE_CONSTASCII_USTRING(FMURL_GRIDVIEW_ATTACHTOFORM);

    DECLARE_CONSTASCII_USTRING(FMARG_ATTACHTO_MASTERFORM);
    DECLARE_CONSTASCII_USTRING(FMARG_ADDCOL_COLUMNTYPE);
    DECLARE_CONSTASCII_USTRING(FMARG_ADDCOL_COLUMNPOS);

}   // namespace svxform

#endif // _SVX_FMURL_HXX

