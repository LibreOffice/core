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

#ifndef __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_

#include <framework/fwedllapi.h>

#include <vector>

#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/implbase1.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace css = ::com::sun::star;

namespace framework{

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
struct ThreadHelpBase2
{
    public:
        mutable ::osl::Mutex m_aLock;
};

class FWE_DLLPUBLIC PreventDuplicateInteraction : private ThreadHelpBase2
                                    ,public ::cppu::WeakImplHelper1< css::task::XInteractionHandler2 >
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

        //_________________________________
        /**
            @interface  XInteractionHandler2
            @short      called from outside to handle a problem
            @descr      We filter the incoming interactions. some of them
                        will be forwarded to the generic UI interaction handler.
                        So we must not implement it twice. Some other ones
                        will be aborted only.

            @threadsafe yes
        */
        virtual ::sal_Bool SAL_CALL handleInteractionRequest( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest )
            throw (::com::sun::star::uno::RuntimeException);

        //_________________________________
        /**
            @interface  XInterface
            @short      called to query another interface of the component
            @descr      Will allow to query for XInteractionHandler2 if and only if m_xHandler supports this interface, too.

            @threadsafe yes
        */
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);
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
                        sal_True if the queried interaction could be found.
                        sal_False otherwise.

            @threadsafe yes
        */
        virtual sal_Bool getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                  PreventDuplicateInteraction::InteractionInfo* pReturn     ) const;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_INTERACTION_PREVENTDUPLICATEINTERACTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
