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

#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <vector>
#include <map>

enum SchXMLCellType
{
    SCH_CELL_TYPE_UNKNOWN,
    SCH_CELL_TYPE_FLOAT,
    SCH_CELL_TYPE_STRING,
    SCH_CELL_TYPE_COMPLEX_STRING
};

struct SchXMLCell
{
    rtl::OUString aString;
    ::com::sun::star::uno::Sequence< rtl::OUString >* pComplexString;
    double fValue;
    SchXMLCellType eType;
    rtl::OUString aRangeId;

    SchXMLCell() : pComplexString(0), fValue( 0.0 ), eType( SCH_CELL_TYPE_UNKNOWN )
    {}

    SchXMLCell( const SchXMLCell& rOther )
        : aString( rOther.aString )
        , pComplexString( rOther.pComplexString ? new ::com::sun::star::uno::Sequence< rtl::OUString >( *rOther.pComplexString ) : 0 )
        , fValue( rOther.fValue )
        , eType( rOther.eType )
        , aRangeId( rOther.aRangeId )
    {}

    ~SchXMLCell()
    {
        if(pComplexString)
        {
            delete pComplexString;
            pComplexString=0;
        }
    }
};

struct SchXMLTable
{
    std::vector< std::vector< SchXMLCell > > aData;     /// an array of rows containing the table contents

    sal_Int32 nRowIndex;                                /// reflects the index of the row currently parsed
    sal_Int32 nColumnIndex;                             /// reflects the index of the column currently parsed
    sal_Int32 nMaxColumnIndex;                          /// the greatest number of columns detected

    sal_Int32 nNumberOfColsEstimate;                    /// parsing column-elements may yield an estimate

    bool bHasHeaderRow;
    bool bHasHeaderColumn;

    ::rtl::OUString aTableNameOfFile;                   /// the table name read at the table:table element

    ::std::vector< sal_Int32 > aHiddenColumns;

    bool bProtected;

    SchXMLTable() : nRowIndex( -1 ),
                    nColumnIndex( -1 ),
                    nMaxColumnIndex( -1 ),
                    nNumberOfColsEstimate( 0 ),
                    bHasHeaderRow( false ),
                    bHasHeaderColumn( false ),
                    bProtected( false )
    {}
};

typedef sal_Int32 tSchXMLIndex;
#define SCH_XML_CATEGORIES_INDEX (static_cast<tSchXMLIndex>(-1))
enum SchXMLLabeledSequencePart
{
    SCH_XML_PART_LABEL,
    SCH_XML_PART_VALUES,
    SCH_XML_PART_ERROR_BARS
};
typedef ::std::pair< tSchXMLIndex, SchXMLLabeledSequencePart > tSchXMLIndexWithPart;
typedef ::std::multimap< tSchXMLIndexWithPart,
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >
    tSchXMLLSequencesPerIndex;

bool operator < ( const tSchXMLIndexWithPart & rFirst, const tSchXMLIndexWithPart & rSecond );

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

enum SchXMLAxisDimension
{
    SCH_XML_AXIS_X = 0,
    SCH_XML_AXIS_Y,
    SCH_XML_AXIS_Z,
    SCH_XML_AXIS_UNDEF
};

struct SchXMLAxis
{
    enum SchXMLAxisDimension eDimension;
    sal_Int8 nAxisIndex;//0->primary axis; 1->secondary axis
    rtl::OUString aName;
    rtl::OUString aTitle;
    bool bHasCategories;

    SchXMLAxis() : eDimension( SCH_XML_AXIS_UNDEF ), nAxisIndex( 0 ), bHasCategories( false ) {}
};

// ----------------------------------------

struct GlobalSeriesImportInfo
{
    GlobalSeriesImportInfo( sal_Bool& rAllRangeAddressesAvailable )
        : rbAllRangeAddressesAvailable( rAllRangeAddressesAvailable )
        , nCurrentDataIndex( 0 )
        , nFirstFirstDomainIndex( -1 )
        , nFirstSecondDomainIndex( -1 )
    {}

    sal_Bool& rbAllRangeAddressesAvailable;

    sal_Int32 nCurrentDataIndex;

    ::rtl::OUString aFirstFirstDomainAddress;
    sal_Int32 nFirstFirstDomainIndex;

    ::rtl::OUString aFirstSecondDomainAddress;
    sal_Int32 nFirstSecondDomainIndex;
};

struct DataRowPointStyle
{
    enum StyleType
    {
        DATA_POINT,
        DATA_SERIES,
        MEAN_VALUE,
        REGRESSION,
        ERROR_INDICATOR
    };

    StyleType meType;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > m_xOldAPISeries;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > m_xEquationProperties;
    sal_Int32 m_nPointIndex;
    sal_Int32 m_nPointRepeat;
    ::rtl::OUString msStyleName;
    ::rtl::OUString msSeriesStyleNameForDonuts;
    sal_Int32 mnAttachedAxis;
    bool mbSymbolSizeForSeriesIsMissingInFile;

    DataRowPointStyle( StyleType eType
                       , const ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::XDataSeries >& xSeries
                        , sal_Int32 nPointIndex
                        , sal_Int32 nPointRepeat
                        , ::rtl::OUString sStyleName
                        , sal_Int32 nAttachedAxis = 0 ) :
            meType( eType ),
            m_xSeries( xSeries ),
            m_xOldAPISeries( 0 ),
            m_nPointIndex( nPointIndex ),
            m_nPointRepeat( nPointRepeat ),
            msStyleName( sStyleName ),
            mnAttachedAxis( nAttachedAxis ),
            mbSymbolSizeForSeriesIsMissingInFile( false )
        {}
};

typedef ::std::multimap< ::rtl::OUString, ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > > tSchXMLRangeSequenceMap;

#endif  // SCH_XML_TRANSPORTTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
