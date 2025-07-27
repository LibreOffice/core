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
#include <tools/time.hxx>

#include "documenttabbar_mocks.hxx"

#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>
#include <random>

using namespace sfx2::test;

namespace {

/**
 * Performance test fixture for DocumentTabBar
 * Tests performance characteristics with large numbers of tabs,
 * rapid operations, and memory usage patterns.
 */
class DocumentTabBarPerformanceTest : public CppUnit::TestFixture, public DocumentTabBarTestFixture
{
private:
    VclPtr<Dialog> m_pDialog;
    VclPtr<DocumentTabBar> m_pTabBar;

    // Performance measurement utilities
    struct PerformanceTimer
    {
        std::chrono::high_resolution_clock::time_point m_start;
        std::chrono::high_resolution_clock::time_point m_end;

        void Start() { m_start = std::chrono::high_resolution_clock::now(); }
        void Stop() { m_end = std::chrono::high_resolution_clock::now(); }

        std::chrono::milliseconds GetDurationMs() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_start);
        }

        std::chrono::microseconds GetDurationUs() const
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start);
        }
    };

public:
    void setUp() override;
    void tearDown() override;

    // Basic performance tests
    void testManyTabsCreation();
    void testManyTabsRemoval();
    void testManyTabsActivation();
    void testTabBarLayoutPerformance();
    void testTabBarRenderingPerformance();

    // Stress tests
    void testRapidTabOperations();
    void testConcurrentTabOperations();
    void testTabBarWithLongTitles();
    void testTabBarWithFrequentUpdates();

    // Memory usage tests
    void testMemoryUsageWithManyTabs();
    void testMemoryLeakDetection();
    void testTabBarScalability();

    // Real-world scenario tests
    void testTypicalUserWorkflow();
    void testDeveloperWorkflow();
    void testPowerUserWorkflow();

    // Regression tests
    void testPerformanceRegression();
    void testMemoryRegression();

    // Benchmark tests
    void benchmarkTabCreation();
    void benchmarkTabActivation();
    void benchmarkTabRemoval();
    void benchmarkLayoutCalculation();

    CPPUNIT_TEST_SUITE(DocumentTabBarPerformanceTest);
    CPPUNIT_TEST(testManyTabsCreation);
    CPPUNIT_TEST(testManyTabsRemoval);
    CPPUNIT_TEST(testManyTabsActivation);
    CPPUNIT_TEST(testTabBarLayoutPerformance);
    CPPUNIT_TEST(testTabBarRenderingPerformance);
    CPPUNIT_TEST(testRapidTabOperations);
    CPPUNIT_TEST(testConcurrentTabOperations);
    CPPUNIT_TEST(testTabBarWithLongTitles);
    CPPUNIT_TEST(testTabBarWithFrequentUpdates);
    CPPUNIT_TEST(testMemoryUsageWithManyTabs);
    CPPUNIT_TEST(testMemoryLeakDetection);
    CPPUNIT_TEST(testTabBarScalability);
    CPPUNIT_TEST(testTypicalUserWorkflow);
    CPPUNIT_TEST(testDeveloperWorkflow);
    CPPUNIT_TEST(testPowerUserWorkflow);
    CPPUNIT_TEST(testPerformanceRegression);
    CPPUNIT_TEST(testMemoryRegression);
    CPPUNIT_TEST(benchmarkTabCreation);
    CPPUNIT_TEST(benchmarkTabActivation);
    CPPUNIT_TEST(benchmarkTabRemoval);
    CPPUNIT_TEST(benchmarkLayoutCalculation);
    CPPUNIT_TEST_SUITE_END();

private:
    // Helper methods
    void processEvents();
    void createManyTabs(int nCount, std::vector<sal_uInt16>& rTabIds);
    void measureOperation(const std::string& rOperationName,
                         std::function<void()> aOperation,
                         int nIterations = 1);
    size_t getCurrentMemoryUsage();
    void verifyPerformanceConstraints(std::chrono::milliseconds aDuration,
                                    std::chrono::milliseconds aMaxAllowed,
                                    const std::string& rOperation);
};

void DocumentTabBarPerformanceTest::setUp()
{
    // Create parent dialog
    m_pDialog = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE | WB_CLOSEABLE, Dialog::InitFlag::NoParent);
    m_pDialog->SetSizePixel(Size(1200, 800)); // Larger size for performance testing

    // Create DocumentTabBar
    m_pTabBar = VclPtr<DocumentTabBar>::Create(m_pDialog, nullptr);
    m_pTabBar->SetSizePixel(Size(1100, 30));
    m_pTabBar->SetPosPixel(Point(10, 10));

    m_pDialog->Show();
    m_pTabBar->Show();
    processEvents();
}

void DocumentTabBarPerformanceTest::tearDown()
{
    CleanupMockFrames();

    if (m_pTabBar)
    {
        m_pTabBar.disposeAndClear();
    }

    if (m_pDialog)
    {
        m_pDialog.disposeAndClear();
    }

    processEvents();
}

void DocumentTabBarPerformanceTest::processEvents()
{
    Application::Yield();
    Scheduler::ProcessEventsToIdle();
}

void DocumentTabBarPerformanceTest::createManyTabs(int nCount, std::vector<sal_uInt16>& rTabIds)
{
    rTabIds.clear();
    rTabIds.reserve(nCount);

    for (int i = 0; i < nCount; ++i)
    {
        OUString aTitle = u"Document "_ustr + OUString::number(i + 1);
        MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);

        sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
        rTabIds.push_back(nTabId);

        // Process events periodically to avoid overwhelming the system
        if (i % 50 == 0)
        {
            processEvents();
        }
    }
}

void DocumentTabBarPerformanceTest::measureOperation(const std::string& rOperationName,
                                                   std::function<void()> aOperation,
                                                   int nIterations)
{
    PerformanceTimer timer;
    timer.Start();

    for (int i = 0; i < nIterations; ++i)
    {
        aOperation();
    }

    timer.Stop();

    auto duration = timer.GetDurationMs();
    auto avgDuration = duration.count() / nIterations;

    // Log performance results (in actual implementation, this would use proper logging)
    printf("Performance: %s - Total: %ldms, Average: %ldms, Iterations: %d\n",
           rOperationName.c_str(), duration.count(), avgDuration, nIterations);
}

size_t DocumentTabBarPerformanceTest::getCurrentMemoryUsage()
{
    // In a real implementation, this would use platform-specific memory measurement
    // For testing purposes, we'll return a mock value
    return 1024 * 1024; // 1MB placeholder
}

void DocumentTabBarPerformanceTest::verifyPerformanceConstraints(
    std::chrono::milliseconds aDuration,
    std::chrono::milliseconds aMaxAllowed,
    const std::string& rOperation)
{
    if (aDuration > aMaxAllowed)
    {
        CPPUNIT_FAIL("Performance constraint violated for " + rOperation +
                    ": took " + std::to_string(aDuration.count()) + "ms, " +
                    "maximum allowed " + std::to_string(aMaxAllowed.count()) + "ms");
    }
}

void DocumentTabBarPerformanceTest::testManyTabsCreation()
{
    const int nTabCount = 1000;
    std::vector<sal_uInt16> aTabIds;

    PerformanceTimer timer;
    timer.Start();

    createManyTabs(nTabCount, aTabIds);

    timer.Stop();

    // Verify all tabs were created
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(nTabCount), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nTabCount), aTabIds.size());

    // Verify performance constraint (should create 1000 tabs in under 5 seconds)
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(5000),
                                "Creating 1000 tabs");

    // Verify memory usage is reasonable
    size_t memoryUsage = getCurrentMemoryUsage();
    CPPUNIT_ASSERT(memoryUsage < 100 * 1024 * 1024); // Less than 100MB
}

void DocumentTabBarPerformanceTest::testManyTabsRemoval()
{
    const int nTabCount = 1000;
    std::vector<sal_uInt16> aTabIds;

    // Create many tabs first
    createManyTabs(nTabCount, aTabIds);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(nTabCount), m_pTabBar->GetTabCount());

    // Measure removal performance
    PerformanceTimer timer;
    timer.Start();

    // Remove all tabs
    for (sal_uInt16 nTabId : aTabIds)
    {
        m_pTabBar->RemoveTab(nTabId);
    }

    timer.Stop();

    // Verify all tabs were removed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetTabCount());

    // Verify performance constraint (should remove 1000 tabs in under 2 seconds)
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(2000),
                                "Removing 1000 tabs");
}

void DocumentTabBarPerformanceTest::testManyTabsActivation()
{
    const int nTabCount = 100;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    // Measure activation performance
    PerformanceTimer timer;
    timer.Start();

    // Activate each tab once
    for (sal_uInt16 nTabId : aTabIds)
    {
        m_pTabBar->SetActiveTab(nTabId);
        processEvents();
    }

    timer.Stop();

    // Verify performance constraint (should activate 100 tabs in under 1 second)
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(1000),
                                "Activating 100 tabs");
}

void DocumentTabBarPerformanceTest::testTabBarLayoutPerformance()
{
    const int nTabCount = 200;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    // Measure layout calculation performance
    measureOperation("Layout calculation with 200 tabs", [this]() {
        m_pTabBar->SetSizePixel(Size(800 + (rand() % 400), 30)); // Random width changes
        processEvents();
    }, 100);

    // Test with different tab widths
    measureOperation("Layout with constrained widths", [this]() {
        m_pTabBar->SetTabConstraints(50 + (rand() % 100), 150 + (rand() % 100));
        processEvents();
    }, 50);
}

void DocumentTabBarPerformanceTest::testTabBarRenderingPerformance()
{
    const int nTabCount = 50;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    // Force multiple repaints to measure rendering performance
    measureOperation("Tab bar rendering with 50 tabs", [this]() {
        m_pTabBar->Invalidate();
        processEvents();
    }, 100);

    // Test rendering with modified tabs
    for (size_t i = 0; i < aTabIds.size(); i += 2)
    {
        m_pTabBar->SetTabModified(aTabIds[i], true);
    }

    measureOperation("Rendering with modified tabs", [this]() {
        m_pTabBar->Invalidate();
        processEvents();
    }, 50);
}

void DocumentTabBarPerformanceTest::testRapidTabOperations()
{
    // Simulate rapid user operations
    const int nOperations = 1000;
    std::vector<sal_uInt16> aTabIds;

    PerformanceTimer timer;
    timer.Start();

    for (int i = 0; i < nOperations; ++i)
    {
        switch (i % 4)
        {
            case 0: // Add tab
            {
                OUString aTitle = u"Rapid Doc "_ustr + OUString::number(i);
                MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
                sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
                aTabIds.push_back(nTabId);
                break;
            }
            case 1: // Activate tab
                if (!aTabIds.empty())
                {
                    sal_uInt16 nTabId = aTabIds[rand() % aTabIds.size()];
                    m_pTabBar->SetActiveTab(nTabId);
                }
                break;
            case 2: // Modify tab
                if (!aTabIds.empty())
                {
                    sal_uInt16 nTabId = aTabIds[rand() % aTabIds.size()];
                    m_pTabBar->SetTabModified(nTabId, (i % 3) == 0);
                }
                break;
            case 3: // Remove tab (occasionally)
                if (aTabIds.size() > 5 && (i % 10) == 0)
                {
                    size_t nIndex = rand() % aTabIds.size();
                    m_pTabBar->RemoveTab(aTabIds[nIndex]);
                    aTabIds.erase(aTabIds.begin() + nIndex);
                }
                break;
        }

        // Process events periodically
        if (i % 100 == 0)
        {
            processEvents();
        }
    }

    timer.Stop();

    // Verify performance constraint (1000 operations in under 5 seconds)
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(5000),
                                "1000 rapid tab operations");

    // Verify tab bar is still in valid state
    CPPUNIT_ASSERT(m_pTabBar->GetTabCount() > 0);
}

void DocumentTabBarPerformanceTest::testConcurrentTabOperations()
{
    // Simulate concurrent operations (as much as possible in single-threaded test)
    const int nTabCount = 100;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    // Perform interleaved operations rapidly
    PerformanceTimer timer;
    timer.Start();

    for (int round = 0; round < 10; ++round)
    {
        // Activate multiple tabs rapidly
        for (int i = 0; i < 20; ++i)
        {
            sal_uInt16 nTabId = aTabIds[i % aTabIds.size()];
            m_pTabBar->SetActiveTab(nTabId);
        }

        // Modify multiple tabs rapidly
        for (int i = 0; i < 20; ++i)
        {
            sal_uInt16 nTabId = aTabIds[i % aTabIds.size()];
            m_pTabBar->SetTabModified(nTabId, (i % 2) == 0);
        }

        // Update titles rapidly
        for (int i = 0; i < 10; ++i)
        {
            sal_uInt16 nTabId = aTabIds[i % aTabIds.size()];
            OUString aTitle = u"Updated "_ustr + OUString::number(round) + u"_"_ustr + OUString::number(i);
            m_pTabBar->SetTabTitle(nTabId, aTitle);
        }

        processEvents();
    }

    timer.Stop();

    // Verify performance constraint
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(2000),
                                "Concurrent operations simulation");
}

void DocumentTabBarPerformanceTest::testTabBarWithLongTitles()
{
    const int nTabCount = 50;
    std::vector<sal_uInt16> aTabIds;

    // Create tabs with very long titles
    PerformanceTimer timer;
    timer.Start();

    for (int i = 0; i < nTabCount; ++i)
    {
        OUString aLongTitle = u"This is a very long document title that should test the performance of tab bar layout and rendering when dealing with extremely long text content that might need to be truncated or handled specially Document Number "_ustr + OUString::number(i + 1);

        MockSfxViewFrame* pFrame = CreateMockViewFrame(aLongTitle);
        sal_uInt16 nTabId = m_pTabBar->AddTab(aLongTitle, pFrame->GetObjectShell());
        aTabIds.push_back(nTabId);

        if (i % 10 == 0)
        {
            processEvents();
        }
    }

    timer.Stop();

    // Verify performance with long titles
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(2000),
                                "Creating 50 tabs with long titles");

    // Test layout calculation with long titles
    measureOperation("Layout with long titles", [this]() {
        m_pTabBar->SetTabConstraints(80, 200);
        processEvents();
    }, 20);
}

void DocumentTabBarPerformanceTest::testTabBarWithFrequentUpdates()
{
    const int nTabCount = 20;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    // Simulate frequent document updates (like auto-save indicators, title changes, etc.)
    measureOperation("Frequent tab updates", [this, &aTabIds]() {
        for (size_t i = 0; i < aTabIds.size(); ++i)
        {
            // Update title
            OUString aTitle = u"Updated Doc "_ustr + OUString::number(rand() % 1000);
            m_pTabBar->SetTabTitle(aTabIds[i], aTitle);

            // Toggle modified state
            m_pTabBar->SetTabModified(aTabIds[i], (rand() % 2) == 0);

            // Occasionally activate tab
            if (i % 5 == 0)
            {
                m_pTabBar->SetActiveTab(aTabIds[i]);
            }
        }
        processEvents();
    }, 100);
}

void DocumentTabBarPerformanceTest::testMemoryUsageWithManyTabs()
{
    size_t initialMemory = getCurrentMemoryUsage();

    const int nTabCount = 500;
    std::vector<sal_uInt16> aTabIds;

    createManyTabs(nTabCount, aTabIds);

    size_t peakMemory = getCurrentMemoryUsage();

    // Remove all tabs
    for (sal_uInt16 nTabId : aTabIds)
    {
        m_pTabBar->RemoveTab(nTabId);
    }
    processEvents();

    size_t finalMemory = getCurrentMemoryUsage();

    // Verify memory usage is reasonable
    size_t memoryIncrease = peakMemory - initialMemory;
    size_t memoryPerTab = memoryIncrease / nTabCount;

    // Each tab should use less than 10KB of memory
    CPPUNIT_ASSERT(memoryPerTab < 10 * 1024);

    // Memory should be mostly freed after removing tabs
    CPPUNIT_ASSERT(finalMemory < initialMemory + (memoryIncrease / 2));
}

void DocumentTabBarPerformanceTest::testMemoryLeakDetection()
{
    size_t initialMemory = getCurrentMemoryUsage();

    // Perform multiple cycles of tab creation/destruction
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        const int nTabCount = 50;
        std::vector<sal_uInt16> aTabIds;

        createManyTabs(nTabCount, aTabIds);

        // Perform operations on tabs
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->SetActiveTab(nTabId);
            m_pTabBar->SetTabModified(nTabId, true);
            m_pTabBar->SetTabTitle(nTabId, u"Updated Title"_ustr);
        }

        // Remove all tabs
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }

        processEvents();
    }

    size_t finalMemory = getCurrentMemoryUsage();

    // Memory should not have grown significantly
    size_t memoryGrowth = finalMemory > initialMemory ? finalMemory - initialMemory : 0;

    // Allow for some memory growth, but it should be minimal
    CPPUNIT_ASSERT(memoryGrowth < 1024 * 1024); // Less than 1MB growth
}

void DocumentTabBarPerformanceTest::testTabBarScalability()
{
    // Test scalability with increasing numbers of tabs
    std::vector<int> tabCounts = {10, 50, 100, 200, 500, 1000};

    for (int nTabCount : tabCounts)
    {
        // Clean up previous tabs
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

        // Measure tab creation time
        PerformanceTimer creationTimer;
        creationTimer.Start();

        std::vector<sal_uInt16> aTabIds;
        for (int i = 0; i < nTabCount; ++i)
        {
            OUString aTitle = u"Document "_ustr + OUString::number(i + 1);
            MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
            sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
            aTabIds.push_back(nTabId);

            if (i % 100 == 0)
            {
                processEvents();
            }
        }

        creationTimer.Stop();

        // Measure activation time
        PerformanceTimer activationTimer;
        activationTimer.Start();

        for (int i = 0; i < std::min(nTabCount, 100); ++i)
        {
            sal_uInt16 nTabId = aTabIds[i];
            m_pTabBar->SetActiveTab(nTabId);
        }

        activationTimer.Stop();

        // Log scalability results
        printf("Scalability test - Tabs: %d, Creation: %ldms, Activation: %ldms\n",
               nTabCount, creationTimer.GetDurationMs().count(), activationTimer.GetDurationMs().count());

        // Verify that performance doesn't degrade exponentially
        if (nTabCount >= 100)
        {
            // Creation time should be roughly linear (allow for 2x growth per 10x tabs)
            auto maxCreationTime = std::chrono::milliseconds(nTabCount * 10); // 10ms per tab max
            verifyPerformanceConstraints(creationTimer.GetDurationMs(), maxCreationTime,
                                       "Tab creation scalability");
        }
    }
}

void DocumentTabBarPerformanceTest::testTypicalUserWorkflow()
{
    // Simulate a typical user workflow: create a few documents, work with them
    PerformanceTimer timer;
    timer.Start();

    std::vector<sal_uInt16> aTabIds;

    // User opens 3-5 documents
    for (int i = 0; i < 4; ++i)
    {
        OUString aTitle = u"User Document "_ustr + OUString::number(i + 1);
        MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
        sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
        aTabIds.push_back(nTabId);
        processEvents();
    }

    // User switches between documents frequently
    for (int i = 0; i < 50; ++i)
    {
        sal_uInt16 nTabId = aTabIds[i % aTabIds.size()];
        m_pTabBar->SetActiveTab(nTabId);

        // Occasionally modify document
        if (i % 7 == 0)
        {
            m_pTabBar->SetTabModified(nTabId, true);
        }

        if (i % 5 == 0)
        {
            processEvents();
        }
    }

    // User closes some documents
    m_pTabBar->RemoveTab(aTabIds[1]);
    processEvents();

    timer.Stop();

    // Typical user workflow should be very fast
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(500),
                                "Typical user workflow");
}

void DocumentTabBarPerformanceTest::testDeveloperWorkflow()
{
    // Simulate a developer workflow: many files open, frequent switching
    PerformanceTimer timer;
    timer.Start();

    std::vector<sal_uInt16> aTabIds;

    // Developer opens many source files
    std::vector<OUString> fileNames = {
        u"main.cpp", u"header.h", u"utils.cpp", u"utils.h", u"config.xml",
        u"Makefile", u"README.md", u"test.cpp", u"data.json", u"style.css",
        u"script.js", u"component.tsx", u"service.java", u"model.py", u"view.html"
    };

    for (const OUString& fileName : fileNames)
    {
        MockSfxViewFrame* pFrame = CreateMockViewFrame(fileName);
        sal_uInt16 nTabId = m_pTabBar->AddTab(fileName, pFrame->GetObjectShell());
        aTabIds.push_back(nTabId);
    }
    processEvents();

    // Developer frequently switches between files while coding
    for (int i = 0; i < 200; ++i)
    {
        sal_uInt16 nTabId = aTabIds[i % aTabIds.size()];
        m_pTabBar->SetActiveTab(nTabId);

        // Files get modified frequently
        m_pTabBar->SetTabModified(nTabId, (i % 3) != 0);

        if (i % 10 == 0)
        {
            processEvents();
        }
    }

    timer.Stop();

    // Developer workflow should handle many tabs efficiently
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(1000),
                                "Developer workflow");
}

void DocumentTabBarPerformanceTest::testPowerUserWorkflow()
{
    // Simulate a power user workflow: very many documents, complex operations
    PerformanceTimer timer;
    timer.Start();

    std::vector<sal_uInt16> aTabIds;

    // Power user opens many documents
    for (int i = 0; i < 30; ++i)
    {
        OUString aTitle = u"PowerUser Doc "_ustr + OUString::number(i + 1);
        MockSfxViewFrame* pFrame = CreateMockViewFrame(aTitle);
        sal_uInt16 nTabId = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
        aTabIds.push_back(nTabId);

        if (i % 5 == 0)
        {
            processEvents();
        }
    }

    // Complex switching patterns
    for (int round = 0; round < 10; ++round)
    {
        // Jump to different areas of tab bar
        for (int pos : {0, 5, 15, 25, 10, 20, 3, 18, 8, 28})
        {
            if (pos < static_cast<int>(aTabIds.size()))
            {
                m_pTabBar->SetActiveTab(aTabIds[pos]);
                m_pTabBar->SetTabModified(aTabIds[pos], (round % 3) == 0);
            }
        }

        // Batch operations
        for (size_t i = round * 3; i < std::min(aTabIds.size(), (round + 1) * 3); ++i)
        {
            OUString aNewTitle = u"Updated "_ustr + OUString::number(round) + u"_"_ustr + OUString::number(i);
            m_pTabBar->SetTabTitle(aTabIds[i], aNewTitle);
        }

        processEvents();
    }

    timer.Stop();

    // Power user workflow should still be responsive
    verifyPerformanceConstraints(timer.GetDurationMs(), std::chrono::milliseconds(2000),
                                "Power user workflow");
}

void DocumentTabBarPerformanceTest::testPerformanceRegression()
{
    // This test would compare current performance against established benchmarks
    // In a real implementation, this would load baseline performance data

    const int nTabCount = 100;
    std::vector<sal_uInt16> aTabIds;

    // Benchmark current performance
    PerformanceTimer creationTimer;
    creationTimer.Start();
    createManyTabs(nTabCount, aTabIds);
    creationTimer.Stop();

    PerformanceTimer activationTimer;
    activationTimer.Start();
    for (sal_uInt16 nTabId : aTabIds)
    {
        m_pTabBar->SetActiveTab(nTabId);
    }
    activationTimer.Stop();

    // Define performance baselines (these would be established from previous runs)
    auto maxCreateTime = std::chrono::milliseconds(1000); // 1 second for 100 tabs
    auto maxActivateTime = std::chrono::milliseconds(500); // 0.5 seconds for 100 activations

    verifyPerformanceConstraints(creationTimer.GetDurationMs(), maxCreateTime,
                                "Performance regression - creation");
    verifyPerformanceConstraints(activationTimer.GetDurationMs(), maxActivateTime,
                                "Performance regression - activation");
}

void DocumentTabBarPerformanceTest::testMemoryRegression()
{
    // Test that memory usage hasn't increased significantly
    size_t initialMemory = getCurrentMemoryUsage();

    const int nTabCount = 100;
    std::vector<sal_uInt16> aTabIds;
    createManyTabs(nTabCount, aTabIds);

    size_t peakMemory = getCurrentMemoryUsage();
    size_t memoryIncrease = peakMemory - initialMemory;

    // Establish memory usage baseline (5KB per tab maximum)
    size_t maxMemoryIncrease = nTabCount * 5 * 1024;

    CPPUNIT_ASSERT_MESSAGE("Memory regression detected", memoryIncrease < maxMemoryIncrease);
}

void DocumentTabBarPerformanceTest::benchmarkTabCreation()
{
    // Detailed benchmark of tab creation performance
    std::vector<int> tabCounts = {1, 10, 50, 100, 500};

    for (int nTabCount : tabCounts)
    {
        // Clean slate
        while (m_pTabBar->GetTabCount() > 0)
        {
            std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
            if (!aTabs.empty())
            {
                m_pTabBar->RemoveTab(aTabs[0].nId);
            }
        }
        CleanupMockFrames();

        // Benchmark creation
        measureOperation("Tab creation - " + std::to_string(nTabCount) + " tabs",
                        [this, nTabCount]() {
                            std::vector<sal_uInt16> aTabIds;
                            createManyTabs(nTabCount, aTabIds);
                        }, 1);
    }
}

void DocumentTabBarPerformanceTest::benchmarkTabActivation()
{
    const int nTabCount = 50;
    std::vector<sal_uInt16> aTabIds;
    createManyTabs(nTabCount, aTabIds);

    // Benchmark sequential activation
    measureOperation("Sequential tab activation", [this, &aTabIds]() {
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->SetActiveTab(nTabId);
        }
    }, 10);

    // Benchmark random activation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, aTabIds.size() - 1);

    measureOperation("Random tab activation", [this, &aTabIds, &gen, &dis]() {
        for (int i = 0; i < 50; ++i)
        {
            sal_uInt16 nTabId = aTabIds[dis(gen)];
            m_pTabBar->SetActiveTab(nTabId);
        }
    }, 10);
}

void DocumentTabBarPerformanceTest::benchmarkTabRemoval()
{
    // Benchmark different removal patterns

    // Sequential removal
    std::vector<sal_uInt16> aTabIds;
    createManyTabs(100, aTabIds);

    measureOperation("Sequential tab removal", [this, &aTabIds]() {
        for (sal_uInt16 nTabId : aTabIds)
        {
            m_pTabBar->RemoveTab(nTabId);
        }
    }, 1);

    // Random removal
    aTabIds.clear();
    createManyTabs(100, aTabIds);

    measureOperation("Random tab removal", [this, &aTabIds]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::vector<sal_uInt16> tabsCopy = aTabIds;

        while (!tabsCopy.empty())
        {
            std::uniform_int_distribution<> dis(0, tabsCopy.size() - 1);
            int index = dis(gen);
            m_pTabBar->RemoveTab(tabsCopy[index]);
            tabsCopy.erase(tabsCopy.begin() + index);
        }
    }, 1);
}

void DocumentTabBarPerformanceTest::benchmarkLayoutCalculation()
{
    const int nTabCount = 100;
    std::vector<sal_uInt16> aTabIds;
    createManyTabs(nTabCount, aTabIds);

    // Benchmark layout calculation with size changes
    measureOperation("Layout calculation - size changes", [this]() {
        for (int width = 400; width <= 1200; width += 100)
        {
            m_pTabBar->SetSizePixel(Size(width, 30));
            processEvents();
        }
    }, 10);

    // Benchmark layout with constraint changes
    measureOperation("Layout calculation - constraint changes", [this]() {
        for (int minWidth = 50; minWidth <= 150; minWidth += 25)
        {
            m_pTabBar->SetTabConstraints(minWidth, 200);
            processEvents();
        }
    }, 10);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentTabBarPerformanceTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */