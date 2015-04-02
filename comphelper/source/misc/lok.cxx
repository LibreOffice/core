/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>

namespace comphelper
{

namespace LibreOfficeKit
{

static bool bActive(false);

void setActive()
{
    bActive = true;
}

bool isActive()
{
    return bActive;
}

} // namespace LibreOfficeKit

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
