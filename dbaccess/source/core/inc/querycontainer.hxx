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

#pragma once

#include <cppuhelper/implbase5.hxx>
#include <connectivity/CommonTools.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/container/XContainerApproveListener.hpp>

#include "definitioncontainer.hxx"

namespace dbtools
{
    class WarningsContainer;
}

namespace dbaccess
{

    typedef ::cppu::ImplHelper5 <   css::container::XContainerListener
                                ,   css::container::XContainerApproveListener
                                ,   css::sdbcx::XDataDescriptorFactory
                                ,   css::sdbcx::XAppend
                                ,   css::sdbcx::XDrop
                                >   OQueryContainer_Base;

    // OQueryContainer
    class OQueryContainer   : public ODefinitionContainer
                            , public OQueryContainer_Base
    {
    private:
        ::dbtools::WarningsContainer*  m_pWarnings;
        css::uno::Reference< css::container::XNameContainer >
                                        m_xCommandDefinitions;
        css::uno::Reference< css::sdbc::XConnection >
                                        m_xConnection;
        // possible actions on our "aggregate"
        enum class AggregateAction { NONE, Inserting };
        AggregateAction        m_eDoingCurrently;

        /** a class which automatically resets m_eDoingCurrently in its destructor
        */
        class OAutoActionReset; // just for the following friend declaration
        friend class OAutoActionReset;
        class OAutoActionReset
        {
            OQueryContainer&        m_rActor;
        public:
            OAutoActionReset(OQueryContainer& _rActor) : m_rActor(_rActor) { }
            ~OAutoActionReset() { m_rActor.m_eDoingCurrently = AggregateAction::NONE; }
        };

        // ODefinitionContainer
        virtual css::uno::Reference< css::ucb::XContent > createObject( const OUString& _rName) override;
        virtual bool checkExistence(const OUString& _rName) override;

        // helper
        virtual void SAL_CALL disposing() override;
        virtual ~OQueryContainer() override;

        /** ctor of the container. The parent has to support the <type scope="css::sdbc">XConnection</type>
            interface.<BR>

            @param _pWarnings
                specifies a warnings container (May be <NULL/>)

                Any errors which occur during the lifetime of the query container,
                which cannot be reported as exceptions (for instance in methods where throwing an SQLException is
                not allowed) will be appended to this container.</p>
                <p>The caller is responsible for ensuring the lifetime of the object pointed to by this parameter.
        */
        OQueryContainer(
            const css::uno::Reference< css::container::XNameContainer >& _rxCommandDefinitions,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConn,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            ::dbtools::WarningsContainer* _pWarnings
            );

        void init();

    public:
        static rtl::Reference<OQueryContainer> create(
            const css::uno::Reference< css::container::XNameContainer >& _rxCommandDefinitions,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConn,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            ::dbtools::WarningsContainer* _pWarnings
            );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        DECLARE_SERVICE_INFO();

    // css::container::XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

        // XContainerApproveListener
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveInsertElement( const css::container::ContainerEvent& Event ) override;
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveReplaceElement( const css::container::ContainerEvent& Event ) override;
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveRemoveElement( const css::container::ContainerEvent& Event ) override;

    // css::lang::XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // css::sdbcx::XDataDescriptorFactory
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override;

    // css::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

    // css::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const OUString& elementName ) override;
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) override;

    // css::container::XElementAccess
        virtual sal_Bool SAL_CALL hasElements(  ) override;
    // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) override;
    // css::container::XNameAccess
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;

    private:
        // OContentHelper overridables
        virtual OUString determineContentType() const override;

        // helper
        /** create a query object wrapping a CommandDefinition given by name. To retrieve the object, the CommandDescription
            container will be asked for the given name.<BR>
            The returned object is acquired once.
        */
        css::uno::Reference< css::ucb::XContent > implCreateWrapper(const OUString& _rName);
        /// create a query object wrapping a CommandDefinition. The returned object is acquired once.
        css::uno::Reference< css::ucb::XContent > implCreateWrapper(const css::uno::Reference< css::ucb::XContent >& _rxCommandDesc);

    };
}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
