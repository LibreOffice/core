/*************************************************************************
 *
 *  $RCSfile: pkgcontent.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kso $ $Date: 2001-02-22 10:57:52 $
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

#ifndef _PKGCONTENT_HXX
#define _PKGCONTENT_HXX

#include <list>

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

#ifndef _PKGURI_HXX
#include "pkguri.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace beans
    {
        struct Property;
        struct PropertyValue;
    }
    namespace container
    {
        class XHierarchicalNameAccess;
        class XEnumeration;
    }
    namespace io
    {
        class XInputStream;
    }
    namespace sdbc
    {
        class XRow;
    }
    namespace ucb
    {
        struct OpenCommandArgument2;
        struct TransferInfo;
    }
} } }

namespace package_ucp
{

//=========================================================================

// UNO service name for the content.
#define PACKAGE_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.PackageFolderContent"
#define PACKAGE_STREAM_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.PackageStreamContent"

//=========================================================================

struct ContentProperties
{
    ::rtl::OUString  aTitle;       // Title
    ::rtl::OUString  aContentType; // ContentType
    sal_Bool         bIsDocument;  // IsDocument
    sal_Bool         bIsFolder;    // IsFolder
    ::rtl::OUString  aMediaType;   // MediaType
    sal_Int64        nSize;        // Size
#if SUPD>616
    sal_Bool         bCompressed;  // Compressed
#endif

#if SUPD>616
    ContentProperties()
    : bIsDocument( sal_True ), bIsFolder( sal_False ), nSize( 0 ),
      bCompressed( sal_True ) {}
#else
    ContentProperties()
    : bIsDocument( sal_True ), bIsFolder( sal_False ), nSize( 0 ) {}
#endif

    ContentProperties( const ::rtl::OUString& rContentType );
};

//=========================================================================

class ContentProvider;

class Content : public ::ucb::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ContentState { TRANSIENT,  // created via CreateNewContent,
                                       // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete"
                      };

    PackageUri              m_aUri;
    ContentProperties       m_aProps;
    ContentState            m_eState;
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >    m_xPackage;
    ContentProvider*        m_pProvider;
    sal_uInt32              m_nModifiedProps;

private:
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const ::com::sun::star::uno::Reference<
                 com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const ContentProperties& rProps );
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const com::sun::star::uno::Reference<
                 com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const com::sun::star::ucb::ContentInfo& Info );

    virtual const ::ucb::PropertyInfoTableEntry& getPropertyInfoTable();
    virtual const ::ucb::CommandInfoTableEntry&  getCommandInfoTable();
    virtual ::rtl::OUString getParentURL();

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const ::vos::ORef< ::ucb::ContentProviderImplHelper >&
                               rProvider,
                       const ::rtl::OUString& rContentId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                             ::com::sun::star::beans::Property >& rProperties );
    void setPropertyValues(
            const ::com::sun::star::uno::Sequence<
                     ::com::sun::star::beans::PropertyValue >& rValues );

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage( const PackageUri& rURI );
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage();

    static sal_Bool
    loadData( ContentProvider* pProvider,
              const PackageUri& rURI,
              ContentProperties& rProps,
              com::sun::star::uno::Reference<
                  com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );
    static sal_Bool
    hasData( ContentProvider* pProvider,
             const PackageUri& rURI,
             com::sun::star::uno::Reference<
                 com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );
    sal_Bool
    hasData( const PackageUri& rURI );
    sal_Bool
    renameData( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XContentIdentifier >& xOldId,
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& xNewId );
    sal_Bool
    storeData( const com::sun::star::uno::Reference<
                    com::sun::star::io::XInputStream >& xStream );
    sal_Bool
    removeData();

    sal_Bool
    flushData();

    typedef vos::ORef< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    sal_Bool
    exchangeIdentity( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Any
    open( const ::com::sun::star::ucb::OpenCommandArgument2& rArg,
          const ::com::sun::star::uno::Reference<
                      ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    void insert( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::io::XInputStream >& xStream,
                 sal_Int32 nNameClashResolve )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    void destroy( sal_Bool bDeletePhysical )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    void transfer( const ::com::sun::star::ucb::TransferInfo& rInfo )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    getInputStream();

    sal_Bool isFolder() const { return m_aProps.bIsFolder; }

public:
    // Create existing content. Fail, if not already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~Content();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContent
    virtual rtl::OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    // XContentCreator
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );

    // Called from resultset data supplier.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XEnumeration >
    getIterator();
};

}

#endif
