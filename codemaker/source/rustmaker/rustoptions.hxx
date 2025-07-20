/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/string.hxx>
#include <codemaker/options.hxx>

/**
 * Command line options parser for rustmaker
 */
class RustOptions : public Options
{
public:
    RustOptions() { m_program = "rustmaker"_ostr; }

    bool initOptions(int argc, char* argv[], bool bCmdFile = false) override;
    OString prepareHelp() override;
};

/* vim:set shiftwidth=2 softtabstop=4 expandtab: */
