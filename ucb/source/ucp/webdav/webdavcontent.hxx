/*************************************************************************
 *
 *  $RCSfile: webdavcontent.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-07 15:49:00 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _WEBDAV_UCP_CONTENT_HXX
#define _WEBDAV_UCP_CONTENT_HXX

#ifndef _NEONTYPES_HXX_
#include "NeonTypes.hxx"
#endif
#ifndef _DAVSESSION_HXX_
#include "DAVSession.hxx"
#endif

#ifndef __VECTOR__
#include <stl/vector>
#endif
#ifndef __LIST__
#include <stl/list>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace webdav_ucp
{

//=========================================================================

// UNO service name for the content.
#define WEBDAV_CONTENT_SERVICE_NAME "com.sun.star.ucb.WebDAVContent"

//=========================================================================

class DAVResource;
class ContentCreationException : public ::com::sun::star::uno::Exception
{};

struct ContentProperties
{
  ::rtl::OUString aTitle;           // Title
  ::rtl::OUString getcontenttype;   // ContentType
  sal_Bool        bIsDocument;      // IsDocument
  sal_Bool        bIsFolder;        // IsFolder
  sal_Int64       size;             // Size
  ::com::sun::star::util::DateTime dateCreated;     // DateCreated
  ::com::sun::star::util::DateTime dateModified;    // DateModified

  ::rtl::OUString creationdate;
  ::rtl::OUString displayname;
  ::rtl::OUString getcontentlanguage;
  ::rtl::OUString getcontentlength;
  ::rtl::OUString getetag;
  ::rtl::OUString getlastmodified;
  //@@@   ::com::sun::star::uno::Sequence<::com::sun::star::dav::lock> lockdiscovery;
  ::rtl::OUString lockdiscovery;
  ::rtl::OUString resourcetype;
  //@@@   ::com::sun::star::uno::Any source;
  ::rtl::OUString source;
  //@@@   ::com::sun::star::uno::Sequence<::com::sun::star::dav::lock> supportedlock;
  ::rtl::OUString supportedlock;

  ContentProperties()
    : bIsDocument( sal_False ), bIsFolder( sal_True ) {}

  void setValues(DAVResource& res);
};

//=========================================================================

class Content : public ::ucb::ContentImplHelper,
        public com::sun::star::ucb::XContentCreator
{
  ContentProperties m_aProps;
  ::vos::ORef< DAVSession > _pWebdavSession;
  ::rtl::OUString _path;
  sal_Bool _transient;
  sal_Bool _upToDate;

private:

  sal_Bool update( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                     const ::com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& Environment);
  void initpath()
    throw ( ContentCreationException );


  virtual const ::ucb::PropertyInfoTableEntry& getPropertyInfoTable();
  virtual const ::ucb::CommandInfoTableEntry&  getCommandInfoTable();
  virtual ::rtl::OUString getParentURL();

  sal_Bool isFolder() const { return ( m_aProps.bIsFolder ); }
  void setFolder() {
    m_aProps.bIsFolder = sal_True;
    m_aProps.bIsDocument = sal_True;
  }

  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
  getPropertyValues( const ::com::sun::star::uno::Sequence<
                         ::com::sun::star::beans::Property >& rProperties,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
  void setPropertyValues(
             const ::com::sun::star::uno::Sequence<
             ::com::sun::star::beans::PropertyValue >& rValues,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XCommandEnvironment >& Environment );

  typedef vos::ORef< Content > ContentRef;
  typedef std::list< ContentRef > ContentRefList;
  void queryChildren( ContentRefList& rChildren);

  sal_Bool exchangeIdentity(
                const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

  // Command "insert"
  void insert(com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
          sal_Bool bReplaceExisting,
          const com::sun::star::uno::Reference<
          com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( ::com::sun::star::ucb::CommandAbortedException );

  // Command "delete"
  void destroy( sal_Bool bDeletePhysical )
    throw( ::com::sun::star::ucb::CommandAbortedException );

public:
  Content( const ::com::sun::star::uno::Reference<
       ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
       ::ucb::ContentProviderImplHelper* pProvider,
       const ::com::sun::star::uno::Reference<
       ::com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw ( ContentCreationException );
  Content( const ::com::sun::star::uno::Reference<
       ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
       ::ucb::ContentProviderImplHelper* pProvider,
       const ::com::sun::star::uno::Reference<
       ::com::sun::star::ucb::XContentIdentifier >& Identifier,
       ::vos::ORef< DAVSession> pSession,
       sal_Bool isCollection)
    throw ( ContentCreationException );
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

  // Add additional interfaces ( like com::sun:.star::ucb::XContentCreator ).
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

  ::vos::ORef< DAVSession > getSession() { return _pWebdavSession; }
  const ::rtl::OUString& getPath() const { return _path; }

  // Called from resultset data supplier.
  static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
  getPropertyValues( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                     const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                        const ContentProperties& rData,
                        const ::vos::ORef<
                         ::ucb::ContentProviderImplHelper >&    rProvider,
                        const ::rtl::OUString& rContentId );
};

};

#endif
