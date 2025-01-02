/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtiledrenderingtest.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <sfx2/lokhelper.hxx>

#include <docsh.hxx>
#include <swdll.hxx>
#include <swmodule.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

using namespace css;

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
                uno::Sequence<OUString> aSeq
                    = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aRectangle));
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
                m_aCursorRectangle = tools::Rectangle(Point(aSeq[0].toInt32(), aSeq[1].toInt32()),
                                                      Size(aSeq[2].toInt32(), aSeq[3].toInt32()));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
