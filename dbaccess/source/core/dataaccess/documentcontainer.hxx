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

#include <definitioncontainer.hxx>
#include <cppuhelper/implbase5.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <rtl/ref.hxx>
#include <apitools.hxx>

namespace dbaccess
{
typedef ::cppu::ImplHelper5 <   css::frame::XComponentLoader
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

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

    // css::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

    // XComponentLoader
    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponentFromURL( const OUString& URL, const OUString& TargetFrameName, sal_Int32 SearchFlags, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

    // css::lang::XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand, sal_Int32 CommandId, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override ;

    // XHierarchicalNameAccess
    virtual css::uno::Any SAL_CALL getByHierarchicalName( const OUString& _sName ) override;
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString& _sName ) override;

    // XHierarchicalNameContainer
    virtual void SAL_CALL insertByHierarchicalName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByHierarchicalName( const OUString& Name ) override;

    // XHierarchicalName
    virtual OUString SAL_CALL getHierarchicalName(  ) override;
    virtual OUString SAL_CALL composeHierarchicalName( const OUString& aRelativeName ) override;

    // XNameContainer
    virtual void SAL_CALL removeByName( const OUString& _rName ) override;

    // XHierarchicalNameReplace
    virtual void SAL_CALL replaceByHierarchicalName( const OUString& aName, const css::uno::Any& aElement ) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // XTransactedObject
    virtual void SAL_CALL commit(  ) override;
    virtual void SAL_CALL revert(  ) override;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) override;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
