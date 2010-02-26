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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "strdecl.hxx"

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
