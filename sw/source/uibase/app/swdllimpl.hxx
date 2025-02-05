/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <sal/types.h>

#include <memory>

namespace sw
{
class Filters;
}

class SvxAutoCorrCfg;

class SwDLL
{
public:
    static void RegisterFactories();
    static void RegisterInterfaces();
    static void RegisterControls();

    SwDLL();
    ~SwDLL() COVERITY_NOEXCEPT_FALSE;

private:
    SwDLL(SwDLL const&) = delete;
    SwDLL& operator=(SwDLL const&) = delete;

    std::unique_ptr<sw::Filters> m_pFilters;
    SvxAutoCorrCfg* m_pAutoCorrCfg;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
