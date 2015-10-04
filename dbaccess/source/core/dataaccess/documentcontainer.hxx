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
#include <cppuhelper/implbase.hxx>
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
typedef ::cppu::ImplHelper  <   css::frame::XComponentLoader
                            ,   css::lang::XMultiServiceFactory
                            ,   css::container::XHierarchicalNameContainer
                            ,   css::container::XHierarchicalName
                            ,   css::embed::XTransactedObject
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
          const css::uno::Reference< css::uno::XComponentContext >& _xORB
        , const css::uno::Reference< css::uno::XInterface >&  _xParentContainer
        ,const TContentPtr& _pImpl
        , bool _bFormsContainer
        );

    // css::uno::XInterface
    DECLARE_XINTERFACE( )

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

    // XComponentLoader
    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponentFromURL( const OUString& URL, const OUString& TargetFrameName, sal_Int32 SearchFlags, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) throw (css::io::IOException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // css::lang::XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand, sal_Int32 CommandId, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) throw (css::uno::Exception, css::ucb::CommandAbortedException, css::uno::RuntimeException, std::exception) override ;

    // XHierarchicalNameAccess
    virtual css::uno::Any SAL_CALL getByHierarchicalName( const OUString& _sName ) throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString& _sName ) throw (css::uno::RuntimeException, std::exception) override;

    // XHierarchicalNameContainer
    virtual void SAL_CALL insertByHierarchicalName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByHierarchicalName( const OUString& Name ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XHierarchicalName
    virtual OUString SAL_CALL getHierarchicalName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL composeHierarchicalName( const OUString& aRelativeName ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XNameContainer
    virtual void SAL_CALL removeByName( const OUString& _rName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XHierarchicalNameReplace
    virtual void SAL_CALL replaceByHierarchicalName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XTransactedObject
    virtual void SAL_CALL commit(  ) throw (css::io::IOException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL revert(  ) throw (css::io::IOException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // helper
    ::rtl::Reference<OContentHelper> getContent(const OUString& _sName) const;
    css::uno::Reference< css::embed::XStorage > getContainerStorage() const;

protected:
    virtual ~ODocumentContainer() override;

    /** OContentHelper
    */
    virtual OUString determineContentType() const override;

    // ODefinitionContainer
    virtual css::uno::Reference< css::ucb::XContent > createObject(
        const OUString& _rName
    ) override;

    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _rDefault ) const override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
