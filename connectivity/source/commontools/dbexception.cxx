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
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include <TConnection.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>

namespace dbtools
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::comphelper;
    using namespace ::connectivity;

SQLExceptionInfo::SQLExceptionInfo()
    :m_eType(TYPE::Undefined)
{
}


SQLExceptionInfo::SQLExceptionInfo(const css::sdbc::SQLException& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}


SQLExceptionInfo::SQLExceptionInfo(const css::sdbc::SQLWarning& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}


SQLExceptionInfo::SQLExceptionInfo(const css::sdb::SQLContext& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}


SQLExceptionInfo::SQLExceptionInfo( const OUString& _rSimpleErrorMessage )
{
    SQLException aError;
    aError.Message = _rSimpleErrorMessage;
    m_aContent <<= aError;
    implDetermineType();
}

SQLExceptionInfo& SQLExceptionInfo::operator=(const css::sdbc::SQLException& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}


SQLExceptionInfo& SQLExceptionInfo::operator=(const css::sdbc::SQLWarning& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}


SQLExceptionInfo& SQLExceptionInfo::operator=(const css::sdb::SQLContext& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
    return *this;
}


SQLExceptionInfo& SQLExceptionInfo::operator=(const css::sdb::SQLErrorEvent& _rErrorEvent)
{
    m_aContent = _rErrorEvent.Reason;
    implDetermineType();
    return *this;
}


SQLExceptionInfo& SQLExceptionInfo::operator=(const css::uno::Any& _rCaughtSQLException)
{
    m_aContent = _rCaughtSQLException;
    implDetermineType();
    return *this;
}


SQLExceptionInfo::SQLExceptionInfo(const css::uno::Any& _rError)
{
    const css::uno::Type& aSQLExceptionType = cppu::UnoType<css::sdbc::SQLException>::get();
    bool bValid = isAssignableFrom(aSQLExceptionType, _rError.getValueType());
    if (bValid)
        m_aContent = _rError;
    // no assertion here : if used with the NextException member of an SQLException bValid==sal_False is allowed.

    implDetermineType();
}


void SQLExceptionInfo::implDetermineType()
{
    const Type& aSQLExceptionType = ::cppu::UnoType<SQLException>::get();
    const Type& aSQLWarningType = ::cppu::UnoType<SQLWarning>::get();
    const Type& aSQLContextType  = ::cppu::UnoType<SQLContext>::get();

    if ( isAssignableFrom( aSQLContextType, m_aContent.getValueType() ) )
        m_eType = TYPE::SQLContext;
    else if ( isAssignableFrom( aSQLWarningType, m_aContent.getValueType() ) )
        m_eType = TYPE::SQLWarning;
    else if ( isAssignableFrom( aSQLExceptionType, m_aContent.getValueType() ) )
        m_eType = TYPE::SQLException;
    else
    {
        m_eType = TYPE::Undefined;
        m_aContent.clear();
    }
}


bool SQLExceptionInfo::isKindOf(TYPE _eType) const
{
    switch (_eType)
    {
        case TYPE::SQLContext:
            return (m_eType == TYPE::SQLContext);
        case TYPE::SQLWarning:
            return (m_eType == TYPE::SQLContext) || (m_eType == TYPE::SQLWarning);
        case TYPE::SQLException:
            return (m_eType == TYPE::SQLContext) || (m_eType == TYPE::SQLWarning) || (m_eType == TYPE::SQLException);
        case TYPE::Undefined:
            return (m_eType == TYPE::Undefined);
    }
    return false;
}


SQLExceptionInfo::operator const css::sdbc::SQLException*() const
{
    OSL_ENSURE(isKindOf(TYPE::SQLException), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return o3tl::doAccess<css::sdbc::SQLException>(m_aContent);
}


SQLExceptionInfo::operator const css::sdb::SQLContext*() const
{
    OSL_ENSURE(isKindOf(TYPE::SQLContext), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return o3tl::doAccess<css::sdb::SQLContext>(m_aContent);
}


void SQLExceptionInfo::prepend( const OUString& _rErrorMessage )
{
    SQLException aException;
    aException.Message = _rErrorMessage;
    aException.ErrorCode = 0;
    aException.SQLState = "S1000";
    aException.NextException = m_aContent;
    m_aContent <<= aException;

    m_eType = TYPE::SQLException;
}


void SQLExceptionInfo::append( TYPE _eType, const OUString& _rErrorMessage, const OUString& _rSQLState, const sal_Int32 _nErrorCode )
{
    // create the to-be-appended exception
    Any aAppend;
    switch ( _eType )
    {
    case TYPE::SQLException: aAppend <<= SQLException(); break;
    case TYPE::SQLWarning:   aAppend <<= SQLWarning();   break;
    case TYPE::SQLContext:   aAppend <<= SQLContext();   break;
    default:
        OSL_FAIL( "SQLExceptionInfo::append: invalid exception type: this will crash!" );
        break;
    }

    SQLException& pAppendException = const_cast<SQLException &>(*o3tl::forceAccess<SQLException>(aAppend));
    pAppendException.Message = _rErrorMessage;
    pAppendException.SQLState = _rSQLState;
    pAppendException.ErrorCode = _nErrorCode;

    // find the end of the current chain
    Any* pChainIterator = &m_aContent;
    SQLException* pLastException = nullptr;
    const Type& aSQLExceptionType( cppu::UnoType<SQLException>::get() );
    while ( pChainIterator )
    {
        if ( !pChainIterator->hasValue() )
            break;

        if ( !isAssignableFrom( aSQLExceptionType, pChainIterator->getValueType() ) )
            break;

        pLastException = const_cast< SQLException* >( o3tl::doAccess<SQLException>( *pChainIterator ) );
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


void SQLExceptionInfo::doThrow()
{
    if ( m_aContent.getValueTypeClass() == TypeClass_EXCEPTION )
        ::cppu::throwException( m_aContent );
    throw RuntimeException();
}

SQLExceptionIteratorHelper::SQLExceptionIteratorHelper( const SQLExceptionInfo& _rChainStart )
    :m_pCurrent( nullptr )
    ,m_eCurrentType( SQLExceptionInfo::TYPE::Undefined )
{
    if ( _rChainStart.isValid() )
    {
        m_pCurrent = static_cast<const SQLException*>(_rChainStart);
        m_eCurrentType = _rChainStart.getType();
    }
}


SQLExceptionIteratorHelper::SQLExceptionIteratorHelper( const css::sdbc::SQLException& _rChainStart )
    :m_pCurrent( &_rChainStart )
    ,m_eCurrentType( SQLExceptionInfo::TYPE::SQLException )
{
}


void SQLExceptionIteratorHelper::current( SQLExceptionInfo& _out_rInfo ) const
{
    switch ( m_eCurrentType )
    {
    case SQLExceptionInfo::TYPE::SQLException:
        _out_rInfo = *m_pCurrent;
        break;

    case SQLExceptionInfo::TYPE::SQLWarning:
        _out_rInfo = *static_cast< const SQLWarning* >( m_pCurrent );
        break;

    case SQLExceptionInfo::TYPE::SQLContext:
        _out_rInfo = *static_cast< const SQLContext* >( m_pCurrent );
        break;

    default:
        _out_rInfo = Any();
        break;
    }
}


const css::sdbc::SQLException* SQLExceptionIteratorHelper::next()
{
    OSL_ENSURE( hasMoreElements(), "SQLExceptionIteratorHelper::next : invalid call (please use hasMoreElements)!" );

    const css::sdbc::SQLException* pReturn = m_pCurrent;
    if ( !m_pCurrent )
        return pReturn;

    // check for the next element within the chain
    const Type aTypeException( ::cppu::UnoType< SQLException >::get() );

    Type aNextElementType = m_pCurrent->NextException.getValueType();
    if ( !isAssignableFrom( aTypeException, aNextElementType ) )
    {
        // no SQLException at all in the next chain element
        m_pCurrent = nullptr;
        m_eCurrentType = SQLExceptionInfo::TYPE::Undefined;
        return pReturn;
    }

    m_pCurrent = o3tl::doAccess< SQLException >( m_pCurrent->NextException );

    // no finally determine the proper type of the exception
    const Type aTypeContext( ::cppu::UnoType< SQLContext >::get() );
    if ( isAssignableFrom( aTypeContext, aNextElementType ) )
    {
        m_eCurrentType = SQLExceptionInfo::TYPE::SQLContext;
        return pReturn;
    }

    const Type aTypeWarning( ::cppu::UnoType< SQLWarning >::get() );
    if ( isAssignableFrom( aTypeWarning, aNextElementType ) )
    {
        m_eCurrentType = SQLExceptionInfo::TYPE::SQLWarning;
        return pReturn;
    }

    // a simple SQLException
    m_eCurrentType = SQLExceptionInfo::TYPE::SQLException;
    return pReturn;
}


void SQLExceptionIteratorHelper::next( SQLExceptionInfo& _out_rInfo )
{
    current( _out_rInfo );
    next();
}


void throwFunctionSequenceException(const Reference< XInterface >& Context, const Any& Next)
{
    ::connectivity::SharedResources aResources;
    throw SQLException(
        aResources.getResourceString(STR_ERRORMSG_SEQUENCE),
        Context,
        getStandardSQLState( StandardSQLState::FUNCTION_SEQUENCE_ERROR ),
        0,
        Next
    );
}

void throwInvalidIndexException(const css::uno::Reference< css::uno::XInterface >& Context,
        const css::uno::Any& Next)
{
    ::connectivity::SharedResources aResources;
    throw SQLException(
        aResources.getResourceString(STR_INVALID_INDEX),
        Context,
        getStandardSQLState( StandardSQLState::INVALID_DESCRIPTOR_INDEX ),
        0,
        Next
    );
}

void throwFunctionNotSupportedSQLException(const OUString& _rFunctionName,
    const css::uno::Reference<css::uno::XInterface>& _rxContext)
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FUNCTION,
            "$functionname$", _rFunctionName
         ) );
    throw SQLException(
        sError,
        _rxContext,
        getStandardSQLState( StandardSQLState::FUNCTION_NOT_SUPPORTED ),
        0,
        css::uno::Any()
    );
}

void throwFunctionNotSupportedRuntimeException(const OUString& _rFunctionName,
    const css::uno::Reference<css::uno::XInterface>& _rxContext)
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FUNCTION,
            "$functionname$", _rFunctionName
         ) );
    throw RuntimeException(
        sError,
        _rxContext
    );
}

void throwGenericSQLException(const OUString& _rMsg, const css::uno::Reference< css::uno::XInterface >& _rxSource)
{
    throwGenericSQLException(_rMsg, _rxSource, Any());
}


void throwGenericSQLException(const OUString& _rMsg, const Reference< XInterface >& _rxSource, const Any& _rNextException)
{
    throw SQLException( _rMsg, _rxSource, getStandardSQLState( StandardSQLState::GENERAL_ERROR ), 0, _rNextException);
}

void throwFeatureNotImplementedSQLException( const OUString& _rFeatureName, const Reference< XInterface >& _rxContext, const Any& _rNextException )
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FEATURE,
            "$featurename$", _rFeatureName
         ) );

    throw SQLException(
        sError,
        _rxContext,
        getStandardSQLState( StandardSQLState::FEATURE_NOT_IMPLEMENTED ),
        0,
        _rNextException
    );
}

void throwFeatureNotImplementedRuntimeException(const OUString& _rFeatureName, const Reference< XInterface >& _rxContext)
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_UNSUPPORTED_FEATURE,
            "$featurename$", _rFeatureName
         ) );

    throw RuntimeException(sError, _rxContext);
}

void throwInvalidColumnException( const OUString& _rColumnName, const Reference< XInterface >& _rxContext)
{
    ::connectivity::SharedResources aResources;
    OUString sErrorMessage( aResources.getResourceStringWithSubstitution(
                                STR_INVALID_COLUMNNAME,
                                "$columnname$",_rColumnName) );
    throwSQLException( sErrorMessage, StandardSQLState::COLUMN_NOT_FOUND, _rxContext );
}

void throwSQLException( const OUString& _rMessage, const OUString& _rSQLState,
        const Reference< XInterface >& _rxContext, const sal_Int32 _nErrorCode )
{
    throw SQLException(
        _rMessage,
        _rxContext,
        _rSQLState,
        _nErrorCode,
        Any()
    );
}


void throwSQLException( const OUString& _rMessage, StandardSQLState _eSQLState,
        const Reference< XInterface >& _rxContext, const sal_Int32 _nErrorCode )
{
    throwSQLException( _rMessage, getStandardSQLState( _eSQLState ), _rxContext, _nErrorCode );
}


OUString getStandardSQLState( StandardSQLState _eState )
{
    switch ( _eState )
    {
    case StandardSQLState::INVALID_DESCRIPTOR_INDEX:  return OUString("07009");
    case StandardSQLState::INVALID_CURSOR_STATE:      return OUString("24000");
    case StandardSQLState::COLUMN_NOT_FOUND:          return OUString("42S22");
    case StandardSQLState::GENERAL_ERROR:             return OUString("HY000");
    case StandardSQLState::INVALID_SQL_DATA_TYPE:     return OUString("HY004");
    case StandardSQLState::FUNCTION_SEQUENCE_ERROR:   return OUString("HY010");
    case StandardSQLState::INVALID_CURSOR_POSITION:   return OUString("HY109");
    case StandardSQLState::FEATURE_NOT_IMPLEMENTED:   return OUString("HYC00");
    case StandardSQLState::FUNCTION_NOT_SUPPORTED:    return OUString("IM001");
    case StandardSQLState::CONNECTION_DOES_NOT_EXIST: return OUString("08003");
    default:                                          return OUString("HY001"); // General Error
    }
}


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
