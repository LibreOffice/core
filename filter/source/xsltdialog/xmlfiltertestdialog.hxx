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
#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERTESTDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERTESTDIALOG_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>

#include <cppuhelper/weakref.hxx>
#include <vcl/weld.hxx>
#include <svl/poolitem.hxx>

class filter_info_impl;

class XMLFilterTestDialog : public weld::GenericDialogController
{
public:
    XMLFilterTestDialog(weld::Window* pParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext);
    virtual ~XMLFilterTestDialog() override;

    void test( const filter_info_impl& rFilterInfo );

    void updateCurrentDocumentButtonState( css::uno::Reference< css::lang::XComponent > const * pRef = nullptr );

private:
    DECL_LINK(ClickHdl_Impl, weld::Button&, void);

    void onExportBrowse();
    void onExportCurrentDocument();
    void onImportBrowse();
    void initDialog();

    css::uno::Reference< css::lang::XComponent > getFrontMostDocument( const OUString& rServiceName );
    void import( const OUString& rURL );
    static void displayXMLFile( const OUString& rURL );
    void doExport( const css::uno::Reference< css::lang::XComponent >& xComp );

private:
    css::uno::Reference< css::uno::XComponentContext >              mxContext;
    css::uno::Reference< css::document::XDocumentEventBroadcaster > mxGlobalBroadcaster;
    css::uno::Reference< css::document::XDocumentEventListener >    mxGlobalEventListener;
    css::uno::WeakReference< css::lang::XComponent >                mxLastFocusModel;

    OUString m_sImportRecentFile;
    OUString m_sExportRecentFile;
    std::unique_ptr<filter_info_impl> m_xFilterInfo;
    OUString m_sDialogTitle;

    std::unique_ptr<weld::Widget> m_xExport;
    std::unique_ptr<weld::Label>  m_xFTExportXSLTFile;
    std::unique_ptr<weld::Button> m_xPBExportBrowse;
    std::unique_ptr<weld::Button> m_xPBCurrentDocument;
    std::unique_ptr<weld::Label>  m_xFTNameOfCurrentFile;
    std::unique_ptr<weld::Widget> m_xImport;
    std::unique_ptr<weld::Label>  m_xFTImportXSLTFile;
    std::unique_ptr<weld::Label>  m_xFTImportTemplate;
    std::unique_ptr<weld::Label>  m_xFTImportTemplateFile;
    std::unique_ptr<weld::CheckButton>   m_xCBXDisplaySource;
    std::unique_ptr<weld::Button> m_xPBImportBrowse;
    std::unique_ptr<weld::Button> m_xPBRecentFile;
    std::unique_ptr<weld::Label>  m_xFTNameOfRecentFile;
    std::unique_ptr<weld::Button> m_xPBClose;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
