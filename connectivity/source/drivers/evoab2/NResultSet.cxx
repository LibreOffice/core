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


#include "NDatabaseMetaData.hxx"
#include "NConnection.hxx"
#include "NResultSet.hxx"
#include <propertyids.hxx>
#include <strings.hrc>
#include <TSortIndex.hxx>
#include <algorithm>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>

#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/sqlerror.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/syslocale.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/collatorwrapper.hxx>

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


OUString SAL_CALL OEvoabResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.evoab.ResultSet");
}

 Sequence< OUString > SAL_CALL OEvoabResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupported { "com.sun.star.sdbc.ResultSet" };
    return aSupported;
}

sal_Bool SAL_CALL OEvoabResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

struct ComparisonData
{
    const SortDescriptor&   rSortOrder;
    IntlWrapper             aIntlWrapper;

    ComparisonData(const SortDescriptor& _rSortOrder)
        : rSortOrder(_rSortOrder)
        , aIntlWrapper(SvtSysLocale().GetUILanguageTag())
    {
    }
};

static OUString
valueToOUString( GValue& _rValue )
{
    const char *pStr = g_value_get_string( &_rValue );
    OString aStr( pStr ? pStr : "" );
    OUString sResult( OStringToOUString( aStr, RTL_TEXTENCODING_UTF8 ) );
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
    EContactAddress *ec = static_cast<EContactAddress *>(e_contact_get(pContact,whichAddress(WORK_ADDR_LINE1)));
    if ( ec && (ec->street[0]!='\0') )
    {
        *value= *value +WORK_ADDR_LINE1 -1;
        return ec;
    }
    else
        {
            ec = static_cast<EContactAddress *>(e_contact_get(pContact,whichAddress(HOME_ADDR_LINE1)));
            if ( ec && (ec->street[0]!='\0') )
            {
                *value=*value+HOME_ADDR_LINE1-1;
                return ec;
            }
        }

    *value=*value+OTHER_ADDR_LINE1-1;
    return static_cast<EContactAddress *>(e_contact_get(pContact,whichAddress(OTHER_ADDR_LINE1)));
}

static EContactAddress*
getContactAddress( EContact *pContact, int * address_enum )
{
    EContactAddress *ec = nullptr;
    switch (*address_enum) {

        case DEFAULT_ADDR_LINE1:
        case DEFAULT_ADDR_LINE2:
        case DEFAULT_CITY:
        case DEFAULT_STATE:
        case DEFAULT_COUNTRY:
        case DEFAULT_ZIP:
            ec = getDefaultContactAddress(pContact,address_enum);break;
           default:
            ec = static_cast<EContactAddress *>(e_contact_get(pContact,whichAddress(*address_enum)));
    }
    return ec;
}

static bool
handleSplitAddress( EContact *pContact,GValue *pStackValue, int value )
{
    EContactAddress *ec = getContactAddress(pContact,&value) ;

    if (ec==nullptr)
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
        SAL_WARN("connectivity.evoab2", "Wrong type (0x" << std::hex << static_cast<int>(G_PARAM_SPEC_VALUE_TYPE(pSpec)) << ") (0x"
                  << std::hex << static_cast<int>(nType) << ") " <<  (pSpec->name ? pSpec->name : "<noname>"));
        return false;
    }

    g_value_init( pStackValue, nType );
    if ( bIsSplittedColumn )
    {
        const SplitEvoColumns* evo_addr( get_evo_addr() );
        for (int i=0;i<OTHER_ZIP;i++)
        {
            if (0 == strcmp (g_param_spec_get_name (pSpec), evo_addr[i].pColumnName))
            {
                _out_rWasNull = handleSplitAddress( pContact, pStackValue, evo_addr[i].value );
                return true;
            }
        }
    }
    else
    {
        g_object_get_property( G_OBJECT (pContact),
                               g_param_spec_get_name (pSpec),
                               pStackValue );
        if ( G_VALUE_TYPE( pStackValue ) != nType )
        {
            SAL_WARN("connectivity.evoab2", "Fetched type mismatch" );
            g_value_unset( pStackValue );
            return false;
        }
    }
    _out_rWasNull = false;
    return true;
}

extern "C" {

static int CompareContacts( gconstpointer _lhs, gconstpointer _rhs, gpointer _userData )
{
    EContact* lhs = const_cast< gpointer >( _lhs );
    EContact* rhs = const_cast< gpointer >( _rhs );

    GValue aLhsValue = { 0, { { 0 } } };
    GValue aRhsValue = { 0, { { 0 } } };
    bool bLhsNull = true;
    bool bRhsNull = true;

    OUString sLhs, sRhs;
    bool bLhs(false), bRhs(false);

    const ComparisonData& rCompData = *static_cast< const ComparisonData* >( _userData );
    for ( const auto& sortCol : rCompData.rSortOrder )
    {
        sal_Int32 nField = sortCol.nField;
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

}

OString OEvoabVersionHelper::getUserName( EBook *pBook )
{
    OString aName;
    if( isLDAP( pBook ) )
        aName = e_source_get_property( e_book_get_source( pBook ), "binddn" );
    else
        aName = e_source_get_property( e_book_get_source( pBook ), "user" );
    return aName;
}

namespace {

bool isBookBackend( EBookClient *pBook, const char *backendname)
{
    if (!pBook)
        return false;
    ESource *pSource = e_client_get_source (reinterpret_cast<EClient *>(pBook));
    return isSourceBackend(pSource, backendname);
}

}

class OEvoabVersion36Helper : public OEvoabVersionHelper
{
private:
    GSList   *m_pContacts;
public:
    OEvoabVersion36Helper()
        : m_pContacts(nullptr)
    {
    }

    virtual ~OEvoabVersion36Helper() override
    {
        freeContacts();
    }

    virtual EBook* openBook(const char *abname) override
    {
        //It would be better if here we had id to begin with, see
        //NDatabaseMetaData.cxx
        const char *id = nullptr;
        GList *pSources = e_source_registry_list_sources(get_e_source_registry(), E_SOURCE_EXTENSION_ADDRESS_BOOK);
        for (GList* liter = pSources; liter; liter = liter->next)
        {
            ESource *pSource = E_SOURCE (liter->data);

            if (strcmp(abname, e_source_get_display_name( pSource )) == 0)
            {
                id = e_source_get_uid( pSource );
                break;
            }
        }
        g_list_foreach (pSources, reinterpret_cast<GFunc>(g_object_unref), nullptr);
        g_list_free (pSources);
        if (!id)
            return nullptr;

        ESource *pSource = e_source_registry_ref_source(get_e_source_registry(), id);
        EBookClient *pBook = pSource ? createClient (pSource) : nullptr;
        if (pBook && !e_client_open_sync (pBook, TRUE, nullptr, nullptr))
        {
            g_object_unref (G_OBJECT (pBook));
            pBook = nullptr;
        }
        if (pSource)
            g_object_unref (pSource);
        return pBook;
    }

    virtual bool isLDAP( EBook *pBook ) override
    {
        return isBookBackend(pBook, "ldap");
    }

    virtual bool isLocal( EBook *pBook ) override
    {
        return isBookBackend(pBook, "local");
    }

    virtual void freeContacts() override
    {
        e_client_util_free_object_slist(m_pContacts);
        m_pContacts = nullptr;
    }

    virtual void executeQuery (EBook* pBook, EBookQuery* pQuery, OString &/*rPassword*/) override
    {
        freeContacts();
        char *sexp = e_book_query_to_string( pQuery );
        e_book_client_get_contacts_sync( pBook, sexp, &m_pContacts, nullptr, nullptr );
        g_free (sexp);
    }

    virtual EContact *getContact(sal_Int32 nIndex) override
    {
        gpointer pData = g_slist_nth_data (m_pContacts, nIndex);
        return pData ? E_CONTACT (pData) : nullptr;
    }

    virtual sal_Int32 getNumContacts() override
    {
        return g_slist_length( m_pContacts );
    }

    virtual bool hasContacts() override
    {
        return m_pContacts != nullptr;
    }

    virtual void sortContacts( const ComparisonData& _rCompData ) override
    {
        OSL_ENSURE( !_rCompData.rSortOrder.empty(), "sortContacts: no need to call this without any sort order!" );
        ENSURE_OR_THROW( _rCompData.aIntlWrapper.getCaseCollator(), "no collator for comparing strings" );

        m_pContacts = g_slist_sort_with_data( m_pContacts, &CompareContacts,
            const_cast< gpointer >( static_cast< gconstpointer >( &_rCompData ) ) );
    }

protected:
    virtual EBookClient * createClient( ESource *pSource )
    {
        return e_book_client_new (pSource, nullptr);
    }
};

class OEvoabVersion38Helper : public OEvoabVersion36Helper
{
protected:
    virtual EBookClient * createClient( ESource *pSource ) override
    {
        return e_book_client_connect_direct_sync (get_e_source_registry (), pSource, nullptr, nullptr);
    }
};

namespace {

ESource * findSource( const char *id )
{
    ESourceList *pSourceList = nullptr;

    g_return_val_if_fail (id != nullptr, nullptr);

    if (!e_book_get_addressbooks (&pSourceList, nullptr))
        pSourceList = nullptr;

    for ( GSList *g = e_source_list_peek_groups (pSourceList); g; g = g->next)
    {
        for (GSList *s = e_source_group_peek_sources (E_SOURCE_GROUP (g->data)); s; s = s->next)
        {
            ESource *pSource = E_SOURCE (s->data);
            if (!strcmp (e_source_peek_name (pSource), id))
                return pSource;
        }
    }
    return nullptr;
}

bool isAuthRequired( EBook *pBook )
{
    return e_source_get_property( e_book_get_source( pBook ),
                                  "auth" ) != nullptr;
}

}

class OEvoabVersion35Helper : public OEvoabVersionHelper
{
private:
    GList *m_pContacts;

public:
    OEvoabVersion35Helper()
        : m_pContacts(nullptr)
    {
    }

    virtual ~OEvoabVersion35Helper() override
    {
        freeContacts();
    }

    virtual EBook* openBook(const char *abname) override
    {
        ESource *pSource = findSource (abname);
        EBook *pBook = pSource ? e_book_new (pSource, nullptr) : nullptr;
        if (pBook && !e_book_open (pBook, TRUE, nullptr))
        {
            g_object_unref (G_OBJECT (pBook));
            pBook = nullptr;
        }
        return pBook;
    }

    virtual bool isLDAP( EBook *pBook ) override
    {
        return pBook && !strncmp( "ldap://", e_book_get_uri( pBook ), 6 );
    }

    virtual bool isLocal( EBook *pBook ) override
    {
        return pBook && ( !strncmp( "file://", e_book_get_uri( pBook ), 6 ) ||
                          !strncmp( "local:", e_book_get_uri( pBook ), 6 ) );
    }

    virtual void freeContacts() override
    {
        g_list_free(m_pContacts);
        m_pContacts = nullptr;
    }

    virtual void executeQuery (EBook* pBook, EBookQuery* pQuery, OString &rPassword) override
    {
        freeContacts();

        ESource *pSource = e_book_get_source( pBook );
        bool bAuthSuccess = true;

        if( isAuthRequired( pBook ) )
        {
            OString aUser( getUserName( pBook ) );
            const char *pAuth = e_source_get_property( pSource, "auth" );
            bAuthSuccess = e_book_authenticate_user( pBook, aUser.getStr(), rPassword.getStr(), pAuth, nullptr );
        }

        if (bAuthSuccess)
            e_book_get_contacts( pBook, pQuery, &m_pContacts, nullptr );
    }

    virtual EContact *getContact(sal_Int32 nIndex) override
    {
        gpointer pData = g_list_nth_data (m_pContacts, nIndex);
        return pData ? E_CONTACT (pData) : nullptr;
    }

    virtual sal_Int32 getNumContacts() override
    {
        return g_list_length( m_pContacts );
    }

    virtual bool hasContacts() override
    {
        return m_pContacts != nullptr;
    }

    virtual void sortContacts( const ComparisonData& _rCompData ) override
    {
        OSL_ENSURE( !_rCompData.rSortOrder.empty(), "sortContacts: no need to call this without any sort order!" );
        ENSURE_OR_THROW( _rCompData.aIntlWrapper.getCaseCollator(), "no collator for comparing strings" );

        m_pContacts = g_list_sort_with_data( m_pContacts, &CompareContacts,
            const_cast< gpointer >( static_cast< gconstpointer >( &_rCompData ) ) );
    }
};

OEvoabResultSet::OEvoabResultSet( OCommonStatement* pStmt, OEvoabConnection *pConnection )
    :OResultSet_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer( OResultSet_BASE::rBHelper )
    ,m_pStatement(pStmt)
    ,m_pConnection(pConnection)
    ,m_bWasNull(true)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
    ,m_nIndex(-1)
    ,m_nLength(0)
{
    if (eds_check_version( 3, 7, 6 ) == nullptr)
        m_pVersionHelper  = std::make_unique<OEvoabVersion38Helper>();
    else if (eds_check_version( 3, 6, 0 ) == nullptr)
        m_pVersionHelper  = std::make_unique<OEvoabVersion36Helper>();
    else
        m_pVersionHelper  = std::make_unique<OEvoabVersion35Helper>();

    #define REGISTER_PROP( id, member ) \
        registerProperty( \
            OMetaConnection::getPropMap().getNameByIndex( id ), \
            id, \
            PropertyAttribute::READONLY, \
            &member, \
            cppu::UnoType<decltype(member)>::get() \
        );

    REGISTER_PROP( PROPERTY_ID_FETCHSIZE, m_nFetchSize );
    REGISTER_PROP( PROPERTY_ID_RESULTSETTYPE, m_nResultSetType );
    REGISTER_PROP( PROPERTY_ID_FETCHDIRECTION, m_nFetchDirection );
    REGISTER_PROP( PROPERTY_ID_RESULTSETCONCURRENCY, m_nResultSetConcurrency );
}

OEvoabResultSet::~OEvoabResultSet()
{}

void OEvoabResultSet::construct( const QueryData& _rData )
{
    ENSURE_OR_THROW( _rData.getQuery(), "internal error: no EBookQuery" );

    EBook *pBook = m_pVersionHelper->openBook(OUStringToOString(_rData.sTable, RTL_TEXTENCODING_UTF8).getStr());
    if ( !pBook )
        m_pConnection->throwGenericSQLException( STR_CANNOT_OPEN_BOOK, *this );

    m_pVersionHelper->freeContacts();
    bool bExecuteQuery = true;
    switch ( _rData.eFilterType )
    {
        case eFilterNone:
            if ( !m_pVersionHelper->isLocal( pBook ) )
            {
                SQLError aErrorFactory;
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
        OString aPassword = m_pConnection->getPassword();
        m_pVersionHelper->executeQuery(pBook, _rData.getQuery(), aPassword);
        m_pConnection->setPassword( aPassword );

        if ( m_pVersionHelper->hasContacts() && !_rData.aSortOrder.empty() )
        {
            ComparisonData aCompData(_rData.aSortOrder);
            m_pVersionHelper->sortContacts(aCompData);
        }
    }
    m_nLength = m_pVersionHelper->getNumContacts();
    SAL_INFO("connectivity.evoab2", "Query return " << m_nLength << " records");
    m_nIndex = -1;

    // create our meta data (need the EBookQuery for this)
    m_xMetaData = new OEvoabResultSetMetaData( _rData.sTable );

    m_xMetaData->setEvoabFields( _rData.xSelectColumns );
}


void OEvoabResultSet::disposing()
{
    ::comphelper::OPropertyContainer::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_pVersionHelper.reset();
    m_pStatement = nullptr;
    m_xMetaData.clear();
}

Any SAL_CALL OEvoabResultSet::queryInterface( const Type & rType )
{
    Any aRet = ::comphelper::OPropertyContainer::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL OEvoabResultSet::getTypes(  )
{
    return ::comphelper::concatSequences(
        OResultSet_BASE::getTypes(),
        getBaseTypes()
    );
}


// XRow Interface

/**
 * getString:
 * @nColumnNum: The column index from the table.
 *
 * If the equivalent NResultSetMetaData.cxx marks the columntype of
 * nColumnNum as DataType::VARCHAR this accessor is used.
 */
OUString SAL_CALL OEvoabResultSet::getString( sal_Int32 nColumnNum )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OUString aResult;
    if ( m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(nColumnNum);
        GValue aValue = { 0, { { 0 } } };
        if ( getValue( getCur(), nFieldNumber, G_TYPE_STRING, &aValue, m_bWasNull ) )
            aResult = valueToOUString( aValue );
    }
    return aResult;
}

sal_Bool SAL_CALL OEvoabResultSet::getBoolean( sal_Int32 nColumnNum )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    bool bResult = false;

    if ( m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(nColumnNum);
        GValue aValue = { 0, { { 0 } } };
        if ( getValue( getCur(), nFieldNumber, G_TYPE_BOOLEAN, &aValue, m_bWasNull ) )
            bResult = valueToBool( aValue );
    }
    return bResult;
}

sal_Int64 SAL_CALL OEvoabResultSet::getLong( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getLong", *this );
    return sal_Int64();
}

Reference< XArray > SAL_CALL OEvoabResultSet::getArray( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getArray", *this );
    return nullptr;
}

Reference< XClob > SAL_CALL OEvoabResultSet::getClob( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getClob", *this );
    return nullptr;
}

Reference< XBlob > SAL_CALL OEvoabResultSet::getBlob( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBlob", *this );
    return nullptr;
}

Reference< XRef > SAL_CALL OEvoabResultSet::getRef( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getRef", *this );
    return nullptr;
}

Any SAL_CALL OEvoabResultSet::getObject( sal_Int32 /*nColumnNum*/, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getObject", *this );
    return Any();
}

sal_Int16 SAL_CALL OEvoabResultSet::getShort( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getShort", *this );
    return 0;
}

css::util::Time SAL_CALL OEvoabResultSet::getTime( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getTime", *this );
    return css::util::Time();
}

util::DateTime SAL_CALL OEvoabResultSet::getTimestamp( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getTimestamp", *this );
    return css::util::DateTime();
}

Reference< XInputStream > SAL_CALL OEvoabResultSet::getBinaryStream( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBinaryStream", *this );
    return nullptr;
}

Reference< XInputStream > SAL_CALL OEvoabResultSet::getCharacterStream( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getCharacterStream", *this );
    return nullptr;
}

sal_Int8 SAL_CALL OEvoabResultSet::getByte( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getByte", *this );
    return 0;
}

Sequence< sal_Int8 > SAL_CALL OEvoabResultSet::getBytes( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBytes", *this );
    return Sequence< sal_Int8 >();
}

css::util::Date SAL_CALL OEvoabResultSet::getDate( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getDate", *this );
    return css::util::Date();
}

double SAL_CALL OEvoabResultSet::getDouble( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getDouble", *this );
    return 0;
}

float SAL_CALL OEvoabResultSet::getFloat( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getFloat", *this );
    return 0;
}

sal_Int32 SAL_CALL OEvoabResultSet::getInt( sal_Int32 /*nColumnNum*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getInt", *this );
    return 0;
}
// XRow Interface Ends


// XResultSetMetaDataSupplier Interface
Reference< XResultSetMetaData > SAL_CALL OEvoabResultSet::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // the meta data should have been created at construction time
    ENSURE_OR_THROW( m_xMetaData.is(), "internal error: no meta data" );
    return m_xMetaData.get();
}
// XResultSetMetaDataSupplier Interface Ends


// XResultSet Interface
sal_Bool SAL_CALL OEvoabResultSet::next(  )
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

sal_Bool SAL_CALL OEvoabResultSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

sal_Bool SAL_CALL OEvoabResultSet::isBeforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex < 0;
}

sal_Int32 SAL_CALL OEvoabResultSet::getRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex;
}

sal_Bool SAL_CALL OEvoabResultSet::isAfterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex >= m_nLength;
}

sal_Bool SAL_CALL OEvoabResultSet::isFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex == 0;
}

sal_Bool SAL_CALL OEvoabResultSet::isLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nIndex == m_nLength - 1;
}

void SAL_CALL OEvoabResultSet::beforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = -1;
}

void SAL_CALL OEvoabResultSet::afterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = m_nLength;
}


sal_Bool SAL_CALL OEvoabResultSet::first(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = 0;
    return true;
}


sal_Bool SAL_CALL OEvoabResultSet::last(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_nIndex = m_nLength - 1;
    return true;
}

sal_Bool SAL_CALL OEvoabResultSet::absolute( sal_Int32 row )
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

sal_Bool SAL_CALL OEvoabResultSet::relative( sal_Int32 row )
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

sal_Bool SAL_CALL OEvoabResultSet::previous(  )
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

Reference< XInterface > SAL_CALL OEvoabResultSet::getStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    css::uno::WeakReferenceHelper      aStatement(static_cast<OWeakObject*>(m_pStatement));
    return aStatement.get();
}


sal_Bool SAL_CALL OEvoabResultSet::rowDeleted(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OEvoabResultSet::rowInserted(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OEvoabResultSet::rowUpdated(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

void SAL_CALL OEvoabResultSet::refreshRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
//XResult Interface ends

// XCancellable

void SAL_CALL OEvoabResultSet::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

//XCloseable
void SAL_CALL OEvoabResultSet::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}

// XWarningsSupplier

void SAL_CALL OEvoabResultSet::clearWarnings(  )
{
    m_aWarnings.clearWarnings();
}

Any SAL_CALL OEvoabResultSet::getWarnings(  )
{
    return m_aWarnings.getWarnings();
}

//XColumnLocate Interface
sal_Int32 SAL_CALL OEvoabResultSet::findColumn( const OUString& columnName )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
    {
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

//XColumnLocate interface ends


::cppu::IPropertyArrayHelper* OEvoabResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

::cppu::IPropertyArrayHelper & OEvoabResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

void SAL_CALL OEvoabResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}

void SAL_CALL OEvoabResultSet::release() throw()
{
    OResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
OEvoabResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


} } // connectivity::evoab

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
