/*************************************************************************
 *
 *  $RCSfile: createdispatcher.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-07-20 08:07:31 $
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

#ifndef __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_
#include <dispatch/basedispatcher.hxx>
#endif

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
    //  debug methods
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS
    private:
        static sal_Bool implcp_ctor( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory ,
                                     const css::uno::Reference< css::frame::XFrame >&              xParent  ,
                                     const ::rtl::OUString&                                        sName    );
    #endif

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:
        TaskCreator                                     m_aTaskCreator  ;   /// we need it to create new tasks on demand
        css::uno::WeakReference< css::frame::XFrame >   m_xTarget       ;   /// new created frame (Don't hold hard reference ... target frame couldn't die then!)
        ::rtl::OUString                                 m_sTargetName   ;   /// name of new created frame

};      //  class CreateDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_
