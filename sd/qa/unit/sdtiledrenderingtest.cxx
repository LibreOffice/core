/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sdtiledrenderingtest.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <o3tl/string_view.hxx>
#include <osl/process.h>

#include <drawdoc.hxx>
#include <unomodel.hxx>

using namespace css;

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
    osl_setEnvironment(u"OOX_NO_SMARTART_WARNING"_ustr.pData, u"1"_ustr.pData);
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

namespace
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
