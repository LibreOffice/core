/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtestviewcallback.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <cppunit/TestAssert.h>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/string.hxx>
#include <sfx2/kit/helper.hxx>

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
    mpViewShell->setCOKitViewCallback(&m_callbackWrapper);
    mnView = KitHelper::getCurrentView();
    m_callbackWrapper.setKitViewId(mnView);
}

SwTestViewCallback::~SwTestViewCallback()
{
    KitHelper::setView(mnView);
    mpViewShell->setCOKitViewCallback(nullptr);
}

void SwTestViewCallback::callback(COKitCallbackType eType, const char* pPayload, void* pData)
{
    static_cast<SwTestViewCallback*>(pData)->callbackImpl(eType, pPayload);
}

void SwTestViewCallback::callbackImpl(COKitCallbackType eType, const char* pPayload)
{
    OString aPayload(pPayload);
    m_bCalled = true;
    switch (eType)
    {
        case COKitCallbackType::STATE_CHANGED:
        {
            m_aStateChanges.push_back(OString(pPayload));
            break;
        }
        case COKitCallbackType::INVALIDATE_TILES:
        {
            m_bTilesInvalidated = true;
        }
        break;
        case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        {
            m_bOwnCursorInvalidated = true;

            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            OString sRect(aTree.get_child("rectangle").get_value<std::string>());
            m_nOwnCursorInvalidatedBy = aTree.get_child("viewId").get_value<int>();

            if (sRect == "EMPTY")
                return;
            cpo::uno::Sequence<OUString> aSeq
                = comphelper::string::convertCommaSeparated(OUString::fromUtf8(sRect));
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aOwnCursor.SetLeft(aSeq[0].toInt32());
            m_aOwnCursor.SetTop(aSeq[1].toInt32());
            m_aOwnCursor.setWidth(aSeq[2].toInt32());
            m_aOwnCursor.setHeight(aSeq[3].toInt32());
            if (m_aOwnCursor.Left() == 0 && m_aOwnCursor.Top() == 0)
                m_bOwnCursorAtOrigin = true;
        }
        break;
        case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
        {
            m_bViewCursorInvalidated = true;
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            OString aRect(aTree.get_child("rectangle").get_value<std::string>());

            cpo::uno::Sequence<OUString> aSeq
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
        case COKitCallbackType::TEXT_SELECTION:
        {
            m_bOwnSelectionSet = true;
        }
        break;
        case COKitCallbackType::TEXT_VIEW_SELECTION:
        {
            m_bViewSelectionSet = true;
            m_aViewSelection = aPayload;
        }
        break;
        case COKitCallbackType::VIEW_CURSOR_VISIBLE:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bViewCursorVisible = aTree.get_child("visible").get_value<std::string>() == "true";
        }
        break;
        case COKitCallbackType::GRAPHIC_VIEW_SELECTION:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bGraphicViewSelection
                = aTree.get_child("selection").get_value<std::string>() != "EMPTY";
        }
        break;
        case COKitCallbackType::GRAPHIC_SELECTION:
        {
            m_bGraphicSelection = aPayload != "EMPTY";
        }
        break;
        case COKitCallbackType::VIEW_LOCK:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bViewLock = aTree.get_child("rectangle").get_value<std::string>() != "EMPTY";
        }
        break;
        case COKitCallbackType::VIEW_RENDER_STATE:
        {
            m_aViewRenderState = OString(pPayload);
        }
        break;
        case COKitCallbackType::REDLINE_TABLE_SIZE_CHANGED:
        {
            m_aRedlineTableChanged.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aRedlineTableChanged);
            m_aRedlineTableChanged = m_aRedlineTableChanged.get_child("redline");
        }
        break;
        case COKitCallbackType::REDLINE_TABLE_ENTRY_MODIFIED:
        {
            m_aRedlineTableModified.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aRedlineTableModified);
            m_aRedlineTableModified = m_aRedlineTableModified.get_child("redline");
        }
        break;
        case COKitCallbackType::COMMENT:
        {
            ++m_nCommentCallbackCount;
            m_aComment.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aComment);
            m_aComment = m_aComment.get_child("comment");
        }
        break;
        case COKitCallbackType::DOCUMENT_BACKGROUND_COLOR:
        {
            m_aDocColor = aPayload;
            break;
        }
        case COKitCallbackType::EXPORT_FILE:
        {
            m_aExportFile = aPayload;
            break;
        }
        case COKitCallbackType::CURSOR_VISIBLE:
        {
            m_bCursorVisible = aPayload == "true";
            break;
        }
        default:
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
