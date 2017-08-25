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

#include "dataprovider.hxx"

namespace sc {

class HTMLFetchThread;
class DataTransformation;

class HTMLDataProvider : public DataProvider
{
private:

    OUString maID;
    OUString maURL;
    ScDocument* mpDocument;
    ScDBDataManager* mpDBDataManager;
    rtl::Reference<HTMLFetchThread> mxHTMLFetchThread;

    ScDocumentUniquePtr mpDoc;
    Idle maIdle;

public:

    HTMLDataProvider(ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pDBManager, const OUString& rID);
    virtual ~HTMLDataProvider() override;

    virtual void Import() override;

    virtual const OUString& GetURL() const override;

    virtual std::map<OUString, OUString> getDataSourcesForURL(const OUString& rURL) override;

    DECL_LINK( ImportFinishedHdl, Timer*, void );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
