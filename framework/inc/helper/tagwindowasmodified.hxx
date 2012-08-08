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

#ifndef __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_
#define __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>

#include <cppuhelper/weak.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          listen for modify events on model and tag frame container window so it can react accordingly
    @descr          Used e.g. by our MAC port where such state is shown seperately on some controls of the
                    title bar.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during startup of instances of this class.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      draft
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TagWindowAsModified :   // interfaces
                              public css::lang::XTypeProvider,
                              public css::lang::XInitialization,
                              public css::frame::XFrameActionListener, // => XEventListener
                              public css::util::XModifyListener,       // => XEventListener
                              // baseclasses (order neccessary for right initialization!)
                              private ThreadHelpBase,
                              public  ::cppu::OWeakObject
{
    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame, where we listen for new loaded documents for updating our own xModel reference
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// reference to the frame container window, where we must set the tag
        css::uno::WeakReference< css::awt::XWindow > m_xWindow;

        /// we list on the model for modify events
        css::uno::WeakReference< css::frame::XModel > m_xModel;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 TagWindowAsModified(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TagWindowAsModified(                                                                   );

        //____________________________
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //____________________________
        // XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    private:

        //____________________________
        // @todo document me
        void impl_update(const css::uno::Reference< css::frame::XFrame >& xFrame);

}; // class TagWindowAsModified

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
