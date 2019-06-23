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
#ifndef INCLUDED_CUI_SOURCE_INC_SVXNOTEBOOKBARCONFIGPAGE_HXX
#define INCLUDED_CUI_SOURCE_INC_SVXNOTEBOOKBARCONFIGPAGE_HXX

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <vcl/weld.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <sfx2/tabdlg.hxx>
#include <memory>
#include <vector>

//#include "cfgutil.hxx"
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

public:
    struct NotebookbarEntries
    {
        OUString sUIItemID;
        OUString sActionName;
        OUString sVisibleValue;
        int nPos;
    };
    SvxNotebookbarConfigPage(TabPageParent pParent, const SfxItemSet& rItemSet);
    virtual ~SvxNotebookbarConfigPage() override;
    SaveInData* CreateSaveInData(const css::uno::Reference<css::ui::XUIConfigurationManager>&,
                                 const css::uno::Reference<css::ui::XUIConfigurationManager>&,
                                 const OUString& aModuleId, bool docConfig) override;
    static void FillFunctionsList(std::vector<NotebookbarEntries>& aEntries,
                                  std::vector<OUString>& aCategoryList, OUString& sActiveCategory);
    static void searchNodeandAttribute(std::vector<NotebookbarEntries>& aEntries,
                                       std::vector<OUString>& aCategoryList,
                                       OUString& sActiveCategory, xmlNode* pNodePtr, int nPos,
                                       bool isCategory);
    static void getNodeValue(xmlNode* pNodePtr, NotebookbarEntries& aNodeEntries);
};

class SvxNotebookbarEntriesListBox final : public SvxMenuEntriesListBox
{
    void ChangedVisibility(int nRow);
    typedef std::pair<int, int> row_col;
    DECL_LINK(CheckButtonHdl, const row_col&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    SvxNotebookbarEntriesListBox(std::unique_ptr<weld::TreeView> xControl, SvxConfigPage* pPg);
    virtual ~SvxNotebookbarEntriesListBox() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_SVXNOTEBOOKBARCONFIGPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
