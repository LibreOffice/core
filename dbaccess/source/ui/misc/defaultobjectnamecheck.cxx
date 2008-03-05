/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultobjectnamecheck.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:05:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_SOURCE_UI_MISC_DEFAULTOBJECTNAMECHECK_HXX
#include "defaultobjectnamecheck.hxx"
#endif

#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_TOOLS_XCONNECTIONTOOLS_HPP_
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#endif
/** === end UNO includes === **/

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::container::XHierarchicalNameAccess;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::sdb::tools::XConnectionTools;
    using ::com::sun::star::uno::UNO_QUERY;
    /** === end UNO using === **/

    using namespace dbtools;

    namespace CommandType = ::com::sun::star::sdb::CommandType;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        void    lcl_fillNameExistsError( const ::rtl::OUString& _rObjectName, SQLExceptionInfo& _out_rErrorToDisplay )
        {
            String sErrorMessage = String( ModuleRes( STR_NAMED_OBJECT_ALREADY_EXISTS ) );
            sErrorMessage.SearchAndReplaceAllAscii( "$#$", _rObjectName );
            SQLException aError;
            aError.Message = sErrorMessage;
            _out_rErrorToDisplay = aError;
        }

    }

    //====================================================================
    //= HierarchicalNameCheck_Impl
    //====================================================================
    struct HierarchicalNameCheck_Impl
    {
        Reference< XHierarchicalNameAccess >    xHierarchicalNames;
        ::rtl::OUString                         sRelativeRoot;
    };

    //====================================================================
    //= HierarchicalNameCheck
    //====================================================================
    //--------------------------------------------------------------------
    HierarchicalNameCheck::HierarchicalNameCheck( const Reference< XHierarchicalNameAccess >& _rxNames, const ::rtl::OUString& _rRelativeRoot )
        :m_pImpl( new HierarchicalNameCheck_Impl )
    {
        m_pImpl->xHierarchicalNames = _rxNames;
        m_pImpl->sRelativeRoot = _rRelativeRoot;

        if ( !m_pImpl->xHierarchicalNames.is() )
            throw IllegalArgumentException();
    }

    //--------------------------------------------------------------------
    HierarchicalNameCheck::~HierarchicalNameCheck()
    {
    }

    //--------------------------------------------------------------------
    bool HierarchicalNameCheck::isNameValid( const ::rtl::OUString& _rObjectName, SQLExceptionInfo& _out_rErrorToDisplay ) const
    {
        try
        {
            ::rtl::OUStringBuffer aCompleteName;
            if ( m_pImpl->sRelativeRoot.getLength() )
            {
                aCompleteName.append( m_pImpl->sRelativeRoot );
                aCompleteName.appendAscii( "/" );
            }
            aCompleteName.append( _rObjectName );

            ::rtl::OUString sCompleteName( aCompleteName.makeStringAndClear() );
            if ( !m_pImpl->xHierarchicalNames->hasByHierarchicalName( sCompleteName ) )
                return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        lcl_fillNameExistsError( _rObjectName, _out_rErrorToDisplay );
        return false;
    }

    //====================================================================
    //= DynamicTableOrQueryNameCheck_Impl
    //====================================================================
    struct DynamicTableOrQueryNameCheck_Impl
    {
        sal_Int32                   nCommandType;
        Reference< XObjectNames >   xObjectNames;
    };

    //====================================================================
    //= DynamicTableOrQueryNameCheck
    //====================================================================
    //--------------------------------------------------------------------
    DynamicTableOrQueryNameCheck::DynamicTableOrQueryNameCheck( const Reference< XConnection >& _rxSdbLevelConnection, sal_Int32 _nCommandType )
        :m_pImpl( new DynamicTableOrQueryNameCheck_Impl )
    {
        Reference< XConnectionTools > xConnTools( _rxSdbLevelConnection, UNO_QUERY );
        if ( xConnTools.is() )
            m_pImpl->xObjectNames.set( xConnTools->getObjectNames() );
        if ( !m_pImpl->xObjectNames.is() )
            throw IllegalArgumentException();

        if ( ( _nCommandType != CommandType::QUERY ) && ( _nCommandType != CommandType::TABLE ) )
            throw IllegalArgumentException();
        m_pImpl->nCommandType = _nCommandType;
    }

    //--------------------------------------------------------------------
    DynamicTableOrQueryNameCheck::~DynamicTableOrQueryNameCheck()
    {
    }

    //--------------------------------------------------------------------
    bool DynamicTableOrQueryNameCheck::isNameValid( const ::rtl::OUString& _rObjectName, ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay ) const
    {
        try
        {
            m_pImpl->xObjectNames->checkNameForCreate( m_pImpl->nCommandType, _rObjectName );
            return true;
        }
        catch( const SQLException& )
        {
            _out_rErrorToDisplay = ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() );
        }
        return false;
    }

//........................................................................
} // namespace dbaui
//........................................................................

