/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pkgprovider.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:56:38 $
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

#ifndef _PKGPROVIDER_HXX
#define _PKGPROVIDER_HXX

#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif

#ifndef _PKGURI_HXX
#include "pkguri.hxx"
#endif

namespace com { namespace sun { namespace star { namespace container {
    class XHierarchicalNameAccess;
} } } }

namespace package_ucp {

//=========================================================================

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define PACKAGE_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.PackageContentProvider"
#define PACKAGE_CONTENT_PROVIDER_SERVICE_NAME_LENGTH    39

// UCB Content Type.
#define PACKAGE_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-folder"
#define PACKAGE_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-stream"
#define PACKAGE_ZIP_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-folder"
#define PACKAGE_ZIP_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-stream"

//=========================================================================

class Packages;

class ContentProvider : public ::ucb::ContentProviderImplHelper
{
    Packages* m_pPackages;

public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess >
    createPackage( const rtl::OUString & rName, const rtl::OUString & rParam );
    sal_Bool
    removePackage( const rtl::OUString & rName );
};

}

#endif
