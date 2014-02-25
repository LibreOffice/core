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

#ifndef EXTENSIONS_DEFAULTHELPPROVIDER_HXX
#define EXTENSIONS_DEFAULTHELPPROVIDER_HXX

#include <com/sun/star/inspection/XPropertyControlObserver.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase2.hxx>

class Window;

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= DefaultHelpProvider
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::inspection::XPropertyControlObserver
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   DefaultHelpProvider_Base;
    class DefaultHelpProvider : public DefaultHelpProvider_Base
    {
    private:
        bool                            m_bConstructed;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >
                                        m_xInspectorUI;

    public:
        DefaultHelpProvider();

        // XServiceInfo - static versions
        static OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        ~DefaultHelpProvider();

        // XPropertyControlObserver
        virtual void SAL_CALL focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        // Service constructors
        void    create( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxUI );

    private:
        Window* impl_getVclControlWindow_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
        OUString impl_getHelpText_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_DEFAULTHELPPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
