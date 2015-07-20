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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_COLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_COLUMN_HXX

#include "columnsettings.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/TColumnsHelper.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/propshlp.hxx>
#include <osl/diagnose.h>

namespace dbaccess
{


    //  OColumn

    typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::container::XNamed
                                           >   OColumnBase;

    class OColumn   :public comphelper::OBaseMutex
                    ,public OColumnBase
                    ,public ::comphelper::OPropertyContainer
                    ,public IPropertyContainer  // convenience for the derived class which also derive from OColumnSettings
    {
        friend class OColumns;

    protected:
        // <properties>
        OUString m_sName;
        // </properties>

    protected:
        OColumn( const bool _bNameIsReadOnly );

    public:
        virtual ~OColumn();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XNamed
        virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setName( const OUString& _rName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void fireValueChange( const ::connectivity::ORowSetValue& _rOldValue );

    protected:
        // IPropertyContainer
        virtual void registerProperty( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, void* _pPointerToMember, const ::com::sun::star::uno::Type& _rMemberType ) SAL_OVERRIDE;
        virtual void registerMayBeVoidProperty( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, ::com::sun::star::uno::Any* _pPointerToMember, const ::com::sun::star::uno::Type& _rExpectedType ) SAL_OVERRIDE;
    };

    // IColumnFactory - used by OColumns for creating new columns
    class SAL_NO_VTABLE IColumnFactory
    {
    public:
        /** creates a OColumn object which should represent the column with a given name
        */
        virtual OColumn*
            createColumn( const OUString& _rName ) const = 0;

        /** creates a column descriptor object.

            A column descriptor object is used to append new columns to the collection. If such an append
            actually happened, columnAppended is called afterwards.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor() = 0;

        /** notifies that a column, created from a column descriptor, has been appended
        */
        virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor ) = 0;

        /** notifies that a column with a given name has been dropped
        */
        virtual void columnDropped( const OUString& _sName ) = 0;

    protected:
        ~IColumnFactory() {}
    };

    class OContainerMediator;
    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XChild > TXChild;
    typedef connectivity::OColumnsHelper OColumns_BASE;

    class OColumns : public OColumns_BASE
                    ,public TXChild
    {
        OContainerMediator*     m_pMediator;

    protected:
        // comes from the driver can be null
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xDrvColumns;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >       m_xParent;
        IColumnFactory*                             m_pColFactoryImpl;
        ::connectivity::sdbcx::IRefreshableColumns* m_pRefreshColumns;

        bool                                    m_bInitialized  : 1;
        bool                                    m_bAddColumn    : 1;
        bool                                    m_bDropColumn   : 1;

        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor() SAL_OVERRIDE;
        virtual connectivity::sdbcx::ObjectType appendObject( const OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) SAL_OVERRIDE;
        virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) SAL_OVERRIDE;

    public:
        connectivity::sdbcx::ObjectType createBaseObject(const OUString& _rName)
        {
            return OColumns_BASE::createObject(_rName);
        }
        /** flag which determines whether the container is filled or not
        */
        inline bool isInitialized() const { return m_bInitialized; }
        inline void     setInitialized() {m_bInitialized = true;}
        inline void     setMediator(OContainerMediator* _pMediator) { m_pMediator = _pMediator; }

    public:
        /** constructs an empty container without configuration location.
            @param      rParent             the parent object. This instance will be used for refcounting, so the parent
                                            cannot die before the container does.
            @param      _rMutex             the mutex of the parent.
            @param      _bCaseSensitive     the initial case sensitivity flag
            @see        setCaseSensitive
        */
        OColumns(
                ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                bool _bCaseSensitive,
                const ::std::vector< OUString>& _rVector,
                IColumnFactory* _pColFactory,
                ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                bool _bAddColumn = false,
                bool _bDropColumn = false,
                bool _bUseHardRef = true);

        OColumns(
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
            bool _bCaseSensitive,
            const ::std::vector< OUString> &_rVector,
            IColumnFactory* _pColFactory,
            ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
            bool _bAddColumn = false,
            bool _bDropColumn = false,
            bool _bUseHardRef = true);
        virtual ~OColumns();

        //XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE { OColumns_BASE::acquire(); }
        virtual void SAL_CALL release() throw() SAL_OVERRIDE { OColumns_BASE::release(); }
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void append(const OUString& rName, OColumn*);
        void clearColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

    private:
        using OColumns_BASE::setParent;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_COLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
