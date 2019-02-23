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

#ifndef INCLUDED_CONNECTIVITY_DBEXCEPTION_HXX
#define INCLUDED_CONNECTIVITY_DBEXCEPTION_HXX

#include <connectivity/standardsqlstate.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/uno/Reference.hxx>

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
                class SQLException;
            }
        }
    }
}

namespace dbtools
{


//= Special exception if cancel is pressed in DBA UI

enum OOoBaseErrorCode
{
    ParameterInteractionCancelled = 1
};


//= SQLExceptionInfo - encapsulating the type info of an SQLException-derived class


class OOO_DLLPUBLIC_DBTOOLS SQLExceptionInfo final
{
public:
    enum class TYPE { SQLException, SQLWarning, SQLContext, Undefined };

private:
    css::uno::Any  m_aContent;
    TYPE            m_eType;    // redundant (could be derived from m_aContent.getValueType())

public:
    SQLExceptionInfo();
    SQLExceptionInfo(const css::sdbc::SQLException& _rError);
    SQLExceptionInfo(const css::sdbc::SQLWarning& _rError);
    SQLExceptionInfo(const css::sdb::SQLContext& _rError);

    /** convenience constructor

    If your error processing relies on SQLExceptions, and SQLExceptionInfos, you still may
    need to display an error which consists of a simple message string only.
    In those cases, you can use this constructor, which behaves as if you would have used
    an SQLException containing exactly the given error message.
    */
    SQLExceptionInfo( const OUString& _rSimpleErrorMessage );

            // use for events got via XSQLErrorListener::errorOccured
    SQLExceptionInfo(const css::uno::Any& _rError);
            // use with the Reason member of an SQLErrorEvent or with NextElement of an SQLException

    /** prepends a plain error message to the chain of exceptions
        @param  _rSimpleErrorMessage
            the error message to prepend
    */
    void    prepend( const OUString& _rErrorMessage );

    /** appends a plain message to the chain of exceptions
        @param  _eType
            the type of exception to append. Must be SQL_EXCEPTION, SQL_WARNING, SQL_CONTEXT, for all other
            values, the behavior is undefined.
        @param  _rErrorMessage
            the message to append
        @param  _rSQLState
            the SQLState of the exception to append
        @param  _nErrorCode
            the error code of the exception to append
    */
    void    append( TYPE _eType, const OUString& _rErrorMessage, const OUString& _rSQLState = OUString(), const sal_Int32 _nErrorCode = 0 );

    /** throws (properly typed) the exception contained in the object
        @precond
            isValid() returns <TRUE/>
        @throws SQLException
        @throws RuntimeException
            if the instance does not contain an SQLException
    */
    void    doThrow();

    SQLExceptionInfo& operator=(const css::sdbc::SQLException& _rError);
    SQLExceptionInfo& operator=(const css::sdbc::SQLWarning& _rError);
    SQLExceptionInfo& operator=(const css::sdb::SQLContext& _rError);
    SQLExceptionInfo& operator=(const css::sdb::SQLErrorEvent& _rErrorEvent);
    SQLExceptionInfo& operator=(const css::uno::Any& _rCaughtSQLException);

    bool        isKindOf(TYPE _eType) const;
        // not just a simple comparison ! e.g. getType() == SQL_CONTEXT implies isKindOf(SQL_EXCEPTION) == sal_True !
    bool        isValid() const { return m_eType != TYPE::Undefined; }
    TYPE        getType() const { return m_eType; }

    operator const css::sdbc::SQLException*    () const;
    operator const css::sdb::SQLContext*       () const;

    const css::uno::Any& get() const { return m_aContent; }

    void    clear()
    {
        m_aContent.clear();
        m_eType = TYPE::Undefined;
    }

private:
    void implDetermineType();
};


//= SQLExceptionIteratorHelper - iterating through an SQLException chain


class OOO_DLLPUBLIC_DBTOOLS SQLExceptionIteratorHelper final
{
    const css::sdbc::SQLException* m_pCurrent;
    SQLExceptionInfo::TYPE                      m_eCurrentType;

public:
    /** constructs an iterator instance from an SQLException

        @param _rChainStart
            the start of the exception chain to iterate. Must live as long as the iterator
            instances lives, at least.
    */
    SQLExceptionIteratorHelper( const css::sdbc::SQLException& _rChainStart );

    /** constructs an iterator instance from an SQLExceptionInfo

        @param _rErrorInfo
            the start of the exception chain to iterate. Must live as long as the iterator
            instances lives, at least.
    */
    SQLExceptionIteratorHelper( const SQLExceptionInfo& _rErrorInfo );

    /** determines whether there are more elements in the exception chain
    */
    bool                                        hasMoreElements() const { return ( m_pCurrent != nullptr ); }

    /** retrieves the current element in the chain, or <NULL/> if the chain has been completely
        traveled.

        In opposite to the second <member>current</member>, this version allows typed access to
        the respective SQLException.
    */
    void                                        current( SQLExceptionInfo& _out_rInfo ) const;

    /** proceeds to the next element in the chain

        @return the current element in the chain, as <b>before</em> the chain move.
    */
    const css::sdbc::SQLException* next();

    /** proceeds to the next element in the chain

        In opposite to the second <member>current</member>, this version allows typed access to
        the respective SQLException.
    */
    void                                        next( SQLExceptionInfo& _out_rInfo );
};


//= StandardExceptions


/** returns a standard error string for a given SQLState

    @param _eState
        describes the state whose description is to retrieve. Must not be SQL_ERROR_UNSPECIFIED.
    @throws RuntimeException
        in case of an internal error
*/
OOO_DLLPUBLIC_DBTOOLS OUString getStandardSQLState( StandardSQLState _eState );


/** throws an exception with SQL state IM001, saying that a certain function is not supported

    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwFunctionNotSupportedSQLException(
        const OUString& _rFunctionName,
        const css::uno::Reference< css::uno::XInterface >& _rxContext
    );

/// @throws css::uno::RuntimeException
OOO_DLLPUBLIC_DBTOOLS void throwFunctionNotSupportedRuntimeException(
        const OUString& _rFunctionName,
        const css::uno::Reference< css::uno::XInterface >& _rxContext
    );

/** throws a function sequence (HY010) exception

    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwFunctionSequenceException(
        const css::uno::Reference< css::uno::XInterface >& Context,
        const css::uno::Any& Next = css::uno::Any()
    );


/** throw a invalid index sqlexception

    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwInvalidIndexException(
        const css::uno::Reference< css::uno::XInterface >& Context,
        const css::uno::Any& Next = css::uno::Any()
    );


/** throw a generic SQLException, i.e. one with an SQLState of HY000, an ErrorCode of 0 and no NextException

    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwGenericSQLException(
        const OUString& _rMsg,
        const css::uno::Reference< css::uno::XInterface >& _rxSource
    );


/** throw a generic SQLException, i.e. one with an SQLState of HY000, an ErrorCode of 0 and no NextException

    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwGenericSQLException(
        const OUString& _rMsg,
        const css::uno::Reference< css::uno::XInterface >& _rxSource,
        const css::uno::Any& _rNextException
    );


/** throw a SQLException with SQLState HYC00 (Optional feature not implemented)
    @param _rFeatureName
        a description of the feature which is not implemented. It's recommended that the feature
        name is built from the name of the interface plus its method, for instance "XParameters::updateBinaryStream"
    @param _rxContext
        the context of the exception
    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwFeatureNotImplementedSQLException(
        const OUString& _rFeatureName,
        const css::uno::Reference< css::uno::XInterface >& _rxContext,
        const css::uno::Any& _rNextException = css::uno::Any()
    );

/** throw a RuntimeException (Optional feature not implemented)
    @param _rFeatureName
        a description of the feature which is not implemented. It's recommended that the feature
        name is built from the name of the interface plus its method, for instance "XParameters::updateBinaryStream"
    @param _rxContext
        the context of the exception
    @throws css::uno::RuntimeException
*/
OOO_DLLPUBLIC_DBTOOLS void throwFeatureNotImplementedRuntimeException(
        const OUString& _rFeatureName,
        const css::uno::Reference< css::uno::XInterface >& _rxContext
    );

/** throw a SQLException with SQLState 42S22 (Column Not Found)
    @param _rColumnNameName
        The column that couldn't be found.
    @param _rxContext
        the context of the exception
    @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwInvalidColumnException(
        const OUString& _rColumnName,
        const css::uno::Reference< css::uno::XInterface >& _rxContext
    );


/** @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwSQLException(
        const OUString& _rMessage,
        const OUString& _rSQLState,
        const css::uno::Reference< css::uno::XInterface >& _rxContext,
        const sal_Int32 _nErrorCode
    );


/** @throws css::sdbc::SQLException
*/
OOO_DLLPUBLIC_DBTOOLS void throwSQLException(
        const OUString& _rMessage,
        StandardSQLState _eSQLState,
        const css::uno::Reference< css::uno::XInterface >& _rxContext,
        const sal_Int32 _nErrorCode = 0
    );


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_DBEXCEPTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
