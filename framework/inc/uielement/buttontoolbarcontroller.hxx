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

class ButtonToolbarController : public css::frame::XStatusListener,
                                public css::frame::XToolbarController,
                                public css::lang::XInitialization,
                                public css::util::XUpdatable,
                                public css::lang::XComponent,
                                public ::comphelper::OBaseMutex,
                                public ::cppu::OWeakObject
{
    public:
        ButtonToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                 ToolBox* pToolBar,
                                 const OUString& aCommand );
        virtual ~ButtonToolbarController();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        bool                                                   m_bInitialized : 1,
                                                               m_bDisposed : 1;
        OUString                                               m_aCommandURL;
        css::uno::Reference< css::frame::XFrame >              m_xFrame;
        css::uno::Reference< css::uno::XComponentContext >     m_xContext;
        css::uno::Reference< css::util::XURLTransformer >      m_xURLTransformer;
        VclPtr<ToolBox>                                        m_pToolbar;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_BUTTONTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
