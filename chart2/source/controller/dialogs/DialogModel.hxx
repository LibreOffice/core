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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DIALOGMODEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DIALOGMODEL_HXX

#include <TimerTriggeredControllerLock.hxx>
#include <rtl/ustring.hxx>

#include <map>
#include <memory>
#include <vector>

namespace chart { class ChartModel; }
namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XChartDocument; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <class E> class Sequence; } } } }

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

struct DialogModelTimeBasedInfo
{
    DialogModelTimeBasedInfo();

    bool bTimeBased;
    sal_Int32 nStart;
    sal_Int32 nEnd;
};

class DialogModel
{
public:
    explicit DialogModel(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument,
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~DialogModel();

    typedef std::pair<
                OUString,
                std::pair< css::uno::Reference< css::chart2::XDataSeries >,
                             css::uno::Reference< css::chart2::XChartType > > >
        tSeriesWithChartTypeByName;

    typedef std::map< OUString, OUString >
        tRolesWithRanges;

    void setTemplate(
        const css::uno::Reference< css::chart2::XChartTypeTemplate > & xTemplate );

    std::shared_ptr< RangeSelectionHelper > const &
        getRangeSelectionHelper() const;

    css::uno::Reference< css::frame::XModel >
        getChartModel() const;

    css::uno::Reference< css::chart2::data::XDataProvider >
        getDataProvider() const;

    std::vector< css::uno::Reference< css::chart2::XDataSeriesContainer > >
        getAllDataSeriesContainers() const;

    std::vector< tSeriesWithChartTypeByName >
        getAllDataSeriesWithLabel() const;

    static tRolesWithRanges getRolesWithRanges(
        const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
        const OUString & aRoleOfSequenceForLabel,
        const css::uno::Reference< css::chart2::XChartType > & xChartType );

    enum class MoveDirection
    {
        Down, Up
    };

    void moveSeries( const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
                     MoveDirection eDirection );

    /// @return the newly inserted series
    css::uno::Reference<
            css::chart2::XDataSeries > insertSeriesAfter(
                const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
                const css::uno::Reference< css::chart2::XChartType > & xChartType,
                bool bCreateDataCachedSequences = false );

    void deleteSeries(
        const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
        const css::uno::Reference< css::chart2::XChartType > & xChartType );

    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
        getCategories() const;

    void setCategories( const css::uno::Reference< css::chart2::data::XLabeledDataSequence > & xCategories );

    OUString getCategoriesRange() const;

    bool isCategoryDiagram() const;

    void detectArguments(
        OUString & rOutRangeString,
        bool & rOutUseColumns, bool & rOutFirstCellAsLabel, bool & rOutHasCategories ) const;

    bool allArgumentsForRectRangeDetected() const;

    void setData( const css::uno::Sequence< css::beans::PropertyValue > & rArguments );

    void setTimeBasedRange( bool bTimeBased, sal_Int32 nStart, sal_Int32 nEnd) const;

    const DialogModelTimeBasedInfo& getTimeBasedInfo() const { return maTimeBasedInfo; }

    void startControllerLockTimer();

    static OUString ConvertRoleFromInternalToUI( const OUString & rRoleString );
    static OUString GetRoleDataLabel();

    // pass a role string (not translated) and get an index that serves for
    // relative ordering, to get e.g. x-values and y-values in the right order
    static sal_Int32 GetRoleIndexForSorting( const OUString & rInternalRoleString );

    ChartModel& getModel() const;

private:
    css::uno::Reference< css::chart2::XChartDocument >
        m_xChartDocument;

    css::uno::Reference< css::chart2::XChartTypeTemplate >
        m_xTemplate;

    css::uno::Reference< css::uno::XComponentContext >
        m_xContext;

    mutable std::shared_ptr< RangeSelectionHelper >
        m_spRangeSelectionHelper;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

private:
    void applyInterpretedData(
        const css::chart2::InterpretedData & rNewData,
        const std::vector< css::uno::Reference< css::chart2::XDataSeries > > & rSeriesToReUse );

    sal_Int32 countSeries() const;

    mutable DialogModelTimeBasedInfo maTimeBasedInfo;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DIALOGMODEL_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
