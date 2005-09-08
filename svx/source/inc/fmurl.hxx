/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmurl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:21:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_FMURL_HXX
#define _SVX_FMURL_HXX

#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif

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

