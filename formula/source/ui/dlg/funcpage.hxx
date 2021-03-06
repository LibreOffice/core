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
    std::unique_ptr<weld::Entry> m_xLbFunctionSearchString;

    Link<FuncPage&,void>     aDoubleClickLink;
    Link<FuncPage&,void>     aSelectionLink;
    const IFunctionManager*  m_pFunctionManager;

    ::std::vector< TFunctionDesc >  aLRUList;
    OString    m_aHelpId;

    void impl_addFunctions(const IFunctionCategory* _pCategory);

    DECL_LINK(SelComboBoxHdl, weld::ComboBox&, void);
    DECL_LINK(SelTreeViewHdl, weld::TreeView&, void);
    DECL_LINK(DblClkHdl, weld::TreeView&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ModifyHdl, weld::Entry&, void);

    void            UpdateFunctionList(const OUString&);

public:

    FuncPage(weld::Container* pContainer, const IFunctionManager* _pFunctionManager);
    ~FuncPage();

    void            SetCategory(sal_Int32  nCat);
    void            SetFunction(sal_Int32  nFunc);
    void            SetFocus();
    sal_Int32       GetCategory() const;
    sal_Int32       GetCategoryEntryCount() const;
    sal_Int32       GetFunction() const;
    sal_Int32       GetFunctionEntryCount() const;

    sal_Int32       GetFuncPos(const IFunctionDescription* _pDesc);
    const IFunctionDescription* GetFuncDesc( sal_Int32  nPos ) const;
    OUString        GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link<FuncPage&,void>& rLink ) { aDoubleClickLink = rLink; }

    void            SetSelectHdl( const Link<FuncPage&,void>& rLink ) { aSelectionLink = rLink; }

    bool            IsVisible() const { return m_xContainer->get_visible(); }
};

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
