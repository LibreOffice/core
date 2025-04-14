/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>

#include <boost/property_tree/ptree.hpp>

#include <tools/gen.hxx>
#include <rtl/string.hxx>
#include <test/lokcallback.hxx>

#include "scqahelperdllapi.h"

class SfxViewShell;

struct SCQAHELPER_DLLPUBLIC ScTestEditCursorMessage final
{
    tools::Rectangle m_aRelRect;
    Point m_aRefPoint;

    void clear();

    bool empty();

    void parseMessage(const char* pMessage);

    tools::Rectangle getBounds();
};

struct SCQAHELPER_DLLPUBLIC ScTestTextSelectionMessage
{
    std::vector<tools::Rectangle> m_aRelRects;
    Point m_aRefPoint;

    void clear();

    bool empty();

    void parseMessage(const char* pMessage);

    tools::Rectangle getBounds(size_t nIndex);
};
/// A view callback tracks callbacks invoked on one specific view.
class SCQAHELPER_DLLPUBLIC ScTestViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    bool m_bOwnCursorInvalidated;
    bool m_bViewCursorInvalidated;
    bool m_textCursorVisible;
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
    ScTestEditCursorMessage m_aInvalidateCursorResult;
    ScTestTextSelectionMessage m_aTextSelectionResult;
    OString m_sInvalidateHeader;
    OString m_sInvalidateSheetGeometry;
    OString m_aHyperlinkClicked;
    OString m_ShapeSelection;
    std::map<std::string, boost::property_tree::ptree> m_aStateChanges;
    std::string decimalSeparator;
    TestLokCallbackWrapper m_callbackWrapper;

    ScTestViewCallback(bool bDeleteListenerOnDestruct = true);

    ~ScTestViewCallback();

    static void callback(int nType, const char* pPayload, void* pData);

    void callbackImpl(int nType, const char* pPayload);

    void ClearAllInvalids();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
