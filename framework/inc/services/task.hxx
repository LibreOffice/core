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

#ifndef __FRAMEWORK_SERVICES_TASK_HXX_
#define __FRAMEWORK_SERVICES_TASK_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <services/frame.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XTask.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

#include <tools/link.hxx>
#include <vcl/evntpost.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      implements an special frame - a task frame

    @descr      -

    @implements XTask

    @base       Frame
                OPropertySet
*//*-*************************************************************************************************************/

class Task  :   public css::frame::XTask    ,   // => XFrame => XComponent
                public Frame                    // Order of baseclasses is neccessary for right initialization!
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor to create instance

            @descr      This constructor initialize a new instance of this class,
                        and will be set valid values on his member and baseclasses.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         Task( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor

            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~Task();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XTask
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL close() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL tileWindows() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsVertical() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsHorizontal() throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XComponent
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      We must overwrite this method, because baseclass Frame implements XFrame and XComponent.
                        XTask is derived from these classes to! The compiler don't know, which base is the right one.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException )
        {
            Frame::dispose();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::addEventListener( xListener );
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::removeEventListener( xListener );
        }

        //---------------------------------------------------------------------------------------------------------
        //   XFrame
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      We must overwrite this method, because baseclass Frame implements XFrame and XComponent.
                        XTask is derived from these classes to! The compiler don't know, which base is right.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException )
        {
            Frame::initialize( xWindow );
        }

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getContainerWindow() throw( css::uno::RuntimeException )
        {
            return Frame::getContainerWindow();
        }

        virtual void SAL_CALL setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
        {
            Frame::setCreator( xCreator );
        }

        virtual css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL getCreator() throw( css::uno::RuntimeException )
        {
            return Frame::getCreator();
        }

        virtual ::rtl::OUString SAL_CALL getName() throw( css::uno::RuntimeException )
        {
            return Frame::getName();
        }

        virtual void SAL_CALL setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException )
        {
            Frame::setName( sName );
        }

        virtual sal_Bool SAL_CALL isTop() throw( css::uno::RuntimeException )
        {
            return Frame::isTop();
        }

        virtual void SAL_CALL activate() throw( css::uno::RuntimeException )
        {
            Frame::activate();
        }

        virtual void SAL_CALL deactivate() throw( css::uno::RuntimeException )
        {
            Frame::deactivate();
        }

        virtual sal_Bool SAL_CALL isActive() throw( css::uno::RuntimeException )
        {
            return Frame::isActive();
        }

        virtual sal_Bool SAL_CALL setComponent( const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow    ,
                                                const   css::uno::Reference< css::frame::XController >& xController         ) throw( css::uno::RuntimeException )
        {
            return Frame::setComponent( xComponentWindow, xController );
        }

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() throw( css::uno::RuntimeException )
        {
            return Frame::getComponentWindow();
        }

        virtual css::uno::Reference< css::frame::XController > SAL_CALL getController() throw( css::uno::RuntimeException )
        {
            return Frame::getController();
        }

        virtual void SAL_CALL contextChanged() throw( css::uno::RuntimeException )
        {
            Frame::contextChanged();
        }

        virtual void SAL_CALL addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::addFrameActionListener( xListener );
        }

        virtual void SAL_CALL removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::removeFrameActionListener( xListener );
        }

        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL findFrame(   const   ::rtl::OUString&    sTargetFrameName    ,
                                                                                           sal_Int32            nSearchFlags        ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XTopWindowListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      This object is forced to release all references to the interfaces given
                        by the parameter Source.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:


    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        DECL_LINK( Close_Impl, void* );



    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        // Not used in the moment!

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    protected:

        // But some values are neede by derived classes!
//        sal_Bool            m_bIsPlugIn         ;   /// In objects of these class this member is set to sal_False.
                                                    /// But in derived class PlugInFrame it's overwrited with sal_True!

    private:

        // Properties
        sal_Bool            m_bIsAlwaysVisible  ;
        sal_Bool            m_bIsFloating       ;
        css::awt::Point     m_aPosition         ;
        css::awt::Size      m_aSize             ;
        ::vcl::EventPoster  m_aPoster           ;

};      //  class Tasks

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_TASKS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
