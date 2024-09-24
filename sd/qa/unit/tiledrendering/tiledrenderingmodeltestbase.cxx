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
#include <comphelper/string.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <comphelper/lok.hxx>
#include <osl/conditn.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>
#include <o3tl/string_view.hxx>

using namespace css;

class SdTiledRenderingTest : public UnoApiXmlTest
{
public:
    SdTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    SdXImpressDocument* createDoc(const char* pName,
                                  const uno::Sequence<beans::PropertyValue>& rArguments
                                  = uno::Sequence<beans::PropertyValue>());
    void setupLibreOfficeKitViewCallback(SfxViewShell& pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    xmlDocUniquePtr parseXmlDump();

    ::tools::Rectangle m_aInvalidation;
    std::vector<::tools::Rectangle> m_aSelection;
    bool m_bFound;
    sal_Int32 m_nPart;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;
    int m_nSearchResultCount;

    /// For document size changed callback.
    osl::Condition m_aDocumentSizeCondition;
    xmlBufferPtr m_pXmlBuffer;
    TestLokCallbackWrapper m_callbackWrapper;
};

SdTiledRenderingTest::SdTiledRenderingTest()
    : UnoApiXmlTest(u"/sd/qa/unit/tiledrendering/data/"_ustr)
    , m_bFound(true)
    , m_nPart(0)
    , m_nSelectionBeforeSearchResult(0)
    , m_nSelectionAfterSearchResult(0)
    , m_nSearchResultCount(0)
    , m_pXmlBuffer(nullptr)
    , m_callbackWrapper(&callback, this)
{
}

void SdTiledRenderingTest::setUp()
{
    UnoApiXmlTest::setUp();

    // prevent showing warning message box
    setenv("OOX_NO_SMARTART_WARNING", "1", 1);
    comphelper::LibreOfficeKit::setActive(true);
}

void SdTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    if (m_pXmlBuffer)
        xmlBufferFree(m_pXmlBuffer);

    m_callbackWrapper.clear();
    comphelper::LibreOfficeKit::setActive(false);

    UnoApiXmlTest::tearDown();
}

SdXImpressDocument*
SdTiledRenderingTest::createDoc(const char* pName,
                                const uno::Sequence<beans::PropertyValue>& rArguments)
{
    loadFromFile(OUString::createFromAscii(pName));
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(rArguments);
    return pImpressDocument;
}

void SdTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell& pViewShell)
{
    pViewShell.setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(&pViewShell));
}

void SdTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SdTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

std::vector<OUString> lcl_convertSeparated(std::u16string_view rString, sal_Unicode nSeparator)
{
    std::vector<OUString> aRet;

    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken(o3tl::trim(o3tl::getToken(rString, 0, nSeparator, nIndex)));
        if (!aToken.isEmpty())
            aRet.push_back(aToken);
    } while (nIndex >= 0);

    return aRet;
}

void lcl_convertRectangle(std::u16string_view rString, ::tools::Rectangle& rRectangle)
{
    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(rString);
    CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
    rRectangle.SetLeft(aSeq[0].toInt32());
    rRectangle.SetTop(aSeq[1].toInt32());
    rRectangle.setWidth(aSeq[2].toInt32());
    rRectangle.setHeight(aSeq[3].toInt32());
}

void SdTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            OUString aPayload = OUString::createFromAscii(pPayload);
            if (aPayload != "EMPTY" && m_aInvalidation.IsEmpty())
                lcl_convertRectangle(aPayload, m_aInvalidation);
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            OUString aPayload = OUString::createFromAscii(pPayload);
            m_aSelection.clear();
            for (const OUString& rString : lcl_convertSeparated(aPayload, u';'))
            {
                ::tools::Rectangle aRectangle;
                lcl_convertRectangle(rString, aRectangle);
                m_aSelection.push_back(aRectangle);
            }
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
        case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        {
            m_aDocumentSizeCondition.set();
        }
        break;
        case LOK_CALLBACK_SET_PART:
        {
            OUString aPayload = OUString::createFromAscii(pPayload);
            m_nPart = aPayload.toInt32();
        }
        break;
        case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
        {
            m_nSearchResultCount++;
            m_aSearchResultSelection.clear();
            m_aSearchResultPart.clear();
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
    }
}

xmlDocUniquePtr SdTiledRenderingTest::parseXmlDump()
{
    if (m_pXmlBuffer)
        xmlBufferFree(m_pXmlBuffer);

    // Create the xml writer.
    m_pXmlBuffer = xmlBufferCreate();
    xmlTextWriterPtr pXmlWriter = xmlNewTextWriterMemory(m_pXmlBuffer, 0);
    (void)xmlTextWriterStartDocument(pXmlWriter, nullptr, nullptr, nullptr);

    // Create the dump.
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->GetDoc()->dumpAsXml(pXmlWriter);

    // Delete the xml writer.
    (void)xmlTextWriterEndDocument(pXmlWriter);
    xmlFreeTextWriter(pXmlWriter);

    auto pCharBuffer = xmlBufferContent(m_pXmlBuffer);
    SAL_INFO("test", "SdTiledRenderingTest::parseXmlDump: pCharBuffer is '" << pCharBuffer << "'");
    return xmlDocUniquePtr(xmlParseDoc(pCharBuffer));
}

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    bool m_bGraphicSelectionInvalidated;
    bool m_bGraphicViewSelectionInvalidated;
    /// Our current part, to be able to decide if a view cursor/selection is relevant for us.
    int m_nPart;
    bool m_bCursorVisibleChanged;
    bool m_bCursorVisible;
    bool m_bViewLock;
    bool m_bTilesInvalidated;
    std::vector<tools::Rectangle> m_aInvalidations;
    std::map<int, bool> m_aViewCursorInvalidations;
    std::map<int, bool> m_aViewCursorVisibilities;
    bool m_bViewSelectionSet;
    boost::property_tree::ptree m_aCommentCallbackResult;
    OString m_ShapeSelection;
    std::vector<std::string> m_aStateChanged;
    std::map<std::string, boost::property_tree::ptree> m_aStateChanges;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback()
        : m_bGraphicSelectionInvalidated(false)
        , m_bGraphicViewSelectionInvalidated(false)
        , m_nPart(0)
        , m_bCursorVisibleChanged(false)
        , m_bCursorVisible(false)
        , m_bViewLock(false)
        , m_bTilesInvalidated(false)
        , m_bViewSelectionSet(false)
        , m_callbackWrapper(&callback, this)
    {
        mpViewShell = SfxViewShell::Current();
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
        switch (nType)
        {
            case LOK_CALLBACK_INVALIDATE_TILES:
            {
                m_bTilesInvalidated = true;
                OString text(pPayload);
                if (!text.startsWith("EMPTY"))
                {
                    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(
                        OUString::createFromAscii(pPayload));
                    CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
                    tools::Rectangle aInvalidationRect;
                    aInvalidationRect.SetLeft(aSeq[0].toInt32());
                    aInvalidationRect.SetTop(aSeq[1].toInt32());
                    aInvalidationRect.setWidth(aSeq[2].toInt32());
                    aInvalidationRect.setHeight(aSeq[3].toInt32());
                    m_aInvalidations.push_back(aInvalidationRect);
                }
            }
            break;
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            {
                m_bGraphicSelectionInvalidated = true;
                m_ShapeSelection = OString(pPayload);
            }
            break;
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                if (aTree.get_child("part").get_value<int>() == m_nPart)
                    // Ignore callbacks which are for a different part.
                    m_bGraphicViewSelectionInvalidated = true;
            }
            break;
            case LOK_CALLBACK_CURSOR_VISIBLE:
            {
                m_bCursorVisibleChanged = true;
                m_bCursorVisible = (std::string_view("true") == pPayload);
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
            case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                int nViewId = aTree.get_child("viewId").get_value<int>();
                m_aViewCursorInvalidations[nViewId] = true;
            }
            break;
            case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                const int nViewId = aTree.get_child("viewId").get_value<int>();
                m_aViewCursorVisibilities[nViewId] = std::string_view("true") == pPayload;
            }
            break;
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            {
                m_bViewSelectionSet = true;
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
            case LOK_CALLBACK_STATE_CHANGED:
            {
                std::stringstream aStream(pPayload);
                if (!aStream.str().starts_with("{"))
                {
                    m_aStateChanged.push_back(aStream.str());
                    break;
                }

                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                auto it = aTree.find("commandName");
                if (it == aTree.not_found())
                {
                    break;
                }

                std::string aCommandName = it->second.get_value<std::string>();
                m_aStateChanges[aCommandName] = aTree;
            }
            break;
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
