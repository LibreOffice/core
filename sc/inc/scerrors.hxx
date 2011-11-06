/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SCERRORS_HXX
#define _SCERRORS_HXX

#include <tools/errcode.hxx>

// ERRCODE_CLASS_READ - file related, displays "Read-Error" in MsgBox
#define SCERR_IMPORT_CONNECT        (   1 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_OPEN           (   2 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_UNKNOWN        (   3 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_OUTOFMEM       (   4 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_UNKNOWN_WK     (   5 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FORMAT         (   6 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_NI             (   7 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_UNKNOWN_BIFF   (   8 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_NI_BIFF        (   9 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FILEPASSWD     (  10 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_INTERNAL       (  11 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_8K_LIMIT       (  12 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_OPEN_FM3      (  13 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_WRONG_FM3     (  14 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_INFOLOST      (  15 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FILE_ROWCOL    (  16 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FORMAT_ROWCOL  (  17 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_FILE_ROWCOL   (  18 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_WRITE - file related, displays "Write-Error" in MsgBox
#define SCERR_EXPORT_CONNECT        (   1 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_DATA           (   2 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_ENCODING       (   3 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_FIELDWIDTH     (   4 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )
#define SCERR_EXPORT_SQLEXCEPTION   (   5 | ERRCODE_CLASS_WRITE | ERRCODE_AREA_SC )

// ERRCODE_CLASS_IMPORT - does not display "Read-Error" in MsgBox
#define SCWARN_IMPORT_RANGE_OVERFLOW (  1 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_ROW_OVERFLOW (  2 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_COLUMN_OVERFLOW (  3 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_SHEET_OVERFLOW (  4 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_EXPORT - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_NONCONVERTIBLE_CHARS  (   1 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_ASCII         (   2 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_MAXROW        (   3 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_EXPORT_DATALOST      (   4 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_GENERAL
#define SCWARN_CORE_HARD_RECALC     (   1 | ERRCODE_CLASS_GENERAL | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

#endif

