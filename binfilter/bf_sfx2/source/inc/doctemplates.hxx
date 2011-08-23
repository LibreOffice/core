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

#ifndef _SFX_DOCTEMPLATES_HXX_
#define _SFX_DOCTEMPLATES_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
 
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/ucb/XContent.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>
 
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <ucbhelper/content.hxx>

#include <sfxuno.hxx>
namespace binfilter {

//--------------------------------------------------------------------------------------------------------

#define LOCALE						::com::sun::star::lang::Locale
#define REFERENCE					::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION			::com::sun::star::uno::RuntimeException
#define PROPERTYVALUE				::com::sun::star::beans::PropertyValue
#define XCONTENT					::com::sun::star::ucb::XContent
#define XDOCUMENTTEMPLATES			::com::sun::star::frame::XDocumentTemplates
#define XINTERFACE					::com::sun::star::uno::XInterface
#define XLOCALIZABLE				::com::sun::star::lang::XLocalizable
#define XMODEL						::com::sun::star::frame::XModel
#define XMULTISERVICEFACTORY		::com::sun::star::lang::XMultiServiceFactory
#define XNAMEACCESS					::com::sun::star::container::XNameAccess
#define XSERVICEINFO				::com::sun::star::lang::XServiceInfo
#define XSTORABLE					::com::sun::star::frame::XStorable

//--------------------------------------------------------------------------------------------------------

class SfxDocTplService_Impl;

class SfxDocTplService: public ::cppu::WeakImplHelper3< XLOCALIZABLE, XDOCUMENTTEMPLATES, XSERVICEINFO >
{
    SfxDocTplService_Impl		*pImp;

public:
                                    SFX_DECL_XSERVICEINFO

                                    SfxDocTplService( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
                                   ~SfxDocTplService();

    // --- XLocalizable ---
    void SAL_CALL					setLocale( const LOCALE & eLocale ) throw( RUNTIMEEXCEPTION );
    LOCALE SAL_CALL					getLocale() throw( RUNTIMEEXCEPTION );

    // --- XDocumentTemplates ---
    REFERENCE< XCONTENT > SAL_CALL	getContent() throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				storeTemplate( const ::rtl::OUString& GroupName,
                                                   const ::rtl::OUString& TemplateName,
                                                   const REFERENCE< XSTORABLE >& Storable ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				addTemplate( const ::rtl::OUString& GroupName,
                                                 const ::rtl::OUString& TemplateName,
                                                 const ::rtl::OUString& SourceURL ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				removeTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& TemplateName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				renameTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& OldTemplateName,
                                                    const ::rtl::OUString& NewTemplateName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				addGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				removeGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL				renameGroup( const ::rtl::OUString& OldGroupName,
                                                 const ::rtl::OUString& NewGroupName ) throw( RUNTIMEEXCEPTION );
    void SAL_CALL					update() throw( RUNTIMEEXCEPTION );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
