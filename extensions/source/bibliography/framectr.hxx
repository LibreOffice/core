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

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <vector>
#include <memory>

#include "bibmod.hxx"
class BibDataManager;
class BibFrameCtrl_Impl;
namespace com::sun::star{
    namespace form::runtime {
        class XFormController;
    }
}
class BibStatusDispatch
{
public:
    css::util::URL                 aURL;
    css::uno::Reference< css::frame::XStatusListener >    xListener;
                        BibStatusDispatch( const css::util::URL& rURL, const css::uno::Reference< css::frame::XStatusListener >& rRef )
                            : aURL( rURL )
                            , xListener( rRef )
                        {}
};

typedef std::vector<std::unique_ptr<BibStatusDispatch> > BibStatusDispatchArr;

class BibFrameController_Impl : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::frame::XController,
    css::frame::XDispatch,
    css::frame::XDispatchProvider,
    css::frame::XDispatchInformationProvider
>
{
friend class BibFrameCtrl_Impl;
    rtl::Reference<BibFrameCtrl_Impl>                         mxImpl;
    BibStatusDispatchArr        aStatusListeners;
    css::uno::Reference< css::awt::XWindow >                  xWindow;
    css::uno::Reference< css::frame::XFrame >                 xFrame;
    bool                        bDisposing;
    rtl::Reference<BibDataManager>                            m_xDatMan;
    VclPtr<vcl::Window> m_xLastQueriedFocusWin;

    DECL_LINK( DisposeHdl, void*, void );

    static bool                 SaveModified(const css::uno::Reference< css::form::runtime::XFormController>& xController);
public:
                                BibFrameController_Impl( const css::uno::Reference< css::awt::XWindow > & xComponent,
                                                        BibDataManager* pDatMan);
                                virtual ~BibFrameController_Impl() override;


    void                        ChangeDataSource(const css::uno::Sequence< css::beans::PropertyValue >& aArgs);
    void                        RemoveFilter();

                                // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                                // css::frame::XController
    virtual void                SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame > & xFrame ) override;
    virtual sal_Bool            SAL_CALL attachModel( const css::uno::Reference< css::frame::XModel > & xModel ) override;
    virtual sal_Bool            SAL_CALL suspend( sal_Bool bSuspend ) override;
    virtual css::uno::Any       SAL_CALL getViewData() override;
    virtual void                SAL_CALL restoreViewData( const css::uno::Any& Value ) override;
    virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame() override;
    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel() override;

                                // css::lang::XComponent
    virtual void                SAL_CALL dispose() override;
    virtual void                SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) override;
    virtual void                SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) override;

                                // css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >          SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

                                //class css::frame::XDispatch
    virtual void                SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs) override;
    virtual void                SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;
    virtual void                SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;

                                // css::frame::XDispatchInformationProvider
    virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups(  ) override;
    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) override;
 };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
