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

#include <sfx2/documenttabbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/scheduler.hxx>
#include <rtl/ref.hxx>

#include "documenttabbar_mocks.hxx"

#include <memory>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <fstream>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/task.h>
#endif

using namespace sfx2::test;

namespace {

/**
 * Memory monitoring and leak detection utilities
 */
class MemoryMonitor
{
private:
    struct MemorySnapshot
    {
        size_t totalMemory;
        size_t vclObjects;
        size_t sfxObjects;
        std::chrono::high_resolution_clock::time_point timestamp;

        MemorySnapshot() : totalMemory(0), vclObjects(0), sfxObjects(0)
        {
            timestamp = std::chrono::high_resolution_clock::now();
        }
    };

    std::vector<MemorySnapshot> m_aSnapshots;

public:
    size_t GetCurrentMemoryUsage() const
    {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            return pmc.WorkingSetSize;
        }
#elif defined(__linux__)
        std::ifstream statm("/proc/self/statm");
        size_t size, resident, share, text, lib, data, dt;
        if (statm >> size >> resident >> share >> text >> lib >> data >> dt)
        {
            return resident * getpagesize();
        }
#elif defined(__APPLE__)
        task_basic_info info;
        mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), TASK_BASIC_INFO,
                     (task_info_t)&info, &infoCount) == KERN_SUCCESS)
        {
            return info.resident_size;
        }
#endif
        // Fallback for unsupported platforms
        return 0;
    }

    size_t GetVclObjectCount() const
    {
        // In a real implementation, this would query VCL's object tracking
        // For testing purposes, we'll use a simplified approach
        return Application::GetVclWindowCount();
    }

    void TakeSnapshot()
    {
        MemorySnapshot snapshot;
        snapshot.totalMemory = GetCurrentMemoryUsage();
        snapshot.vclObjects = GetVclObjectCount();
        // snapshot.sfxObjects would query SFX object registry
        m_aSnapshots.push_back(snapshot);
    }

    bool HasMemoryLeak(size_t nToleranceBytes = 1024 * 1024) const
    {
        if (m_aSnapshots.size() < 2)
            return false;

        const MemorySnapshot& first = m_aSnapshots.front();
        const MemorySnapshot& last = m_aSnapshots.back();

        return (last.totalMemory > first.totalMemory + nToleranceBytes);
    }

    size_t GetMemoryGrowth() const
    {
        if (m_aSnapshots.size() < 2)
            return 0;

        return m_aSnapshots.back().totalMemory - m_aSnapshots.front().totalMemory;
    }

    void Clear()
    {
        m_aSnapshots.clear();
    }

    const std::vector<MemorySnapshot>& GetSnapshots() const
    {
        return m_aSnapshots;
    }
};

/**
 * Memory leak detection test fixture for DocumentTabBar
 */
class DocumentTabBarMemoryTest : public CppUnit::TestFixture, public DocumentTabBarTestFixture
{
private:
    VclPtr<Dialog> m_pDialog;
    VclPtr<DocumentTabBar> m_pTabBar;
    std::unique_ptr<MemoryMonitor> m_pMemoryMonitor;

    // Reference counting helpers
    std::map<void*, int> m_aObjectReferences;

public:
    void setUp() override;
    void tearDown() override;

    // Basic memory leak tests
    void testTabCreationMemoryLeaks();
    void testTabRemovalMemoryLeaks();
    void testTabActivationMemoryLeaks();
    void testEventHandlerMemoryLeaks();

    // VCL object lifecycle tests
    void testVclObjectLifecycle();
    void testWidgetDisposalMemoryLeaks();
    void testParentChildMemoryLeaks();

    // Mock object memory tests
    void testMockObjectMemoryLeaks();
    void testViewFrameMemoryLeaks();
    void testObjectShellMemoryLeaks();

    // Stress testing for memory leaks
    void testLongRunningMemoryLeaks();
    void testCyclicOperationsMemoryLeaks();
    void testMemoryLeaksUnderStress();

    // Reference counting tests
    void testTabItemReferenceCounting();
    void testEventListenerReferenceCounting();
    void testCallbackReferenceCounting();

    // Resource cleanup tests
    void testResourceCleanupOnClose();
    void testResourceCleanupOnDispose();
    void testResourceCleanupOnDestroy();

    // Integration memory tests
    void testSfxIntegrationMemoryLeaks();
    void testApplicationShutdownMemoryCleanup();

    CPPUNIT_TEST_SUITE(DocumentTabBarMemoryTest);
    CPPUNIT_TEST(testTabCreationMemoryLeaks);
    CPPUNIT_TEST(testTabRemovalMemoryLeaks);
    CPPUNIT_TEST(testTabActivationMemoryLeaks);
    CPPUNIT_TEST(testEventHandlerMemoryLeaks);
    CPPUNIT_TEST(testVclObjectLifecycle);
    CPPUNIT_TEST(testWidgetDisposalMemoryLeaks);
    CPPUNIT_TEST(testParentChildMemoryLeaks);
    CPPUNIT_TEST(testMockObjectMemoryLeaks);
    CPPUNIT_TEST(testViewFrameMemoryLeaks);
    CPPUNIT_TEST(testObjectShellMemoryLeaks);
    CPPUNIT_TEST(testLongRunningMemoryLeaks);
    CPPUNIT_TEST(testCyclicOperationsMemoryLeaks);
    CPPUNIT_TEST(testMemoryLeaksUnderStress);
    CPPUNIT_TEST(testTabItemReferenceCounting);
    CPPUNIT_TEST(testEventListenerReferenceCounting);
    CPPUNIT_TEST(testCallbackReferenceCounting);
    CPPUNIT_TEST(testResourceCleanupOnClose);
    CPPUNIT_TEST(testResourceCleanupOnDispose);
    CPPUNIT_TEST(testResourceCleanupOnDestroy);
    CPPUNIT_TEST(testSfxIntegrationMemoryLeaks);
    CPPUNIT_TEST(testApplicationShutdownMemoryCleanup);
    CPPUNIT_TEST_SUITE_END();

private:
    // Helper methods
    void processEvents();
    void forceGarbageCollection();
    void verifyNoMemoryLeak(const std::string& rTestName);
    void addObjectReference(void* pObject);
    void removeObjectReference(void* pObject);
    bool hasOutstandingReferences() const;
};

void DocumentTabBarMemoryTest::setUp()
{
    m_pMemoryMonitor = std::make_unique<MemoryMonitor>();

    // Take initial memory snapshot
    m_pMemoryMonitor->TakeSnapshot();

    // Create parent dialog
    m_pDialog = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE | WB_CLOSEABLE, Dialog::InitFlag::NoParent);
    m_pDialog->SetSizePixel(Size(800, 600));

    // Create DocumentTabBar
    m_pTabBar = VclPtr<DocumentTabBar>::Create(m_pDialog, nullptr);
    m_pTabBar->SetSizePixel(Size(700, 30));
    m_pTabBar->SetPosPixel(Point(10, 10));

    addObjectReference(m_pDialog.get());
    addObjectReference(m_pTabBar.get());

    m_pDialog->Show();
    m_pTabBar->Show();
    processEvents();
}

void DocumentTabBarMemoryTest::tearDown()
{
    CleanupMockFrames();

    if (m_pTabBar)
    {
        removeObjectReference(m_pTabBar.get());
        m_pTabBar.disposeAndClear();
    }

    if (m_pDialog)
    {
        removeObjectReference(m_pDialog.get());
        m_pDialog.disposeAndClear();
    }

    processEvents();
    forceGarbageCollection();

    // Verify no outstanding references
    CPPUNIT_ASSERT_MESSAGE("Outstanding object references detected", !hasOutstandingReferences());

    m_pMemoryMonitor->TakeSnapshot();
    m_pMemoryMonitor.reset();
}

void DocumentTabBarMemoryTest::processEvents()
{
    Application::Yield();
    Scheduler::ProcessEventsToIdle();
}

void DocumentTabBarMemoryTest::forceGarbageCollection()
{
    // Force cleanup of VCL objects
    processEvents();

    // Multiple rounds to ensure cleanup
    for (int i = 0; i < 3; ++i)
    {
        processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void DocumentTabBarMemoryTest::verifyNoMemoryLeak(const std::string& rTestName)
{
    forceGarbageCollection();
    m_pMemoryMonitor->TakeSnapshot();

    // Allow for some tolerance in memory growth
    const size_t nToleranceBytes = 512 * 1024; // 512KB tolerance

    if (m_pMemoryMonitor->HasMemoryLeak(nToleranceBytes))
    {
        size_t nGrowth = m_pMemoryMonitor->GetMemoryGrowth();
        CPPUNIT_FAIL("Memory leak detected in " + rTestName +
                    ": grew by " + std::to_string(nGrowth) + " bytes");
    }
}

void DocumentTabBarMemoryTest::addObjectReference(void* pObject)
{
    if (pObject)
    {
        m_aObjectReferences[pObject]++;
    }
}

void DocumentTabBarMemoryTest::removeObjectReference(void* pObject)
{
    if (pObject)
    {
        auto it = m_aObjectReferences.find(pObject);
        if (it != m_aObjectReferences.end())
        {
            it->second--;
            if (it->second <= 0)
            {
                m_aObjectReferences.erase(it);
            }
        }
    }
}

bool DocumentTabBarMemoryTest::hasOutstandingReferences() const
{
    return !m_aObjectReferences.empty();
}

void DocumentTabBarMemoryTest::testTabCreationMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 100;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 10; ++i)
        {
            OUString aTitle = u"Test Tab "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        processEvents();

        // Remove all tabs
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        // Clean up mock frames
        CleanupMockFrames();

        processEvents();

        // Check for memory leaks every 10 iterations
        if (iteration % 10 == 9)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testTabCreationMemoryLeaks");
}

void DocumentTabBarMemoryTest::testTabRemovalMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 50;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create many tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 20; ++i)
        {
            OUString aTitle = u"Remove Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        processEvents();

        // Remove tabs in different patterns
        switch (iteration % 3)
        {
            case 0: // Remove from beginning
                for (sal_uInt16 nTabId : aTabIds)
                {
                    m_pTabBar->RemoveTab(nTabId);
                }
                break;

            case 1: // Remove from end
                for (auto it = aTabIds.rbegin(); it != aTabIds.rend(); ++it)
                {
                    m_pTabBar->RemoveTab(*it);
                }
                break;

            case 2: // Remove alternating
                for (size_t i = 0; i < aTabIds.size(); i += 2)
                {
                    m_pTabBar->RemoveTab(aTabIds[i]);
                }
                for (size_t i = 1; i < aTabIds.size(); i += 2)
                {
                    m_pTabBar->RemoveTab(aTabIds[i]);
                }
                break;
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testTabRemovalMemoryLeaks");
}

void DocumentTabBarMemoryTest::testTabActivationMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Create tabs once
    std::vector<sal_uInt16> aTabIds;
    for (int i = 0; i < 10; ++i)
    {
        OUString aTitle = u"Activation Test "_ustr + OUString::number(i);
        MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
        addObjectReference(pFrame);

        sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
        aTabIds.push_back(nTabId);
    }

    processEvents();

    // Perform many activation operations
    for (int iteration = 0; iteration < 1000; ++iteration)
    {
        sal_uInt16 nTabId = aTabIds[iteration % aTabIds.size()];
        m_pTabBar->SetActiveTab(nTabId);

        if (iteration % 100 == 99)
        {
            processEvents();
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    // Clean up
    for (sal_uInt16 nTabId : aTabIds)
    {
        m_pTabBar->RemoveTab(nTabId);
    }
    CleanupMockFrames();

    verifyNoMemoryLeak("testTabActivationMemoryLeaks");
}

void DocumentTabBarMemoryTest::testEventHandlerMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Track event handler invocations
    int nActivateEvents = 0;
    int nCloseEvents = 0;

    // Set up event handlers
    m_pTabBar->SetTabActivatedHdl(LINK_HELPER(DocumentTabBarMemoryTest, [&nActivateEvents](DocumentTabBar*) {
        nActivateEvents++;
    }));

    m_pTabBar->SetTabClosedHdl(LINK_HELPER(DocumentTabBarMemoryTest, [&nCloseEvents](DocumentTabBar*) {
        nCloseEvents++;
    }));

    const int nIterations = 100;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"Event Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        // Trigger events
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->SetActiveTab(nTabId);
            processEvents();
        }

        // Remove tabs (should trigger close events if implemented)
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();
        processEvents();

        if (iteration % 10 == 9)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    // Clear event handlers
    m_pTabBar->SetTabActivatedHdl(Link<DocumentTabBar*, void>());
    m_pTabBar->SetTabClosedHdl(Link<DocumentTabBar*, void>());

    verifyNoMemoryLeak("testEventHandlerMemoryLeaks");

    // Verify events were triggered
    CPPUNIT_ASSERT(nActivateEvents > 0);
}

void DocumentTabBarMemoryTest::testVclObjectLifecycle()
{
    m_pMemoryMonitor->TakeSnapshot();

    size_t nInitialVclObjects = m_pMemoryMonitor->GetVclObjectCount();

    const int nIterations = 50;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create a new DocumentTabBar
        VclPtr<DocumentTabBar> pTestTabBar = VclPtr<DocumentTabBar>::Create(m_pDialog, nullptr);
        addObjectReference(pTestTabBar.get());

        pTestTabBar->SetSizePixel(Size(600, 25));
        pTestTabBar->Show();

        // Add some tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"VCL Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = pTestTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        processEvents();

        // Clean up
        for (sal_uInt16 nTabId : aTabIds)
        {
            pTestTabBar->RemoveTab(nTabId);
        }

        removeObjectReference(pTestTabBar.get());
        pTestTabBar.disposeAndClear();

        CleanupMockFrames();
        processEvents();

        if (iteration % 10 == 9)
        {
            forceGarbageCollection();
            size_t nCurrentVclObjects = m_pMemoryMonitor->GetVclObjectCount();

            // Should not have accumulated VCL objects
            CPPUNIT_ASSERT_MESSAGE("VCL objects accumulated",
                                  nCurrentVclObjects <= nInitialVclObjects + 2); // Small tolerance
        }
    }

    verifyNoMemoryLeak("testVclObjectLifecycle");
}

void DocumentTabBarMemoryTest::testWidgetDisposalMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 30;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create nested widget hierarchy
        VclPtr<Dialog> pTestDialog = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE, Dialog::InitFlag::NoParent);
        addObjectReference(pTestDialog.get());

        VclPtr<DocumentTabBar> pTestTabBar = VclPtr<DocumentTabBar>::Create(pTestDialog, nullptr);
        addObjectReference(pTestTabBar.get());

        pTestDialog->SetSizePixel(Size(400, 300));
        pTestTabBar->SetSizePixel(Size(350, 25));

        // Add tabs with complex data
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 8; ++i)
        {
            OUString aTitle = u"Widget Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = pTestTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            // Set various properties
            pTestTabBar->SetTabModified(nTabId, (i % 2) == 0);
            pTestTabBar->SetTabTitle(nTabId, aTitle + u" Modified"_ustr);
        }

        pTestDialog->Show();
        processEvents();

        // Test disposal order - child first vs parent first
        if (iteration % 2 == 0)
        {
            // Dispose child first
            removeObjectReference(pTestTabBar.get());
            pTestTabBar.disposeAndClear();
            removeObjectReference(pTestDialog.get());
            pTestDialog.disposeAndClear();
        }
        else
        {
            // Dispose parent first (should dispose children)
            removeObjectReference(pTestTabBar.get());
            removeObjectReference(pTestDialog.get());
            pTestDialog.disposeAndClear();
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testWidgetDisposalMemoryLeaks");
}

void DocumentTabBarMemoryTest::testParentChildMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 20;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create parent-child relationships
        VclPtr<Dialog> pParent = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE, Dialog::InitFlag::NoParent);
        addObjectReference(pParent.get());

        std::vector<VclPtr<DocumentTabBar>> aChildren;

        // Create multiple child tab bars
        for (int child = 0; child < 3; ++child)
        {
            VclPtr<DocumentTabBar> pChild = VclPtr<DocumentTabBar>::Create(pParent, nullptr);
            addObjectReference(pChild.get());

            pChild->SetSizePixel(Size(200, 20));
            pChild->SetPosPixel(Point(10, 10 + child * 25));

            // Add tabs to each child
            for (int tab = 0; tab < 3; ++tab)
            {
                OUString aTitle = u"Child "_ustr + OUString::number(child) + u" Tab "_ustr + OUString::number(tab);
                MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
                addObjectReference(pFrame);

                pChild->AddTab(aTitle, pFrame->GetObjectShell());
            }

            aChildren.push_back(pChild);
        }

        pParent->Show();
        processEvents();

        // Clean up in different orders
        switch (iteration % 3)
        {
            case 0: // Parent first
                for (auto& pChild : aChildren)
                {
                    removeObjectReference(pChild.get());
                }
                removeObjectReference(pParent.get());
                pParent.disposeAndClear(); // Should dispose children
                break;

            case 1: // Children first
                for (auto& pChild : aChildren)
                {
                    removeObjectReference(pChild.get());
                    pChild.disposeAndClear();
                }
                removeObjectReference(pParent.get());
                pParent.disposeAndClear();
                break;

            case 2: // Mixed order
                removeObjectReference(aChildren[0].get());
                aChildren[0].disposeAndClear();
                removeObjectReference(pParent.get());
                for (size_t i = 1; i < aChildren.size(); ++i)
                {
                    removeObjectReference(aChildren[i].get());
                }
                pParent.disposeAndClear();
                break;
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testParentChildMemoryLeaks");
}

void DocumentTabBarMemoryTest::testMockObjectMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 100;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create and destroy mock objects rapidly
        std::vector<std::unique_ptr<MockSfxViewFrame>> aMockFrames;

        for (int i = 0; i < 10; ++i)
        {
            OUString aTitle = u"Mock Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            auto pFrame = std::make_unique<MockSfxViewFrame>(aTitle);
            addObjectReference(pFrame.get());

            // Use the mock object
            pFrame->GetMockObjectShell()->SetModified(true);
            pFrame->GetMockObjectShell()->SetTitle(aTitle + u" Modified"_ustr);
            pFrame->Activate();

            aMockFrames.push_back(std::move(pFrame));
        }

        // Clean up mock objects
        for (auto& pFrame : aMockFrames)
        {
            removeObjectReference(pFrame.get());
        }
        aMockFrames.clear();

        processEvents();

        if (iteration % 20 == 19)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testMockObjectMemoryLeaks");
}

void DocumentTabBarMemoryTest::testViewFrameMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test the static view frame tracking
    size_t nInitialFrameCount = MockSfxViewFrame::GetFrameCount();

    const int nIterations = 50;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        std::vector<std::unique_ptr<MockSfxViewFrame>> aFrames;

        // Create frames
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"Frame Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            auto pFrame = std::make_unique<MockSfxViewFrame>(aTitle);
            addObjectReference(pFrame.get());
            aFrames.push_back(std::move(pFrame));
        }

        // Verify frame tracking
        CPPUNIT_ASSERT_EQUAL(nInitialFrameCount + 5, MockSfxViewFrame::GetFrameCount());

        // Use frames with tab bar
        for (auto& pFrame : aFrames)
        {
            sal_uInt16 nTabId = m_pTabBar->AddTab(pFrame->GetMockObjectShell()->GetTitle(),
                                                pFrame->GetObjectShell());

            m_pTabBar->SetActiveTab(nTabId);
            pFrame->Activate();
        }

        processEvents();

        // Remove tabs
        while (m_pTabBar->GetTabCount() > 0)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            if (!aTabs.empty())
            {
                m_pTabBar->RemoveTab(aTabs[0].nId);
            }
        }

        // Clean up frames
        for (auto& pFrame : aFrames)
        {
            removeObjectReference(pFrame.get());
        }
        aFrames.clear();

        // Verify frame tracking cleanup
        CPPUNIT_ASSERT_EQUAL(nInitialFrameCount, MockSfxViewFrame::GetFrameCount());

        processEvents();
    }

    verifyNoMemoryLeak("testViewFrameMemoryLeaks");
}

void DocumentTabBarMemoryTest::testObjectShellMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 50;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        std::vector<sal_uInt16> aTabIds;

        // Create tabs with complex object shell operations
        for (int i = 0; i < 10; ++i)
        {
            OUString aTitle = u"ObjShell Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            MockSfxObjectShell* pObjShell = pFrame->GetMockObjectShell();

            // Perform operations that might cause leaks
            pObjShell->SetModified(true);
            pObjShell->SetTitle(aTitle + u" Modified"_ustr);
            pObjShell->SetReadOnly((i % 2) == 0);

            // Trigger frequent calls
            for (int call = 0; call < 10; ++call)
            {
                pObjShell->GetTitle();
                pObjShell->IsModified();
            }

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pObjShell);
            aTabIds.push_back(nTabId);
        }

        processEvents();

        // Perform operations on existing tabs
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->SetActiveTab(nTabId);
            m_pTabBar->SetTabModified(nTabId, true);

            DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTabId);
            // Use the info to ensure it's not optimized away
            CPPUNIT_ASSERT(!aInfo.sTitle.isEmpty());
        }

        // Clean up
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testObjectShellMemoryLeaks");
}

void DocumentTabBarMemoryTest::testLongRunningMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Simulate a long-running session with periodic activity
    const int nPhases = 20;
    const int nOperationsPerPhase = 50;

    for (int phase = 0; phase < nPhases; ++phase)
    {
        std::vector<sal_uInt16> aTabIds;

        // Create initial tabs
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"LongRun Phase "_ustr + OUString::number(phase) + u" Doc "_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        // Perform sustained operations
        for (int op = 0; op < nOperationsPerPhase; ++op)
        {
            sal_uInt16 nTabId = aTabIds[op % aTabIds.size()];

            // Mix of operations
            switch (op % 5)
            {
                case 0:
                    m_pTabBar->SetActiveTab(nTabId);
                    break;
                case 1:
                    m_pTabBar->SetTabModified(nTabId, (op % 3) == 0);
                    break;
                case 2:
                    {
                        OUString aNewTitle = u"Updated "_ustr + OUString::number(op);
                        m_pTabBar->SetTabTitle(nTabId, aNewTitle);
                    }
                    break;
                case 3:
                    {
                        DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTabId);
                        // Use the info
                        CPPUNIT_ASSERT(aInfo.nId == nTabId);
                    }
                    break;
                case 4:
                    {
                        std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
                        CPPUNIT_ASSERT(!aTabs.empty());
                    }
                    break;
            }

            if (op % 10 == 9)
            {
                processEvents();
            }
        }

        // Clean up this phase
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();
        processEvents();

        // Check memory every few phases
        if (phase % 5 == 4)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testLongRunningMemoryLeaks");
}

void DocumentTabBarMemoryTest::testCyclicOperationsMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nCycles = 100;

    for (int cycle = 0; cycle < nCycles; ++cycle)
    {
        // Phase 1: Create tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 8; ++i)
        {
            OUString aTitle = u"Cycle "_ustr + OUString::number(cycle) + u" Tab "_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        // Phase 2: Heavy operations
        for (int op = 0; op < 20; ++op)
        {
            for (sal_uInt16 nTabId : aTabIds)
            {
                m_pTabBar->SetActiveTab(nTabId);
                m_pTabBar->SetTabModified(nTabId, (op % 2) == 0);
            }
        }

        // Phase 3: Partial removal and recreation
        for (size_t i = 0; i < aTabIds.size() / 2; ++i)
        {
            m_pTabBar->RemoveTab(aTabIds[i]);
        }

        for (int i = 0; i < 4; ++i)
        {
            OUString aTitle = u"Cycle "_ustr + OUString::number(cycle) + u" New "_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        // Phase 4: Complete cleanup
        while (m_pTabBar->GetTabCount() > 0)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            if (!aTabs.empty())
            {
                m_pTabBar->RemoveTab(aTabs[0].nId);
            }
        }

        CleanupMockFrames();
        processEvents();

        if (cycle % 20 == 19)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testCyclicOperationsMemoryLeaks");
}

void DocumentTabBarMemoryTest::testMemoryLeaksUnderStress()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Stress test with rapid operations
    const int nStressIterations = 500;

    for (int i = 0; i < nStressIterations; ++i)
    {
        // Rapid creation
        OUString aTitle = u"Stress "_ustr + OUString::number(i);
        MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
        addObjectReference(pFrame);

        sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());

        // Rapid operations
        m_pTabBar->SetActiveTab(nTabId);
        m_pTabBar->SetTabModified(nTabId, true);
        m_pTabBar->SetTabTitle(nTabId, aTitle + u" Modified"_ustr);

        DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTabId);

        // Rapid removal
        m_pTabBar->RemoveTab(nTabId);

        // Keep only a few tabs at a time
        if (m_pTabBar->GetTabCount() > 5)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            m_pTabBar->RemoveTab(aTabs[0].nId);
        }

        // Minimal event processing
        if (i % 50 == 49)
        {
            CleanupMockFrames();
            processEvents();
        }

        if (i % 100 == 99)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    // Final cleanup
    while (m_pTabBar->GetTabCount() > 0)
    {
        std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
        if (!aTabs.empty())
        {
            m_pTabBar->RemoveTab(aTabs[0].nId);
        }
    }

    CleanupMockFrames();

    verifyNoMemoryLeak("testMemoryLeaksUnderStress");
}

void DocumentTabBarMemoryTest::testTabItemReferenceCounting()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 50;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        std::vector<sal_uInt16> aTabIds;

        // Create tabs
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"RefCount Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);
        }

        // Get multiple references to the same tab info
        for (sal_uInt16 nTabId : aTabIds)
        {
            for (int ref = 0; ref < 10; ++ref)
            {
                DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTabId);
                // Use the info to prevent optimization
                CPPUNIT_ASSERT(aInfo.nId == nTabId);
            }
        }

        // Get all tabs multiple times
        for (int ref = 0; ref < 10; ++ref)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), aTabs.size());
        }

        // Clean up
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testTabItemReferenceCounting");
}

void DocumentTabBarMemoryTest::testEventListenerReferenceCounting()
{
    m_pMemoryMonitor->TakeSnapshot();

    const int nIterations = 30;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create event handlers that capture references
        std::vector<std::shared_ptr<int>> aSharedData;

        for (int i = 0; i < 5; ++i)
        {
            aSharedData.push_back(std::make_shared<int>(iteration * 10 + i));
        }

        // Set up event handlers with captured references
        auto pSharedRef = aSharedData[0];
        m_pTabBar->SetTabActivatedHdl(LINK_HELPER(DocumentTabBarMemoryTest, [pSharedRef](DocumentTabBar*) {
            // Use shared reference to prevent optimization
            int value = *pSharedRef;
            (void)value;
        }));

        // Create and use tabs
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 3; ++i)
        {
            OUString aTitle = u"EventRef Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            // Trigger event
            m_pTabBar->SetActiveTab(nTabId);
            processEvents();
        }

        // Clear event handler
        m_pTabBar->SetTabActivatedHdl(Link<DocumentTabBar*, void>());

        // Clean up tabs
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();

        // Clear shared data
        aSharedData.clear();
        pSharedRef.reset();

        processEvents();
    }

    verifyNoMemoryLeak("testEventListenerReferenceCounting");
}

void DocumentTabBarMemoryTest::testCallbackReferenceCounting()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test that callbacks don't create circular references
    const int nIterations = 40;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create callback chains
        auto pData = std::make_shared<std::vector<int>>();
        for (int i = 0; i < 100; ++i)
        {
            pData->push_back(iteration * 100 + i);
        }

        // Set up complex callback
        m_pTabBar->SetTabActivatedHdl(LINK_HELPER(DocumentTabBarMemoryTest, [pData](DocumentTabBar* pTabBar) {
            // Callback that references both data and tab bar
            if (pTabBar && !pData->empty())
            {
                sal_uInt16 nActiveTab = pTabBar->GetActiveTab();
                if (nActiveTab > 0)
                {
                    DocumentTabBar::TabInfo aInfo = pTabBar->GetTabInfo(nActiveTab);
                    // Use both callback data and tab bar data
                    int sum = (*pData)[0] + aInfo.nId;
                    (void)sum;
                }
            }
        }));

        // Create tabs and trigger callbacks
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 3; ++i)
        {
            OUString aTitle = u"Callback Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            m_pTabBar->SetActiveTab(nTabId);
            processEvents();
        }

        // Clear callback before cleanup
        m_pTabBar->SetTabActivatedHdl(Link<DocumentTabBar*, void>());

        // Clean up
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();
        pData.reset();
        processEvents();
    }

    verifyNoMemoryLeak("testCallbackReferenceCounting");
}

void DocumentTabBarMemoryTest::testResourceCleanupOnClose()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test resource cleanup when tabs are closed normally
    const int nIterations = 30;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        std::vector<sal_uInt16> aTabIds;

        // Create tabs with various resources
        for (int i = 0; i < 8; ++i)
        {
            OUString aTitle = u"Close Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            // Set up resources that need cleanup
            m_pTabBar->SetTabModified(nTabId, true);
            m_pTabBar->SetTabTitle(nTabId, aTitle + u" With Resources"_ustr);
            // Add other resource-intensive operations
        }

        processEvents();

        // Close tabs one by one (simulating user closing tabs)
        for (sal_uInt16 nTabId : aTabIds)
        {
            // Activate before closing
            m_pTabBar->SetActiveTab(nTabId);
            processEvents();

            // Close tab
            m_pTabBar->RemoveTab(nTabId);
            processEvents();
        }

        CleanupMockFrames();
        processEvents();

        if (iteration % 10 == 9)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testResourceCleanupOnClose");
}

void DocumentTabBarMemoryTest::testResourceCleanupOnDispose()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test resource cleanup when widget is disposed
    const int nIterations = 20;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        // Create separate tab bar for disposal testing
        VclPtr<DocumentTabBar> pTestTabBar = VclPtr<DocumentTabBar>::Create(m_pDialog, nullptr);
        addObjectReference(pTestTabBar.get());

        pTestTabBar->SetSizePixel(Size(500, 25));
        pTestTabBar->Show();

        // Add tabs with resources
        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < 6; ++i)
        {
            OUString aTitle = u"Dispose Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = pTestTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            // Create resources
            pTestTabBar->SetTabModified(nTabId, true);
            pTestTabBar->SetActiveTab(nTabId);
        }

        processEvents();

        // Dispose tab bar without explicitly removing tabs
        removeObjectReference(pTestTabBar.get());
        pTestTabBar.disposeAndClear();

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testResourceCleanupOnDispose");
}

void DocumentTabBarMemoryTest::testResourceCleanupOnDestroy()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test resource cleanup when objects are destroyed
    const int nIterations = 25;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        {
            // Scope to ensure destruction
            VclPtr<Dialog> pTestDialog = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE, Dialog::InitFlag::NoParent);
            addObjectReference(pTestDialog.get());

            VclPtr<DocumentTabBar> pTestTabBar = VclPtr<DocumentTabBar>::Create(pTestDialog, nullptr);
            addObjectReference(pTestTabBar.get());

            // Create complex resource hierarchy
            std::vector<sal_uInt16> aTabIds;
            for (int i = 0; i < 5; ++i)
            {
                OUString aTitle = u"Destroy Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
                MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
                addObjectReference(pFrame);

                sal_uInt16 nTabId = pTestTabBar->AddTab(aTitle, pFrame->GetObjectShell());
                aTabIds.push_back(nTabId);
            }

            pTestDialog->Show();
            processEvents();

            // Destroy in proper order
            removeObjectReference(pTestTabBar.get());
            pTestTabBar.disposeAndClear();
            removeObjectReference(pTestDialog.get());
            pTestDialog.disposeAndClear();

            CleanupMockFrames();

        } // End scope - ensure destruction

        processEvents();

        if (iteration % 10 == 9)
        {
            forceGarbageCollection();
            m_pMemoryMonitor->TakeSnapshot();
        }
    }

    verifyNoMemoryLeak("testResourceCleanupOnDestroy");
}

void DocumentTabBarMemoryTest::testSfxIntegrationMemoryLeaks()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Test memory leaks in SFX integration
    const int nIterations = 40;

    for (int iteration = 0; iteration < nIterations; ++iteration)
    {
        std::vector<MockSfxViewFrame*> aFrames;

        // Create view frames with SFX integration
        for (int i = 0; i < 5; ++i)
        {
            OUString aTitle = u"SFX Test "_ustr + OUString::number(iteration) + u"_"_ustr + OUString::number(i);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);
            aFrames.push_back(pFrame);

            // Simulate SFX operations
            pFrame->Activate();
            pFrame->GetMockObjectShell()->SetModified(true);

            // Add to tab bar
            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());

            // Simulate SFX state changes
            m_pTabBar->SetActiveTab(nTabId);
            m_pTabBar->SetTabModified(nTabId, pFrame->GetMockObjectShell()->IsModified());
        }

        // Simulate SFX frame management operations
        for (MockSfxViewFrame* pFrame : aFrames)
        {
            pFrame->Activate();
            processEvents();
            pFrame->Deactivate();
        }

        // Clean up through tab bar
        while (m_pTabBar->GetTabCount() > 0)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            if (!aTabs.empty())
            {
                m_pTabBar->RemoveTab(aTabs[0].nId);
            }
        }

        CleanupMockFrames();
        processEvents();
    }

    verifyNoMemoryLeak("testSfxIntegrationMemoryLeaks");
}

void DocumentTabBarMemoryTest::testApplicationShutdownMemoryCleanup()
{
    m_pMemoryMonitor->TakeSnapshot();

    // Simulate application shutdown scenario
    const int nSessions = 10;

    for (int session = 0; session < nSessions; ++session)
    {
        // Simulate user session with multiple documents
        std::vector<sal_uInt16> aTabIds;

        for (int doc = 0; doc < 8; ++doc)
        {
            OUString aTitle = u"Session "_ustr + OUString::number(session) + u" Doc "_ustr + OUString::number(doc);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            addObjectReference(pFrame);

            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            // Simulate document work
            m_pTabBar->SetActiveTab(nTabId);
            pFrame->GetMockObjectShell()->SetModified((doc % 2) == 0);
            m_pTabBar->SetTabModified(nTabId, pFrame->GetMockObjectShell()->IsModified());
        }

        // Simulate user work
        for (int work = 0; work < 20; ++work)
        {
            sal_uInt16 nTabId = aTabIds[work % aTabIds.size()];
            m_pTabBar->SetActiveTab(nTabId);
            processEvents();
        }

        // Simulate shutdown - close all documents
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        CleanupMockFrames();

        // Simulate complete cleanup
        forceGarbageCollection();
        processEvents();

        m_pMemoryMonitor->TakeSnapshot();
    }

    verifyNoMemoryLeak("testApplicationShutdownMemoryCleanup");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentTabBarMemoryTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */