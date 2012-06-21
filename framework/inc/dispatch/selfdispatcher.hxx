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

#ifndef __FRAMEWORK_DISPATCH_SELFDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_SELFDISPATCHER_HXX_

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
    @short          helper to dispatch into an existing owner frame
    @descr          You should use it as dispatcher for "_self", flag::SELF ... cases, to load a document into an
                    owner frame without creation of a new one.
                    Use this class as member only! Never use it as baseclass.
                    XInterface will be ambigous and we hold a weakreference to ouer OWNER - not to ouer SUPERCLASS!

    @implements     XInterface
                    XDispatch
                    XStatusListener
                    XLoadEventListener
                    XEventListener

    @base           BaseDispatcher
                    OWeakObject

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class SelfDispatcher   :   // -interfaces  ... are supported by our BaseDispatcher!
                           // -baseclasses ... order is neccessary for right initialization!
                           public BaseDispatcher
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        //  ctor
                              SelfDispatcher      ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory   ,
                                                    const css::uno::Reference< css::frame::XFrame >&              xTarget    );
        //  XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                         aURL       ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&        lArguments ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        virtual void SAL_CALL reactForLoadingState ( const css::util::URL&                                      aURL        ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&     lDescriptor ,
                                                     const css::uno::Reference< css::frame::XFrame >&           xTarget     ,
                                                           sal_Bool                                             bState      ,
                                                     const css::uno::Any&                                       aAsyncInfo  );

        virtual void SAL_CALL reactForHandlingState( const css::util::URL&                                      aURL        ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&     lDescriptor ,
                                                           sal_Bool                                             bState      ,
                                                     const css::uno::Any&                                       aAsyncInfo  );

};      //  class SelfDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_SELFDISPATCHER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
