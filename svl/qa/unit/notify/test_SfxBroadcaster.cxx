/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/SfxBroadcaster.hxx>

#include <svl/lstner.hxx>
#include <svl/hint.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class SfxBroadcasterTest : public CppUnit::TestFixture
{
    void AddingListenersIncreasesCount();
    void RemovingListenersDecreasesCount();
    void HintsAreNotForwardedToRemovedListeners();
    void SameListenerCanBeAddedMoreThanOnce();
    void StoppingListeningAffectsOnlyFirstOfIdenticalListeners();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(SfxBroadcasterTest);
    CPPUNIT_TEST(AddingListenersIncreasesCount);
    CPPUNIT_TEST(RemovingListenersDecreasesCount);
    CPPUNIT_TEST(HintsAreNotForwardedToRemovedListeners);
    CPPUNIT_TEST(SameListenerCanBeAddedMoreThanOnce);
    CPPUNIT_TEST(StoppingListeningAffectsOnlyFirstOfIdenticalListeners);

    CPPUNIT_TEST_SUITE_END();
};

class MockedSfxListener : public SfxListener
{
public:
    MockedSfxListener()
    : mNotifyWasCalled(false) {}

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override {
        (void)(rBC); (void)(rHint); // avoid warnings about unused parameters
        mNotifyWasCalled = true;
    }

    bool NotifyWasCalled() const {
        return mNotifyWasCalled;
    }

private:
    bool mNotifyWasCalled;
};

void
SfxBroadcasterTest::AddingListenersIncreasesCount()
{
    SfxBroadcaster sb;
    MockedSfxListener sl;

    CPPUNIT_ASSERT_EQUAL((size_t)0, sb.GetListenerCount());

    sl.StartListening(sb, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, sb.GetListenerCount());
}

void
SfxBroadcasterTest::RemovingListenersDecreasesCount()
{
    SfxBroadcaster sb;
    MockedSfxListener sl;

    CPPUNIT_ASSERT_EQUAL((size_t)0, sb.GetListenerCount());
    sl.StartListening(sb, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, sb.GetListenerCount());
    sl.EndListening(sb, true);
    CPPUNIT_ASSERT_EQUAL((size_t)0, sb.GetListenerCount());
}

void
SfxBroadcasterTest::HintsAreNotForwardedToRemovedListeners()
{
    SfxBroadcaster sb;
    MockedSfxListener sl1;
    MockedSfxListener sl2;
    SfxHint hint;

    sl1.StartListening(sb, true);
    sl2.StartListening(sb, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All listeners were added.", (size_t)2, sb.GetListenerCount());
    sl1.EndListening(sb, true);
    sb.Forward(sb, hint);
    CPPUNIT_ASSERT_EQUAL(true, sl2.NotifyWasCalled());
    CPPUNIT_ASSERT_EQUAL(false, sl1.NotifyWasCalled());
}

void
SfxBroadcasterTest::SameListenerCanBeAddedMoreThanOnce()
{
    MockedSfxListener sl;
    SfxBroadcaster sb;
    sb.AddListener(sl);
    sb.AddListener(sl);
    CPPUNIT_ASSERT_EQUAL((size_t)2, sb.GetListenerCount());
}

void
SfxBroadcasterTest::StoppingListeningAffectsOnlyFirstOfIdenticalListeners()
{
    MockedSfxListener sl;
    SfxBroadcaster sb;
    sb.AddListener(sl);
    sb.AddListener(sl);
    sb.RemoveListener(sl);
    CPPUNIT_ASSERT_EQUAL((size_t)1, sb.GetListenerCount());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SfxBroadcasterTest);

CPPUNIT_PLUGIN_IMPLEMENT();
