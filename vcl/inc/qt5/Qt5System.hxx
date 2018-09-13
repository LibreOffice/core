/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/sysdata.hxx>

#ifdef _WIN32
#include <win/salsys.h>
#else
#ifdef MACOSX
#include <osx/salsys.h>
#else
#include <unx/gensys.h>
#endif
#endif

class Qt5System
#ifdef _WIN32
    : public WinSalSystem
#else
#ifdef MACOSX
    : public AquaSalSystem
#else
    : public SalGenericSystem
#endif
#endif
{
public:
#if !(defined MACOSX || defined _WIN32)
    virtual unsigned int GetDisplayScreenCount() override;
    virtual tools::Rectangle GetDisplayScreenPosSizePixel(unsigned int nScreen) override;
    virtual int ShowNativeDialog(const OUString& rTitle, const OUString& rMessage,
                                 const std::vector<OUString>& rButtons) override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
