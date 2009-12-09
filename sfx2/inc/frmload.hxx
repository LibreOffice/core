/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frmload.hxx,v $
 * $Revision: 1.13 $
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

#ifndef _SFX_FRMLOAD_HXX
#define _SFX_FRMLOAD_HXX

#include "sfx2/sfxuno.hxx"
#include "sfx2/objsh.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
/** === end UNO includes === **/

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>

class SfxFilter;
class SfxFilterMatcher;

class SfxFrameWeak;

class SAL_DLLPRIVATE SfxFrameLoader_Impl : public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XSynchronousFrameLoader, ::com::sun::star::lang::XServiceInfo >
{
    ::comphelper::ComponentContext  m_aContext;

public:
                            SfxFrameLoader_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

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
                            const ::rtl::OUString& i_rServiceName
                        ) const;

    ::rtl::OUString     impl_askForFilter_nothrow(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& i_rxHandler,
                            const ::rtl::OUString& i_rDocumentURL
                        ) const;

    const SfxFilter*    impl_detectFilterForURL(
                            const ::rtl::OUString& _rURL,
                            const ::comphelper::NamedValueCollection& i_rDescriptor,
                            const SfxFilterMatcher& rMatcher
                        ) const;

    sal_Bool            impl_createNewDocWithSlotParam(
                            const USHORT _nSlotID,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rxFrame
                        );

    void                impl_determineFilter(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    sal_Bool            impl_cleanUp(
                            const sal_Bool i_bSuccess,
                            const SfxFrameWeak& i_wFrame
                        ) const;

    bool                impl_determineTemplateDocument(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    USHORT              impl_findSlotParam(
                            const ::rtl::OUString& i_rFactoryURL
                        ) const;

    SfxObjectShellLock  impl_findObjectShell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& i_rxDocument
                        ) const;

    void                impl_lockHiddenDocument(
                                  SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    void                impl_handleCaughtError_nothrow(
                            const ::com::sun::star::uno::Any& i_rCaughtError,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    SfxFrame*           impl_getOrCreateEmptySfxFrame(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame
                        ) const;

    void                impl_removeLoaderArguments(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        );
};

#endif
