/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_SQLDATAPROVIDER_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_SQLDATAPROVIDER_HXX

#include <dataprovider.hxx>

namespace sc
{
class SQLFetchThread;
class DataTransformation;

class SQLDataProvider : public DataProvider
{
private:
    ScDocument* mpDocument;
    rtl::Reference<SQLFetchThread> mxSQLFetchThread;

    ScDocumentUniquePtr mpDoc;

public:
    SQLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource);
    virtual ~SQLDataProvider() override;

    virtual void Import() override;

    virtual const OUString& GetURL() const override;

    void ImportFinished();
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
