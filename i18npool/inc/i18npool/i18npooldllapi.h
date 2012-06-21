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

#ifndef INCLUDED_I18NPOOL_I18NPOOLDLLAPI_H
#define INCLUDED_I18NPOOL_I18NPOOLDLLAPI_H

#include "sal/types.h"

#if defined(I18NPAPER_DLLIMPLEMENTATION)
#define I18NPAPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define I18NPAPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define I18NPAPER_DLLPRIVATE SAL_DLLPRIVATE

#if defined(I18NISOLANG_DLLIMPLEMENTATION)
#define I18NISOLANG_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define I18NISOLANG_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define I18NISOLANG_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_I18NPOOL_I18NPOOLDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
