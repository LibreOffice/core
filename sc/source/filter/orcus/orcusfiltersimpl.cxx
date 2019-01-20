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

#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <orcus/orcus_csv.hpp>
#include <orcus/orcus_gnumeric.hpp>
#include <orcus/orcus_xlsx.hpp>
#include <orcus/orcus_xls_xml.hpp>
#include <orcus/orcus_ods.hpp>
#include <orcus/orcus_import_ods.hpp>
#include <orcus/stream.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#ifdef _WIN32
#define SYSTEM_PATH FSysStyle::Dos
#else
#define SYSTEM_PATH FSysStyle::Unix
#endif

using namespace com::sun::star;

namespace {

uno::Reference<task::XStatusIndicator> getStatusIndicator(const SfxMedium& rMedium)
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    SfxItemSet* pSet = rMedium.GetItemSet();
    if (pSet)
    {
        const SfxUnoAnyItem* pItem = pSet->GetItem<SfxUnoAnyItem>(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem)
            xStatusIndicator.set(pItem->GetValue(), uno::UNO_QUERY);
    }
    return xStatusIndicator;
}

bool loadFileContent(SfxMedium& rMedium, orcus::iface::import_filter& filter)
{
    SvStream* pStream = rMedium.GetInStream();
    pStream->Seek(0);
    static const size_t nReadBuffer = 1024*32;
    OStringBuffer aBuffer((int(nReadBuffer)));
    size_t nRead = 0;
    do
    {
        char pData[nReadBuffer];
        nRead = pStream->ReadBytes(pData, nReadBuffer);
        aBuffer.append(static_cast<sal_Char*>(pData), nRead);
    }
    while (nRead == nReadBuffer);

    try
    {
        filter.read_stream(aBuffer.getStr(), aBuffer.getLength());
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load file via orcus filter! " << e.what());
        return false;
    }

    return true;
}

}

OString ScOrcusFiltersImpl::toSystemPath(const OUString& rPath)
{
    INetURLObject aURL(rPath);
    return OUStringToOString(aURL.getFSysPath(SYSTEM_PATH), RTL_TEXTENCODING_UTF8);
}

bool ScOrcusFiltersImpl::importCSV(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));

    orcus::orcus_csv filter(&aFactory);
    return loadFileContent(rMedium, filter);
}

bool ScOrcusFiltersImpl::importGnumeric(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));

    orcus::orcus_gnumeric filter(&aFactory);
    return loadFileContent(rMedium, filter);
}

bool ScOrcusFiltersImpl::importExcel2003XML(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));

    orcus::orcus_xls_xml filter(&aFactory);
    return loadFileContent(rMedium, filter);
}

bool ScOrcusFiltersImpl::importXLSX(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));

    orcus::orcus_xlsx filter(&aFactory);
    return loadFileContent(rMedium, filter);
}

bool ScOrcusFiltersImpl::importODS(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));

    orcus::orcus_ods filter(&aFactory);
    return loadFileContent(rMedium, filter);
}

bool ScOrcusFiltersImpl::importODS_Styles(ScDocument& rDoc, OUString& aPath) const
{
    OString aUrl = OUStringToOString(aPath, RTL_TEXTENCODING_UTF8);
    const char* path = aUrl.getStr();

    try
    {
        std::string content = orcus::load_file_content(path);
        ScOrcusFactory aFactory(rDoc);
        ScOrcusStyles aStyles(aFactory);
        orcus::import_ods::read_styles(content.c_str(), content.size(), &aStyles);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load styles from xml file! " << e.what());
        return false;
    }

    return true;
}

ScOrcusXMLContext* ScOrcusFiltersImpl::createXMLContext(ScDocument& rDoc, const OUString& rPath) const
{
    return new ScOrcusXMLContextImpl(rDoc, rPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
