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

#ifndef INC_SCCHART2DATASEQUENCE_HXX_
#define INC_SCCHART2DATASEQUENCE_HXX_

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/chart2/data/DataSequenceRole.hpp"
#include "com/sun/star/chart2/data/XDataProvider.hpp"
#include "com/sun/star/chart2/data/XDataSequence.hpp"
#include "com/sun/star/chart2/data/XTextualDataSequence.hpp"
#include "com/sun/star/chart2/data/XNumericalDataSequence.hpp"
#include "com/sun/star/chart2/XTimeBased.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Sequence.h"
#include "com/sun/star/util/XCloneable.hpp"
#include "com/sun/star/util/XModifyBroadcaster.hpp"
#include "cppuhelper/implbase8.hxx"
#include "rtl/ustring.hxx"
#include "svl/itemprop.hxx"
#include "svl/lstner.hxx"
#include "tools/link.hxx"

#include "cellsuno.hxx"
#include "chartlis.hxx"
#include "externalrefmgr.hxx"

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <unordered_set>

class ScDocument;
class ScLinkListener;

// DataSequence
class ScChart2DataSequence : public
                ::cppu::WeakImplHelper8<
                    ::com::sun::star::chart2::data::XDataSequence,
                    ::com::sun::star::chart2::data::XTextualDataSequence,
                    ::com::sun::star::chart2::data::XNumericalDataSequence,
                    com::sun::star::chart2::XTimeBased,
                    ::com::sun::star::util::XCloneable,
                    ::com::sun::star::util::XModifyBroadcaster,
                    ::com::sun::star::beans::XPropertySet,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener,
                boost::noncopyable
{
public:
    explicit ScChart2DataSequence( ScDocument* pDoc,
            const com::sun::star::uno::Reference< com::sun::star::chart2::data::XDataProvider >& xDP,
            ::std::vector<ScTokenRef>* pTokens, bool bIncludeHiddenCells );

    virtual ~ScChart2DataSequence();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    // XDataSequence
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getData()
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString >
        SAL_CALL generateLabel(::com::sun::star::chart2::data::LabelOrigin nOrigin)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Get the number format key for the n-th data entry
     * If nIndex == -1, then you will get the number format key for the first non-empty entry */
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;

    // XNumericalDataSequence
    virtual ::com::sun::star::uno::Sequence< double >
        SAL_CALL getNumericalData()
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE;

    // XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< OUString >
        SAL_CALL getTextualData()
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE;

    // XTimeBased
    virtual sal_Bool SAL_CALL switchToNext(sal_Bool bWrap) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setToPointInTime(sal_Int32 nPoint) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setRange(sal_Int32 nStart, sal_Int32 nEnd) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw(::com::sun::star::beans::UnknownPropertyException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException,
              std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static void convertTokensToString(OUString& rStr, const std::vector<ScTokenRef>& rTokens, ScDocument* pDoc);

private:
    void setDataChangedHint(bool b);

    // Implementation
    void    RefChanged();
    DECL_LINK( ValueListenerHdl, SfxHint* );

private:
    ScChart2DataSequence(const ScChart2DataSequence& r) SAL_DELETED_FUNCTION;

    class ExternalRefListener : public ScExternalRefManager::LinkListener
    {
    public:
        ExternalRefListener(ScChart2DataSequence& rParent, ScDocument* pDoc);
        virtual ~ExternalRefListener();
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) SAL_OVERRIDE;
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        const std::unordered_set<sal_uInt16>& getAllFileIds() { return maFileIds;}

    private:
        ExternalRefListener(const ExternalRefListener& r) SAL_DELETED_FUNCTION;

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

    /** Call this method to ensure that mAddressByIndex is filled */
    void EnsureAddressByIndexIsFilled();

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
        Item();
    };

    class HiddenRangeListener : public ScChartHiddenRangeListener
    {
    public:
        HiddenRangeListener(ScChart2DataSequence& rParent);
        virtual ~HiddenRangeListener();

        virtual void notify() SAL_OVERRIDE;

    private:
        ScChart2DataSequence& mrParent;
    };

    ::std::list<Item>           m_aDataArray;

    /** This vector contains the addresses of data elements by their index.
     * It is used to make the lookups in getNumberFormatKeyByIndex() faster.
     * The address at position n will either be the n-th data element, or the n-th non-hidden data element, depending
     * on whether m_bIncludeHiddenCells is true or not.
     */
    std::vector<ScAddress> mAddressByIndex;

    /**
     * Cached data for getData.  We may also need to cache data for the
     * numerical and textural data series if they turn out to be bottlenecks
     * under certain scenarios.
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > m_aMixedDataCache;

    ::com::sun::star::uno::Sequence<sal_Int32>  m_aHiddenValues;

    // properties
    ::com::sun::star::chart2::data::DataSequenceRole  m_aRole;
    bool                        m_bIncludeHiddenCells;

    // internals
    typedef boost::scoped_ptr<std::vector<ScTokenRef> >  TokenListPtr;
    typedef boost::scoped_ptr<std::vector<sal_uInt32> >  RangeIndexMapPtr;
    typedef boost::scoped_ptr<ExternalRefListener>       ExtRefListenerPtr;

    sal_Int64                   m_nObjectId;
    ScDocument*                 m_pDocument;
    TokenListPtr                m_pTokens;
    RangeIndexMapPtr            m_pRangeIndices;
    ExtRefListenerPtr           m_pExtRefListener;
    com::sun::star::uno::Reference < com::sun::star::chart2::data::XDataProvider > m_xDataProvider;
    SfxItemPropertySet          m_aPropSet;

    boost::scoped_ptr<HiddenRangeListener> m_pHiddenListener;

    ScLinkListener*             m_pValueListener;
    XModifyListenerArr_Impl     m_aValueListeners;

    bool                        m_bGotDataChangedHint;
    bool                        m_bExtDataRebuildQueued;

    bool mbTimeBased;
    SCTAB mnTimeBasedStart;
    SCTAB mnTimeBasedEnd;
    SCTAB mnCurrentTab;
};




#endif /* INC_SCCHART2DATASEQUENCE_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
