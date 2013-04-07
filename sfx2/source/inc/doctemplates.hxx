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
#include <sfx2/sfxuno.hxx>


class SfxDocTplService_Impl;

class SfxDocTplService: public ::cppu::WeakImplHelper3< css::lang::XLocalizable, css::frame::XDocumentTemplates, css::lang::XServiceInfo >
{
    SfxDocTplService_Impl       *pImp;

public:
                                    SFX_DECL_XSERVICEINFO

                                    SfxDocTplService( const css::uno::Reference < css::lang::XMultiServiceFactory >& xFactory );
                                   ~SfxDocTplService();

    // --- XLocalizable ---
    void SAL_CALL                   setLocale( const css::lang::Locale & eLocale ) throw( css::uno::RuntimeException );
    css::lang::Locale SAL_CALL              getLocale() throw( css::uno::RuntimeException );

    // --- XDocumentTemplates ---
    css::uno::Reference< css::ucb::XContent > SAL_CALL  getContent() throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               storeTemplate( const OUString& GroupName,
                                                   const OUString& TemplateName,
                                                   const css::uno::Reference< css::frame::XStorable >& Storable ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               addTemplate( const OUString& GroupName,
                                                 const OUString& TemplateName,
                                                 const OUString& SourceURL ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               removeTemplate( const OUString& GroupName,
                                                    const OUString& TemplateName ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               renameTemplate( const OUString& GroupName,
                                                    const OUString& OldTemplateName,
                                                    const OUString& NewTemplateName ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               addGroup( const OUString& GroupName ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               removeGroup( const OUString& GroupName ) throw( css::uno::RuntimeException );
    sal_Bool SAL_CALL               renameGroup( const OUString& OldGroupName,
                                                 const OUString& NewGroupName ) throw( css::uno::RuntimeException );
    void SAL_CALL                   update() throw( css::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
