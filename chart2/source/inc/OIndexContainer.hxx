/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OIndexContainer.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART_OINDEXCONTAINER_HXX
#define CHART_OINDEXCONTAINER_HXX

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <vector>

namespace comphelper
{

class OIndexContainer :
        public ::com::sun::star::container::XIndexContainer
{
public:
    explicit OIndexContainer( ::osl::Mutex & rMutex );
    virtual ~OIndexContainer();

    ::osl::Mutex & GetMutex();

    // ____ XInterface ____
    virtual ::com::sun::star::uno::Any SAL_CALL
        queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// implemented as a standard-library vector
    typedef ::std::vector< ::com::sun::star::uno::Any > tHelperContainerType;

    /// allows direct standard-library access to the container
    tHelperContainerType      m_aContainer;

    // you have to implement this in derived classes
    // ____ XElementAccess ____
//     virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
//         throw (::com::sun::star::uno::RuntimeException) = 0;

    // implemented by this class
    // =========================

    // ____ XIndexContainer ____
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XIndexReplace ____
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XIndexAccess ____
    virtual sal_Int32 SAL_CALL getCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XElementAccess ____
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::osl::Mutex &  m_rMutex;
};

} //  namespace comphelper

// CHART_OINDEXCONTAINER_HXX
#endif
