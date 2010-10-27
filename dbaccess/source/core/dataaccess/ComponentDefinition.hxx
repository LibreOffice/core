/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
//........................................................................
namespace dbaccess
{
//........................................................................

    typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XColumnsSupplier > OComponentDefinition_BASE;

    class OComponentDefinition_Impl : public OContentHelper_Impl
                                     ,public ODataSettings_Base
    {
    public:
        typedef ::std::map  <   ::rtl::OUString
                            ,   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                            >   Columns;
    typedef Columns::iterator           iterator;
    typedef Columns::const_iterator     const_iterator;

    private:
        Columns             m_aColumns;

    public:
        ::rtl::OUString     m_sSchemaName;
        ::rtl::OUString     m_sCatalogName;

    public:
        OComponentDefinition_Impl();
        virtual ~OComponentDefinition_Impl();

        inline size_t size() const { return m_aColumns.size(); }

        inline const_iterator begin() const   { return m_aColumns.begin(); }
        inline const_iterator end() const     { return m_aColumns.end(); }

        inline const_iterator find( const ::rtl::OUString& _rName ) const { return m_aColumns.find( _rName ); }

        inline void erase( const ::rtl::OUString& _rName ) { m_aColumns.erase( _rName ); }

        inline void insert( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
        {
            OSL_PRECOND( m_aColumns.find( _rName ) == m_aColumns.end(), "OComponentDefinition_Impl::insert: there's already an element with this name!" );
            m_aColumns.insert( Columns::value_type( _rName, _rxColumn ) );
        }
    };

class OColumnPropertyListener;
//=========================================================================
//= OComponentDefinition - a database "document" which describes a query
//=========================================================================
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
    OComponentDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        ,sal_Bool _bTable = sal_True);

    const   OComponentDefinition_Impl& getDefinition() const { return dynamic_cast< const OComponentDefinition_Impl& >( *m_pImpl.get() ); }
            OComponentDefinition_Impl& getDefinition()       { return dynamic_cast<       OComponentDefinition_Impl& >( *m_pImpl.get() ); }
public:

    OComponentDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const ::rtl::OUString& _rElementName
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
            ,const TContentPtr& _pImpl
            ,sal_Bool _bTable = sal_True
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // XColumnsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw (::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // IColumnFactory
    virtual OColumn*    createColumn(const ::rtl::OUString& _rName) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor();
    virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor );
    virtual void columnDropped(const ::rtl::OUString& _sName);
    virtual void notifyDataSourceModified() { OContentHelper::notifyDataSourceModified(); }

protected:
// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);

    // OContentHelper overridables
    virtual ::rtl::OUString determineContentType() const;

private:
    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // DBA_COREDATAACESS_COMPONENTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
