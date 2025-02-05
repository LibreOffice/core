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

#include <vcl/weld.hxx>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace formula
{

class IFunctionDescription;
class IFunctionManager;
class IFunctionCategory;

typedef const IFunctionDescription* TFunctionDesc;

class FuncPage final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;

    std::unique_ptr<weld::ComboBox> m_xLbCategory;
    std::unique_ptr<weld::TreeView> m_xLbFunction;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    std::unique_ptr<weld::Entry> m_xLbFunctionSearchString;
    std::unique_ptr<weld::CheckButton> m_xSimilaritySearch;
    std::unique_ptr<weld::Button> m_xHelpButton;

    Link<FuncPage&,void>     aDoubleClickLink;
    Link<FuncPage&,void>     aSelectionLink;
    const IFunctionManager*  m_pFunctionManager;

    ::std::vector< TFunctionDesc >  aLRUList;
    ::std::unordered_set<sal_uInt16> aFavouritesList;
    ::std::unordered_map<OUString, std::unique_ptr<weld::TreeIter>> mCategories;
    ::std::set<std::pair<std::pair<sal_Int32, sal_Int32>, std::pair<OUString, TFunctionDesc>>>
        sFuncScores;
    OUString    m_aHelpId;

    // tdf#104487 - remember last used function category
    static sal_Int32 m_nRememberedFunctionCategory;

    void impl_addFunctions(const IFunctionCategory*, bool);
    weld::TreeIter* FillCategoriesMap(const OUString&, bool);

    DECL_LINK(SelComboBoxHdl, weld::ComboBox&, void);
    DECL_LINK(SelTreeViewHdl, weld::TreeView&, void);
    DECL_LINK(DblClkHdl, weld::TreeView&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(SelHelpClickHdl, weld::Button&, void);
    DECL_LINK(SimilarityToggleHdl, weld::Toggleable&, void);

public:

    FuncPage(weld::Container* pContainer, const IFunctionManager* _pFunctionManager);
    ~FuncPage();

    void            SetCategory(sal_Int32  nCat);
    void            SetFunction(sal_Int32  nFunc);
    void            SetFocus();
    sal_Int32       GetCategory() const;
    sal_Int32       GetCategoryEntryCount() const;
    sal_Int32       GetFunction() const;

    // tdf#104487 - remember last used function category
    static sal_Int32 GetRememeberdFunctionCategory() { return m_nRememberedFunctionCategory; };

    sal_Int32       GetFuncPos(const IFunctionDescription* _pDesc);
    const IFunctionDescription* GetFuncDesc() const;
    OUString        GetSelFunctionName() const;
    sal_uInt16      GetFuncIndex() const;

    void            SetDoubleClickHdl( const Link<FuncPage&,void>& rLink ) { aDoubleClickLink = rLink; }

    void            SetSelectHdl( const Link<FuncPage&,void>& rLink ) { aSelectionLink = rLink; }

    bool            IsVisible() const { return m_xContainer->get_visible(); }

    bool            IsFavourite(sal_uInt16) const;

    bool            UpdateFavouritesList();
    void            UpdateFunctionList(const OUString&);

    void            SearchFunction(const OUString&, const OUString&, TFunctionDesc, const bool);
};

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
