/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultobjectnamecheck.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:31:54 $
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

#ifndef DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX

#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#include "objectnamecheck.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
/** === end UNO includes === **/

#include <memory>
#include <boost/noncopyable.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= HierarchicalNameCheck
    //====================================================================
    struct HierarchicalNameCheck_Impl;
    /** class implementing the IObjectNameCheck interface, and checking given object names
        against a hierarchical name container
    */
    class HierarchicalNameCheck :public ::boost::noncopyable
                                ,public IObjectNameCheck
    {
    private:
        std::auto_ptr< HierarchicalNameCheck_Impl > m_pImpl;

    public:
        /** constructs a HierarchicalNameCheck instance
        @param _rxNames
            the hierarchic container of named objects, against which given names should be
            checked
        @param _rRelativeRoot
            the root in the hierarchy against which given names should be checked
        @throws ::com::sun::star::lang::IllegalArgumentException
            if the given container is <NULL/>
        */
        HierarchicalNameCheck(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& _rxNames,
            const ::rtl::OUString& _rRelativeRoot
        );

        ~HierarchicalNameCheck();

        // IObjectNameCheck overridables
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const;

    private:
        HierarchicalNameCheck();                                            // never implemented
    };

    //====================================================================
    //= DynamicTableOrQueryNameCheck
    //====================================================================
    struct DynamicTableOrQueryNameCheck_Impl;
    /** class implementing the IObjectNameCheck interface, and checking a given name
        for being valid as either a query or a table name.

        The class can be parametrized to act as either table name or query name validator.

        For databases which support queries in queries, the name check is implicitly extended
        to both queries and tables, no matter which category is checked. This prevents, for
        such databases, that users can create a query with the name of an existing table,
        or vice versa.

        @seealso dbtools::DatabaseMetaData::supportsSubqueriesInFrom
        @seealso com::sun::star::sdb::tools::XObjectNames::checkNameForCreate
    */
    class DynamicTableOrQueryNameCheck  :public ::boost::noncopyable
                                        ,public IObjectNameCheck
    {
    private:
        std::auto_ptr< DynamicTableOrQueryNameCheck_Impl > m_pImpl;

    public:
        /** constructs a DynamicTableOrQueryNameCheck instance
        @param _rxSdbLevelConnection
            a connection supporting the css.sdb.Connection service, in other word, it
            does expose the XTablesSupplier and XQueriesSupplier interfaces.
        @param _nCommandType
            specifies whether table names or query names should be checked. Only valid values
            are CommandType::TABLE and CommandType::QUERY.
        @throws ::com::sun::star::lang::IllegalArgumentException
            if the given connection is <NULL/>, or the given command type is neither
            CommandType::TABLE nor CommandType::QUERY.
        */
        DynamicTableOrQueryNameCheck(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxSdbLevelConnection,
            sal_Int32 _nCommandType
        );

        ~DynamicTableOrQueryNameCheck();

        // IObjectNameCheck overridables
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const;

    private:
        DynamicTableOrQueryNameCheck();                                                // never implemented
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX

