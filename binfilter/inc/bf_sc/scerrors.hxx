/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SCERRORS_HXX
#define _SCERRORS_HXX


// ERRCODE_CLASS_READ - file related, displays "Read-Error" in MsgBox
#define SCERR_IMPORT_CONNECT        (   1 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_OPEN           (   2 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_UNKNOWN        (   3 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FORMAT         (   6 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_NI             (   7 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_8K_LIMIT       (  12 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_INFOLOST      (  15 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FILE_ROWCOL    (  16 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCERR_IMPORT_FORMAT_ROWCOL  (  17 | ERRCODE_CLASS_READ | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_FILE_ROWCOL   (  18 | ERRCODE_CLASS_READ | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_WRITE - file related, displays "Write-Error" in MsgBox

// ERRCODE_CLASS_IMPORT - does not display "Read-Error" in MsgBox
#define SCWARN_IMPORT_RANGE_OVERFLOW (  1 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_ROW_OVERFLOW (  2 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_COLUMN_OVERFLOW (  3 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )
#define SCWARN_IMPORT_SHEET_OVERFLOW (  4 | ERRCODE_CLASS_IMPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_EXPORT - does not display "Write-Error" in MsgBox
#define SCWARN_EXPORT_MAXROW        (   3 | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

// ERRCODE_CLASS_GENERAL
#define SCWARN_CORE_HARD_RECALC     (   1 | ERRCODE_CLASS_GENERAL | ERRCODE_WARNING_MASK | ERRCODE_AREA_SC )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
