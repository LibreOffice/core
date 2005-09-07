/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OStyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:44:44 $
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
#ifndef CHART_OSTYLE_HXX
#define CHART_OSTYLE_HXX

#include "OPropertySet.hxx"

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

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

