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

#ifndef INCLUDED_UCB_SOURCE_UCP_GIO_GIO_RESULTSET_HXX
#define INCLUDED_UCB_SOURCE_UCP_GIO_GIO_RESULTSET_HXX

#include <ucbhelper/resultsethelper.hxx>
#include "gio_content.hxx"

namespace gio
{

    class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
    {
        css::uno::Reference< Content > m_xContent;
        css::uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;

    private:
       virtual void initStatic() override;
        virtual void initDynamic() override;

    public:
        DynamicResultSet(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Reference< Content >& rxContent,
            const css::ucb::OpenCommandArgument2& rCommand,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& rxEnv );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
