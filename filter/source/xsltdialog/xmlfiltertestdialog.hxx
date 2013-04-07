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
#ifndef _XMLFILTERTESTDIALOG_HXX_
#define _XMLFILTERTESTDIALOG_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <svl/poolitem.hxx>

class filter_info_impl;

class XMLFilterTestDialog : public ModalDialog
{
public:
    XMLFilterTestDialog(Window* pParent,
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~XMLFilterTestDialog();

    void test( const filter_info_impl& rFilterInfo );

    void updateCurrentDocumentButtonState( com::sun::star::uno::Reference< com::sun::star::lang::XComponent > * pRef = NULL );

private:
    DECL_LINK(ClickHdl_Impl, PushButton * );

    void onExportBrowse();
    void onExportCurrentDocument();
    void onImportBrowse();
    void onImportRecentDocument();
    void initDialog();

    com::sun::star::uno::Reference< com::sun::star::lang::XComponent > getFrontMostDocument( const OUString& rServiceName );
    void import( const OUString& rURL );
    void displayXMLFile( const OUString& rURL );
    void doExport( com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComp );

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::document::XEventBroadcaster > mxGlobalBroadcaster;
    com::sun::star::uno::Reference< com::sun::star::document::XEventListener > mxGlobalEventListener;
    com::sun::star::uno::WeakReference< com::sun::star::lang::XComponent > mxLastFocusModel;

    OUString m_sImportRecentFile;
    OUString m_sExportRecentFile;

    VclContainer* m_pExport;
    FixedText*  m_pFTExportXSLTFile;
    PushButton* m_pPBExportBrowse;
    PushButton* m_pPBCurrentDocument;
    FixedText*  m_pFTNameOfCurrentFile;

    VclContainer* m_pImport;
    FixedText*  m_pFTImportXSLTFile;
    FixedText*  m_pFTImportTemplate;
    FixedText*  m_pFTImportTemplateFile;
    CheckBox*   m_pCBXDisplaySource;
    PushButton* m_pPBImportBrowse;
    PushButton* m_pPBRecentFile;
    FixedText*  m_pFTNameOfRecentFile;
    CloseButton* m_pPBClose;

    filter_info_impl* m_pFilterInfo;

    OUString m_sDialogTitle;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
