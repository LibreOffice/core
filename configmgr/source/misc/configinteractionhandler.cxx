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
