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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_BUTTONTOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_BUTTONTOOLBARCONTROLLER_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

namespace framework
{

class ButtonToolbarController : public ::com::sun::star::frame::XStatusListener,
                                public ::com::sun::star::frame::XToolbarController,
                                public ::com::sun::star::lang::XInitialization,
                                public ::com::sun::star::util::XUpdatable,
                                public ::com::sun::star::lang::XComponent,
                                public ::comphelper::OBaseMutex,
                                public ::cppu::OWeakObject
{
    public:
        ButtonToolbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                                 ToolBox* pToolBar,
                                 const OUString& aCommand );
        virtual ~ButtonToolbarController();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        bool                                                                             m_bInitialized : 1,
                                                                                         m_bDisposed : 1;
        OUString                                                                         m_aCommandURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >     m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >      m_xURLTransformer;
        VclPtr<ToolBox>                                                                  m_pToolbar;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_BUTTONTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
