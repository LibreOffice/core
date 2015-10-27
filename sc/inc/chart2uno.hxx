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

#ifndef INCLUDED_SC_INC_CHART2UNO_HXX
#define INCLUDED_SC_INC_CHART2UNO_HXX

#include "cellsuno.hxx"
#include "rangelst.hxx"
#include "externalrefmgr.hxx"
#include "types.hxx"
#include "chartlis.hxx"

#include <svl/lstner.hxx>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XSheetDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/chart2/XTimeBased.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include <list>
#include <memory>
#include <unordered_set>
#include <vector>
#include <boost/noncopyable.hpp>

class ScDocument;

// DataProvider
class ScChart2DataProvider : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataProvider,
                    css::chart2::data::XSheetDataProvider,
                    css::chart2::data::XRangeXMLConversion,
                    css::beans::XPropertySet,
                    css::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataProvider( ScDocument* pDoc );
    virtual ~ScChart2DataProvider();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataProvider
    virtual sal_Bool SAL_CALL createDataSourcePossible(
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSource > SAL_CALL createDataSource(
            const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<
        css::beans::PropertyValue > SAL_CALL detectArguments(
            const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource )
            throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
            const OUString& aRangeRepresentation )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection()
        throw (css::uno::RuntimeException, std::exception) override;

    // XSheetDataProvider
    virtual sal_Bool SAL_CALL createDataSequenceByFormulaTokensPossible(
        const css::uno::Sequence< css::sheet::FormulaToken >& aTokens )
            throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::chart2::data::XDataSequence >
        SAL_CALL createDataSequenceByFormulaTokens(
            const css::uno::Sequence< css::sheet::FormulaToken >& aTokens )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XRangeXMLConversion
    virtual OUString SAL_CALL convertRangeToXML( const OUString& sRangeRepresentation )
        throw ( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) override;

    virtual OUString SAL_CALL convertRangeFromXML( const OUString& sXMLRange )
        throw ( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw( css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& xListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw(
                css::uno::RuntimeException, std::exception) override;

private:

    ScDocument*                 m_pDocument;
    SfxItemPropertySet          m_aPropSet;
    bool                        m_bIncludeHiddenCells;
};

// DataSource
class ScChart2DataSource : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataSource,
                    css::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataSource( ScDocument* pDoc);
    virtual ~ScChart2DataSource();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataSource
    virtual css::uno::Sequence< css::uno::Reference<
            css::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences() throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw(
                css::uno::RuntimeException, std::exception) override;

    // implementation

    void AddLabeledSequence(const css::uno::Reference < css::chart2::data::XLabeledDataSequence >& xNew);

private:

    ScDocument*                 m_pDocument;
    typedef std::list < css::uno::Reference< css::chart2::data::XLabeledDataSequence > >  LabeledList;
    LabeledList                 m_aLabeledSequences;

};

// DataSequence
class ScChart2DataSequence : public
                ::cppu::WeakImplHelper<
                    css::chart2::data::XDataSequence,
                    css::chart2::data::XTextualDataSequence,
                    css::chart2::data::XNumericalDataSequence,
                    css::chart2::XTimeBased,
                    css::util::XCloneable,
                    css::util::XModifyBroadcaster,
                    css::beans::XPropertySet,
                    css::lang::XServiceInfo>,
                SfxListener,
                boost::noncopyable
{
public:
    explicit ScChart2DataSequence( ScDocument* pDoc,
            const css::uno::Reference< css::chart2::data::XDataProvider >& xDP,
            ::std::vector<ScTokenRef>&& rTokens, bool bIncludeHiddenCells );

    virtual ~ScChart2DataSequence();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XDataSequence
    virtual css::uno::Sequence< css::uno::Any >
        SAL_CALL getData()
            throw (css::uno::RuntimeException,
                   std::exception) override;
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString >
        SAL_CALL generateLabel(css::chart2::data::LabelOrigin nOrigin)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Get the number format key for the n-th data entry
     * If nIndex == -1, then you will get the number format key for the first non-empty entry
     */
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    // XNumericalDataSequence
    virtual css::uno::Sequence< double >
        SAL_CALL getNumericalData()
            throw (css::uno::RuntimeException,
                   std::exception) override;

    // XTextualDataSequence
    virtual css::uno::Sequence< OUString >
        SAL_CALL getTextualData()
            throw (css::uno::RuntimeException,
                   std::exception) override;

    // XTimeBased
    virtual sal_Bool SAL_CALL switchToNext(sal_Bool bWrap) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setToPointInTime(sal_Int32 nPoint) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setRange(sal_Int32 nStart, sal_Int32 nEnd) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference<
        css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw( css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw(css::beans::UnknownPropertyException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException,
              std::exception) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& xListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<  css::beans::XVetoableChangeListener>& rListener)
        throw( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw(
                css::uno::RuntimeException, std::exception) override;

private:
    void setDataChangedHint(bool b);

    // Implementation
    void    RefChanged();
    DECL_LINK_TYPED( ValueListenerHdl, const SfxHint&, void );

private:
    ScChart2DataSequence(const ScChart2DataSequence& r) = delete;

    class ExternalRefListener : public ScExternalRefManager::LinkListener
    {
    public:
        ExternalRefListener(ScChart2DataSequence& rParent, ScDocument* pDoc);
        virtual ~ExternalRefListener();
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) override;
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        const std::unordered_set<sal_uInt16>& getAllFileIds() { return maFileIds;}

    private:
        ExternalRefListener(const ExternalRefListener& r) = delete;

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

    class HiddenRangeListener : public ScChartHiddenRangeListener
    {
    public:
        HiddenRangeListener(ScChart2DataSequence& rParent);
        virtual ~HiddenRangeListener();

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
    typedef std::unique_ptr<ExternalRefListener>       ExtRefListenerPtr;

    sal_Int64                   m_nObjectId;
    ScDocument*                 m_pDocument;
    std::vector<ScTokenRef>     m_aTokens;
    RangeIndexMapPtr            m_pRangeIndices;
    ExtRefListenerPtr           m_pExtRefListener;
    css::uno::Reference < css::chart2::data::XDataProvider > m_xDataProvider;
    SfxItemPropertySet          m_aPropSet;

    std::unique_ptr<HiddenRangeListener> m_pHiddenListener;

    ScLinkListener*             m_pValueListener;
    XModifyListenerArr_Impl     m_aValueListeners;

    bool                        m_bGotDataChangedHint;
    bool                        m_bExtDataRebuildQueued;

    bool mbTimeBased;
    SCTAB mnTimeBasedStart;
    SCTAB mnTimeBasedEnd;
    SCTAB mnCurrentTab;

};

#endif // INCLUDED_SC_INC_CHART2UNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
