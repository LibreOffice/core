/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_DATASUPPLIER_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_DATASUPPLIER_HXX

#include <vector>

#include <ucbhelper/resultset.hxx>

#include "children_provider.hxx"

namespace cmis
{

    class Content;

    struct ResultListEntry
    {
        css::uno::Reference< css::ucb::XContent > xContent;
        css::uno::Reference< css::sdbc::XRow > xRow;

        explicit ResultListEntry( css::uno::Reference< css::ucb::XContent > xCnt ) : xContent( xCnt )
        {
        }

        ~ResultListEntry()
        {
        }
    };

    typedef std::vector< ResultListEntry* > ResultList;

    class DataSupplier : public ucbhelper::ResultSetDataSupplier
    {
        private:
            ChildrenProvider* m_pChildrenProvider;
            sal_Int32 mnOpenMode;
            bool mbCountFinal;
            bool getData();
            ResultList maResults;

        public:
            DataSupplier( ChildrenProvider* pChildrenProvider, sal_Int32 nOpenMode );

            virtual ~DataSupplier();

            virtual OUString queryContentIdentifierString( sal_uInt32 nIndex ) override;
            virtual css::uno::Reference< css::ucb::XContentIdentifier >
                queryContentIdentifier( sal_uInt32 nIndex ) override;
            virtual css::uno::Reference< css::ucb::XContent >
                queryContent( sal_uInt32 nIndex ) override;

            virtual bool getResult( sal_uInt32 nIndex ) override;

            virtual sal_uInt32 totalCount() override;
            virtual sal_uInt32 currentCount() override;
            virtual bool isCountFinal() override;

            virtual css::uno::Reference< css::sdbc::XRow >
                queryPropertyValues( sal_uInt32 nIndex  ) override;
            virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

            virtual void close() override;

            virtual void validate()
                throw( css::ucb::ResultSetException ) override;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
