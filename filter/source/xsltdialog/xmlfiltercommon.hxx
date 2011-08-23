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
    rtl::OUString	maFilterName;
    rtl::OUString	maType;
    rtl::OUString	maDocumentService;
    rtl::OUString	maFilterService;
    rtl::OUString	maInterfaceName;
    rtl::OUString	maComment;
    rtl::OUString	maExtension;
    rtl::OUString	maDTD;
    rtl::OUString	maExportXSLT;
    rtl::OUString	maImportXSLT;
    rtl::OUString	maImportTemplate;
    rtl::OUString	maDocType;
    rtl::OUString	maImportService;
    rtl::OUString	maExportService;

    sal_Int32		maFlags;
    sal_Int32		maFileFormatVersion;
    sal_Int32		mnDocumentIconID;

    sal_Bool        mbReadonly;
    filter_info_impl();
    filter_info_impl( const filter_info_impl& rInfo );
    int operator==( const filter_info_impl& ) const;

    com::sun::star::uno::Sequence< rtl::OUString > getFilterUserData() const;
};

// --------------------------------------------------------------------

struct application_info_impl
{
    rtl::OUString	maDocumentService;
    rtl::OUString	maDocumentUIName;
    rtl::OUString	maXMLImporter;
    rtl::OUString	maXMLExporter;

    application_info_impl( const sal_Char * pDocumentService, ResId& rUINameRes, const sal_Char * mpXMLImporter, const sal_Char * mpXMLExporter );
};

// --------------------------------------------------------------------

extern std::vector< application_info_impl* >& getApplicationInfos();
extern rtl::OUString getApplicationUIName( const rtl::OUString& rServiceName );
extern const application_info_impl* getApplicationInfo( const rtl::OUString& rServiceName );

extern ResMgr* getXSLTDialogResMgr();

#define RESID(x) ResId(x, *getXSLTDialogResMgr() )

#endif
