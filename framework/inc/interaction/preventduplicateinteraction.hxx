/*************************************************************************
 *
 *  $RCSfile: preventduplicateinteraction.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 12:03:33 $
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

#ifndef __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <vector>

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

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#ifdef css
    #error "Conflict during define of namespace alias ..."
#else
    #define css ::com::sun::star
#endif

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/**
    @short      Prevent us from showing the same interaction more then once during
                the same transaction.

    @descr      Every interaction provided to this helper will be safed ... handled by the internal
                used UUIInteractionHandler (!) and never be handled a second time!

                On the other side there exists some interactions, which allow a retry.
                So this helper allow to set a list of interactions combined with a retry value.
 */
struct ThreadHelpBase
{
    public:
        mutable ::osl::Mutex m_aLock;
};

class PreventDuplicateInteraction : public ::cppu::WeakImplHelper1< css::task::XInteractionHandler >
                                  , private ThreadHelpBase
{
    //_____________________________________
    // structs, types etcp.
    public:

        struct InteractionInfo
        {
            public:
                /// describe the interaction.
                css::uno::Type m_aInteraction;
                /// after max count was reached this interaction will be blocked.
                sal_Int32 m_nMaxCount;
                /// count how often this interaction was called.
                sal_Int32 m_nCallCount;
                /** hold the last intercepted request (matching the set interaction type) alive
                so it can be used for further checks */
                css::uno::Reference< css::task::XInteractionRequest > m_xRequest;

            public:

                InteractionInfo(const css::uno::Type& aInteraction,
                                      sal_Int32       nMaxCount   )
                    : m_aInteraction(aInteraction)
                    , m_nMaxCount   (nMaxCount   )
                    , m_nCallCount  (0           )
                {}

                InteractionInfo(const InteractionInfo& aCopy)
                    : m_aInteraction(aCopy.m_aInteraction)
                    , m_nMaxCount   (aCopy.m_nMaxCount   )
                    , m_nCallCount  (aCopy.m_nCallCount  )
                    , m_xRequest    (aCopy.m_xRequest    )
                {}
        };

        typedef ::std::vector< InteractionInfo > InteractionList;

    //_____________________________________
    // member
    private:

        /// Used to create needed uno services at runtime.
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** The outside interaction handler, which is used to handle every incoming interaction,
            if it's not blocked. */
        css::uno::Reference< css::task::XInteractionHandler > m_xHandler;

        /** This list describe which and how incoming interactions must be handled.
            Further it contains all collected informations after this interaction
            object was used.*/
        InteractionList m_lInteractionRules;

    //_____________________________________
    // uno interface
    public:

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
        virtual void SAL_CALL handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
            throw(css::uno::RuntimeException);

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
        PreventDuplicateInteraction(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //_________________________________
        /**
            @short      dtor to free used memory.
         */
        virtual ~PreventDuplicateInteraction();

        //_________________________________
        /**
            @short      set the outside interaction handler, which must be used internaly
                        if the interaction will not be blocked by the set list of rules.

            @note       This overwrites the settings of e.g. useDefaultUUIHandler()!

            @param      xHandler
                        the new interaction handler
         */
        virtual void setHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler);

        //_________________________________
        /**
            @short      instead of setting an outside interaction handler, this method
                        make sure the default UUI interaction handler of the office is used.

            @note       This overwrites the settings of e.g. setHandler()!
         */
        virtual void useDefaultUUIHandler();

        //_________________________________
        /**
            @short      add a new interaction to the list of interactions, which
                        must be handled by this helper.

            @descr      This method must be called immediatly after a new instance of this helper was
                        created. Without such list of InteractionRules, this instances does nothing!
                        On the other side there is no possibility to remove rules.
                        So the same instance cant be used within different transactions.
                        It's a OneWay-object .-)

            @param      aInteractionInfo
                        describe the type of interaction, hos often it can be called etcpp.

            @threadsafe yes
        */
        virtual void addInteractionRule(const PreventDuplicateInteraction::InteractionInfo& aInteractionInfo);

        //_________________________________
        /**
            @short      return the info struct for the specified interaction.

            @param      aInteraction
                        specify the interaction.

            @param      pReturn
                        provides informations about:
                        - the count how often this interaction was handled during the
                          lifetime of this helper.
                        - the interaction itself, so it can be analyzed further

            @return     [boolean]
                        TRUE if the queried interaction could be found.
                        FALSE otherwise.

            @threadsafe yes
        */
        virtual sal_Bool getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                  PreventDuplicateInteraction::InteractionInfo* pReturn     ) const;
};

#undef css

} // namespace framework

#endif // #ifndef __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_
