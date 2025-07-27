/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_QA_CPPUNIT_DOCUMENTTABBAR_MOCKS_HXX
#define INCLUDED_SFX2_QA_CPPUNIT_DOCUMENTTABBAR_MOCKS_HXX

#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <memory>
#include <vector>
#include <map>

namespace sfx2::test {

/**
 * Mock SfxMedium for testing document operations
 */
class MockSfxMedium : public SfxMedium
{
private:
    OUString m_aName;
    OUString m_aURL;
    bool m_bReadOnly;

public:
    explicit MockSfxMedium(const OUString& rName = u"MockDocument.odt"_ustr)
        : SfxMedium(), m_aName(rName), m_bReadOnly(false)
    {
        m_aURL = u"file:///" + rName;
    }

    virtual const OUString& GetName() const override { return m_aName; }
    virtual const OUString& GetURLObject() const { return m_aURL; }
    virtual bool IsReadOnly() const override { return m_bReadOnly; }

    void SetReadOnly(bool bReadOnly) { m_bReadOnly = bReadOnly; }
    void SetName(const OUString& rName) { m_aName = rName; m_aURL = u"file:///" + rName; }
};

/**
 * Mock SfxObjectShell for testing document lifecycle
 */
class MockSfxObjectShell : public SfxObjectShell
{
private:
    OUString m_aTitle;
    bool m_bModified;
    bool m_bReadOnly;
    std::unique_ptr<MockSfxMedium> m_pMedium;

    // Event tracking for tests
    mutable int m_nGetTitleCalls;
    mutable int m_nIsModifiedCalls;
    int m_nSetModifiedCalls;

public:
    explicit MockSfxObjectShell(const OUString& rTitle = u"Untitled Document"_ustr)
        : SfxObjectShell(SfxObjectCreateMode::STANDARD)
        , m_aTitle(rTitle)
        , m_bModified(false)
        , m_bReadOnly(false)
        , m_pMedium(std::make_unique<MockSfxMedium>(rTitle + u".odt"_ustr))
        , m_nGetTitleCalls(0)
        , m_nIsModifiedCalls(0)
        , m_nSetModifiedCalls(0)
    {
    }

    // SfxObjectShell interface implementation
    virtual OUString GetTitle(sal_uInt16 nMaxLen = 0) const override
    {
        ++m_nGetTitleCalls;
        if (nMaxLen > 0 && m_aTitle.getLength() > nMaxLen)
            return m_aTitle.copy(0, nMaxLen) + "...";
        return m_aTitle;
    }

    virtual bool IsModified() const override
    {
        ++m_nIsModifiedCalls;
        return m_bModified;
    }

    virtual void SetModified(bool bModified = true) override
    {
        ++m_nSetModifiedCalls;
        if (m_bModified != bModified)
        {
            m_bModified = bModified;
            // Notify listeners if needed
        }
    }

    virtual bool IsReadOnly() const override { return m_bReadOnly; }
    virtual SfxMedium* GetMedium() const override { return m_pMedium.get(); }

    // Mock-specific methods
    void SetTitle(const OUString& rTitle) { m_aTitle = rTitle; }
    void SetReadOnly(bool bReadOnly) { m_bReadOnly = bReadOnly; }
    void SetMediumName(const OUString& rName) { m_pMedium->SetName(rName); }

    // Test verification methods
    int GetTitleCallCount() const { return m_nGetTitleCalls; }
    int GetIsModifiedCallCount() const { return m_nIsModifiedCalls; }
    int GetSetModifiedCallCount() const { return m_nSetModifiedCalls; }
    void ResetCallCounts() { m_nGetTitleCalls = m_nIsModifiedCalls = m_nSetModifiedCalls = 0; }

    // Minimal required overrides
    virtual void GetState(SfxItemSet&) override {}
    virtual void ExecView(SfxRequest&) override {}
    virtual bool PrepareClose(bool bUI = true) override { return !IsModified() || !bUI; }
};

/**
 * Mock SfxFrame for testing frame operations
 */
class MockSfxFrame : public SfxFrame
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    OUString m_aName;
    bool m_bVisible;

public:
    explicit MockSfxFrame(const OUString& rName = u"MockFrame"_ustr)
        : m_aName(rName), m_bVisible(true)
    {
    }

    virtual const OUString& GetName() const { return m_aName; }
    virtual bool IsVisible() const { return m_bVisible; }

    void SetVisible(bool bVisible) { m_bVisible = bVisible; }
    void SetName(const OUString& rName) { m_aName = rName; }

    // Mock implementations of essential methods
    virtual void Appear() override { m_bVisible = true; }
    virtual void AppearWithUpdate() override { Appear(); }
};

/**
 * Mock SfxViewFrame for testing view operations
 */
class MockSfxViewFrame : public SfxViewFrame
{
private:
    std::unique_ptr<MockSfxObjectShell> m_pObjectShell;
    std::unique_ptr<MockSfxFrame> m_pFrame;
    static MockSfxViewFrame* s_pCurrent;
    static std::vector<MockSfxViewFrame*> s_aFrames;

    // Event tracking
    int m_nActivationCount;
    int m_nDeactivationCount;

public:
    explicit MockSfxViewFrame(const OUString& rDocTitle = u"Mock Document"_ustr)
        : m_pObjectShell(std::make_unique<MockSfxObjectShell>(rDocTitle))
        , m_pFrame(std::make_unique<MockSfxFrame>(rDocTitle + u" Frame"_ustr))
        , m_nActivationCount(0)
        , m_nDeactivationCount(0)
    {
        s_aFrames.push_back(this);
        if (!s_pCurrent)
            s_pCurrent = this;
    }

    virtual ~MockSfxViewFrame() override
    {
        auto it = std::find(s_aFrames.begin(), s_aFrames.end(), this);
        if (it != s_aFrames.end())
        {
            s_aFrames.erase(it);
        }
        if (s_pCurrent == this)
        {
            s_pCurrent = s_aFrames.empty() ? nullptr : s_aFrames.front();
        }
    }

    // SfxViewFrame interface
    virtual SfxObjectShell* GetObjectShell() override
    {
        return m_pObjectShell.get();
    }

    virtual SfxFrame& GetFrame() const override
    {
        return *m_pFrame;
    }

    // Mock-specific methods
    MockSfxObjectShell* GetMockObjectShell() { return m_pObjectShell.get(); }
    MockSfxFrame* GetMockFrame() { return m_pFrame.get(); }

    void Activate()
    {
        ++m_nActivationCount;
        s_pCurrent = this;
    }

    void Deactivate()
    {
        ++m_nDeactivationCount;
        if (s_pCurrent == this && !s_aFrames.empty())
        {
            s_pCurrent = (s_aFrames.size() > 1) ?
                (s_aFrames[0] == this ? s_aFrames[1] : s_aFrames[0]) : nullptr;
        }
    }

    // Test verification methods
    int GetActivationCount() const { return m_nActivationCount; }
    int GetDeactivationCount() const { return m_nDeactivationCount; }
    void ResetCounts() { m_nActivationCount = m_nDeactivationCount = 0; }

    // Static methods for managing mock frame list
    static MockSfxViewFrame* GetCurrent() { return s_pCurrent; }
    static void SetCurrent(MockSfxViewFrame* pFrame) { s_pCurrent = pFrame; }
    static const std::vector<MockSfxViewFrame*>& GetAllFrames() { return s_aFrames; }
    static void ClearAllFrames()
    {
        s_aFrames.clear();
        s_pCurrent = nullptr;
    }
    static size_t GetFrameCount() { return s_aFrames.size(); }
};

/**
 * Test fixture providing common mock setup and utilities
 */
class DocumentTabBarTestFixture
{
protected:
    std::vector<std::unique_ptr<MockSfxViewFrame>> m_aMockFrames;

    // Event simulation helpers
    class MockTimer : public Timer
    {
    private:
        std::function<void()> m_aCallback;

    public:
        explicit MockTimer(std::function<void()> aCallback)
            : m_aCallback(std::move(aCallback))
        {
        }

        virtual void Invoke() override
        {
            if (m_aCallback)
                m_aCallback();
        }
    };

public:
    DocumentTabBarTestFixture() = default;

    virtual ~DocumentTabBarTestFixture()
    {
        CleanupMockFrames();
    }

    /**
     * Create a mock view frame with specified document title
     */
    MockSfxViewFrame* CreateMockViewFrame(const OUString& rTitle = u"Test Document"_ustr)
    {
        auto pFrame = std::make_unique<MockSfxViewFrame>(rTitle);
        MockSfxViewFrame* pResult = pFrame.get();
        m_aMockFrames.push_back(std::move(pFrame));
        return pResult;
    }

    /**
     * Create multiple mock view frames with different titles
     */
    std::vector<MockSfxViewFrame*> CreateMultipleMockFrames(int nCount)
    {
        std::vector<MockSfxViewFrame*> aFrames;
        for (int i = 0; i < nCount; ++i)
        {
            OUString aTitle = u"Document "_ustr + OUString::number(i + 1);
            aFrames.push_back(CreateMockViewFrame(aTitle));
        }
        return aFrames;
    }

    /**
     * Simulate document modification
     */
    void ModifyDocument(MockSfxViewFrame* pFrame, bool bModified = true)
    {
        if (pFrame && pFrame->GetMockObjectShell())
        {
            pFrame->GetMockObjectShell()->SetModified(bModified);
        }
    }

    /**
     * Simulate title change
     */
    void ChangeDocumentTitle(MockSfxViewFrame* pFrame, const OUString& rNewTitle)
    {
        if (pFrame && pFrame->GetMockObjectShell())
        {
            pFrame->GetMockObjectShell()->SetTitle(rNewTitle);
        }
    }

    /**
     * Simulate frame activation
     */
    void ActivateFrame(MockSfxViewFrame* pFrame)
    {
        if (pFrame)
        {
            pFrame->Activate();
        }
    }

    /**
     * Clean up all mock frames
     */
    void CleanupMockFrames()
    {
        m_aMockFrames.clear();
        MockSfxViewFrame::ClearAllFrames();
    }

    /**
     * Verify frame state
     */
    void VerifyFrameState(MockSfxViewFrame* pFrame,
                         const OUString& rExpectedTitle,
                         bool bExpectedModified = false,
                         bool bShouldBeCurrent = false)
    {
        CPPUNIT_ASSERT(pFrame != nullptr);
        CPPUNIT_ASSERT(pFrame->GetMockObjectShell() != nullptr);

        CPPUNIT_ASSERT_EQUAL(rExpectedTitle, pFrame->GetMockObjectShell()->GetTitle());
        CPPUNIT_ASSERT_EQUAL(bExpectedModified, pFrame->GetMockObjectShell()->IsModified());

        if (bShouldBeCurrent)
        {
            CPPUNIT_ASSERT_EQUAL(pFrame, MockSfxViewFrame::GetCurrent());
        }
    }

    /**
     * Simulate asynchronous operation with timer
     */
    void SimulateAsyncOperation(std::function<void()> aOperation, sal_uInt64 nDelayMs = 100)
    {
        auto pTimer = std::make_unique<MockTimer>(std::move(aOperation));
        pTimer->SetTimeout(nDelayMs);
        pTimer->Start();

        // In a real test environment, we would wait for the timer
        // For unit tests, we can invoke immediately
        pTimer->Invoke();
    }

    /**
     * Create a scenario with multiple documents in different states
     */
    struct DocumentScenario
    {
        MockSfxViewFrame* pActiveDocument;
        MockSfxViewFrame* pModifiedDocument;
        MockSfxViewFrame* pReadOnlyDocument;
        std::vector<MockSfxViewFrame*> pAllDocuments;
    };

    DocumentScenario CreateMixedDocumentScenario()
    {
        DocumentScenario scenario;

        // Create active document
        scenario.pActiveDocument = CreateMockViewFrame(u"Active Document"_ustr);
        ActivateFrame(scenario.pActiveDocument);

        // Create modified document
        scenario.pModifiedDocument = CreateMockViewFrame(u"Modified Document"_ustr);
        ModifyDocument(scenario.pModifiedDocument, true);

        // Create read-only document
        scenario.pReadOnlyDocument = CreateMockViewFrame(u"ReadOnly Document"_ustr);
        scenario.pReadOnlyDocument->GetMockObjectShell()->SetReadOnly(true);

        scenario.pAllDocuments = {
            scenario.pActiveDocument,
            scenario.pModifiedDocument,
            scenario.pReadOnlyDocument
        };

        return scenario;
    }

    /**
     * Simulate rapid user operations for stress testing
     */
    void SimulateRapidOperations(int nIterations = 100)
    {
        std::vector<MockSfxViewFrame*> aFrames = CreateMultipleMockFrames(3);

        for (int i = 0; i < nIterations; ++i)
        {
            // Rapid activation changes
            MockSfxViewFrame* pFrame = aFrames[i % aFrames.size()];
            ActivateFrame(pFrame);

            // Rapid modification changes
            ModifyDocument(pFrame, (i % 2) == 0);

            // Rapid title changes
            OUString aTitle = u"Document "_ustr + OUString::number(i);
            ChangeDocumentTitle(pFrame, aTitle);
        }
    }

    /**
     * Measure performance of operations
     */
    struct PerformanceMetrics
    {
        sal_uInt64 nOperationCount;
        sal_uInt64 nTotalTimeMs;
        sal_uInt64 nAverageTimeMs;
    };

    PerformanceMetrics MeasurePerformance(std::function<void()> aOperation, int nIterations = 1000)
    {
        PerformanceMetrics metrics = {};

        auto nStartTime = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < nIterations; ++i)
        {
            aOperation();
        }

        auto nEndTime = std::chrono::high_resolution_clock::now();
        auto nDuration = std::chrono::duration_cast<std::chrono::milliseconds>(nEndTime - nStartTime);

        metrics.nOperationCount = nIterations;
        metrics.nTotalTimeMs = nDuration.count();
        metrics.nAverageTimeMs = metrics.nTotalTimeMs / nIterations;

        return metrics;
    }
};

// Static member definitions
MockSfxViewFrame* MockSfxViewFrame::s_pCurrent = nullptr;
std::vector<MockSfxViewFrame*> MockSfxViewFrame::s_aFrames;

} // namespace sfx2::test

#endif // INCLUDED_SFX2_QA_CPPUNIT_DOCUMENTTABBAR_MOCKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */