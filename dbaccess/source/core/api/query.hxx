/*************************************************************************
 *
 *  $RCSfile: query.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:03:13 $
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
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
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
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif

#include <map>

//........................................................................
namespace dbaccess
{
//........................................................................

    class IWarningsContainer;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xCommandDefinition;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > m_xCommandPropInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xColumnMediator;
    OContainerMediator* m_pMediator;
    IWarningsContainer* m_pWarnings;
    sal_Bool            m_bCaseSensitiv : 1;        // assume case sensitivity of the column names ?

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
    void                setWarningsContainer( IWarningsContainer* _pWarnings )  { m_pWarnings = _pWarnings; }
    IWarningsContainer* getWarningsContainer( ) const                           { return m_pWarnings; }

    // XRename
    virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual void SAL_CALL disposing();

    virtual OColumn* createColumn(const ::rtl::OUString& _rName) const;

    virtual void rebuildColumns( );

private:
    /** creates column objects for all columns which are necessary when executing our command
    @param _bOnlyTemporary
        if <TRUE/>, the resulting columns are inserted into m_aColumnMap only, else they're really appended to m_pColumns
    */
    void implCollectColumns( );

    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREAPI_QUERY_HXX_


