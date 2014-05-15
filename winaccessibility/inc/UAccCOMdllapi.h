/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WINACCESSIBILITY_INC_UACCCOMDLLAPI_H
#define INCLUDED_WINACCESSIBILITY_INC_UACCCOMDLLAPI_H

#include <sal/types.h>

#if defined(UACCCOM_DLLIMPLEMENTATION)
#define UACCCOM_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define UACCCOM_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
