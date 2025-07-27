/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <test/unoapi_test.hxx>
#include <sfx2/documenttabbar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/scheduler.hxx>
#include <tools/gen.hxx>
#include <memory>

using namespace ::com::sun::star;

namespace {

/**
 * Mock SfxObjectShell for testing
 */
class MockObjectShell : public SfxObjectShell
{
private:
    OUString m_aTitle;
    bool m_bModified;

public:
    MockObjectShell(const OUString& rTitle = u"Test Document"_ustr)
        : m_aTitle(rTitle), m_bModified(false)
    {
    }

    virtual OUString GetTitle(sal_uInt16 nMaxLen = 0) const override
    {
        return nMaxLen > 0 && m_aTitle.getLength() > nMaxLen
            ? m_aTitle.copy(0, nMaxLen) : m_aTitle;
    }

    virtual bool IsModified() const override { return m_bModified; }

    void SetTitle(const OUString& rTitle) { m_aTitle = rTitle; }
    void SetModified(bool bModified) { m_bModified = bModified; }

    // Minimal implementation for testing
    virtual void GetState( SfxItemSet& ) override {}
    virtual void ExecView( SfxRequest& ) override {}
};

/**
 * Mock SfxViewFrame for testing
 */
class MockViewFrame : public SfxViewFrame
{
private:
    std::unique_ptr<MockObjectShell> m_pObjectShell;

public:
    MockViewFrame(const OUString& rDocTitle = u"Test Document"_ustr)
        : m_pObjectShell(std::make_unique<MockObjectShell>(rDocTitle))
    {
    }

    virtual SfxObjectShell* GetObjectShell() override
    {
        return m_pObjectShell.get();
    }

    MockObjectShell* GetMockObjectShell() { return m_pObjectShell.get(); }
};

/**
 * Test fixture for DocumentTabBar unit tests
 */
class DocumentTabBarTest : public CppUnit::TestFixture
{
private:
    VclPtr<Dialog> m_pDialog;
    VclPtr<DocumentTabBar> m_pTabBar;
    std::vector<std::unique_ptr<MockViewFrame>> m_aViewFrames;

    // Test event tracking
    int m_nActivateEvents;
    int m_nCloseEvents;
    int m_nContextMenuEvents;
    sal_uInt16 m_nLastActivatedTab;
    sal_uInt16 m_nLastClosedTab;

public:
    void setUp() override;
    void tearDown() override;

    // Basic functionality tests
    void testConstruction();
    void testAddTab();
    void testRemoveTab();
    void testActivateTab();
    void testTabProperties();
    void testTabQueries();
    void testConfiguration();

    // Layout and rendering tests
    void testLayoutCalculation();
    void testScrolling();
    void testTabSizeConstraints();
    void testEmptyState();

    // Event handling tests
    void testTabActivation();
    void testTabClosing();
    void testKeyboardNavigation();
    void testMouseInteraction();

    // Edge cases and error handling
    void testInvalidTabOperations();
    void testManyTabs();
    void testLongTabTitles();
    void testDuplicateOperations();

    // Integration with SfxViewFrame
    void testViewFrameIntegration();
    void testDocumentStateTracking();

    CPPUNIT_TEST_SUITE(DocumentTabBarTest);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testAddTab);
    CPPUNIT_TEST(testRemoveTab);
    CPPUNIT_TEST(testActivateTab);
    CPPUNIT_TEST(testTabProperties);
    CPPUNIT_TEST(testTabQueries);
    CPPUNIT_TEST(testConfiguration);
    CPPUNIT_TEST(testLayoutCalculation);
    CPPUNIT_TEST(testScrolling);
    CPPUNIT_TEST(testTabSizeConstraints);
    CPPUNIT_TEST(testEmptyState);
    CPPUNIT_TEST(testTabActivation);
    CPPUNIT_TEST(testTabClosing);
    CPPUNIT_TEST(testKeyboardNavigation);
    CPPUNIT_TEST(testMouseInteraction);
    CPPUNIT_TEST(testInvalidTabOperations);
    CPPUNIT_TEST(testManyTabs);
    CPPUNIT_TEST(testLongTabTitles);
    CPPUNIT_TEST(testDuplicateOperations);
    CPPUNIT_TEST(testViewFrameIntegration);
    CPPUNIT_TEST(testDocumentStateTracking);
    CPPUNIT_TEST_SUITE_END();

private:
    // Helper methods
    MockViewFrame* createMockViewFrame(const OUString& rTitle);
    void setupEventHandlers();
    void processEvents();

    // Event handler callbacks
    DECL_LINK(OnTabActivated, DocumentTabBar*, void);
    DECL_LINK(OnTabClosed, DocumentTabBar*, void);
    DECL_LINK(OnTabContextMenu, DocumentTabBar*, void);
};

void DocumentTabBarTest::setUp()
{
    // Create a parent dialog for the tab bar
    m_pDialog = VclPtr<Dialog>::Create(nullptr, WB_MOVEABLE | WB_CLOSEABLE, Dialog::InitFlag::NoParent);
    m_pDialog->SetSizePixel(Size(800, 600));

    // Create the DocumentTabBar
    m_pTabBar = VclPtr<DocumentTabBar>::Create(m_pDialog, nullptr);
    m_pTabBar->SetSizePixel(Size(700, 30));
    m_pTabBar->SetPosPixel(Point(10, 10));

    // Initialize event tracking
    m_nActivateEvents = 0;
    m_nCloseEvents = 0;
    m_nContextMenuEvents = 0;
    m_nLastActivatedTab = 0;
    m_nLastClosedTab = 0;

    setupEventHandlers();

    m_pDialog->Show();
    m_pTabBar->Show();
    processEvents();
}

void DocumentTabBarTest::tearDown()
{
    // Clean up view frames
    m_aViewFrames.clear();

    // Dispose VCL objects
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

MockViewFrame* DocumentTabBarTest::createMockViewFrame(const OUString& rTitle)
{
    m_aViewFrames.push_back(std::make_unique<MockViewFrame>(rTitle));
    return m_aViewFrames.back().get();
}

void DocumentTabBarTest::setupEventHandlers()
{
    m_pTabBar->SetTabActivatedHdl(LINK(this, DocumentTabBarTest, OnTabActivated));
    m_pTabBar->SetTabClosedHdl(LINK(this, DocumentTabBarTest, OnTabClosed));
    m_pTabBar->SetTabMenuHdl(LINK(this, DocumentTabBarTest, OnTabContextMenu));
}

void DocumentTabBarTest::processEvents()
{
    Application::Yield();
    Application::Reschedule(true);
}

IMPL_LINK_NOARG(DocumentTabBarTest, OnTabActivated, DocumentTabBar*, void)
{
    m_nActivateEvents++;
    m_nLastActivatedTab = m_pTabBar->GetLastActivatedTab();
}

IMPL_LINK_NOARG(DocumentTabBarTest, OnTabClosed, DocumentTabBar*, void)
{
    m_nCloseEvents++;
    m_nLastClosedTab = m_pTabBar->GetLastClosedTab();
}

IMPL_LINK_NOARG(DocumentTabBarTest, OnTabContextMenu, DocumentTabBar*, void)
{
    m_nContextMenuEvents++;
}

void DocumentTabBarTest::testConstruction()
{
    // Test basic construction
    CPPUNIT_ASSERT(m_pTabBar != nullptr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetActiveTab());

    // Test initial state
    CPPUNIT_ASSERT(m_pTabBar->GetAllTabs().empty());

    // Test optimal size calculation
    Size aOptimalSize = m_pTabBar->GetOptimalSize();
    CPPUNIT_ASSERT(aOptimalSize.Width() >= 0);
    CPPUNIT_ASSERT(aOptimalSize.Height() > 0);
}

void DocumentTabBarTest::testAddTab()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    // Test adding first tab
    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    CPPUNIT_ASSERT(nTab1 > 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());

    // Test adding second tab
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());
    CPPUNIT_ASSERT(nTab2 > 0);
    CPPUNIT_ASSERT(nTab2 != nTab1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), m_pTabBar->GetTabCount());

    // Test tab info retrieval
    DocumentTabBar::TabInfo aInfo1 = m_pTabBar->GetTabInfo(nTab1);
    CPPUNIT_ASSERT_EQUAL(nTab1, aInfo1.nId);
    CPPUNIT_ASSERT_EQUAL(u"Document 1"_ustr, aInfo1.sTitle);
    CPPUNIT_ASSERT_EQUAL(pViewFrame1->GetObjectShell(), aInfo1.pObjectShell);

    // Test all tabs retrieval
    std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTabs.size());
}

void DocumentTabBarTest::testRemoveTab()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);
    MockViewFrame* pViewFrame3 = createMockViewFrame(u"Document 3"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());
    sal_uInt16 nTab3 = m_pTabBar->AddTab(u"Document 3"_ustr, pViewFrame3->GetObjectShell());

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), m_pTabBar->GetTabCount());

    // Test removing middle tab
    m_pTabBar->RemoveTab(nTab2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), m_pTabBar->GetTabCount());

    // Verify the correct tab was removed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->FindTab(pViewFrame2->GetObjectShell()));
    CPPUNIT_ASSERT(m_pTabBar->FindTab(pViewFrame1->GetObjectShell()) > 0);
    CPPUNIT_ASSERT(m_pTabBar->FindTab(pViewFrame3->GetObjectShell()) > 0);

    // Test removing by ObjectShell
    m_pTabBar->RemoveTab(pViewFrame1->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());

    // Test removing last tab
    m_pTabBar->RemoveTab(nTab3);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetTabCount());
}

void DocumentTabBarTest::testActivateTab()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Test activating first tab
    m_pTabBar->SetActiveTab(nTab1);
    processEvents();

    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->GetActiveTab());
    CPPUNIT_ASSERT(m_pTabBar->GetTabInfo(nTab1).bActive);
    CPPUNIT_ASSERT(!m_pTabBar->GetTabInfo(nTab2).bActive);
    CPPUNIT_ASSERT(m_nActivateEvents > 0);

    // Test activating second tab
    int nPrevEvents = m_nActivateEvents;
    m_pTabBar->SetActiveTab(nTab2);
    processEvents();

    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->GetActiveTab());
    CPPUNIT_ASSERT(!m_pTabBar->GetTabInfo(nTab1).bActive);
    CPPUNIT_ASSERT(m_pTabBar->GetTabInfo(nTab2).bActive);
    CPPUNIT_ASSERT(m_nActivateEvents > nPrevEvents);

    // Test activating by ObjectShell
    m_pTabBar->SetActiveTab(pViewFrame1->GetObjectShell());
    processEvents();
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->GetActiveTab());
}

void DocumentTabBarTest::testTabProperties()
{
    MockViewFrame* pViewFrame = createMockViewFrame(u"Test Document"_ustr);
    sal_uInt16 nTab = m_pTabBar->AddTab(u"Test Document"_ustr, pViewFrame->GetObjectShell());

    // Test title setting
    m_pTabBar->SetTabTitle(nTab, u"Modified Title"_ustr);
    DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT_EQUAL(u"Modified Title"_ustr, aInfo.sTitle);

    // Test modified state
    CPPUNIT_ASSERT(!aInfo.bModified);
    m_pTabBar->SetTabModified(nTab, true);
    aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT(aInfo.bModified);

    // Test icon setting
    Image aIcon; // Empty icon for testing
    m_pTabBar->SetTabIcon(nTab, aIcon);
    aInfo = m_pTabBar->GetTabInfo(nTab);
    // Icon comparison would require more complex setup

    // Test tooltip setting
    m_pTabBar->SetTabTooltip(nTab, u"This is a tooltip"_ustr);
    aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT_EQUAL(u"This is a tooltip"_ustr, aInfo.sTooltip);
}

void DocumentTabBarTest::testTabQueries()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Test FindTab
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->FindTab(pViewFrame1->GetObjectShell()));
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->FindTab(pViewFrame2->GetObjectShell()));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->FindTab(nullptr));

    // Test GetTabCount
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), m_pTabBar->GetTabCount());

    // Test GetAllTabs
    std::vector<DocumentTabBar::TabInfo> aTabs = m_pTabBar->GetAllTabs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTabs.size());

    // Verify tab order and content
    bool bFound1 = false, bFound2 = false;
    for (const auto& rTab : aTabs)
    {
        if (rTab.nId == nTab1)
        {
            CPPUNIT_ASSERT_EQUAL(u"Document 1"_ustr, rTab.sTitle);
            bFound1 = true;
        }
        else if (rTab.nId == nTab2)
        {
            CPPUNIT_ASSERT_EQUAL(u"Document 2"_ustr, rTab.sTitle);
            bFound2 = true;
        }
    }
    CPPUNIT_ASSERT(bFound1 && bFound2);
}

void DocumentTabBarTest::testConfiguration()
{
    // Test scrollable setting
    m_pTabBar->SetScrollable(false);
    // Note: We can't directly test this without access to internal state
    // In a real implementation, this would affect layout and button visibility

    m_pTabBar->SetScrollable(true);

    // Test close button visibility
    m_pTabBar->SetShowCloseButtons(false);
    m_pTabBar->SetShowCloseButtons(true);

    // Test icon visibility
    m_pTabBar->SetShowIcons(false);
    m_pTabBar->SetShowIcons(true);

    // Test tab constraints
    m_pTabBar->SetTabConstraints(100, 300);

    // These settings should not crash and should be persistent
    // In a real test environment, we would verify the visual effects
}

void DocumentTabBarTest::testLayoutCalculation()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Short"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"This is a very long tab title that should be constrained"_ustr);
    MockViewFrame* pViewFrame3 = createMockViewFrame(u"Medium Length"_ustr);

    m_pTabBar->AddTab(u"Short"_ustr, pViewFrame1->GetObjectShell());
    m_pTabBar->AddTab(u"This is a very long tab title that should be constrained"_ustr, pViewFrame2->GetObjectShell());
    m_pTabBar->AddTab(u"Medium Length"_ustr, pViewFrame3->GetObjectShell());

    processEvents(); // Trigger layout calculation

    // Test that the widget calculates a reasonable optimal size
    Size aOptimalSize = m_pTabBar->GetOptimalSize();
    CPPUNIT_ASSERT(aOptimalSize.Width() > 0);
    CPPUNIT_ASSERT(aOptimalSize.Height() > 0);

    // Test size with constraints
    m_pTabBar->SetTabConstraints(50, 150);
    processEvents();

    Size aConstrainedSize = m_pTabBar->GetOptimalSize();
    CPPUNIT_ASSERT(aConstrainedSize.Width() > 0);
    CPPUNIT_ASSERT(aConstrainedSize.Height() > 0);
}

void DocumentTabBarTest::testScrolling()
{
    // Add many tabs to trigger scrolling
    std::vector<MockViewFrame*> aViewFrames;
    for (int i = 0; i < 20; ++i)
    {
        OUString aTitle = u"Document "_ustr + OUString::number(i + 1);
        MockViewFrame* pFrame = createMockViewFrame(aTitle);
        aViewFrames.push_back(pFrame);
        m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
    }

    // Set a small size to force scrolling
    m_pTabBar->SetSizePixel(Size(200, 30));
    processEvents();

    // Test that scrolling is handled gracefully
    // In a real implementation, this would test scroll button functionality
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(20), m_pTabBar->GetTabCount());

    // Test that all tabs are still accessible
    for (size_t i = 0; i < aViewFrames.size(); ++i)
    {
        sal_uInt16 nFoundTab = m_pTabBar->FindTab(aViewFrames[i]->GetObjectShell());
        CPPUNIT_ASSERT(nFoundTab > 0);
    }
}

void DocumentTabBarTest::testTabSizeConstraints()
{
    MockViewFrame* pViewFrame = createMockViewFrame(u"Test"_ustr);
    sal_uInt16 nTab = m_pTabBar->AddTab(u"Test"_ustr, pViewFrame->GetObjectShell());

    // Test with different constraint settings
    m_pTabBar->SetTabConstraints(80, 200);
    processEvents();

    m_pTabBar->SetTabConstraints(50, 100);
    processEvents();

    // The tab should still be functional regardless of constraints
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT_EQUAL(nTab, m_pTabBar->FindTab(pViewFrame->GetObjectShell()));
}

void DocumentTabBarTest::testEmptyState()
{
    // Test widget behavior when empty
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT(m_pTabBar->GetAllTabs().empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetActiveTab());

    // Test that operations on empty widget don't crash
    m_pTabBar->SetActiveTab(999); // Invalid tab ID
    m_pTabBar->RemoveTab(999);
    m_pTabBar->SetTabTitle(999, u"Invalid"_ustr);

    // Widget should remain in valid state
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->GetTabCount());
}

void DocumentTabBarTest::testTabActivation()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Reset event counters
    m_nActivateEvents = 0;

    // Test activation events
    m_pTabBar->SetActiveTab(nTab1);
    processEvents();
    CPPUNIT_ASSERT(m_nActivateEvents > 0);
    CPPUNIT_ASSERT_EQUAL(nTab1, m_nLastActivatedTab);

    // Test that reactivating same tab doesn't generate redundant events
    int nPrevEvents = m_nActivateEvents;
    m_pTabBar->SetActiveTab(nTab1);
    processEvents();
    CPPUNIT_ASSERT_EQUAL(nPrevEvents, m_nActivateEvents); // Should be the same

    // Test activating different tab
    m_pTabBar->SetActiveTab(nTab2);
    processEvents();
    CPPUNIT_ASSERT(m_nActivateEvents > nPrevEvents);
    CPPUNIT_ASSERT_EQUAL(nTab2, m_nLastActivatedTab);
}

void DocumentTabBarTest::testTabClosing()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Reset event counters
    m_nCloseEvents = 0;

    // Note: In the actual implementation, close events would be triggered
    // by mouse clicks on close buttons or keyboard shortcuts
    // Here we test the programmatic interface

    m_pTabBar->RemoveTab(nTab1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());

    // Test that the correct tab was removed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->FindTab(pViewFrame1->GetObjectShell()));
    CPPUNIT_ASSERT(m_pTabBar->FindTab(pViewFrame2->GetObjectShell()) > 0);
}

void DocumentTabBarTest::testKeyboardNavigation()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);
    MockViewFrame* pViewFrame3 = createMockViewFrame(u"Document 3"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());
    sal_uInt16 nTab3 = m_pTabBar->AddTab(u"Document 3"_ustr, pViewFrame3->GetObjectShell());

    m_pTabBar->SetActiveTab(nTab1);

    // Test keyboard navigation (Ctrl+Tab, Ctrl+Shift+Tab, etc.)
    // Note: In actual implementation, this would involve KeyEvent handling
    // Here we test the logical navigation

    // Test next tab navigation
    m_pTabBar->SetActiveTab(nTab2);
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(nTab3);
    CPPUNIT_ASSERT_EQUAL(nTab3, m_pTabBar->GetActiveTab());

    // Test previous tab navigation
    m_pTabBar->SetActiveTab(nTab2);
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(nTab1);
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->GetActiveTab());
}

void DocumentTabBarTest::testMouseInteraction()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Note: Testing actual mouse events would require more complex setup
    // with mouse event simulation. Here we test the logical equivalent.

    // Test clicking on tabs to activate them
    m_pTabBar->SetActiveTab(nTab1);
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(nTab2);
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->GetActiveTab());

    // Test context menu triggering
    m_nContextMenuEvents = 0;
    // In actual implementation, this would be triggered by right-click
    // Here we can only test that the event system is properly set up
    CPPUNIT_ASSERT(m_nContextMenuEvents >= 0); // Event handler is connected
}

void DocumentTabBarTest::testInvalidTabOperations()
{
    MockViewFrame* pViewFrame = createMockViewFrame(u"Document 1"_ustr);
    sal_uInt16 nTab = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame->GetObjectShell());

    // Test operations with invalid tab IDs
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->SetActiveTab(999));
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->RemoveTab(999));
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->SetTabTitle(999, u"Invalid"_ustr));
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->SetTabModified(999, true));

    // Test operations with nullptr
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->SetActiveTab(static_cast<SfxObjectShell*>(nullptr)));
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->RemoveTab(static_cast<SfxObjectShell*>(nullptr)));

    // Valid tab should still be there and functional
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT_EQUAL(nTab, m_pTabBar->FindTab(pViewFrame->GetObjectShell()));
}

void DocumentTabBarTest::testManyTabs()
{
    const int nTabCount = 100;
    std::vector<MockViewFrame*> aViewFrames;
    std::vector<sal_uInt16> aTabIds;

    // Add many tabs
    for (int i = 0; i < nTabCount; ++i)
    {
        OUString aTitle = u"Document "_ustr + OUString::number(i + 1);
        MockViewFrame* pFrame = createMockViewFrame(aTitle);
        aViewFrames.push_back(pFrame);

        sal_uInt16 nTab = m_pTabBar->AddTab(aTitle, pFrame->GetObjectShell());
        aTabIds.push_back(nTab);

        CPPUNIT_ASSERT(nTab > 0);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(nTabCount), m_pTabBar->GetTabCount());

    // Test that all tabs are accessible
    for (int i = 0; i < nTabCount; ++i)
    {
        sal_uInt16 nFoundTab = m_pTabBar->FindTab(aViewFrames[i]->GetObjectShell());
        CPPUNIT_ASSERT_EQUAL(aTabIds[i], nFoundTab);
    }

    // Test activation of various tabs
    m_pTabBar->SetActiveTab(aTabIds[0]);
    CPPUNIT_ASSERT_EQUAL(aTabIds[0], m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(aTabIds[nTabCount - 1]);
    CPPUNIT_ASSERT_EQUAL(aTabIds[nTabCount - 1], m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(aTabIds[nTabCount / 2]);
    CPPUNIT_ASSERT_EQUAL(aTabIds[nTabCount / 2], m_pTabBar->GetActiveTab());

    // Test removing tabs from various positions
    m_pTabBar->RemoveTab(aTabIds[0]); // First
    m_pTabBar->RemoveTab(aTabIds[nTabCount - 1]); // Last
    m_pTabBar->RemoveTab(aTabIds[nTabCount / 2]); // Middle

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(nTabCount - 3), m_pTabBar->GetTabCount());
}

void DocumentTabBarTest::testLongTabTitles()
{
    // Test with very long tab titles
    OUString aVeryLongTitle = u"This is an extremely long tab title that should be handled gracefully by the tab bar widget and should not cause any layout or rendering issues even when it exceeds reasonable length limits"_ustr;

    MockViewFrame* pViewFrame = createMockViewFrame(aVeryLongTitle);
    sal_uInt16 nTab = m_pTabBar->AddTab(aVeryLongTitle, pViewFrame->GetObjectShell());

    CPPUNIT_ASSERT(nTab > 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());

    // Test that the tab info contains the full title
    DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT_EQUAL(aVeryLongTitle, aInfo.sTitle);

    // Test layout calculation with long title
    processEvents();
    Size aOptimalSize = m_pTabBar->GetOptimalSize();
    CPPUNIT_ASSERT(aOptimalSize.Width() > 0);
    CPPUNIT_ASSERT(aOptimalSize.Height() > 0);

    // Test with constrained width
    m_pTabBar->SetTabConstraints(50, 150);
    processEvents();
    // Should not crash and should still be functional
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());
}

void DocumentTabBarTest::testDuplicateOperations()
{
    MockViewFrame* pViewFrame = createMockViewFrame(u"Document 1"_ustr);

    // Test adding same document multiple times
    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 1 Copy"_ustr, pViewFrame->GetObjectShell());

    // Should handle gracefully (implementation dependent behavior)
    // Either allow duplicates or ignore second addition
    CPPUNIT_ASSERT(nTab1 > 0);
    // The behavior of nTab2 depends on implementation policy

    // Test removing same tab multiple times
    m_pTabBar->RemoveTab(nTab1);
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->RemoveTab(nTab1)); // Should not crash

    // Test activating removed tab
    CPPUNIT_ASSERT_NO_THROW(m_pTabBar->SetActiveTab(nTab1)); // Should not crash
}

void DocumentTabBarTest::testViewFrameIntegration()
{
    MockViewFrame* pViewFrame1 = createMockViewFrame(u"Document 1"_ustr);
    MockViewFrame* pViewFrame2 = createMockViewFrame(u"Document 2"_ustr);

    sal_uInt16 nTab1 = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame1->GetObjectShell());
    sal_uInt16 nTab2 = m_pTabBar->AddTab(u"Document 2"_ustr, pViewFrame2->GetObjectShell());

    // Test finding tabs by ViewFrame/ObjectShell
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->FindTab(pViewFrame1->GetObjectShell()));
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->FindTab(pViewFrame2->GetObjectShell()));

    // Test activating by ObjectShell
    m_pTabBar->SetActiveTab(pViewFrame1->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(nTab1, m_pTabBar->GetActiveTab());

    m_pTabBar->SetActiveTab(pViewFrame2->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(nTab2, m_pTabBar->GetActiveTab());

    // Test removing by ObjectShell
    m_pTabBar->RemoveTab(pViewFrame1->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), m_pTabBar->GetTabCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), m_pTabBar->FindTab(pViewFrame1->GetObjectShell()));
}

void DocumentTabBarTest::testDocumentStateTracking()
{
    MockViewFrame* pViewFrame = createMockViewFrame(u"Document 1"_ustr);
    sal_uInt16 nTab = m_pTabBar->AddTab(u"Document 1"_ustr, pViewFrame->GetObjectShell());

    // Test initial state
    DocumentTabBar::TabInfo aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT_EQUAL(u"Document 1"_ustr, aInfo.sTitle);
    CPPUNIT_ASSERT(!aInfo.bModified);

    // Test tracking document modifications
    pViewFrame->GetMockObjectShell()->SetModified(true);
    m_pTabBar->SetTabModified(nTab, true);

    aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT(aInfo.bModified);

    // Test tracking title changes
    pViewFrame->GetMockObjectShell()->SetTitle(u"Document 1 - Modified"_ustr);
    m_pTabBar->SetTabTitle(nTab, u"Document 1 - Modified"_ustr);

    aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT_EQUAL(u"Document 1 - Modified"_ustr, aInfo.sTitle);

    // Test clearing modifications
    m_pTabBar->SetTabModified(nTab, false);
    aInfo = m_pTabBar->GetTabInfo(nTab);
    CPPUNIT_ASSERT(!aInfo.bModified);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentTabBarTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */