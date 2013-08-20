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

#ifndef __FRAMEWORK_INTERACTION_QUIETINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_QUIETINTERACTION_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>

#include <cppuhelper/implbase1.hxx>

namespace framework{

/**
    @short      handle interactions non visible
    @descr      Sometimes it's neccessary to use a non visible interaction handler.
                He can't do anything, which a visible one can handle.
                But it can be used to intercept problems e.g. during loading of documents.

                In current implementation we solve conflicts for following situations only:
                    - AmbigousFilterRequest
                    - InteractiveIOException
                    - InteractiveAugmentedIOException
                All other requests will be aborted.
 */
class QuietInteraction : private ThreadHelpBase
                       , public  ::cppu::WeakImplHelper1<
                                    css::task::XInteractionHandler >
{
    //_____________________________________
    // member
    private:

        /// in case an unknown interaction was aborted - we save it for our external user!
        css::uno::Any m_aRequest;

    //_____________________________________
    // uno interface
    public:

        // XInterface, XTypeProvider

        //_________________________________
        /**
            @interface  XInteractionHandler
            @short      called from outside to handle a problem
            @descr      The only interaction we can handle here is to
                        decide which of two ambigous filters should be realy used.
                        We use the user selected one every time.
                        All other request will be aborted and can break the code,
                        which use this interaction handler.

                        But you can use another method of this class to check for
                        some special interactions too: IO Exceptions
                        May a ComponentLoader needs that to throw suitable exception
                        on his own interface.

            @threadsafe yes
        */
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException );

    //_____________________________________
    // c++ interface
    public:

        //_________________________________
        /**
            @short      ctor to guarantee right initialized instances of this class
            @descr      -

            @threadsafe not neccessary
        */
        QuietInteraction();

        //_________________________________
        /**
            @short      return the handled interaction request
            @descr      We saved any getted interaction request internaly.
                        May the outside user of this class is interessted
                        on that. Especialy we gotted an unknown interaction
                        and aborted it hard.

            @return     [com.sun.star.uno.Any]
                            the packed interaction request
                            Can be empty if no interaction was used!

            @threadsafe yes
        */
        css::uno::Any getRequest() const;

        //_________________________________
        /**
            @short      returns information if interaction was used
            @descr      It can be useful to know the reason for a failed operation.

            @return     [boolean]
                            <TRUE/> for used interaction
                            <FALSE/> otherwise

            @threadsafe yes
        */
        sal_Bool wasUsed() const;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_INTERACTION_STILLINTERACTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
