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

#include "xmlstrings.hxx"

//.........................................................................
namespace configmgr
{
//.........................................................................

    namespace xml
    {
//----------------------------------------------------------------------------
// For now: Include the fixed OOR prefix into (most) attribute names
#define OOR_PREFIX_ "oor:"
// ... but not into (most) tag names
#define OOR_TAG_PREFIX_
// ... but into root tag names
#define OOR_ROOTTAG_PREFIX_ OOR_PREFIX_
//----------------------------------------------------------------------------
    // extern declaration for strings used in the XML format
        // namespace prefixes
        IMPLEMENT_CONSTASCII_USTRING(NS_PREFIX_OOR, "oor");
        IMPLEMENT_CONSTASCII_USTRING(NS_PREFIX_XS,  "xs");

        // namespace urls
        IMPLEMENT_CONSTASCII_USTRING(NS_URI_OOR,"http://openoffice.org/2001/registry");
        IMPLEMENT_CONSTASCII_USTRING(NS_URI_XS, "http://www.w3.org/2001/XMLSchema");

        // tag names
        IMPLEMENT_CONSTASCII_USTRING(TAG_SCHEMA,    OOR_ROOTTAG_PREFIX_"component-schema");
        IMPLEMENT_CONSTASCII_USTRING(TAG_LAYER,     OOR_ROOTTAG_PREFIX_"component-data");
        IMPLEMENT_CONSTASCII_USTRING(DEPRECATED_TAG_LAYER, OOR_ROOTTAG_PREFIX_"node");

        IMPLEMENT_CONSTASCII_USTRING(TAG_COMPONENT, OOR_TAG_PREFIX_"component");
        IMPLEMENT_CONSTASCII_USTRING(TAG_TEMPLATES, OOR_TAG_PREFIX_"templates");

        IMPLEMENT_CONSTASCII_USTRING(TAG_NODE,      OOR_TAG_PREFIX_"node");
        IMPLEMENT_CONSTASCII_USTRING(TAG_GROUP,     OOR_TAG_PREFIX_"group");
        IMPLEMENT_CONSTASCII_USTRING(TAG_SET,       OOR_TAG_PREFIX_"set");
        IMPLEMENT_CONSTASCII_USTRING(TAG_PROP,      OOR_TAG_PREFIX_"prop");

        IMPLEMENT_CONSTASCII_USTRING(TAG_VALUE,     OOR_TAG_PREFIX_"value");
        IMPLEMENT_CONSTASCII_USTRING(TAG_IMPORT,    OOR_TAG_PREFIX_"import");
        IMPLEMENT_CONSTASCII_USTRING(TAG_INSTANCE,  OOR_TAG_PREFIX_"node-ref");
        IMPLEMENT_CONSTASCII_USTRING(TAG_ITEMTYPE,  OOR_TAG_PREFIX_"item");
        IMPLEMENT_CONSTASCII_USTRING(TAG_USES,      OOR_TAG_PREFIX_"uses");

        // attribute names
        IMPLEMENT_CONSTASCII_USTRING(ATTR_NAME,     OOR_PREFIX_"name");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_CONTEXT,  OOR_PREFIX_"context");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_PACKAGE,  OOR_PREFIX_"package");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_COMPONENT,OOR_PREFIX_"component");

        IMPLEMENT_CONSTASCII_USTRING(ATTR_ITEMTYPE,         OOR_PREFIX_"node-type");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_ITEMTYPECOMPONENT,OOR_PREFIX_"component");

        IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUETYPE,        OOR_PREFIX_"type");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUESEPARATOR,   OOR_PREFIX_"separator");

        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_EXTENSIBLE,  OOR_PREFIX_"extensible");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_FINALIZED,   OOR_PREFIX_"finalized");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_READONLY,    OOR_PREFIX_"readonly");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_MANDATORY,   OOR_PREFIX_"mandatory");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_NULLABLE,    OOR_PREFIX_"nillable");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_FLAG_LOCALIZED,   OOR_PREFIX_"localized");

        IMPLEMENT_CONSTASCII_USTRING(ATTR_OPERATION,        OOR_PREFIX_"op");

    // attributes defined elsewhere
        IMPLEMENT_CONSTASCII_USTRING(EXT_ATTR_LANGUAGE,     "xml:lang");
        IMPLEMENT_CONSTASCII_USTRING(EXT_ATTR_NULL,         "xsi:nil");

    // attribute contents
        // boolean constants
        IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUE_TRUE,       "true");
        IMPLEMENT_CONSTASCII_USTRING(ATTR_VALUE_FALSE,      "false");

        // simple types names
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_BOOLEAN,     "boolean");
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_SHORT,       "short");
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_INT,         "int");
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_LONG,        "long");
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_DOUBLE,      "double");
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_STRING,      "string");
        // Type: Sequence<bytes>
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_BINARY,      "hexBinary");
        // Universal type: Any
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_ANY,         "any");

        // modifier suffix for list types
        IMPLEMENT_CONSTASCII_USTRING(VALUETYPE_LIST_SUFFIX, "-list");

        // States for update actions
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_MODIFY,      "modify");
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_REPLACE,     "replace");
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_FUSE,        "fuse");
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_REMOVE,      "remove");

        // the default separator for strings
        IMPLEMENT_CONSTASCII_USTRING(SEPARATOR_WHITESPACE,  " ");

        // Needed for building attribute lists
        IMPLEMENT_CONSTASCII_USTRING(XML_ATTRTYPE_CDATA,    "CDATA");

    } // namespace xml

} // namespace configmgr


