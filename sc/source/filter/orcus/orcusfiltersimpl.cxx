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

#include "tools/urlobj.hxx"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_csv.hpp>
#include <orcus/orcus_gnumeric.hpp>
#include <orcus/global.hpp>

#ifdef WNT
#define SYSTEM_PATH INetURLObject::FSYS_DOS
#else
#define SYSTEM_PATH INetURLObject::FSYS_UNX
#endif

OString ScOrcusFiltersImpl::toSystemPath(const OUString& rPath)
{
    INetURLObject aURL(rPath);
    return OUStringToOString(aURL.getFSysPath(SYSTEM_PATH), RTL_TEXTENCODING_UTF8);
}

bool ScOrcusFiltersImpl::importCSV(ScDocument& rDoc, const OUString& rPath) const
{
    ScOrcusFactory aFactory(rDoc);
    OString aSysPath = toSystemPath(rPath);
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_csv filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception&)
    {
        rDoc.InsertTab(SC_TAB_APPEND, OUString("Foo"));
        rDoc.SetString(0, 0, 0, "Failed to load!!!");
        return false;
    }
    return true;
}

bool ScOrcusFiltersImpl::importGnumeric(ScDocument& rDoc, const OUString& rPath) const
{
    ScOrcusFactory aFactory(rDoc);
    OString aSysPath = toSystemPath(rPath);
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

ScOrcusXMLContext* ScOrcusFiltersImpl::createXMLContext(ScDocument& rDoc, const OUString& rPath) const
{
    return new ScOrcusXMLContextImpl(rDoc, rPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
