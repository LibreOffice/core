/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/comphelperdllapi.h>

#include <rtl/ustring.hxx>

// Helper to convert URL to tooltip

namespace comphelper
{
class COMPHELPER_DLLPUBLIC URLHelper
{
public:
    static OUString getTooltip(const OUString& rURL);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
