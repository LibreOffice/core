/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transporttypes.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:01:05 $
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
#ifndef SCH_XML_TRANSPORTTYPES_HXX_
#define SCH_XML_TRANSPORTTYPES_HXX_

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif

#include <vector>
#include <map>

enum SchXMLCellType
{
    SCH_CELL_TYPE_UNKNOWN,
    SCH_CELL_TYPE_FLOAT,
    SCH_CELL_TYPE_STRING
};

struct SchXMLCell
{
    rtl::OUString aString;
    double fValue;
    SchXMLCellType eType;
    rtl::OUString aRangeId;

    SchXMLCell() : fValue( 0.0 ), eType( SCH_CELL_TYPE_UNKNOWN ) {}
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

    SchXMLTable() : nRowIndex( -1 ),
                    nColumnIndex( -1 ),
                    nMaxColumnIndex( -1 ),
                    nNumberOfColsEstimate( 0 ),
                    bHasHeaderRow( false ),
                    bHasHeaderColumn( false )
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

enum SchXMLAxisClass
{
    SCH_XML_AXIS_X = 0,
    SCH_XML_AXIS_Y,
    SCH_XML_AXIS_Z,
    SCH_XML_AXIS_UNDEF
};

struct SchXMLAxis
{
    enum SchXMLAxisClass eClass;
    sal_Int8 nIndexInCategory;
    rtl::OUString aName;
    rtl::OUString aTitle;
    bool bHasCategories;

    SchXMLAxis() : eClass( SCH_XML_AXIS_UNDEF ), nIndexInCategory( 0 ), bHasCategories( false ) {}
};

// ----------------------------------------

// struct PostponedStyleObject
// {
// };

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
