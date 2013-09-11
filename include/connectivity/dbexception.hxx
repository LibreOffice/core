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

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#define _DBHELPER_DBEXCEPTION_HXX_

#include <com/sun/star/sdbc/SQLException.hpp>
#include "connectivity/standardsqlstate.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace sdb
            {
                class SQLContext;
                struct SQLErrorEvent;
            }
            namespace sdbc
            {
                class SQLWarning;
            }
        }
    }
}
//.........................................................................
namespace dbtools
{
//.........................................................................

//==============================================================================
//= Special exception if cancel is pressed in DBA UI
//==============================================================================
enum OOoBaseErrorCode
{
    ParameterInteractionCancelled = 1
};

//==============================================================================
//= SQLExceptionInfo - encapsulating the type info of an SQLException-derived class
//==============================================================================

class OOO_DLLPUBLIC_DBTOOLS SQLExceptionInfo
{
public:
    enum TYPE { SQL_EXCEPTION, SQL_WARNING, SQL_CONTEXT, UNDEFINED };

private:
    ::com::sun::star::uno::Any  m_aContent;
    TYPE            m_eType;    // redundant (could be derived from m_aContent.getValueType())

public:
    SQLExceptionInfo();
    SQLExceptionInfo(const ::com::sun::star::sdbc::SQLException& _rError);
    SQLExceptionInfo(const ::com::sun::star::sdbc::SQLWarning& _rError);
    SQLExceptionInfo(const ::com::sun::star::sdb::SQLContext& _rError);

    /** convenience constructor

    If your error processing relies on SQLExceptions, and SQLExceptionInfos, you still may
    need to display an error which consists of a simple message string only.
    In those cases, you can use this constructor, which behaves as if you would have used
    an SQLException containing exactly the given error message.
    */
    SQLExceptionInfo( const OUString& _rSimpleErrorMessage );

    SQLExceptionInfo(const SQLExceptionInfo& _rCopySource);

            // use for events got via XSQLErrorListener::errorOccured
    SQLExceptionInfo(const ::com::sun::star::uno::Any& _rError);
            // use with the Reason member of an SQLErrorEvent or with NextElement of an SQLException

    /** prepends a plain error message to the chain of exceptions
        @param  _rSimpleErrorMessage
            the error message to prepend
        @param  _pAsciiSQLState
            the SQLState of the to-be-constructed SQLException, or NULL if this should be defaulted to HY000
        @param  _nErrorCode
            the ErrorCode of the to-be-constructed SQLException
    */
    void    prepend( const OUString& _rErrorMessage, const sal_Char* _pAsciiSQLState = NULL, const sal_Int32 _nErrorCode = 0 );

    /** appends a plain message to the chain of exceptions
        @param  _eType
            the type of exception to append. Must be SQL_EXCEPTION, SQL_WARNING, SQL_CONTEXT, for all other
            values, the behavior is undefined.
        @param  _rErrorMessage
            the message to append
        @param  _pAsciiSQLState
            the SQLState of the exception to append
        @param  _nErrorCode
            the error code of the exception to append
    */
    void    append( TYPE _eType, const OUString& _rErrorMessage, const sal_Char* _pAsciiSQLState = NULL, const sal_Int32 _nErrorCode = 0 );

    /** throws (properly typed) the exception contained in the object
        @precond
            isValid() returns <TRUE/>
        @throws SQLException
        @throws RuntimeException
            if the instance does not contain an SQLException
    */
    void    doThrow();

    const SQLExceptionInfo& operator=(const ::com::sun::star::sdbc::SQLException& _rError);
    const SQLExceptionInfo& operator=(const ::com::sun::star::sdbc::SQLWarning& _rError);
    const SQLExceptionInfo& operator=(const ::com::sun::star::sdb::SQLContext& _rError);
    const SQLExceptionInfo& operator=(const ::com::sun::star::sdb::SQLErrorEvent& _rErrorEvent);
    const SQLExceptionInfo& operator=(const ::com::sun::star::uno::Any& _rCaughtSQLException);

    sal_Bool    isKindOf(TYPE _eType) const;
        // not just a simple comparisation ! e.g. getType() == SQL_CONTEXT implies isKindOf(SQL_EXCEPTION) == sal_True !
    sal_Bool    isValid() const { return m_eType != UNDEFINED; }
    TYPE        getType() const { return m_eType; }

    operator const ::com::sun::star::sdbc::SQLException*    () const;
    operator const ::com::sun::star::sdbc::SQLWarning*      () const;
    operator const ::com::sun::star::sdb::SQLContext*       () const;

    const ::com::sun::star::uno::Any& get() const { return m_aContent; }

    void    clear()
    {
        m_aContent.clear();
        m_eType = UNDEFINED;
    }

protected:
    void implDetermineType();
};

//==============================================================================
//= SQLExceptionIteratorHelper - iterating through an SQLException chain
//==============================================================================

class OOO_DLLPUBLIC_DBTOOLS SQLExceptionIteratorHelper
{
protected:
    const ::com::sun::star::sdbc::SQLException* m_pCurrent;
    SQLExceptionInfo::TYPE                      m_eCurrentType;

public:
    /** constructs an iterator instance from an SQLException

        @param _rChainStart
            the start of the exception chain to iterate. Must live as long as the iterator
            instances lives, at least.
    */
    SQLExceptionIteratorHelper( const ::com::sun::star::sdbc::SQLException& _rChainStart );

    /** constructs an iterator instance from an SQLExceptionInfo

        @param _rErrorInfo
            the start of the exception chain to iterate. Must live as long as the iterator
            instances lives, at least.
    */
    SQLExceptionIteratorHelper( const SQLExceptionInfo& _rErrorInfo );

    /** determines whether there are more elements in the exception chain
    */
    sal_Bool                                    hasMoreElements() const { return ( m_pCurrent != NULL ); }

    /** returns the type of the current element in the exception chain
    */
    SQLExceptionInfo::TYPE                      currentType() const { return m_eCurrentType; }

    /** retrieves the current element in the chain, or <NULL/> if the chain has been completely
        traveled.
    */
    const ::com::sun::star::sdbc::SQLException* current() const { return m_pCurrent; }

    /** retrieves the current element in the chain, or <NULL/> if the chain has been completely
        traveled.

        In opposite to the second <member>current</member>, this version allows typed access to
        the respective SQLException.
    */
    void                                        current( SQLExceptionInfo& _out_rInfo ) const;

    /** proceeds to the next element in the chain

        @return the current element in the chain, as <b>before</em> the chain move.
    */
    const ::com::sun::star::sdbc::SQLException* next();

    /** proceeds to the next element in the chain

        In opposite to the second <member>current</member>, this version allows typed access to
        the respective SQLException.
    */
    void                                        next( SQLExceptionInfo& _out_rInfo );
};

//==================================================================================
//= StandardExceptions
//==================================================================================
//----------------------------------------------------------------------------------
/** returns a standard error string for a given SQLState

    @param _eState
        describes the state whose description is to retrieve. Must not be SQL_ERROR_UNSPECIFIED.
    @raises RuntimeException
        in case of an internal error
*/
OOO_DLLPUBLIC_DBTOOLS OUString getStandardSQLState( StandardSQLState _eState );

//----------------------------------------------------------------------------------
/** returns a standard ASCII string for a given SQLState

    @param _eState
        describes the state whose description is to retrieve. Must not be SQL_ERROR_UNSPECIFIED.
    @return
        a non-<NULL/> pointer to an ASCII character string denoting the requested SQLState
    @raises RuntimeException
        in case of an internal error
*/
OOO_DLLPUBLIC_DBTOOLS const sal_Char* getStandardSQLStateAscii( StandardSQLState _eState );

//----------------------------------------------------------------------------------
OOO_DLLPUBLIC_DBTOOLS void throwFunctionNotSupportedException(
        const OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()
    )
    throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throws an exception with SQL state IM001, saying that a certain function is not supported
*/
OOO_DLLPUBLIC_DBTOOLS void throwFunctionNotSupportedException(
        const sal_Char* _pAsciiFunctionName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const ::com::sun::star::uno::Any* _pNextException = NULL
    )
    throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throws a function sequence (HY010) exception
*/
OOO_DLLPUBLIC_DBTOOLS void throwFunctionSequenceException(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()
    )
    throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throw a invalid index sqlexception
*/
OOO_DLLPUBLIC_DBTOOLS void throwInvalidIndexException(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()
    )
    throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throw a generic SQLException, i.e. one with an SQLState of HY000, an ErrorCode of 0 and no NextException
*/
OOO_DLLPUBLIC_DBTOOLS void throwGenericSQLException(
        const OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throw a generic SQLException, i.e. one with an SQLState of HY000, an ErrorCode of 0 and no NextException
*/
OOO_DLLPUBLIC_DBTOOLS void throwGenericSQLException(
        const OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource,
        const ::com::sun::star::uno::Any& _rNextException
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throw a SQLException with SQLState HYC00 (Optional feature not implemented)
    @param _pAsciiFeatureName
        an ASCII description of the feature which is not implemented. It's recommended that the feature
        name is built from the name of the interface plus its method, for instance "XParameters::updateBinaryStream"
    @param _rxContext
        the context of the exception
    @param _pNextException
        the next exception to chain into the thrown exception, if any
*/
OOO_DLLPUBLIC_DBTOOLS void throwFeatureNotImplementedException(
        const sal_Char* _pAsciiFeatureName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const ::com::sun::star::uno::Any* _pNextException = NULL
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throw a SQLException with SQLState 42S22 (Column Not Found)
    @param _rColumnNameName
        The column that couldn't be found.
    @param _rxContext
        the context of the exception
*/
OOO_DLLPUBLIC_DBTOOLS void throwInvalidColumnException(
        const OUString& _rColumnName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throws an SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwSQLException(
        const sal_Char* _pAsciiMessage,
        const sal_Char* _pAsciiState,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const sal_Int32 _nErrorCode = 0,
        const ::com::sun::star::uno::Any* _pNextException = NULL
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throws an SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwSQLException(
        const sal_Char* _pAsciiMessage,
        StandardSQLState _eSQLState,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const sal_Int32 _nErrorCode = 0,
        const ::com::sun::star::uno::Any* _pNextException = NULL
    )
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throws an SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwSQLException(
        const OUString& _rMessage,
        StandardSQLState _eSQLState,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const sal_Int32 _nErrorCode = 0,
        const ::com::sun::star::uno::Any* _pNextException = NULL
    )
    throw (::com::sun::star::sdbc::SQLException);

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBEXCEPTION_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
