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

class StatusIndicator   : public ::com::sun::star::awt::XLayoutConstrains
                        , public ::com::sun::star::task::XStatusIndicator
                        , public BaseContainerControl
{

    //  public methods

    public:

        //  construct/destruct

        /**_______________________________________________________________________________________________________
            @short
            @descr

            @seealso

            @param

            @return

            @onerror
        */

        StatusIndicator( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        /**_______________________________________________________________________________________________________
            @short
            @descr

            @seealso

            @param

            @return

            @onerror
        */

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

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
            @short      increment refcount
            @seealso    XInterface
            @seealso    release()
            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
            @short      decrement refcount
            @seealso    XInterface
            @seealso    acquire()
            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL release() throw() SAL_OVERRIDE;

        //  XTypeProvider

        /**_______________________________________________________________________________________________________
            @short      get information about supported interfaces
            @seealso    XTypeProvider
            @return     Sequence of types of all supported interfaces

            @onerror    A RuntimeException is thrown.
        */

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XAggregation

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XStatusIndicator

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL start(
            const OUString&  sText   ,
            sal_Int32 nRange
        ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL end() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL reset() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setText( const OUString& sText ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XLayoutConstrains

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XControl

        /**_______________________________________________________________________________________________________
        */

        virtual void SAL_CALL createPeer(
            const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&    xToolkit    ,
            const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent
        ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XComponent

        /**_______________________________________________________________________________________________________
        */

        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  XWindow

        /**_______________________________________________________________________________________________________
        */

        virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                            sal_Int32   nY      ,
                                            sal_Int32   nWidth  ,
                                            sal_Int32   nHeight ,
                                            sal_Int16   nFlags  ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //  BaseControl

        /**_______________________________________________________________________________________________________
        */

        static const ::com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

        /**_______________________________________________________________________________________________________
        */

        static const OUString impl_getStaticImplementationName();

//  protected methods

protected:

        /**_______________________________________________________________________________________________________
        */

        virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
        ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual void impl_paint (
            sal_Int32 nX,
            sal_Int32 nY,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > & rGraphics
        ) SAL_OVERRIDE;

        /**_______________________________________________________________________________________________________
        */

        virtual void impl_recalcLayout( const ::com::sun::star::awt::WindowEvent& aEvent ) SAL_OVERRIDE;

// debug methods

private:

    /**_______________________________________________________________________________________________________
    */

// private variables

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedText >       m_xText;
    rtl::Reference<ProgressBar> m_xProgressBar;

};  // class StatusIndicator

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_SOURCE_INC_STATUSINDICATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
