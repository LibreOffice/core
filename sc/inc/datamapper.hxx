/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_EXTERNALDATAMAPPER_HXX
#define INCLUDED_SC_INC_EXTERNALDATAMAPPER_HXX

#include <memory>
#include <vector>

#include "scdllapi.h"

#include <rtl/ustring.hxx>

class ScDocument;
class ScDBData;

namespace sc {

class ScDBDataManager;
class DataProvider;
class ScDBDataManager;

class SC_DLLPUBLIC ExternalDataSource
{
private:

    /**
     * The URL for the external data provider. The URL
     * will be passed to the data provider together with
     * the ID.
     *
     * A data provider may decide to ignore the URL string.
     */
    OUString maURL;

    /**
     * The data provider is a unique identifier that will
     * allow to identify and instantiate the required data
     * provider.
     *
     * Examples for the internal data providers are:
     *
     * org.libreoffice.dataprovider.calc.csv
     * org.libreoffice.dataprovider.calc.json
     *
     * Only internal data providers should use the:
     * "org.libreoffice.dataprovider prefix".
     */
    OUString maProvider;

    /**
     * The ID allows the same data provider to support different
     * data streams.
     *
     * A data provider may decide to ignore the ID string.
     */
    OUString maID;

    double mnUpdateFrequency;

    std::shared_ptr<DataProvider> mpDataProvider;
    std::shared_ptr<ScDBDataManager> mpDBDataManager;

    ScDocument* mpDoc;

public:

    ExternalDataSource(const OUString& rURL,
            const OUString& rProvider, ScDocument* pDoc);

    ~ExternalDataSource();

    void setUpdateFrequency(double nUpdateFrequency);

    void setID(const OUString& rID);

    const OUString& getURL() const;
    const OUString& getProvider() const;
    const OUString& getID() const;
    double getUpdateFrequency() const;
    OUString getDBName() const;
    void setDBData(const ScDBData* pDBData);

    void refresh(ScDocument* pDoc, bool bDeterministic = false);
};

class SC_DLLPUBLIC ExternalDataMapper
{
    //ScDocument* mpDoc;
    std::vector<ExternalDataSource> maDataSources;

public:
    ExternalDataMapper(ScDocument* pDoc);

    ~ExternalDataMapper();

    void insertDataSource(const ExternalDataSource& rSource);

    const std::vector<ExternalDataSource>& getDataSources() const;
    std::vector<ExternalDataSource>& getDataSources();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
