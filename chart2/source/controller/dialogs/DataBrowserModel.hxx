/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataBrowserModel.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:25:42 $
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
#ifndef CHART2_DATABROWSERMODEL_HXX
#define CHART2_DATABROWSERMODEL_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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

    void setModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    /** Inserts a new data series after the data series to which the data column
        with index nAfterColumnIndex belongs.
     */
    void insertDataSeries( sal_Int32 nAfterColumnIndex );

    /** Removes a data series to which the data column with index nAtColumnIndex
        belongs.
     */
    void removeDataSeries( sal_Int32 nAtColumnIndex );

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
        TEXT
    };

    eCellType getCellType( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    /// If getCellType( nAtColumn, nAtRow ) returns TEXT, the result will be Nan
    double getCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    ::rtl::OUString getCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow );
    sal_uInt32 getNumberFormatKey( sal_Int32 nAtColumn, sal_Int32 nAtRow );

    /// returns </TRUE> if the number could successfully be set at the given position
    bool setCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow, double fValue );
    /// returns </TRUE> if the text could successfully be set at the given position
    bool setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const ::rtl::OUString & rText );
    bool setCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow, const ::com::sun::star::uno::Any & aValue );

    sal_Int32 getColumnCount() const;
    sal_Int32 getMaxRowCount() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >
        getDataOfColumn( sal_Int32 nColumnIndex ) const;
    // returns the UI string of the corresponding role
    ::rtl::OUString getRoleOfColumn( sal_Int32 nColumnIndex ) const;

    /** Applies the content of xSource to xDestination.  As a result
        xDestination is a copy of xSource, but maintains its identity.
        (Something like a flat assignment operator)
     */
    static void restoreModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xDestination );

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

    tDataHeaderVector getDataHeaders() const;

    tDataHeader getHeaderForSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > &xSeries ) const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >
        getDataSeriesByColumn( sal_Int32 nColumn ) const;

private:
    void updateFromModel();
    void applyToModel();

    void addErrorBarRanges(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        sal_Int32 nNumberFormatKey,
        sal_Int32 & rInOutSequenceIndex,
        sal_Int32 & rInOutHeaderEnd );

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >
        getCategories() const throw();

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
