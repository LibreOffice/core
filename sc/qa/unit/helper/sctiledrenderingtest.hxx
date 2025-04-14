/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/unoapixml_test.hxx>

#include <osl/conditn.hxx>
#include <test/lokcallback.hxx>

#include <types.hxx>

#include "scqahelperdllapi.h"

class ScTestViewCallback;
class ScModelObj;
class ScTabViewShell;
class ScAddress;
class SfxViewShell;

class SCQAHELPER_DLLPUBLIC ScTiledRenderingTest : public UnoApiXmlTest
{
public:
    ScTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void checkSampleInvalidation(const ScTestViewCallback& rView, bool bFullRow);
    void cellInvalidationHelper(ScModelObj* pModelObj, ScTabViewShell* pView, const ScAddress& rAdr,
                                bool bAddText, bool bFullRow);

    ScModelObj* createDoc(const char* pName);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    void typeCharsInCell(const std::string& aStr, SCCOL nCol, SCROW nRow, ScTabViewShell* pView,
                         ScModelObj* pModelObj, bool bInEdit = false, bool bCommit = true);

    /// document size changed callback.
    osl::Condition m_aDocSizeCondition;
    Size m_aDocumentSize;

    TestLokCallbackWrapper m_callbackWrapper;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
