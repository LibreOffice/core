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

#include "cellsuno.hxx"
#include "rangelst.hxx"
#include "externalrefmgr.hxx"
#include "types.hxx"
#include "chartlis.hxx"

#include <svl/lstner.hxx>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XSheetDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/chart2/XTimeBased.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include <memory>
#include <unordered_set>
#include <vector>

namespace com::sun::star::chart2::data { class XLabeledDataSequence; }

class ScDocument;

// DataProvider
class SC_DLLPUBLIC ScChart2DataProvider final : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataProvider,
                    css::chart2::data::XSheetDataProvider,
                    css::chart2::data::XRangeXMLConversion,
                    css::beans::XPropertySet,
                    css::lang::XServiceInfo>,
                public SfxListener
{
public:

    explicit ScChart2DataProvider( ScDocument* pDoc );
    virtual ~ScChart2DataProvider() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataProvider
    virtual sal_Bool SAL_CALL createDataSourcePossible(
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSource > SAL_CALL createDataSource(
            const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;

    virtual css::uno::Sequence<
        css::beans::PropertyValue > SAL_CALL detectArguments(
            const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource ) override;

    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
            const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection() override;

    // XSheetDataProvider
    virtual sal_Bool SAL_CALL createDataSequenceByFormulaTokensPossible(
        const css::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

    virtual css::uno::Reference< css::chart2::data::XDataSequence >
        SAL_CALL createDataSequenceByFormulaTokens(
            const css::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

    // XRangeXMLConversion
    virtual OUString SAL_CALL convertRangeToXML( const OUString& sRangeRepresentation ) override;

    virtual OUString SAL_CALL convertRangeFromXML( const OUString& sXMLRange ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& xListener) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& rListener) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

private:

    ScDocument*                 m_pDocument;
    SfxItemPropertySet          m_aPropSet;
    bool                        m_bIncludeHiddenCells;
};

// DataSource
class ScChart2DataSource final : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataSource,
                    css::lang::XServiceInfo>,
                public SfxListener
{
public:

    explicit ScChart2DataSource( ScDocument* pDoc);
    virtual ~ScChart2DataSource() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataSource
    virtual css::uno::Sequence< css::uno::Reference<
            css::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    // implementation

    void AddLabeledSequence(const css::uno::Reference < css::chart2::data::XLabeledDataSequence >& xNew);

private:

    ScDocument*                 m_pDocument;
    std::vector < css::uno::Reference< css::chart2::data::XLabeledDataSequence > > m_aLabeledSequences;

};

// DataSequence
class ScChart2DataSequence final : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataSequence,
                    css::chart2::data::XTextualDataSequence,
                    css::chart2::data::XNumericalDataSequence,
                    css::chart2::XTimeBased,
                    css::util::XCloneable,
                    css::util::XModifyBroadcaster,
                    css::beans::XPropertySet,
                    css::lang::XServiceInfo>,
                public SfxListener
{
public:
    explicit ScChart2DataSequence( ScDocument* pDoc,
            ::std::vector<ScTokenRef>&& rTokens, bool bIncludeHiddenCells );

    virtual ~ScChart2DataSequence() override;
    ScChart2DataSequence(const ScChart2DataSequence&) = delete;
    ScChart2DataSequence& operator=(const ScChart2DataSequence&) = delete;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataSequence
    virtual css::uno::Sequence< css::uno::Any >
        SAL_CALL getData() override;
    virtual OUString SAL_CALL getSourceRangeRepresentation() override;
    virtual css::uno::Sequence< OUString >
        SAL_CALL generateLabel(css::chart2::data::LabelOrigin nOrigin) override;

    /** Get the number format key for the n-th data entry
     * If nIndex == -1, then you will get the number format key for the first non-empty entry
     */
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex ) override;

    // XNumericalDataSequence
    virtual css::uno::Sequence< double >
        SAL_CALL getNumericalData() override;

    // XTextualDataSequence
    virtual css::uno::Sequence< OUString >
        SAL_CALL getTextualData() override;

    // XTimeBased
    virtual sal_Bool SAL_CALL switchToNext(sal_Bool bWrap) override;
    virtual sal_Bool SAL_CALL setToPointInTime(sal_Int32 nPoint) override;

    virtual void SAL_CALL setRange(sal_Int32 nStart, sal_Int32 nEnd) override;

    // XPropertySet
    virtual css::uno::Reference<
        css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& xListener) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& rListener) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<  css::beans::XVetoableChangeListener>& rListener) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

private:
    void setDataChangedHint(bool b);

    // Implementation
    void    RefChanged();
    DECL_LINK( ValueListenerHdl, const SfxHint&, void );

private:
    class ExternalRefListener final : public ScExternalRefManager::LinkListener
    {
    public:
        ExternalRefListener(ScChart2DataSequence& rParent, ScDocument* pDoc);
        virtual ~ExternalRefListener() override;
        ExternalRefListener(const ExternalRefListener&) = delete;
        ExternalRefListener& operator=(const ExternalRefListener&) = delete;

        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) override;
        void addFileId(sal_uInt16 nFileId);
        const std::unordered_set<sal_uInt16>& getAllFileIds() const { return maFileIds;}

    private:
        ScChart2DataSequence&       mrParent;
        std::unordered_set<sal_uInt16> maFileIds;
        ScDocument*                 mpDoc;
    };

    /**
     * Build an internal data array to cache the data ranges, and other
     * information such as hidden values.
     */
    void BuildDataCache();

    void RebuildDataCache();

    sal_Int32 FillCacheFromExternalRef(const ScTokenRef& pToken);

    void UpdateTokensFromRanges(const ScRangeList& rRanges);

    ExternalRefListener* GetExtRefListener();

    void StopListeningToAllExternalRefs();

    void CopyData(const ScChart2DataSequence& r);

private:

    // data array
    struct Item
    {
        double              mfValue;
        OUString     maString;
        bool                mbIsValue;
        ScAddress   mAddress;
        Item();
    };

    class HiddenRangeListener final : public ScChartHiddenRangeListener
    {
    public:
        HiddenRangeListener(ScChart2DataSequence& rParent);
        virtual ~HiddenRangeListener() override;

        virtual void notify() override;

    private:
        ScChart2DataSequence& mrParent;
    };

    /** This vector contains the cached data which was calculated with BuildDataCache(). */
    std::vector<Item>           m_aDataArray;

    /**
     * Cached data for getData.  We may also need to cache data for the
     * numerical and textural data series if they turn out to be bottlenecks
     * under certain scenarios.
     */
    css::uno::Sequence< css::uno::Any > m_aMixedDataCache;

    css::uno::Sequence<sal_Int32>  m_aHiddenValues;

    // properties
    css::chart2::data::DataSequenceRole  m_aRole;
    bool                        m_bIncludeHiddenCells;

    // internals
    typedef std::unique_ptr<std::vector<sal_uInt32> >  RangeIndexMapPtr;

    sal_Int64                   m_nObjectId;
    ScDocument*                 m_pDocument;
    std::vector<ScTokenRef>     m_aTokens;
    RangeIndexMapPtr            m_pRangeIndices;
    std::unique_ptr<ExternalRefListener>
                                m_pExtRefListener;
    SfxItemPropertySet          m_aPropSet;

    std::unique_ptr<HiddenRangeListener> m_pHiddenListener;

    std::unique_ptr<ScLinkListener>      m_pValueListener;
    XModifyListenerArr_Impl     m_aValueListeners;

    bool                        m_bGotDataChangedHint;
    bool                        m_bExtDataRebuildQueued;

    bool mbTimeBased;
    SCTAB mnTimeBasedStart;
    SCTAB mnTimeBasedEnd;
    SCTAB mnCurrentTab;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
