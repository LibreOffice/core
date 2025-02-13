/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <swmodule.hxx>
#include <swdll.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SwTiledRenderingTest : public SwModelTestBase
{
public:
    SwTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    SwXTextDocument* createDoc(const char* pName = nullptr);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    // First invalidation.
    tools::Rectangle m_aInvalidation;
    /// Union of all invalidations.
    tools::Rectangle m_aInvalidations;
    tools::Rectangle m_aCursorRectangle;
    Size m_aDocumentSize;
    OString m_aTextSelection;
    bool m_bFound;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;
    int m_nInvalidations;
    int m_nRedlineTableSizeChanged;
    int m_nRedlineTableEntryModified;
    int m_nTrackedChangeIndex;
    bool m_bFullInvalidateSeen;
    OString m_sHyperlinkText;
    OString m_sHyperlinkLink;
    OString m_aFormFieldButton;
    OString m_aContentControl;
    OString m_ShapeSelection;
    struct
    {
        std::string text;
        std::string rect;
    } m_aTooltip;
    TestLokCallbackWrapper m_callbackWrapper;
};

SwTiledRenderingTest::SwTiledRenderingTest()
    : SwModelTestBase(u"/sw/qa/extras/tiledrendering/data/"_ustr)
    , m_bFound(true)
    , m_nSelectionBeforeSearchResult(0)
    , m_nSelectionAfterSearchResult(0)
    , m_nInvalidations(0)
    , m_nRedlineTableSizeChanged(0)
    , m_nRedlineTableEntryModified(0)
    , m_nTrackedChangeIndex(-1)
    , m_bFullInvalidateSeen(false)
    , m_callbackWrapper(&callback, this)
{
}

void SwTiledRenderingTest::setUp()
{
    SwModelTestBase::setUp();

    SwGlobals::ensure();
    SwModule::get()->ClearRedlineAuthors();

    comphelper::LibreOfficeKit::setActive(true);
}

void SwTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        if (pWrtShell)
        {
            pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(nullptr);
        }
        mxComponent->dispose();
        mxComponent.clear();
    }
    m_callbackWrapper.clear();
    comphelper::LibreOfficeKit::setActive(false);

    test::BootstrapFixture::tearDown();
}

SwXTextDocument* SwTiledRenderingTest::createDoc(const char* pName)
{
    if (!pName)
        createSwDoc();
    else
        createSwDoc(pName);

    SwXTextDocument* pTextDocument = getSwTextDoc();
    pTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pTextDocument;
}

void SwTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell)
{
    pViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pViewShell));
}

void SwTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SwTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void SwTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    OString aPayload(pPayload);
    switch (nType)
    {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            tools::Rectangle aInvalidation;
            uno::Sequence<OUString> aSeq
                = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
            if (std::string_view("EMPTY") == pPayload)
            {
                m_bFullInvalidateSeen = true;
                return;
            }

            CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
            aInvalidation.SetLeft(aSeq[0].toInt32());
            aInvalidation.SetTop(aSeq[1].toInt32());
            aInvalidation.setWidth(aSeq[2].toInt32());
            aInvalidation.setHeight(aSeq[3].toInt32());
            if (m_aInvalidation.IsEmpty())
            {
                m_aInvalidation = aInvalidation;
            }
            m_aInvalidations.Union(aInvalidation);
            ++m_nInvalidations;
        }
        break;
        case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        {
            uno::Sequence<OUString> aSeq
                = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeq.getLength());
            m_aDocumentSize.setWidth(aSeq[0].toInt32());
            m_aDocumentSize.setHeight(aSeq[1].toInt32());
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            m_aTextSelection = pPayload;
            if (m_aSearchResultSelection.empty())
                ++m_nSelectionBeforeSearchResult;
            else
                ++m_nSelectionAfterSearchResult;
        }
        break;
        case LOK_CALLBACK_SEARCH_NOT_FOUND:
        {
            m_bFound = false;
        }
        break;
        case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
        {
            m_aSearchResultSelection.clear();
            boost::property_tree::ptree aTree;
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, aTree);
            for (const boost::property_tree::ptree::value_type& rValue :
                 aTree.get_child("searchResultSelection"))
            {
                m_aSearchResultSelection.emplace_back(
                    rValue.second.get<std::string>("rectangles").c_str());
                m_aSearchResultPart.push_back(
                    std::atoi(rValue.second.get<std::string>("part").c_str()));
            }
        }
        break;
        case LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
        {
            ++m_nRedlineTableSizeChanged;
        }
        break;
        case LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
        {
            ++m_nRedlineTableEntryModified;
        }
        break;
        case LOK_CALLBACK_STATE_CHANGED:
        {
            OString aTrackedChangeIndexPrefix(".uno:TrackedChangeIndex="_ostr);
            if (aPayload.startsWith(aTrackedChangeIndexPrefix))
            {
                OString sIndex = aPayload.copy(aTrackedChangeIndexPrefix.getLength());
                if (sIndex.isEmpty())
                    m_nTrackedChangeIndex = -1;
                else
                    m_nTrackedChangeIndex = sIndex.toInt32();
            }
        }
        break;
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        {
            if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
            {
                boost::property_tree::ptree aTree;
                std::stringstream aStream(pPayload);
                boost::property_tree::read_json(aStream, aTree);
                boost::property_tree::ptree& aChild = aTree.get_child("hyperlink");
                m_sHyperlinkText = OString(aChild.get("text", ""));
                m_sHyperlinkLink = OString(aChild.get("link", ""));

                OString aRectangle(aTree.get_child("rectangle").get_value<std::string>());
                uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aRectangle));
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
                m_aCursorRectangle = tools::Rectangle(Point(aSeq[0].toInt32(), aSeq[1].toInt32()), Size(aSeq[2].toInt32(), aSeq[3].toInt32()));

            }
        }
        break;
        case LOK_CALLBACK_FORM_FIELD_BUTTON:
        {
            m_aFormFieldButton = OString(pPayload);
        }
        break;
        case LOK_CALLBACK_CONTENT_CONTROL:
        {
            m_aContentControl = OString(pPayload);
        }
        break;
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        {
            m_ShapeSelection = OString(pPayload);
        }
        break;
        case LOK_CALLBACK_TOOLTIP:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_aTooltip.text = aTree.get_child("text").get_value<std::string>();
            m_aTooltip.rect = aTree.get_child("rectangle").get_value<std::string>();
        }
        break;
    }
}

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    bool m_bOwnCursorInvalidated;
    int m_nOwnCursorInvalidatedBy;
    bool m_bOwnCursorAtOrigin;
    tools::Rectangle m_aOwnCursor;
    bool m_bViewCursorInvalidated;
    tools::Rectangle m_aViewCursor;
    bool m_bOwnSelectionSet;
    bool m_bViewSelectionSet;
    OString m_aViewSelection;
    OString m_aViewRenderState;
    bool m_bTilesInvalidated;
    bool m_bViewCursorVisible;
    bool m_bGraphicViewSelection;
    bool m_bGraphicSelection;
    bool m_bViewLock;
    OString m_aDocColor;
    /// Set if any callback was invoked.
    bool m_bCalled;
    /// Redline table size changed payload
    boost::property_tree::ptree m_aRedlineTableChanged;
    /// Redline table modified payload
    boost::property_tree::ptree m_aRedlineTableModified;
    /// Post-it / annotation payload.
    boost::property_tree::ptree m_aComment;
    std::vector<OString> m_aStateChanges;
    TestLokCallbackWrapper m_callbackWrapper;
    OString m_aExportFile;

    ViewCallback(SfxViewShell* pViewShell = nullptr,
                 std::function<void(ViewCallback&)> const& rBeforeInstallFunc = {})
        : m_bOwnCursorInvalidated(false)
        , m_nOwnCursorInvalidatedBy(-1)
        , m_bOwnCursorAtOrigin(false)
        , m_bViewCursorInvalidated(false)
        , m_bOwnSelectionSet(false)
        , m_bViewSelectionSet(false)
        , m_bTilesInvalidated(false)
        , m_bViewCursorVisible(false)
        , m_bGraphicViewSelection(false)
        , m_bGraphicSelection(false)
        , m_bViewLock(false)
        , m_bCalled(false)
        , m_callbackWrapper(&callback, this)
    {
        // Because one call-site wants to set the bool fields up before the callback is installed
        if (rBeforeInstallFunc)
            rBeforeInstallFunc(*this);

        mpViewShell = pViewShell ? pViewShell : SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId(mnView);
    }

    ~ViewCallback()
    {
        SfxLokHelper::setView(mnView);
        mpViewShell->setLibreOfficeKitViewCallback(nullptr);
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        OString aPayload(pPayload);
        m_bCalled = true;
        switch (nType)
        {
            case LOK_CALLBACK_STATE_CHANGED:
            {
                m_aStateChanges.push_back(pPayload);
                break;
            }
            case LOK_CALLBACK_INVALIDATE_TILES:
            {
                m_bTilesInvalidated = true;
            }
            break;
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            {
                m_bOwnCursorInvalidated = true;

                OString sRect;
                if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
                {
                    std::stringstream aStream(pPayload);
                    boost::property_tree::ptree aTree;
                    boost::property_tree::read_json(aStream, aTree);
                    sRect = OString(aTree.get_child("rectangle").get_value<std::string>());
                    m_nOwnCursorInvalidatedBy = aTree.get_child("viewId").get_value<int>();
                }
                else
                    sRect = aPayload;
                uno::Sequence<OUString> aSeq
                    = comphelper::string::convertCommaSeparated(OUString::fromUtf8(sRect));
                if (std::string_view("EMPTY") == pPayload)
                    return;
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
                m_aOwnCursor.SetLeft(aSeq[0].toInt32());
                m_aOwnCursor.SetTop(aSeq[1].toInt32());
                m_aOwnCursor.setWidth(aSeq[2].toInt32());
                m_aOwnCursor.setHeight(aSeq[3].toInt32());
                if (m_aOwnCursor.Left() == 0 && m_aOwnCursor.Top() == 0)
                    m_bOwnCursorAtOrigin = true;
            }
            break;
            case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            {
                m_bViewCursorInvalidated = true;
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                OString aRect(aTree.get_child("rectangle").get_value<std::string>());

                uno::Sequence<OUString> aSeq
                    = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aRect));
                if (std::string_view("EMPTY") == pPayload)
                    return;
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
                m_aViewCursor.SetLeft(aSeq[0].toInt32());
                m_aViewCursor.SetTop(aSeq[1].toInt32());
                m_aViewCursor.setWidth(aSeq[2].toInt32());
                m_aViewCursor.setHeight(aSeq[3].toInt32());
            }
            break;
            case LOK_CALLBACK_TEXT_SELECTION:
            {
                m_bOwnSelectionSet = true;
            }
            break;
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            {
                m_bViewSelectionSet = true;
                m_aViewSelection = aPayload;
            }
            break;
            case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                m_bViewCursorVisible
                    = aTree.get_child("visible").get_value<std::string>() == "true";
            }
            break;
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                m_bGraphicViewSelection
                    = aTree.get_child("selection").get_value<std::string>() != "EMPTY";
            }
            break;
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            {
                m_bGraphicSelection = aPayload != "EMPTY";
            }
            break;
            case LOK_CALLBACK_VIEW_LOCK:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                m_bViewLock = aTree.get_child("rectangle").get_value<std::string>() != "EMPTY";
            }
            break;
            case LOK_CALLBACK_VIEW_RENDER_STATE:
            {
                m_aViewRenderState = pPayload;
            }
            break;
            case LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
            {
                m_aRedlineTableChanged.clear();
                std::stringstream aStream(pPayload);
                boost::property_tree::read_json(aStream, m_aRedlineTableChanged);
                m_aRedlineTableChanged = m_aRedlineTableChanged.get_child("redline");
            }
            break;
            case LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
            {
                m_aRedlineTableModified.clear();
                std::stringstream aStream(pPayload);
                boost::property_tree::read_json(aStream, m_aRedlineTableModified);
                m_aRedlineTableModified = m_aRedlineTableModified.get_child("redline");
            }
            break;
            case LOK_CALLBACK_COMMENT:
            {
                m_aComment.clear();
                std::stringstream aStream(pPayload);
                boost::property_tree::read_json(aStream, m_aComment);
                m_aComment = m_aComment.get_child("comment");
            }
            break;
            case LOK_CALLBACK_DOCUMENT_BACKGROUND_COLOR:
            {
                m_aDocColor = aPayload;
                break;
            }
            case LOK_CALLBACK_EXPORT_FILE:
            {
                m_aExportFile = aPayload;
                break;
            }
        }
    }
};

class TestResultListener : public cppu::WeakImplHelper<css::frame::XDispatchResultListener>
{
public:
    sal_uInt32 m_nDocRepair;

    TestResultListener()
        : m_nDocRepair(0)
    {
    }

    virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& rEvent) override
    {
        if (rEvent.State == frame::DispatchResultState::SUCCESS)
        {
            rEvent.Result >>= m_nDocRepair;
        }
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}
};

/// Test callback that works with comphelper::LibreOfficeKit::setAnyInputCallback().
class AnyInputCallback final
{
public:
    static bool callback(void* /*pData*/, int /*nPriority*/) { return true; }

    AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(&callback, this,
                                                        []() -> int { return -1; });
    }

    ~AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(nullptr, nullptr,
                                                        []() -> int { return -1; });
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
