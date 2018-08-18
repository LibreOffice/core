/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CPPUHELPER_BOOTSTRAP_HXX
#define INCLUDED_CPPUHELPER_BOOTSTRAP_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "cppuhelper/cppuhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace container { class XHierarchicalNameAccess; }
    namespace uno { class XComponentContext; }
} } }

namespace cppu
{

/** Installs type description manager instance, i.e. registers a callback at cppu core.

    @param xTDMgr manager instance
    @return true, if successfully registered
*/
CPPUHELPER_DLLPUBLIC sal_Bool SAL_CALL installTypeDescriptionManager(
    css::uno::Reference< css::container::XHierarchicalNameAccess > const & xTDMgr );

/** Bootstraps an initial component context with service manager upon
    information from bootstrap variables.

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @return component context
*/
CPPUHELPER_DLLPUBLIC css::uno::Reference< css::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext();


/** Bootstraps an initial component context with service manager upon
    information from an ini file.

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @param iniFile ini filename to get bootstrap variables
    @return component context
*/
CPPUHELPER_DLLPUBLIC css::uno::Reference< css::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext(const ::rtl::OUString & iniFile);

/**
 * An exception indicating a bootstrap error.
 *
 * @since UDK 3.2.0
 */
class CPPUHELPER_DLLPUBLIC BootstrapException
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
CPPUHELPER_DLLPUBLIC css::uno::Reference< css::uno::XComponentContext >
SAL_CALL bootstrap();

/// @cond INTERNAL
/**
 * Helper function to expand vnd.sun.star.expand URLs in contexts where no
 * properly bootstrapped UNO is (yet) available.
 *
 * @param uri
 * Some URI (but not a URI reference).
 *
 * @return
 * If uri is a vnd.sun.star.expand URL, then the expansion of that URL is
 * returned; expansion may lead to a string that is not a legal URI. Otherwise,
 * the uri is returned unchanged.
 *
 * @exception css::lang::IllegalArgumentException
 * If uri is a vnd.sun.star.expand URL that contains unknown macros.
 *
 * @since UDK 3.2.8
 */
CPPUHELPER_DLLPUBLIC ::rtl::OUString
SAL_CALL bootstrap_expandUri(::rtl::OUString const & uri);
/// @endcond

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
