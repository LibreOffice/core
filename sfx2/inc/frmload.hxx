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

#ifndef _SFX_FRMLOAD_HXX
#define _SFX_FRMLOAD_HXX

#include "sfx2/sfxuno.hxx"
#include "sfx2/objsh.hxx"

#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <comphelper/namedvaluecollection.hxx>

class SfxFilter;
class SfxFilterMatcher;

class SAL_DLLPRIVATE SfxFrameLoader_Impl : public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XSynchronousFrameLoader, ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >  m_aContext;

public:
                            SfxFrameLoader_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >& _rxContext );

    SFX_DECL_XSERVICEINFO

    //----------------------------------------------------------------------------------
    // XSynchronousFrameLoader
    //----------------------------------------------------------------------------------
    virtual sal_Bool SAL_CALL load( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL cancel() throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual                 ~SfxFrameLoader_Impl();

private:
    const SfxFilter*    impl_getFilterFromServiceName_nothrow(
                            const OUString& i_rServiceName
                        ) const;

    OUString     impl_askForFilter_nothrow(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& i_rxHandler,
                            const OUString& i_rDocumentURL
                        ) const;

    const SfxFilter*    impl_detectFilterForURL(
                            const OUString& _rURL,
                            const ::comphelper::NamedValueCollection& i_rDescriptor,
                            const SfxFilterMatcher& rMatcher
                        ) const;

    sal_Bool            impl_createNewDocWithSlotParam(
                            const sal_uInt16 _nSlotID,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rxFrame,
                            const bool i_bHidden
                        );

    void                impl_determineFilter(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    bool                impl_determineTemplateDocument(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    sal_uInt16              impl_findSlotParam(
                            const OUString& i_rFactoryURL
                        ) const;

    SfxObjectShellRef   impl_findObjectShell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel2 >& i_rxDocument
                        ) const;

    void                impl_lockHiddenDocument(
                                  SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    void                impl_handleCaughtError_nothrow(
                            const ::com::sun::star::uno::Any& i_rCaughtError,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    void                impl_removeLoaderArguments(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    sal_Int16           impl_determineEffectiveViewId_nothrow(
                            const SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        );

    ::comphelper::NamedValueCollection
                        impl_extractViewCreationArgs(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >
                        impl_createDocumentView(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel2 >& i_rModel,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
                            const ::comphelper::NamedValueCollection& i_rViewFactoryArgs,
                            const OUString& i_rViewName
                        );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
