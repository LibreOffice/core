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
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>

class filter_info_impl;

class XMLFilterTestDialog : public ModalDialog
{
public:
    XMLFilterTestDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF  );
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

    com::sun::star::uno::Reference< com::sun::star::lang::XComponent > getFrontMostDocument( const rtl::OUString& rServiceName );
    void import( const rtl::OUString& rURL );
    void displayXMLFile( const rtl::OUString& rURL );
    void doExport( com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComp );

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::document::XEventBroadcaster > mxGlobalBroadcaster;
    com::sun::star::uno::Reference< com::sun::star::document::XEventListener > mxGlobalEventListener;
    com::sun::star::uno::WeakReference< com::sun::star::lang::XComponent > mxLastFocusModel;

    rtl::OUString   maImportRecentFile;
    rtl::OUString   maExportRecentFile;

    FixedLine   maFLExport;
    FixedText   maFTExportXSLT;
    FixedText   maFTExportXSLTFile;
    FixedText   maFTTransformDocument;
    PushButton  maPBExportBrowse;
    PushButton  maPBCurrentDocument;
    FixedText   maFTNameOfCurentFile;
    FixedLine   maFLImport;
    FixedText   maFTImportXSLT;
    FixedText   maFTImportXSLTFile;
    FixedText   maFTImportTemplate;
    FixedText   maFTImportTemplateFile;
    FixedText   maFTTransformFile;
    CheckBox    maCBXDisplaySource;
    PushButton  maPBImportBrowse;
    PushButton  maPBRecentDocument;
    FixedText   maFTNameOfRecentFile;
    PushButton  maPBClose;
    HelpButton  maPBHelp;

    filter_info_impl*       mpFilterInfo;

    String maDialogTitle;

    ::rtl::OUString sDTDPath;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
