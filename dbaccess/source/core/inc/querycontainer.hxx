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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCONTAINER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCONTAINER_HXX

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerApproveListener.hpp>

#include "definitioncontainer.hxx"
#include "apitools.hxx"

namespace dbtools
{
    class WarningsContainer;
}

namespace dbaccess
{

    typedef ::cppu::ImplHelper  <   css::container::XContainerListener
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
        enum AGGREGATE_ACTION { NONE, INSERTING, FLUSHING };
        AGGREGATE_ACTION        m_eDoingCurrently;

        /** a class which automatically resets m_eDoingCurrently in its destructor
        */
        class OAutoActionReset; // just for the following friend declaration
        friend class OAutoActionReset;
        class OAutoActionReset
        {
            OQueryContainer*        m_pActor;
        public:
            OAutoActionReset(OQueryContainer* _pActor) : m_pActor(_pActor) { }
            ~OAutoActionReset() { m_pActor->m_eDoingCurrently = NONE; }
        };

        // ODefinitionContainer
        virtual css::uno::Reference< css::ucb::XContent > createObject( const OUString& _rName) override;
        virtual bool checkExistence(const OUString& _rName) override;

        // helper
        virtual void SAL_CALL disposing() override;
        virtual ~OQueryContainer();

        /** ctor of the container. The parent has to support the <type scope="css::sdbc">XConnection</type>
            interface.<BR>

            @param _pWarnings
                specifies a warnings container (May be <NULL/>)

                Any errors which occur during the lifetime of the query container,
                which cannot be reported as exceptionts (for instance in methods where throwing an SQLException is
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
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;

        // XContainerApproveListener
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveInsertElement( const css::container::ContainerEvent& Event ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveReplaceElement( const css::container::ContainerEvent& Event ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::util::XVeto > SAL_CALL approveRemoveElement( const css::container::ContainerEvent& Event ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::lang::XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XDataDescriptorFactory
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const OUString& elementName ) throw(css::sdbc::SQLException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(css::sdbc::SQLException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // css::container::XElementAccess
        virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;
    // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    // css::container::XNameAccess
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(css::uno::RuntimeException, std::exception) override;

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

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
