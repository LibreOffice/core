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
#ifndef INCLUDED_SC_SOURCE_UI_INC_DWFUNCTR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DWFUNCTR_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include "anyrefdg.hxx"
#include "global.hxx"
#include "funcdesc.hxx"

class ScFunctionWin : public PanelLayout
{

private:
    VclPtr<ListBox>     aCatBox;
    VclPtr<ListBox>     aFuncList;

    VclPtr<PushButton>  aInsertButton;
    VclPtr<FixedText>   aFiFuncDesc;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16          nArgs;

    ::std::vector< const formula::IFunctionDescription*> aLRUList;

    void            UpdateFunctionList();
    void            UpdateLRUList();
    void            DoEnter();
    void            SetDescription();

                    DECL_LINK( SetSelectionHdl, ListBox&, void );
                    DECL_LINK( SetSelectionClickHdl, Button*, void );
                    DECL_LINK( SelHdl, ListBox&, void );

public:
    ScFunctionWin(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame> &rFrame);

    virtual ~ScFunctionWin() override;
    virtual void    dispose() override;

    void            InitLRUList();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
