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
#ifndef _XMLFILTERCOMMON_HXX_
#define _XMLFILTERCOMMON_HXX_

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <vector>

// --------------------------------------------------------------------

extern ::rtl::OUString string_encode( const ::rtl::OUString & rText );
extern ::rtl::OUString string_decode( const ::rtl::OUString & rText );

// --------------------------------------------------------------------

extern bool isFileURL( const ::rtl::OUString & rURL );

// --------------------------------------------------------------------

bool copyStreams( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xIS, ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOS );
bool createDirectory( ::rtl::OUString& rURL );

// --------------------------------------------------------------------

class filter_info_impl
{
public:
    rtl::OUString   maFilterName;
    rtl::OUString   maType;
    rtl::OUString   maDocumentService;
    rtl::OUString   maFilterService;
    rtl::OUString   maInterfaceName;
    rtl::OUString   maComment;
    rtl::OUString   maExtension;
    rtl::OUString   maDTD;
    rtl::OUString   maExportXSLT;
    rtl::OUString   maImportXSLT;
    rtl::OUString   maImportTemplate;
    rtl::OUString   maDocType;
    rtl::OUString   maImportService;
    rtl::OUString   maExportService;

    sal_Int32       maFlags;
    sal_Int32       maFileFormatVersion;
    sal_Int32       mnDocumentIconID;

    sal_Bool        mbReadonly;

    rtl::OUString   maXSLTTransformerImpl;

    filter_info_impl();
    filter_info_impl( const filter_info_impl& rInfo );
    int operator==( const filter_info_impl& ) const;

    com::sun::star::uno::Sequence< rtl::OUString > getFilterUserData() const;
};

// --------------------------------------------------------------------

struct application_info_impl
{
    rtl::OUString   maDocumentService;
    rtl::OUString   maDocumentUIName;
    rtl::OUString   maXMLImporter;
    rtl::OUString   maXMLExporter;

    application_info_impl( const sal_Char * pDocumentService, ResId& rUINameRes, const sal_Char * mpXMLImporter, const sal_Char * mpXMLExporter );
};

// --------------------------------------------------------------------

extern std::vector< application_info_impl* >& getApplicationInfos();
extern rtl::OUString getApplicationUIName( const rtl::OUString& rServiceName );
extern const application_info_impl* getApplicationInfo( const rtl::OUString& rServiceName );

extern ResMgr* getXSLTDialogResMgr();

#define RESID(x) ResId(x, *getXSLTDialogResMgr() )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
