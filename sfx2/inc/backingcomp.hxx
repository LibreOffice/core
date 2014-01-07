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

#ifndef INCLUDED_SFX2_INC_BACKINGCOMP_HXX
#define INCLUDED_SFX2_INC_BACKINGCOMP_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/weak.hxx>

//__________________________________________
// definition

//__________________________________________
/**
    implements the backing component.

    This component is a special one, which doesn't provide a controller
    nor a model. It supports the following features:
        - Drag & Drop
        - Key Accelerators
        - Simple Menu
        - Progress Bar
        - Background
 */
class BackingComp : public  css::lang::XTypeProvider
                  , public  css::lang::XServiceInfo
                  , public  css::lang::XInitialization
                  , public  css::frame::XController  // => XComponent
                  , public  css::awt::XKeyListener // => XEventListener
                  , public css::frame::XDispatchProvider
                  , public css::frame::XDispatch
                  , public  ::cppu::OWeakObject
{
    //______________________________________
    // member

    private:
        /** the global uno service manager.
            Must be used to create own needed services. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** reference to the component window. */
        css::uno::Reference< css::awt::XWindow > m_xWindow;

        /** the owner frame of this component. */
        css::uno::Reference< css::frame::XFrame > m_xFrame;

    //______________________________________
    // interface

    public:

                 BackingComp( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~BackingComp(                                                                    );

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException);
        virtual void          SAL_CALL acquire       (                             ) throw(                          );
        virtual void          SAL_CALL release       (                             ) throw(                          );

        // XTypeProvide
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes           () throw(css::uno::RuntimeException);
        virtual css::uno::Sequence< sal_Int8 >       SAL_CALL getImplementationId() throw(css::uno::RuntimeException);

        // XServiceInfo
        virtual OUString                       SAL_CALL getImplementationName   (                                     ) throw(css::uno::RuntimeException);
        virtual sal_Bool                              SAL_CALL supportsService         ( const OUString& sServiceName ) throw(css::uno::RuntimeException);
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(                                     ) throw(css::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArgs ) throw(css::uno::Exception, css::uno::RuntimeException);

        // XController
        virtual void                                      SAL_CALL attachFrame    ( const css::uno::Reference< css::frame::XFrame >& xFrame   ) throw(css::uno::RuntimeException);
        virtual sal_Bool                                  SAL_CALL attachModel    ( const css::uno::Reference< css::frame::XModel >& xModel   ) throw(css::uno::RuntimeException);
        virtual sal_Bool                                  SAL_CALL suspend        (       sal_Bool                                   bSuspend ) throw(css::uno::RuntimeException);
        virtual css::uno::Any                             SAL_CALL getViewData    (                                                           ) throw(css::uno::RuntimeException);
        virtual void                                      SAL_CALL restoreViewData( const css::uno::Any&                             aData    ) throw(css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel       (                                                           ) throw(css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame       (                                                           ) throw(css::uno::RuntimeException);

        // XKeyListener
        virtual void SAL_CALL keyPressed ( const css::awt::KeyEvent& aEvent ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& aEvent ) throw(css::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose            (                                                                   ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addEventListener   ( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException);

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName , sal_Int32 nSearchFlags ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );

    //______________________________________
    // helper

    public:

        static css::uno::Sequence< OUString >                   SAL_CALL impl_getStaticSupportedServiceNames(                                                                     );
        static OUString                                         SAL_CALL impl_getStaticImplementationName   (                                                                     );
        static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ) throw( css::uno::Exception );
        static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
};

#endif // INCLUDED_SFX2_INC_BACKINGCOMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
