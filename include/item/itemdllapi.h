/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_ITEMDLLAPI_H
#define INCLUDED_ITEM_ITEMDLLAPI_H

#include <sal/types.h>

#if defined(ITEM_DLLIMPLEMENTATION)
#define ITEM_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define ITEM_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif
#define ITEM_DLLPRIVATE SAL_DLLPRIVATE

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
