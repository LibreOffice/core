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

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#define _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_

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
#include <cppuhelper/implbase2.hxx>

namespace dbaccess
{

// OCommandDefinition - a database "document" which describes a query
    class OCommandDefinition_Impl : public OComponentDefinition_Impl
                                   ,public OCommandBase
    {
    public:
    };

typedef ::cppu::ImplHelper2 <   ::com::sun::star::sdbcx::XRename,
                                ::com::sun::star::sdb::XQueryDefinition
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

    OCommandDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        );

    inline const OCommandDefinition_Impl& getCommandDefinition() const { return dynamic_cast< const OCommandDefinition_Impl& >( *m_pImpl.get() ); }
    inline       OCommandDefinition_Impl& getCommandDefinition()       { return dynamic_cast<       OCommandDefinition_Impl& >( *m_pImpl.get() ); }

public:

    OCommandDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const OUString& _rElementName
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
            ,const TContentPtr& _pImpl
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // overrides to resolve ambiguity
    virtual void SAL_CALL setPropertyValue(const OUString& p1, const com::sun::star::uno::Any& p2) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { OComponentDefinition::setPropertyValue(p1, p2); }
    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(const OUString& p1) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { return OComponentDefinition::getPropertyValue(p1); }
    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::removeVetoableChangeListener(p1, p2); }
    virtual com::sun::star::uno::Reference<com::sun::star::ucb::XContentIdentifier> SAL_CALL getIdentifier() throw( ::com::sun::star::uno::RuntimeException )
        { return OComponentDefinition::getIdentifier(); }
    virtual OUString SAL_CALL getContentType() throw( ::com::sun::star::uno::RuntimeException )
        { return OComponentDefinition::getContentType(); }
    virtual void SAL_CALL addContentEventListener(const com::sun::star::uno::Reference<com::sun::star::ucb::XContentEventListener>& p1) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::addContentEventListener(p1); }
    virtual void SAL_CALL removeContentEventListener(const com::sun::star::uno::Reference<com::sun::star::ucb::XContentEventListener>& p1) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::removeContentEventListener(p1); }
     virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::dispose(); }
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::addEventListener(p1); }
     virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw( ::com::sun::star::uno::RuntimeException )
        { OComponentDefinition::removeEventListener(p1); }

    // XQueryDefinition properties
    virtual OUString getName() throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString getCommand() throw( ::com::sun::star::uno::RuntimeException );
    virtual void setCommand(const OUString&) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool getEscapeProcessing() throw( ::com::sun::star::uno::RuntimeException );
    virtual void setEscapeProcessing(sal_Bool) throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString getUpdateTableName() throw( ::com::sun::star::uno::RuntimeException );
    virtual void setUpdateTableName(const OUString&) throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString getUpdateCatalogName() throw( ::com::sun::star::uno::RuntimeException );
    virtual void setUpdateCatalogName(const OUString&) throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString getUpdateSchemaName() throw( ::com::sun::star::uno::RuntimeException );
    virtual void setUpdateSchemaName(const OUString&) throw( ::com::sun::star::uno::RuntimeException );

    // OPropertySetHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );

private:
    // helper
    void registerProperties();
};

}   // namespace dbaccess

#endif // _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
