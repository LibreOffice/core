/*************************************************************************
 *
 *  $RCSfile: restricteduiinteraction.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-08-12 11:41:52 $
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

#ifndef __FRAMEWORK_INTERACTION_RESTRICTEDUIINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_RESTRICTEDUIINTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_LOADSTATE_H_
#include <loadstate.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

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
