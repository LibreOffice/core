/*************************************************************************
 *
 *  $RCSfile: loadstate.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-08-12 11:40:05 $
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

#ifndef __FRAMEWORK_LOADSTATE_H_
#define __FRAMEWORK_LOADSTATE_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif

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
