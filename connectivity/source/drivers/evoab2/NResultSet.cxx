 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NResultSet.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:52:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_TSORTINDEX_HXX_
#include <TSortIndex.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#include <vector>
#include <algorithm>
#ifndef _CONNECTIVITY_EVOAB_RESULTSET_HXX_
#include "NResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_DATABASEMETADATA_HXX_
#include "NDatabaseMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _SVTOOLS_LOGINDLG_HXX
#include <svtools/logindlg.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::evoab;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace ::com::sun::star::util;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OEvoabResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
::rtl::OUString SAL_CALL OEvoabResultSet::getImplementationName(  ) throw ( RuntimeException)   \
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.evoab.ResultSet");
}
// -------------------------------------------------------------------------
 Sequence< ::rtl::OUString > SAL_CALL OEvoabResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// -------------------------------------------------------------------------
OEvoabResultSet::OEvoabResultSet(OStatement_Base* pStmt,OEvoabConnection *pConnection,OSQLParseTreeIterator&    _aSQLIterator)
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_xMetaData(NULL)
    ,m_pStatement(pStmt)
    ,m_pConnection(pConnection)
    ,m_bWasNull(sal_True)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_pContacts(NULL)
    ,m_nIndex(-1)
    ,m_nLength(0)
    ,m_aSQLIterator(_aSQLIterator)
{
}

// -------------------------------------------------------------------------
OEvoabResultSet::~OEvoabResultSet()
{
}

// -------------------------------------------------------------------------

void OEvoabResultSet::construct(void)
{
}

// -------------------------------------------------------------------------

static ESource *
findSource( const char *name )
{
    ESourceList *pSourceList = NULL;

    g_return_val_if_fail (name != NULL, NULL);

    if (!e_book_get_addressbooks (&pSourceList, NULL))
        pSourceList = NULL;

    for ( GSList *g = e_source_list_peek_groups (pSourceList); g; g = g->next)
    {
        for (GSList *s = e_source_group_peek_sources (E_SOURCE_GROUP (g->data)); s; s = s->next)
        {
            ESource *pSource = E_SOURCE (s->data);
            if (!strcmp (e_source_peek_name (pSource), name))
                return pSource;
        }
    }
    return NULL;
}

static EBook *
openBook( const char *abname )
{
    ESource *pSource = findSource (abname);
    EBook *pBook = NULL;
    if (pSource)
            pBook = e_book_new (pSource, NULL);

    if (pBook && !e_book_open (pBook, TRUE, NULL))
    {
        g_object_unref (G_OBJECT (pBook));
        pBook = NULL;
    }

    return pBook;
}

static bool isLDAP( EBook *pBook )
{
    return pBook && !strncmp( "ldap://", e_book_get_uri( pBook ), 6 );
}

static bool isLocal( EBook *pBook )
{
    return pBook && !strncmp( "file://", e_book_get_uri( pBook ), 6 );
}

static bool isAuthRequired( EBook *pBook )
{
    return e_source_get_property( e_book_get_source( pBook ),
                                  "auth" ) != NULL;
}

static rtl::OString getUserName( EBook *pBook )
{
    rtl::OString aName;
    if( isLDAP( pBook ) )
        aName = e_source_get_property( e_book_get_source( pBook ), "binddn" );
    else
        aName = e_source_get_property( e_book_get_source( pBook ), "user" );
    return aName;
}

static bool
executeQuery (EBook *pBook, EBookQuery *pQuery, GList **ppList,
              rtl::OString &rPassword, GError **pError)
{
    ESource *pSource = e_book_get_source (pBook);
    bool bSuccess = false;
    bool bAuthSuccess = true;

    *ppList = NULL;

    if( isAuthRequired( pBook ) )
    {
        rtl::OString aUser( getUserName( pBook ) );
        const char *pAuth = e_source_get_property( pSource, "auth" );
        bAuthSuccess = e_book_authenticate_user( pBook, aUser, rPassword, pAuth, pError );
    }

    if (bAuthSuccess)
        bSuccess = e_book_get_contacts( pBook, pQuery, ppList, pError );

    return bSuccess;
}

void OEvoabResultSet::construct( EBookQuery *pQuery, rtl::OString aTable, bool bIsWithoutWhere )
{
    EBook *pBook = openBook( aTable );

    if (pBook)
    {
        if( bIsWithoutWhere && !isLocal( pBook ) )
        {
            OSL_TRACE( "large query on non-local book - ignored" );
            m_pContacts = NULL;
            ::dbtools::throwGenericSQLException(
                ::rtl::OUString::createFromAscii(
                "Use SQL Query with \"where clause\" or \"criteria\", to get the results." ), NULL );
        }
        else
        {
            OSL_TRACE( "Query without where ? '%d'", bIsWithoutWhere );
            rtl::OString aPassword = m_pConnection->getPassword();
            executeQuery( pBook, pQuery, &m_pContacts, aPassword, NULL );
            m_pConnection->setPassword( aPassword );
        }
        m_nLength = g_list_length( m_pContacts );
        OSL_TRACE( "Query return %d records", m_nLength );
        m_nIndex = -1;
    }
}

// -------------------------------------------------------------------------
void OEvoabResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_pStatement    = NULL;
    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL OEvoabResultSet::getTypes(  ) throw( RuntimeException)
{
    OTypeCollection aTypes( ::getCppuType( (const  Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    /* Dont know what is this yet */
    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------
// XRow Interface

bool
OEvoabResultSet::getValue( sal_Int32 nColumnNum, GType nType, GValue *pStackValue)
{
    const GParamSpec *pSpec = evoab::getField (nColumnNum - 1);
    EContact *pContact = getCur();

    m_bWasNull = true;
    if (!pSpec || !pContact)
        return false;

    if (G_PARAM_SPEC_VALUE_TYPE (pSpec) != nType)
    {
        g_warning ("Wrong type");
        return false;
    }
    g_value_init (pStackValue, nType);
    g_object_get_property (G_OBJECT (pContact),
                           g_param_spec_get_name ((GParamSpec *) pSpec),
                           pStackValue);
    if (G_VALUE_TYPE (pStackValue) != nType)
    {
        g_warning ("Fetched type mismatch");
        g_value_unset (pStackValue);
        return false;
    }

    m_bWasNull = false;
    return true;
}

/**
 * getString:
 * @nColumnNum: The column index from the table.
 *
 * If the equivalent NResultSetMetaData.cxx marks the columntype of
 * nColumnNum as DataType::VARCHAR this accessor is used.
 */
::rtl::OUString SAL_CALL OEvoabResultSet::getString( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    rtl::OUString aResult;

    GValue aValue = { 0, };
    if (getValue (nColumnNum, G_TYPE_STRING, &aValue))
    {
        const char *pStr = g_value_get_string (&aValue);
        rtl::OString aStr (pStr ? pStr : "");
        aResult = rtl::OStringToOUString( aStr, RTL_TEXTENCODING_UTF8 );
        g_value_unset (&aValue);
    }
    return aResult;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::getBoolean( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    sal_Bool bResult = sal_False;

    GValue aValue = { 0, };
    if (getValue (nColumnNum, G_TYPE_BOOLEAN, &aValue))
    {
        bResult = g_value_get_boolean (&aValue);
        g_value_unset (&aValue);
    }
    return bResult ? sal_True : sal_False;
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL OEvoabResultSet::getLong( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return sal_Int64();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OEvoabResultSet::getArray( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL OEvoabResultSet::getClob( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OEvoabResultSet::getBlob( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL OEvoabResultSet::getRef( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::getObject( sal_Int32 nColumnNum, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return Any();
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL OEvoabResultSet::getShort( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    sal_Int16 nRet=0;
    return nRet ;
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OEvoabResultSet::getTime( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return ::com::sun::star::util::Time();
}
// -------------------------------------------------------------------------
DateTime SAL_CALL OEvoabResultSet::getTimestamp( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return ::com::sun::star::util::DateTime();
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OEvoabResultSet::getBinaryStream( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OEvoabResultSet::getCharacterStream( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return NULL;
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OEvoabResultSet::getByte( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    sal_Int8 nRet=0;
    return nRet;
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OEvoabResultSet::getBytes( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OEvoabResultSet::getDate( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return ::com::sun::star::util::Date();
}
// -------------------------------------------------------------------------
double SAL_CALL OEvoabResultSet::getDouble( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    double nRet=0;
    return nRet;
}
// -------------------------------------------------------------------------
float SAL_CALL OEvoabResultSet::getFloat( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    float nRet(0);
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSet::getInt( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    sal_Int32 nRet=0;
    return nRet;
}
// XRow Interface Ends
// -------------------------------------------------------------------------

// XResultSetMetaDataSupplier Interface
Reference< XResultSetMetaData > SAL_CALL OEvoabResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("OEvoabResultSet::getMetaData");
    if(!m_xMetaData.is())
        m_xMetaData = new OEvoabResultSetMetaData(m_pConnection->getCurrentTableName());
    return m_xMetaData;
}
// XResultSetMetaDataSupplier Interface Ends
// -------------------------------------------------------------------------

// XResultSet Interface
sal_Bool SAL_CALL OEvoabResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if (m_nIndex+1 < m_nLength) {
        ++m_nIndex ;
        return true;
    }
    else
        return false;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex < 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex >= m_nLength;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex == 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex == m_nLength - 1;
}
// -------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = -1;
}
// -------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = m_nLength;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = 0;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = m_nLength - 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if (row < m_nLength) {
        m_nIndex = row;
        return true;
    }
    else
        return false;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if ((m_nIndex+row) < m_nLength) {
        m_nIndex += row;
        return true;
    }
    else
        return false;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(m_nIndex > 0) {
        m_nIndex--;
        return true;
    }
        else
        return false;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OEvoabResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
::com::sun::star::uno::WeakReferenceHelper      aStatement((OWeakObject*)m_pStatement);
    return aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
//XResult Interface ends
// -------------------------------------------------------------------------
// XCancellable

void SAL_CALL OEvoabResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OSL_TRACE("In/Out: OEvoabResultSet::cancel" );

}

//XCloseable
void SAL_CALL OEvoabResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    OSL_TRACE("In/Out: OEvoabResultSet::close" );
    dispose();
}

// XWarningsSupplier
// -------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OEvoabResultSet::clearWarnings" );
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OEvoabResultSet::getWarnings" );
    return Any();
}
// -------------------------------------------------------------------------
//XColumnLocate Interface
sal_Int32 SAL_CALL OEvoabResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -------------------------------------------------------------------------
//XColumnLocate interface ends

// -------------------------------------------------------------------------
IPropertyArrayHelper* OEvoabResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OEvoabResultSet::getInfoHelper()
{
    return *const_cast<OEvoabResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OEvoabResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OEvoabResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OEvoabResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{

    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= (sal_Int32)m_nResultSetConcurrency;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= m_nResultSetType;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= m_nFetchDirection;
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= m_nFetchSize;
            break;
    }

}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabResultSet::release() throw()
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
OEvoabResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
