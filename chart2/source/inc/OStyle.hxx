/*************************************************************************
 *
 *  $RCSfile: OStyle.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

