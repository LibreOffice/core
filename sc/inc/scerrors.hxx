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
#define SCERR_IMPORT_CONNECT        ErrCode( ErrCodeArea::Sc,  1 | ERRCODE_CLASS_READ )
#define SCERR_IMPORT_OPEN           ErrCode( ErrCodeArea::Sc,  2 | ERRCODE_CLASS_READ )
#define SCERR_IMPORT_UNKNOWN        ErrCode( ErrCodeArea::Sc,  3 | ERRCODE_CLASS_READ )
  // out of memory
#define SCERR_IMPORT_OUTOFMEM       ErrCode( ErrCodeArea::Sc,  4 | ERRCODE_CLASS_READ )
  // unknown WK? format (Lotus 1-2-3)
#define SCERR_IMPORT_UNKNOWN_WK     ErrCode( ErrCodeArea::Sc,  5 | ERRCODE_CLASS_READ )
  // format error during reading (no formula error!)
#define SCERR_IMPORT_FORMAT         ErrCode( ErrCodeArea::Sc,  6 | ERRCODE_CLASS_READ )
  // filter not implemented
#define SCERR_IMPORT_NI             ErrCode( ErrCodeArea::Sc,  7 | ERRCODE_CLASS_READ )
  // unknown BIFF format (Excel)
#define SCERR_IMPORT_UNKNOWN_BIFF   ErrCode( ErrCodeArea::Sc,  8 | ERRCODE_CLASS_READ )
#define SCERR_IMPORT_NI_BIFF        ErrCode( ErrCodeArea::Sc,  9 | ERRCODE_CLASS_READ )
  // file password protected
#define SCERR_IMPORT_FILEPASSWD     ErrCode( ErrCodeArea::Sc, 10 | ERRCODE_CLASS_READ )
  // internal error
#define SCERR_IMPORT_INTERNAL       ErrCode( ErrCodeArea::Sc, 11 | ERRCODE_CLASS_READ )
#define SCERR_IMPORT_8K_LIMIT       ErrCode( ErrCodeArea::Sc, 12 | ERRCODE_CLASS_READ )
#define SCWARN_IMPORT_OPEN_FM3      ErrCode( ErrCodeArea::Sc, 13 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_WRONG_FM3     ErrCode( ErrCodeArea::Sc, 14 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_INFOLOST      ErrCode( ErrCodeArea::Sc, 15 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK )
#define SCERR_IMPORT_FILE_ROWCOL    ErrCode( ErrCodeArea::Sc, 16 | ERRCODE_CLASS_READ )
#define SCERR_IMPORT_FORMAT_ROWCOL  ErrCode( ErrCodeArea::Sc, 17 | ERRCODE_CLASS_READ )
#define SCWARN_IMPORT_FILE_ROWCOL   ErrCode( ErrCodeArea::Sc, 18 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK )

// ERRCODE_CLASS_WRITE - file related, displays "Write-Error" in MsgBox
#define SCERR_EXPORT_CONNECT        ErrCode( ErrCodeArea::Sc,  1 | ERRCODE_CLASS_WRITE )
#define SCERR_EXPORT_DATA           ErrCode( ErrCodeArea::Sc,  2 | ERRCODE_CLASS_WRITE )
#define SCERR_EXPORT_ENCODING       ErrCode( ErrCodeArea::Sc,  3 | ERRCODE_CLASS_WRITE )
#define SCERR_EXPORT_FIELDWIDTH     ErrCode( ErrCodeArea::Sc,  4 | ERRCODE_CLASS_WRITE )
#define SCERR_EXPORT_SQLEXCEPTION   ErrCode( ErrCodeArea::Sc,  5 | ERRCODE_CLASS_WRITE )

// ERRCODE_CLASS_IMPORT - does not display "Read-Error" in MsgBox
    // overflow of cell coordinates
    // table restricted to valid area (?)
#define SCWARN_IMPORT_RANGE_OVERFLOW  ErrCode( ErrCodeArea::Sc, 1 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_ROW_OVERFLOW    ErrCode( ErrCodeArea::Sc, 2 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_COLUMN_OVERFLOW ErrCode( ErrCodeArea::Sc, 3 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_SHEET_OVERFLOW  ErrCode( ErrCodeArea::Sc, 4 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_CELL_OVERFLOW   ErrCode( ErrCodeArea::Sc, 5 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )
#define SCWARN_IMPORT_FEATURES_LOST   ErrCode( ErrCodeArea::Sc, 6 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK )

// ERRCODE_CLASS_EXPORT - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_NONCONVERTIBLE_CHARS  \
                                    ErrCode( ErrCodeArea::Sc,  1 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )
#define SCWARN_EXPORT_ASCII         ErrCode( ErrCodeArea::Sc,  2 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )
#define SCWARN_EXPORT_MAXROW        ErrCode( ErrCodeArea::Sc,  3 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )
#define SCWARN_EXPORT_MAXCOL        ErrCode( ErrCodeArea::Sc,  4 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )
#define SCWARN_EXPORT_MAXTAB        ErrCode( ErrCodeArea::Sc,  5 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )
#define SCWARN_EXPORT_DATALOST      ErrCode( ErrCodeArea::Sc,  6 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK )

// ERRCODE_CLASS_GENERAL
#define SCWARN_CORE_HARD_RECALC     ErrCode( ErrCodeArea::Sc,  1 | ERRCODE_CLASS_GENERAL | ERRCODE_WARNING_MASK )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
