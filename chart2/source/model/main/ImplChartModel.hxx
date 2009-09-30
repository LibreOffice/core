/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImplChartModel.hxx,v $
 * $Revision: 1.10.44.1 $
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
#ifndef CHART_IMPLCHARTMODEL_HXX
#define CHART_IMPLCHARTMODEL_HXX

#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XChartTypeManager.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XUndoManager.hpp>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <cppuhelper/weakref.hxx>

#include "ChartData.hxx"

#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace embed {
        class XStorage;
    }
    namespace document {
        class XFilter;
    }
    namespace util {
        class XModifyListener;
    }
}}}

class SvNumberFormatter;

namespace chart
{
namespace impl
{

class ImplChartModel
{
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > tDataSourceType;

public:
    ImplChartModel( ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext > const & xContext,
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::util::XModifyListener > & xListener );
    explicit ImplChartModel( const ImplChartModel & rOther,
                             const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::util::XModifyListener > & xListener );
    ~ImplChartModel();

//     ::com::sun::star::uno::Sequence<
//         ::com::sun::star::uno::Reference<
//         ::com::sun::star::chart2::XDataSeries > >
//         GetDataSeries() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess >
        GetStyleFamilies();

    // Diagram Access
    void RemoveAllDiagrams();
    /** @return true, if the chart was found and removed, false otherwise.
     */
    void AppendDiagram( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::chart2::XDiagram > & xDiagram );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >
        GetDiagram( size_t nIndex ) const
            throw( ::com::sun::star::container::NoSuchElementException );

    void SetDataProvider( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::data::XDataProvider > & xProvider );

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider > GetDataProvider() const;

    void CreateInternalDataProvider(
        bool bCloneExistingData,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    bool HasInternalDataProvider() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > SAL_CALL SetArguments(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & rArgs,
                bool bSetData )
        throw (::com::sun::star::lang::IllegalArgumentException);

    void SetChartTypeManager(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartTypeManager > & xManager );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartTypeManager >
        GetChartTypeManager();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartTypeTemplate >
        GetChartTypeTemplate();

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle >
        GetTitle();

    void SetTitle( const ::com::sun::star::uno::Reference<
                   ::com::sun::star::chart2::XTitle >& rTitle );

    /** Is called by the ChartModel's XComponent::dispose() to notify the
        impl-class to release resources
     */
    void dispose();

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        GetPageBackground();

    void CreateDefaultChart();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetDashTable() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetGradientTable() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetHatchTable() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetBitmapTable() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetTransparencyGradientTable() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > GetXMLNameSpaceMap() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
        GetNumberFormatsSupplier();

    void SetNumberFormatsSupplier(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier );

private:
    void CreateDefaultChartTypeTemplate();
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > CreateDefaultData();
    void SetNewData( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::chart2::data::XDataSource > & xDataSource,
                     const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & rArgs );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > SAL_CALL SetRangeRepresentation(
                const ::rtl::OUString & rRangeRepresentation, bool bSetData )
        throw (::com::sun::star::lang::IllegalArgumentException);

//     void CreateDefaultLayout();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  m_xFamilies;

    ::boost::shared_ptr< ChartData >                                              m_spChartData;

    // Data Access (deprecated, temporary solution)
//     ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > m_xChartData;
//     ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > m_xDataProvider;

//     ::std::vector< ::com::sun::star::uno::Reference<
//         ::com::sun::star::chart2::XDataSeries > >                               m_aInterpretedData;

    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                                m_xOwnNumberFormatsSupplier;
    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                                m_xNumberFormatsSupplier;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartTypeManager >
        m_xChartTypeManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartTypeTemplate >
        m_xChartTypeTemplate;

    // Diagram Access
    typedef ::std::vector< ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XDiagram > >
        tDiagramContainer;

    tDiagramContainer                     m_aDiagrams;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle >
                                          m_xTitle;

    bool                                  m_bIsDisposed;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                          m_xPageBackground;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xDashTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xHatchTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xBitmapTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xTransparencyGradientTable;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xXMLNamespaceMap;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >     m_xModifyListener;
};

}  // namespace impl
}  // namespace chart

// CHART_IMPLCHARTMODEL_HXX
#endif
