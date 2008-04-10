/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OStyle.hxx,v $
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
#ifndef CHART_OSTYLE_HXX
#define CHART_OSTYLE_HXX

#include "OPropertySet.hxx"
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace property
{

class OStyle :
        public OPropertySet,
        public ::com::sun::star::style::XStyle
{
public:
    OStyle( const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess > &xStyleFamily,
            ::osl::Mutex & rMutex );
    virtual ~OStyle();

    ::osl::Mutex & GetMutex();

//  inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () throw ()
//     { return static_cast< ::cppu::OWeakObject * >( this ); }

protected:

    // ____ XStyle ____
    virtual sal_Bool SAL_CALL isUserDefined()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isInUse()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getParentStyle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParentStyle( const ::rtl::OUString& aParentStyle )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XNamed (base of XStyle) ____
    virtual ::rtl::OUString SAL_CALL getName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    // necessary overloads from OPropertySet

    // ____ XInterface ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
//     virtual void SAL_CALL acquire() throw ();
//     virtual void SAL_CALL release() throw ();

    // ____ XServiceInfo ____
//     virtual ::rtl::OUString SAL_CALL
//         getImplementationName()
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual sal_Bool SAL_CALL
//         supportsService( const ::rtl::OUString& ServiceName )
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
//         getSupportedServiceNames()
//         throw (::com::sun::star::uno::RuntimeException);

    // ____ XTypeProvider ____
//     virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
//         getTypes()
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
//         getImplementationId()
//         throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > m_xStyleFamily;

    ::rtl::OUString    m_aName;
    ::rtl::OUString    m_aParentStyleName;
    sal_Bool           m_bUserDefined;

    ::osl::Mutex &     m_rMutex;
};

} //  namespace property

// CHART_OSTYLE_HXX
#endif

