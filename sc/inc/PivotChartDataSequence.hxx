/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_INC_PIVOTCHARTDATASEQUENCE_HXX
#define INCLUDED_SC_INC_PIVOTCHARTDATASEQUENCE_HXX

#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <com/sun/star/sheet/XDataPilotResults.hpp>

#include <svl/lstner.hxx>
#include <svl/itemprop.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/math.hxx>

#include "unonames.hxx"
#include "document.hxx"

#include "dpsave.hxx"

namespace sc
{

typedef cppu::WeakImplHelper<css::chart2::data::XDataSequence,
                             css::chart2::data::XTextualDataSequence,
                             css::chart2::data::XNumericalDataSequence,
                             css::util::XCloneable,
                             css::util::XModifyBroadcaster,
                             css::beans::XPropertySet,
                             css::lang::XServiceInfo>
        PivotChartDataSequence_Base;

struct PivotChartItem
{
    double m_fValue;
    OUString m_aString;
    bool m_bIsValue;

    explicit PivotChartItem()
        : m_fValue(0.0)
        , m_aString()
        , m_bIsValue(true)
    {
        rtl::math::setNan(&m_fValue);
    }

    explicit PivotChartItem(double fValue)
        : m_fValue(fValue)
        , m_aString()
        , m_bIsValue(true)
    {}

    explicit PivotChartItem(OUString const & rString)
        : m_fValue(0.0)
        , m_aString(rString)
        , m_bIsValue(false)
    {
        rtl::math::setNan(&m_fValue);
    }
};

class PivotChartDataSequence : public PivotChartDataSequence_Base, public SfxListener
{
public:
    explicit PivotChartDataSequence(ScDocument* pDocument, OUString const & sPivotTableName, OUString const & sID,
                                    std::vector<PivotChartItem> const & rData);

    virtual ~PivotChartDataSequence() override;
    PivotChartDataSequence(const PivotChartDataSequence&) = delete;
    PivotChartDataSequence& operator=(const PivotChartDataSequence&) = delete;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataSequence
    virtual css::uno::Sequence<css::uno::Any> SAL_CALL getData() override;
    virtual OUString SAL_CALL getSourceRangeRepresentation() override;
    virtual css::uno::Sequence<OUString> SAL_CALL
        generateLabel(css::chart2::data::LabelOrigin nOrigin) override;

    virtual sal_Int32 SAL_CALL getNumberFormatKeyByIndex(sal_Int32 nIndex) override;

    // XNumericalDataSequence
    virtual css::uno::Sequence<double> SAL_CALL getNumericalData() override;

    // XTextualDataSequence
    virtual css::uno::Sequence<OUString> SAL_CALL getTextualData() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue(const OUString& rPropertyName,
                                           const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& rPropertyName) override;

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
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // Other

    void setRole(css::chart2::data::DataSequenceRole aRole)
    {
        m_aRole = aRole;
    }

    void setShortSideLabels(css::uno::Sequence<OUString> const & rShortSideLabels)
    {
        m_aShortSideLabels = rShortSideLabels;
    }

    void setLongSideLabels(css::uno::Sequence<OUString> const & rLongSideLabels)
    {
        m_aLongSideLabels = rLongSideLabels;
    }

private:
    ScDocument* m_pDocument;
    OUString m_sPivotTableName;
    OUString m_aID;
    std::vector<PivotChartItem> m_aData;
    css::uno::Sequence<OUString> m_aShortSideLabels;
    css::uno::Sequence<OUString> m_aLongSideLabels;
    SfxItemPropertySet m_aPropSet;
    css::chart2::data::DataSequenceRole m_aRole;
    std::vector<css::uno::Reference<css::util::XModifyListener>> m_aValueListeners;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
