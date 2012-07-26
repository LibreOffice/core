
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _MORK_DLLAPI_H
#define _MORK_DLLAPI_H

#include "sal/config.h"
#include "sal/types.h"

#if defined LO_DLLIMPLEMENTATION_MORK
#define LO_DLLPUBLIC_MORK SAL_DLLPUBLIC_EXPORT
#else
#define LO_DLLPUBLIC_MORK SAL_DLLPUBLIC_IMPORT
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
