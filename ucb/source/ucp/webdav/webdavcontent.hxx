/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webdavcontent.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 14:32:29 $
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

#ifndef _WEBDAV_UCP_CONTENT_HXX
#define _WEBDAV_UCP_CONTENT_HXX

#include <memory>
#include <list>

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_CONTENTCREATIONEXCEPTION_HPP_
#include <com/sun/star/ucb/ContentCreationException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

#ifndef _DAVRESOURCEACCESS_HXX_
#include "DAVResourceAccess.hxx"
#endif
#ifndef _WEBDAV_UCP_PROPERTYMAP_HXX
#include "PropertyMap.hxx"
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct OpenCommandArgument2;
    struct PostCommandArgument2;
    struct TransferInfo;
} } } }

namespace webdav_ucp
{

//=========================================================================

// UNO service name for the content.
#define WEBDAV_CONTENT_SERVICE_NAME "com.sun.star.ucb.WebDAVContent"

//=========================================================================

class ContentProvider;
class ContentProperties;

class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ResourceType
    {
        UNKNOWN,
        FTP,
        NON_DAV,
        DAV
    };

    std::auto_ptr< DAVResourceAccess > m_xResAccess;
    std::auto_ptr< ContentProperties > m_xCachedProps; // locally cached props
    rtl::OUString     m_aEscapedTitle;
    ResourceType      m_eResourceType;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
      sal_Bool        m_bTransient;
    sal_Bool          m_bCollection;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
      virtual ::rtl::OUString getParentURL();

      sal_Bool isFolder( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    void getProperties( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ucb::XCommandEnvironment >& xEnv,
                        PropertyMap & rProps );

      ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
      getPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                          const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues(
                     const ::com::sun::star::uno::Sequence<
                         ::com::sun::star::beans::PropertyValue >& rValues,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    typedef rtl::Reference< Content > ContentRef;
      typedef std::list< ContentRef > ContentRefList;
      void queryChildren( ContentRefList& rChildren);

      sal_Bool exchangeIdentity(
                const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    const rtl::OUString getBaseURI( const std::auto_ptr< DAVResourceAccess > & rResAccess );

    const ResourceType & getResourceType(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    const ResourceType & getResourceType(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv,
                    const std::auto_ptr< DAVResourceAccess > & rResAccess )
        throw ( ::com::sun::star::uno::Exception );

    // Command "open"
    com::sun::star::uno::Any open(
                const com::sun::star::ucb::OpenCommandArgument2 & rArg,
                const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    // Command "post"
    void post( const com::sun::star::ucb::PostCommandArgument2 & rArg,
               const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

      // Command "insert"
      void insert( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XInputStream > & xInputStream,
                   sal_Bool bReplaceExisting,
                   const com::sun::star::uno::Reference<
                      com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "transfer"
    void transfer( const ::com::sun::star::ucb::TransferInfo & rArgs,
                   const com::sun::star::uno::Reference<
                      com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "delete"
      void destroy( sal_Bool bDeletePhysical )
        throw( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Any MapDAVException( const DAVException & e,
                                                sal_Bool bWrite );
    void cancelCommandExecution(
                    const DAVException & e,
                    const ::com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    sal_Bool bWrite = sal_False )
        throw( ::com::sun::star::uno::Exception );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

public:
      Content( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                ContentProvider* pProvider,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory )
        throw ( ::com::sun::star::ucb::ContentCreationException );
      Content( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                ContentProvider* pProvider,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory,
                sal_Bool isCollection )
        throw ( ::com::sun::star::ucb::ContentCreationException );
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

      // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const rtl::OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeProperty( const rtl::OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException );

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

    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

      // Called from resultset data supplier.
      static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
      getPropertyValues( const ::com::sun::star::uno::Reference<
                              ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                              ::com::sun::star::beans::Property >& rProperties,
                          const ContentProperties& rData,
                       const rtl::Reference<
                           ::ucbhelper::ContentProviderImplHelper >& rProvider,
                          const ::rtl::OUString& rContentId );
};

}

#endif
