/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#ifndef HEADER_PQ_ARRAY_HXX
#define HEADER_PQ_ARRAY_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/sdbc/XArray.hpp>

#include "pq_connection.hxx"

namespace pq_sdbc_driver
{

class Array : public cppu::WeakImplHelper1< com::sun::star::sdbc::XArray >
{
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_data;
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > m_owner;
    com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > m_tc;
    rtl::Reference< RefCountedMutex > m_refMutex;

public:
    Array(
        const rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & data,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & owner,
        const com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > &tc) :
        m_data( data ),
        m_owner( owner ),
        m_tc( tc ),
        m_refMutex( mutex )
    {}

public: // XArray

    // Methods
    virtual OUString SAL_CALL getBaseTypeName(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBaseType(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getArray(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getArrayAtIndex(
        sal_Int32 index,
        sal_Int32 count,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL
    getResultSet(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSetAtIndex(
        sal_Int32 index,
        sal_Int32 count,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

private:
    void checkRange( sal_Int32 index, sal_Int32 count );
};


};

#endif
