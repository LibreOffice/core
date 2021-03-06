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
        : ConfigurationListenerProperty(rListener, "EnglishFunctionName")
        , m_pFunctionWin(pFunctionWin)
    {
    }
};

class ScFunctionWin : public PanelLayout
{

private:
    std::unique_ptr<weld::ComboBox> xCatBox;
    std::unique_ptr<weld::TreeView> xFuncList;
    std::unique_ptr<weld::Button> xInsertButton;
    std::unique_ptr<weld::Label> xFiFuncDesc;

    rtl::Reference<comphelper::ConfigurationListener> xConfigListener;
    std::unique_ptr<EnglishFunctionNameChange> xConfigChange;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16          nArgs;

    ::std::vector< const formula::IFunctionDescription*> aLRUList;

    void            UpdateLRUList();
    void            DoEnter();
    void            SetDescription();

                    DECL_LINK( SetRowActivatedHdl, weld::TreeView&, bool );
                    DECL_LINK( SetSelectionClickHdl, weld::Button&, void );
                    DECL_LINK( SelComboHdl, weld::ComboBox&, void );
                    DECL_LINK( SelTreeHdl, weld::TreeView&, void );

public:
    ScFunctionWin(weld::Widget* pParent);

    virtual ~ScFunctionWin() override;

    void            InitLRUList();
    void            UpdateFunctionList();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
