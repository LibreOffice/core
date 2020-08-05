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
#pragma once

#include <com/sun/star/uno/Reference.hxx>

#include <memory>
#include <vector>

namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::uno { class XComponentContext; }

namespace com::sun::star::chart2 {
    class XDataSeries;
    class XChartType;
}

namespace chart
{

class DialogModel;

class DataBrowserModel final
{
public:
    explicit DataBrowserModel(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc,
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~DataBrowserModel();

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

    eCellType getCellType( sal_Int32 nAtColumn ) const;
    /// If getCellType( nAtColumn, nAtRow ) returns TEXT, the result will be Nan
    double getCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    OUString getCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    css::uno::Any getCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    sal_uInt32 getNumberFormatKey( sal_Int32 nAtColumn );

    /// returns </sal_True> if the number could successfully be set at the given position
    bool setCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow, double fValue );
    /// returns </sal_True> if the text could successfully be set at the given position
    bool setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const OUString & rText );
    bool setCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow, const css::uno::Any & aValue );

    sal_Int32 getColumnCount() const;
    sal_Int32 getMaxRowCount() const;

    // returns the UI string of the corresponding role
    OUString getRoleOfColumn( sal_Int32 nColumnIndex ) const;
    bool isCategoriesColumn( sal_Int32 nColumnIndex ) const;

    struct tDataHeader
    {
        css::uno::Reference< css::chart2::XDataSeries > m_xDataSeries;
        css::uno::Reference< css::chart2::XChartType >  m_xChartType;
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
            css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries,
            css::uno::Reference< css::chart2::XChartType > const &xChartType,
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

    typedef std::vector< tDataHeader > tDataHeaderVector;

    const tDataHeaderVector& getDataHeaders() const { return m_aHeaders;}

    tDataHeader getHeaderForSeries(
        const css::uno::Reference< css::chart2::XDataSeries > &xSeries ) const;

    css::uno::Reference< css::chart2::XDataSeries >
        getDataSeriesByColumn( sal_Int32 nColumn ) const;

private:
    void updateFromModel();

    void removeComplexCategoryLevel( sal_Int32 nAtColumnIndex );

    void addErrorBarRanges(
        const css::uno::Reference<css::chart2::XDataSeries > & xDataSeries,
        sal_Int32 nNumberFormatKey,
        sal_Int32 & rInOutSequenceIndex,
        sal_Int32 & rInOutHeaderEnd, bool bYError );

    sal_Int32 getCategoryColumnCount();

    css::uno::Reference< css::chart2::XChartDocument > m_xChartDocument;
    std::unique_ptr< DialogModel > m_apDialogModel;

    struct tDataColumn;
    struct implColumnLess;

    typedef std::vector< tDataColumn > tDataColumnVector;

    tDataColumnVector m_aColumns;
    tDataHeaderVector m_aHeaders;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
