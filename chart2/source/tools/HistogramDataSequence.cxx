/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <HistogramDataSequence.hxx>

#include <CommonFunctors.hxx>
#include <ModifyListenerHelper.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <strings.hrc>
#include <ResId.hxx>
#include <utility>

using namespace css;

namespace
{
constexpr OUString lcl_aServiceName = u"com.sun.star.comp.chart.HistogramDataSequence"_ustr;

enum
{
    PROP_PROPOSED_ROLE
};

} // anonymous namespace

namespace chart
{
HistogramDataSequence::HistogramDataSequence()
    : m_xModifyEventForwarder(new ModifyEventForwarder())
{
    registerProperty(u"Role"_ustr, PROP_PROPOSED_ROLE,
                     0, // PropertyAttributes
                     &m_sRole, cppu::UnoType<decltype(m_sRole)>::get());
}

HistogramDataSequence::~HistogramDataSequence() {}

IMPLEMENT_FORWARD_XINTERFACE2(HistogramDataSequence, HistogramDataSequence_Base,
                              comphelper::OPropertyContainer2)

IMPLEMENT_FORWARD_XTYPEPROVIDER2(HistogramDataSequence, HistogramDataSequence_Base,
                                 comphelper::OPropertyContainer2)

// XPropertySet
uno::Reference<beans::XPropertySetInfo> SAL_CALL HistogramDataSequence::getPropertySetInfo()
{
    return createPropertySetInfo(getInfoHelper());
}

// comphelper::OPropertySetHelper
::cppu::IPropertyArrayHelper& HistogramDataSequence::getInfoHelper() { return *getArrayHelper(); }

// comphelper::OPropertyArrayHelper
::cppu::IPropertyArrayHelper* HistogramDataSequence::createArrayHelper() const
{
    uno::Sequence<beans::Property> aProperties;
    // describes all properties which have been registered in the ctor
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

// XNumericalDataSequence
uno::Sequence<double> SAL_CALL HistogramDataSequence::getNumericalData()
{
    std::unique_lock<std::mutex> aGuard;

    return mxValues;
}

// XTextualDataSequence
uno::Sequence<OUString> SAL_CALL HistogramDataSequence::getTextualData()
{
    std::unique_lock<std::mutex> aGuard;
    return {};
}

// XDataSequence
uno::Sequence<uno::Any> SAL_CALL HistogramDataSequence::getData()
{
    std::unique_lock<std::mutex> aGuard;
    uno::Sequence<uno::Any> aSequence(mxValues.size());
    auto pSequence = aSequence.getArray();

    size_t i = 0;
    for (double nValue : mxValues)
    {
        pSequence[i] <<= nValue;
        i++;
    }
    return aSequence;
}

OUString SAL_CALL HistogramDataSequence::getSourceRangeRepresentation() { return m_sRole; }

uno::Sequence<OUString>
    SAL_CALL HistogramDataSequence::generateLabel(css::chart2::data::LabelOrigin)
{
    return mxLabels;
}

sal_Int32 SAL_CALL HistogramDataSequence::getNumberFormatKeyByIndex(sal_Int32)
{
    return 0; // TODO - NumberFormat support
}

// XModifyBroadcaster
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

// XCloneable

uno::Reference<util::XCloneable> SAL_CALL HistogramDataSequence::createClone()
{
    rtl::Reference<HistogramDataSequence> pClone(new HistogramDataSequence());
    return pClone;
}

} // namespace chart2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
