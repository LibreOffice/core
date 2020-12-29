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

#include <memory>

#include <osl/thread.hxx>
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

#if defined _WIN32
#include <boost/filesystem/operations.hpp> // for boost::filesystem::filesystem_error
#include <o3tl/char16_t2wchar_t.hxx>
#include <prewin.h>
#include <postwin.h>
#endif

using namespace com::sun::star;

namespace
{
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
    static const size_t nReadBuffer = 1024 * 32;
    OStringBuffer aBuffer((int(nReadBuffer)));
    size_t nRead = 0;
    do
    {
        char pData[nReadBuffer];
        nRead = pStream->ReadBytes(pData, nReadBuffer);
        aBuffer.append(pData, nRead);
    } while (nRead == nReadBuffer);

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
    OString aPath8 = OUStringToOString(aPath, osl_getThreadTextEncoding());

    try
    {
#if defined _WIN32
        std::unique_ptr<orcus::file_content> content;
        try
        {
            content = std::make_unique<orcus::file_content>(aPath8.getStr());
        }
        catch (const boost::filesystem::filesystem_error&)
        {
            // Maybe the path contains characters not representable in ACP. It's not
            // yet possible to pass Unicode path to orcus::file_content ctor - see
            // https://gitlab.com/orcus/orcus/-/issues/30; try short path.
            wchar_t buf[32767];
            if (GetShortPathNameW(o3tl::toW(aPath.getStr()), buf, std::size(buf)) == 0)
                throw;
            aPath8 = OUStringToOString(o3tl::toU(buf), osl_getThreadTextEncoding());
            content = std::make_unique<orcus::file_content>(aPath8.getStr());
        }
#else
        auto content = std::make_unique<orcus::file_content>(aPath8.getStr());
#endif
        ScOrcusFactory aFactory(rDoc);
        ScOrcusStyles aStyles(aFactory);
        orcus::import_ods::read_styles(content->data(), content->size(), &aStyles);
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
