/*************************************************************************
 *
 *  $RCSfile: TypeInfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-02 15:42:27 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_TYPEINFO_HXX
#define DBAUI_TYPEINFO_HXX


#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNSEARCH_HPP_
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _MAP_
#include <map>
#endif


namespace dbaui
{
//========================================================================
// Anhand dieser Ids werden die sprachabhaengigen ::rtl::OUString aus der Resource geholt
const sal_uInt16 TYPE_UNKNOWN   = 0;
const sal_uInt16 TYPE_TEXT      = 1;
const sal_uInt16 TYPE_NUMERIC   = 2;
const sal_uInt16 TYPE_DATETIME  = 3;
const sal_uInt16 TYPE_DATE      = 4;
const sal_uInt16 TYPE_TIME      = 5;
const sal_uInt16 TYPE_BOOL      = 6;
const sal_uInt16 TYPE_CURRENCY  = 7;
const sal_uInt16 TYPE_MEMO      = 8;
const sal_uInt16 TYPE_COUNTER   = 9;
const sal_uInt16 TYPE_IMAGE     = 10;
const sal_uInt16 TYPE_CHAR      = 11;
const sal_uInt16 TYPE_DECIMAL   = 12;
const sal_uInt16 TYPE_BINARY    = 13;
const sal_uInt16 TYPE_VARBINARY = 14;
const sal_uInt16 TYPE_BIGINT    = 15;
const sal_uInt16 TYPE_DOUBLE    = 16;
const sal_uInt16 TYPE_FLOAT     = 17;
const sal_uInt16 TYPE_REAL      = 18;
const sal_uInt16 TYPE_INTEGER   = 19;
const sal_uInt16 TYPE_SMALLINT  = 20;
const sal_uInt16 TYPE_TINYINT   = 21;
const sal_uInt16 TYPE_SQLNULL   = 22;
const sal_uInt16 TYPE_OBJECT    = 23;
const sal_uInt16 TYPE_DISTINCT  = 24;
const sal_uInt16 TYPE_STRUCT    = 25;
const sal_uInt16 TYPE_ARRAY     = 26;
const sal_uInt16 TYPE_BLOB      = 27;
const sal_uInt16 TYPE_CLOB      = 28;
const sal_uInt16 TYPE_REF       = 29;
const sal_uInt16 TYPE_OTHER     = 30;

    class OTypeInfo
    {
    public:
        ::rtl::OUString aUIName;        // the name which is the user see (a combination of resource text and aTypeName)
        ::rtl::OUString aTypeName;      // Name des Types in der Datenbank
        ::rtl::OUString aLiteralPrefix; // Prefix zum Quoten
        ::rtl::OUString aLiteralSuffix; // Suffix zum Quoten
        ::rtl::OUString aCreateParams;  // Parameter zum Erstellen
        ::rtl::OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Laenge des Types
        sal_Int32       nNumPrecRadix;  // indicating the radix, which is usually 2 or 10

        sal_Int16       nMaximumScale;  // Nachkommastellen
        sal_Int16       nMinimumScale;  // Min Nachkommastellen

        sal_Int16       nType;          // Datenbanktyp
        sal_Int16       nSearchType;    // kann nach dem Typen gesucht werden


        sal_Bool        bCurrency       : 1,    // Waehrung
                        bAutoIncrement  : 1,    // Ist es ein automatisch incrementierendes Feld
                        bNullable       : 1,    // Kann das Feld NULL annehmen
                        bCaseSensitive  : 1,    // Ist der Type Casesensitive
                        bUnsigned       : 1,    // Ist der Type Unsigned
                        bEmpty_1        : 1,    // for later use
                        bEmpty_2        : 1;

        OTypeInfo()
                :bCurrency(sal_False)
                ,bAutoIncrement(sal_False)
                ,bNullable(sal_True)
                ,bCaseSensitive(sal_False)
                ,bUnsigned(sal_False)
                ,nMaximumScale(0)
                ,nMinimumScale(0)
                ,nType(::com::sun::star::sdbc::DataType::OTHER)
                ,nPrecision(0)
                ,nSearchType(::com::sun::star::sdbc::ColumnSearch::FULL)
        {}
        sal_Bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        sal_Bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }
    };


    typedef ::std::multimap<sal_Int32,OTypeInfo*> OTypeInfoMap;
}

#endif // DBAUI_TYPEINFO_HXX


