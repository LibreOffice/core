/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_NP_SDK_MOZSRC_PLUGIN_H
#define INCLUDED_NP_SDK_MOZSRC_PLUGIN_H

#include "npapi.h"

/* OpenOffice.org/LibreOffice modified the implementations of NP_Initialize and
   NP_Shutdown in np_sdk/npsdk/ to call NPP_Initialize and NPP_Shutdown, resp.,
   defined in extensions/source/nsplugin/source/npshell.cxx:
*/

#if defined __cplusplus
extern "C" {
#endif

NPError NPP_Initialize(void);

void NPP_Shutdown(void);

#if defined __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
