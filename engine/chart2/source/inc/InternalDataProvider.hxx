/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "InternalData.hxx"
#include <ChartModel.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

#include <map>

namespace chart
{
class ChartModel;
class UncachedDataSequence;

namespace impl
{

typedef ::cppu::WeakImplHelper<
        css::chart2::XInternalDataProvider,
        css::chart2::data::XRangeXMLConversion,
        css::chart2::XAnyDescriptionAccess,
        css::chart::XDateCategories,
        css::util::XCloneable,
        css::lang::XInitialization,
        css::lang::XServiceInfo >
    InternalDataProvider_Base;
}

/** Data provider that handles data internally.  This is used for charts with
    their own data.

    <p>The format for single ranges is "categories|label n|n" where n is a
    non-negative number. Meaning return all categories, the label of sequence n,
    or the data of sequence n.</p>

    <p>The format for a complete range is "all". (Do we need more than
    that?)</p>
 */
class InternalDataProvider final :
        public impl::InternalDataProvider_Base
{
public:
    explicit InternalDataProvider();

    // #i120559# allow handing over a default for data orientation
    // (DataInColumns) that will be used when no data is available
    explicit InternalDataProvider(
        const rtl::Reference< ::chart::ChartModel > & xChartDoc,
        bool bConnectToModel );
    explicit InternalDataProvider( const InternalDataProvider & rOther );
    virtual ~InternalDataProvider() override;

    /// declare XServiceInfo methods
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // ____ XInternalDataProvider ____
    virtual bool hasDataByRangeRepresentation( const OUString& aRange ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any >
        getDataByRangeRepresentation( const OUString& aRange ) override;
    virtual void setDataByRangeRepresentation(
        const OUString& aRange,
        const cpo::uno::Sequence< cpo::uno::Any >& aNewData ) override;
    virtual void insertSequence( ::sal_Int32 nAfterIndex ) override;
    virtual void deleteSequence( ::sal_Int32 nAtIndex ) override;
    virtual void appendSequence() override;
    virtual void insertComplexCategoryLevel( ::sal_Int32 nLevel ) override;
    virtual void deleteComplexCategoryLevel( ::sal_Int32 nLevel ) override;
    virtual void insertDataPointForAllSequences( ::sal_Int32 nAfterIndex ) override;
    virtual void deleteDataPointForAllSequences( ::sal_Int32 nAtIndex ) override;
    virtual void swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex ) override;
    virtual void registerDataSequenceForChanges(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSeq ) override;
    virtual void insertDataSeries( ::sal_Int32 nAfterIndex ) override;

    // ____ XDataProvider (base of XInternalDataProvider) ____
    virtual bool createDataSourcePossible(
        const cpo::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > createDataSource(
        const cpo::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > detectArguments(
        const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource ) override;
    virtual bool createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > createDataSequenceByRangeRepresentation(
        const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation,
            const OUString& aRoleQualifier ) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > getRangeSelection() override;

    // ____ XRangeXMLConversion ____
    virtual OUString convertRangeToXML(
        const OUString& aRangeRepresentation ) override;
    virtual OUString convertRangeFromXML(
        const OUString& aXMLRange ) override;

    // ____ XDateCategories ____
    virtual cpo::uno::Sequence< double > getDateCategories() override;
    virtual void setDateCategories( const cpo::uno::Sequence< double >& rDates ) override;

    // ____ XAnyDescriptionAccess ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >
        getAnyRowDescriptions() override;
    virtual void setAnyRowDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >
        getAnyColumnDescriptions() override;
    virtual void setAnyColumnDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aColumnDescriptions ) override;

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >
        getComplexRowDescriptions() override;
    virtual void setComplexRowDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >
        getComplexColumnDescriptions() override;
    virtual void setComplexColumnDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< double > > getData() override;
    virtual void setData(
        const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aData ) override;
    virtual cpo::uno::Sequence< OUString > getRowDescriptions() override;
    virtual void setRowDescriptions(
        const cpo::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< OUString > getColumnDescriptions() override;
    virtual void setColumnDescriptions(
        const cpo::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void addChartDataChangeEventListener(
        const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void removeChartDataChangeEventListener(
        const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double getNotANumber() override;
    virtual bool isNotANumber(
        double nNumber ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;
    // css::lang::XInitialization:
    virtual void initialize(const cpo::uno::Sequence< cpo::uno::Any > & aArguments) override;

    void setChartModel(ChartModel* pChartModel);

private:
    void addDataSequenceToMap(
        const OUString & rRangeRepresentation,
        const css::uno::Reference< css::chart2::data::XDataSequence > & xSequence );

    css::uno::Reference< css::chart2::data::XDataSequence >
        createDataSequenceAndAddToMap( const OUString & rRangeRepresentation,
                                       const OUString & rRole );
    rtl::Reference< UncachedDataSequence >
        createDataSequenceAndAddToMap( const OUString & rRangeRepresentation );

    rtl::Reference<UncachedDataSequence>
        createDataSequenceFromArray( const OUString& rArrayStr, std::u16string_view rRole,
            std::u16string_view rRoleQualifier);

    void deleteMapReferences( const OUString & rRangeRepresentation );

    void adaptMapReferences(
        const OUString & rOldRangeRepresentation,
        const OUString & rNewRangeRepresentation );

    void increaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );
    void decreaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );

    typedef std::multimap< OUString,
            css::uno::WeakReference< css::chart2::data::XDataSequence > >
        tSequenceMap;
    typedef std::pair< tSequenceMap::iterator, tSequenceMap::iterator > tSequenceMapRange;

    /** cache for all sequences that have been returned.

        If the range-representation of a sequence changes and it is still
        referred to by some component (weak reference is valid), the range will
        be adapted.
     */
    tSequenceMap m_aSequenceMap;
    InternalData m_aInternalData;
    bool m_bDataInColumns;

    // keep a weak reference to the owning m_xChartModel for insertDataSeries
    unotools::WeakReference<ChartModel> m_xChartModel;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
