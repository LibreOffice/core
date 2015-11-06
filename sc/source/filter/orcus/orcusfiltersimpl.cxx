/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcusfiltersimpl.hxx"
#include "orcusinterface.hxx"

#include "document.hxx"

#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_csv.hpp>
#include <orcus/orcus_gnumeric.hpp>
#include <orcus/orcus_xlsx.hpp>
#include <orcus/orcus_ods.hpp>
#include <orcus/global.hpp>

#include <com/sun/star/task/XStatusIndicator.hpp>

#ifdef WNT
#define SYSTEM_PATH INetURLObject::FSYS_DOS
#else
#define SYSTEM_PATH INetURLObject::FSYS_UNX
#endif

using namespace com::sun::star;

namespace {

uno::Reference<task::XStatusIndicator> getStatusIndicator(SfxMedium& rMedium)
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    SfxItemSet* pSet = rMedium.GetItemSet();
    if (pSet)
    {
        const SfxUnoAnyItem* pItem = static_cast<const SfxUsrAnyItem*>(pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));
        if (pItem)
            xStatusIndicator.set(pItem->GetValue(), uno::UNO_QUERY);
    }
    return xStatusIndicator;
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
    OString aSysPath = toSystemPath(rMedium.GetName());
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_csv filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception&)
    {
        rDoc.InsertTab(SC_TAB_APPEND, "Foo");
        rDoc.SetString(0, 0, 0, "Failed to load!!!");
        return false;
    }
    return true;
}

bool ScOrcusFiltersImpl::importGnumeric(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));
    OString aSysPath = toSystemPath(rMedium.GetName());
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_gnumeric filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load gnumeric file! " << e.what());
        return false;
    }

    return true;
}

bool ScOrcusFiltersImpl::importXLSX(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));
    OString aSysPath = toSystemPath(rMedium.GetName());
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_xlsx filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load xlsx file! " << e.what());
        return false;
    }

    return true;
}

bool ScOrcusFiltersImpl::importODS(ScDocument& rDoc, SfxMedium& rMedium) const
{
    ScOrcusFactory aFactory(rDoc);
    aFactory.setStatusIndicator(getStatusIndicator(rMedium));
    OString aSysPath = toSystemPath(rMedium.GetName());
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_ods filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc", "Unable to load ods file! " << e.what());
        return false;
    }

    return true;
}

ScOrcusXMLContext* ScOrcusFiltersImpl::createXMLContext(ScDocument& rDoc, const OUString& rPath) const
{
    return new ScOrcusXMLContextImpl(rDoc, rPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
