/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/chart/xchartdata.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
namespace
{
class MockedChartDataChangeEventListener
    : public ::cppu::WeakImplHelper<chart::XChartDataChangeEventListener>
{
public:
    MockedChartDataChangeEventListener()
        : m_bListenerCalled(false)
    {
    }

    bool m_bListenerCalled;
    virtual void SAL_CALL chartDataChanged(const chart::ChartDataChangeEvent& /* rEvent */) override
    {
        m_bListenerCalled = true;
    }

    virtual void SAL_CALL disposing(const lang::EventObject& /* xEvent */) override {}
};
}

void XChartData::testGetNotANumber()
{
    uno::Reference<chart::XChartData> xCD(init(), uno::UNO_QUERY_THROW);
    const double fNaN = xCD->getNotANumber();

    CPPUNIT_ASSERT(fNaN != 1.0);
}

void XChartData::testIsNotANumber()
{
    uno::Reference<chart::XChartData> xCD(init(), uno::UNO_QUERY_THROW);
    const double fNaN = xCD->getNotANumber();
    CPPUNIT_ASSERT(xCD->isNotANumber(fNaN));
    CPPUNIT_ASSERT(!xCD->isNotANumber(fNaN + 1.0));
}

void XChartData::testChartDataChangeEventListener()
{
    uno::Reference<chart::XChartData> xCD(init(), uno::UNO_QUERY_THROW);

    rtl::Reference<MockedChartDataChangeEventListener> pListener0
        = new MockedChartDataChangeEventListener();
    xCD->addChartDataChangeEventListener(
        uno::Reference<chart::XChartDataChangeEventListener>(pListener0));
    rtl::Reference<MockedChartDataChangeEventListener> pListener1
        = new MockedChartDataChangeEventListener();
    xCD->addChartDataChangeEventListener(
        uno::Reference<chart::XChartDataChangeEventListener>(pListener1));

    uno::Reference<chart::XChartDataArray> xCDD(xCD, uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Sequence<double>> aData = xCDD->getData();
    aData[0][0] += 1.0;
    xCDD->setData(aData);
    CPPUNIT_ASSERT(pListener0->m_bListenerCalled);
    CPPUNIT_ASSERT(pListener1->m_bListenerCalled);

    pListener0->m_bListenerCalled = false;
    pListener1->m_bListenerCalled = false;

    xCD->removeChartDataChangeEventListener(
        uno::Reference<chart::XChartDataChangeEventListener>(pListener1));
    aData[0][0] += 1.0;
    xCDD->setData(aData);
    CPPUNIT_ASSERT(pListener0->m_bListenerCalled);
    CPPUNIT_ASSERT(!pListener1->m_bListenerCalled);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
