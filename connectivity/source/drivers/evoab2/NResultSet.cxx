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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "NDatabaseMetaData.hxx"
#include "NConnection.hxx"
#include "NResultSet.hxx"
#include "propertyids.hxx"
#include "resource/evoab2_res.hrc"
#include "TSortIndex.hxx"
#include <algorithm>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/sqlerror.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/string.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/syslocale.hxx>
#include <unotools/intlwrapper.hxx>

#include <cstring>
#include <vector>

namespace connectivity { namespace evoab {

using namespace ::comphelper;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
namespace ErrorCondition = ::com::sun::star::sdb::ErrorCondition;

//------------------------------------------------------------------------------
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
OEvoabResultSet::OEvoabResultSet( OCommonStatement* pStmt, OEvoabConnection *pConnection )
    :OResultSet_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer( OResultSet_BASE::rBHelper )
    ,m_pStatement(pStmt)
    ,m_pConnection(pConnection)
    ,m_xMetaData(NULL)
    ,m_bWasNull(sal_True)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
    ,m_pContacts(NULL)
    ,m_nIndex(-1)
    ,m_nLength(0)
{
    #define REGISTER_PROP( id, member ) \
        registerProperty( \
            OMetaConnection::getPropMap().getNameByIndex( id ), \
            id, \
            PropertyAttribute::READONLY, \
            &member, \
            ::getCppuType( &member ) \
        );

    REGISTER_PROP( PROPERTY_ID_FETCHSIZE, m_nFetchSize );
    REGISTER_PROP( PROPERTY_ID_RESULTSETTYPE, m_nResultSetType );
    REGISTER_PROP( PROPERTY_ID_FETCHDIRECTION, m_nFetchDirection );
    REGISTER_PROP( PROPERTY_ID_RESULTSETCONCURRENCY, m_nResultSetConcurrency );
}

// -------------------------------------------------------------------------
OEvoabResultSet::~OEvoabResultSet()
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
    return pBook && ( !strncmp( "file://", e_book_get_uri( pBook ), 6 ) ||
                      !strncmp( "local:", e_book_get_uri( pBook ), 6 ) );
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

static ::rtl::OUString
valueToOUString( GValue& _rValue )
{
    const char *pStr = g_value_get_string( &_rValue );
    rtl::OString aStr( pStr ? pStr : "" );
    ::rtl::OUString sResult( ::rtl::OStringToOUString( aStr, RTL_TEXTENCODING_UTF8 ) );
    g_value_unset( &_rValue );
    return sResult;
}

static bool
valueToBool( GValue& _rValue )
{
    bool bResult = g_value_get_boolean( &_rValue );
    g_value_unset( &_rValue );
    return bResult;
}

static bool
executeQuery (EBook* pBook, EBookQuery* pQuery, GList **ppList,
              rtl::OString &rPassword, GError **pError)
{
    ESource *pSource = e_book_get_source( pBook );
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

static int
whichAddress(int value)
{
    int fieldEnum;
    switch (value)
    {
        case HOME_ADDR_LINE1:
        case HOME_ADDR_LINE2:
        case HOME_CITY:
        case HOME_STATE:
        case HOME_COUNTRY:
        case HOME_ZIP:
            fieldEnum = e_contact_field_id("address_home");
            break;

        case WORK_ADDR_LINE1:
        case WORK_ADDR_LINE2:
        case WORK_CITY:
        case WORK_STATE:
        case WORK_COUNTRY:
        case WORK_ZIP:
            fieldEnum = e_contact_field_id("address_work");
            break;

        case OTHER_ADDR_LINE1:
        case OTHER_ADDR_LINE2:
        case OTHER_CITY:
        case OTHER_STATE:
        case OTHER_COUNTRY:
        case OTHER_ZIP:
            fieldEnum = e_contact_field_id("address_other");
            break;

            default: fieldEnum = e_contact_field_id("address_home");
      }
    return fieldEnum;
}

/*
* This function decides the default column values based on the first field of EContactAddress.
* The search order is Work->Home->other(defaults).
*/
static EContactAddress *
getDefaultContactAddress( EContact *pContact,int *value )
{
    EContactAddress *ec = (EContactAddress *)e_contact_get(pContact,whichAddress(WORK_ADDR_LINE1));
    if ( ec && (strlen(ec->street)>0) )
    {
        *value= *value +WORK_ADDR_LINE1 -1;
        return ec;
    }
    else
        {
            ec = (EContactAddress *)e_contact_get(pContact,whichAddress(HOME_ADDR_LINE1));
            if ( ec && (strlen(ec->street)>0) )
            {
                *value=*value+HOME_ADDR_LINE1-1;
                return ec;
            }
        }

    *value=*value+OTHER_ADDR_LINE1-1;
    return (EContactAddress *)e_contact_get(pContact,whichAddress(OTHER_ADDR_LINE1));
}

static EContactAddress*
getContactAddress( EContact *pContact, int * address_enum )
{
    EContactAddress *ec = NULL;
    switch (*address_enum) {

        case DEFAULT_ADDR_LINE1:
        case DEFAULT_ADDR_LINE2:
        case DEFAULT_CITY:
        case DEFAULT_STATE:
        case DEFAULT_COUNTRY:
        case DEFAULT_ZIP:
            ec = getDefaultContactAddress(pContact,address_enum);break;
           default:
            ec = (EContactAddress *)e_contact_get(pContact,whichAddress(*address_enum));
    }
    return ec;
}

static bool
handleSplitAddress( EContact *pContact,GValue *pStackValue, int value )
{
    EContactAddress *ec = getContactAddress(pContact,&value) ;

    if (ec==NULL)
        return true;

    switch (value) {
        case WORK_ADDR_LINE1:
            g_value_set_string(pStackValue,ec->street ); break;
        case WORK_ADDR_LINE2:
            g_value_set_string(pStackValue,ec->po ); break;
        case WORK_CITY:
            g_value_set_string(pStackValue,ec->locality ); break;
        case WORK_STATE:
             g_value_set_string(pStackValue,ec->region ); break;
        case WORK_COUNTRY:
            g_value_set_string(pStackValue,ec->country ); break;
        case WORK_ZIP:
            g_value_set_string(pStackValue,ec->code ); break;

        case HOME_ADDR_LINE1:
            g_value_set_string(pStackValue,ec->street ); break;
        case HOME_ADDR_LINE2:
            g_value_set_string(pStackValue,ec->po ); break;
        case HOME_CITY:
            g_value_set_string(pStackValue,ec->locality ); break;
        case HOME_STATE:
            g_value_set_string(pStackValue,ec->region ); break;
        case HOME_COUNTRY:
            g_value_set_string(pStackValue,ec->country ); break;
        case HOME_ZIP:
            g_value_set_string(pStackValue,ec->code ); break;

        case OTHER_ADDR_LINE1:
            g_value_set_string(pStackValue,ec->street ); break;
        case OTHER_ADDR_LINE2:
            g_value_set_string(pStackValue,ec->po ); break;
        case OTHER_CITY:
            g_value_set_string(pStackValue,ec->locality ); break;
        case OTHER_STATE:
            g_value_set_string(pStackValue,ec->region ); break;
        case OTHER_COUNTRY:
            g_value_set_string(pStackValue,ec->country ); break;
        case OTHER_ZIP:
            g_value_set_string(pStackValue,ec->code ); break;

    }

    return false;
}
static bool
getValue( EContact* pContact, sal_Int32 nColumnNum, GType nType, GValue* pStackValue, bool& _out_rWasNull )
{
    const ColumnProperty * pSpecs = evoab::getField( nColumnNum );
    if ( !pSpecs )
        return false;

    GParamSpec* pSpec = pSpecs->pField;
    gboolean bIsSplittedColumn = pSpecs->bIsSplittedValue;

    _out_rWasNull = true;
    if ( !pSpec || !pContact)
        return false;

    if ( G_PARAM_SPEC_VALUE_TYPE (pSpec) != nType )
    {

        OSL_TRACE( "Wrong type (0x%x) (0x%x) '%s'",
                   (int)G_PARAM_SPEC_VALUE_TYPE (pSpec), (int) nType,
                   pSpec->name ? pSpec->name : "<noname>");
        return false;
    }

    g_value_init( pStackValue, nType );
    if ( bIsSplittedColumn )
    {
        const SplitEvoColumns* evo_addr( get_evo_addr() );
        for (int i=0;i<OTHER_ZIP;i++)
        {
            if (0 == strcmp (g_param_spec_get_name ((GParamSpec *)pSpec), evo_addr[i].pColumnName))
            {
                _out_rWasNull = handleSplitAddress( pContact, pStackValue, evo_addr[i].value );
                return true;
            }
        }
    }
    else
    {
        g_object_get_property( G_OBJECT (pContact),
                               g_param_spec_get_name ((GParamSpec *) pSpec),
                               pStackValue );
        if ( G_VALUE_TYPE( pStackValue ) != nType )
        {
            OSL_TRACE( "Fetched type mismatch" );
            g_value_unset( pStackValue );
            return false;
        }
    }
    _out_rWasNull = false;
    return true;
}

namespace
{
    struct ComparisonData
    {
        const SortDescriptor&   rSortOrder;
        IntlWrapper             aIntlWrapper;

        ComparisonData( const SortDescriptor& _rSortOrder, const Reference< XMultiServiceFactory >& _rxFactory )
            :rSortOrder( _rSortOrder )
            ,aIntlWrapper( _rxFactory, SvtSysLocale().GetLocaleData().getLocale() )
        {
        }
    };
}

extern "C"
int CompareContacts( gconstpointer _lhs, gconstpointer _rhs, gpointer _userData )
{
    EContact* lhs = static_cast< EContact* >( const_cast< gpointer >( _lhs ) );
    EContact* rhs = static_cast< EContact* >( const_cast< gpointer >( _rhs ) );

    GValue aLhsValue = { 0, { { 0 } } };
    GValue aRhsValue = { 0, { { 0 } } };
    bool bLhsNull = true;
    bool bRhsNull = true;

    ::rtl::OUString sLhs, sRhs;
    bool bLhs(false), bRhs(false);

    const ComparisonData& rCompData = *static_cast< const ComparisonData* >( _userData );
    for (   SortDescriptor::const_iterator sortCol = rCompData.rSortOrder.begin();
            sortCol != rCompData.rSortOrder.end();
            ++sortCol
        )
    {
        sal_Int32 nField = sortCol->nField;
        GType eFieldType = evoab::getGFieldType( nField );

        bool success =  getValue( lhs, nField, eFieldType, &aLhsValue, bLhsNull )
                    &&  getValue( rhs, nField, eFieldType, &aRhsValue, bRhsNull );
        OSL_ENSURE( success, "CompareContacts: could not retrieve both values!" );
        if ( !success )
            return 0;

        if ( bLhsNull && !bRhsNull )
            return -1;
        if ( !bLhsNull && bRhsNull )
            return 1;
        if ( bLhsNull && bRhsNull )
            continue;

        if ( eFieldType == G_TYPE_STRING )
        {
            sLhs = valueToOUString( aLhsValue );
            sRhs = valueToOUString( aRhsValue );
            sal_Int32 nCompResult = rCompData.aIntlWrapper.getCaseCollator()->compareString( sLhs, sRhs );
            if ( nCompResult != 0 )
                return nCompResult;
            continue;
        }

        bLhs = valueToBool( aLhsValue );
        bRhs = valueToBool( aRhsValue );
        if ( bLhs && !bRhs )
            return -1;
        if ( !bLhs && bRhs )
            return 1;
        continue;
    }

    return 0;
}

static GList*
sortContacts( GList* _pContactList, const ComparisonData& _rCompData )
{
    OSL_ENSURE( !_rCompData.rSortOrder.empty(), "sortContacts: no need to call this without any sort order!" );
    ENSURE_OR_THROW( _rCompData.aIntlWrapper.getCaseCollator(), "no collator for comparing strings" );

    return g_list_sort_with_data( _pContactList, &CompareContacts, const_cast< gpointer >( static_cast< gconstpointer >( &_rCompData ) ) );
}

// -------------------------------------------------------------------------
void OEvoabResultSet::construct( const QueryData& _rData )
{
    ENSURE_OR_THROW( _rData.getQuery(), "internal error: no EBookQuery" );

    EBook *pBook = openBook( ::rtl::OUStringToOString( _rData.sTable, RTL_TEXTENCODING_UTF8 ) );
    if ( !pBook )
        m_pConnection->throwGenericSQLException( STR_CANNOT_OPEN_BOOK, *this );

    g_list_free(m_pContacts);
    m_pContacts = NULL;
    bool bExecuteQuery = true;
    switch ( _rData.eFilterType )
    {
        case eFilterNone:
            if ( !isLocal( pBook ) )
            {
                SQLError aErrorFactory( m_pConnection->getDriver().getMSFactory() );
                SQLException aAsException = aErrorFactory.getSQLException( ErrorCondition::DATA_CANNOT_SELECT_UNFILTERED, *this );
                m_aWarnings.appendWarning( SQLWarning(
                    aAsException.Message,
                    aAsException.Context,
                    aAsException.SQLState,
                    aAsException.ErrorCode,
                    aAsException.NextException
                ) );
                bExecuteQuery = false;
            }
            break;
        case eFilterAlwaysFalse:
            bExecuteQuery = false;
            break;
        case eFilterOther:
            bExecuteQuery = true;
            break;
    }
    if ( bExecuteQuery )
    {
        rtl::OString aPassword = m_pConnection->getPassword();
        executeQuery( pBook, _rData.getQuery(), &m_pContacts, aPassword, NULL );
        m_pConnection->setPassword( aPassword );

        if ( m_pContacts && !_rData.aSortOrder.empty() )
        {
            ComparisonData aCompData( _rData.aSortOrder, getConnection()->getDriver().getMSFactory() );
            m_pContacts = sortContacts( m_pContacts, aCompData );
        }
    }
    m_nLength = g_list_length( m_pContacts );
    OSL_TRACE( "Query return %d records", m_nLength );
    m_nIndex = -1;

    // create our meta data (need the EBookQuery for this)
    OEvoabResultSetMetaData* pMeta = new OEvoabResultSetMetaData( _rData.sTable );
    m_xMetaData = pMeta;

    pMeta->setEvoabFields( _rData.xSelectColumns );
}

// -------------------------------------------------------------------------
void OEvoabResultSet::disposing(void)
{
    ::comphelper::OPropertyContainer::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    g_list_free(m_pContacts);
    m_pContacts = NULL;
    m_pStatement = NULL;
m_xMetaData.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::comphelper::OPropertyContainer::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OEvoabResultSet::getTypes(  ) throw( RuntimeException)
{
    return ::comphelper::concatSequences(
        OResultSet_BASE::getTypes(),
        ::comphelper::OPropertyContainer::getTypes()
    );
}

// -------------------------------------------------------------------------
// XRow Interface

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
    if ( m_xMetaData.is())
    {
        OEvoabResultSetMetaData *pMeta = (OEvoabResultSetMetaData *) m_xMetaData.get();
        sal_Int32 nFieldNumber = pMeta->fieldAtColumn(nColumnNum);
        GValue aValue = { 0, { { 0 } } };
        if ( getValue( getCur(), nFieldNumber, G_TYPE_STRING, &aValue, m_bWasNull ) )
            aResult = valueToOUString( aValue );
    }
    return aResult;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSet::getBoolean( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    sal_Bool bResult = sal_False;

    if ( m_xMetaData.is())
    {
        OEvoabResultSetMetaData *pMeta = (OEvoabResultSetMetaData *) m_xMetaData.get();
        sal_Int32 nFieldNumber = pMeta->fieldAtColumn(nColumnNum);
        GValue aValue = { 0, { { 0 } } };
        if ( getValue( getCur(), nFieldNumber, G_TYPE_BOOLEAN, &aValue, m_bWasNull ) )
            bResult = valueToBool( aValue );
    }
    return bResult ? sal_True : sal_False;
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL OEvoabResultSet::getLong( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getLong", *this );
    return sal_Int64();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OEvoabResultSet::getArray( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getArray", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL OEvoabResultSet::getClob( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getClob", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OEvoabResultSet::getBlob( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getBlob", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL OEvoabResultSet::getRef( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getRef", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::getObject( sal_Int32 /*nColumnNum*/, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getObject", *this );
    return Any();
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL OEvoabResultSet::getShort( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getShort", *this );
    return 0;
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OEvoabResultSet::getTime( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getTime", *this );
    return ::com::sun::star::util::Time();
}
// -------------------------------------------------------------------------
util::DateTime SAL_CALL OEvoabResultSet::getTimestamp( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getTimestamp", *this );
    return ::com::sun::star::util::DateTime();
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OEvoabResultSet::getBinaryStream( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getBinaryStream", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OEvoabResultSet::getCharacterStream( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getCharacterStream", *this );
    return NULL;
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OEvoabResultSet::getByte( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getByte", *this );
    return 0;
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OEvoabResultSet::getBytes( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getBytes", *this );
    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OEvoabResultSet::getDate( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getDate", *this );
    return ::com::sun::star::util::Date();
}
// -------------------------------------------------------------------------
double SAL_CALL OEvoabResultSet::getDouble( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getDouble", *this );
    return 0;
}
// -------------------------------------------------------------------------
float SAL_CALL OEvoabResultSet::getFloat( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getFloat", *this );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSet::getInt( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getInt", *this );
    return 0;
}
// XRow Interface Ends
// -------------------------------------------------------------------------

// XResultSetMetaDataSupplier Interface
Reference< XResultSetMetaData > SAL_CALL OEvoabResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // the meta data should have been created at construction time
    ENSURE_OR_THROW( m_xMetaData.is(), "internal error: no meta data" );
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
    return true;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = m_nLength - 1;
    return true;
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
    m_aWarnings.clearWarnings();
}
// -------------------------------------------------------------------------
Any SAL_CALL OEvoabResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OEvoabResultSet::getWarnings" );
    return m_aWarnings.getWarnings();
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
::cppu::IPropertyArrayHelper* OEvoabResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OEvoabResultSet::getInfoHelper()
{
    return *const_cast<OEvoabResultSet*>(this)->getArrayHelper();
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

} } // connectivity::evoab

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
