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

#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#define _DBA_CORE_QUERYCONTAINER_HXX_

#include <cppuhelper/implbase5.hxx>
#include <comphelper/stl_types.hxx>
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
    class IWarningsContainer;
}

namespace dbaccess
{

    typedef ::cppu::ImplHelper5 <   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::container::XContainerApproveListener
                                ,   ::com::sun::star::sdbcx::XDataDescriptorFactory
                                ,   ::com::sun::star::sdbcx::XAppend
                                ,   ::com::sun::star::sdbcx::XDrop
                                >   OQueryContainer_Base;

    //==========================================================================
    //= OQueryContainer
    //==========================================================================
    class OQueryContainer   : public ODefinitionContainer
                            , public OQueryContainer_Base
    {
    private:
        ::dbtools::IWarningsContainer*  m_pWarnings;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                        m_xCommandDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                        m_xConnection;
        // possible actions on our "aggregate"
        enum AGGREGATE_ACTION { NONE, INSERTING, FLUSHING };
        AGGREGATE_ACTION        m_eDoingCurrently;

        /** a class which automatically resets m_eDoingCurrently in it's destructor
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
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject( const OUString& _rName);
        virtual sal_Bool checkExistence(const OUString& _rName);

        // helper
        virtual void SAL_CALL disposing();
        virtual ~OQueryContainer();

        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>

            @param _pWarnings
                specifies a warnings container (May be <NULL/>)

                Any errors which occur during the lifetime of the query container,
                which cannot be reported as exceptionts (for instance in methods where throwing an SQLException is
                not allowed) will be appended to this container.</p>
                <p>The caller is responsible for ensuring the lifetime of the object pointed to by this parameter.
        */
        OQueryContainer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxCommandDefinitions,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            ::dbtools::IWarningsContainer* _pWarnings
            );

        void init();

    public:
        static rtl::Reference<OQueryContainer> create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxCommandDefinitions,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            ::dbtools::IWarningsContainer* _pWarnings
            );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        DECLARE_SERVICE_INFO();

    // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XContainerApproveListener
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveInsertElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveReplaceElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveRemoveElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDataDescriptorFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XNameAccess
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);

    private:
        // OContentHelper overridables
        virtual OUString determineContentType() const;

        // helper
        /** create a query object wrapping a CommandDefinition given by name. To retrieve the object, the CommandDescription
            container will be asked for the given name.<BR>
            The returned object is acquired once.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const OUString& _rName);
        /// create a query object wrapping a CommandDefinition. The returned object is acquired once.
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxCommandDesc);

    };
}   // namespace dbaccess

#endif // _DBA_CORE_QUERYCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
