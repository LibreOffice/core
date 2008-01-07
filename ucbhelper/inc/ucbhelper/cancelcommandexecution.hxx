/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cancelcommandexecution.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:42:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#define _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

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
