/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configinteractionhandler.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 15:14:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "sal/config.h"

#include "configinteractionhandler.hxx"

#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "uno/current_context.hxx"

namespace {

namespace css = com::sun::star;

static char const INTERACTION_HANDLER[] = "configuration.interaction-handler";

}

namespace configmgr { namespace apihelper {

class ConfigurationInteractionHandler::Context:
    public cppu::WeakImplHelper1< css::uno::XCurrentContext >
{
public:
    explicit Context(ConfigurationInteractionHandler * parent):
        m_parent(parent) {}

    virtual css::uno::Any SAL_CALL getValueByName(rtl::OUString const & name)
        throw (css::uno::RuntimeException)
    {
        return
            name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(INTERACTION_HANDLER))
            ? m_handler : m_parent->getPreviousContextValue(name);
    }

    void setInteractionHandler(
        css::uno::Reference< css::task::XInteractionHandler > const & handler)
    { m_handler <<= handler; }

private:
    Context(Context &); // not defined
    void operator =(Context &); // not defined

    virtual ~Context() {}

    ConfigurationInteractionHandler * m_parent;
    css::uno::Any m_handler;
};

ConfigurationInteractionHandler::ConfigurationInteractionHandler():
    m_context(new Context(this)), m_layer(m_context.get()) {}

ConfigurationInteractionHandler::~ConfigurationInteractionHandler() {}

css::uno::Reference< css::task::XInteractionHandler >
ConfigurationInteractionHandler::get() const {
    return css::uno::Reference< css::task::XInteractionHandler >(
        getPreviousContextValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(INTERACTION_HANDLER))),
        css::uno::UNO_QUERY);
}

void ConfigurationInteractionHandler::setRecursive(
    css::uno::Reference< css::task::XInteractionHandler > const & handler)
{
    m_context->setInteractionHandler(handler);
}

css::uno::Any ConfigurationInteractionHandler::getPreviousContextValue(
    rtl::OUString const & name) const
{
    css::uno::Reference< css::uno::XCurrentContext > c(
        m_layer.getPreviousContext());
    return c.is() ? c->getValueByName(name) : css::uno::Any();
}

} }
