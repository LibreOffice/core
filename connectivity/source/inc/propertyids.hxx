/*************************************************************************
 *
 *  $RCSfile: propertyids.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-31 11:04:19 $
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

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#ifndef CONNECTIVITY_PROPERTY_NAME_SPACE
#pragma warning("CONNECTIVITY_PROPERTY_NAME_SPACE not set")
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

namespace connectivity
{
namespace dbtools
{
    extern const sal_Char* getPROPERTY_QUERYTIMEOUT();
    extern const sal_Char* getPROPERTY_MAXFIELDSIZE();
    extern const sal_Char* getPROPERTY_MAXROWS();
    extern const sal_Char* getPROPERTY_CURSORNAME();
    extern const sal_Char* getPROPERTY_RESULTSETCONCURRENCY();
    extern const sal_Char* getPROPERTY_RESULTSETTYPE();
    extern const sal_Char* getPROPERTY_FETCHDIRECTION();
    extern const sal_Char* getPROPERTY_FETCHSIZE();
    extern const sal_Char* getPROPERTY_ESCAPEPROCESSING();
    extern const sal_Char* getPROPERTY_USEBOOKMARKS();

    extern const sal_Char* getPROPERTY_NAME();
    extern const sal_Char* getPROPERTY_TYPE();
    extern const sal_Char* getPROPERTY_TYPENAME();
    extern const sal_Char* getPROPERTY_PRECISION();
    extern const sal_Char* getPROPERTY_SCALE();
    extern const sal_Char* getPROPERTY_ISNULLABLE();
    extern const sal_Char* getPROPERTY_ISAUTOINCREMENT();
    extern const sal_Char* getPROPERTY_ISROWVERSION();
    extern const sal_Char* getPROPERTY_DESCRIPTION();
    extern const sal_Char* getPROPERTY_DEFAULTVALUE();

    extern const sal_Char* getPROPERTY_REFERENCEDTABLE();
    extern const sal_Char* getPROPERTY_UPDATERULE();
    extern const sal_Char* getPROPERTY_DELETERULE();
    extern const sal_Char* getPROPERTY_CATALOG();
    extern const sal_Char* getPROPERTY_ISUNIQUE();
    extern const sal_Char* getPROPERTY_ISPRIMARYKEYINDEX();
    extern const sal_Char* getPROPERTY_ISCLUSTERED();
    extern const sal_Char* getPROPERTY_ISASCENDING();
    extern const sal_Char* getPROPERTY_SCHEMANAME();
    extern const sal_Char* getPROPERTY_CATALOGNAME();
    extern const sal_Char* getPROPERTY_COMMAND();
    extern const sal_Char* getPROPERTY_CHECKOPTION();
    extern const sal_Char* getPROPERTY_PASSWORD();
    extern const sal_Char* getPROPERTY_REFERENCEDCOLUMN();

    extern const sal_Char* getSTAT_INVALID_INDEX();

    extern const sal_Char* getPROPERTY_FUNCTION();
    extern const sal_Char* getPROPERTY_TABLENAME();
    extern const sal_Char* getPROPERTY_REALNAME();
    extern const sal_Char* getPROPERTY_DBASEPRECISIONCHANGED();
    extern const sal_Char* getPROPERTY_ISCURRENCY();

    extern const sal_Char* getPROPERTY_ISBOOKMARKABLE();
//  ====================================================
//  error messages
//  ====================================================
    extern const sal_Char* getERRORMSG_SEQUENCE();
    extern const sal_Char* getSQLSTATE_SEQUENCE();
}
}

namespace connectivity
{
    namespace CONNECTIVITY_PROPERTY_NAME_SPACE
    {
        typedef const sal_Char* (*PVFN)();

        struct UStringDescription
        {
            const sal_Char* pZeroTerminatedName;
            sal_Int32 nLength;

            UStringDescription(PVFN _fCharFkt);
            operator ::rtl::OUString() const { return ::rtl::OUString(pZeroTerminatedName,nLength,RTL_TEXTENCODING_ASCII_US); }
            ~UStringDescription();
        private:
            UStringDescription();
        };

#define DECLARE_CONSTASCII_USTRING(name,nsp)    \
    extern connectivity::nsp::UStringDescription name;

    DECLARE_CONSTASCII_USTRING(PROPERTY_CURSORNAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_RESULTSETCONCURRENCY,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_RESULTSETTYPE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_FETCHDIRECTION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_FETCHSIZE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_QUERYTIMEOUT,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_MAXFIELDSIZE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_MAXROWS,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ESCAPEPROCESSING,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_USEBOOKMARKS,CONNECTIVITY_PROPERTY_NAME_SPACE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_NAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TYPE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TYPENAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_PRECISION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_SCALE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISNULLABLE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISAUTOINCREMENT,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISROWVERSION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DESCRIPTION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DEFAULTVALUE,CONNECTIVITY_PROPERTY_NAME_SPACE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_REFERENCEDTABLE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_UPDATERULE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DELETERULE,CONNECTIVITY_PROPERTY_NAME_SPACE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_CATALOG,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISUNIQUE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISPRIMARYKEYINDEX,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISCLUSTERED,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISASCENDING,CONNECTIVITY_PROPERTY_NAME_SPACE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_SCHEMANAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_CATALOGNAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_COMMAND,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_CHECKOPTION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_PASSWORD,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_REFERENCEDCOLUMN,CONNECTIVITY_PROPERTY_NAME_SPACE)

    DECLARE_CONSTASCII_USTRING(PROPERTY_FUNCTION,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_TABLENAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_REALNAME,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_DBASEPRECISIONCHANGED,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISCURRENCY,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(PROPERTY_ISBOOKMARKABLE,CONNECTIVITY_PROPERTY_NAME_SPACE)

    // error msg
    DECLARE_CONSTASCII_USTRING(STAT_INVALID_INDEX,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(ERRORMSG_SEQUENCE,CONNECTIVITY_PROPERTY_NAME_SPACE)
    DECLARE_CONSTASCII_USTRING(SQLSTATE_SEQUENCE,CONNECTIVITY_PROPERTY_NAME_SPACE)

    }
}


//------------------------------------------------------------------------------
#define DECL_PROP1IMPL(varname, type) \
pProperties[nPos++] = ::com::sun::star::beans::Property(connectivity::CONNECTIVITY_PROPERTY_NAME_SPACE::PROPERTY_##varname, PROPERTY_ID_##varname, ::getCppuType(reinterpret_cast< type*>(NULL)),
//------------------------------------------------------------------------------
#define DECL_PROP0(varname, type)   \
    DECL_PROP1IMPL(varname, type) 0)
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP1IMPL(varname) \
        pProperties[nPos++] = ::com::sun::star::beans::Property(connectivity::CONNECTIVITY_PROPERTY_NAME_SPACE::PROPERTY_##varname, PROPERTY_ID_##varname, ::getBooleanCppuType(),
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


