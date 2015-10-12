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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXCONTAINER_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXCONTAINER_HXX


#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <toolkit/awt/vclxwindow.hxx>



//  class VCLXContainer


class TOOLKIT_DLLPUBLIC VCLXContainer : public ::com::sun::star::awt::XVclContainer,
                        public ::com::sun::star::awt::XVclContainerPeer,
                        public VCLXWindow
{
public:
                    VCLXContainer();
                    virtual ~VCLXContainer();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclContainer
    void SAL_CALL addVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > SAL_CALL getWindows(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclContainerPeer
    void SAL_CALL enableDialogControl( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setTabOrder( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& WindowOrder, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Tabs, sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& Windows ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};




#endif // INCLUDED_TOOLKIT_AWT_VCLXCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
