/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONFIGMGR_SOURCE_DCONF_HXX
#define INCLUDED_CONFIGMGR_SOURCE_DCONF_HXX

#include <sal/config.h>

namespace configmgr {
    class Components;
    struct Data;
}

namespace configmgr { namespace dconf {

void readLayer(Data & data, int layer);

void writeModifications(Components & components, Data & data);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
