/*************************************************************************
 *
 *  $RCSfile: resultset.cxx,v $
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
#ifndef _DBA_COREAPI_RESULTSET_HXX_
#include <resultset.hxx>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _DBA_COREAPI_DATACOLUMN_HXX_
#include <datacolumn.hxx>
#endif


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
//using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;

DBG_NAME(OResultSet);

//--------------------------------------------------------------------------
OResultSet::OResultSet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _xResultSet,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xStatement,
                       sal_Bool _bCaseSensitive)
           :OResultSetBase(m_aMutex)
           ,OPropertySetHelper(OResultSetBase::rBHelper)
           ,m_aColumns(*this, m_aMutex, _bCaseSensitive, ::std::vector< ::rtl::OUString>())
           ,m_xAggregateAsResultSet(_xResultSet)
           ,m_bIsBookmarkable(sal_False)
{
    DBG_CTOR(OResultSet, NULL);

    m_aStatement = _xStatement;
    m_xAggregateAsRow = Reference< ::com::sun::star::sdbc::XRow >(m_xAggregateAsResultSet, UNO_QUERY);
    m_xAggregateAsRowUpdate = Reference< ::com::sun::star::sdbc::XRowUpdate >(m_xAggregateAsResultSet, UNO_QUERY);

    Reference< XPropertySet > xSet(m_xAggregateAsResultSet, UNO_QUERY);
    xSet->getPropertyValue(PROPERTY_RESULTSETTYPE) >>= m_nResultSetType;
    xSet->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY) >>= m_nResultSetConcurrency;

    // test for Bookmarks
    if (ResultSetType::FORWARD_ONLY != m_nResultSetType)
    {
        Reference <XPropertySetInfo > xInfo(xSet->getPropertySetInfo());
        if (xInfo->hasPropertyByName(PROPERTY_ISBOOKMARKABLE))
            m_bIsBookmarkable = ::comphelper::getBOOL(xSet->getPropertyValue(PROPERTY_ISBOOKMARKABLE));
    }
}

//--------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    DBG_DTOR(OResultSet, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OResultSet::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           OResultSetBase::getTypes());

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OResultSet::getImplementationId() throw (RuntimeException)
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
Any OResultSet::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OResultSetBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ));

    return aIface;
}

//--------------------------------------------------------------------------
void OResultSet::acquire() throw (RuntimeException)
{
    OResultSetBase::acquire();
}

//--------------------------------------------------------------------------
void OResultSet::release() throw (RuntimeException)
{
    OResultSetBase::release();
}


// OResultSetBase
//------------------------------------------------------------------------------
void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // free the columns
    m_aColumns.disposing();

    // close the pending result set
    Reference< XCloseable > (m_xAggregateAsResultSet, UNO_QUERY)->close();

    m_xAggregateAsResultSet = NULL;
    m_xAggregateAsRow = NULL;
    m_xAggregateAsRowUpdate = NULL;

    m_aStatement = Reference< XInterface >();
}

// XCloseable
//------------------------------------------------------------------------------
void OResultSet::close(void) throw( SQLException, RuntimeException )
{
    {
        MutexGuard aGuard( m_aMutex );
        if (OResultSetBase::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OResultSet::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OResultSet");
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OResultSet::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDB_RESULTSET;
    return aSNS;
}

// com::sun::star::beans::XPropertySet
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OResultSet::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(6)
        DECL_PROP1(CURSORNAME,              ::rtl::OUString,    READONLY);
        DECL_PROP0(FETCHDIRECTION,          sal_Int32);
        DECL_PROP0(FETCHSIZE,               sal_Int32);
        DECL_PROP1_BOOL(ISBOOKMARKABLE,         READONLY);
        DECL_PROP1(RESULTSETCONCURRENCY,    sal_Int32,      READONLY);
        DECL_PROP1(RESULTSETTYPE,           sal_Int32,      READONLY);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException  )
{
    // set it for the driver result set
    Reference< XPropertySet > xSet(m_xAggregateAsResultSet, UNO_QUERY);
    switch (nHandle)
    {
        case PROPERTY_ID_FETCHDIRECTION:
            xSet->setPropertyValue(PROPERTY_FETCHDIRECTION, rValue);
            break;
        case PROPERTY_ID_FETCHSIZE:
            xSet->setPropertyValue(PROPERTY_FETCHSIZE, rValue);
            break;
        default:
            DBG_ERROR("unknown Property");
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void OResultSet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    // done in convert...
}

//------------------------------------------------------------------------------
void OResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        {
            sal_Bool bVal = m_bIsBookmarkable;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        default:
        {
            // get the property name
            ::rtl::OUString aPropName;
            sal_Int16 nAttributes;
            const_cast<OResultSet*>(this)->getInfoHelper().
                fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSHURE(aPropName.getLength(), "property not found?");

            // now read the value
            rValue = Reference< XPropertySet >(m_xAggregateAsResultSet, UNO_QUERY)->getPropertyValue(aPropName);
        }
    }
}

// XWarningsSupplier
//------------------------------------------------------------------------------
Any OResultSet::getWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XWarningsSupplier >(m_xAggregateAsResultSet, UNO_QUERY)->getWarnings();
}

//------------------------------------------------------------------------------
void OResultSet::clearWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XWarningsSupplier >(m_xAggregateAsResultSet, UNO_QUERY)->clearWarnings();
}

// ::com::sun::star::sdbc::XResultSetMetaDataSupplier
//------------------------------------------------------------------------------
Reference< XResultSetMetaData > OResultSet::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XResultSetMetaDataSupplier >(m_xAggregateAsResultSet, UNO_QUERY)->getMetaData();
}

// ::com::sun::star::sdbc::XColumnLocate
//------------------------------------------------------------------------------
sal_Int32 OResultSet::findColumn(const rtl::OUString& columnName) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XColumnLocate >(m_xAggregateAsResultSet, UNO_QUERY)->findColumn(columnName);
}

// ::com::sun::star::sdbcx::XColumnsSupplier
//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > OResultSet::getColumns(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    // do we have to populate the columns
    if (!m_aColumns.isInitialized())
    {
        // get the metadata
        Reference< XResultSetMetaData > xMetaData = Reference< XResultSetMetaDataSupplier >(m_xAggregateAsResultSet, UNO_QUERY)->getMetaData();
        // do we have columns
        try
        {
            for (sal_Int32 i = 0, nCount = xMetaData->getColumnCount(); i < nCount; ++i)
            {
                // retrieve the name of the column
                rtl::OUString aName = xMetaData->getColumnName(i + 1);
                ODataColumn* pColumn = new ODataColumn(xMetaData, m_xAggregateAsRow, m_xAggregateAsRowUpdate, i + 1);
                m_aColumns.append(aName, pColumn);
            }
        }
        catch (SQLException)
        {
        }
        m_aColumns.setInitialized();
    }
    return &m_aColumns;
}

// ::com::sun::star::sdbc::XRow
//------------------------------------------------------------------------------
sal_Bool OResultSet::wasNull(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->wasNull();
}
//------------------------------------------------------------------------------
rtl::OUString OResultSet::getString(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getString(columnIndex);
}
//------------------------------------------------------------------------------
sal_Bool OResultSet::getBoolean(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getBoolean(columnIndex);
}
//------------------------------------------------------------------------------
sal_Int8 OResultSet::getByte(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getByte(columnIndex);
}
//------------------------------------------------------------------------------
sal_Int16 OResultSet::getShort(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getShort(columnIndex);
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getInt(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getInt(columnIndex);
}
//------------------------------------------------------------------------------
sal_Int64 OResultSet::getLong(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getLong(columnIndex);
}
//------------------------------------------------------------------------------
float OResultSet::getFloat(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getFloat(columnIndex);
}
//------------------------------------------------------------------------------
double OResultSet::getDouble(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getDouble(columnIndex);
}
//------------------------------------------------------------------------------
Sequence< sal_Int8 > OResultSet::getBytes(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getBytes(columnIndex);
}
//------------------------------------------------------------------------------
::com::sun::star::util::Date OResultSet::getDate(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getDate(columnIndex);
}
//------------------------------------------------------------------------------
::com::sun::star::util::Time OResultSet::getTime(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getTime(columnIndex);
}
//------------------------------------------------------------------------------
::com::sun::star::util::DateTime OResultSet::getTimestamp(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getTimestamp(columnIndex);
}
//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream >  OResultSet::getBinaryStream(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getBinaryStream(columnIndex);
}
//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream >  OResultSet::getCharacterStream(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getCharacterStream(columnIndex);
}
//------------------------------------------------------------------------------
Any OResultSet::getObject(sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getObject(columnIndex, typeMap);
}
//------------------------------------------------------------------------------
Reference< XRef >  OResultSet::getRef(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getRef(columnIndex);
}
//------------------------------------------------------------------------------
Reference< XBlob >  OResultSet::getBlob(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getBlob(columnIndex);
}
//------------------------------------------------------------------------------
Reference< XClob >  OResultSet::getClob(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getClob(columnIndex);
}
//------------------------------------------------------------------------------
Reference< XArray >  OResultSet::getArray(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsRow->getArray(columnIndex);
}

// ::com::sun::star::sdbc::XRowUpdate
//------------------------------------------------------------------------------
void OResultSet::updateNull(sal_Int32 columnIndex) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateNull(columnIndex);
}

//------------------------------------------------------------------------------
void OResultSet::updateBoolean(sal_Int32 columnIndex, sal_Bool x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateBoolean(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateByte(sal_Int32 columnIndex, sal_Int8 x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateByte(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateShort(sal_Int32 columnIndex, sal_Int16 x) throw( SQLException, RuntimeException )
{
        MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateShort(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateInt(sal_Int32 columnIndex, sal_Int32 x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateInt(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateLong(sal_Int32 columnIndex, sal_Int64 x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateLong(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateFloat(sal_Int32 columnIndex, float x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateFloat(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateDouble(sal_Int32 columnIndex, double x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateDouble(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateString(sal_Int32 columnIndex, const rtl::OUString& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateString(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateBytes(sal_Int32 columnIndex, const Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateBytes(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateDate(sal_Int32 columnIndex, const ::com::sun::star::util::Date& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateDate(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateTime(sal_Int32 columnIndex, const ::com::sun::star::util::Time& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateTime(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateTimestamp(sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateTimestamp(columnIndex, x);
}
//------------------------------------------------------------------------------
void OResultSet::updateBinaryStream(sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream > & x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateBinaryStream(columnIndex, x, length);
}
//------------------------------------------------------------------------------
void OResultSet::updateCharacterStream(sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream > & x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateCharacterStream(columnIndex, x, length);
}
//------------------------------------------------------------------------------
void OResultSet::updateNumericObject(sal_Int32 columnIndex, const Any& x, sal_Int32 scale) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateNumericObject(columnIndex, x, scale);
}

//------------------------------------------------------------------------------
void OResultSet::updateObject(sal_Int32 columnIndex, const Any& x) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    m_xAggregateAsRowUpdate->updateObject(columnIndex, x);
}

// ::com::sun::star::sdbc::XResultSet
//------------------------------------------------------------------------------
sal_Bool OResultSet::next(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->next();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::isBeforeFirst(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->isBeforeFirst();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::isAfterLast(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->isAfterLast();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::isFirst(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->isFirst();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::isLast(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->isLast();
}

//------------------------------------------------------------------------------
void OResultSet::beforeFirst(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsResultSet->beforeFirst();
}

//------------------------------------------------------------------------------
void OResultSet::afterLast(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsResultSet->afterLast();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::first(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->first();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::last(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->last();
}

//------------------------------------------------------------------------------
sal_Int32 OResultSet::getRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->getRow();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::absolute(sal_Int32 row) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->absolute(row);
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::relative(sal_Int32 rows) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->relative(rows);
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::previous(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->previous();
}

//------------------------------------------------------------------------------
void OResultSet::refreshRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsResultSet->refreshRow();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::rowUpdated(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->rowUpdated();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::rowInserted(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->rowInserted();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::rowDeleted(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_xAggregateAsResultSet->rowDeleted();
}

//------------------------------------------------------------------------------
Reference< XInterface > OResultSet::getStatement(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    return m_aStatement.get();
}

// ::com::sun::star::sdbcx::XRowLocate
//------------------------------------------------------------------------------
Any OResultSet::getBookmark(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->getBookmark();
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::moveToBookmark(const Any& bookmark) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->moveToBookmark(bookmark);
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();
    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->moveRelativeToBookmark(bookmark, rows);
}

//------------------------------------------------------------------------------
sal_Int32 OResultSet::compareBookmarks(const Any& first, const Any& second) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->compareBookmarks(first, second);
}

//------------------------------------------------------------------------------
sal_Bool OResultSet::hasOrderedBookmarks(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->hasOrderedBookmarks();
}

//------------------------------------------------------------------------------
sal_Int32 OResultSet::hashBookmark(const Any& bookmark) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (!m_bIsBookmarkable)
        throw FunctionSequenceException(*this);

    return Reference< XRowLocate >(m_xAggregateAsResultSet, UNO_QUERY)->hashBookmark(bookmark);
}

// ::com::sun::star::sdbc::XResultSetUpdate
//------------------------------------------------------------------------------
void OResultSet::insertRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->insertRow();
}

//------------------------------------------------------------------------------
void OResultSet::updateRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->updateRow();
}

//------------------------------------------------------------------------------
void OResultSet::deleteRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->deleteRow();
}

//------------------------------------------------------------------------------
void OResultSet::cancelRowUpdates(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->cancelRowUpdates();
}

//------------------------------------------------------------------------------
void OResultSet::moveToInsertRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->moveToInsertRow();
}

//------------------------------------------------------------------------------
void OResultSet::moveToCurrentRow(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OResultSetBase::rBHelper.bDisposed)
        throw DisposedException();

    if (isReadOnly())
        throw FunctionSequenceException(*this);

    Reference< XResultSetUpdate >(m_xAggregateAsResultSet, UNO_QUERY)->moveToCurrentRow();
}

