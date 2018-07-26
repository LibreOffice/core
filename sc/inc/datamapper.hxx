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

#include <vector>

#include "scdllapi.h"

#include <rtl/ustring.hxx>

#include "orcusxml.hxx"

class ScDocument;
class ScDBData;

namespace sc {

class ScDBDataManager;
class DataProvider;
class ScDBDataManager;
class DataTransformation;

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

    ScOrcusImportXMLParam maParam;

    double mnUpdateFrequency;

    std::shared_ptr<DataProvider> mpDataProvider;
    std::shared_ptr<ScDBDataManager> mpDBDataManager;

    std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;

    ScDocument* mpDoc;

public:

    ExternalDataSource(const OUString& rURL,
            const OUString& rProvider, ScDocument* pDoc);

    void setUpdateFrequency(double nUpdateFrequency);

    void setID(const OUString& rID);
    void setURL(const OUString& rURL);
    void setProvider(const OUString& rProvider);
    void setXMLImportParam(const ScOrcusImportXMLParam& rParam);

    const OUString& getURL() const;
    const OUString& getProvider() const;
    const OUString& getID() const;
    const ScOrcusImportXMLParam& getXMLImportParam() const;
    double getUpdateFrequency() const;
    OUString getDBName() const;
    void setDBData(const OUString& rDBName);
    ScDBDataManager* getDBManager();

    void refresh(ScDocument* pDoc, bool bDeterministic = false);

    void AddDataTransformation(std::shared_ptr<sc::DataTransformation> mpDataTransformation);
    const std::vector<std::shared_ptr<sc::DataTransformation>>& getDataTransformation() const;
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
