/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loadstate.h,v $
 * $Revision: 1.3 $
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

#ifndef __FRAMEWORK_LOADSTATE_H_
#define __FRAMEWORK_LOADSTATE_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_______________________________________________________________________
/**
    These enum values specify all possible results of a load request.
    It doesn't matter, if this load operation used dispatch() or loadComponentFromURL().
    The meaning is everytime the same.
 */
enum ELoadState
{
    E_UNSPECIFIED   =   0,  // indicates the operation was not already started
    E_SUCCESS       =   1,  // the load request was successfull
    E_IOERROR       =   2,  // there was an io error internaly
    E_INTERACTION   =   3,  // there was an interaction, which couldn't be handled (doesn't include IO interactions => see E_IOERROR before)
    E_FAILED        =   4   // for unknown or unspecified errors
};

//_______________________________________________________________________
/**
    Helper, which provides some functionality to identify the reason for
    a failed load request and can describe it.
 */
class LoadStateHelper
{
    public:

        //_________________________________
        /**
            @short      checks if the given interaction request was an io error
            @descr      This information can be used to throw
                        a suitable IOException. (e.g. loadComponentFromURL())

            @param      aRequest
                            the original interaction request, which may produced
                            the failed load request

            @param      rReason
                            in case this Method returns <TRUE/> the referred string object
                            will be used to set the original message of the
                            aborted io exception on it.
                            If method returns <FALSE/> rReason was not used.

            @return     [boolean]
                            <TRUE/> in case it was an IO error
                            <FALSE/> in case it wasn't an IO error or interaction was not used
        */
        static sal_Bool wasIOError( const css::uno::Any& aRequest ,
                                          rtl::OUString& rReason  )
        {
            if ( ! aRequest.hasValue() )
                return sal_False;

            css::ucb::InteractiveIOException           exIOInteractive ;
            css::ucb::InteractiveAugmentedIOException  exIOAugmented   ;

            if (aRequest>>=exIOInteractive)
            {
                rReason = exIOInteractive.Message;
                return sal_True;
            }

            if (aRequest>>=exIOAugmented)
            {
                rReason = exIOAugmented.Message;
                return sal_True;
            }

            return sal_False;
        }
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_LOADSTATE_H_
