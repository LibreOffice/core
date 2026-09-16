/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>


#include <svl/lstner.hxx>
#include <svl/itemprop.hxx>
#include <cppuhelper/implbase.hxx>

#include <limits>
#include <utility>

class ScDocument;

namespace sc
{

enum class ValueType
{
    Empty,
    String,
    Numeric
};

struct ValueAndFormat
{
    double m_fValue;
    OUString m_aString;
    ValueType m_eType;
    sal_uInt32 m_nNumberFormat;

    explicit ValueAndFormat()
        : m_fValue(std::numeric_limits<double>::quiet_NaN())
        , m_aString()
        , m_eType(ValueType::Empty)
        , m_nNumberFormat(0)
    {
    }

    explicit ValueAndFormat(double fValue, sal_uInt32 nNumberFormat)
        : m_fValue(fValue)
        , m_aString()
        , m_eType(ValueType::Numeric)
        , m_nNumberFormat(nNumberFormat)
    {}

    explicit ValueAndFormat(OUString aString)
        : m_fValue(std::numeric_limits<double>::quiet_NaN())
        , m_aString(std::move(aString))
        , m_eType(ValueType::String)
        , m_nNumberFormat(0)
    {
    }
};

typedef cppu::WeakImplHelper<css::chart2::data::XDataSequence,
                             css::chart2::data::XTextualDataSequence,
                             css::chart2::data::XNumericalDataSequence,
                             css::util::XCloneable,
                             css::util::XModifyBroadcaster,
                             css::beans::XPropertySet,
                             css::lang::XServiceInfo>
        PivotTableDataSequence_Base;

class PivotTableDataSequence final : public PivotTableDataSequence_Base, public SfxListener
{
public:
    explicit PivotTableDataSequence(ScDocument* pDocument,
                                    OUString sID, std::vector<ValueAndFormat>&& rData);

    virtual ~PivotTableDataSequence() override;
    PivotTableDataSequence(const PivotTableDataSequence&) = delete;
    PivotTableDataSequence& operator=(const PivotTableDataSequence&) = delete;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataSequence
    virtual cpo::uno::Sequence<cpo::uno::Any> getData() override;
    virtual OUString getSourceRangeRepresentation() override;
    virtual cpo::uno::Sequence<OUString>
        generateLabel(css::chart2::data::LabelOrigin nOrigin) override;

    virtual sal_Int32 getNumberFormatKeyByIndex(sal_Int32 nIndex) override;

    // XNumericalDataSequence
    virtual cpo::uno::Sequence<double> getNumericalData() override;

    // XTextualDataSequence
    virtual cpo::uno::Sequence<OUString> getTextualData() override;

    // XPropertySet
    virtual cpo::uno::Reference<css::beans::XPropertySetInfo>
        getPropertySetInfo() override;

    virtual void setPropertyValue(const OUString& rPropertyName,
                                           const cpo::uno::Any& rValue) override;

    virtual cpo::uno::Any getPropertyValue(const OUString& rPropertyName) override;

    virtual void addPropertyChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener>& xListener) override;

    virtual void removePropertyChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener>& rListener) override;

    virtual void addVetoableChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    virtual void removeVetoableChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference<  css::beans::XVetoableChangeListener>& rListener) override;

    // XCloneable
    virtual cpo::uno::Reference<css::util::XCloneable> createClone() override;

    // XModifyBroadcaster
    virtual void addModifyListener(
        const cpo::uno::Reference<css::util::XModifyListener>& aListener) override;

    virtual void removeModifyListener(
        const cpo::uno::Reference<css::util::XModifyListener>& aListener) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool supportsService(const OUString& rServiceName) override;

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // Other

    void setRole(css::chart2::data::DataSequenceRole const & aRole)
    {
        m_aRole = aRole;
    }

private:
    ScDocument* m_pDocument;
    OUString m_aID;
    std::vector<ValueAndFormat> m_aData;
    SfxItemPropertySet m_aPropSet;
    css::chart2::data::DataSequenceRole m_aRole;
    std::vector<cpo::uno::Reference<css::util::XModifyListener>> m_aValueListeners;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
