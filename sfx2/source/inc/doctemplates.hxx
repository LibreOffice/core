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

#pragma once
#if 1

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
#include <sfx2/sfxuno.hxx>

//--------------------------------------------------------------------------------------------------------

#define UNOLOCALE                   ::com::sun::star::lang::Locale
#define REFERENCE                   ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION            ::com::sun::star::uno::RuntimeException
#define PROPERTYVALUE               ::com::sun::star::beans::PropertyValue
#define XCONTENT                    ::com::sun::star::ucb::XContent
#define XDOCUMENTTEMPLATES          ::com::sun::star::frame::XDocumentTemplates
#define XINTERFACE                  ::com::sun::star::uno::XInterface
#define XLOCALIZABLE                ::com::sun::star::lang::XLocalizable
#define XMODEL                      ::com::sun::star::frame::XModel
#define XMULTISERVICEFACTORY        ::com::sun::star::lang::XMultiServiceFactory
#define XSERVICEINFO                ::com::sun::star::lang::XServiceInfo
#define XSTORABLE                   ::com::sun::star::frame::XStorable

//--------------------------------------------------------------------------------------------------------

class SfxDocTplService_Impl;

class SfxDocTplService: public ::cppu::WeakImplHelper3< XLOCALIZABLE, XDOCUMENTTEMPLATES, XSERVICEINFO >
{
    SfxDocTplService_Impl       *pImp;

public:
                                    SFX_DECL_XSERVICEINFO

                                    SfxDocTplService( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
                                   ~SfxDocTplService();

    // --- XLocalizable ---
    void SAL_CALL                   setLocale( const UNOLOCALE & eLocale ) throw( RUNTIMEEXCEPTION );
    UNOLOCALE SAL_CALL              getLocale() throw( RUNTIMEEXCEPTION );

    // --- XDocumentTemplates ---
    REFERENCE< XCONTENT > SAL_CALL  getContent() throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               storeTemplate( const ::rtl::OUString& GroupName,
                                                   const ::rtl::OUString& TemplateName,
                                                   const REFERENCE< XSTORABLE >& Storable ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               addTemplate( const ::rtl::OUString& GroupName,
                                                 const ::rtl::OUString& TemplateName,
                                                 const ::rtl::OUString& SourceURL ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               removeTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& TemplateName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               renameTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& OldTemplateName,
                                                    const ::rtl::OUString& NewTemplateName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               addGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               removeGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               renameGroup( const ::rtl::OUString& OldGroupName,
                                                 const ::rtl::OUString& NewGroupName ) throw( RUNTIMEEXCEPTION );
    void SAL_CALL                   update() throw( RUNTIMEEXCEPTION );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
