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

#include <vcl/errcode.hxx>

#define ERROR_SW_READ_BASE      (ERRCODE_AREA_SW | ERRCODE_CLASS_READ)
#define ERROR_SW_WRITE_BASE     (ERRCODE_AREA_SW | ERRCODE_CLASS_WRITE)

#define WARN_SW_BASE            (ERRCODE_AREA_SW | ERRCODE_WARNING_MASK)
#define WARN_SW_READ_BASE       (WARN_SW_BASE | ERRCODE_CLASS_READ )
#define WARN_SW_WRITE_BASE      (WARN_SW_BASE | ERRCODE_CLASS_WRITE )


// Import Errors
#define ERR_SWG_FILE_FORMAT_ERROR       ErrCode(ERROR_SW_READ_BASE | 1 )
#define ERR_SWG_READ_ERROR              ErrCode(ERROR_SW_READ_BASE | 2 )

#define ERR_SW6_INPUT_FILE              ErrCode(ERROR_SW_READ_BASE | 4 )
#define ERR_SW6_NOWRITER_FILE           ErrCode(ERROR_SW_READ_BASE | 5 )
#define ERR_SW6_UNEXPECTED_EOF          ErrCode(ERROR_SW_READ_BASE | 6 )
#define ERR_SW6_PASSWD                  ErrCode(ERROR_SW_READ_BASE | 7 )

#define ERR_WW6_NO_WW6_FILE_ERR         ErrCode(ERROR_SW_READ_BASE | 8 )
#define ERR_WW6_FASTSAVE_ERR            ErrCode(ERROR_SW_READ_BASE | 9 )

#define ERR_FORMAT_ROWCOL               ErrCode(ERROR_SW_READ_BASE | 12)

#define ERR_SWG_NEW_VERSION             ErrCode(ERROR_SW_READ_BASE | 13)
#define ERR_WW8_NO_WW8_FILE_ERR         ErrCode(ERROR_SW_READ_BASE | 14)

#define ERR_FORMAT_FILE_ROWCOL          ErrCode(ERROR_SW_READ_BASE | 15)

// Export errors
#define ERR_SWG_WRITE_ERROR             ErrCode(ERROR_SW_WRITE_BASE | 30 )
#define   ERR_SWG_OLD_GLOSSARY          ErrCode(ERROR_SW_WRITE_BASE | 31 )
#define ERR_WRITE_ERROR_FILE            ErrCode(ERROR_SW_WRITE_BASE | 35 )

// Import/Export errors
#define ERR_SWG_INTERNAL_ERROR          ErrCode(ERRCODE_AREA_SW  | 50 )

#define ERR_TXTBLOCK_NEWFILE_ERROR      ErrCode(ERRCODE_AREA_SW|ERRCODE_CLASS_LOCKING | 55 )

// other errors and errorclasses
#define ERR_AUTOPATH_ERROR              ErrCode(ERRCODE_AREA_SW|ERRCODE_CLASS_PATH | 55 )
#define ERR_TBLSPLIT_ERROR              ErrCode(ERRCODE_AREA_SW|ERRCODE_CLASS_NONE| 56 )
#define ERR_TBLINSCOL_ERROR             ErrCode(ERRCODE_AREA_SW|ERRCODE_CLASS_NONE| 57 )
#define ERR_TBLDDECHG_ERROR             ErrCode(ERRCODE_AREA_SW|ERRCODE_CLASS_NONE| 58 )

// Import - Warnings
#define WARN_SWG_NO_DRAWINGS            ErrCode(WARN_SW_READ_BASE | 70 )
#define WARN_WW6_FASTSAVE_ERR           ErrCode(WARN_SW_READ_BASE | 71 )
// continued below

// Import & Export - Warnings
#define WARN_SWG_FEATURES_LOST          ErrCode(WARN_SW_BASE | 72 )
#define WARN_SWG_OLE                    ErrCode(WARN_SW_BASE | 73 )
#define WARN_SWG_POOR_LOAD              (WARN_SW_BASE | 74 )

// Export warnings
#define WARN_SWG_HTML_NO_MACROS         ErrCode(WARN_SW_WRITE_BASE |75)
#define WARN_WRITE_ERROR_FILE           ErrCode(WARN_SW_WRITE_BASE |76)

// More Import & Export  - Warnings
#define WARN_FORMAT_FILE_ROWCOL         ErrCode(WARN_SW_READ_BASE | 77)


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
