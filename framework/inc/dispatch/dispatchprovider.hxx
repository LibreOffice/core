/*************************************************************************
 *
 *  $RCSfile: dispatchprovider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-07-20 08:08:45 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

enum EDispatchHelper
{
    E_MENUDISPATCHER        ,
    E_MAILTODISPATCHER      ,
    E_HELPAGENTDISPATCHER   ,
    E_CREATEDISPATCHER      ,
    E_BLANKDISPATCHER       ,
    E_SELFDISPATCHER        ,
    E_PLUGINDISPATCHER
};

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement a helper for XDispatchProvider interface
    @descr          Use this class as member only! Never use it as baseclass.
                    XInterface will be ambigous and we hold a weakreference to ouer OWNER - not to ouer SUPERCLASS!

    @implements     XInterface
                    XDispatchProvider
                    XEventListener

    @base           ThreadHelpBase
                    TransactionBase
                    OWeakObject

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class DispatchProvider  :   // interfaces
                            public  css::lang::XTypeProvider            ,
                            public  css::frame::XDispatchProvider       ,
                            public  css::lang::XEventListener           ,
                            // baseclasses
                            // Order is neccessary for right initialization!
                            private ThreadHelpBase                      ,
                            private TransactionBase                     ,
                            public  ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        //  constructor / destructor
        DispatchProvider( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory ,
                          const css::uno::Reference< css::frame::XFrame >&              xFrame   );

        //  XInterface
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //  XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >                       SAL_CALL queryDispatch       ( const css::util::URL&                                       aURL             ,
                                                                                                                  const ::rtl::OUString&                                      sTargetFrameName ,
                                                                                                                        sal_Int32                                             nSearchFlags     ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches     ( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

        //   XEventListener
        virtual void                                                               SAL_CALL disposing           ( const css::lang::EventObject&                               aEvent           ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        // Let him protected!
        virtual ~DispatchProvider();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        css::uno::Reference< css::frame::XDispatchProvider > implts_getOrCreateAppDispatchProvider(                                     );
        css::uno::Reference< css::frame::XDispatch >         implts_getOrCreateDispatchHelper     ( EDispatchHelper         eHelper     ,
                                                                                                    const css::uno::Any&    aParameters = css::uno::Any() );

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS
    private:
        static sal_Bool implcp_ctor           ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory        ,
                                                const css::uno::Reference< css::frame::XFrame >&              xFrame          );
        static sal_Bool implcp_queryDispatch  ( const css::util::URL&                                         aURL            ,
                                                const ::rtl::OUString&                                        sTargetFrameName,
                                                      sal_Int32                                               nSearchFlags    );
        static sal_Bool implcp_queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >&   lDescriptions   );
        static sal_Bool implcp_disposing      ( const css::lang::EventObject&                                 aEvent          );
    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory                  ;   /// reference to global service manager to create new services
        css::uno::WeakReference< css::frame::XFrame >               m_xFrame                    ;   /// weakreference to owner frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::Reference< css::frame::XDispatchProvider >        m_xAppDispatchProvider      ;   /// For some dispatches we should call our global app dispatch provider
        css::uno::Reference< css::frame::XDispatch >                m_xMenuDispatcher           ;   /// different dispatcher to handle special dispatch calls, protocols or URLs
        css::uno::Reference< css::frame::XDispatch >                m_xHelpAgentDispatcher      ;
        css::uno::Reference< css::frame::XDispatch >                m_xMailToDispatcher         ;
        css::uno::Reference< css::frame::XDispatch >                m_xBlankDispatcher          ;
        css::uno::Reference< css::frame::XDispatch >                m_xSelfDispatcher           ;
        css::uno::Reference< css::frame::XDispatch >                m_xPlugInDispatcher         ;

};      //  class DispatchProvider

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
