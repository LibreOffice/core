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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/weld.hxx>
#include <vcl/locktoplevels.hxx>
#include <unotools/moduleoptions.hxx>

#include "xmlfiltercommon.hxx"

class XMLFilterSettingsDialog : public weld::GenericDialogController
{
public:
    XMLFilterSettingsDialog(weld::Window* pParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext);
    virtual ~XMLFilterSettingsDialog() override;

    DECL_LINK(ClickHdl_Impl, weld::Button&, void );
    DECL_LINK(SelectionChangedHdl_Impl, weld::TreeView&, void);
    DECL_LINK(DoubleClickHdl_Impl, weld::TreeView&, bool);

    void    UpdateWindow();

    void    present() { m_xDialog->present(); }

    void    onNew();
    void    onEdit();
    void    onTest();
    void    onDelete();
    void    onSave();
    void    onOpen();

    void    updateStates();
private:
    void    initFilterList();
    void    disposeFilterList();

    void    incBusy() { maBusy.incBusy(m_xDialog.get()); }
    void    decBusy() { maBusy.decBusy(); }

    bool    insertOrEdit( filter_info_impl* pNewInfo, const filter_info_impl* pOldInfo = nullptr );

    OUString createUniqueFilterName( const OUString& rUIName );
    OUString createUniqueTypeName( const OUString& rTypeName );
    OUString createUniqueInterfaceName( const OUString& rInterfaceName );

    /** adds a new filter info entry to the ui filter list */
    void addFilterEntry( const filter_info_impl* pInfo );

    void changeEntry( const filter_info_impl* pInfo );

    static OUString getEntryString( const filter_info_impl* pInfo );

private:
    css::uno::Reference< css::uno::XComponentContext >    mxContext;
    css::uno::Reference< css::container::XNameContainer > mxFilterContainer;
    css::uno::Reference< css::container::XNameContainer > mxTypeDetection;
    css::uno::Reference< css::container::XNameContainer > mxExtendedTypeDetection;

    std::vector< std::unique_ptr<filter_info_impl> > maFilterVector;

    TopLevelWindowLocker maBusy;

    OUString m_sTemplatePath;
    OUString m_sDocTypePrefix;

    SvtModuleOptions maModuleOpt;

    std::unique_ptr<weld::Button> m_xPBNew;
    std::unique_ptr<weld::Button> m_xPBEdit;
    std::unique_ptr<weld::Button> m_xPBTest;
    std::unique_ptr<weld::Button> m_xPBDelete;
    std::unique_ptr<weld::Button> m_xPBSave;
    std::unique_ptr<weld::Button> m_xPBOpen;
    std::unique_ptr<weld::Button> m_xPBClose;
    std::unique_ptr<weld::TreeView> m_xFilterListBox;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
