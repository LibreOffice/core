/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatecheckconfig.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-04 09:56:38 $
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

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

class UpdateCheckConfig : public ::cppu::WeakImplHelper3<
    ::com::sun::star::container::XNameReplace,
    ::com::sun::star::util::XChangesBatch,
    ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xUpdateAccess;

public:
    UpdateCheckConfig(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext);
    virtual ~UpdateCheckConfig();

    static ::com::sun::star::uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw (::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPendingChanges(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw (::com::sun::star::uno::RuntimeException);

     // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);
};

