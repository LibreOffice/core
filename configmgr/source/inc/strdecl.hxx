/*************************************************************************
 *
 *  $RCSfile: strdecl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-17 08:34:06 $
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
#ifndef _CONFIGMGR_STRDECL_HXX_
#define _CONFIGMGR_STRDECL_HXX_

#ifndef _CONFIGMGR_STRINGS_HXX_
#include "strings.hxx"
#endif

//.........................................................................
namespace configmgr
{
//.........................................................................

// extern declaration for predefined strings, uncompleted
    DECLARE_CONSTASCII_USTRING(TAG_VALUE);
    DECLARE_CONSTASCII_USTRING(TAG_USERVALUE);
    DECLARE_CONSTASCII_USTRING(TAG_VALUE_ITEM);
    DECLARE_CONSTASCII_USTRING(TAG_DEFAULT_ITEM);
    DECLARE_CONSTASCII_USTRING(TAG_NODEFAULT);
    DECLARE_CONSTASCII_USTRING(TAG_DEFAULT);
    DECLARE_CONSTASCII_USTRING(TAG_DEFAULTVALUE);
    DECLARE_CONSTASCII_USTRING(TAG_DATA);

    DECLARE_CONSTASCII_USTRING(TYPE_BOOLEAN);
    DECLARE_CONSTASCII_USTRING(TYPE_SHORT);
    DECLARE_CONSTASCII_USTRING(TYPE_INT);
    DECLARE_CONSTASCII_USTRING(TYPE_LONG);
    DECLARE_CONSTASCII_USTRING(TYPE_DOUBLE);
    DECLARE_CONSTASCII_USTRING(TYPE_STRING);
    DECLARE_CONSTASCII_USTRING(TYPE_BINARY);
    DECLARE_CONSTASCII_USTRING(TYPE_ANY);
    DECLARE_CONSTASCII_USTRING(TYPE_SET);
    DECLARE_CONSTASCII_USTRING(TYPE_GROUP);

    DECLARE_CONSTASCII_USTRING(ATTR_ENCODING_HEX);
    DECLARE_CONSTASCII_USTRING(ATTR_NAME);
    DECLARE_CONSTASCII_USTRING(ATTR_TYPE);
    DECLARE_CONSTASCII_USTRING(ATTR_GROUP);
    DECLARE_CONSTASCII_USTRING(ATTR_DERIVED);
    DECLARE_CONSTASCII_USTRING(ATTR_INSTANCE);
    DECLARE_CONSTASCII_USTRING(ATTR_ENCODING);
    DECLARE_CONSTASCII_USTRING(ATTR_MODE);
    DECLARE_CONSTASCII_USTRING(ATTR_PATH);
    DECLARE_CONSTASCII_USTRING(ATTR_DERIVED_LIST);
    DECLARE_CONSTASCII_USTRING(ATTR_VALUE_ADD);
    DECLARE_CONSTASCII_USTRING(ATTR_VALUE_REMOVE);
    DECLARE_CONSTASCII_USTRING(ATTR_LOCALIZE);
    DECLARE_CONSTASCII_USTRING(ATTR_LANG);
    DECLARE_CONSTASCII_USTRING(ATTR_SEPARATOR);
    DECLARE_CONSTASCII_USTRING(ATTR_NULL);
    DECLARE_CONSTASCII_USTRING(ATTR_NULLABLE);
    DECLARE_CONSTASCII_USTRING(ATTR_STATE);
    DECLARE_CONSTASCII_USTRING(ATTR_WRITABLE);

    DECLARE_CONSTASCII_USTRING(ATTR_VALUE_TRUE);
    DECLARE_CONSTASCII_USTRING(ATTR_VALUE_FALSE);

    DECLARE_CONSTASCII_USTRING(PARAM_OBJECT);
    DECLARE_CONSTASCII_USTRING(PARAM_NAME);
    DECLARE_CONSTASCII_USTRING(PARAM_ISNEWOBJECT);

    DECLARE_CONSTASCII_USTRING(STATE_MODIFIED);
    DECLARE_CONSTASCII_USTRING(STATE_REPLACED);
    DECLARE_CONSTASCII_USTRING(STATE_DELETED);


    DECLARE_CONSTASCII_USTRING(XML_CDATA);
} // namespace configmgr
#endif

