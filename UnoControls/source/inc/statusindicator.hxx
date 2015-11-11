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

#ifndef INCLUDED_UNOCONTROLS_SOURCE_INC_STATUSINDICATOR_HXX
#define INCLUDED_UNOCONTROLS_SOURCE_INC_STATUSINDICATOR_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ref.hxx>

#include "basecontainercontrol.hxx"

namespace unocontrols{

class ProgressBar;

#define STATUSINDICATOR_FREEBORDER              5                                                       // border around and between the controls
#define FIXEDTEXT_SERVICENAME                   "com.sun.star.awt.UnoControlFixedText"
#define FIXEDTEXT_MODELNAME                     "com.sun.star.awt.UnoControlFixedTextModel"
#define CONTROLNAME_TEXT                        "Text"                                                  // identifier the control in container
#define CONTROLNAME_PROGRESSBAR                 "ProgressBar"                                           //              -||-
#define STATUSINDICATOR_BACKGROUNDCOLOR         TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )                // lightgray
#define STATUSINDICATOR_LINECOLOR_BRIGHT        TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )                // white
#define STATUSINDICATOR_LINECOLOR_SHADOW        TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )                // black
#define STATUSINDICATOR_DEFAULT_WIDTH           300
#define STATUSINDICATOR_DEFAULT_HEIGHT          25

//  structs, types

//  class declaration

class StatusIndicator   : public css::awt::XLayoutConstrains
                        , public css::task::XStatusIndicator
                        , public BaseContainerControl
{

    public:

        StatusIndicator( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        virtual ~StatusIndicator();

        //  XInterface

        /**_______________________________________________________________________________________________________
            @short      give answer, if interface is supported
            @descr      The interfaces are searched by type.

            @seealso    XInterface

            @param      "rType" is the type of searched interface.

            @return     Any     information about found interface

            @onerror    A RuntimeException is thrown.
        */

        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
            throw( css::uno::RuntimeException, std::exception ) override;

        /**_______________________________________________________________________________________________________
            @short      increment refcount
            @seealso    XInterface
            @seealso    release()
            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL acquire() throw() override;

        /**_______________________________________________________________________________________________________
            @short      decrement refcount
            @seealso    XInterface
            @seealso    acquire()
            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL release() throw() override;

        //  XTypeProvider

        /**_______________________________________________________________________________________________________
            @short      get information about supported interfaces
            @seealso    XTypeProvider
            @return     Sequence of types of all supported interfaces

            @onerror    A RuntimeException is thrown.
        */

        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
            throw( css::uno::RuntimeException, std::exception ) override;

        //  XAggregation

        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType )
            throw( css::uno::RuntimeException, std::exception ) override;

        //  XStatusIndicator

        virtual void SAL_CALL start(
            const OUString&  sText   ,
            sal_Int32 nRange
        ) throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL end() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL reset() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setText( const OUString& sText ) throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( css::uno::RuntimeException, std::exception ) override;

        //  XLayoutConstrains

        virtual css::awt::Size SAL_CALL getMinimumSize() throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::awt::Size SAL_CALL getPreferredSize() throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize )
            throw( css::uno::RuntimeException, std::exception ) override;

        //  XControl

        virtual void SAL_CALL createPeer(
            const   css::uno::Reference< css::awt::XToolkit >&    xToolkit    ,
            const   css::uno::Reference< css::awt::XWindowPeer >& xParent
        ) throw( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& xModel )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel()
            throw( css::uno::RuntimeException, std::exception ) override;

        //  XComponent

        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

        //  XWindow

        virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                            sal_Int32   nY      ,
                                            sal_Int32   nWidth  ,
                                            sal_Int32   nHeight ,
                                            sal_Int16   nFlags  ) throw( css::uno::RuntimeException, std::exception ) override;

        //  BaseControl

        static const css::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

        static const OUString impl_getStaticImplementationName();

protected:

        virtual css::awt::WindowDescriptor* impl_getWindowDescriptor(
            const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
        ) override;

        virtual void impl_paint (
            sal_Int32 nX,
            sal_Int32 nY,
            const css::uno::Reference< css::awt::XGraphics > & rGraphics
        ) override;

        virtual void impl_recalcLayout( const css::awt::WindowEvent& aEvent ) override;

private:

    css::uno::Reference< css::awt::XFixedText >   m_xText;
    rtl::Reference<ProgressBar>                   m_xProgressBar;

};  // class StatusIndicator

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_SOURCE_INC_STATUSINDICATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
