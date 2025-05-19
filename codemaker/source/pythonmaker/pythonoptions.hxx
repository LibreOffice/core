/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <codemaker/options.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

class PythonOptions : public Options
{
public:
    PythonOptions();

    bool initOptions(int ac, char* av[], bool bCmdFile = false) override;
    OString prepareHelp() override;
    OString prepareVersion() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
