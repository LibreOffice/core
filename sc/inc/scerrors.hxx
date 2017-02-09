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

#ifndef INCLUDED_SC_INC_SCERRORS_HXX
#define INCLUDED_SC_INC_SCERRORS_HXX

#include <vcl/errcode.hxx>

// ERRCODE_CLASS_READ - file related, displays "Read-Error" in MsgBox
#define SCERR_IMPORT_CONNECT        ErrCode(   1 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_OPEN           ErrCode(   2 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_UNKNOWN        ErrCode(   3 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // out of memory
#define SCERR_IMPORT_OUTOFMEM       ErrCode(   4 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // unknown WK? format (Lotus 1-2-3)
#define SCERR_IMPORT_UNKNOWN_WK     ErrCode(   5 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // format error during reading (no formula error!)
#define SCERR_IMPORT_FORMAT         ErrCode(   6 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // filter not implemented
#define SCERR_IMPORT_NI             ErrCode(   7 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // unknown BIFF format (Excel)
#define SCERR_IMPORT_UNKNOWN_BIFF   ErrCode(   8 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_NI_BIFF        ErrCode(   9 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // file password protected
#define SCERR_IMPORT_FILEPASSWD     ErrCode(  10 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
  // internal error
#define SCERR_IMPORT_INTERNAL       ErrCode(  11 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_8K_LIMIT       ErrCode(  12 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_OPEN_FM3      ErrCode(  13 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_WRONG_FM3     ErrCode(  14 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_INFOLOST      ErrCode(  15 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FILE_ROWCOL    ErrCode(  16 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FORMAT_ROWCOL  ErrCode(  17 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_FILE_ROWCOL   ErrCode(  18 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_WRITE - file related, displays "Write-Error" in MsgBox
#define SCERR_EXPORT_CONNECT        ErrCode(   1 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_DATA           ErrCode(   2 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_ENCODING       ErrCode(   3 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_FIELDWIDTH     ErrCode(   4 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_SQLEXCEPTION   ErrCode(   5 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )

// ERRCODE_CLASS_IMPORT - does not display "Read-Error" in MsgBox
    // overflow of cell coordinates
    // table restricted to valid area (?)
#define SCWARN_IMPORT_RANGE_OVERFLOW  ErrCode(  1 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_ROW_OVERFLOW    ErrCode(   2 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_COLUMN_OVERFLOW ErrCode( 3 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_SHEET_OVERFLOW  ErrCode(  4 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_CELL_OVERFLOW   ErrCode(   5 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_FEATURES_LOST   ErrCode(   6 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_EXPORT - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_NONCONVERTIBLE_CHARS  ErrCode(   1 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_ASCII         ErrCode(   2 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_MAXROW        ErrCode(   3 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_MAXCOL        ErrCode(   4 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_MAXTAB        ErrCode(   5 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_DATALOST      ErrCode(   6 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_GENERAL
#define SCWARN_CORE_HARD_RECALC     ErrCode(   1 | ERRCODE_CLASS_GENERAL | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
