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

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_FRAMECTR_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_FRAMECTR_HXX
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <cppuhelper/implbase.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

#include "bibmod.hxx"
class BibDataManager;
class BibFrameCtrl_Impl;
namespace com{namespace sun{namespace star{
    namespace form { namespace runtime {
        class XFormController;
    } }
}}}
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

typedef boost::ptr_vector<BibStatusDispatch> BibStatusDispatchArr;

class BibFrameController_Impl : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::frame::XController,
    css::frame::XDispatch,
    css::frame::XDispatchProvider,
    css::frame::XDispatchInformationProvider
>
{
friend class BibFrameCtrl_Impl;
    BibFrameCtrl_Impl*          pImp;
    BibStatusDispatchArr        aStatusListeners;
    css::uno::Reference< css::awt::XWindow >                  xWindow;
    css::uno::Reference< css::frame::XFrame >                 xFrame;
    bool                        bDisposing;
    bool                        bHierarchical;
    css::uno::Reference< css::form::XLoadable >               m_xDatMan;
    BibDataManager*             pDatMan;
    HdlBibModul                 pBibMod;

    DECL_LINK_TYPED( DisposeHdl, void*, void );

    static bool                 SaveModified(const css::uno::Reference< css::form::runtime::XFormController>& xController);
public:
                                BibFrameController_Impl( const css::uno::Reference< css::awt::XWindow > & xComponent,
                                                        BibDataManager* pDatMan);
                                virtual ~BibFrameController_Impl();


    void                        ChangeDataSource(const css::uno::Sequence< css::beans::PropertyValue >& aArgs);
    void                        RemoveFilter();

                                // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

                                // css::frame::XController
    virtual void                SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame > & xFrame ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool            SAL_CALL attachModel( const css::uno::Reference< css::frame::XModel > & xModel ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool            SAL_CALL suspend( sal_Bool bSuspend ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any       SAL_CALL getViewData() throw (css::uno::RuntimeException, std::exception) override;
    virtual void                SAL_CALL restoreViewData( const css::uno::Any& Value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel() throw (css::uno::RuntimeException, std::exception) override;

                                // css::lang::XComponent
    virtual void                SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
    virtual void                SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void                SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) throw (css::uno::RuntimeException, std::exception) override;

                                // css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >          SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) throw (css::uno::RuntimeException, std::exception) override;

                                //class css::frame::XDispatch
    virtual void                SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void                SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void                SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) throw (css::uno::RuntimeException, std::exception) override;

                                // css::frame::XDispatchInformationProvider
    virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (css::uno::RuntimeException, std::exception) override;
 };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
