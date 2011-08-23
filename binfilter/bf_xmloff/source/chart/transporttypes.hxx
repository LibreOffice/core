/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SCH_XML_TRANSPORTTYPES_HXX_
#define SCH_XML_TRANSPORTTYPES_HXX_

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
namespace binfilter {

enum SchXMLCellType
{
    SCH_CELL_TYPE_UNKNOWN,
    SCH_CELL_TYPE_FLOAT,
    SCH_CELL_TYPE_STRING
};

struct SchXMLCell
{
    ::rtl::OUString aString;
    double fValue;
    SchXMLCellType eType;

    SchXMLCell() : fValue( 0.0 ), eType( SCH_CELL_TYPE_UNKNOWN ) {}
};

struct SchXMLTable
{
    std::vector< ::std::vector< SchXMLCell > > aData;		/// an array of rows containing the table contents

    sal_Int32 nRowIndex;								/// reflects the index of the row currently parsed
    sal_Int32 nColumnIndex;								/// reflects the index of the column currently parsed
    sal_Int32 nMaxColumnIndex;							/// the greatest number of columns detected

    sal_Int32 nNumberOfColsEstimate;					/// parsing column-elements may yield an estimate

    SchXMLTable() : nRowIndex( -1 ),
                    nColumnIndex( -1 ),
                    nMaxColumnIndex( -1 ),
                    nNumberOfColsEstimate( 0 ) {}
};

// ----------------------------------------

struct SchNumericCellRangeAddress
{
    sal_Int32 nRow1, nRow2;
    sal_Int32 nCol1, nCol2;

    SchNumericCellRangeAddress() :
            nRow1( -1 ), nRow2( -1 ),
            nCol1( -1 ), nCol2( -1 )
        {}

    SchNumericCellRangeAddress( const SchNumericCellRangeAddress& aOther )
        {
            nRow1 = aOther.nRow1; nRow2 = aOther.nRow2;
            nCol1 = aOther.nCol1; nCol2 = aOther.nCol2;
        }
};

// ----------------------------------------

enum SchXMLAxisClass
{
    SCH_XML_AXIS_CATEGORY,
    SCH_XML_AXIS_DOMAIN,
    SCH_XML_AXIS_VALUE,
    SCH_XML_AXIS_SERIES,
    SCH_XML_AXIS_UNDEF
};

struct SchXMLAxis
{
    enum SchXMLAxisClass eClass;
    sal_Int8 nIndexInCategory;
    ::rtl::OUString aName;
    ::rtl::OUString aTitle;
    ::com::sun::star::awt::Point aPosition;

    SchXMLAxis() : eClass( SCH_XML_AXIS_UNDEF ), nIndexInCategory( 0 ) {}
};

}//end of namespace binfilter
#endif	// SCH_XML_TRANSPORTTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
