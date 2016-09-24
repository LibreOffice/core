/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "address.hxx"
#include "chartlis.hxx"

namespace {

const ScRange RANGE_1(10, 10, 0, 19, 10, 0);
const ScRange RANGE_2(20, 10, 0, 29, 10, 0);

const ScRange RANGE_INTERSECTING_1_AND_2(10, 10, 0, 29, 10, 0);


class ChartListenerCollectionTest : public CppUnit::TestFixture {

    void ListenerGetsNotifiedWhenItsRangeIsSetDirty();
    void ListenerGetsNotifiedTwiceWhenRegisteredTwoTimes();
    void ListenerDoesNotGetNotifiedWhenListeningStops();
    void ListenerStopsListeningForAllRanges();
    void ListenersStopListeningIdependently();

    CPPUNIT_TEST_SUITE(ChartListenerCollectionTest);

    CPPUNIT_TEST(ListenerGetsNotifiedWhenItsRangeIsSetDirty);
    CPPUNIT_TEST(ListenerGetsNotifiedTwiceWhenRegisteredTwoTimes);
    CPPUNIT_TEST(ListenerDoesNotGetNotifiedWhenListeningStops);
    CPPUNIT_TEST(ListenerStopsListeningForAllRanges);
    CPPUNIT_TEST(ListenersStopListeningIdependently);

    CPPUNIT_TEST_SUITE_END();

};

struct MockedHiddenRangeListener : public ScChartHiddenRangeListener {
    unsigned mNotifyCount;
    MockedHiddenRangeListener()
    : mNotifyCount(0) {
    }

    void notify() override {
        mNotifyCount++;
    }
};

void ChartListenerCollectionTest::ListenerGetsNotifiedWhenItsRangeIsSetDirty() {
    MockedHiddenRangeListener listener;
    ScChartListenerCollection sut(nullptr);

    sut.StartListeningHiddenRange(RANGE_1, &listener);
    sut.SetRangeDirty(RANGE_INTERSECTING_1_AND_2);

    CPPUNIT_ASSERT_EQUAL(1u, listener.mNotifyCount);
}

void ChartListenerCollectionTest::ListenerGetsNotifiedTwiceWhenRegisteredTwoTimes() {
    MockedHiddenRangeListener listener;
    ScChartListenerCollection sut(nullptr);

    sut.StartListeningHiddenRange(RANGE_1, &listener);
    sut.StartListeningHiddenRange(RANGE_2, &listener);
    sut.SetRangeDirty(RANGE_INTERSECTING_1_AND_2);

    CPPUNIT_ASSERT_EQUAL(2u, listener.mNotifyCount);
}

void ChartListenerCollectionTest::ListenerDoesNotGetNotifiedWhenListeningStops() {
    MockedHiddenRangeListener listener;
    ScChartListenerCollection sut(nullptr);
    sut.StartListeningHiddenRange(RANGE_1, &listener);

    sut.EndListeningHiddenRange(&listener);
    sut.SetRangeDirty(RANGE_INTERSECTING_1_AND_2);

    CPPUNIT_ASSERT_EQUAL(0u, listener.mNotifyCount);

}

void ChartListenerCollectionTest::ListenerStopsListeningForAllRanges() {
    MockedHiddenRangeListener listener;
    ScChartListenerCollection sut(nullptr);
    sut.StartListeningHiddenRange(RANGE_1, &listener);
    sut.StartListeningHiddenRange(RANGE_2, &listener);

    sut.EndListeningHiddenRange(&listener);
    sut.SetRangeDirty(RANGE_INTERSECTING_1_AND_2);

    CPPUNIT_ASSERT_EQUAL(0u, listener.mNotifyCount);
}

void ChartListenerCollectionTest::ListenersStopListeningIdependently() {
    MockedHiddenRangeListener listener1;
    MockedHiddenRangeListener listener2;

    ScChartListenerCollection sut(nullptr);
    sut.StartListeningHiddenRange(RANGE_1, &listener1);
    sut.StartListeningHiddenRange(RANGE_2, &listener2);

    sut.EndListeningHiddenRange(&listener1);
    sut.SetRangeDirty(RANGE_INTERSECTING_1_AND_2);

    CPPUNIT_ASSERT_EQUAL(0u, listener1.mNotifyCount);
    CPPUNIT_ASSERT_EQUAL(1u, listener2.mNotifyCount);
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(ChartListenerCollectionTest);


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
