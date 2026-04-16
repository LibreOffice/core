/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <HistogramCalculator.hxx>
#include <HistogramDataSequence.hxx>

#include <algorithm>
#include <cmath>
#include <CommonFunctors.hxx>
#include <ModifyListenerHelper.hxx>
#include <utility>

#include <cppuhelper/implbase.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace css;

namespace
{
constexpr OUString lcl_aServiceName = u"com.sun.star.chart2.data.HistogramDataSequence"_ustr;

enum
{
    PROP_PROPOSED_ROLE
};

} // anonymous namespace

namespace chart
{
HistogramDataSequence::HistogramDataSequence(
    const uno::Reference<chart2::data::XDataSequence>& xRawData, bool bIsCategory,
    sal_Int32 nFrequencyType, double fBinWidth, sal_Int32 nBinCount)
    : ::comphelper::OPropertyContainer(GetBroadcastHelper())
    , m_xModifyEventForwarder(new ModifyEventForwarder())
    , m_xRawData(xRawData)
    , m_bIsCategory(bIsCategory)
    , m_bIsDirty(true)
    , m_nFrequencyType(nFrequencyType)
    , m_fBinWidth(fBinWidth)
    , m_nBinCount(nBinCount)
{
    registerProperty(u"Role"_ustr, PROP_PROPOSED_ROLE,
                     0, // PropertyAttributes
                     &m_sRole, cppu::UnoType<decltype(m_sRole)>::get());

    if (m_xRawData.is())
    {
        // Listen for changes in the raw spreadsheet data
        uno::Reference<util::XModifyBroadcaster> xBroadcaster(m_xRawData, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addModifyListener(this);
    }
}

HistogramDataSequence::~HistogramDataSequence()
{
    if (m_xRawData.is())
    {
        uno::Reference<util::XModifyBroadcaster> xBroadcaster(m_xRawData, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeModifyListener(this);
    }
}

IMPLEMENT_FORWARD_XINTERFACE2(HistogramDataSequence, HistogramDataSequence_Base,
                              ::comphelper::OPropertyContainer)

IMPLEMENT_FORWARD_XTYPEPROVIDER2(HistogramDataSequence, HistogramDataSequence_Base,
                                 ::comphelper::OPropertyContainer)

uno::Reference<beans::XPropertySetInfo> SAL_CALL HistogramDataSequence::getPropertySetInfo()
{
    return comphelper::OPropertyStateHelper::createPropertySetInfo(getInfoHelper());
}

::cppu::IPropertyArrayHelper& SAL_CALL HistogramDataSequence::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* HistogramDataSequence::createArrayHelper() const
{
    uno::Sequence<beans::Property> aProperties;
    describeProperties(aProperties);
    return new ::cppu::OPropertyArrayHelper(aProperties);
}

OUString SAL_CALL HistogramDataSequence::getImplementationName() { return lcl_aServiceName; }

sal_Bool SAL_CALL HistogramDataSequence::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL HistogramDataSequence::getSupportedServiceNames()
{
    return { lcl_aServiceName, u"com.sun.star.chart2.data.DataSequence"_ustr,
             u"com.sun.star.chart2.data.NumericalDataSequence"_ustr,
             u"com.sun.star.chart2.data.TextualDataSequence"_ustr };
}

uno::Sequence<double> SAL_CALL HistogramDataSequence::getNumericalData()
{
    ::osl::MutexGuard aGuard(GetMutex());
    ensureCalculated();
    return CommonFunctors::convertToSequence(mxValues, CommonFunctors::ToDouble());
}

uno::Sequence<OUString> SAL_CALL HistogramDataSequence::getTextualData()
{
    ::osl::MutexGuard aGuard(GetMutex());
    ensureCalculated();
    return CommonFunctors::convertToSequence(mxValues, CommonFunctors::ToString());
}

uno::Sequence<uno::Any> SAL_CALL HistogramDataSequence::getData()
{
    ::osl::MutexGuard aGuard(GetMutex());
    ensureCalculated();
    return mxValues;
}

OUString SAL_CALL HistogramDataSequence::getSourceRangeRepresentation()
{
    // synthesized sequence — no underlying cell range
    return OUString();
}

uno::Sequence<OUString>
    SAL_CALL HistogramDataSequence::generateLabel(css::chart2::data::LabelOrigin)
{
    ::osl::MutexGuard aGuard(GetMutex());
    return mxLabels;
}

sal_Int32 SAL_CALL HistogramDataSequence::getNumberFormatKeyByIndex(sal_Int32)
{
    return 0; // TODO - NumberFormat support
}

void SAL_CALL
HistogramDataSequence::addModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    m_xModifyEventForwarder->addModifyListener(aListener);
}

void SAL_CALL
HistogramDataSequence::removeModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    m_xModifyEventForwarder->removeModifyListener(aListener);
}

void SAL_CALL HistogramDataSequence::modified(const lang::EventObject& /* aEvent */)
{
    // The raw data changed! Mark ourselves dirty and tell the chart to redraw
    {
        ::osl::MutexGuard aGuard(GetMutex());
        m_bIsDirty = true;
    }
    m_xModifyEventForwarder->modified(lang::EventObject(static_cast<cppu::OWeakObject*>(this)));
}

void SAL_CALL HistogramDataSequence::disposing(const lang::EventObject& /* Source */)
{
    m_xRawData.clear();
}

uno::Reference<util::XCloneable> SAL_CALL HistogramDataSequence::createClone()
{
    rtl::Reference<HistogramDataSequence> pClone(new HistogramDataSequence(
        m_xRawData, m_bIsCategory, m_nFrequencyType, m_fBinWidth, m_nBinCount));

    ::osl::MutexGuard aGuard(GetMutex());
    pClone->mxLabels = mxLabels;
    pClone->mxValues = mxValues;
    pClone->m_sRole = m_sRole;
    pClone->m_bIsDirty = m_bIsDirty;

    return pClone;
}

void HistogramDataSequence::ensureCalculated()
{
    if (!m_bIsDirty || !m_xRawData.is())
        return;

    std::vector<double> rawData;
    try
    {
        uno::Sequence<uno::Any> aRawAnyValues = m_xRawData->getData();
        for (const auto& aAny : aRawAnyValues)
        {
            double fValue = 0.0;
            if (aAny >>= fValue)
                rawData.push_back(fValue);
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    HistogramCalculator aCalculator;
    aCalculator.computeBinFrequencyHistogram(rawData, m_nFrequencyType, m_fBinWidth, m_nBinCount);

    std::vector<uno::Any> aNewValues;

    if (m_bIsCategory)
    {
        const auto& binRanges = aCalculator.getBinRanges();

        // keep ~3 significant figures relative to the bin width, so typical
        // widths (~1) get 2 decimals and small widths (~0.002) get 5
        int labelDigits = 2;
        if (!binRanges.empty())
        {
            double firstBinWidth = binRanges.front().second - binRanges.front().first;
            if (firstBinWidth > 0.0)
                labelDigits
                    = std::max(1, 2 - static_cast<int>(std::floor(std::log10(firstBinWidth))));
        }
        const double roundingScale = std::pow(10.0, labelDigits);
        auto formatBoundary = [roundingScale](double value) {
            return OUString::number(std::round(value * roundingScale) / roundingScale);
        };

        for (size_t i = 0; i < binRanges.size(); ++i)
        {
            OUString aLabel = (i == 0) ? u"["_ustr + formatBoundary(binRanges[i].first) + u"-"_ustr
                                             + formatBoundary(binRanges[i].second) + u"]"_ustr
                                       : u"("_ustr + formatBoundary(binRanges[i].first) + u"-"_ustr
                                             + formatBoundary(binRanges[i].second) + u"]"_ustr;
            aNewValues.push_back(uno::Any(aLabel));
        }
    }
    else
    {
        const auto& binFrequencies = aCalculator.getBinFrequencies();
        for (sal_Int32 freq : binFrequencies)
        {
            aNewValues.push_back(uno::Any(static_cast<double>(freq)));
        }
    }

    mxValues = comphelper::containerToSequence(aNewValues);
    m_bIsDirty = false;
}

} // namespace chart2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */