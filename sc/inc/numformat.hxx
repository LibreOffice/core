/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"

#include <tools/solar.h>

class ScPatternAttr;
class ScDocument;

namespace sc
{
class SC_DLLPUBLIC NumFmtUtil
{
public:
    /**
     * Check if the attribute pattern has a number format that only produces
     * latin script output.
     */
    static bool isLatinScript(const ScPatternAttr& rPat, ScDocument& rDoc);

    static bool isLatinScript(sal_uLong nFormat, ScDocument& rDoc);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
