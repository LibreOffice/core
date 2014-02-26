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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARWRAPPER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARWRAPPER_HXX

#include <helper/uiconfigelementwrapperbase.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework
{

class ToolBarManager;
class ToolBarWrapper : public ::com::sun::star::ui::XUIFunctionListener,
                       public UIConfigElementWrapperBase
{
    public:
        ToolBarWrapper( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );
        virtual ~ToolBarWrapper();

        // XInterface
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XUIElementSettings
        virtual void SAL_CALL updateSettings() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XUIFunctionListener
        virtual void SAL_CALL functionExecute( const OUString& aUIElementName, const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception);


    //  protected methods

    protected:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any&  aValue ) throw( com::sun::star::uno::Exception, std::exception );
        virtual void impl_fillNewData();

    private:
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >              m_xToolBarManager;
        com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
