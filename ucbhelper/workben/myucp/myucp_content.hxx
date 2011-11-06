/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// @@@ Adjust multi-include-protection-ifdef.
#ifndef _MYUCP_CONTENT_HXX
#define _MYUCP_CONTENT_HXX

#include <list>

#include "rtl/ref.hxx"
#include "ucbhelper/contenthelper.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }

// @@@ Adjust namespace name.
namespace myucp
{

//=========================================================================

// @@@ Adjust service name.

// UNO service name for the content. Prefix with reversed company domain main.
#define MYUCP_CONTENT_SERVICE_NAME "com.sun.star.ucb.MyContent"

//=========================================================================

struct ContentProperties
{
    ::rtl::OUString aTitle;         // Title
    ::rtl::OUString aContentType;   // ContentType
    sal_Bool        bIsDocument;    // IsDocument
    sal_Bool        bIsFolder;      // IsFolder

    // @@@ Add other properties supported by your content.

    ContentProperties()
    : bIsDocument( sal_True ), bIsFolder( sal_False ) {}
};

//=========================================================================

class Content : public ::ucbhelper::ContentImplHelper
{
    ContentProperties m_aProps;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv );

#define IMPLEMENT_COMMAND_OPEN
#define IMPLEMENT_COMMAND_INSERT
#define IMPLEMENT_COMMAND_DELETE

#ifdef IMPLEMENT_COMMAND_INSERT
    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    // Command "insert"
    void insert( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::io::XInputStream > & xInputStream,
                 sal_Bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );
#endif

#ifdef IMPLEMENT_COMMAND_DELETE

    // Command "delete"
    void destroy( sal_Bool bDeletePhysical )
        throw( ::com::sun::star::uno::Exception );
#endif

public:
    Content( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ::ucbhelper::ContentProviderImplHelper* pProvider,
             const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContentIdentifier >& Identifier );
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

    // @@@ Add additional interfaces ( like com::sun::star::ucb::XContentCreator ).

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

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
