/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbexception.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:05:49 $
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

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
using namespace comphelper;

//.........................................................................
namespace dbtools
{
//.........................................................................

    using namespace ::connectivity;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;

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
SQLExceptionInfo::SQLExceptionInfo(const ::com::sun::star::sdb::SQLErrorEvent& _rError)
{
    const staruno::Type& aSQLExceptionType = ::getCppuType(reinterpret_cast< ::com::sun::star::sdbc::SQLException*>(NULL));
    staruno::Type aReasonType = _rError.Reason.getValueType();

    sal_Bool bValid = isAssignableFrom(aSQLExceptionType, aReasonType);
    OSL_ENSURE(bValid, "SQLExceptionInfo::SQLExceptionInfo : invalid argument (does not contain an SQLException) !");
    if (bValid)
        m_aContent = _rError.Reason;

    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const staruno::Any& _rError)
{
    const staruno::Type& aSQLExceptionType = ::getCppuType(reinterpret_cast< ::com::sun::star::sdbc::SQLException*>(NULL));
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
    if (isA(aContentType, static_cast< ::com::sun::star::sdb::SQLContext*>(NULL)))
        m_eType = SQL_CONTEXT;
    else if (isA(aContentType, static_cast< ::com::sun::star::sdbc::SQLWarning*>(NULL)))
        m_eType = SQL_WARNING;
    else if (isA(aContentType, static_cast< ::com::sun::star::sdbc::SQLException*>(NULL)))
        m_eType = SQL_EXCEPTION;
    else
        m_eType = UNDEFINED;
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

//==============================================================================
//= SQLExceptionIteratorHelper - iterating through an SQLException chain
//==============================================================================

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const SQLExceptionInfo& _rStart)
    :m_pCurrent(NULL)
    ,m_eCurrentType(SQLExceptionInfo::UNDEFINED)
        // no other chance without RTTI
{
    if (_rStart.isValid())
    {
        m_pCurrent = (const SQLException*)_rStart;
        m_eCurrentType = _rStart.getType();
    }
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const ::com::sun::star::sdbc::SQLException* _pStart)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_EXCEPTION)
                // no other chance without RTTI
{
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const ::com::sun::star::sdbc::SQLWarning* _pStart)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_WARNING)
                // no other chance without RTTI
{
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const ::com::sun::star::sdb::SQLContext* _pStart)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_CONTEXT)
                // no other chance without RTTI
{
}

//------------------------------------------------------------------------------
void SQLExceptionIteratorHelper::next(SQLExceptionInfo& _rOutInfo)
{
    SQLExceptionInfo::TYPE eType = m_eCurrentType;
    const SQLException* pNext = next();
    switch (eType)
    {
        case SQLExceptionInfo::SQL_EXCEPTION:
            _rOutInfo = *pNext;
            break;
        case SQLExceptionInfo::SQL_WARNING:
            _rOutInfo = *static_cast<const SQLWarning*>(pNext);
            break;
        case SQLExceptionInfo::SQL_CONTEXT:
            _rOutInfo = *static_cast<const SQLContext*>(pNext);
            break;
        default:
            OSL_ENSURE(sal_False, "SQLExceptionIteratorHelper::next: invalid type!");
    }
}

//------------------------------------------------------------------------------
const ::com::sun::star::sdbc::SQLException* SQLExceptionIteratorHelper::next()
{
    OSL_ENSURE(hasMoreElements(), "SQLExceptionIteratorHelper::next : invalid call (please use hasMoreElements) !");

    const ::com::sun::star::sdbc::SQLException* pReturn = m_pCurrent;
    if (m_pCurrent)
    {   // check for the next element within the chain
        const staruno::Type& aSqlExceptionCompare = ::getCppuType(reinterpret_cast< ::com::sun::star::sdbc::SQLException*>(NULL));

        const ::com::sun::star::sdbc::SQLException* pSearch         = m_pCurrent;
        SQLExceptionInfo::TYPE eSearchType  = m_eCurrentType;

        do
        {
            if ( !pSearch )
                break;

            if (!pSearch->NextException.hasValue())
            {   // last chain element
                pSearch = NULL;
                break;
            }

            staruno::Type aNextElementType = pSearch->NextException.getValueType();
            if (!isAssignableFrom(aSqlExceptionCompare, aNextElementType))
            {
                // the next chain element isn't an SQLException
                OSL_ENSURE(sal_False, "SQLExceptionIteratorHelper::next : the exception chain is invalid !");
                pSearch = NULL;
                break;
            }

            // the next element
            SQLExceptionInfo aInfo(pSearch->NextException);
            eSearchType = aInfo.getType();
            switch (eSearchType)
            {
                case SQLExceptionInfo::SQL_CONTEXT:
                    pSearch = reinterpret_cast<const ::com::sun::star::sdb::SQLContext*>(pSearch->NextException.getValue());
                    break;

                case SQLExceptionInfo::SQL_WARNING:
                    pSearch = reinterpret_cast<const ::com::sun::star::sdbc::SQLWarning*>(pSearch->NextException.getValue());
                    break;

                case SQLExceptionInfo::SQL_EXCEPTION:
                    pSearch = reinterpret_cast<const ::com::sun::star::sdbc::SQLException*>(pSearch->NextException.getValue());
                    break;

                default:
                    pSearch = NULL;
                    break;
            }
        }
        while ( false );

        m_pCurrent = pSearch;
        m_eCurrentType = eSearchType;
    }

    return pReturn;
}
using namespace ::com::sun::star::uno;
//------------------------------------------------------------
void throwFunctionSequenceException(const Reference< XInterface >& _Context, const Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    throw SQLException(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ERRORMSG_SEQUENCE), _Context, OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000), 0, _Next);
}
// -----------------------------------------------------------------------------
void throwInvalidIndexException(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    static ::rtl::OUString sStatus = ::rtl::OUString::createFromAscii("07009");
    throw SQLException(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_INVALID_INDEX),_Context,sStatus,0,_Next);
}
// -----------------------------------------------------------------------------
void throwFunctionNotSupportedException(const ::rtl::OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next)  throw ( ::com::sun::star::sdbc::SQLException )
{
    static ::rtl::OUString sStatus = ::rtl::OUString::createFromAscii("IM001");
    throw SQLException(_rMsg,_Context,sStatus,0,_Next);
}
// -----------------------------------------------------------------------------
void throwFunctionNotSupportedException( const sal_Char* _pAsciiFunctionName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const ::com::sun::star::uno::Any* _pNextException ) throw ( ::com::sun::star::sdbc::SQLException )
{
    ::rtl::OUString sMessage = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": Driver does not support this function: " ) );
    sMessage += ::rtl::OUString::createFromAscii( _pAsciiFunctionName );
    ::rtl::OUString sState( RTL_CONSTASCII_USTRINGPARAM( "IM001" ) );
    throw SQLException( sMessage, _rxContext, sState, 0, _pNextException ? *_pNextException : Any() );
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
    static ::rtl::OUString sStatus = ::rtl::OUString::createFromAscii("S1000");
    throw SQLException(_rMsg, _rxSource, sStatus, 0, _rNextException);
}

// -----------------------------------------------------------------------------
void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": feature not implemented." ) );
    ::rtl::OUString sState( RTL_CONSTASCII_USTRINGPARAM( "HYC00" ) );
    throw SQLException( sMessage, _rxContext, sState, 0, _pNextException ? *_pNextException : Any() );
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
        case SQL_OPERATION_CANCELED:        pAsciiState = "HY008"; break;
        case SQL_FUNCTION_SEQUENCE_ERROR:   pAsciiState = "HY010"; break;
        case SQL_INVALID_CURSOR_POSITION:   pAsciiState = "HY109"; break;
        case SQL_INVALID_BOOKMARK_VALUE:    pAsciiState = "HY111"; break;
        case SQL_FEATURE_NOT_IMPLEMENTED:   pAsciiState = "HYC00"; break;
        case SQL_FUNCTION_NOT_SUPPORTED:    pAsciiState = "IM001"; break;
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


