/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configinteractionhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-01-24 16:43:00 $
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

#ifndef CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX
#define CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include "uno/current_context.hxx"
#endif

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
