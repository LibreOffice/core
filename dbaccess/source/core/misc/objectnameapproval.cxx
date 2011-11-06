/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_OBJECTNAMEAPPROVAL_HXX
#include "objectnameapproval.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_TOOLS_XCONNECTIONTOOLS_HPP_
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::WeakReference;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::sdb::tools::XConnectionTools;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::sdbc::SQLException;
    /** === end UNO using === **/

    namespace CommandType = com::sun::star::sdb::CommandType;

    //====================================================================
    //= ObjectNameApproval_Impl
    //====================================================================
    struct ObjectNameApproval_Impl
    {
        WeakReference< XConnection >        aConnection;
        sal_Int32                           nCommandType;
    };

    //====================================================================
    //= ObjectNameApproval
    //====================================================================
    //--------------------------------------------------------------------
    ObjectNameApproval::ObjectNameApproval( const Reference< XConnection >& _rxConnection, ObjectType _eType )
        :m_pImpl( new ObjectNameApproval_Impl )
    {
        m_pImpl->aConnection = _rxConnection;
        m_pImpl->nCommandType = _eType == TypeQuery ? CommandType::QUERY : CommandType::TABLE;
    }

    //--------------------------------------------------------------------
    ObjectNameApproval::~ObjectNameApproval()
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL ObjectNameApproval::approveElement( const ::rtl::OUString& _rName, const Reference< XInterface >& /*_rxElement*/ )
    {
        Reference< XConnection > xConnection( m_pImpl->aConnection );
        if ( !xConnection.is() )
            throw DisposedException();

        Reference< XConnectionTools > xConnectionTools( xConnection, UNO_QUERY_THROW );
        Reference< XObjectNames > xObjectNames( xConnectionTools->getObjectNames(), UNO_QUERY_THROW );
        xObjectNames->checkNameForCreate( m_pImpl->nCommandType, _rName );
    }

//........................................................................
} // namespace dbaccess
//........................................................................

