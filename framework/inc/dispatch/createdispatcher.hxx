/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: createdispatcher.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <dispatch/basedispatcher.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

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
    @short          helper to create new frames by using non special target name and CREATE flag
    @descr          If user whish to find any frame by name or will create it, if it not already exist ...
                    these dispatch helper should be used. Because - a frame must be created at dispatch call ...
                    not on queryDispatch()! So we hold weakreferences to the possible parent frame, safe the name
                    of new frame. If user call dispatch at us - we check a weakreference to our last created frame
                    (he should have same name!) ... if he exist - we do nothing ... if not - we create it again.
                    But if possible parent for new created task doesn't exist - we couldn't do anything!
                    Then we wait for our own dispose and do nothing.

    @attention      Use this class as member only! Never use it as baseclass.
                    XInterface will be ambigous and we hold a weakreference to ouer OWNER - not to ouer SUPERCLASS!

    @implements     XInterface
                    XDispatch
                    XStatusListener
                    XLoadEventListener
                    XEventListener

    @base           BaseDispatcher

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class CreateDispatcher  :   // -interfaces  ... are supported by our BaseDispatcher!
                            // -baseclasses ... order is neccessary for right initialization!
                            public BaseDispatcher
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        //  ctor
                              CreateDispatcher    ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory    ,
                                                    const css::uno::Reference< css::frame::XFrame >&              xParent     ,
                                                    const rtl::OUString&                                          sName       );
        //  XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                         aURL        ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&        lArguments  ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        virtual void SAL_CALL reactForLoadingState ( const css::util::URL&                                        aURL        ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&       lDescriptor ,
                                                     const css::uno::Reference< css::frame::XFrame >&             xTarget     ,
                                                           sal_Bool                                               bState      ,
                                                     const css::uno::Any&                                         aAsyncInfo  = css::uno::Any() );

        virtual void SAL_CALL reactForHandlingState( const css::util::URL&                                        aURL        ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&       lDescriptor ,
                                                           sal_Bool                                               bState      ,
                                                     const css::uno::Any&                                         aAsyncInfo  = css::uno::Any() )
        { LOG_WARNING( "CreateDispatcher::reactForHandlingState()", "Who call this function! It's a non used pure virtual function overload ..." ) }

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:
        css::uno::WeakReference< css::frame::XFrame >   m_xTarget       ;   /// new created frame (Don't hold hard reference ... target frame couldn't die then!)
        ::rtl::OUString                                 m_sTargetName   ;   /// name of new created frame

};      //  class CreateDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_
