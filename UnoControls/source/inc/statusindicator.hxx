/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

#define UNO3_ANY                                        ::com::sun::star::uno::Any
#define UNO3_OUSTRING                                   ::rtl::OUString
#define UNO3_RECTANGLE                                  ::com::sun::star::awt::Rectangle
#define UNO3_REFERENCE                                  ::com::sun::star::uno::Reference
#define UNO3_RUNTIMEEXCEPTION                           ::com::sun::star::uno::RuntimeException
#define UNO3_SEQUENCE                                   ::com::sun::star::uno::Sequence
#define UNO3_SIZE                                       ::com::sun::star::awt::Size
#define UNO3_TYPE                                       ::com::sun::star::uno::Type
#define UNO3_XCONTROLMODEL                              ::com::sun::star::awt::XControlModel
#define UNO3_XFIXEDTEXT                                 ::com::sun::star::awt::XFixedText
#define UNO3_XGRAPHICS                                  ::com::sun::star::awt::XGraphics
#define UNO3_XLAYOUTCONSTRAINS                          ::com::sun::star::awt::XLayoutConstrains
#define UNO3_XMULTISERVICEFACTORY                       ::com::sun::star::lang::XMultiServiceFactory
#define UNO3_XPROGRESSBAR                               ::com::sun::star::awt::XProgressBar
#define UNO3_XSTATUSINDICATOR                           ::com::sun::star::task::XStatusIndicator
#define UNO3_XTOOLKIT                                   ::com::sun::star::awt::XToolkit
#define UNO3_XWINDOWPEER                                ::com::sun::star::awt::XWindowPeer

//____________________________________________________________________________________________________________
//  defines
//____________________________________________________________________________________________________________

#define SERVICENAME_STATUSINDICATOR                     "com.sun.star.task.XStatusIndicator"
#define IMPLEMENTATIONNAME_STATUSINDICATOR              "stardiv.UnoControls.StatusIndicator"
#undef  FREEBORDER
#define FREEBORDER                                      5                                                       // border around and between the controls
#define FIXEDTEXT_SERVICENAME                           "com.sun.star.awt.UnoControlFixedText"
#define FIXEDTEXT_MODELNAME                             "com.sun.star.awt.UnoControlFixedTextModel"
#define CONTROLNAME_TEXT                                "Text"                                                  // identifier the control in container
#define CONTROLNAME_PROGRESSBAR                         "ProgressBar"                                           //              -||-
#define DEFAULT_TEXT                                    "\0"
#define BACKGROUNDCOLOR                                 TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )                // lighgray
#define LINECOLOR_BRIGHT                                TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )                // white
#define LINECOLOR_SHADOW                                TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )                // black
// Overwrite defines from basecontrol.hxx!!!
#undef  DEFAULT_WIDTH
#undef  DEFAULT_HEIGHT
#define DEFAULT_WIDTH                                   300
#define DEFAULT_HEIGHT                                   25

//____________________________________________________________________________________________________________
//  structs, types
//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________
//  class declaration
//____________________________________________________________________________________________________________

class StatusIndicator   : public UNO3_XLAYOUTCONSTRAINS
                        , public UNO3_XSTATUSINDICATOR
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

        StatusIndicator( const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >& xFactory );

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

        virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getTypes() throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual UNO3_ANY SAL_CALL queryAggregation( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL start(    const   UNO3_OUSTRING&  sText   ,
                                                sal_Int32       nRange  ) throw( UNO3_RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL end() throw( UNO3_RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL reset() throw( UNO3_RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setText( const UNO3_OUSTRING& sText ) throw( UNO3_RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual UNO3_SIZE SAL_CALL getMinimumSize() throw( UNO3_RUNTIMEEXCEPTION );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual UNO3_SIZE SAL_CALL getPreferredSize() throw( UNO3_RUNTIMEEXCEPTION );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual UNO3_SIZE SAL_CALL calcAdjustedSize( const UNO3_SIZE& aNewSize ) throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL createPeer(   const   UNO3_REFERENCE< UNO3_XTOOLKIT >&    xToolkit    ,
                                            const   UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParent     ) throw( UNO3_RUNTIMEEXCEPTION );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual sal_Bool SAL_CALL setModel( const UNO3_REFERENCE< UNO3_XCONTROLMODEL >& xModel ) throw( UNO3_RUNTIMEEXCEPTION );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual UNO3_REFERENCE< UNO3_XCONTROLMODEL > SAL_CALL getModel() throw( UNO3_RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL dispose() throw( UNO3_RUNTIMEEXCEPTION );

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
                                            sal_Int16   nFlags  ) throw( UNO3_RUNTIMEEXCEPTION );

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

        static const UNO3_SEQUENCE< UNO3_OUSTRING > impl_getStaticSupportedServiceNames();

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        static const UNO3_OUSTRING impl_getStaticImplementationName();

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

        virtual UNO3_WINDOWDESCRIPTOR* impl_getWindowDescriptor( const UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParentPeer );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void impl_paint ( sal_Int32 nX, sal_Int32 nY, const UNO3_REFERENCE< UNO3_XGRAPHICS > & rGraphics );

        /**_______________________________________________________________________________________________________
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        */

        virtual void impl_recalcLayout( const UNO3_WINDOWEVENT& aEvent );

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

    #if OSL_DEBUG_LEVEL > 1

    #endif

//____________________________________________________________________________________________________________
// private variables
//____________________________________________________________________________________________________________

private:

    UNO3_REFERENCE< UNO3_XFIXEDTEXT >       m_xText         ;
    UNO3_REFERENCE< UNO3_XPROGRESSBAR >     m_xProgressBar  ;

};  // class StatusIndicator

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_STATUSINDICATOR_CTRL_HXX
