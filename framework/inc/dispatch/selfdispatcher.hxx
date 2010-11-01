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
