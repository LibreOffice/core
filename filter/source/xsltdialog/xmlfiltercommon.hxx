/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfiltercommon.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 08:12:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _XMLFILTERCOMMON_HXX_
#define _XMLFILTERCOMMON_HXX_

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#include <vector>

// --------------------------------------------------------------------

extern ::rtl::OUString string_encode( const ::rtl::OUString & rText );
extern ::rtl::OUString xmlname_encode( const ::rtl::OUString & rText );
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
