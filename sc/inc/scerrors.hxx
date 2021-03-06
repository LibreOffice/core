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

#pragma once

#include <vcl/errcode.hxx>

// ErrCodeClass::Read - file related, displays "Read-Error" in MsgBox
#define SCERR_IMPORT_CONNECT        ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  1 )
#define SCERR_IMPORT_OPEN           ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  2 )
#define SCERR_IMPORT_UNKNOWN        ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  3 )
  // out of memory
#define SCERR_IMPORT_OUTOFMEM       ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  4 )
  // unknown WK? format (Lotus 1-2-3)
#define SCERR_IMPORT_UNKNOWN_WK     ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  5 )
  // format error during reading (no formula error!)
#define SCERR_IMPORT_FORMAT         ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  6 )
  // filter not implemented
#define SCERR_IMPORT_NI             ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  7 )
  // unknown BIFF format (Excel)
#define SCERR_IMPORT_UNKNOWN_BIFF   ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  8 )
#define SCERR_IMPORT_NI_BIFF        ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read,  9 )
  // file password protected
#define SCERR_IMPORT_FILEPASSWD     ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read, 10 )
  // internal error
#define SCERR_IMPORT_INTERNAL       ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read, 11 )
#define SCERR_IMPORT_8K_LIMIT       ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read, 12 )
#define SCWARN_IMPORT_OPEN_FM3      ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Read, 13 )
#define SCWARN_IMPORT_WRONG_FM3     ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Read, 14 )
#define SCWARN_IMPORT_INFOLOST      ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Read, 15 )
#define SCERR_IMPORT_FILE_ROWCOL    ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read, 16 )
#define SCERR_IMPORT_FORMAT_ROWCOL  ErrCode( ErrCodeArea::Sc, ErrCodeClass::Read, 17 )
#define SCWARN_IMPORT_FILE_ROWCOL   ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Read, 18 )

// ErrCodeClass::Write - file related, displays "Write-Error" in MsgBox
#define SCERR_EXPORT_CONNECT        ErrCode( ErrCodeArea::Sc,  ErrCodeClass::Write, 1 )
#define SCERR_EXPORT_DATA           ErrCode( ErrCodeArea::Sc,  ErrCodeClass::Write, 2 )
#define SCERR_EXPORT_ENCODING       ErrCode( ErrCodeArea::Sc,  ErrCodeClass::Write, 3 )
#define SCERR_EXPORT_FIELDWIDTH     ErrCode( ErrCodeArea::Sc,  ErrCodeClass::Write, 4 )
#define SCERR_EXPORT_SQLEXCEPTION   ErrCode( ErrCodeArea::Sc,  ErrCodeClass::Write, 5 )

// ErrCodeClass::Import - does not display "Read-Error" in MsgBox
    // overflow of cell coordinates
    // table restricted to valid area (?)
#define SCWARN_IMPORT_RANGE_OVERFLOW  ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 1 )
#define SCWARN_IMPORT_ROW_OVERFLOW    ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 2 )
#define SCWARN_IMPORT_COLUMN_OVERFLOW ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 3 )
#define SCWARN_IMPORT_SHEET_OVERFLOW  ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 4 )
#define SCWARN_IMPORT_CELL_OVERFLOW   ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 5 )
#define SCWARN_IMPORT_FEATURES_LOST   ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 6 )

// ErrCodeClass::Export - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_NONCONVERTIBLE_CHARS  \
                                    ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 1 )
#define SCWARN_EXPORT_ASCII         ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 2 )
#define SCWARN_EXPORT_MAXROW        ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 3 )
#define SCWARN_EXPORT_MAXCOL        ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 4 )
#define SCWARN_EXPORT_MAXTAB        ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 5 )
#define SCWARN_EXPORT_DATALOST      ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 6 )

// ErrCodeClass::General
#define SCWARN_CORE_HARD_RECALC     ErrCode( WarningFlag::Yes, ErrCodeArea::Sc, ErrCodeClass::General, 1 )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
