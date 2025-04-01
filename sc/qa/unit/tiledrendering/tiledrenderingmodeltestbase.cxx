/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <osl/conditn.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/string.hxx>
#include <tabvwsh.hxx>
#include <test/lokcallback.hxx>
#include <docuno.hxx>
#include <vcl/scheduler.hxx>

using namespace css;

class ViewCallback;

class ScTiledRenderingTest : public UnoApiXmlTest
{
public:
    ScTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void checkSampleInvalidation(const ViewCallback& rView, bool bFullRow);
    void cellInvalidationHelper(ScModelObj* pModelObj, ScTabViewShell* pView, const ScAddress& rAdr,
                                bool bAddText, bool bFullRow);

    ScModelObj* createDoc(const char* pName);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    /// document size changed callback.
    osl::Condition m_aDocSizeCondition;
    Size m_aDocumentSize;

    TestLokCallbackWrapper m_callbackWrapper;
};

ScTiledRenderingTest::ScTiledRenderingTest()
    : UnoApiXmlTest(u"/sc/qa/unit/tiledrendering/data/"_ustr)
    , m_callbackWrapper(&callback, this)
{
}

void ScTiledRenderingTest::setUp()
{
    UnoApiXmlTest::setUp();

    comphelper::LibreOfficeKit::setActive(true);
}

void ScTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    m_callbackWrapper.clear();

    comphelper::LibreOfficeKit::resetCompatFlag();

    comphelper::LibreOfficeKit::setActive(false);

    UnoApiXmlTest::tearDown();
}

ScModelObj* ScTiledRenderingTest::createDoc(const char* pName)
{
    loadFromFile(OUString::createFromAscii(pName));

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pModelObj;
}

void ScTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell)
{
    pViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pViewShell));
}

void ScTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ScTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void ScTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
        case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        {
            OString aPayload(pPayload);
            sal_Int32 nIndex = 0;
            OString aToken = aPayload.getToken(0, ',', nIndex);
            m_aDocumentSize.setWidth(aToken.toInt32());
            aToken = aPayload.getToken(0, ',', nIndex);
            m_aDocumentSize.setHeight(aToken.toInt32());
            m_aDocSizeCondition.set();
        }
        break;
    }
}

struct EditCursorMessage final
{
    tools::Rectangle m_aRelRect;
    Point m_aRefPoint;

    void clear()
    {
        m_aRelRect.SetEmpty();
        m_aRefPoint = Point(-1, -1);
    }

    bool empty() { return m_aRelRect.IsEmpty() && m_aRefPoint.X() == -1 && m_aRefPoint.Y() == -1; }

    void parseMessage(const char* pMessage)
    {
        clear();
        if (!pMessage
            || !comphelper::LibreOfficeKit::isCompatFlagSet(
                   comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs)
            || !comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
            return;

        std::stringstream aStream(pMessage);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        std::string aVal;
        boost::property_tree::ptree::const_assoc_iterator it = aTree.find("refpoint");
        if (it != aTree.not_found())
            aVal = aTree.get_child("refpoint").get_value<std::string>();
        else
            return; // happens in testTextBoxInsert test

        uno::Sequence<OUString> aSeq
            = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aSeq.getLength());
        m_aRefPoint.setX(aSeq[0].toInt32());
        m_aRefPoint.setY(aSeq[1].toInt32());

        aVal = aTree.get_child("relrect").get_value<std::string>();
        aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aSeq.getLength());
        m_aRelRect.SetLeft(aSeq[0].toInt32());
        m_aRelRect.SetTop(aSeq[1].toInt32());
        m_aRelRect.setWidth(aSeq[2].toInt32());
        m_aRelRect.setHeight(aSeq[3].toInt32());
    }

    tools::Rectangle getBounds()
    {
        tools::Rectangle aBounds = m_aRelRect;
        aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
        return aBounds;
    }
};

struct TextSelectionMessage
{
    std::vector<tools::Rectangle> m_aRelRects;
    Point m_aRefPoint;

    void clear()
    {
        m_aRefPoint.setX(0);
        m_aRefPoint.setY(0);
        m_aRelRects.clear();
    }

    bool empty() { return m_aRelRects.empty(); }

    void parseMessage(const char* pMessage)
    {
        clear();
        if (!pMessage)
            return;

        std::string aStr(pMessage);
        if (aStr.find(",") == std::string::npos)
            return;

        size_t nRefDelimStart = aStr.find("::");
        std::string aRectListString
            = (nRefDelimStart == std::string::npos) ? aStr : aStr.substr(0, nRefDelimStart);
        std::string aRefPointString
            = (nRefDelimStart == std::string::npos)
                  ? std::string("0, 0")
                  : aStr.substr(nRefDelimStart + 2, aStr.length() - 2 - nRefDelimStart);
        uno::Sequence<OUString> aSeq
            = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aRefPointString));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aSeq.getLength());
        m_aRefPoint.setX(aSeq[0].toInt32());
        m_aRefPoint.setY(aSeq[1].toInt32());

        size_t nStart = 0;
        size_t nEnd = aRectListString.find(";");
        if (nEnd == std::string::npos)
            nEnd = aRectListString.length();
        do
        {
            std::string aRectString = aRectListString.substr(nStart, nEnd - nStart);
            {
                aSeq = comphelper::string::convertCommaSeparated(
                    OUString::createFromAscii(aRectString));
                CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aSeq.getLength());
                tools::Rectangle aRect;
                aRect.SetLeft(aSeq[0].toInt32());
                aRect.SetTop(aSeq[1].toInt32());
                aRect.setWidth(aSeq[2].toInt32());
                aRect.setHeight(aSeq[3].toInt32());

                m_aRelRects.push_back(aRect);
            }

            nStart = nEnd + 1;
            nEnd = aRectListString.find(";", nStart);
        } while (nEnd != std::string::npos);
    }

    tools::Rectangle getBounds(size_t nIndex)
    {
        if (nIndex >= m_aRelRects.size())
            return tools::Rectangle();

        tools::Rectangle aBounds = m_aRelRects[nIndex];
        aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
        return aBounds;
    }
};

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    bool m_bOwnCursorInvalidated;
    bool m_bViewCursorInvalidated;
    bool m_bTextViewSelectionInvalidated;
    bool m_bGraphicSelection;
    bool m_bGraphicViewSelection;
    bool m_bFullInvalidateTiles;
    bool m_bInvalidateTiles;
    std::vector<tools::Rectangle> m_aInvalidations;
    tools::Rectangle m_aCellCursorBounds;
    std::vector<int> m_aInvalidationsParts;
    std::vector<int> m_aInvalidationsMode;
    bool m_bViewLock;
    OString m_sCellFormula;
    boost::property_tree::ptree m_aCommentCallbackResult;
    EditCursorMessage m_aInvalidateCursorResult;
    TextSelectionMessage m_aTextSelectionResult;
    OString m_sInvalidateHeader;
    OString m_sInvalidateSheetGeometry;
    OString m_aHyperlinkClicked;
    OString m_ShapeSelection;
    std::map<std::string, boost::property_tree::ptree> m_aStateChanges;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback(bool bDeleteListenerOnDestruct = true)
        : m_bOwnCursorInvalidated(false)
        , m_bViewCursorInvalidated(false)
        , m_bTextViewSelectionInvalidated(false)
        , m_bGraphicSelection(false)
        , m_bGraphicViewSelection(false)
        , m_bFullInvalidateTiles(false)
        , m_bInvalidateTiles(false)
        , m_bViewLock(false)
        , m_callbackWrapper(&callback, this)
    {
        mpViewShell = SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId(mnView);
        if (!bDeleteListenerOnDestruct)
            mpViewShell = nullptr;
    }

    ~ViewCallback()
    {
        if (mpViewShell)
        {
            SfxLokHelper::setView(mnView);
            mpViewShell->setLibreOfficeKitViewCallback(nullptr);
        }
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        switch (nType)
        {
            case LOK_CALLBACK_CELL_CURSOR:
            {
                m_bOwnCursorInvalidated = true;
                uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(
                    OUString::createFromAscii(pPayload));
                m_aCellCursorBounds = tools::Rectangle();
                if (aSeq.getLength() == 6)
                {
                    m_aCellCursorBounds.SetLeft(aSeq[0].toInt32());
                    m_aCellCursorBounds.SetTop(aSeq[1].toInt32());
                    m_aCellCursorBounds.setWidth(aSeq[2].toInt32());
                    m_aCellCursorBounds.setHeight(aSeq[3].toInt32());
                }
            }
            break;
            case LOK_CALLBACK_CELL_VIEW_CURSOR:
            {
                m_bViewCursorInvalidated = true;
            }
            break;
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            {
                m_bTextViewSelectionInvalidated = true;
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
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            {
                m_bGraphicSelection = true;
                m_ShapeSelection = OString(pPayload);
            }
            break;
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            {
                m_bGraphicViewSelection = true;
            }
            break;
            case LOK_CALLBACK_INVALIDATE_TILES:
            {
                OString text(pPayload);
                if (text.startsWith("EMPTY"))
                {
                    m_bFullInvalidateTiles = true;
                }
                else
                {
                    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(
                        OUString::createFromAscii(pPayload));
                    CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 6);
                    tools::Rectangle aInvalidationRect;
                    aInvalidationRect.SetLeft(aSeq[0].toInt32());
                    aInvalidationRect.SetTop(aSeq[1].toInt32());
                    aInvalidationRect.setWidth(aSeq[2].toInt32());
                    aInvalidationRect.setHeight(aSeq[3].toInt32());
                    m_aInvalidations.push_back(aInvalidationRect);
                    if (aSeq.getLength() == 6)
                    {
                        m_aInvalidationsParts.push_back(aSeq[4].toInt32());
                        m_aInvalidationsMode.push_back(aSeq[5].toInt32());
                    }
                    m_bInvalidateTiles = true;
                }
            }
            break;
            case LOK_CALLBACK_CELL_FORMULA:
            {
                m_sCellFormula = pPayload;
            }
            break;
            case LOK_CALLBACK_COMMENT:
            {
                m_aCommentCallbackResult.clear();
                std::stringstream aStream(pPayload);
                boost::property_tree::read_json(aStream, m_aCommentCallbackResult);
                m_aCommentCallbackResult = m_aCommentCallbackResult.get_child("comment");
            }
            break;
            case LOK_CALLBACK_INVALIDATE_HEADER:
            {
                m_sInvalidateHeader = pPayload;
            }
            break;
            case LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
            {
                m_sInvalidateSheetGeometry = pPayload;
            }
            break;
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            {
                m_aInvalidateCursorResult.parseMessage(pPayload);
            }
            break;
            case LOK_CALLBACK_HYPERLINK_CLICKED:
            {
                m_aHyperlinkClicked = pPayload;
            }
            break;
            case LOK_CALLBACK_TEXT_SELECTION:
            {
                m_aTextSelectionResult.parseMessage(pPayload);
            }
            break;
            case LOK_CALLBACK_STATE_CHANGED:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                std::string aCommandName;

                if (aStream.str().starts_with("{"))
                {
                    boost::property_tree::read_json(aStream, aTree);
                    auto it = aTree.find("commandName");
                    if (it == aTree.not_found())
                    {
                        break;
                    }

                    aCommandName = it->second.get_value<std::string>();
                }
                else
                {
                    std::string aState = aStream.str();
                    auto it = aState.find("=");
                    if (it == std::string::npos)
                    {
                        break;
                    }
                    aCommandName = aState.substr(0, it);
                    aTree.put("state", aState.substr(it + 1));
                }

                m_aStateChanges[aCommandName] = aTree;
            }
            break;
        }
    }

    void ClearAllInvalids()
    {
        m_bInvalidateTiles = false;
        m_aInvalidations.clear();
        m_aInvalidationsParts.clear();
        m_aInvalidationsMode.clear();
    }
};

namespace
{
void lcl_typeCharsInCell(const std::string& aStr, SCCOL nCol, SCROW nRow, ScTabViewShell* pView,
                         ScModelObj* pModelObj, bool bInEdit = false, bool bCommit = true)
{
    if (!bInEdit)
        pView->SetCursor(nCol, nRow);

    for (const char& cChar : aStr)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, cChar, 0);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, cChar, 0);
        Scheduler::ProcessEventsToIdle();
    }

    if (bCommit)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    }
}
} //namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
