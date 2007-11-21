/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablename.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:47:38 $
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

#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX
#define DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#include "connectiondependent.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDB_TOOLS_XTABLENAME_HPP_
#include <com/sun/star/sdb/tools/XTableName.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <memory>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= TableName
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XTableName
                                    >   TableName_Base;
    struct TableName_Impl;
    /** default implementation for XTableName
    */
    class TableName :public TableName_Base
                    ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< TableName_Impl >   m_pImpl;

    public:
        /** constructs the instance

            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.

            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        TableName(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XTableName
        virtual ::rtl::OUString SAL_CALL getCatalogName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCatalogName( const ::rtl::OUString& _catalogname ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSchemaName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSchemaName( const ::rtl::OUString& _schemaname ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTableName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTableName( const ::rtl::OUString& _tablename ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getNameForSelect() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getTable() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTable( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _table ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getComposedName( ::sal_Int32 Type, ::sal_Bool _Quote ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setComposedName( const ::rtl::OUString& ComposedName, ::sal_Int32 Type ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~TableName();

    private:
        TableName();                                // never implemented
        TableName( const TableName& );              // never implemented
        TableName& operator=( const TableName& );   // never implemented
    };

//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

