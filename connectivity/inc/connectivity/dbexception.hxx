/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbexception.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:01:10 $
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
#define _DBHELPER_DBEXCEPTION_HXX_

#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
//#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
//#include <com/sun/star/sdbc/SQLWarning.hpp>
//#endif
//#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
//#include <com/sun/star/sdb/SQLContext.hpp>
//#endif

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
//= SQLExceptionInfo - encapsulating the type info of an SQLException-derived class
//==============================================================================

class SQLExceptionInfo
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
    SQLExceptionInfo( const ::rtl::OUString& _rSimpleErrorMessage );

    SQLExceptionInfo(const SQLExceptionInfo& _rCopySource);

    SQLExceptionInfo(const ::com::sun::star::sdb::SQLErrorEvent& _rError);
            // use for events got via XSQLErrorListener::errorOccured
    SQLExceptionInfo(const ::com::sun::star::uno::Any& _rError);
            // use with the Reason member of an SQLErrorEvent or with NextElement of an SQLException

    const SQLExceptionInfo& operator=(const ::com::sun::star::sdbc::SQLException& _rError);
    const SQLExceptionInfo& operator=(const ::com::sun::star::sdbc::SQLWarning& _rError);
    const SQLExceptionInfo& operator=(const ::com::sun::star::sdb::SQLContext& _rError);

    sal_Bool    isKindOf(TYPE _eType) const;
        // not just a simple comparisation ! e.g. getType() == SQL_CONTEXT implies isKindOf(SQL_EXCEPTION) == sal_True !
    sal_Bool    isValid() const { return m_eType != UNDEFINED; }
    TYPE        getType() const { return m_eType; }

    operator const ::com::sun::star::sdbc::SQLException*    () const;
    operator const ::com::sun::star::sdbc::SQLWarning*  () const;
    operator const ::com::sun::star::sdb::SQLContext*       () const;

    ::com::sun::star::uno::Any get() const { return m_aContent; }

protected:
    void implDetermineType();
};

//==============================================================================
//= SQLExceptionIteratorHelper - iterating through an SQLException chain
//==============================================================================

class SQLExceptionIteratorHelper
{
public:
    // specifying the type of the elements to include
    enum NODES_INCLUDED { NI_EXCEPTIONS, NI_WARNINGS, NI_CONTEXTINFOS };
        // as ContextInfos are derived from Warnings and Warnings from Exceptions this is sufficient ...

protected:
    const ::com::sun::star::sdbc::SQLException* m_pCurrent;
    SQLExceptionInfo::TYPE          m_eCurrentType;

    NODES_INCLUDED                  m_eMask;

public:
    SQLExceptionIteratorHelper(const ::com::sun::star::sdbc::SQLException* _pStart, NODES_INCLUDED _eMask = NI_EXCEPTIONS);
    SQLExceptionIteratorHelper(const ::com::sun::star::sdbc::SQLWarning* _pStart, NODES_INCLUDED _eMask = NI_EXCEPTIONS);
    SQLExceptionIteratorHelper(const ::com::sun::star::sdb::SQLContext* _pStart, NODES_INCLUDED _eMask = NI_EXCEPTIONS);
        // same note as above for the SQLExceptionInfo ctors
    SQLExceptionIteratorHelper(const SQLExceptionInfo& _rStart, NODES_INCLUDED _eMask = NI_EXCEPTIONS);

    sal_Bool                                    hasMoreElements() const { return (m_pCurrent != NULL); }
    const ::com::sun::star::sdbc::SQLException* next();
    void                                        next(SQLExceptionInfo& _rOutInfo);
};

//==================================================================================
//= StandardExceptions
//==================================================================================
//----------------------------------------------------------------------------------
void throwFunctionNotSupportedException(
        const ::rtl::OUString& _rMsg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()
    )   throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throw a function sequence exception
*/
void throwFunctionSequenceException(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()) throw ( ::com::sun::star::sdbc::SQLException );
//----------------------------------------------------------------------------------
/** throw a invalid index sqlexception
*/
void throwInvalidIndexException(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any()) throw ( ::com::sun::star::sdbc::SQLException );

//----------------------------------------------------------------------------------
/** throw a generic SQLException, i.e. one with an SQLState of S1000, an ErrorCode of 0 and no NextException
*/
void throwGenericSQLException(const ::rtl::OUString& _rMsg, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource)
    throw (::com::sun::star::sdbc::SQLException);

//----------------------------------------------------------------------------------
/** throw a generic SQLException, i.e. one with an SQLState of S1000, an ErrorCode of 0 and no NextException
*/
void throwGenericSQLException(
            const ::rtl::OUString& _rMsg,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource,
            const ::com::sun::star::uno::Any& _rNextException
            )
    throw (::com::sun::star::sdbc::SQLException);

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBEXCEPTION_HXX_


