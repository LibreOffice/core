/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <COKit/COKit.hxx>
#include <openuriexternally.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/viewsh.hxx>

void sfx2::openUriExternally(const OUString& sURI)
{
    if (SfxViewShell* pViewShell = SfxViewShell::Current())
        pViewShell->viewCallback(COKitCallbackType::HYPERLINK_CLICKED, sURI.toUtf8());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
