/*************************************************************************
 *
 *  $RCSfile: strimpl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-16 17:28:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONFIGMGR_STRINGS_HXX_
#include "strings.hxx"
#endif

namespace configmgr
{
    // tag names
    // <name>...</name>
    IMPLEMENT_CONSTASCII_USTRING(TAG_VALUE,     "cfg:value");

    // Attributes name="..."
    IMPLEMENT_CONSTASCII_USTRING(ATTR_NAME,         "cfg:name");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_TYPE,         "cfg:type");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_TYPE_MODIFIER,"cfg:derivedBy");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_INSTANCE,     "cfg:element-type");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_MODULE,       "cfg:component");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_ENCODING,     "cfg:encoding");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_SEPARATOR,    "cfg:separator");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_NULL,         "xsi:null");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_LANG,         "xml:lang");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_LOCALIZE,     "cfg:localized");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_NULLABLE,     "cfg:nullable");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_WRITABLE,     "cfg:writable");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_STATE,        "state");

    // boolean constants
    IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUE_TRUE,   "true");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUE_FALSE,  "false");

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
    // special non-value types
    IMPLEMENT_CONSTASCII_USTRING(TYPE_SET,      "set");
    IMPLEMENT_CONSTASCII_USTRING(TYPE_GROUP,    "group");

    // type modifier values
    IMPLEMENT_CONSTASCII_USTRING(TYPE_MODIFIER_LIST, "list");

    // States for update actions
    IMPLEMENT_CONSTASCII_USTRING(STATE_MODIFIED, "modified");
    IMPLEMENT_CONSTASCII_USTRING(STATE_REPLACED, "replaced");
    IMPLEMENT_CONSTASCII_USTRING(STATE_DELETED,  "deleted");
    IMPLEMENT_CONSTASCII_USTRING(STATE_DEFAULT,  "default");

    // the encoding for binary values
    IMPLEMENT_CONSTASCII_USTRING(ENCODING_HEX,      "hex");
    IMPLEMENT_CONSTASCII_USTRING(ENCODING_BASE64,   "base64");

    IMPLEMENT_CONSTASCII_USTRING(XML_CDATA, "CDATA");

    // special template names for native/localized value types
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_PREFIX,     "cfg:");
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_VALUE,      "cfg:value");
    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_MODULE_LOCALIZED_VALUE,   "cfg:localized");

    IMPLEMENT_CONSTASCII_USTRING(TEMPLATE_LIST_SUFFIX, "-list");

    // the default separator for strings
    IMPLEMENT_CONSTASCII_USTRING(DEFAULT_SEPARATOR,  " ");

    // im-/export specific
    IMPLEMENT_CONSTASCII_USTRING(TAG_DATA,  "data");
    IMPLEMENT_CONSTASCII_USTRING(ATTR_PATH, "path");

    // Parameter (?)
    IMPLEMENT_CONSTASCII_USTRING(PARAM_OBJECT, "Object");
    IMPLEMENT_CONSTASCII_USTRING(PARAM_NAME, "Name");
    IMPLEMENT_CONSTASCII_USTRING(PARAM_ISNEWOBJECT, "IsNewObject");

// emacs:
// create the declare from the implement
// (fset 'create-declare-from-implement
//   [home M-right ?\C-  ?\C-s ?, left right left ?\M-w f12 return up tab ?D ?E ?C ?L ?A ?R ?E ?\C-y ?) ?; home down f12 home down])

} // namespace configmgr
