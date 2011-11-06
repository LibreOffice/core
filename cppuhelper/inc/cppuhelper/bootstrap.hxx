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


#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#define _CPPUHELPER_BOOTSTRAP_HXX_

#include "sal/config.h"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace container { class XHierarchicalNameAccess; }
    namespace registry { class XSimpleRegistry; }
    namespace uno { class XComponentContext; }
} } }

namespace cppu
{

/** Creates a simple registry service instance.

    @rBootstrapPath optional bootstrap path for initial components
    @return simple registry service instance
*/
::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry >
SAL_CALL createSimpleRegistry(
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( () );

/** Creates a nested registry service instance.

    @rBootstrapPath optional bootstrap path for initial components
    @return nested registry service instance
*/
::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry >
SAL_CALL createNestedRegistry(
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( () );

/** Installs type description manager instance, i.e. registers a callback at cppu core.

    @param xTDMgr manager instance
    @return true, if successfully registered
*/
sal_Bool SAL_CALL installTypeDescriptionManager(
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess > const & xTDMgr )
    SAL_THROW( () );

/** Bootstraps an initial component context with service manager upon a given registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    @param xRegistry registry for service manager and singleton objects of context (may be null)
    @param rBootstrapPath optional bootstrap path for initial components
    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
bootstrap_InitialComponentContext(
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry > const & xRegistry,
    ::rtl::OUString const & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( (::com::sun::star::uno::Exception) );


/** Bootstraps an initial component context with service manager upon default types and
    services registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs
      - UNO_WRITERDB      -- a file url of a write rdb (e.g. user.rdb)

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext() SAL_THROW( (::com::sun::star::uno::Exception) );


/** Bootstraps an initial component context with service manager upon default types and
    services registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs
      - UNO_WRITERDB      -- a file url of a write rdb (e.g. user.rdb)

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @param iniFile ini filename to get bootstrap variables
    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext(const ::rtl::OUString & iniFile) SAL_THROW( (::com::sun::star::uno::Exception) );

/**
 * An exception indicating a bootstrap error.
 *
 * @since UDK 3.2.0
 */
class BootstrapException
{
public:
    /**
     * Constructs a BootstrapException.
     */
    BootstrapException();

    /**
     * Constructs a BootstrapException with the specified detail message.
     *
     * @param rMessage
     * A message containing any details about the exception.
     */
    BootstrapException( const ::rtl::OUString & rMessage );

    /**
     * Copy constructs a BootstrapException.
     */
    BootstrapException( const BootstrapException & e );

    /**
     * Destructs a BootstrapException.
     */
    virtual ~BootstrapException();

    /**
     * Assigns a BootstrapException.
     */
    BootstrapException & operator=( const BootstrapException & e );

    /** Gets the message.

        @return
        A reference to the message. The reference is valid for the lifetime of
        this BootstrapException.
     */
    const ::rtl::OUString & getMessage() const;

private:
    ::rtl::OUString m_aMessage;
};

/**
 * Bootstraps the component context from a UNO installation.
 *
 * @return a bootstrapped component context
 * @exception BootstrapException
 * Thrown in case bootstrap() signals an exception due to a
 * bootstrap error.
 *
 * @since UDK 3.2.0
 */
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
SAL_CALL bootstrap();

/**
 * Helper function to expand vnd.sun.star.expand URLs in contexts where no
 * properly bootstrapped UNO is (yet) available.
 *
 * @internal
 *
 * @param uri
 * Some URI (but not a URI reference).
 *
 * @return
 * If uri is a vnd.sun.star.expand URL, then the expansion of that URL is
 * returned; expansion may lead to a string that is not a legal URI. Otherwise,
 * the uri is returned unchanged.
 *
 * @exception com::sun::star::lang::IllegalArgumentException
 * If uri is a vnd.sun.star.expand URL that contains unknown macros.
 *
 * @since UDK 3.2.8
 */
::rtl::OUString
SAL_CALL bootstrap_expandUri(::rtl::OUString const & uri);

} // end namespace cppu

#endif
