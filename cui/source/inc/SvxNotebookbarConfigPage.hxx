/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <libxml/parser.h>
#include <vcl/weld.hxx>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

#include <memory>
#include <vector>

#include "cfg.hxx" //for SvxConfigPage and SaveInData

class SvxNotebookbarConfigPage : public SvxConfigPage
{
private:
    void UpdateButtonStates() override;
    short QueryReset() override;
    void Init() override;
    void DeleteSelectedContent() override;
    void DeleteSelectedTopLevel() override;
    virtual void SelectElement() override;
    void SetElement();

public:
    struct NotebookbarEntries
    {
        OUString sUIItemId;
        OUString sClassId;
        OUString sActionName;
        OUString sDisplayName;
        OUString sVisibleValue;
    };
    struct CategoriesEntries
    {
        OUString sDisplayName;
        OUString sUIItemId;
        OUString sClassType;
    };
    SvxNotebookbarConfigPage(weld::Container* pPage, weld::DialogController* pController,
                             const SfxItemSet& rItemSet);
    virtual ~SvxNotebookbarConfigPage() override;
    SaveInData* CreateSaveInData(const css::uno::Reference<css::ui::XUIConfigurationManager>&,
                                 const css::uno::Reference<css::ui::XUIConfigurationManager>&,
                                 const OUString& aModuleId, bool docConfig) override;
    static void FillFunctionsList(xmlNodePtr pRootNodePtr,
                                  std::vector<NotebookbarEntries>& aEntries,
                                  std::vector<CategoriesEntries>& aCategoryList,
                                  OUString& sActiveCategory);
    static void searchNodeandAttribute(std::vector<NotebookbarEntries>& aEntries,
                                       std::vector<CategoriesEntries>& aCategoryList,
                                       OUString& sActiveCategory,
                                       CategoriesEntries& aCurCategoryEntry, xmlNode* pNodePtr,
                                       bool isCategory);
    static void getNodeValue(xmlNode* pNodePtr, NotebookbarEntries& aNodeEntries);
};

class SvxNotebookbarEntriesListBox final : public SvxMenuEntriesListBox
{
    void ChangedVisibility(int nRow);
    DECL_LINK(CheckButtonHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    SvxNotebookbarEntriesListBox(std::unique_ptr<weld::TreeView> xControl, SvxConfigPage* pPg);
    virtual ~SvxNotebookbarEntriesListBox() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
