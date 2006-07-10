/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectnameapproval.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:17:07 $
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

