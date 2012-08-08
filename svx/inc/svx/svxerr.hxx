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
#ifndef _SVXERR_HXX
#define _SVXERR_HXX

#include <tools/errcode.hxx>

// define ----------------------------------------------------------------

#define ERRCODE_SVX_LINGU_THESAURUSNOTEXISTS   (1UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS)

#define ERRCODE_SVX_LINGU_LINGUNOTEXISTS       (3UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_HYPHENNOTEXISTS      (4UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_DICT_NOTREADABLE     (5UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
#define ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE    (6UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_GRAPHIC_NOTREADABLE        (7UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )

#define ERRCODE_SVX_LINGU_NOLANGUAGE           (9UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_FORMS_NOIOSERVICES         (10UL | ERRCODE_AREA_SVX )
#define ERRCODE_SVX_FORMS_READWRITEFAILED      (11UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_BULLETITEM_NOBULLET        (12UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_MODIFIED_VBASIC_STORAGE    (13UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_VBASIC_STORAGE_EXIST       (14UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

/** Error message: "Wrong password." */
#define ERRCODE_SVX_WRONGPASS               (15UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_NONE)

/** Error message: "Read error. Unsupported encryption method." */
#define ERRCODE_SVX_READ_FILTER_CRYPT       (16UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Read error. Passwort encrypted Powerpoint documents..." */
#define ERRCODE_SVX_READ_FILTER_PPOINT      (17UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Warning. Passwort protection is not suppported when ..." */
#define ERRCODE_SVX_EXPORT_FILTER_CRYPT      (18UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK)


// both codes will be used twice : with ERRCODE_CLASS_READ- and ERRCODE_CLASS_WRITE-bits
#define ERRCTX_SVX_LINGU_THESAURUS              1
#define ERRCTX_SVX_LINGU_SPELLING               2
#define ERRCTX_SVX_LINGU_HYPHENATION            3
#define ERRCTX_SVX_LINGU_DICTIONARY             4
#define ERRCTX_SVX_BACKGROUND                   5
#define ERRCTX_SVX_IMPORT_GRAPHIC               6

// class SvxErrorHandler -------------------------------------------------

#ifndef __RSC

#include <svtools/ehdl.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SvxErrorHandler : private SfxErrorHandler
{
public:
    SvxErrorHandler();
    static void ensure();
};

#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
