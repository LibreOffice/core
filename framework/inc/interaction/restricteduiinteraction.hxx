/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: restricteduiinteraction.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __FRAMEWORK_INTERACTION_RESTRICTEDUIINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_RESTRICTEDUIINTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>
#include <loadstate.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

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

/**
    @short      handle interactions visible ... but ignore some of them
    @descr      Sometimes it's neccessary to use a visible interaction handler,
                which ignore some of all possible ones. So for example requests for
                missing password or ambigous filter must be solved by the real UI user.
                So we have to show dialogs. But some interactions are showing of message
                boxes only. They doesn't help to solve a problem. They inform the user only.
                But such things can be done by the code, which uses this interaction handler too.
                So we collect this informations only and break the outside process.

    @modified   12.08.2002 08:11
    @by         Andreas Schlüns
 */
class RestrictedUIInteraction : public  css::lang::XTypeProvider
                              , public  css::task::XInteractionHandler
                              , private ThreadHelpBase
                              , public  ::cppu::OWeakObject
{
    //_____________________________________
    // member
    private:

        /// in case an unknown interaction was aborted - we save it for our external user!
        css::uno::Any m_aRequest;
        /// some interaction has to be forwarded to a generic UI interaction handler implementation
        css::uno::Reference< css::task::XInteractionHandler > m_xGenericUIHandler;
        /// for some interactions which supports a non visible retry, we must kn ow, how often they was handled before
        sal_Int32 m_nIORetry;
        sal_Int32 m_nNetworkRetry;
        sal_Int32 m_nChaosRetry;
        sal_Int32 m_nWrongMediumRetry;
        sal_Int32 m_nWrongJavaVersionRetry;
        sal_Int32 m_nBadPartnershipRetry;
        /// for retries we must know, when we have to stop it
        sal_Int32 m_nMaxRetry;

    //_____________________________________
    // uno interface
    public:

        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //_________________________________
        /**
            @interface  XInteractionHandler
            @short      called from outside to handle a problem
            @descr      We filter the incoming interactions. some of them
                        will be forwarded to the generic UI interaction handler.
                        So we must not implement it twice. Some other ones
                        will be aborted only.

            @threadsafe yes
        */
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException );

    //_____________________________________
    // c++ interface
    public:

        //_________________________________
        /**
            @short      ctor to guarantee right initialized instances of this class
            @descr      It uses the given uno service manager to create the global
                        generic UI interaction handler for later internal using.

            @param      xSMGR
                            uno service manager for creating services internaly

            @threadsafe not neccessary
        */
        RestrictedUIInteraction( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR        ,
                                       sal_Int32                                               nMaxRetry = 2);

        //_________________________________
        /**
            @short      return the handled interaction request
            @descr      We save the last getted interaction request internaly.
                        May the outside user of this class is interessted
                        on that. Especialy we gotted an unknown interaction
                        and aborted it hard.

                        We save only the last one - because only this interaction can
                        have breaked the operation. All possible ones before was solved.

            @return     [com.sun.star.uno.Any]
                            the packed interaction request
                            Can be empty if no interaction was used!

            @threadsafe yes
        */
        css::uno::Any getRequest() const;

        //_________________________________
        /**
            @short      returns information if interaction was used
            @descr      It can be usefully to know the reason for a failed operation.

            @return     [boolean]
                            <TRUE/> for used interaction
                            <FALSE/> otherwhise

            @threadsafe yes
        */
        sal_Bool wasUsed() const;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_INTERACTION_RESTRICTEDUIINTERACTION_HXX_
