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
#ifndef CHART2_DIALOGMODEL_HXX
#define CHART2_DIALOGMODEL_HXX

#include "TimerTriggeredControllerLock.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star { namespace chart2 {
    class XDataSeriesContainer;
    class XDataSeries;
    class XChartType;
    class XChartTypeTemplate;
    struct InterpretedData;
    namespace data {
        class XDataProvider;
        class XLabeledDataSequence;
    }
}}}}

namespace chart
{

class RangeSelectionHelper;

class DialogModel
{
public:
    explicit DialogModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    ~DialogModel();

    typedef ::std::pair<
                ::rtl::OUString,
                ::std::pair< ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XDataSeries >,
                             ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XChartType > > >
        tSeriesWithChartTypeByName;

    typedef ::std::map< ::rtl::OUString, ::rtl::OUString >
        tRolesWithRanges;

    void setTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartTypeTemplate > & xTemplate );

    ::boost::shared_ptr< RangeSelectionHelper >
        getRangeSelectionHelper() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >
        getChartModel() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider >
        getDataProvider() const;

    ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeriesContainer > >
        getAllDataSeriesContainers() const;

    ::std::vector< tSeriesWithChartTypeByName >
        getAllDataSeriesWithLabel() const;

    tRolesWithRanges getRolesWithRanges(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::rtl::OUString & aRoleOfSequenceForLabel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType ) const;

    enum eMoveDirection
    {
        MOVE_DOWN,
        MOVE_UP
    };

    void moveSeries( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::chart2::XDataSeries > & xSeries,
                     eMoveDirection eDirection );

    /// @return the newly inserted series
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > insertSeriesAfter(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries > & xSeries,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartType > & xChartType,
                bool bCreateDataCachedSequences = false );

    void deleteSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType );

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >
        getCategories() const;

    void setCategories( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::data::XLabeledDataSequence > & xCategories );

    ::rtl::OUString getCategoriesRange() const;

    bool isCategoryDiagram() const;

    void detectArguments(
        ::rtl::OUString & rOutRangeString,
        bool & rOutUseColumns, bool & rOutFirstCellAsLabel, bool & rOutHasCategories ) const;

    bool allArgumentsForRectRangeDetected() const;

    bool setData( const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::beans::PropertyValue > & rArguments );

    void startControllerLockTimer();

    static ::rtl::OUString ConvertRoleFromInternalToUI( const ::rtl::OUString & rRoleString );
    static ::rtl::OUString GetRoleDataLabel();

    // pass a role string (not translated) and get an index that serves for
    // relative ordering, to get e.g. x-values and y-values in the right order
    static sal_Int32 GetRoleIndexForSorting( const ::rtl::OUString & rInternalRoleString );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartTypeTemplate >
        m_xTemplate;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext >
        m_xContext;

    mutable ::boost::shared_ptr< RangeSelectionHelper >
        m_spRangeSelectionHelper;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

private:
    void createBackup();

    void applyInterpretedData(
        const ::com::sun::star::chart2::InterpretedData & rNewData,
        const ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > > & rSeriesToReUse,
        bool bSetStyles );

    sal_Int32 countSeries() const;
};

} //  namespace chart

// CHART2_DIALOGMODEL_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
