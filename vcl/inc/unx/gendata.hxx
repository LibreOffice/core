/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <osl/socket.hxx>

#include <svdata.hxx>

#include <memory>

#ifndef IOS
class FreetypeManager;
#endif
class SalGenericDisplay;

#ifndef IOS

namespace psp
{
class PrintFontManager;
class PrinterInfoManager;
}

// SalData is a bit of a mess. For ImplSVData we need a SalData base class.
// Windows, MacOS and iOS implement their own SalData class, so there is no
// way to do inheritance from the "top" in all plugins. We also really don't
// want to rename GenericUnixSalData and don't want to reinterpret_cast some
// dummy pointer everywhere, so this seems the only sensible solution.
class VCL_PLUGIN_PUBLIC SalData
{
protected:
    SalData();

public:
    virtual ~SalData();
};

#endif

// This class is kind of a misnomer. What this class is mainly about is the
// usage of Freetype and Fontconfig, which happens to match all *nix backends;
// except that the osx and ios backends are *nix but don't use this.
class VCL_PLUGIN_PUBLIC GenericUnixSalData : public SalData
{
#ifndef IOS
    friend class ::psp::PrinterInfoManager;
#endif

#ifndef IOS
    std::unique_ptr<FreetypeManager> m_pFreetypeManager;
    std::unique_ptr<psp::PrintFontManager> m_pPrintFontManager;
    std::unique_ptr<psp::PrinterInfoManager> m_pPrinterInfoManager;
#endif

public:
    GenericUnixSalData();
    virtual ~GenericUnixSalData() override;

#ifndef IOS
    FreetypeManager* GetFreetypeManager();
    psp::PrintFontManager* GetPrintFontManager();
#endif
};

inline GenericUnixSalData* GetGenericUnixSalData()
{
    return static_cast<GenericUnixSalData*>(GetSalData());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
