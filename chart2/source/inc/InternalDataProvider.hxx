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

#include "InternalData.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <map>

namespace com::sun::star::chart2 { class XChartDocument; }

namespace chart
{

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
class InternalDataProvider :
        public impl::InternalDataProvider_Base
{
public:
    explicit InternalDataProvider();

    // #i120559# allow handing over a default for data orientation
    // (DataInColumns) that will be used when no data is available
    explicit InternalDataProvider(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc,
        bool bConnectToModel,
        bool bDefaultDataInColumns );
    explicit InternalDataProvider( const InternalDataProvider & rOther );
    virtual ~InternalDataProvider() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XInternalDataProvider ____
    virtual sal_Bool SAL_CALL hasDataByRangeRepresentation( const OUString& aRange ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
        getDataByRangeRepresentation( const OUString& aRange ) override;
    virtual void SAL_CALL setDataByRangeRepresentation(
        const OUString& aRange,
        const css::uno::Sequence< css::uno::Any >& aNewData ) override;
    virtual void SAL_CALL insertSequence( ::sal_Int32 nAfterIndex ) override;
    virtual void SAL_CALL deleteSequence( ::sal_Int32 nAtIndex ) override;
    virtual void SAL_CALL appendSequence() override;
    virtual void SAL_CALL insertComplexCategoryLevel( ::sal_Int32 nLevel ) override;
    virtual void SAL_CALL deleteComplexCategoryLevel( ::sal_Int32 nLevel ) override;
    virtual void SAL_CALL insertDataPointForAllSequences( ::sal_Int32 nAfterIndex ) override;
    virtual void SAL_CALL deleteDataPointForAllSequences( ::sal_Int32 nAtIndex ) override;
    virtual void SAL_CALL swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex ) override;
    virtual void SAL_CALL registerDataSequenceForChanges(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSeq ) override;

    // ____ XDataProvider (base of XInternalDataProvider) ____
    virtual sal_Bool SAL_CALL createDataSourcePossible(
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL createDataSource(
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL detectArguments(
        const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource ) override;
    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
        const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection() override;

    // ____ XRangeXMLConversion ____
    virtual OUString SAL_CALL convertRangeToXML(
        const OUString& aRangeRepresentation ) override;
    virtual OUString SAL_CALL convertRangeFromXML(
        const OUString& aXMLRange ) override;

    // ____ XDateCategories ____
    virtual css::uno::Sequence< double > SAL_CALL getDateCategories() override;
    virtual void SAL_CALL setDateCategories( const css::uno::Sequence< double >& rDates ) override;

    // ____ XAnyDescriptionAccess ____
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyRowDescriptions() override;
    virtual void SAL_CALL setAnyRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aRowDescriptions ) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyColumnDescriptions() override;
    virtual void SAL_CALL setAnyColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aColumnDescriptions ) override;

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexRowDescriptions() override;
    virtual void SAL_CALL setComplexRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexColumnDescriptions() override;
    virtual void SAL_CALL setComplexColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData() override;
    virtual void SAL_CALL setData(
        const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions() override;
    virtual void SAL_CALL setRowDescriptions(
        const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions() override;
    virtual void SAL_CALL setColumnDescriptions(
        const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(
        const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL removeChartDataChangeEventListener(
        const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber() override;
    virtual sal_Bool SAL_CALL isNotANumber(
        double nNumber ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;
    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) override;

private:
    void addDataSequenceToMap(
        const OUString & rRangeRepresentation,
        const css::uno::Reference< css::chart2::data::XDataSequence > & xSequence );

    css::uno::Reference< css::chart2::data::XDataSequence >
        createDataSequenceAndAddToMap( const OUString & rRangeRepresentation,
                                       const OUString & rRole );
    css::uno::Reference< css::chart2::data::XDataSequence >
        createDataSequenceAndAddToMap( const OUString & rRangeRepresentation );

    css::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceFromArray( const OUString& rArrayStr, const OUString& rRole );

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
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
