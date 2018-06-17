/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmldataprovider.hxx"
#include <datatransformation.hxx>
#include <salhelper/thread.hxx>

#include <comphelper/string.hxx>
#include <orcusfiltersimpl.hxx>

#include <orcusxml.hxx>
#include <filter.hxx>
#include <document.hxx>

#include <svtools/treelistbox.hxx>
#include <svtools/treelistentry.hxx>
#include <ucbhelper/content.hxx>
#include <o3tl/make_unique.hxx>
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/orcus_xml.hpp>
#include <orcus/global.hpp>
#include <orcus/sax_parser_base.hpp>

#include <orcus/stream.hpp>
#include <orcusfiltersimpl.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <comphelper/processfactory.hxx>

#include <string>
#include <sstream>
#include <vector>

#include <orcusfilters.hxx>

using namespace com::sun::star;

namespace sc
{
class SetNamespaceAlias
{
    orcus::orcus_xml& mrFilter;
    orcus::xmlns_repository& mrNsRepo;

public:
    SetNamespaceAlias(orcus::orcus_xml& filter, orcus::xmlns_repository& repo)
        : mrFilter(filter)
        , mrNsRepo(repo)
    {
    }

    void operator()(size_t index)
    {
        orcus::xmlns_id_t nsid = mrNsRepo.get_identifier(index);
        if (nsid == orcus::XMLNS_UNKNOWN_ID)
            return;

        std::string alias = mrNsRepo.get_short_name(index);
        mrFilter.set_namespace_alias(alias.c_str(), nsid);
    }
};

class InsertFieldPath
{
    orcus::orcus_xml& mrFilter;

public:
    explicit InsertFieldPath(orcus::orcus_xml& rFilter)
        : mrFilter(rFilter)
    {
    }
    void operator()(const OString& rPath) { mrFilter.append_field_link(rPath.getStr()); }
};

class XMLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;
    OUString maID;
    OUString maTreePath;
    ScOrcusImportXMLParam maParam;
    std::unique_ptr<ScOrcusXMLContext> mpXMLContext;
    const std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;
    std::function<void()> maImportFinishedHdl;

public:
    XMLFetchThread(ScDocument& rDoc, const OUString&, const OUString& rTreePath,
                   const ScOrcusImportXMLParam& rParam, const OUString& rID,
                   std::function<void()> aImportFinishedHdl,
                   const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations);
    void removeDuplicates(std::vector<size_t>& rArray);
    virtual void execute() override;
};

XMLFetchThread::XMLFetchThread(
    ScDocument& rDoc, const OUString& rURL, const OUString& rTreePath,
    const ScOrcusImportXMLParam& rParam, const OUString& rID,
    std::function<void()> aImportFinishedHdl,
    const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations)
    : salhelper::Thread("XML Fetch Thread")
    , mrDocument(rDoc)
    , maURL(rURL)
    , maID(rID)
    , maTreePath(rTreePath)
    , maParam(rParam)
    , maDataTransformations(rTransformations)
    , maImportFinishedHdl(aImportFinishedHdl)
{
}

void XMLFetchThread::removeDuplicates(std::vector<size_t>& rArray)
{
    std::sort(rArray.begin(), rArray.end());
    std::vector<size_t>::iterator it = std::unique(rArray.begin(), rArray.end());
    rArray.erase(it, rArray.end());
}

void XMLFetchThread::execute()
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    mpXMLContext.reset(pOrcus->createXMLContext(mrDocument, maURL));
    if (!mpXMLContext)
        return;

    if (maID != OUString(""))
    {
        ScOrcusImportXMLParam::RangeLink aRangeLink;
        aRangeLink.maPos = ScAddress(0, 0, 0);
        aRangeLink.maFieldPaths.push_back(OUStringToOString(maID, RTL_TEXTENCODING_UTF8));
        maParam.maRangeLinks.clear();
        maParam.maRangeLinks.push_back(aRangeLink);
    }
    // Do the import.
    mpXMLContext->importXML(maParam);

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    maImportFinishedHdl();
}

XMLDataProvider::XMLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource)
    : DataProvider(rDataSource)
    , mpDocument(pDoc)
{
}

XMLDataProvider::~XMLDataProvider()
{
    if (mxXMLFetchThread.is())
    {
        SolarMutexReleaser aReleaser;
        mxXMLFetchThread->join();
    }
}

void XMLDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxXMLFetchThread = new XMLFetchThread(*mpDoc, mrDataSource.getURL(), mrDataSource.getTreePath(),
                                          mrDataSource.getXMLImportParam(), mrDataSource.getID(),
                                          std::bind(&XMLDataProvider::ImportFinished, this),
                                          mrDataSource.getDataTransformation());
    mxXMLFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxXMLFetchThread->join();
    }
}

std::map<OUString, OUString> XMLDataProvider::getDataSourcesForURL(const OUString& /*rURL*/)
{
    std::map<OUString, OUString> aMap;

    OStringBuffer aBuffer(64000);
    std::unique_ptr<SvStream> pStream
        = DataProvider::FetchStreamFromURL(mrDataSource.getURL(), aBuffer);

    if (aBuffer.isEmpty())
        return std::map<OUString, OUString>();

    return aMap;
}

void XMLDataProvider::ImportFinished()
{
    mrDataSource.getDBManager()->WriteToDoc(*mpDoc);
    mxXMLFetchThread.clear();
    mpDoc.reset();
}

const OUString& XMLDataProvider::GetURL() const { return mrDataSource.getURL(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
