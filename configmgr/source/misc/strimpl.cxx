/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strimpl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:13:10 $
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

#ifndef _CONFIGMGR_STRDECL_HXX_
#include "strdecl.hxx"
#endif

namespace configmgr
{
    // simple types names
    IMPLEMENT_CONSTASCII_USTRING(TYPE_BOOLEAN,  "boolean");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_SHORT,    "short");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_INT,      "int");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_LONG,     "long");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_DOUBLE,   "double");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_STRING,   "string");
    // Type: Sequence<bytes>
    IMPLEMENT_CONSTASCII_USTRING(TYPE_BINARY,   "binary");
    // Universal type: Any
    IMPLEMENT_CONSTASCII_USTRING(TYPE_ANY,      "any");

    // special template names for native/localized value types
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_PREFIX,     "cfg:");
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_VALUE,      "cfg:value");
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_LOCALIZED_VALUE,   "cfg:localized");

    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_LIST_SUFFIX, "-list");



// emacs:
// create the declare from the implement
// (fset 'create-declare-from-implement
//   [home M-right ?\C-  ?\C-s ?, left right left ?\M-w f12 return up tab ?D ?E ?C ?L ?A ?R ?E ?\C-y ?) ?; home down f12 home down])

} // namespace configmgr
