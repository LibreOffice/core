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
#ifndef CONFIGMGR_XML_STRINGS_HXX_
#define CONFIGMGR_XML_STRINGS_HXX_

#include "strings.hxx"

//.........................................................................
namespace configmgr
{
//.........................................................................

    namespace xml
    {
    // extern declaration for strings used in the XML format
        // namespace prefixes
        DECLARE_CONSTASCII_USTRING(NS_PREFIX_OOR);
        DECLARE_CONSTASCII_USTRING(NS_PREFIX_XS);

        const sal_Unicode k_NS_SEPARATOR(':');

        // namespace urls
        DECLARE_CONSTASCII_USTRING(NS_URI_OOR);
        DECLARE_CONSTASCII_USTRING(NS_URI_XS);
        DECLARE_CONSTASCII_USTRING(NS_URI_XSI);

        // tag names
        DECLARE_CONSTASCII_USTRING(TAG_SCHEMA);
        DECLARE_CONSTASCII_USTRING(TAG_LAYER);
        DECLARE_CONSTASCII_USTRING(DEPRECATED_TAG_LAYER);

        DECLARE_CONSTASCII_USTRING(TAG_COMPONENT);
        DECLARE_CONSTASCII_USTRING(TAG_TEMPLATES);

        DECLARE_CONSTASCII_USTRING(TAG_NODE);
        DECLARE_CONSTASCII_USTRING(TAG_GROUP);
        DECLARE_CONSTASCII_USTRING(TAG_SET);
        DECLARE_CONSTASCII_USTRING(TAG_PROP);

        DECLARE_CONSTASCII_USTRING(TAG_IMPORT);
        DECLARE_CONSTASCII_USTRING(TAG_INSTANCE);
        DECLARE_CONSTASCII_USTRING(TAG_ITEMTYPE);
        DECLARE_CONSTASCII_USTRING(TAG_VALUE);
        DECLARE_CONSTASCII_USTRING(TAG_USES);

        // attribute names
        DECLARE_CONSTASCII_USTRING(ATTR_NAME);
        DECLARE_CONSTASCII_USTRING(ATTR_CONTEXT);
        DECLARE_CONSTASCII_USTRING(ATTR_PACKAGE);
        DECLARE_CONSTASCII_USTRING(ATTR_COMPONENT);

        DECLARE_CONSTASCII_USTRING(ATTR_ITEMTYPE);
        DECLARE_CONSTASCII_USTRING(ATTR_ITEMTYPECOMPONENT);

        DECLARE_CONSTASCII_USTRING(ATTR_VALUETYPE);
        DECLARE_CONSTASCII_USTRING(ATTR_VALUESEPARATOR);

        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_EXTENSIBLE);
        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_FINALIZED);
        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_READONLY);
        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_MANDATORY);
        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_NULLABLE);
        DECLARE_CONSTASCII_USTRING(ATTR_FLAG_LOCALIZED);

        DECLARE_CONSTASCII_USTRING(ATTR_OPERATION);

    // attributes defined elsewhere
        DECLARE_CONSTASCII_USTRING(EXT_ATTR_LANGUAGE);
        DECLARE_CONSTASCII_USTRING(EXT_ATTR_NULL);

    // attribute contents
        // boolean constants
        DECLARE_CONSTASCII_USTRING(ATTR_VALUE_TRUE);
        DECLARE_CONSTASCII_USTRING(ATTR_VALUE_FALSE);

        // simple types names
        DECLARE_CONSTASCII_USTRING(VALUETYPE_BOOLEAN);
        DECLARE_CONSTASCII_USTRING(VALUETYPE_SHORT);
        DECLARE_CONSTASCII_USTRING(VALUETYPE_INT);
        DECLARE_CONSTASCII_USTRING(VALUETYPE_LONG);
        DECLARE_CONSTASCII_USTRING(VALUETYPE_DOUBLE);
        DECLARE_CONSTASCII_USTRING(VALUETYPE_STRING);
        // Type: Sequence<bytes>
        DECLARE_CONSTASCII_USTRING(VALUETYPE_BINARY);
        // Universal type: Any
        DECLARE_CONSTASCII_USTRING(VALUETYPE_ANY);

        // modifier suffix for list types
        DECLARE_CONSTASCII_USTRING(VALUETYPE_LIST_SUFFIX);

        // States for update actions
        DECLARE_CONSTASCII_USTRING(OPERATION_MODIFY);
        DECLARE_CONSTASCII_USTRING(OPERATION_REPLACE);
        DECLARE_CONSTASCII_USTRING(OPERATION_FUSE);
        DECLARE_CONSTASCII_USTRING(OPERATION_REMOVE);

        // the default separator for strings
        DECLARE_CONSTASCII_USTRING(SEPARATOR_WHITESPACE);

        // Needed for building attribute lists
        DECLARE_CONSTASCII_USTRING(XML_ATTRTYPE_CDATA);

    } // namespace xml

} // namespace configmgr
#endif

