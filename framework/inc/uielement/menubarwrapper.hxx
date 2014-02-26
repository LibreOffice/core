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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX

#include <helper/uiconfigelementwrapperbase.hxx>
#include <uielement/menubarmanager.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <boost/unordered_map.hpp>

namespace framework
{

class MenuBarWrapper : public UIConfigElementWrapperBase,
                       public ::com::sun::star::container::XNameAccess

{
    public:
        MenuBarWrapper(
            const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );
        virtual ~MenuBarWrapper();


        //  XInterface, XTypeProvider

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        MenuBarManager* GetMenuBarManager() const { return static_cast< MenuBarManager* >( m_xMenuBarManager.get() ); }
        void            InvalidatePopupControllerCache() { m_bRefreshPopupControllerCache = sal_True; }

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XUIElementSettings
        virtual void SAL_CALL updateSettings(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    private:
        virtual void impl_fillNewData();
        void fillPopupControllerCache();

        sal_Bool                                                                                m_bRefreshPopupControllerCache : 1;
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >                      m_xMenuBarManager;
        PopupControllerCache                                                                    m_aPopupControllerCache;
        com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >              m_xContext;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
