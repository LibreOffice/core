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
                            in case this Method returns <sal_True/> the referred string object
                            will be used to set the original message of the
                            aborted io exception on it.
                            If method returns <sal_False/> rReason was not used.

            @return     [boolean]
                            <sal_True/> in case it was an IO error
                            <sal_False/> in case it wasn't an IO error or interaction was not used
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
