/*************************************************************************
 *
 *  $RCSfile: xmlstrings.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:20:14 $
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
#ifndef CONFIGMGR_XML_STRINGS_HXX_
#define CONFIGMGR_XML_STRINGS_HXX_

#ifndef _CONFIGMGR_STRINGS_HXX_
#include "strings.hxx"
#endif

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
        DECLARE_CONSTASCII_USTRING(OPERATION_REMOVE);

        // the default separator for strings
        DECLARE_CONSTASCII_USTRING(SEPARATOR_WHITESPACE);

        // Needed for building attribute lists
        DECLARE_CONSTASCII_USTRING(XML_ATTRTYPE_CDATA);

    } // namespace xml

} // namespace configmgr
#endif

