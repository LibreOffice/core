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

#ifndef INCLUDED_SW_INC_SWERROR_H
#define INCLUDED_SW_INC_SWERROR_H

// Import & Export
#define ERR_SWG_FILE_FORMAT_ERROR       ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 1 )
#define ERR_SWG_READ_ERROR              ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 2 )
#define ERR_WW6_NO_WW6_FILE_ERR         ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 8 )
#define ERR_FORMAT_ROWCOL               ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 12)
#define ERR_WW8_NO_WW8_FILE_ERR         ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 14)
#define ERR_FORMAT_FILE_ROWCOL          ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 15)
#define ERR_SWG_WRITE_ERROR             ErrCode(ErrCodeArea::Sw, ErrCodeClass::Write, 30 )
#define ERR_SWG_OLD_GLOSSARY            ErrCode(ErrCodeArea::Sw, ErrCodeClass::Write, 31 )
#define ERR_WRITE_ERROR_FILE            ErrCode(ErrCodeArea::Sw, ErrCodeClass::Write, 35 )
#define ERR_SWG_INTERNAL_ERROR          ErrCode(ErrCodeArea::Sw, 50 )
#define ERR_TXTBLOCK_NEWFILE_ERROR      ErrCode(ErrCodeArea::Sw, ErrCodeClass::Locking, 55 )
#define ERR_AUTOPATH_ERROR              ErrCode(ErrCodeArea::Sw, ErrCodeClass::Path, 55 )
#define ERR_TBLSPLIT_ERROR              ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 56 )
#define ERR_TBLINSCOL_ERROR             ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 57 )
#define ERR_TBLDDECHG_ERROR             ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 58 )
#define WARN_SWG_FEATURES_LOST          ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::NONE, 72 )
#define WARN_SWG_POOR_LOAD              ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 74 )
#define WARN_SWG_HTML_NO_MACROS         ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 75)
#define WARN_WRITE_ERROR_FILE           ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 76)
#define WARN_FORMAT_FILE_ROWCOL         ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Read, 77)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
