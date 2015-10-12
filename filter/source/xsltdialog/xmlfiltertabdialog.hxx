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
#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERTABDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERTABDIALOG_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/button.hxx>

namespace vcl { class Window; }
class ResMgr;

class filter_info_impl;
class XMLFilterTabPageBasic;
class XMLFilterTabPageXSLT;

class XMLFilterTabDialog: public TabDialog
{
public:
    XMLFilterTabDialog(vcl::Window *pParent, ResMgr& rResMgr, const css::uno::Reference< css::uno::XComponentContext >& rxContext, const filter_info_impl* pInfo);
    virtual ~XMLFilterTabDialog();
    virtual void dispose() override;

    bool onOk();

    filter_info_impl* getNewFilterInfo() const { return mpNewInfo;}

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    DECL_STATIC_LINK_TYPED( XMLFilterTabDialog, ActivatePageHdl, TabControl*, void );
    DECL_LINK_TYPED(OkHdl, Button*, void);

    ResMgr& mrResMgr;

    const filter_info_impl* mpOldInfo;
    filter_info_impl* mpNewInfo;

    VclPtr<TabControl>     m_pTabCtrl;
    VclPtr<OKButton>       m_pOKBtn;

    sal_Int16 m_nBasicPageId;
    sal_Int16 m_nXSLTPageId;

    VclPtr<XMLFilterTabPageBasic>  mpBasicPage;
    VclPtr<XMLFilterTabPageXSLT> mpXSLTPage;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
