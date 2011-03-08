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

#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#define _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace uno { class Any; }
    namespace ucb { class XCommandEnvironment; }
} } }

namespace ucbhelper
{

//============================================================================
/** Cancel the execution of a command by throwing the appropriate exception.
    If an Interaction Handler is given with the command environment and the
    handler handles the exception by selecting the supplied continuation,
    then this function will put the original exception supplied into a
    com::sun::star::ucb::CommandFailedException and throw the
    CommandFailedException. If no handler was given or the handler was not
    able to handle the exception, then the given exception will be thrown
    directly.

    NOTE THAT THIS FUNCTION NEVER RETURNS! IT ALWAYS THROWS AN EXCEPTION!

    @param rException is the exception describing the error to handle.

    @param xEnv is the command environment that may contain an Interaction
           Handler to use before throwing the appropriate exception.
 */
UCBHELPER_DLLPUBLIC void cancelCommandExecution( const com::sun::star::uno::Any & rException,
                             const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XCommandEnvironment > &
                                    xEnv )
    throw( com::sun::star::uno::Exception );

/** Cancel the execution of a command by throwing the appropriate exception.
    If an Interaction Handler is given with the command environment and the
    handler handles the exception by selecting the supplied continuation,
    then this function will put the original exception supplied into a
    com::sun::star::ucb::CommandFailedException and throw the
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
UCBHELPER_DLLPUBLIC void cancelCommandExecution( const com::sun::star::ucb::IOErrorCode eError,
                             const com::sun::star::uno::Sequence<
                                com::sun::star::uno::Any > & rArgs,
                             const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XCommandEnvironment > &
                                    xEnv,
                             const rtl::OUString & rMessage = rtl::OUString(),
                             const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XCommandProcessor > &
                                    xContext = 0 )
    throw( com::sun::star::uno::Exception );
}

#endif // _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
