/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <utility>
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
                        BibStatusDispatch( css::util::URL _aURL, css::uno::Reference< css::frame::XStatusListener > xRef )
                            : aURL(std::move( _aURL ))
                            , xListener(std::move( xRef ))
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
    rtl::Reference<BibFrameCtrl_Impl>                         m_xImpl;
    BibStatusDispatchArr                                      m_aStatusListeners;
    css::uno::Reference< css::awt::XWindow >                  m_xWindow;
    css::uno::Reference< css::frame::XFrame >                 m_xFrame;
    bool                                                      m_bDisposing;
    rtl::Reference<BibDataManager>                            m_xDatMan;
    VclPtr<vcl::Window>                                       m_xLastQueriedFocusWin;

    DECL_LINK( DisposeHdl, void*, void );

    static bool                 SaveModified(const css::uno::Reference< css::form::runtime::XFormController>& xController);
public:
                                BibFrameController_Impl( css::uno::Reference< css::awt::XWindow > xComponent,
                                                        BibDataManager* pDatMan);
                                virtual ~BibFrameController_Impl() override;


    void                        ChangeDataSource(const cpo::uno::Sequence< css::beans::PropertyValue >& aArgs);
    void                        RemoveFilter();

                                // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& sServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

                                // css::frame::XController
    virtual void                attachFrame( const css::uno::Reference< css::frame::XFrame > & xFrame ) override;
    virtual bool            attachModel( const css::uno::Reference< css::frame::XModel > & xModel ) override;
    virtual bool            suspend( bool bSuspend ) override;
    virtual cpo::uno::Any       getViewData() override;
    virtual void                restoreViewData( const cpo::uno::Any& Value ) override;
    virtual css::uno::Reference< css::frame::XFrame > getFrame() override;
    virtual css::uno::Reference< css::frame::XModel > getModel() override;

                                // css::lang::XComponent
    virtual void                dispose() override;
    virtual void                addEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) override;
    virtual void                removeEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) override;

                                // css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >          queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual cpo::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > queryDispatches( const cpo::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

                                //class css::frame::XDispatch
    virtual void                dispatch(const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& aArgs) override;
    virtual void                addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;
    virtual void                removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;

                                // css::frame::XDispatchInformationProvider
    virtual cpo::uno::Sequence< ::sal_Int16 > getSupportedCommandGroups(  ) override;
    virtual cpo::uno::Sequence< css::frame::DispatchInformation > getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) override;
 };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
