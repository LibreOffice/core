/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
