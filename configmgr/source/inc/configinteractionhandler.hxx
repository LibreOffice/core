/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX
#define CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"
#include "uno/current_context.hxx"

namespace com { namespace sun { namespace star {
    namespace task { class XInteractionHandler; }
    namespace uno { class Any; }
} } }
namespace rtl { class OUString; }

namespace configmgr { namespace apihelper {

/**
   represents the InteractionHandler for configuration errors from the current
   context.

   <p>Should only be kept in scope while the error is being handled.</p>
*/
class ConfigurationInteractionHandler {
public:
    ConfigurationInteractionHandler();

    ~ConfigurationInteractionHandler();

    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >
    get() const; // throw (com::sun::star::uno::RuntimeException)

    void setRecursive(
        com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionHandler > const & handler);

private:
    ConfigurationInteractionHandler(ConfigurationInteractionHandler &);
        // not defined
    void operator =(ConfigurationInteractionHandler &); // not defined

    com::sun::star::uno::Any getPreviousContextValue(
        rtl::OUString const & name) const;
        // throw (com::sun::star::uno::RuntimeException)

    class Context;
    friend class Context;

    rtl::Reference< Context > m_context;
    com::sun::star::uno::ContextLayer m_layer;
};

} }

#endif
