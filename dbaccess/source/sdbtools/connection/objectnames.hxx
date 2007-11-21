/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectnames.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:47:01 $
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

#ifndef DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX
#define DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX

#include "connectiondependent.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/tools/XObjectNames.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= ObjectNames
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XObjectNames
                                    >   ObjectNames_Base;
    struct ObjectNames_Impl;
    /** default implementation for XObjectNames
    */
    class ObjectNames   :public ObjectNames_Base
                        ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< ObjectNames_Impl >   m_pImpl;

    public:
        /** constructs the instance

            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.

            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        ObjectNames(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XObjectNames
        virtual ::rtl::OUString SAL_CALL suggestName( ::sal_Int32 CommandType, const ::rtl::OUString& BaseName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL convertToSQLName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isNameUsed( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isNameValid( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL checkNameForCreate( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ObjectNames();

    private:
        ObjectNames();                                  // never implemented
        ObjectNames( const ObjectNames& );              // never implemented
        ObjectNames& operator=( const ObjectNames& );   // never implemented
    };

//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX

