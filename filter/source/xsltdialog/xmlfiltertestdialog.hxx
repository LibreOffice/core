/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
