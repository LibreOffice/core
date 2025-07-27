/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ucbhelper/resultsethelper.hxx>
#include "dropbox_content.hxx"

namespace ucp::dropbox
{

class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
{
    rtl::Reference< Content > m_xContent;
    css::uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;

private:
    virtual void initStatic() override;
    virtual void initDynamic() override;

public:
    DynamicResultSet(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        rtl::Reference< Content > xContent,
        const css::ucb::OpenCommandArgument2& rCommand,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& rxEnv );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */