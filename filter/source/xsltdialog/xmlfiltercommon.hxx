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
#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERCOMMON_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERCOMMON_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <vector>



extern OUString string_encode( const OUString & rText );
extern OUString string_decode( const OUString & rText );

bool copyStreams( css::uno::Reference< css::io::XInputStream > xIS, css::uno::Reference< css::io::XOutputStream > xOS );
bool createDirectory( OUString& rURL );



class filter_info_impl
{
public:
    OUString   maFilterName;
    OUString   maType;
    OUString   maDocumentService;
    OUString   maFilterService;
    OUString   maInterfaceName;
    OUString   maComment;
    OUString   maExtension;
    OUString   maExportXSLT;
    OUString   maImportXSLT;
    OUString   maImportTemplate;
    OUString   maDocType;
    OUString   maImportService;
    OUString   maExportService;

    sal_Int32       maFlags;
    sal_Int32       maFileFormatVersion;
    sal_Int32       mnDocumentIconID;

    bool        mbReadonly;

    bool        mbNeedsXSLT2;

    filter_info_impl();
    filter_info_impl( const filter_info_impl& rInfo );
    bool operator==( const filter_info_impl& ) const;

    css::uno::Sequence< OUString > getFilterUserData() const;
};



struct application_info_impl
{
    OUString   maDocumentService;
    OUString   maDocumentUIName;
    OUString   maXMLImporter;
    OUString   maXMLExporter;

    application_info_impl( const sal_Char * pDocumentService, ResId& rUINameRes, const sal_Char * mpXMLImporter, const sal_Char * mpXMLExporter );
};



extern std::vector< application_info_impl* >& getApplicationInfos();
extern OUString getApplicationUIName( const OUString& rServiceName );
extern const application_info_impl* getApplicationInfo( const OUString& rServiceName );

extern ResMgr* getXSLTDialogResMgr();

#define RESID(x) ResId(x, *getXSLTDialogResMgr())
#define RESIDSTR(x) RESID(x).toString()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
