/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <ucbhelper/resultset.hxx>

#include "children_provider.hxx"

namespace cmis
{

    struct ResultListEntry
    {
        css::uno::Reference< css::ucb::XContent > xContent;
        css::uno::Reference< css::sdbc::XRow > xRow;

        explicit ResultListEntry( css::uno::Reference< css::ucb::XContent > xCnt ) : xContent( std::move(xCnt) )
        {
        }
    };

    class DataSupplier : public ucbhelper::ResultSetDataSupplier
    {
        private:
            ChildrenProvider* m_pChildrenProvider;
            sal_Int32 mnOpenMode;
            bool mbCountFinal;
            void getData();
            std::vector< ResultListEntry > maResults;

        public:
            DataSupplier( ChildrenProvider* pChildrenProvider, sal_Int32 nOpenMode );

            virtual ~DataSupplier() override;

            virtual OUString queryContentIdentifierString( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
            virtual css::uno::Reference< css::ucb::XContentIdentifier >
                queryContentIdentifier( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
            virtual css::uno::Reference< css::ucb::XContent >
                queryContent( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

            virtual bool getResult( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

            virtual sal_uInt32 totalCount(std::unique_lock<std::mutex>& rResultSetGuard) override;
            virtual sal_uInt32 currentCount() override;
            virtual bool isCountFinal() override;

            virtual css::uno::Reference< css::sdbc::XRow >
                queryPropertyValues( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex  ) override;
            virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

            virtual void close() override;

            virtual void validate() override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
