/*************************************************************************
 *
 *  $RCSfile: JDriver.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:05:00 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_DRIVER_HXX_
#include "java/sql/Driver.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
#include "java/lang/Class.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_
#include "java/sql/DriverManager.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERPOPERTYINFO_HXX_
#include "java/sql/DriverPropertyInfo.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#include "java/sql/Connection.hxx"
#endif
#include "java/util/Property.hxx"
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
java_sql_Driver::java_sql_Driver(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : m_xORB(_rxFactory)
{
}
// --------------------------------------------------------------------------------
java_sql_Driver::~java_sql_Driver()
{}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString java_sql_Driver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.JDBCDriver");
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > java_sql_Driver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL connectivity::java_sql_Driver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new java_sql_Driver(_rxFactory));
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_Driver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Driver::connect( const ::rtl::OUString& url, const
                                                         Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    Reference< XConnection > xOut;
    if ( acceptsURL(url ) )
    {
        java_sql_Connection* pConnection = new java_sql_Connection(this );
        xOut = pConnection;
        if ( !pConnection->construct(url,info) )
            xOut = NULL; // an error occured and the java driver doesn't throw an exception
    }
    return xOut;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::acceptsURL( const ::rtl::OUString& url ) throw(SQLException, RuntimeException)
{
    // don't ask the real driver for the url
    // I feel responsible for all jdbc url's
    static const ::rtl::OUString s_sJdbcPrefix = ::rtl::OUString::createFromAscii("jdbc:");
    return 0 == url.compareTo(s_sJdbcPrefix, 5);
}
// -------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL java_sql_Driver::getPropertyInfo( const ::rtl::OUString& url,
                                                                         const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBoolean(2);
        aBoolean[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
        aBoolean[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1"));

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClass"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The JDBC driver class name."))
                ,sal_True
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.mysql.jdbc.Driver"))
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParameterNameSubstitution"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Change named parameters with '?'."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreDriverPrivileges"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ignore the privileges from the database driver."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoRetrievingEnabled"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Retrieve generated values."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoRetrievingStatement"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Auto-increment statement."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(aDriverInfo.begin(),aDriverInfo.size());
    }
    ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);
    return Sequence< DriverPropertyInfo >();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------


