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

#ifndef INCLUDED_TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX
#define INCLUDED_TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weakagg.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>


class StdTabController :    public ::com::sun::star::awt::XTabController,
                            public ::com::sun::star::lang::XServiceInfo,
                            public ::com::sun::star::lang::XTypeProvider,
                            public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex            maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >    mxControlContainer;

protected:
    ::osl::Mutex&               GetMutex() { return maMutex; }
    static bool                 ImplCreateComponentSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rControls, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rModels, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& rComponents, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>* pTabStops, bool bPeerComponent );
    // if sequence length of rModels is less than rControls, return only the matching elements in rModels sequence and remove corresponding elements from rControls
    void                        ImplActivateControl( bool bFirst ) const;

public:
                            StdTabController();
                            virtual ~StdTabController();

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  FindControl( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rCtrls, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & rxCtrlModel );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTabController
    void SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& Model ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& Container ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SAL_CALL getContainer(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL getControls(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL autoTabOrder(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL activateTabOrder(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL activateFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL activateLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};



#endif // _TOOLKIT_AWT_STDTABCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
