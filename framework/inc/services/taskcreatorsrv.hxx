/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
#define __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

#include <cppuhelper/weak.hxx>
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
class TaskCreatorService : public  css::lang::XTypeProvider
                         , public  css::lang::XServiceInfo
                         , public  css::lang::XSingleServiceFactory
                           // attention! Must be the first base class to guarentee right initialize lock ...
                         , private ThreadHelpBase
                         , public  ::cppu::OWeakObject
{
    //___________________________________________
    // member

    private:

        //---------------------------------------
        /** @short  the global uno service manager.
            @descr  Must be used to create own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

    //___________________________________________
    // interface

    public:

                 TaskCreatorService(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TaskCreatorService(                                                                   );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
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

        css::uno::Reference< css::frame::XFrame > implts_createFrame( const css::uno::Reference< css::frame::XFrame >& xParentFrame     ,
                                                                      const css::uno::Reference< css::awt::XWindow >&  xContainerWindow ,
                                                                      const ::rtl::OUString&                           sName            );

        void implts_establishWindowStateListener( const css::uno::Reference< css::frame::XFrame >& xFrame );
        void implts_establishTitleBarUpdate( const css::uno::Reference< css::frame::XFrame >& xFrame );

        void implts_establishDocModifyListener( const css::uno::Reference< css::frame::XFrame >& xFrame );

        ::rtl::OUString impl_filterNames( const ::rtl::OUString& sName );
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
