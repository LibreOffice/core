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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLECONTAINER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLECONTAINER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "FilteredContainer.hxx"
#include <connectivity/warningscontainer.hxx>
#include "RefreshListener.hxx"
#include "apitools.hxx"

namespace dbaccess
{
    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XContainerListener> OTableContainer_Base;

    // OTableContainer
    class OContainerMediator;

    class OTableContainer :  public OFilteredContainer,
                             public OTableContainer_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xTableDefinitions;
        ::rtl::Reference< OContainerMediator >                                          m_pTableMediator;
        bool                m_bInDrop;                  // set when we are in the drop method

        // OFilteredContainer
        virtual void addMasterContainerListener() SAL_OVERRIDE;
        virtual void removeMasterContainerListener() SAL_OVERRIDE;
        virtual OUString getTableTypeRestriction() const SAL_OVERRIDE;

        // ::connectivity::sdbcx::OCollection
        virtual connectivity::sdbcx::ObjectType     createObject(const OUString& _rName) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   createDescriptor() SAL_OVERRIDE;
        virtual connectivity::sdbcx::ObjectType appendObject( const OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) SAL_OVERRIDE;
        virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) SAL_OVERRIDE;

        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE { OFilteredContainer::acquire();}
        virtual void SAL_CALL release() throw() SAL_OVERRIDE { OFilteredContainer::release();}
    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>
            @param          _rParent            the object which acts as parent for the container.
                                                all refcounting is rerouted to this object
            @param          _rMutex             the access safety object of the parent
            @param          _rTableFilter       restricts the visible tables by name
            @param          _rTableTypeFilter   restricts the visible tables by type
            @see            construct
        */
        OTableContainer( ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon,
            bool _bCase,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >&  _xTableDefinitions,
            IRefreshListener*   _pRefreshListener,
            ::dbtools::WarningsContainer* _pWarningsContainer,
            oslInterlockedCount& _nInAppend
            );

        virtual ~OTableContainer();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
