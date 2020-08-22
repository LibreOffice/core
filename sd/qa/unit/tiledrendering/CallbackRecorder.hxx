/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/string.hxx>
#include <osl/conditn.hxx>
#include <sfx2/viewsh.hxx>

namespace
{
std::vector<OUString> lcl_convertSeparated(const OUString& rString, sal_Unicode nSeparator)
{
    std::vector<OUString> aRet;

    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = rString.getToken(0, nSeparator, nIndex);
        aToken = aToken.trim();
        if (!aToken.isEmpty())
            aRet.push_back(aToken);
    } while (nIndex >= 0);

    return aRet;
}

void lcl_convertRectangle(const OUString& rString, tools::Rectangle& rRectangle)
{
    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(rString);
    CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
    rRectangle.setX(aSeq[0].toInt32());
    rRectangle.setY(aSeq[1].toInt32());
    rRectangle.setWidth(aSeq[2].toInt32());
    rRectangle.setHeight(aSeq[3].toInt32());
}
}

struct CallbackRecorder
{
    CallbackRecorder()
        : m_bFound(true)
        , m_nPart(0)
        , m_nSelectionBeforeSearchResult(0)
        , m_nSelectionAfterSearchResult(0)
        , m_nSearchResultCount(0)
    {
    }

    tools::Rectangle m_aInvalidation;
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

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<CallbackRecorder*>(pData)->processCallback(nType, pPayload);
    }

    void processCallback(int nType, const char* pPayload)
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
                if (pPayload != "EMPTY")
                {
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

    void registerCallbacksFor(SfxViewShell& rViewShell)
    {
        rViewShell.registerLibreOfficeKitViewCallback(&CallbackRecorder::callback, this);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
