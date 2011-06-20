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

#ifndef _DBA_COREAPI_QUERY_HXX_
#define _DBA_COREAPI_QUERY_HXX_

#include "querydescriptor.hxx"
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include "ContentHelper.hxx"

#include <map>

namespace dbtools
{
    class IWarningsContainer;
}

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQuery - an object implementing the sdb.Query service
//==========================================================================
typedef ::cppu::ImplHelper3 <   ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                ::com::sun::star::beans::XPropertyChangeListener,
                                ::com::sun::star::sdbcx::XRename
                            >   OQuery_Base;
class OQuery;
class OColumn;
typedef ::comphelper::OPropertyArrayUsageHelper< OQuery >   OQuery_ArrayHelperBase;


class OQuery    :public OContentHelper
                ,public OQueryDescriptor_Base
                ,public OQuery_Base
                ,public OQuery_ArrayHelperBase
                ,public ODataSettings
{
    friend struct TRelease;

public:
    typedef ::std::map< ::rtl::OUString,OColumn*,::comphelper::UStringMixLess> TNameColumnMap;

protected:
//  TNameColumnMap      m_aColumnMap; // contains all columnnames to columns
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xCommandDefinition;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >       m_xCommandPropInfo;
    ::rtl::Reference< OContainerMediator >                                              m_pColumnMediator;
    ::dbtools::IWarningsContainer*                                                      m_pWarnings;
    sal_Bool                                                                            m_bCaseSensitiv : 1;        // assume case sensitivity of the column names ?

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
        OQuery*             m_pActor;
    public:
        OAutoActionReset(OQuery* _pActor) : m_pActor(_pActor) { }
        ~OAutoActionReset() { m_pActor->m_eDoingCurrently = NONE; }
    };

protected:
    virtual ~OQuery();

// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
    ::cppu::IPropertyArrayHelper*   getArrayHelper() { return OQuery_ArrayHelperBase::getArrayHelper(); }

public:
    OQuery(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxCommandDefinition,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

// ::com::sun::star::sdbcx::XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue )
            throw (::com::sun::star::uno::Exception);

public:
    // the caller is responsible for the lifetime!
    void                            setWarningsContainer( ::dbtools::IWarningsContainer* _pWarnings )   { m_pWarnings = _pWarnings; }
    ::dbtools::IWarningsContainer*  getWarningsContainer( ) const                                       { return m_pWarnings; }

    // XRename
    virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual void SAL_CALL disposing();

    virtual OColumn* createColumn(const ::rtl::OUString& _rName) const;

    virtual void rebuildColumns( );

    // OContentHelper overridables
    virtual ::rtl::OUString determineContentType() const;

private:
    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREAPI_QUERY_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
