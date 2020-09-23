/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <tools/stream.hxx>
#include <comphelper/processfactory.hxx>

#include "htmldataprovider.hxx"
#include "xmldataprovider.hxx"
#include "sqldataprovider.hxx"
#include <datamapper.hxx>
#include <dbdata.hxx>
#include <docsh.hxx>

using namespace com::sun::star;

namespace sc {

std::unique_ptr<SvStream> DataProvider::FetchStreamFromURL(const OUString& rURL, OStringBuffer& rBuffer)
{
    try
    {
        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess = ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() );

        uno::Reference< io::XInputStream > xStream = xFileAccess->openFileRead( rURL );

        const sal_Int32 BUF_LEN = 8000;
        uno::Sequence< sal_Int8 > buffer( BUF_LEN );

        sal_Int32 nRead = 0;
        while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
        {
            rBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        if ( nRead > 0 )
        {
            rBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        xStream->closeInput();

        SvStream* pStream = new SvMemoryStream(const_cast<char*>(rBuffer.getStr()), rBuffer.getLength(), StreamMode::READ);
        return std::unique_ptr<SvStream>(pStream);
    }
    catch(...)
    {
        rBuffer.setLength(0);
        return nullptr;
    }
}

ExternalDataSource::ExternalDataSource(const OUString& rURL,
        const OUString& rProvider, ScDocument* pDoc)
    : maURL(rURL)
    , maProvider(rProvider)
    , mpDoc(pDoc)
{
}

void ExternalDataSource::setID(const OUString& rID)
{
    maID = rID;
}

void ExternalDataSource::setXMLImportParam(const ScOrcusImportXMLParam& rParam)
{
    maParam = rParam;
}



void ExternalDataSource::setURL(const OUString& rURL)
{
    maURL = rURL;
}

void ExternalDataSource::setProvider(const OUString& rProvider)
{
    maProvider = rProvider;
    mpDataProvider.reset();
}

const OUString& ExternalDataSource::getURL() const
{
    return maURL;
}

const OUString& ExternalDataSource::getProvider() const
{
    return maProvider;
}

const OUString& ExternalDataSource::getID() const
{
    return maID;
}

const ScOrcusImportXMLParam& ExternalDataSource::getXMLImportParam() const
{
    return maParam;
}

OUString ExternalDataSource::getDBName() const
{
    if (mpDBDataManager)
    {
        ScDBData* pDBData = mpDBDataManager->getDBData();
        if (pDBData)
            return pDBData->GetName();
    }
    return OUString();
}

void ExternalDataSource::setDBData(const OUString& rDBName)
{
    if (!mpDBDataManager)
    {
        mpDBDataManager = std::make_shared<ScDBDataManager>(rDBName, mpDoc);
    }
    else
    {
        mpDBDataManager->SetDatabase(rDBName);
    }
}

double ExternalDataSource::getUpdateFrequency()
{
    return 0;
}

ScDBDataManager* ExternalDataSource::getDBManager()
{
    return mpDBDataManager.get();
}

void ExternalDataSource::refresh(ScDocument* pDoc, bool bDeterministic)
{
    // no DB data available
    if (!mpDBDataManager)
        return;

    // if no data provider exists, try to create one
    if (!mpDataProvider)
        mpDataProvider = DataProviderFactory::getDataProvider(pDoc, *this);

    // if we still have not been able to create one, we can not refresh the data
    if (!mpDataProvider)
        return;

    if (bDeterministic)
        mpDataProvider->setDeterministic();

    mpDataProvider->Import();
}

void ExternalDataSource::AddDataTransformation(
    const std::shared_ptr<sc::DataTransformation>& mpDataTransformation)
{
    maDataTransformations.push_back(mpDataTransformation);
}

const std::vector<std::shared_ptr<sc::DataTransformation>>& ExternalDataSource::getDataTransformation() const
{
    return maDataTransformations;
}

ExternalDataMapper::ExternalDataMapper(ScDocument& /*rDoc*/)
    //mrDoc(rDoc)
{
}

ExternalDataMapper::~ExternalDataMapper()
{
}

void ExternalDataMapper::insertDataSource(const sc::ExternalDataSource& rSource)
{
    maDataSources.push_back(rSource);
}

const std::vector<sc::ExternalDataSource>& ExternalDataMapper::getDataSources() const
{
    return maDataSources;
}

std::vector<sc::ExternalDataSource>& ExternalDataMapper::getDataSources()
{
    return maDataSources;
}

DataProvider::DataProvider(sc::ExternalDataSource& rDataSource):
    mbDeterministic(false),
    mrDataSource(rDataSource)
{
}

void DataProvider::setDeterministic()
{
    mbDeterministic = true;
}

DataProvider::~DataProvider()
{
}

void ScDBDataManager::WriteToDoc(ScDocument& rDoc)
{
    // first apply all data transformations

    bool bShrunk = false;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = rDoc.MaxCol();
    SCROW nEndRow = rDoc.MaxRow();
    rDoc.ShrinkToUsedDataArea(bShrunk, 0, nStartCol, nStartRow, nEndCol, nEndRow, false, true, true);
    ScRange aClipRange(nStartCol, nStartRow, 0, nEndCol, nEndRow, 0);
    rDoc.SetClipArea(aClipRange);

    ScRange aDestRange;
    getDBData()->GetArea(aDestRange);
    SCCOL nColSize = std::min<SCCOL>(aDestRange.aEnd.Col() - aDestRange.aStart.Col(), nEndCol);
    aDestRange.aEnd.SetCol(aDestRange.aStart.Col() + nColSize);

    SCROW nRowSize = std::min<SCROW>(aDestRange.aEnd.Row() - aDestRange.aStart.Row(), nEndRow);
    aDestRange.aEnd.SetRow(aDestRange.aStart.Row() + nRowSize);

    ScMarkData aMark(mpDoc->GetSheetLimits());
    aMark.SelectTable(0, true);
    mpDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &rDoc);
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDoc->GetDocumentShell());
    if (pDocShell)
        pDocShell->PostPaint(aDestRange, PaintPartFlags::All);
}

ScDBDataManager::ScDBDataManager(const OUString& rDBName, ScDocument* pDoc):
    maDBName(rDBName),
    mpDoc(pDoc)
{
}

ScDBDataManager::~ScDBDataManager()
{
}

void ScDBDataManager::SetDatabase(const OUString& rDBName)
{
    maDBName = rDBName;
}

ScDBData* ScDBDataManager::getDBData()
{
    ScDBData* pDBData = mpDoc->GetDBCollection()->getNamedDBs().findByUpperName(ScGlobal::getCharClassPtr()->uppercase(maDBName));
    return pDBData;
}

bool DataProviderFactory::isInternalDataProvider(const OUString& rProvider)
{
    return rProvider.startsWith("org.libreoffice.calc");
}

std::shared_ptr<DataProvider> DataProviderFactory::getDataProvider(ScDocument* pDoc,
        sc::ExternalDataSource& rDataSource)
{
    const OUString& rDataProvider = rDataSource.getProvider();
    bool bInternal = DataProviderFactory::isInternalDataProvider(rDataProvider);
    if (bInternal)
    {
        if (rDataProvider == "org.libreoffice.calc.csv")
            return std::make_shared<CSVDataProvider>(pDoc, rDataSource);
        else if (rDataProvider == "org.libreoffice.calc.html")
            return std::make_shared<HTMLDataProvider>(pDoc, rDataSource);
        else if (rDataProvider == "org.libreoffice.calc.xml")
            return std::make_shared<XMLDataProvider>(pDoc, rDataSource);
        else if (rDataProvider == "org.libreoffice.calc.sql")
            return std::make_shared<SQLDataProvider>(pDoc, rDataSource);
    }
    else
    {
        SAL_WARN("sc", "no external data provider supported yet");
        return std::shared_ptr<DataProvider>();
    }

    return std::shared_ptr<DataProvider>();
}

std::vector<OUString> DataProviderFactory::getDataProviders()
{
    std::vector<OUString> aDataProviders;
    aDataProviders.emplace_back("org.libreoffice.calc.csv");
    aDataProviders.emplace_back("org.libreoffice.calc.html");
    aDataProviders.emplace_back("org.libreoffice.calc.xml");
    aDataProviders.emplace_back("org.libreoffice.calc.sql");

    return aDataProviders;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
