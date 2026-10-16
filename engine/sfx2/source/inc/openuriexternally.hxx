/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_OPENURIEXTERNALLY_HXX
#define INCLUDED_SFX2_SOURCE_INC_OPENURIEXTERNALLY_HXX

#include <sal/config.h>

#include <rtl/ustring.hxx>

namespace sfx2
{
/** Send the URI to the client of the current view as a clicked hyperlink. Nothing is sent when
    there is no current view.
*/
void openUriExternally(const OUString& sURI);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
