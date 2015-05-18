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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTCONTAINER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTCONTAINER_HXX

#include "definitioncontainer.hxx"
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include "apitools.hxx"

namespace dbaccess
{
typedef ::cppu::ImplHelper5 <   ::com::sun::star::frame::XComponentLoader
                            ,   ::com::sun::star::lang::XMultiServiceFactory
                            ,   ::com::sun::star::container::XHierarchicalNameContainer
                            ,   ::com::sun::star::container::XHierarchicalName
                            ,   ::com::sun::star::embed::XTransactedObject
                            >   ODocumentContainer_Base;
// ODocumentContainer -    collections of database documents (reports/forms)
class ODocumentContainer    : public ODefinitionContainer
                            , public ODocumentContainer_Base
                            , public ::comphelper::OPropertyStateContainer
                            , public ::comphelper::OPropertyArrayUsageHelper< ODocumentContainer >
{
    bool        m_bFormsContainer;

public:
    /** constructs the container.<BR>
    */
    ODocumentContainer(
          const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  _xParentContainer
        ,const TContentPtr& _pImpl
        , bool _bFormsContainer
        );

    // ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

    // XComponentLoader
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL loadComponentFromURL( const OUString& URL, const OUString& TargetFrameName, sal_Int32 SearchFlags, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCommandProcessor
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;

    // XHierarchicalNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByHierarchicalName( const OUString& _sName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString& _sName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHierarchicalNameContainer
    virtual void SAL_CALL insertByHierarchicalName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeByHierarchicalName( const OUString& Name ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHierarchicalName
    virtual OUString SAL_CALL getHierarchicalName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL composeHierarchicalName( const OUString& aRelativeName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XNameContainer
    virtual void SAL_CALL removeByName( const OUString& _rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHierarchicalNameReplace
    virtual void SAL_CALL replaceByHierarchicalName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTransactedObject
    virtual void SAL_CALL commit(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL revert(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

    // helper
    ::rtl::Reference<OContentHelper> getContent(const OUString& _sName) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getContainerStorage() const;

protected:
    virtual ~ODocumentContainer();

    /** OContentHelper
    */
    virtual OUString determineContentType() const SAL_OVERRIDE;

    // ODefinitionContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(
        const OUString& _rName
    ) SAL_OVERRIDE;

    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const SAL_OVERRIDE;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;
};

}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
