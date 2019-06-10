/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_HTMLDATAPROVIDER_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_HTMLDATAPROVIDER_HXX

#include <dataprovider.hxx>

namespace sc {

class HTMLFetchThread;

class HTMLDataProvider : public DataProvider
{
private:

    ScDocument* const mpDocument;
    rtl::Reference<HTMLFetchThread> mxHTMLFetchThread;

    ScDocumentUniquePtr mpDoc;

public:

    HTMLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource);
    virtual ~HTMLDataProvider() override;

    virtual void Import() override;

    virtual const OUString& GetURL() const override;

    std::map<OUString, OUString> getDataSourcesForURL(const OUString& rURL);

    void ImportFinished();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
