/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strdecl.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:57:19 $
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
#define _CONFIGMGR_STRDECL_HXX_

#ifndef _CONFIGMGR_STRINGS_HXX_
#include "strings.hxx"
#endif

//.........................................................................
namespace configmgr
{
//.........................................................................

// extern declaration for predefined strings

    // simple types names
    DECLARE_CONSTASCII_USTRING(TYPE_BOOLEAN);
    DECLARE_CONSTASCII_USTRING(TYPE_SHORT);
    DECLARE_CONSTASCII_USTRING(TYPE_INT);
    DECLARE_CONSTASCII_USTRING(TYPE_LONG);
    DECLARE_CONSTASCII_USTRING(TYPE_DOUBLE);
    DECLARE_CONSTASCII_USTRING(TYPE_STRING);
    // Type: Sequence<bytes>
    DECLARE_CONSTASCII_USTRING(TYPE_BINARY);
    // Universal type: Any
    DECLARE_CONSTASCII_USTRING(TYPE_ANY);

    // special template names for native/localized value types
    DECLARE_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_PREFIX);
    DECLARE_CONSTASCII_USTRING(TEMPLATE_MODULE_NATIVE_VALUE);
    DECLARE_CONSTASCII_USTRING(TEMPLATE_MODULE_LOCALIZED_VALUE);

    DECLARE_CONSTASCII_USTRING(TEMPLATE_LIST_SUFFIX);

} // namespace configmgr
#endif

