/*************************************************************************
 *
 *  $RCSfile: callablestatement.cxx,v $
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
#ifndef _DBA_COREAPI_CALLABLESTATEMENT_HXX_
#include <callablestatement.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OCallableStatement::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XRow > *)0 ),
                           ::getCppuType( (const Reference< XOutParameters > *)0 ),
                            OPreparedStatement::getTypes() );

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OCallableStatement::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OCallableStatement::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OPreparedStatement::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XRow * >( this ),
                    static_cast< XOutParameters * >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void OCallableStatement::acquire() throw (RuntimeException)
{
    OPreparedStatement::acquire();
}

//--------------------------------------------------------------------------
void OCallableStatement::release() throw (RuntimeException)
{
    OPreparedStatement::release();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OCallableStatement::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OCallableStatement");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OCallableStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_CALLABLESTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_CALLABLESTATEMENT;
    return aSNS;
}

// XOutParameters
//------------------------------------------------------------------------------
void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerOutParameter( parameterIndex, sqlType, typeName );
}

//------------------------------------------------------------------------------
void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerNumericOutParameter( parameterIndex, sqlType, scale );
}

// XRow
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OCallableStatement::wasNull(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->wasNull();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getString( columnIndex );
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBoolean( columnIndex );
}

//------------------------------------------------------------------------------
sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getByte( columnIndex );
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getShort( columnIndex );
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getInt( columnIndex );
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getLong( columnIndex );
}

//------------------------------------------------------------------------------
float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getFloat( columnIndex );
}

//------------------------------------------------------------------------------
double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDouble( columnIndex );
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBytes( columnIndex );
}

//------------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDate( columnIndex );
}

//------------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTime( columnIndex );
}

//------------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTimestamp( columnIndex );
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBinaryStream( columnIndex );
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getCharacterStream( columnIndex );
}

//------------------------------------------------------------------------------
Any SAL_CALL OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getObject( columnIndex, typeMap );
}

//------------------------------------------------------------------------------
Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getRef( columnIndex );
}

//------------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBlob( columnIndex );
}

//------------------------------------------------------------------------------
Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getClob( columnIndex );
}

//------------------------------------------------------------------------------
Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getArray( columnIndex );
}


