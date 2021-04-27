/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/profilezone.hxx>
#include <comphelper/traceevent.hxx>

#include <rtl/ustring.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TestTraceEvent : public CppUnit::TestFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(TestTraceEvent);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
void trace_event_test()
{
    // When we start recording is off and this will not generate any 'X' event
    comphelper::ProfileZone aZone0("test().0");

    // This will not generate any 'b' and 'e' events either
    auto pAsync1(std::make_shared<comphelper::AsyncEvent>("async 1"));

    std::weak_ptr<comphelper::AsyncEvent> pAsync2;
    {
        // No 'X' by this either
        comphelper::ProfileZone aZone1("block 1");

        // Now we turn on recording
        comphelper::TraceEvent::startRecording();

        // As this is nested in the parent that was created with recording turned off,
        // this will not generate any 'b' and 'e' events either
        pAsync2 = comphelper::AsyncEvent::createWithParent("async 2", pAsync1);
    }

    // This will generate an 'i' event
    comphelper::TraceEvent::addInstantEvent("instant 1");

    std::shared_ptr<comphelper::AsyncEvent> pAsync25;
    {
        comphelper::ProfileZone aZone2("block 2");

        // This does not generate any 'e' event as it was created when recording was off
        // And the nested "async 2" object will thus not generate anything either
        pAsync1.reset();

        // This will generate 'b' event and an 'e' event when the pointer is reset or goes out of scope
        pAsync25 = std::make_shared<comphelper::AsyncEvent>("async 2.5");

        // Leaving this scope will generate an 'X' event for "block 2"
    }

    // Verify that the weak_ptr to pAsync2 has expired as its parent pAsync1 has been finished off
    CPPUNIT_ASSERT(pAsync2.expired());

    // This will generate a 'b' event
    auto pAsync3(std::make_shared<comphelper::AsyncEvent>("async 3"));

    std::weak_ptr<comphelper::AsyncEvent> pAsync4;
    {
        comphelper::ProfileZone aZone3("block 3");

        pAsync4 = comphelper::AsyncEvent::createWithParent("async 4", pAsync3);

        // Leaving this scope will generate an 'X' event for "block 3"
    }

    // This will generate an 'e' event for "async 2.5"
    pAsync25.reset();

    comphelper::ProfileZone aZone4("test().2");

    // This will generate an 'i' event
    comphelper::TraceEvent::addInstantEvent("instant 2");

    // Leaving this scope will generate 'X' events for "test().2" and a 'e' event for pAsync4 and pAsync3
}
}

void TestTraceEvent::test()
{
    trace_event_test();
    auto aEvents = comphelper::TraceEvent::getEventVectorAndClear();
    for (const auto& s : aEvents)
    {
        std::cerr << s << "\n";
    }

    CPPUNIT_ASSERT(aEvents[0].startsWith("{\"name:\"instant 1\",\"ph\":\"i\","));
    CPPUNIT_ASSERT(aEvents[1].startsWith("{\"name\":\"async 2.5\",\"ph\":\"b\",\"id\":1\","));
    CPPUNIT_ASSERT(aEvents[2].startsWith("{\"name\":\"block 2\",\"ph\":\"X\","));
    CPPUNIT_ASSERT(aEvents[3].startsWith("{\"name\":\"async 3\",\"ph\":\"b\",\"id\":2\","));
    CPPUNIT_ASSERT(aEvents[4].startsWith("{\"name\":\"async 4\",\"ph\":\"b\",\"id\":2\","));
    CPPUNIT_ASSERT(aEvents[5].startsWith("{\"name\":\"block 3\",\"ph\":\"X\","));
    CPPUNIT_ASSERT(aEvents[6].startsWith("{\"name\":\"async 2.5\",\"ph\":\"e\",\"id\":1\","));
    CPPUNIT_ASSERT(aEvents[7].startsWith("{\"name:\"instant 2\",\"ph\":\"i\","));
    CPPUNIT_ASSERT(aEvents[8].startsWith("{\"name\":\"test().2\",\"ph\":\"X\""));
    CPPUNIT_ASSERT(aEvents[9].startsWith("{\"name\":\"async 4\",\"ph\":\"e\",\"id\":2\","));
    CPPUNIT_ASSERT(aEvents[10].startsWith("{\"name\":\"async 3\",\"ph\":\"e\",\"id\":2\","));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestTraceEvent);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
