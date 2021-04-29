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
    std::shared_ptr<comphelper::AsyncEvent> pAsync7Locked;

    {
        // When we start recording is off and this will not generate any 'X' event when we leave the scope
        comphelper::ProfileZone aZone0("test0");

        // This will not generate any 'b' and 'e' events either
        auto pAsync1(std::make_shared<comphelper::AsyncEvent>("async1"));

        std::weak_ptr<comphelper::AsyncEvent> pAsync2;
        {
            // No 'X' by this either
            comphelper::ProfileZone aZone1("block1");

            // Now we turn on recording
            comphelper::TraceEvent::startRecording();

            // As this is nested in the parent that was created with recording turned off,
            // this will not generate any 'b' and 'e' events either even if recording is now on.
            pAsync2 = comphelper::AsyncEvent::createWithParent("async2in1", pAsync1);
        }

        // This will generate an 'i' event for instant1
        comphelper::TraceEvent::addInstantEvent("instant1");

        std::shared_ptr<comphelper::AsyncEvent> pAsync25;
        {
            comphelper::ProfileZone aZone2("block2");

            // This does not generate any 'e' event as it was created when recording was off
            // And the nested async2 object will thus not generate anything either
            pAsync1.reset();

            // This will generate 'b' event and an 'e' event when the pointer is reset or goes out of scope
            pAsync25 = std::make_shared<comphelper::AsyncEvent>("async2.5");

            // Leaving this scope will generate an 'X' event for block2
        }

        // Verify that the weak_ptr to pAsync2 has expired as its parent pAsync1 has been finished off
        CPPUNIT_ASSERT(pAsync2.expired());

        // This will generate a 'b' event for async3
        auto pAsync3(std::make_shared<comphelper::AsyncEvent>("async3"));

        std::weak_ptr<comphelper::AsyncEvent> pAsync4;

        {
            comphelper::ProfileZone aZone3("block3");

            pAsync4 = comphelper::AsyncEvent::createWithParent("async4in3", pAsync3);

            // Leaving this scope will generate an 'X' event for block3
        }

        // This will generate an 'e' event for async2.5
        pAsync25.reset();

        comphelper::ProfileZone aZone4("test2");

        // This will generate an 'i' event for instant2"
        comphelper::TraceEvent::addInstantEvent("instant2");

        std::weak_ptr<comphelper::AsyncEvent> pAsync5;
        {
            auto pAsync4Locked = pAsync4.lock();
            CPPUNIT_ASSERT(pAsync4Locked);
            // This will generate a 'b' event for async5in4
            pAsync5 = comphelper::AsyncEvent::createWithParent("async5in4", pAsync4Locked);
        }

        CPPUNIT_ASSERT(!pAsync5.expired());

        // This will generate a 'b' event for async6in5
        std::weak_ptr<comphelper::AsyncEvent> pAsync6(
            comphelper::AsyncEvent::createWithParent("async6in5", pAsync5.lock()));
        CPPUNIT_ASSERT(!pAsync6.expired());

        // This will generate an 'e' event for async6in5 and async5in4
        pAsync5.lock()->finish();

        pAsync7Locked = comphelper::AsyncEvent::createWithParent("async7in3", pAsync3).lock();

        CPPUNIT_ASSERT(pAsync6.expired());

        // Leaving this scope will generate 'X' events for test2 and a
        // 'e' event for async4in3, async7in3, and async3. The
        // pAsync7Locked pointer will now point to an AsyncEvent
        // object that has already had its 'e' event generated.
    }

    // This incorrect use of overlapping (not nested) ProfileZones
    // will generate a SAL_WARN but should not crash
    auto p1 = new comphelper::ProfileZone("error1");
    auto p2 = new comphelper::ProfileZone("error2");
    delete p1;
    delete p2;

    // Nothing is generated from this
    pAsync7Locked.reset();
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

    CPPUNIT_ASSERT_EQUAL(17, static_cast<int>(aEvents.size()));

    CPPUNIT_ASSERT(aEvents[0].startsWith("{\"name:\"instant1\",\"ph\":\"i\","));
    CPPUNIT_ASSERT(aEvents[1].startsWith("{\"name\":\"async2.5\",\"ph\":\"b\",\"id\":1,"));
    CPPUNIT_ASSERT(aEvents[2].startsWith("{\"name\":\"block2\",\"ph\":\"X\","));
    CPPUNIT_ASSERT(aEvents[3].startsWith("{\"name\":\"async3\",\"ph\":\"b\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[4].startsWith("{\"name\":\"async4in3\",\"ph\":\"b\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[5].startsWith("{\"name\":\"block3\",\"ph\":\"X\","));
    CPPUNIT_ASSERT(aEvents[6].startsWith("{\"name\":\"async2.5\",\"ph\":\"e\",\"id\":1,"));
    CPPUNIT_ASSERT(aEvents[7].startsWith("{\"name:\"instant2\",\"ph\":\"i\","));
    CPPUNIT_ASSERT(aEvents[8].startsWith("{\"name\":\"async5in4\",\"ph\":\"b\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[9].startsWith("{\"name\":\"async6in5\",\"ph\":\"b\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[10].startsWith("{\"name\":\"async6in5\",\"ph\":\"e\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[11].startsWith("{\"name\":\"async5in4\",\"ph\":\"e\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[12].startsWith("{\"name\":\"async7in3\",\"ph\":\"b\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[13].startsWith("{\"name\":\"test2\",\"ph\":\"X\""));
    CPPUNIT_ASSERT(aEvents[14].startsWith("{\"name\":\"async4in3\",\"ph\":\"e\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[15].startsWith("{\"name\":\"async7in3\",\"ph\":\"e\",\"id\":2,"));
    CPPUNIT_ASSERT(aEvents[16].startsWith("{\"name\":\"async3\",\"ph\":\"e\",\"id\":2,"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestTraceEvent);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
