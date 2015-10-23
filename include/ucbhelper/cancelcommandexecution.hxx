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

#ifndef INCLUDED_UCBHELPER_CANCELCOMMANDEXECUTION_HXX
#define INCLUDED_UCBHELPER_CANCELCOMMANDEXECUTION_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <ucbhelper/ucbhelperdllapi.h>

namespace com { namespace sun { namespace star {
    namespace uno { class Any; }
    namespace ucb { class XCommandEnvironment; }
} } }

namespace ucbhelper
{


/** Cancel the execution of a command by throwing the appropriate exception.
    If an Interaction Handler is given with the command environment and the
    handler handles the exception by selecting the supplied continuation,
    then this function will put the original exception supplied into a
    css::ucb::CommandFailedException and throw the
    CommandFailedException. If no handler was given or the handler was not
    able to handle the exception, then the given exception will be thrown
    directly.

    NOTE THAT THIS FUNCTION NEVER RETURNS! IT ALWAYS THROWS AN EXCEPTION!

    @param rException is the exception describing the error to handle.

    @param xEnv is the command environment that may contain an Interaction
           Handler to use before throwing the appropriate exception.
 */
UCBHELPER_DLLPUBLIC void cancelCommandExecution( const css::uno::Any & rException,
                             const css::uno::Reference<
                                css::ucb::XCommandEnvironment > &
                                    xEnv )
    throw( css::uno::Exception );

/** Cancel the execution of a command by throwing the appropriate exception.
    If an Interaction Handler is given with the command environment and the
    handler handles the exception by selecting the supplied continuation,
    then this function will put the original exception supplied into a
    css::ucb::CommandFailedException and throw the
    CommandFailedException. If no handler was given or the handler was not
    able to handle the exception, then the given exception will be thrown
    directly.

    NOTE THAT THIS FUNCTION NEVER RETURNS! IT ALWAYS THROWS AN EXCEPTION!

    @param eError is an IO error code.

    @param rArgs is a sequeence containing the arguments to pass along with
           the exception. Each IO error code can be combined with one or
           more additional arguments. Refer to com/sun/star/ucb/IOErroprCode.idl
           for details.

    @param xEnv is the command environment that may contain an Interaction
           Handler to use before throwing the appropriate exception.

    @param rMessage is a text containing additional error information.
           Used as debugging aid only. Passed to the member 'Message' of the
           uno::Exception thrown by this function.

    @param xContext is the command processor executing the command to cancel.
           Used as debugging aid only. Passed to the member 'Context' of the
           uno::Exception thrown by this function.
 */
UCBHELPER_DLLPUBLIC void cancelCommandExecution( const css::ucb::IOErrorCode eError,
                             const css::uno::Sequence< css::uno::Any > & rArgs,
                             const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
                             const OUString & rMessage = OUString(),
                             const css::uno::Reference< css::ucb::XCommandProcessor > & xContext = 0 )
    throw( css::uno::Exception );
}

#endif // INCLUDED_UCBHELPER_CANCELCOMMANDEXECUTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
