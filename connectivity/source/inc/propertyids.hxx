/*************************************************************************
 *
 *  $RCSfile: propertyids.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:24 $
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
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#define _CONNECTIVITY_PROPERTYIDS_HXX_


namespace connectivity
{
    struct UStringDescription
    {
        const sal_Char* pZeroTerminatedName;

        UStringDescription(sal_Char* _pName) { pZeroTerminatedName = _pName; }
        operator ::rtl::OUString() const { return ::rtl::OUString::createFromAscii(pZeroTerminatedName); }
    private:
        UStringDescription();
    };

#define DECLARE_CONSTASCII_USTRING(name)    \
    extern connectivity::UStringDescription name;

#define IMPLEMENT_CONSTASCII_USTRING(name, asciivalue)  \
    connectivity::UStringDescription name(asciivalue)


    DECLARE_CONSTASCII_USTRING(PROPERTY_CURSORNAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_RESULTSETCONCURRENCY)
    DECLARE_CONSTASCII_USTRING(PROPERTY_RESULTSETTYPE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_FETCHDIRECTION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_FETCHSIZE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_QUERYTIMEOUT)
    DECLARE_CONSTASCII_USTRING(PROPERTY_MAXFIELDSIZE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_MAXROWS)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ESCAPEPROCESSING)
    DECLARE_CONSTASCII_USTRING(PROPERTY_USEBOOKMARKS)

    DECLARE_CONSTASCII_USTRING(PROPERTY_NAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TYPE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TYPENAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_PRECISION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_SCALE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISNULLABLE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISAUTOINCREMENT)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISROWVERSION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DESCRIPTION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DEFAULTVALUE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_REFERENCEDTABLE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_UPDATERULE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DELETERULE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_CATALOG)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISUNIQUE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISPRIMARYKEYINDEX)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISCLUSTERED)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISASCENDING)

    DECLARE_CONSTASCII_USTRING(PROPERTY_SCHEMANAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_CATALOGNAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_COMMAND)
    DECLARE_CONSTASCII_USTRING(PROPERTY_CHECKOPTION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_PASSWORD)
    DECLARE_CONSTASCII_USTRING(PROPERTY_REFERENCEDCOLUMN)

    DECLARE_CONSTASCII_USTRING(PROPERTY_FUNCTION)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TABLENAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_REALNAME)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DBASEPRECISIONCHANGED)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISCURRENCY)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISBOOKMARKABLE)

    // error msg
    DECLARE_CONSTASCII_USTRING(STAT_INVALID_INDEX)
}


//------------------------------------------------------------------------------
#define DECL_PROP1IMPL(varname, type) \
        pProperties[nPos++] = ::com::sun::star::beans::Property(connectivity::PROPERTY_##varname, PROPERTY_ID_##varname, ::getCppuType(reinterpret_cast< type*>(NULL)),
//------------------------------------------------------------------------------
#define DECL_PROP0(varname, type)   \
    DECL_PROP1IMPL(varname, type) 0)
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP1IMPL(varname) \
        pProperties[nPos++] = ::com::sun::star::beans::Property(connectivity::PROPERTY_##varname, PROPERTY_ID_##varname, ::getBooleanCppuType(),
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP0(varname)    \
    DECL_BOOL_PROP1IMPL(varname) 0)


#define PROPERTY_ID_QUERYTIMEOUT                    1
#define PROPERTY_ID_MAXFIELDSIZE                    2
#define PROPERTY_ID_MAXROWS                         3
#define PROPERTY_ID_CURSORNAME                      4
#define PROPERTY_ID_RESULTSETCONCURRENCY            5
#define PROPERTY_ID_RESULTSETTYPE                   6
#define PROPERTY_ID_FETCHDIRECTION                  7
#define PROPERTY_ID_FETCHSIZE                       8
#define PROPERTY_ID_ESCAPEPROCESSING                9
#define PROPERTY_ID_USEBOOKMARKS                    10
// Column
#define PROPERTY_ID_NAME                            11
#define PROPERTY_ID_TYPE                            12
#define PROPERTY_ID_TYPENAME                        13
#define PROPERTY_ID_PRECISION                       14
#define PROPERTY_ID_SCALE                           15
#define PROPERTY_ID_ISNULLABLE                      16
#define PROPERTY_ID_ISAUTOINCREMENT                 17
#define PROPERTY_ID_ISROWVERSION                    18
#define PROPERTY_ID_DESCRIPTION                     19
#define PROPERTY_ID_DEFAULTVALUE                    20

#define PROPERTY_ID_REFERENCEDTABLE                 21
#define PROPERTY_ID_UPDATERULE                      22
#define PROPERTY_ID_DELETERULE                      23
#define PROPERTY_ID_CATALOG                         24
#define PROPERTY_ID_ISUNIQUE                        25
#define PROPERTY_ID_ISPRIMARYKEYINDEX               26
#define PROPERTY_ID_ISCLUSTERED                     27
#define PROPERTY_ID_ISASCENDING                     28
#define PROPERTY_ID_SCHEMANAME                      29
#define PROPERTY_ID_CATALOGNAME                     30

#define PROPERTY_ID_COMMAND                         31
#define PROPERTY_ID_CHECKOPTION                     32
#define PROPERTY_ID_PASSWORD                        33
#define PROPERTY_ID_REFERENCEDCOLUMN                34

#define PROPERTY_ID_FUNCTION                        35
#define PROPERTY_ID_TABLENAME                       36
#define PROPERTY_ID_REALNAME                        37
#define PROPERTY_ID_DBASEPRECISIONCHANGED           38
#define PROPERTY_ID_ISCURRENCY                      39
#define PROPERTY_ID_ISBOOKMARKABLE                  40

#endif // _CONNECTIVITY_PROPERTYIDS_HXX_


