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

// ErrCodeClass::Read - file related, displays "Read-Error" in MsgBox
#define SCERR_IMPORT_CONNECT        ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  1 )
#define SCERR_IMPORT_OPEN           ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  2 )
#define SCERR_IMPORT_UNKNOWN        ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  3 )
  // out of memory
#define SCERR_IMPORT_OUTOFMEM       ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  4 )
  // unknown WK? format (Lotus 1-2-3)
#define SCERR_IMPORT_UNKNOWN_WK     ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  5 )
  // format error during reading (no formula error!)
#define SCERR_IMPORT_FORMAT         ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  6 )
  // filter not implemented
#define SCERR_IMPORT_NI             ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  7 )
  // unknown BIFF format (Excel)
#define SCERR_IMPORT_UNKNOWN_BIFF   ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  8 )
#define SCERR_IMPORT_NI_BIFF        ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read,  9 )
  // file password protected
#define SCERR_IMPORT_FILEPASSWD     ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read, 10 )
  // internal error
#define SCERR_IMPORT_INTERNAL       ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read, 11 )
#define SCERR_IMPORT_8K_LIMIT       ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read, 12 )
#define SCWARN_IMPORT_OPEN_FM3      ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrorCodeClass::Read, 13 )
#define SCWARN_IMPORT_WRONG_FM3     ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrorCodeClass::Read, 14 )
#define SCWARN_IMPORT_INFOLOST      ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrorCodeClass::Read, 15 )
#define SCERR_IMPORT_FILE_ROWCOL    ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read, 16 )
#define SCERR_IMPORT_FORMAT_ROWCOL  ErrCode( ErrCodeArea::Sc, ErrorCodeClass::Read, 17 )
#define SCWARN_IMPORT_FILE_ROWCOL   ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrorCodeClass::Read, 18 )

// ErrCodeClass::Write - file related, displays "Write-Error" in MsgBox
#define SCERR_EXPORT_CONNECT        ErrCode( ErrCodeArea::Sc,  1 | ErrCodeClass::Write )
#define SCERR_EXPORT_DATA           ErrCode( ErrCodeArea::Sc,  2 | ErrCodeClass::Write )
#define SCERR_EXPORT_ENCODING       ErrCode( ErrCodeArea::Sc,  3 | ErrCodeClass::Write )
#define SCERR_EXPORT_FIELDWIDTH     ErrCode( ErrCodeArea::Sc,  4 | ErrCodeClass::Write )
#define SCERR_EXPORT_SQLEXCEPTION   ErrCode( ErrCodeArea::Sc,  5 | ErrCodeClass::Write )

// ErrCodeClass::Import - does not display "Read-Error" in MsgBox
    // overflow of cell coordinates
    // table restricted to valid area (?)
#define SCWARN_IMPORT_RANGE_OVERFLOW  ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 1 )
#define SCWARN_IMPORT_ROW_OVERFLOW    ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 2 )
#define SCWARN_IMPORT_COLUMN_OVERFLOW ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 3 )
#define SCWARN_IMPORT_SHEET_OVERFLOW  ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 4 )
#define SCWARN_IMPORT_CELL_OVERFLOW   ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 5 )
#define SCWARN_IMPORT_FEATURES_LOST   ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Import, 6 )

// ErrCodeClass::Export - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_NONCONVERTIBLE_CHARS  \
                                    ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 1 )
#define SCWARN_EXPORT_ASCII         ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 2 )
#define SCWARN_EXPORT_MAXROW        ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 3 )
#define SCWARN_EXPORT_MAXCOL        ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 4 )
#define SCWARN_EXPORT_MAXTAB        ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 5 )
#define SCWARN_EXPORT_DATALOST      ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::Export, 6 )

// ErrCodeClass::General
#define SCWARN_CORE_HARD_RECALC     ErrCode( IsWarning::Yes, ErrCodeArea::Sc, ErrCodeClass::General, 1 )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
