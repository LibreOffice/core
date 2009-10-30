/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: column.hxx,v $
 * $Revision: 1.30 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#define _DBA_COREAPI_COLUMN_HXX_

#include <hash_map>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#include <connectivity/TColumnsHelper.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include <connectivity/FValue.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include <connectivity/sdbcx/IRefreshable.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

namespace dbaccess
{

    //************************************************************
    //  OColumn
    //************************************************************
    typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::container::XNamed,
                                                ::com::sun::star::lang::XUnoTunnel > OColumnBase;

    //------------------------------------------------------------
    class OColumn   :public comphelper::OBaseMutex
                    ,public OColumnBase
                    ,public ::cppu::OPropertySetHelper

    {
        friend class OColumns;

    protected:
        // <properties>
        ::rtl::OUString m_sName;
        // </properties>

    protected:
        OColumn();

    public:
        virtual ~OColumn();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) = 0;

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::cppu::OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XNamed
        virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException);

        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue);

    protected:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };

    //============================================================
    //= IColumnFactory - used by OColumns for creating new columns
    //============================================================
    class SAL_NO_VTABLE IColumnFactory
    {
    public:
        /** creates a OColumn object which should represent the column with a given name
        */
        virtual OColumn*
            createColumn( const ::rtl::OUString& _rName ) const = 0;

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
        virtual void columnDropped( const ::rtl::OUString& _sName ) = 0;
    };

    //============================================================
    //= OColumns
    //= general columns map, could be used for readonly access
    //= no appending and dropping is supported
    //============================================================
    typedef ::std::hash_map<rtl::OUString, OColumn*, ::comphelper::UStringMixHash, ::comphelper::UStringMixEqual> OColumnMap;
    typedef ::std::vector<OColumn*> OColumnArray;

    class OContainerMediator;
    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XChild > TXChild;
    typedef connectivity::OColumnsHelper OColumns_BASE;
    //------------------------------------------------------------
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

        sal_Bool                                    m_bInitialized  : 1;
        sal_Bool                                    m_bAddColumn    : 1;
        sal_Bool                                    m_bDropColumn   : 1;

        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual connectivity::sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

    public:
        connectivity::sdbcx::ObjectType createBaseObject(const ::rtl::OUString& _rName)
        {
            return OColumns_BASE::createObject(_rName);
        }
        /** flag which determines whether the container is filled or not
        */
        inline sal_Bool isInitialized() const { return m_bInitialized; }
        inline void     setInitialized() {m_bInitialized = sal_True;}
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
                sal_Bool _bCaseSensitive,
                const ::std::vector< ::rtl::OUString>& _rVector,
                IColumnFactory* _pColFactory,
                ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                sal_Bool _bAddColumn = sal_False,
                sal_Bool _bDropColumn = sal_False,
                sal_Bool _bUseHardRef = sal_True);

        OColumns(
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
            sal_Bool _bCaseSensitive,
            const ::std::vector< ::rtl::OUString> &_rVector,
            IColumnFactory* _pColFactory,
            ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
            sal_Bool _bAddColumn = sal_False,
            sal_Bool _bDropColumn = sal_False,
            sal_Bool _bUseHardRef = sal_True);
        virtual ~OColumns();

        //XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw() { OColumns_BASE::acquire(); }
        virtual void SAL_CALL release() throw() { OColumns_BASE::release(); }
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        void append(const ::rtl::OUString& rName, OColumn*);
        void clearColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    private:
        using OColumns_BASE::setParent;
    };
}
#endif // _DBA_COREAPI_COLUMN_HXX_

