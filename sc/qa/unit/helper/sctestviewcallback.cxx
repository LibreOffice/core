/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sctestviewcallback.hxx"

#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <test/unoapixml_test.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>

using namespace com::sun::star;

void ScTestEditCursorMessage::clear()
{
    m_aRelRect.SetEmpty();
    m_aRefPoint = Point(-1, -1);
}

bool ScTestEditCursorMessage::empty()
{
    return m_aRelRect.IsEmpty() && m_aRefPoint.X() == -1 && m_aRefPoint.Y() == -1;
}

void ScTestEditCursorMessage::parseMessage(const char* pMessage)
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

tools::Rectangle ScTestEditCursorMessage::getBounds()
{
    tools::Rectangle aBounds = m_aRelRect;
    aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
    return aBounds;
}

void ScTestTextSelectionMessage::clear()
{
    m_aRefPoint.setX(0);
    m_aRefPoint.setY(0);
    m_aRelRects.clear();
}

bool ScTestTextSelectionMessage::empty() { return m_aRelRects.empty(); }

void ScTestTextSelectionMessage::parseMessage(const char* pMessage)
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
            aSeq
                = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aRectString));
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

tools::Rectangle ScTestTextSelectionMessage::getBounds(size_t nIndex)
{
    if (nIndex >= m_aRelRects.size())
        return tools::Rectangle();

    tools::Rectangle aBounds = m_aRelRects[nIndex];
    aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
    return aBounds;
}

ScTestViewCallback::ScTestViewCallback(bool bDeleteListenerOnDestruct)
    : m_bOwnCursorInvalidated(false)
    , m_bViewCursorInvalidated(false)
    , m_textCursorVisible(false)
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
    mnView = SfxLokHelper::getCurrentView();
    m_callbackWrapper.setLOKViewId(mnView);
    if (!bDeleteListenerOnDestruct)
        mpViewShell = nullptr;
}

ScTestViewCallback::~ScTestViewCallback()
{
    if (mpViewShell)
    {
        SfxLokHelper::setView(mnView);
        mpViewShell->setLibreOfficeKitViewCallback(nullptr);
    }
}

void ScTestViewCallback::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ScTestViewCallback*>(pData)->callbackImpl(nType, pPayload);
}

void ScTestViewCallback::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        {
            boost::property_tree::ptree aTree;
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, aTree);
            m_textCursorVisible = aTree.get_child("visible").get_value<std::string>() == "true";
        }
        break;
        case LOK_CALLBACK_CELL_CURSOR:
        {
            m_bOwnCursorInvalidated = true;
            uno::Sequence<OUString> aSeq
                = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
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
        case LOK_CALLBACK_JSDIALOG:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            if (aTree.get_child("jsontype").get_value<std::string>() == "formulabar")
            {
                if (aTree.find("data") != aTree.not_found())
                {
                    if (aTree.get_child("data").find("separator")
                        != aTree.get_child("data").not_found())
                    {
                        decimalSeparator = aTree.get_child("data")
                                               .get_child("separator")
                                               .get_value<std::string>();
                    }
                }
            }
        }
        break;
    }
}

void ScTestViewCallback::ClearAllInvalids()
{
    m_bInvalidateTiles = false;
    m_aInvalidations.clear();
    m_aInvalidationsParts.clear();
    m_aInvalidationsMode.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
