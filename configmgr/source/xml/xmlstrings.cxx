/*************************************************************************
 *
 *  $RCSfile: xmlstrings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-27 10:38:31 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "xmlstrings.hxx"

//.........................................................................
namespace configmgr
{
//.........................................................................

    namespace xml
    {
//----------------------------------------------------------------------------
// For now: Include the fixed OOR prefix into (most) tag and attribute names
#define OOR_PREFIX_ "oor:"
//----------------------------------------------------------------------------
    // extern declaration for strings used in the XML format
        // namespace prefixes
        IMPLEMENT_CONSTASCII_USTRING(NS_PREFIX_OOR, "oor");
        IMPLEMENT_CONSTASCII_USTRING(NS_PREFIX_XS,  "xs");

        // namespace urls
        IMPLEMENT_CONSTASCII_USTRING(NS_URI_OOR,"http://openoffice.org/2001/registry");
        IMPLEMENT_CONSTASCII_USTRING(NS_URI_XS, "http://www.w3.org/2001/XMLSchema");

        // tag names
        IMPLEMENT_CONSTASCII_USTRING(TAG_SCHEMA,    OOR_PREFIX_"component-schema");

        IMPLEMENT_CONSTASCII_USTRING(TAG_COMPONENT, OOR_PREFIX_"component");
        IMPLEMENT_CONSTASCII_USTRING(TAG_TEMPLATES, OOR_PREFIX_"templates");

        IMPLEMENT_CONSTASCII_USTRING(TAG_NODE,      OOR_PREFIX_"node");
        IMPLEMENT_CONSTASCII_USTRING(TAG_GROUP,     OOR_PREFIX_"group");
        IMPLEMENT_CONSTASCII_USTRING(TAG_SET,       OOR_PREFIX_"set");
        IMPLEMENT_CONSTASCII_USTRING(TAG_PROP,      OOR_PREFIX_"prop");

        IMPLEMENT_CONSTASCII_USTRING(TAG_VALUE,     OOR_PREFIX_"value");
        IMPLEMENT_CONSTASCII_USTRING(TAG_IMPORT,    OOR_PREFIX_"import");
        IMPLEMENT_CONSTASCII_USTRING(TAG_INSTANCE,  OOR_PREFIX_"node-ref");
        IMPLEMENT_CONSTASCII_USTRING(TAG_ITEMTYPE,  OOR_PREFIX_"item");

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
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_MODIFY,      OOR_PREFIX_"modify");
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_REPLACE,     OOR_PREFIX_"replace");
        IMPLEMENT_CONSTASCII_USTRING(OPERATION_REMOVE,      OOR_PREFIX_"remove");

        // the default separator for strings
        IMPLEMENT_CONSTASCII_USTRING(SEPARATOR_WHITESPACE,  " ");

        // Needed for building attribute lists
        IMPLEMENT_CONSTASCII_USTRING(XML_ATTRTYPE_CDATA,    "CDATA");

    } // namespace xml

} // namespace configmgr


