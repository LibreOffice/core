/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zipfileaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:49:05 $
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

#ifndef _ZIPFILEACCESS_HXX_
#define _ZIPFILEACCESS_HXX_

#ifndef _COM_SUN_STAR_PACKAGES_ZIP_XZIPFILEACCESS_HPP_
#include <com/sun/star/packages/zip/XZipFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase5.hxx>
#include <osl/mutex.hxx>

#include <ZipFile.hxx>
#include <HashMaps.hxx>

class OZipFileAccess : public ::cppu::WeakImplHelper5<
                        ::com::sun::star::packages::zip::XZipFileAccess,
                        ::com::sun::star::container::XNameAccess,
                        ::com::sun::star::lang::XInitialization,
                        ::com::sun::star::lang::XComponent,
                        ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xContentStream;
    ZipFile* m_pZipFile;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer;

    sal_Bool m_bDisposed;

public:
    OZipFileAccess( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    virtual ~OZipFileAccess();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPatternsFromString_Impl( const ::rtl::OUString& aString );

    sal_Bool StringGoodForPattern_Impl( const ::rtl::OUString& aString,
                                        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPattern );


    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    // XZipFileAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getStreamByPattern( const ::rtl::OUString& aPattern ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

#endif

