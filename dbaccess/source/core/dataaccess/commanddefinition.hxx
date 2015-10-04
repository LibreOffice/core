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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_COMMANDDEFINITION_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_COMMANDDEFINITION_HXX

#include "commandbase.hxx"
#include "apitools.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdb/XQueryDefinition.hpp>
#include "datasettings.hxx"
#include "ContentHelper.hxx"
#include "ComponentDefinition.hxx"

#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>

namespace dbaccess
{

// OCommandDefinition - a database "document" which describes a query
    class OCommandDefinition_Impl : public OComponentDefinition_Impl
                                   ,public OCommandBase
    {
    public:
    };

typedef ::cppu::ImplHelper  <   css::sdbcx::XRename,
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
    virtual ~OCommandDefinition();

    OCommandDefinition(const css::uno::Reference< css::uno::XComponentContext >&
        ,const css::uno::Reference< css::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        );

    inline const OCommandDefinition_Impl& getCommandDefinition() const { return dynamic_cast< const OCommandDefinition_Impl& >( *m_pImpl.get() ); }
    inline       OCommandDefinition_Impl& getCommandDefinition()       { return dynamic_cast<       OCommandDefinition_Impl& >( *m_pImpl.get() ); }

public:

    OCommandDefinition(
             const css::uno::Reference< css::uno::XInterface >& _rxContainer
            ,const OUString& _rElementName
            ,const css::uno::Reference< css::uno::XComponentContext >&
            ,const TContentPtr& _pImpl
        );

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

// css::uno::XInterface
    DECLARE_XINTERFACE( )

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::lang::XServiceInfo - static methods
    static css::uno::Sequence< OUString > getSupportedServiceNames_static() throw( css::uno::RuntimeException );
    static OUString getImplementationName_static() throw( css::uno::RuntimeException );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        Create(const css::uno::Reference< css::uno::XComponentContext >&);

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    // overrides to resolve ambiguity
    virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OComponentDefinition::setPropertyValue(p1, p2); }
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p1) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OComponentDefinition::getPropertyValue(p1); }
    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OComponentDefinition::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OComponentDefinition::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OComponentDefinition::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OComponentDefinition::removeVetoableChangeListener(p1, p2); }
    virtual css::uno::Reference<css::ucb::XContentIdentifier> SAL_CALL getIdentifier() throw( css::uno::RuntimeException, std::exception ) override
        { return OComponentDefinition::getIdentifier(); }
    virtual OUString SAL_CALL getContentType() throw( css::uno::RuntimeException, std::exception ) override
        { return OComponentDefinition::getContentType(); }
    virtual void SAL_CALL addContentEventListener(const css::uno::Reference<css::ucb::XContentEventListener>& p1) throw( css::uno::RuntimeException, std::exception ) override
        { OComponentDefinition::addContentEventListener(p1); }
    virtual void SAL_CALL removeContentEventListener(const css::uno::Reference<css::ucb::XContentEventListener>& p1) throw( css::uno::RuntimeException, std::exception ) override
        { OComponentDefinition::removeContentEventListener(p1); }
     virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override
        { OComponentDefinition::dispose(); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw( css::uno::RuntimeException, std::exception ) override
        { OComponentDefinition::addEventListener(p1); }
     virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw( css::uno::RuntimeException, std::exception ) override
        { OComponentDefinition::removeEventListener(p1); }

    // OPropertySetHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );

private:
    // helper
    void registerProperties();
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_COMMANDDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
