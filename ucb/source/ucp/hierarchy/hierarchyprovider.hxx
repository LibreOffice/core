/*************************************************************************
 *
 *  $RCSfile: hierarchyprovider.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 14:23:37 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef _HIERARCHYPROVIDER_HXX
#define _HIERARCHYPROVIDER_HXX

#include <hash_map>

#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace util {
        class XOfficeInstallationDirectories;
    }
} } }

namespace hierarchy_ucp {

//=========================================================================

#define HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.HierarchyContentProvider"
#define HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  41

#define HIERARCHY_URL_SCHEME \
                "vnd.sun.star.hier"
#define HIERARCHY_URL_SCHEME_LENGTH 17

#define HIERARCHY_FOLDER_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-folder"
#define HIERARCHY_LINK_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-link"

//=========================================================================

struct ConfigProviderMapEntry
{
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > xConfigProvider;
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > xRootReadAccess;
    bool bTriedToGetRootReadAccess;  // #82494#

    ConfigProviderMapEntry() : bTriedToGetRootReadAccess( false ) {}
};

struct equalString
{
    bool operator()(
        const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
    {
        return !!( rKey1 == rKey2 );
    }
};

struct hashString
{
    size_t operator()( const rtl::OUString & rName ) const
    {
        return rName.hashCode();
    }
};

typedef std::hash_map
<
    rtl::OUString,  // servcie specifier
    ConfigProviderMapEntry,
    hashString,
    equalString
>
ConfigProviderMap;

//=========================================================================

class HierarchyContentProvider : public ::ucb::ContentProviderImplHelper,
                                 public com::sun::star::lang::XInitialization
{
    ConfigProviderMap   m_aConfigProviderMap;
    com::sun::star::uno::Reference<
        com::sun::star::util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

public:
    HierarchyContentProvider(
                const com::sun::star::uno::Reference<
                    com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~HierarchyContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( com::sun::star::ucb::IllegalIdentifierException,
               com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL
    initialize( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::uno::Any >& aArguments )
        throw( ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException );

    // Non-Interface methods
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
    getConfigProvider( const rtl::OUString & rServiceSpecifier );
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getRootConfigReadNameAccess( const rtl::OUString & rServiceSpecifier );

    // Note: may retrun an empty reference.
    com::sun::star::uno::Reference<
        com::sun::star::util::XOfficeInstallationDirectories >
    getOfficeInstallationDirectories();
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYPROVIDER_HXX */
