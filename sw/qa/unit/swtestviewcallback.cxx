/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtestviewcallback.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <cppunit/TestAssert.h>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <sfx2/lokhelper.hxx>

using namespace css;

SwTestViewCallback::SwTestViewCallback(
    SfxViewShell* pViewShell, std::function<void(SwTestViewCallback&)> const& rBeforeInstallFunc)
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

SwTestViewCallback::~SwTestViewCallback()
{
    SfxLokHelper::setView(mnView);
    mpViewShell->setLibreOfficeKitViewCallback(nullptr);
}

void SwTestViewCallback::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SwTestViewCallback*>(pData)->callbackImpl(nType, pPayload);
}

void SwTestViewCallback::callbackImpl(int nType, const char* pPayload)
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
            m_bViewCursorVisible = aTree.get_child("visible").get_value<std::string>() == "true";
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
        case LOK_CALLBACK_CURSOR_VISIBLE:
        {
            m_bCursorVisible = aPayload == "true";
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
