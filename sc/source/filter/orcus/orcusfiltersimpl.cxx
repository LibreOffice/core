/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcusfiltersimpl.hxx>
#include <orcusinterface.hxx>
#include <tokenarray.hxx>

#include <osl/thread.hxx>
#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/tempfile.hxx>

#include <orcus/format_detection.hpp>
#include <orcus/orcus_import_ods.hpp>
#include <orcus/stream.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

using namespace com::sun::star;

namespace
{
/**
 * Stream copied to a temporary file with a filepath.
 */
class CopiedTempStream
{
    utl::TempFileNamed maTemp;

public:
    CopiedTempStream(SvStream& rSrc)
    {
        maTemp.EnableKillingFile();
        SvStream* pDest = maTemp.GetStream(StreamMode::WRITE);

        rSrc.Seek(0);

        const std::size_t nReadBuffer = 1024 * 32;
        std::size_t nRead = 0;

        do
        {
            char pData[nReadBuffer];
            nRead = rSrc.ReadBytes(pData, nReadBuffer);
            pDest->WriteBytes(pData, nRead);
        } while (nRead == nReadBuffer);

        maTemp.CloseStream();
    }

    OString getFileName() const { return maTemp.GetFileName().toUtf8(); }
};

uno::Reference<task::XStatusIndicator> getStatusIndicator(const SfxMedium& rMedium)
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    const SfxUnoAnyItem* pItem
        = rMedium.GetItemSet().GetItem<SfxUnoAnyItem>(SID_PROGRESS_STATUSBAR_CONTROL);
    if (pItem)
        xStatusIndicator.set(pItem->GetValue(), uno::UNO_QUERY);
    return xStatusIndicator;
}

bool loadFileContent(SfxMedium& rMedium, orcus::iface::import_filter& filter)
{
    SvStream* pSrc = rMedium.GetInStream();
    if (!pSrc)
        return false;

    try
    {
        // memory-map the temp file and start the import
        CopiedTempStream aTemp(*pSrc);
        orcus::file_content input(aTemp.getFileName());
        filter.read_stream(input.str());
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load file via orcus filter! " << e.what());
        return false;
    }

    return true;
}
}

ScOrcusFilters::ImportResult ScOrcusFiltersImpl::importByName(ScDocument& rDoc, SfxMedium& rMedium,
                                                              const OUString& rFilterName) const
{
    const std::unordered_map<OUString, orcus::format_t> aMap = {
        { "Apache Parquet Spreadsheet", orcus::format_t::parquet },
        { "Gnumeric Spreadsheet", orcus::format_t::gnumeric },
        { "MS Excel 2003 XML Orcus", orcus::format_t::xls_xml },
        { "Orcus CSV", orcus::format_t::csv },
        { "csv", orcus::format_t::csv },
        { "gnumeric", orcus::format_t::gnumeric },
        { "ods", orcus::format_t::ods },
        { "parquet", orcus::format_t::parquet },
        { "xls-xml", orcus::format_t::xls_xml },
        { "xlsx", orcus::format_t::xlsx },
    };

    if (auto it = aMap.find(rFilterName); it != aMap.end())
    {
        ScOrcusFactory aFactory(rDoc);
        aFactory.setStatusIndicator(getStatusIndicator(rMedium));

        auto filter = orcus::create_filter(it->second, &aFactory);
        if (!filter)
            return ImportResult::Failure;

        bool res = loadFileContent(rMedium, *filter);
        return res ? ImportResult::Success : ImportResult::Failure;
    }

    return ImportResult::NotSupported;
}

bool ScOrcusFiltersImpl::importODS_Styles(ScDocument& rDoc, OUString& aPath) const
{
    try
    {
        OUString aURL;
        if (osl::FileBase::getFileURLFromSystemPath(aPath, aURL) != osl::FileBase::E_None)
            return false;

        SvFileStream aSrc(aURL, StreamMode::READ);
        CopiedTempStream aTemp(aSrc);
        orcus::file_content content(aTemp.getFileName());

        ScOrcusFactory aFactory(rDoc);
        ScOrcusStyles aStyles(aFactory);
        orcus::import_ods::read_styles(content.str(), &aStyles);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load styles from xml file! " << e.what());
        return false;
    }

    return true;
}

std::unique_ptr<ScOrcusXMLContext> ScOrcusFiltersImpl::createXMLContext(ScDocument& rDoc,
                                                                        const OUString& rPath) const
{
    return std::make_unique<ScOrcusXMLContextImpl>(rDoc, rPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
