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

#ifndef INCLUDED_FRAMEWORK_INC_INTERACTION_QUIETINTERACTION_HXX
#define INCLUDED_FRAMEWORK_INC_INTERACTION_QUIETINTERACTION_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>

#include <cppuhelper/implbase.hxx>

namespace framework{

/**
    @short      handle interactions non visible
    @descr      Sometimes it's necessary to use a non visible interaction handler.
                He can't do anything, which a visible one can handle.
                But it can be used to intercept problems e.g. during loading of documents.

                In current implementation we solve conflicts for following situations only:
                    - InteractiveIOException
                    - InteractiveAugmentedIOException
                All other requests will be aborted.
 */
class QuietInteraction : public  ::cppu::WeakImplHelper<
                                    css::task::XInteractionHandler >
{
    // member
    private:

        /// in case an unknown interaction was aborted - we save it for our external user!
        css::uno::Any m_aRequest;

    // uno interface
    public:

        // XInterface, XTypeProvider

        /**
            @interface  XInteractionHandler
            @short      called from outside to handle a problem
            @descr      The only interaction we can handle here is to
                        decide which of two ambigous filters should be really used.
                        We use the user selected one every time.
                        All other request will be aborted and can break the code,
                        which use this interaction handler.

                        But you can use another method of this class to check for
                        some special interactions too: IO Exceptions
                        May a ComponentLoader needs that to throw suitable exception
                        on his own interface.

            @threadsafe yes
        */
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException, std::exception ) override;

    // c++ interface
    public:
        /**
            @short      ctor to guarantee right initialized instances of this class
            @threadsafe not necessary
        */
        QuietInteraction();

        /**
            @short      return the handled interaction request
            @descr      We saved any obtained interaction request internally.
                        Maybe the outside user of this class is interested
                        on that. Especially we got an unknown interaction
                        and aborted it hard.

            @return     [com.sun.star.uno.Any]
                            the packed interaction request
                            Can be empty if no interaction was used!

            @threadsafe yes
        */
        css::uno::Any getRequest() const;

        /**
            @short      returns information if interaction was used
            @descr      It can be useful to know the reason for a failed operation.

            @return     [boolean]
                            <TRUE/> for used interaction
                            <FALSE/> otherwise

            @threadsafe yes
        */
        bool wasUsed() const;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_INTERACTION_QUIETINTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
