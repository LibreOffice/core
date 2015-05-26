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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_QUERY_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_QUERY_HXX

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
    class WarningsContainer;
}

namespace dbaccess
{

// OQuery - an object implementing the sdb.Query service
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
    typedef ::std::map< OUString,OColumn*,::comphelper::UStringMixLess> TNameColumnMap;

protected:
//  TNameColumnMap      m_aColumnMap; // contains all columnnames to columns
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xCommandDefinition;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >       m_xCommandPropInfo;
    ::rtl::Reference< OContainerMediator >                                              m_pColumnMediator;
    ::dbtools::WarningsContainer*                                                       m_pWarnings;
    bool                                                                                m_bCaseSensitiv : 1;        // assume case sensitivity of the column names ?

    // possible actions on our "aggregate"
    enum AGGREGATE_ACTION { NONE, SETTING_PROPERTIES, FLUSHING };
    AGGREGATE_ACTION    m_eDoingCurrently;

    /** a class which automatically resets m_eDoingCurrently in its destructor
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
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper*   getArrayHelper() { return OQuery_ArrayHelperBase::getArrayHelper(); }

public:
    OQuery(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxCommandDefinition,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
        );

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

// ::com::sun::star::sdbcx::XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OPropertySetHelper
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue )
            throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

public:
    // the caller is responsible for the lifetime!
    void                            setWarningsContainer( ::dbtools::WarningsContainer* _pWarnings )   { m_pWarnings = _pWarnings; }
    ::dbtools::WarningsContainer*   getWarningsContainer( ) const                                      { return m_pWarnings; }

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    virtual OColumn* createColumn(const OUString& _rName) const SAL_OVERRIDE;

    virtual void rebuildColumns( ) SAL_OVERRIDE;

    // OContentHelper overridables
    virtual OUString determineContentType() const SAL_OVERRIDE;

private:
    void registerProperties();
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_QUERY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
