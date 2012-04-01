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

#ifndef _UNOCONTROLS_STATUSINDICATOR_CTRL_HXX
#define _UNOCONTROLS_STATUSINDICATOR_CTRL_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________
#include "basecontainercontrol.hxx"

//____________________________________________________________________________________________________________
//  namespace
//____________________________________________________________________________________________________________

namespace unocontrols{

//____________________________________________________________________________________________________________
//  defines
//____________________________________________________________________________________________________________

#define SERVICENAME_STATUSINDICATOR             "com.sun.star.task.XStatusIndicator"
#define IMPLEMENTATIONNAME_STATUSINDICATOR      "stardiv.UnoControls.StatusIndicator"
#define STATUSINDICATOR_FREEBORDER              5                                                       // border around and between the controls
#define FIXEDTEXT_SERVICENAME                   "com.sun.star.awt.UnoControlFixedText"
#define FIXEDTEXT_MODELNAME                     "com.sun.star.awt.UnoControlFixedTextModel"
#define CONTROLNAME_TEXT                        "Text"                                                  // identifier the control in container
#define CONTROLNAME_PROGRESSBAR                 "ProgressBar"                                           //              -||-
#define STATUSINDICATOR_DEFAULT_TEXT            "\0"
#define STATUSINDICATOR_BACKGROUNDCOLOR         TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )                // lighgray
#define STATUSINDICATOR_LINECOLOR_BRIGHT        TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )                // white
#define STATUSINDICATOR_LINECOLOR_SHADOW        TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )                // black
#define STATUSINDICATOR_DEFAULT_WIDTH           300
#define STATUSINDICATOR_DEFAULT_HEIGHT          25

//____________________________________________________________________________________________________________
//  structs, types
//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________
//  class declaration
//____________________________________________________________________________________________________________

class StatusIndicator   : public ::com::sun::star::awt::XLayoutConstrains
                        , public ::com::sun::star::task::XStatusIndicator
                        , public BaseContainerControl
{

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  construct/destruct
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short
            @descr

            @seealso

            @param

            @return

            @onerror
        */

        StatusIndicator( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

        /**_______________________________________________________________________________________________________
            @short
            @descr

            @seealso

            @param

            @return

            @onerror
        */

        virtual ~StatusIndicator();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      give answer, if interface is supported
            @descr      The interfaces are searched by type.

            @seealso    XInterface

            @param      "rType" is the type of searched interface.

            @return     Any     information about found interface

            @onerror    A RuntimeException is thrown.
        */

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw( ::com::sun::star::uno::RuntimeException );

        /**_______________________________________________________________________________________________________
            @short      increment refcount
            @descr      -

            @seealso    XInterface
            @seealso    release()

            @param      -

            @return     -

            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL acquire() throw();

        /**_______________________________________________________________________________________________________
            @short      decrement refcount
            @descr      -

            @seealso    XInterface
            @seealso    acquire()

            @param      -

            @return     -

            @onerror    A RuntimeException is thrown.
        */

        virtual void SAL_CALL release() throw();

        //---------------------------------------------------------------------------------------------------------
        //  XTypeProvider
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      get information about supported interfaces
            @descr      -

            @seealso    XTypeProvider

            @param      -

            @return     Sequence of types of all supported interfaces

            @onerror    A RuntimeException is thrown.
        */

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
            throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XAggregation
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType )
            throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XStatusIndicator
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL start(
            const ::rtl::OUString&  sText   ,
            sal_Int32 nRange
        ) throw( ::com::sun::star::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL end() throw( ::com::sun::star::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL reset() throw( ::com::sun::star::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setText( const ::rtl::OUString& sText ) throw( ::com::sun::star::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XLayoutConstrains
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize() throw( ::com::sun::star::uno::RuntimeException );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize() throw( ::com::sun::star::uno::RuntimeException );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize )
            throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XControl
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void SAL_CALL createPeer(
            const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&    xToolkit    ,
            const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent
        ) throw( ::com::sun::star::uno::RuntimeException );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel )
            throw( ::com::sun::star::uno::RuntimeException );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
            throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XComponent
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XWindow
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                            sal_Int32   nY      ,
                                            sal_Int32   nWidth  ,
                                            sal_Int32   nHeight ,
                                            sal_Int16   nFlags  ) throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  BaseControl
        //---------------------------------------------------------------------------------------------------------

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        static const ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        static const ::rtl::OUString impl_getStaticImplementationName();

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

protected:

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
        );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void impl_paint (
            sal_Int32 nX,
            sal_Int32 nY,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > & rGraphics
        );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void impl_recalcLayout( const ::com::sun::star::awt::WindowEvent& aEvent );

//____________________________________________________________________________________________________________
// debug methods
//____________________________________________________________________________________________________________

private:

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

//____________________________________________________________________________________________________________
// private variables
//____________________________________________________________________________________________________________

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedText >       m_xText         ;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressBar >     m_xProgressBar  ;

};  // class StatusIndicator

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_STATUSINDICATOR_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
