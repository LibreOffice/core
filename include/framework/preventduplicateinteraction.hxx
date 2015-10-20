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

#ifndef INCLUDED_FRAMEWORK_PREVENTDUPLICATEINTERACTION_HXX
#define INCLUDED_FRAMEWORK_PREVENTDUPLICATEINTERACTION_HXX

#include <framework/fwedllapi.h>

#include <vector>

#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>

#include <cppuhelper/implbase.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace framework{

/**
    @short      Prevent us from showing the same interaction more than once during
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
                                    ,public ::cppu::WeakImplHelper< css::task::XInteractionHandler2 >
{

    // structs, types etc.
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


    // member
    private:

        /// Used to create needed uno services at runtime.
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** The outside interaction handler, which is used to handle every incoming interaction,
            if it's not blocked. */
        css::uno::Reference< css::task::XInteractionHandler > m_xHandler;

        /** This list describe which and how incoming interactions must be handled.
            Further it contains all collected information after this interaction
            object was used.*/
        InteractionList m_lInteractionRules;


    // uno interface
    public:


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
            throw(css::uno::RuntimeException, std::exception) override;


        /**
            @interface  XInteractionHandler2
            @short      called from outside to handle a problem
            @descr      We filter the incoming interactions. some of them
                        will be forwarded to the generic UI interaction handler.
                        So we must not implement it twice. Some other ones
                        will be aborted only.

            @threadsafe yes
        */
        virtual sal_Bool SAL_CALL handleInteractionRequest( const css::uno::Reference< css::task::XInteractionRequest >& xRequest )
            throw (css::uno::RuntimeException, std::exception) override;


        /**
            @interface  XInterface
            @short      called to query another interface of the component
            @descr      Will allow to query for XInteractionHandler2 if and only if m_xHandler supports this interface, too.

            @threadsafe yes
        */
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
            throw (css::uno::RuntimeException, std::exception) override;

    // c++ interface
    public:


        /**
            @short      ctor to guarantee right initialized instances of this class
            @descr      It uses the given uno service manager to create the global
                        generic UI interaction handler for later internal using.

            @param      xSMGR
                            uno service manager for creating services internally

            @threadsafe not necessary
        */
        PreventDuplicateInteraction(const css::uno::Reference< css::uno::XComponentContext >& rxContext);


        /**
            @short      dtor to free used memory.
         */
        virtual ~PreventDuplicateInteraction();


        /**
            @short      set the outside interaction handler, which must be used internally
                        if the interaction will not be blocked by the set list of rules.

            @note       This overwrites the settings of e.g. useDefaultUUIHandler()!

            @param      xHandler
                        the new interaction handler
         */
        void setHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler);


        /**
            @short      instead of setting an outside interaction handler, this method
                        make sure the default UUI interaction handler of the office is used.

            @note       This overwrites the settings of e.g. setHandler()!
         */
        void useDefaultUUIHandler();


        /**
            @short      add a new interaction to the list of interactions, which
                        must be handled by this helper.

            @descr      This method must be called immediately after a new instance of this helper was
                        created. Without such list of InteractionRules, this instances does nothing!
                        On the other side there is no possibility to remove rules.
                        So the same instance can't be used within different transactions.
                        It's a OneWay-object .-)

            @param      aInteractionInfo
                        describe the type of interaction, hos often it can be called etcpp.

            @threadsafe yes
        */
        void addInteractionRule(const PreventDuplicateInteraction::InteractionInfo& aInteractionInfo);


        /**
            @short      return the info struct for the specified interaction.

            @param      aInteraction
                        specify the interaction.

            @param      pReturn
                        provides information about:
                        - the count how often this interaction was handled during the
                          lifetime of this helper.
                        - the interaction itself, so it can be analyzed further

            @return     [boolean]
                        true if the queried interaction could be found.
                        false otherwise.

            @threadsafe yes
        */
        bool getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                  PreventDuplicateInteraction::InteractionInfo* pReturn     ) const;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_PREVENTDUPLICATEINTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
