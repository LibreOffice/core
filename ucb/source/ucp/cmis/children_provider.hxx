/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CHILDREN_PROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CHILDREN_PROVIDER_HXX

#include <list>

#include <com/sun/star/ucb/XContent.hpp>

namespace cmis
{
    class ChildrenProvider
    {
        public:
            virtual ~ChildrenProvider( ) { };

            virtual std::list< css::uno::Reference< css::ucb::XContent > > getChildren( ) = 0;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
