/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <commandbase.hxx>
#include <apitools.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdb/XQueryDefinition.hpp>
#include <datasettings.hxx>
#include <ContentHelper.hxx>
#include "ComponentDefinition.hxx"

#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase2.hxx>

namespace dbaccess
{

// OCommandDefinition - a database "document" which describes a query
    class OCommandDefinition_Impl : public OComponentDefinition_Impl
                                   ,public OCommandBase
    {
    public:
    };

typedef ::cppu::ImplHelper2 <   css::sdbcx::XRename,
                                css::sdb::XQueryDefinition
                                >   OCommandDefinition_Base;
class OCommandDefinition;
typedef ::comphelper::OPropertyArrayUsageHelper< OCommandDefinition >
                        OCommandDefinition_PROP;

class OCommandDefinition   : public OComponentDefinition
                            ,public OCommandDefinition_Base
                            ,public OCommandDefinition_PROP
{
protected:
    virtual ~OCommandDefinition() override;

public:
    OCommandDefinition(const css::uno::Reference< css::uno::XComponentContext >& ,
        const css::uno::Reference< css::uno::XInterface >& _xParentContainer,
        const TContentPtr& _pImpl);

    OCommandDefinition(
             const css::uno::Reference< css::uno::XInterface >& _rxContainer
            ,const OUString& _rElementName
            ,const css::uno::Reference< css::uno::XComponentContext >&
            ,const TContentPtr& _pImpl
        );

    virtual cpo::uno::Sequence<cpo::uno::Type> getTypes() override;
    virtual cpo::uno::Sequence<sal_Int8> getImplementationId() override;

// css::uno::XInterface
    DECLARE_XINTERFACE( )

// css::lang::XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XRename
    virtual void rename( const OUString& newName ) override;

    // overrides to resolve ambiguity
    virtual void setPropertyValue(const OUString& p1, const cpo::uno::Any& p2) override
        { OComponentDefinition::setPropertyValue(p1, p2); }
    virtual cpo::uno::Any getPropertyValue(const OUString& p1) override
        { return OComponentDefinition::getPropertyValue(p1); }
    virtual void addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { OComponentDefinition::addPropertyChangeListener(p1, p2); }
    virtual void removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { OComponentDefinition::removePropertyChangeListener(p1, p2); }
    virtual void addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { OComponentDefinition::addVetoableChangeListener(p1, p2); }
    virtual void removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { OComponentDefinition::removeVetoableChangeListener(p1, p2); }
    virtual css::uno::Reference<css::ucb::XContentIdentifier> getIdentifier() override
        { return OComponentDefinition::getIdentifier(); }
    virtual OUString getContentType() override
        { return OComponentDefinition::getContentType(); }
    virtual void addContentEventListener(const css::uno::Reference<css::ucb::XContentEventListener>& p1) override
        { OComponentDefinition::addContentEventListener(p1); }
    virtual void removeContentEventListener(const css::uno::Reference<css::ucb::XContentEventListener>& p1) override
        { OComponentDefinition::removeContentEventListener(p1); }
     virtual void dispose() override
        { OComponentDefinition::dispose(); }
    virtual void addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { OComponentDefinition::addEventListener(p1); }
     virtual void removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { OComponentDefinition::removeEventListener(p1); }

    // OPropertySetHelper
    virtual css::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override;
    virtual cppu::IPropertyArrayHelper& getInfoHelper() override;
    virtual cppu::IPropertyArrayHelper* createArrayHelper() const override;


private:
    // helper
    void registerProperties();
};

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
