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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/weld.hxx>

class filter_info_impl;
class XMLFilterTabPageBasic;
class XMLFilterTabPageXSLT;

class XMLFilterTabDialog : public weld::GenericDialogController
{
public:
    XMLFilterTabDialog(weld::Window *pParent, const css::uno::Reference< css::uno::XComponentContext >& rxContext, const filter_info_impl* pInfo);
    virtual ~XMLFilterTabDialog() override;

    bool onOk();

    filter_info_impl* getNewFilterInfo() const { return mpNewInfo.get(); }

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    DECL_LINK(OkHdl, weld::Button&, void);

    const filter_info_impl* mpOldInfo;
    std::unique_ptr<filter_info_impl>   mpNewInfo;

    std::unique_ptr<weld::Notebook>     m_xTabCtrl;
    std::unique_ptr<weld::Button>       m_xOKBtn;

    std::unique_ptr<XMLFilterTabPageBasic>  mpBasicPage;
    std::unique_ptr<XMLFilterTabPageXSLT> mpXSLTPage;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
