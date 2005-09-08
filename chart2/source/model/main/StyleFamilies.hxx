/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleFamilies.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:05:23 $
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
#ifndef CHART_STYLEFAMILIES_HXX
#define CHART_STYLEFAMILIES_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

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
