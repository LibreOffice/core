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

#pragma once

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>

#include <cppuhelper/implbase.hxx>

namespace dbaccess
{

    // SubComponentLoader
    typedef ::cppu::WeakImplHelper<   css::awt::XWindowListener
                                  >   SubComponentLoader_Base;
    /** is a helper class which loads/opens a given sub component as soon as the main application
        window becomes visible.
    */
    class SubComponentLoader : public SubComponentLoader_Base
    {
    public:
        SubComponentLoader(
            const css::uno::Reference< css::frame::XController >& i_rApplicationController,
            const css::uno::Reference< css::ucb::XCommandProcessor >& i_rSubDocumentDefinition
        );

        SubComponentLoader(
            const css::uno::Reference< css::frame::XController >& i_rApplicationController,
            const css::uno::Reference< css::lang::XComponent >& i_rNonDocumentComponent
        );

        // XWindowListener
        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) override;
        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) override;
        virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    protected:
        virtual ~SubComponentLoader() override;

    private:
        const css::uno::Reference< css::ucb::XCommandProcessor >  mxDocDefCommands;
        const css::uno::Reference< css::lang::XComponent >        mxNonDocComponent;
        css::uno::Reference< css::awt::XWindow >                  mxAppComponentWindow;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
