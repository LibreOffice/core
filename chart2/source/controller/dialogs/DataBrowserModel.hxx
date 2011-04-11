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
#ifndef CHART2_DATABROWSERMODEL_HXX
#define CHART2_DATABROWSERMODEL_HXX

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace chart2 {
    class XDataSeries;
    class XChartType;
    namespace data {
        class XLabeledDataSequence;
    }
}}}}

namespace chart
{

class DialogModel;

class DataBrowserModel
{
public:
    explicit DataBrowserModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDoc,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataBrowserModel();

    /** Inserts a new data series after the data series to which the data column
        with index nAfterColumnIndex belongs.
     */
    void insertDataSeries( sal_Int32 nAfterColumnIndex );

    /** Inserts a new text column for complex categories.
     */
    void insertComplexCategoryLevel( sal_Int32 nAfterColumnIndex );

    /** Removes a data series to which the data column with index nAtColumnIndex
        belongs.
     */
    void removeDataSeriesOrComplexCategoryLevel( sal_Int32 nAtColumnIndex );

    /** Swaps the series to which the data column with index nFirstIndex belongs
        with the next series (which starts at an index >= nFirstIndex + 1)
     */
    void swapDataSeries( sal_Int32 nFirstIndex );
    void swapDataPointForAllSeries( sal_Int32 nFirstIndex );

    void insertDataPointForAllSeries( sal_Int32 nAfterIndex );
    void removeDataPointForAllSeries( sal_Int32 nAtIndex );

    enum eCellType
    {
        NUMBER,
        TEXT,
        TEXTORDATE
    };

    eCellType getCellType( sal_Int32 nAtColumn, sal_Int32 nAtRow ) const;
    /// If getCellType( nAtColumn, nAtRow ) returns TEXT, the result will be Nan
    double getCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    ::rtl::OUString getCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    ::com::sun::star::uno::Any getCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    sal_uInt32 getNumberFormatKey( sal_Int32 nAtColumn, sal_Int32 nAtRow );

    /// returns </sal_True> if the number could successfully be set at the given position
    bool setCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow, double fValue );
    /// returns </sal_True> if the text could successfully be set at the given position
    bool setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const ::rtl::OUString & rText );
    bool setCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow, const ::com::sun::star::uno::Any & aValue );

    sal_Int32 getColumnCount() const;
    sal_Int32 getMaxRowCount() const;

    // returns the UI string of the corresponding role
    ::rtl::OUString getRoleOfColumn( sal_Int32 nColumnIndex ) const;
    bool isCategoriesColumn( sal_Int32 nColumnIndex ) const;

    struct tDataHeader
    {
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >     m_xDataSeries;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >      m_xChartType;
        bool                                            m_bSwapXAndYAxis;
        sal_Int32                                       m_nStartColumn;
        sal_Int32                                       m_nEndColumn;

        // default CTOR
        tDataHeader() :
                m_bSwapXAndYAxis( false ),
                m_nStartColumn( -1 ),
                m_nEndColumn( -1 )
        {}
        // "full" CTOR
        tDataHeader(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > xDataSeries,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >  xChartType,
            bool                                        bSwapXAndYAxis,
            sal_Int32                                   nStartColumn,
            sal_Int32                                   nEndColumn ) :
                m_xDataSeries( xDataSeries ),
                m_xChartType( xChartType ),
                m_bSwapXAndYAxis( bSwapXAndYAxis ),
                m_nStartColumn( nStartColumn ),
                m_nEndColumn( nEndColumn )
        {}
    };

    typedef ::std::vector< tDataHeader > tDataHeaderVector;

    const tDataHeaderVector& getDataHeaders() const;

    tDataHeader getHeaderForSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > &xSeries ) const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >
        getDataSeriesByColumn( sal_Int32 nColumn ) const;

private:
    void updateFromModel();

    void addErrorBarRanges(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        sal_Int32 nNumberFormatKey,
        sal_Int32 & rInOutSequenceIndex,
        sal_Int32 & rInOutHeaderEnd );

    sal_Int32 getCategoryColumnCount();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > m_xContext;
    ::std::auto_ptr< DialogModel > m_apDialogModel;

    struct tDataColumn;
    struct implColumnLess;

    typedef ::std::vector< tDataColumn > tDataColumnVector;

    tDataColumnVector m_aColumns;
    tDataHeaderVector m_aHeaders;
};

} //  namespace chart

// CHART2_DATABROWSERMODEL_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
