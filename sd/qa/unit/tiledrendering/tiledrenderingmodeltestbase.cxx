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
    bool invalidatedAll;
    int editModeOfInvalidation;
    int partOfInvalidation;

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
        , invalidatedAll(false)
        , editModeOfInvalidation(0)
        , partOfInvalidation(0)
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
                else
                {
                    editModeOfInvalidation = mpViewShell->getEditMode();
                    partOfInvalidation = mpViewShell->getPart();
                    invalidatedAll = true;
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
