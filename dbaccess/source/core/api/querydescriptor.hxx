/*************************************************************************
 *
 *  $RCSfile: querydescriptor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-18 16:16:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_COREAPI_QUERYDESCRIPTOR_HXX_
#define _DBA_COREAPI_QUERYDESCRIPTOR_HXX_

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef _DBA_CORE_DATASETTINGS_HXX_
#include "datasettings.hxx"
#endif
#ifndef _DBA_CORE_COMMANDBASE_HXX_
#include "commandbase.hxx"
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODescriptorColumn - a single column of an OQueryDescriptor
//==========================================================================
class ODescriptorColumn :public OColumn
                        ,public OColumnSettings
                        ,public ::comphelper::OPropertyArrayUsageHelper< ODescriptorColumn >
{
public:
    ODescriptorColumn(const ::rtl::OUString& _rName);

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);
    void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception);
    void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    virtual OColumnSettings*    getSettings() { return static_cast<OColumnSettings*>(this); }

protected:
    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() { return *getArrayHelper(); }

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
};

//==========================================================================
//= OQueryDescriptor - a query descriptor (as the name suggests :)
//==========================================================================
typedef ::cppu::WeakImplHelper3<
        ::com::sun::star::sdbcx::XColumnsSupplier,
        ::com::sun::star::lang::XUnoTunnel,
        ::com::sun::star::lang::XServiceInfo >  OQueryDescriptor_Base;

class OQueryDescriptor
        :public OQueryDescriptor_Base
        ,public OMutexAndBroadcastHelper
        ,public ODataSettings
        ,public OCommandBase
        ,public ::comphelper::OPropertyArrayUsageHelper< OQueryDescriptor >
        ,public IColumnFactory
{
protected:
    OColumns        m_aColumns;             // our column descriptions

protected:
    ~OQueryDescriptor();

public:
    OQueryDescriptor();
    /** constructs the object with a UNO QueryDescriptor. If you use this ctor, the resulting object
        won't have any column informations (the column container will be empty)
    */
    OQueryDescriptor(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForeignDescriptor);
    OQueryDescriptor(const OQueryDescriptor& _rSource);

// ::com::sun::star::uno::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(::com::sun::star::uno::RuntimeException) { OQueryDescriptor_Base::acquire(); }
    virtual void SAL_CALL release(  ) throw(::com::sun::star::uno::RuntimeException) { OQueryDescriptor_Base::release(); }

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbcx::XColumnsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// persistence
    /** store all configuration relevant informations under the given configuration node
        @param      _rxConfigLocation       the configuration node. must not be readonly
    */
    virtual void    storeTo(const OConfigurationTreeRoot& _rConfigLocation);

    /** initialize with the informations stored under the given configuration node
        @param      _rxConfigLocation       the configuration node.
    */
    virtual void    initializeFrom(const OConfigurationNode& _rConfigLocation);

// pseudo-XComponent
    virtual void SAL_CALL dispose();

protected:
// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

// IColumnFactory
    virtual OColumn*    createColumn(const ::rtl::OUString& _rName) const;

protected:
    // helper
    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREAPI_QUERYDESCRIPTOR_HXX_


