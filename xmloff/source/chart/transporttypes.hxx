/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
    OUString aString;
    ::com::sun::star::uno::Sequence< OUString > aComplexString;
    double fValue;
    SchXMLCellType eType;
    OUString aRangeId;

    SchXMLCell(): fValue( 0.0 ), eType( SCH_CELL_TYPE_UNKNOWN )
    {}
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

    OUString aTableNameOfFile;                   /// the table name read at the table:table element

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
    OUString aName;
    OUString aTitle;
    bool bHasCategories;

    SchXMLAxis() : eDimension( SCH_XML_AXIS_UNDEF ), nAxisIndex( 0 ), bHasCategories( false ) {}
};

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

    OUString aFirstFirstDomainAddress;
    sal_Int32 nFirstFirstDomainIndex;

    OUString aFirstSecondDomainAddress;
    sal_Int32 nFirstSecondDomainIndex;
};

struct RegressionStyle
{
    com::sun::star::uno::Reference<
                com::sun::star::chart2::XDataSeries > m_xSeries;
    com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > m_xEquationProperties;

    OUString msStyleName;

    RegressionStyle(const com::sun::star::uno::Reference<
                          com::sun::star::chart2::XDataSeries >& xSeries,
                    OUString sStyleName) :
            m_xSeries    ( xSeries ),
            msStyleName  ( sStyleName )
    {}
};

struct DataRowPointStyle
{
    enum StyleType
    {
        DATA_POINT,
        DATA_SERIES,
        MEAN_VALUE,
        ERROR_INDICATOR
    };

    StyleType meType;
    com::sun::star::uno::Reference<
                com::sun::star::chart2::XDataSeries > m_xSeries;

    com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > m_xOldAPISeries;

    com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > m_xErrorXProperties;

    com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > m_xErrorYProperties;

    sal_Int32 m_nPointIndex;
    sal_Int32 m_nPointRepeat;
    OUString msStyleName;
    OUString msSeriesStyleNameForDonuts;
    sal_Int32 mnAttachedAxis;
    bool mbSymbolSizeForSeriesIsMissingInFile;

    DataRowPointStyle( StyleType eType
                        , const com::sun::star::uno::Reference<
                          com::sun::star::chart2::XDataSeries >& xSeries
                        , sal_Int32 nPointIndex
                        , sal_Int32 nPointRepeat
                        , OUString sStyleName
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

typedef ::std::multimap< OUString, ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > > tSchXMLRangeSequenceMap;

#endif  // SCH_XML_TRANSPORTTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
