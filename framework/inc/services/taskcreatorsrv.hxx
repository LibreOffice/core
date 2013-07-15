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

#ifndef __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
#define __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

#include <cppuhelper/implbase2.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// definition

/// [XFrame] if it's set, it will be used as parent frame for the new created frame.
const char ARGUMENT_PARENTFRAME[] = "ParentFrame"; // XFrame

/** [OUString] if it's not a special name (beginning with "_" ... which are not allowed here!)
               it will be set as the API name of the new created frame.
 */
const char ARGUMENT_FRAMENAME[] = "FrameName"; // OUString

/// [sal_Bool] If its set to sal_True we will make the new created frame visible.
const char ARGUMENT_MAKEVISIBLE[] = "MakeVisible"; // sal_Bool

/** [sal_Bool] If not "ContainerWindow" property is set it force creation of a
               top level window as new container window.
 */
const char ARGUMENT_CREATETOPWINDOW[] = "CreateTopWindow"; // sal_Bool

/// [Rectangle] Place the new created frame on this place and resize the container window.
const char ARGUMENT_POSSIZE[] = "PosSize"; // Rectangle

/// [XWindow] an outside created window, used as container window of the new created frame.
const char ARGUMENT_CONTAINERWINDOW[] = "ContainerWindow"; // XWindow

/** [sal_Bool] enable/disable special mode, where this frame will be part of
               the persistent window state feature suitable for any office module window
 */
const char ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE[] = "SupportPersistentWindowState"; // sal_Bool

/** [sal_Bool] enable/disable special mode, where the title bar of our
               the new created frame will be updated automaticly.
               Default = ON !
 */
const char ARGUMENT_ENABLE_TITLEBARUPDATE[] = "EnableTitleBarUpdate"; // sal_Bool


namespace framework
{

//_______________________________________________
/**
 *  TODO document me
 */
class TaskCreatorService : // attention! Must be the first base class to guarentee right initialize lock ...
                           private ThreadHelpBase,
                           public ::cppu::WeakImplHelper2<
                               css::lang::XServiceInfo,
                               css::lang::XSingleServiceFactory>
{
    //___________________________________________
    // member

    private:

        //---------------------------------------
        /** @short  the global uno service manager.
            @descr  Must be used to create own needed services.
         */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    //___________________________________________
    // interface

    public:

                 TaskCreatorService(const css::uno::Reference< css::uno::XComponentContext >& xContext);
        virtual ~TaskCreatorService(                                                                   );

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XSingleServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance()
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);
    //___________________________________________
    // helper

    private:

        css::uno::Reference< css::awt::XWindow > implts_createContainerWindow( const css::uno::Reference< css::awt::XWindow >& xParentWindow ,
                                                                               const css::awt::Rectangle&                      aPosSize      ,
                                                                                     sal_Bool                                  bTopWindow    );

        void implts_applyDocStyleToWindow(const css::uno::Reference< css::awt::XWindow >& xWindow) const;

        css::uno::Reference< css::frame::XFrame2 > implts_createFrame( const css::uno::Reference< css::frame::XFrame >& xParentFrame     ,
                                                                      const css::uno::Reference< css::awt::XWindow >&  xContainerWindow ,
                                                                      const OUString&                           sName            );

        void implts_establishWindowStateListener( const css::uno::Reference< css::frame::XFrame2 >& xFrame );
        void implts_establishTitleBarUpdate( const css::uno::Reference< css::frame::XFrame2 >& xFrame );

        void implts_establishDocModifyListener( const css::uno::Reference< css::frame::XFrame2 >& xFrame );

        OUString impl_filterNames( const OUString& sName );
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
