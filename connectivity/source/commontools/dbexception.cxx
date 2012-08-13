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

#include <connectivity/dbexception.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include "TConnection.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

//.........................................................................
namespace dbtools
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::comphelper;
    using namespace ::connectivity;

//==============================================================================
//= SQLExceptionInfo - encapsulating the type info of an SQLException-derived class
//==============================================================================
//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo()
    :m_eType(UNDEFINED)
{
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const ::com::sun::star::sdbc::SQLException& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const ::com::sun::star::sdbc::SQLWarning& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const ::com::sun::star::sdb::SQLContext& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo( const ::rtl::OUString& _rSimpleErrorMessage )
{
    SQLException aError;
    aError.Message = _rSimpleErrorMessage;
    m_aContent <<= aError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const SQLExceptionInfo& _rCopySource)
    :m_aContent(_rCopySource.m_aContent)
    ,m_eType(_rCopySource.m_eType)
{
}

//------------------------------------------------------------------------------
const SQLExceptionInfo& SQLExceptionInfo::operator=(const ::com::sun::star::sdbc::SQLException& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}

//------------------------------------------------------------------------------
const SQLExceptionInfo& SQLExceptionInfo::operator=(const ::com::sun::star::sdbc::SQLWarning& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}

//------------------------------------------------------------------------------
const SQLExceptionInfo& SQLExceptionInfo::operator=(const ::com::sun::star::sdb::SQLContext& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}

//------------------------------------------------------------------------------
const SQLExceptionInfo& SQLExceptionInfo::operator=(const ::com::sun::star::sdb::SQLErrorEvent& _rErrorEvent)
{
    m_aContent = _rErrorEvent.Reason;
    implDetermineType();
    return *this;
}

//------------------------------------------------------------------------------
const SQLExceptionInfo& SQLExceptionInfo::operator=(const ::com::sun::star::uno::Any& _rCaughtSQLException)
{
    m_aContent = _rCaughtSQLException;
    implDetermineType();
    return *this;
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const staruno::Any& _rError)
{
    const staruno::Type& aSQLExceptionType = ::getCppuType(static_cast< ::com::sun::star::sdbc::SQLException*>(0));
    sal_Bool bValid = isAssignableFrom(aSQLExceptionType, _rError.getValueType());
    if (bValid)
        m_aContent = _rError;
    // no assertion here : if used with the NextException member of an SQLException bValid==sal_False is allowed.

    implDetermineType();
}

//------------------------------------------------------------------------------
void SQLExceptionInfo::implDetermineType()
{
    staruno::Type aContentType = m_aContent.getValueType();

    const Type& aSQLExceptionType = ::getCppuType( static_cast< SQLException* >( 0 ) );
    const Type& aSQLWarningType = ::getCppuType( static_cast< SQLWarning* >( 0 ) );
    const Type& aSQLContextType  = ::getCppuType( static_cast< SQLContext* >( 0 ) );

    if ( isAssignableFrom( aSQLContextType, m_aContent.getValueType() ) )
        m_eType = SQL_CONTEXT;
    else if ( isAssignableFrom( aSQLWarningType, m_aContent.getValueType() ) )
        m_eType = SQL_WARNING;
    else if ( isAssignableFrom( aSQLExceptionType, m_aContent.getValueType() ) )
        m_eType = SQL_EXCEPTION;
    else
    {
        m_eType = UNDEFINED;
        m_aContent.clear();
    }
}

//------------------------------------------------------------------------------
sal_Bool SQLExceptionInfo::isKindOf(TYPE _eType) const
{
    switch (_eType)
    {
        case SQL_CONTEXT:
            return (m_eType == SQL_CONTEXT);
        case SQL_WARNING:
            return (m_eType == SQL_CONTEXT) || (m_eType == SQL_WARNING);
        case SQL_EXCEPTION:
            return (m_eType == SQL_CONTEXT) || (m_eType == SQL_WARNING) || (m_eType == SQL_EXCEPTION);
        case UNDEFINED:
            return (m_eType == UNDEFINED);
    }
    return sal_False;
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const ::com::sun::star::sdbc::SQLException*() const
{
    OSL_ENSURE(isKindOf(SQL_EXCEPTION), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const ::com::sun::star::sdbc::SQLException*>(m_aContent.getValue());
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const ::com::sun::star::sdbc::SQLWarning*() const
{
    OSL_ENSURE(isKindOf(SQL_WARNING), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const ::com::sun::star::sdbc::SQLWarning*>(m_aContent.getValue());
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const ::com::sun::star::sdb::SQLContext*() const
{
    OSL_ENSURE(isKindOf(SQL_CONTEXT), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const ::com::sun::star::sdb::SQLContext*>(m_aContent.getValue());
}

//------------------------------------------------------------------------------
void SQLExceptionInfo::prepend( const ::rtl::OUString& _rErrorMessage, const sal_Char* _pAsciiSQLState, const sal_Int32 _nErrorCode )
{
    SQLException aException;
    aException.Message = _rErrorMessage;
    aException.ErrorCode = _nErrorCode;
    aException.SQLState = _pAsciiSQLState ? ::rtl::OUString::createFromAscii( _pAsciiSQLState ) : ::rtl::OUString("S1000" );
    aException.NextException = m_aContent;
    m_aContent <<= aException;

    m_eType = SQL_EXCEPTION;
}

//------------------------------------------------------------------------------
void SQLExceptionInfo::append( TYPE _eType, const ::rtl::OUString& _rErrorMessage, const sal_Char* _pAsciiSQLState, const sal_Int32 _nErrorCode )
{
    // create the to-be-appended exception
    Any aAppend;
    switch ( _eType )
    {
    case SQL_EXCEPTION: aAppend <<= SQLException(); break;
    case SQL_WARNING:   aAppend <<= SQLWarning();   break;
    case SQL_CONTEXT:   aAppend <<= SQLContext();   break;
    default:
        OSL_FAIL( "SQLExceptionInfo::append: invalid exception type: this will crash!" );
        break;
    }

    SQLException* pAppendException( static_cast< SQLException* >( const_cast< void* >( aAppend.getValue() ) ) );
    pAppendException->Message = _rErrorMessage;
    pAppendException->SQLState = ::rtl::OUString::createFromAscii( _pAsciiSQLState );
    pAppendException->ErrorCode = _nErrorCode;

    // find the end of the current chain
    Any* pChainIterator = &m_aContent;
    SQLException* pLastException = NULL;
    const Type& aSQLExceptionType( ::getCppuType< SQLException >() );
    while ( pChainIterator )
    {
        if ( !pChainIterator->hasValue() )
            break;

        if ( !isAssignableFrom( aSQLExceptionType, pChainIterator->getValueType() ) )
            break;

        pLastException = static_cast< SQLException* >( const_cast< void* >( pChainIterator->getValue() ) );
        pChainIterator = &pLastException->NextException;
    }

    // append
    if ( pLastException )
        pLastException->NextException = aAppend;
    else
    {
        m_aContent = aAppend;
        m_eType = _eType;
    }
}

//------------------------------------------------------------------------------
void SQLExceptionInfo::doThrow()
{
    if ( m_aContent.getValueTypeClass() == TypeClass_EXCEPTION )
        ::cppu::throwException( m_aContent );
    throw RuntimeException();
}

//==============================================================================
//= SQLExceptionIteratorHelper
//==============================================================================

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper( const SQLExceptionInfo& _rChainStart )
    :m_pCurrent( NULL )
    ,m_eCurrentType( SQLExceptionInfo::UNDEFINED )
{
    if ( _rChainStart.isValid() )
    {
        m_pCurrent = (const SQLException*)_rChainStart;
        m_eCurrentType = _rChainStart.getType();
    }
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper( const ::com::sun::star::sdbc::SQLException& _rChainStart )
    :m_pCurrent( &_rChainStart )
    ,m_eCurrentType( SQLExceptionInfo::SQL_EXCEPTION )
{
}

//------------------------------------------------------------------------------
void SQLExceptionIteratorHelper::current( SQLExceptionInfo& _out_rInfo ) const
{
    switch ( m_eCurrentType )
    {
    case SQLExceptionInfo::SQL_EXCEPTION:
        _out_rInfo = *m_pCurrent;
        break;

    case SQLExceptionInfo::SQL_WARNING:
        _out_rInfo = *static_cast< const SQLWarning* >( m_pCurrent );
        break;

    case SQLExceptionInfo::SQL_CONTEXT:
        _out_rInfo = *static_cast< const SQLContext* >( m_pCurrent );
        break;

    default:
        _out_rInfo = Any();
        break;
    }
}

//------------------------------------------------------------------------------
const ::com::sun::star::sdbc::SQLException* SQLExceptionIteratorHelper::next()
{
    OSL_ENSURE( hasMoreElements(), "SQLExceptionIteratorHelper::next : invalid call (please use hasMoreElements)!" );

    const ::com::sun::star::sdbc::SQLException* pReturn = current();
    if ( !m_pCurrent )
        return pReturn;

    // check for the next element within the chain
    const Type aTypeException( ::cppu::UnoType< SQLException >::get() );

    Type aNextElementType = m_pCurrent->NextException.getValueType();
    if ( !isAssignableFrom( aTypeException, aNextElementType ) )
    {
        // no SQLException at all in the next chain element
        m_pCurrent = NULL;
        m_eCurrentType = SQLExceptionInfo::UNDEFINED;
        return pReturn;
    }

    m_pCurrent = static_cast< const SQLException* >( m_pCurrent->NextException.getValue() );

    // no finally determine the proper type of the exception
    const Type aTypeContext( ::cppu::UnoType< SQLContext >::get() );
    if ( isAssignableFrom( aTypeContext, aNextElementType ) )
    {
        m_eCurrentType = SQLExceptionInfo::SQL_CONTEXT;
        return pReturn;
    }

    const Type aTypeWarning( ::cppu::UnoType< SQLWarning >::get() );
    if ( isAssignableFrom( aTypeWarning, aNextElementType ) )
    {
        m_eCurrentType = SQLExceptionInfo::SQL_WARNING;
        return pReturn;
    }

    // a simple SQLException
    m_eCurrentType = SQLExceptionInfo::SQL_EXCEPTION;
    return pReturn;
}

//------------------------------------------------------------------------------
void SQLExceptionIteratorHelper::next( SQLExceptionInfo& _out_rInfo )
{
    current( _out_rInfo );
    next();
}

//------------------------------------------------------------
void throwFunctionSequenceException(const Reference< XInterface >& _Context, const Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    ::connectivity::SharedResources aResources;
    throw SQLException(
        aResources.getResourceString(STR_ERRORMSG_SEQUENCE),
        _Context,
        getStandardSQLState( SQL_FUNCTION_SEQUENCE_ERROR ),
        0,
        _Next
    );
}
// -----------------------------------------------------------------------------
void throwInvalidIndexException(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    ::connectivity::SharedResources aResources;
    throw SQLException(
        aResources.getResourceString(STR_INVALID_INDEX),
        _Context,
        getStandardSQLState( SQL_INVALID_DESCRIPTOR_INDEX ),
        0,
        _Next
    );
}
// -----------------------------------------------------------------------------
void throwFunctionNotSupportedException(const ::rtl::OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    throw SQLException(
        _rMsg,
        _Context,
        getStandardSQLState( SQL_FUNCTION_NOT_SUPPORTED ),
        0,
        _Next
    );
}
// -----------------------------------------------------------------------------
void throwFunctionNotSupportedException( const sal_Char* _pAsciiFunctionName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const ::com::sun::star::uno::Any* _pNextException ) throw ( ::com::sun::star::sdbc::SQLException )
{
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FUNCTION,
            "$functionname$", ::rtl::OUString::createFromAscii( _pAsciiFunctionName )
         ) );
    throwFunctionNotSupportedException(
        sError,
        _rxContext,
        _pNextException ? *_pNextException : Any()
    );
}
// -----------------------------------------------------------------------------
void throwGenericSQLException(const ::rtl::OUString& _rMsg, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource)
    throw (::com::sun::star::sdbc::SQLException)
{
    throwGenericSQLException(_rMsg, _rxSource, Any());
}

// -----------------------------------------------------------------------------
void throwGenericSQLException(const ::rtl::OUString& _rMsg, const Reference< XInterface >& _rxSource, const Any& _rNextException)
    throw (SQLException)
{
    throw SQLException( _rMsg, _rxSource, getStandardSQLState( SQL_GENERAL_ERROR ), 0, _rNextException);
}

// -----------------------------------------------------------------------------
void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FEATURE,
            "$featurename$", ::rtl::OUString::createFromAscii( _pAsciiFeatureName )
         ) );

    throw SQLException(
        sError,
        _rxContext,
        getStandardSQLState( SQL_FEATURE_NOT_IMPLEMENTED ),
        0,
        _pNextException ? *_pNextException : Any()
    );
}

// -----------------------------------------------------------------------------
void throwSQLException( const sal_Char* _pAsciiMessage, const sal_Char* _pAsciiState,
        const Reference< XInterface >& _rxContext, const sal_Int32 _nErrorCode, const Any* _pNextException ) throw (SQLException)
{
    throw SQLException(
        ::rtl::OUString::createFromAscii( _pAsciiMessage ),
        _rxContext,
        ::rtl::OUString::createFromAscii( _pAsciiState ),
        _nErrorCode,
        _pNextException ? *_pNextException : Any()
    );
}

// -----------------------------------------------------------------------------
void throwSQLException( const sal_Char* _pAsciiMessage, StandardSQLState _eSQLState,
        const Reference< XInterface >& _rxContext, const sal_Int32 _nErrorCode,
        const Any* _pNextException ) throw (SQLException)
{
    throwSQLException( _pAsciiMessage, getStandardSQLStateAscii( _eSQLState ), _rxContext, _nErrorCode, _pNextException );
}

// -----------------------------------------------------------------------------
void throwSQLException( const ::rtl::OUString& _rMessage, StandardSQLState _eSQLState,
        const Reference< XInterface >& _rxContext, const sal_Int32 _nErrorCode,
        const Any* _pNextException ) throw (SQLException)
{
    throw SQLException(
        _rMessage,
        _rxContext,
        getStandardSQLState( _eSQLState ),
        _nErrorCode,
        _pNextException ? *_pNextException : Any()
    );
}

// -----------------------------------------------------------------------------
const sal_Char* getStandardSQLStateAscii( StandardSQLState _eState )
{
    const sal_Char* pAsciiState = NULL;
    switch ( _eState )
    {
        case SQL_WRONG_PARAMETER_NUMBER:    pAsciiState = "07001"; break;
        case SQL_INVALID_DESCRIPTOR_INDEX:  pAsciiState = "07009"; break;
        case SQL_UNABLE_TO_CONNECT:         pAsciiState = "08001"; break;
        case SQL_NUMERIC_OUT_OF_RANGE:      pAsciiState = "22003"; break;
        case SQL_INVALID_DATE_TIME:         pAsciiState = "22007"; break;
        case SQL_INVALID_CURSOR_STATE:      pAsciiState = "24000"; break;
        case SQL_TABLE_OR_VIEW_EXISTS:      pAsciiState = "42S01"; break;
        case SQL_TABLE_OR_VIEW_NOT_FOUND:   pAsciiState = "42S02"; break;
        case SQL_INDEX_ESISTS:              pAsciiState = "42S11"; break;
        case SQL_INDEX_NOT_FOUND:           pAsciiState = "42S12"; break;
        case SQL_COLUMN_EXISTS:             pAsciiState = "42S21"; break;
        case SQL_COLUMN_NOT_FOUND:          pAsciiState = "42S22"; break;
        case SQL_GENERAL_ERROR:             pAsciiState = "HY000"; break;
        case SQL_INVALID_SQL_DATA_TYPE:     pAsciiState = "HY004"; break;
        case SQL_OPERATION_CANCELED:        pAsciiState = "HY008"; break;
        case SQL_FUNCTION_SEQUENCE_ERROR:   pAsciiState = "HY010"; break;
        case SQL_INVALID_CURSOR_POSITION:   pAsciiState = "HY109"; break;
        case SQL_INVALID_BOOKMARK_VALUE:    pAsciiState = "HY111"; break;
        case SQL_FEATURE_NOT_IMPLEMENTED:   pAsciiState = "HYC00"; break;
        case SQL_FUNCTION_NOT_SUPPORTED:    pAsciiState = "IM001"; break;
        case SQL_CONNECTION_DOES_NOT_EXIST: pAsciiState = "08003"; break;

        default:
            break;
    }
    if ( !pAsciiState )
        throw RuntimeException();
    return pAsciiState;
}

// -----------------------------------------------------------------------------
::rtl::OUString getStandardSQLState( StandardSQLState _eState )
{
    return ::rtl::OUString::createFromAscii( getStandardSQLStateAscii( _eState ) );
}

// -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbtools
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
