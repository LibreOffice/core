/*************************************************************************
 *
 *  $RCSfile: query.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:18:11 $
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

#ifndef _DBA_COREAPI_QUERY_HXX_
#define _DBA_COREAPI_QUERY_HXX_

#ifndef _DBA_COREAPI_QUERYDESCRIPTOR_HXX_
#include "querydescriptor.hxx"
#endif
#ifndef _DBA_CORE_CONFIGURATIONFLUSHABLE_HXX_
#include "configurationflushable.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

//==========================================================================
//= OQuery_LINUX - an object implementing the sdb.Query service
//==========================================================================
typedef ::cppu::ImplHelper2 <   ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                ::com::sun::star::beans::XPropertyChangeListener
                            >   OQuery_Base;
class OQuery;
typedef ::comphelper::OPropertyArrayUsageHelper< OQuery >   OQuery_ArrayHelperBase;
class OQuery_LINUX  :public OQueryDescriptor
                ,public OQuery_Base
                ,public OConfigurationFlushable
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xCommandDefinition;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;
    sal_Bool            m_bCaseSensitiv : 1;        // assume case sensitivity of the column names ?
    sal_Bool            m_bColumnsOutOfDate : 1;    // the columns have to be rebuild on the next getColumns ?

    // possible actions on our "aggregate"
    enum AGGREGATE_ACTION { NONE, SETTING_PROPERTIES, FLUSHING };
    AGGREGATE_ACTION    m_eDoingCurrently;

    // ------------------------------------------------------------------------
    /** a class which automatically resets m_eDoingCurrently in it's destructor
    */
    class OAutoActionReset; // just for the following friend declaration
    friend class OAutoActionReset;
    class OAutoActionReset
    {
        OQuery_LINUX*               m_pActor;
    public:
        OAutoActionReset(OQuery_LINUX* _pActor) : m_pActor(_pActor) { }
        ~OAutoActionReset() { m_pActor->m_eDoingCurrently = NONE; }
    };

protected:
    ~OQuery_LINUX();

public:
    OQuery_LINUX(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxCommandDefinition,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);

// ::com::sun::star::uno::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(::com::sun::star::uno::RuntimeException) { OQueryDescriptor::acquire(); }
    virtual void SAL_CALL release(  ) throw(::com::sun::star::uno::RuntimeException) { OQueryDescriptor::release(); }

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// ::com::sun::star::sdbcx::XColumnsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

// ::com::sun::star::sdbcx::XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource );

// OPropertySetHelper
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue )
            throw (::com::sun::star::uno::Exception);

public:
// helper

// pseudo-XComponent
    virtual void SAL_CALL dispose();

// OQueryDescriptor
    virtual void    initializeFrom(
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigLocation);

protected:
// OConfigurationFlushable
    virtual void flush_NoBroadcast();

};
class OQuery : public OQuery_LINUX
        ,public OQuery_ArrayHelperBase
{
public:
    OQuery( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxCommandDefinition,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn)
    : OQuery_LINUX(_rxCommandDefinition,_rxConn){}
// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
    using OQuery_ArrayHelperBase::getArrayHelper;
};

#endif // _DBA_COREAPI_QUERY_HXX_


