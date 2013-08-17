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

#ifndef DBA_COREDATAACESS_COMPONENTDEFINITION_HXX
#define DBA_COREDATAACESS_COMPONENTDEFINITION_HXX

#include "commandbase.hxx"
#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/proparrhlp.hxx>
#include "datasettings.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "ContentHelper.hxx"
#include "apitools.hxx"
#include <column.hxx>
#include <comphelper/implementationreference.hxx>

#include <memory>
namespace dbaccess
{

    typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XColumnsSupplier > OComponentDefinition_BASE;

    class OComponentDefinition_Impl : public OContentHelper_Impl
                                     ,public ODataSettings_Base
    {
    public:
        typedef ::std::map  <   OUString
                            ,   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                            >   Columns;
    typedef Columns::iterator           iterator;
    typedef Columns::const_iterator     const_iterator;

    private:
        Columns             m_aColumns;

    public:
        OUString     m_sSchemaName;
        OUString     m_sCatalogName;

    public:
        OComponentDefinition_Impl();
        virtual ~OComponentDefinition_Impl();

        inline size_t size() const { return m_aColumns.size(); }

        inline const_iterator begin() const   { return m_aColumns.begin(); }
        inline const_iterator end() const     { return m_aColumns.end(); }

        inline const_iterator find( const OUString& _rName ) const { return m_aColumns.find( _rName ); }

        inline void erase( const OUString& _rName ) { m_aColumns.erase( _rName ); }

        inline void insert( const OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
        {
            OSL_PRECOND( m_aColumns.find( _rName ) == m_aColumns.end(), "OComponentDefinition_Impl::insert: there's already an element with this name!" );
            m_aColumns.insert( Columns::value_type( _rName, _rxColumn ) );
        }
    };

class OColumnPropertyListener;
// OComponentDefinition - a database "document" which describes a query
class OComponentDefinition  :public OContentHelper
                            ,public ODataSettings
                            ,public IColumnFactory
                            ,public OComponentDefinition_BASE
                            ,public ::comphelper::OPropertyArrayUsageHelper< OComponentDefinition >
{
    OComponentDefinition();

protected:
    ::std::auto_ptr< OColumns >     m_pColumns;
    ::comphelper::ImplementationReference< OColumnPropertyListener,::com::sun::star::beans::XPropertyChangeListener>
                                    m_xColumnPropertyListener;
    sal_Bool                        m_bTable;

    virtual ~OComponentDefinition();
    virtual void SAL_CALL disposing();

protected:
    OComponentDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        ,sal_Bool _bTable = sal_True);

    const   OComponentDefinition_Impl& getDefinition() const { return dynamic_cast< const OComponentDefinition_Impl& >( *m_pImpl.get() ); }
            OComponentDefinition_Impl& getDefinition()       { return dynamic_cast<       OComponentDefinition_Impl& >( *m_pImpl.get() ); }
public:

    OComponentDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const OUString& _rElementName
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
            ,const TContentPtr& _pImpl
            ,sal_Bool _bTable = sal_True
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XInitialization
    virtual void SAL_CALL initialize( com::sun::star::uno::Sequence< com::sun::star::uno::Any > const & rArguments) throw (com::sun::star::uno::Exception);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // XColumnsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw (::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // IColumnFactory
    virtual OColumn*    createColumn(const OUString& _rName) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor();
    virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor );
    virtual void columnDropped(const OUString& _sName);
    virtual void notifyDataSourceModified() { OContentHelper::notifyDataSourceModified(); }

protected:
// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);

    // OContentHelper overridables
    virtual OUString determineContentType() const;

private:
    void registerProperties();
};

}   // namespace dbaccess

#endif // DBA_COREDATAACESS_COMPONENTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
