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

#ifndef _BIB_FRAMECTR_HXX
#define _BIB_FRAMECTR_HXX
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <cppuhelper/implbase5.hxx>
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
    ::com::sun::star::util::URL                 aURL;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >    xListener;
                        BibStatusDispatch( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef )
                            : aURL( rURL )
                            , xListener( xRef )
                        {}
};

typedef boost::ptr_vector<BibStatusDispatch> BibStatusDispatchArr;

class BibFrameController_Impl : public cppu::WeakImplHelper5 <
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::frame::XController,
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::frame::XDispatchProvider,
    ::com::sun::star::frame::XDispatchInformationProvider
>
{
friend class BibFrameCtrl_Impl;
    BibFrameCtrl_Impl*          pImp;
    BibStatusDispatchArr        aStatusListeners;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  xWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 xFrame;
    sal_Bool                        bDisposing;
    sal_Bool                        bHierarchical;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >               m_xDatMan;
    BibDataManager*             pDatMan;
    HdlBibModul                 pBibMod;

    DECL_STATIC_LINK( BibFrameController_Impl, DisposeHdl, void* );

    sal_Bool                    SaveModified(const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController>& xController);
public:
                                BibFrameController_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xComponent,
                                                        BibDataManager* pDatMan);
                                ~BibFrameController_Impl();


    void                        activate();
    void                        deactivate();

    void                        ChangeDataSource(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs);
    void                        RemoveFilter();

                                // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                                // ::com::sun::star::frame::XController
    virtual void                SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool            SAL_CALL attachModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool            SAL_CALL suspend( sal_Bool bSuspend ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any  SAL_CALL    getViewData() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL                           restoreViewData( const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                                // ::com::sun::star::lang::XComponent
    virtual void                SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void                SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void                SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                                // ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                                //class ::com::sun::star::frame::XDispatch
    virtual void                SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void                SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void                SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                                // ::com::sun::star::frame::XDispatchInformationProvider
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
 };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
