/*************************************************************************
 *
 *  $RCSfile: statusindicatorfactory.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-08-10 11:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#define __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWEVENT_HPP_
#include <com/sun/star/awt/WindowEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

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
    @descr          These struct hold some informations about all currently running progress proccesses.
                    We need a reference to right child indicator wrapper, his ranges and values.
*//*-*************************************************************************************************************/
struct IndicatorInfo
{
    public:
        //---------------------------------------------------------------------------------------------------------
        // Initialize struct with new indicator and set default values for ranges and values
        IndicatorInfo( const css::uno::Reference< css::task::XStatusIndicator >& xNewIndicator )
        {
            xIndicator = xNewIndicator    ;
            sText      = ::rtl::OUString();
            nRange     = 0                ;
            nOldValue  = 0                ;
            nNewValue  = 0                ;
        }

        //---------------------------------------------------------------------------------------------------------
        // Don't forget to free used references!
        ~IndicatorInfo()
        {
            xIndicator = css::uno::Reference< css::task::XStatusIndicator >();
            sText      = ::rtl::OUString()                                   ;
            nRange     = 0                                                   ;
            nOldValue  = 0                                                   ;
            nNewValue  = 0                                                   ;
        }

        //---------------------------------------------------------------------------------------------------------
        // Used by status indicator only, if other values of struct are unknown!
        sal_Bool operator==( const css::uno::Reference< css::task::XStatusIndicator >& rIndicator )
        {
            return( xIndicator == rIndicator );
        }

    public:
        css::uno::Reference< css::task::XStatusIndicator >      xIndicator  ;
        ::rtl::OUString                                         sText       ;
        sal_Int32                                               nRange      ;
        sal_Int32                                               nOldValue   ;
        sal_Int32                                               nNewValue   ;
};

typedef ::std::vector< IndicatorInfo > IndicatorStack;

/*-************************************************************************************************************//**
    @short          implement a factory to create new status indicator objects
    @descr          We use it as helper for our frame implementation.
                    The factory create different indicators and control his access to shared output device!
                    Only the last activated component can write his state to this device.

    @implements     XInterface
                    XStatusIndicatorFactory
                    XWindowListener
                    XEventListener

    @base           ThreadHelpBase
                    TransactionBase
                    OWeakObject

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class StatusIndicatorFactory   :   public  css::task::XStatusIndicatorFactory  ,
                                   public  css::awt::XWindowListener           , // => XEventListener
                                   private ThreadHelpBase                      ,
                                   private TransactionBase                     ,
                                   public  ::cppu::OWeakObject                   // => XInterface
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
        StatusIndicatorFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory      ,
                                const css::uno::Reference< css::frame::XFrame >&              xOwner        ,
                                const css::uno::Reference< css::awt::XWindow >&               xParentWindow );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XINTERFACE

        //---------------------------------------------------------------------------------------------------------
        //  XStatusIndicatorFactory
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL createStatusIndicator() throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XWindowListener
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL windowResized (   const   css::awt::WindowEvent&  aEvent  ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL windowMoved   (   const   css::awt::WindowEvent&  aEvent  ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL windowShown   (   const   css::lang::EventObject& aEvent  ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL windowHidden  (   const   css::lang::EventObject& aEvent  ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  public shared method!
        //---------------------------------------------------------------------------------------------------------
        void start      (   const   css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                            const   ::rtl::OUString&                                    sText  ,
                                    sal_Int32                                           nRange );
        void end        (   const   css::uno::Reference< css::task::XStatusIndicator >& xChild );
        void reset      (   const   css::uno::Reference< css::task::XStatusIndicator >& xChild );
        void setText    (   const   css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                            const   ::rtl::OUString&                                    sText  );
        void setValue   (   const   css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                    sal_Int32                                           nValue );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        virtual ~StatusIndicatorFactory();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        void implts_recalcLayout();

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS

    private:
        static sal_Bool implcp_StatusIndicatorFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory      ,
                                                       const css::uno::Reference< css::frame::XFrame >&              xOwner        ,
                                                       const css::uno::Reference< css::awt::XWindow >&               xParentWindow );
        static sal_Bool implcp_windowResized         ( const css::awt::WindowEvent&                                  aEvent        );
        static sal_Bool implcp_windowMoved           ( const css::awt::WindowEvent&                                  aEvent        );
        static sal_Bool implcp_disposing             ( const css::lang::EventObject&                                 aEvent        );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:

        IndicatorStack                                          m_aStack                    ;   /// stack with all current indicator childs
        css::uno::Reference< css::lang::XMultiServiceFactory >  m_xFactory                  ;   /// uno service manager to create new services
        css::uno::Reference< css::frame::XFrame >               m_xOwner                    ;   /// Reference to our owner frame. We are listener for disposing() to die if he die!
        css::uno::Reference< css::task::XStatusIndicator >      m_xActiveIndicator          ;   /// most active indicator child, which could work with our shared indicator window only
        css::uno::Reference< css::task::XStatusIndicator >      m_xSharedIndicator          ;   /// one shared indicator window for all our indicator childs!
        css::uno::Reference< css::awt::XWindow >                m_xParentWindow             ;   /// we are listener on this window to resize shared statrus indicator
        css::uno::Reference< css::awt::XWindow >                m_xIndicatorWindow          ;   /// we must resize this window, if m_xParentWindow is changed

};      //  class StatusIndicatorFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
