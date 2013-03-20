/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#if 1

#include <ucbhelper/resultsethelper.hxx>

#include "children_provider.hxx"

namespace cmis
{

    class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
    {
        ChildrenProvider* m_pChildrenProvider;
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xEnv;

        private:
            virtual void initStatic();
            virtual void initDynamic();

        public:

            DynamicResultSet(
                    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                    ChildrenProvider* pChildrenProvider,
                    const com::sun::star::ucb::OpenCommandArgument2& rCommand,
                    const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& rxEnv );

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
