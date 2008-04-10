/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleFamilies.hxx,v $
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
#ifndef CHART_STYLEFAMILIES_HXX
#define CHART_STYLEFAMILIES_HXX

#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>

#include "ServiceMacros.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

namespace chart
{

class StyleFamilies : public ::cppu::WeakImplHelper2 <
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo >
{
public:
    StyleFamilies();
    virtual ~StyleFamilies();

    /** adds a style family.

        @param rName the name under which the family can later be accessed
               through the XNameAccess.

        @param rFamily the style family object supporting the service
               ::com::sun::star::style::StyleFamily.

        @return true, if the family was added.  In case there already exists a
                family with the name given in rName, nothing is done and false
                is returned.
     */
    bool AddStyleFamily(
        const ::rtl::OUString & rName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess > & rFamily );

protected:
    // container::XNameAccess
    //
    // Note: The elements within this collection are StyleFamily instances.
    //       (only mandatory interface: XNameAccess)
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // container::XIndexAccess (optional)
    virtual sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // container::XElementAccess (base of XNameAccess and XIndexAccess)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

    APPHELPER_XSERVICEINFO_DECL()

    typedef ::std::map< ::rtl::OUString,
                        ::com::sun::star::uno::Reference<
                            ::com::sun::star::container::XNameAccess > >
        tStyleFamilyContainer;

private:
    tStyleFamilyContainer     m_aStyleFamilies;
    ::osl::Mutex              m_aMutex;
};

} //  namespace chart

// CHART_STYLEFAMILIES_HXX
#endif
