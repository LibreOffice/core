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
#ifndef CHART2_INTERNALDATAPROVIDER_HXX
#define CHART2_INTERNALDATAPROVIDER_HXX

#include "InternalData.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/implbase7.hxx>
#include "ServiceMacros.hxx"

#include "CachedDataSequence.hxx"

#include <map>
#include <memory>

namespace chart
{

namespace impl
{

typedef ::cppu::WeakImplHelper7<
        ::com::sun::star::chart2::XInternalDataProvider,
        ::com::sun::star::chart2::data::XRangeXMLConversion,
        ::com::sun::star::chart2::XAnyDescriptionAccess,
        ::com::sun::star::chart::XDateCategories,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::lang::XServiceInfo >
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
    explicit InternalDataProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & _xContext);
    explicit InternalDataProvider( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::chart2::XChartDocument > & xChartDoc, bool bConnectToModel );
    explicit InternalDataProvider( const InternalDataProvider & rOther );
    virtual ~InternalDataProvider();

    void createDefaultData();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(InternalDataProvider)

    // ____ XInternalDataProvider ____
    virtual ::sal_Bool SAL_CALL hasDataByRangeRepresentation( const OUString& aRange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
        getDataByRangeRepresentation( const OUString& aRange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataByRangeRepresentation(
        const OUString& aRange,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aNewData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertSequence( ::sal_Int32 nAfterIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteSequence( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL appendSequence()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertComplexCategoryLevel( ::sal_Int32 nLevel )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteComplexCategoryLevel( ::sal_Int32 nLevel )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertDataPointForAllSequences( ::sal_Int32 nAfterIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteDataPointForAllSequences( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerDataSequenceForChanges(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSeq )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataProvider (base of XInternalDataProvider) ____
    virtual ::sal_Bool SAL_CALL createDataSourcePossible(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
        const OUString& aRangeRepresentation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XRangeXMLConversion ____
    virtual OUString SAL_CALL convertRangeToXML(
        const OUString& aRangeRepresentation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL convertRangeFromXML(
        const OUString& aXMLRange )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XDateCategories ____
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getDateCategories() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDateCategories( const ::com::sun::star::uno::Sequence< double >& rDates ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XAnyDescriptionAccess ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL
        getAnyRowDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAnyRowDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL
        getAnyColumnDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAnyColumnDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL
        getComplexRowDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComplexRowDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< OUString > >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL
        getComplexColumnDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComplexColumnDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< OUString > >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRowDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions(
        const ::com::sun::star::uno::Sequence< OUString >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions(
        const ::com::sun::star::uno::Sequence< OUString >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isNotANumber(
        double nNumber )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

private:
    void lcl_addDataSequenceToMap(
        const OUString & rRangeRepresentation,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xSequence );

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >
        lcl_createDataSequenceAndAddToMap( const OUString & rRangeRepresentation,
                                       const OUString & rRole );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >
        lcl_createDataSequenceAndAddToMap( const OUString & rRangeRepresentation );

    void lcl_deleteMapReferences( const OUString & rRangeRepresentation );

    void lcl_adaptMapReferences(
        const OUString & rOldRangeRepresentation,
        const OUString & rNewRangeRepresentation );

    void lcl_increaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );
    void lcl_decreaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );

    typedef ::std::multimap< OUString,
            ::com::sun::star::uno::WeakReference< ::com::sun::star::chart2::data::XDataSequence > >
        tSequenceMap;
    typedef ::std::pair< tSequenceMap::iterator, tSequenceMap::iterator > tSequenceMapRange;
    typedef ::std::pair< tSequenceMap::const_iterator, tSequenceMap::const_iterator > tConstSequenceMapRange;

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

// CHART2_INTERNALDATAPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
