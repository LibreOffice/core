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

#include <comphelper/configurationlistener.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <unordered_map>
#include <sfx2/bindings.hxx>

class ScFuncDesc;
namespace formula { class IFunctionDescription; }

class ScFunctionWin;

class EnglishFunctionNameChange : public comphelper::ConfigurationListenerProperty<bool>
{
    ScFunctionWin* m_pFunctionWin;
protected:
    virtual void setProperty(const css::uno::Any &rProperty) override;
public:
    EnglishFunctionNameChange(const rtl::Reference<comphelper::ConfigurationListener> &rListener, ScFunctionWin* pFunctionWin)
        : ConfigurationListenerProperty(rListener, u"EnglishFunctionName"_ustr)
        , m_pFunctionWin(pFunctionWin)
    {
    }
};

class ScFunctionWin : public PanelLayout
{

private:
    std::unique_ptr<weld::ComboBox> xCatBox;
    std::unique_ptr<weld::TreeView> xFuncList;
    std::unique_ptr<weld::TreeIter> xScratchIter;
    std::unique_ptr<weld::Button> xInsertButton;
    std::unique_ptr<weld::Button> xHelpButton;
    std::unique_ptr<weld::CheckButton> xSimilaritySearch;
    std::unique_ptr<weld::TextView> xFiFuncDesc;
    std::unique_ptr<weld::Entry> m_xSearchString;
    SfxBindings* m_pBindings;

    rtl::Reference<comphelper::ConfigurationListener> xConfigListener;
    std::unique_ptr<EnglishFunctionNameChange> xConfigChange;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16          nArgs;
    OUString m_aListHelpId;
    OUString m_aSearchHelpId;

    ::std::set<std::pair<std::pair<sal_Int32, sal_Int32>, std::pair<OUString, const ScFuncDesc*>>>
                     sFuncScores;
    ::std::vector< const formula::IFunctionDescription*> aLRUList;
    ::std::unordered_map<OUString, std::unique_ptr<weld::TreeIter>> mCategories;

    void            UpdateLRUList();
    void            DoEnter(bool bDouble_or_Enter = false);
    void            SetDescription();
    SfxBindings&    GetBindings() const { return *m_pBindings; }
    weld::TreeIter* FillCategoriesMap(const OUString&, bool);

                    DECL_LINK( SetRowActivatedHdl, weld::TreeView&, bool );
                    DECL_LINK( SetSelectionClickHdl, weld::Button&, void );
                    DECL_LINK( SetHelpClickHdl, weld::Button&, void );
                    DECL_LINK( SetSimilarityToggleHdl, weld::Toggleable&, void );
                    DECL_LINK( SelComboHdl, weld::ComboBox&, void );
                    DECL_LINK( SelTreeHdl, weld::TreeView&, void );
                    DECL_LINK( ModifyHdl, weld::Entry&, void );
                    DECL_LINK( KeyInputHdl, const KeyEvent&, bool);

public:
    ScFunctionWin(weld::Widget* pParent, SfxBindings* pBindings);

    virtual ~ScFunctionWin() override;

    void            InitLRUList();
    void            UpdateFunctionList(const OUString&);
    void            SearchFunction(const OUString&, const OUString&, const ScFuncDesc*, const bool);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
