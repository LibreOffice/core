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

#ifndef INCLUDED_SD_INC_SDERROR_HXX
#define INCLUDED_SD_INC_SDERROR_HXX

#include <vcl/errcode.hxx>

#define ERROR_SD_BASE           (ERRCODE_AREA_SD)
#define ERROR_SD_READ_BASE      (ERROR_SD_BASE | ERRCODE_CLASS_READ)
#define ERROR_SD_WRITE_BASE     (ERROR_SD_BASE | ERRCODE_CLASS_WRITE)

#define WARN_SD_BASE            (ERRCODE_AREA_SD | ERRCODE_WARNING_MASK)
#define WARN_SD_READ_BASE       (WARN_SD_BASE | ERRCODE_CLASS_READ )
#define WARN_SD_WRITE_BASE      (WARN_SD_BASE | ERRCODE_CLASS_WRITE )

// Import errors
#define ERR_FORMAT_ROWCOL               ErrCode(ERROR_SD_READ_BASE | 1)
#define ERR_FORMAT_FILE_ROWCOL          ErrCode(ERROR_SD_READ_BASE | 2)

// ----- Warnings ---------------------------

#define WARN_FORMAT_FILE_ROWCOL         ErrCode(WARN_SD_READ_BASE | 100)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
