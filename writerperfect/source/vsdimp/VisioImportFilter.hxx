/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2000 by Sun Microsystems, Inc.
 * Copyright (C) 2002-2004 William Lachance (wlach@interlog.com)
 * Copyright (C) 2004 Net Integration Technologies (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _VISIOIMPORTFILTER_HXX
#define _VISIOIMPORTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase5.hxx>

#include <stdio.h>

enum FilterType
{
    FILTER_IMPORT,
    FILTER_EXPORT
};
/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class VisioImportFilter : public cppu::WeakImplHelper5
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
protected:
    // oo.org declares
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    ::rtl::OUString msFilterName;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > mxHandler;

    FilterType meType;

public:
    VisioImportFilter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    : mxMSF( rxMSF ), meType((FilterType)0) {}
    virtual ~VisioImportFilter() {}

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XExtendedFilterDetection
    virtual ::rtl::OUString SAL_CALL detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Descriptor )
        throw( com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

};

::rtl::OUString VisioImportFilter_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL VisioImportFilter_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL VisioImportFilter_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL VisioImportFilter_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( ::com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
