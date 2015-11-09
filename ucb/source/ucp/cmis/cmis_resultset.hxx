/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_RESULTSET_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_RESULTSET_HXX

#include <ucbhelper/resultsethelper.hxx>

#include "children_provider.hxx"

namespace cmis
{

    class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
    {
        ChildrenProvider* m_pChildrenProvider;
        css::uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;

        private:
            virtual void initStatic() override;
            virtual void initDynamic() override;

        public:

            DynamicResultSet(
                    const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                    ChildrenProvider* pChildrenProvider,
                    const css::ucb::OpenCommandArgument2& rCommand,
                    const css::uno::Reference< css::ucb::XCommandEnvironment >& rxEnv );

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
