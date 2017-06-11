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
#ifndef INCLUDED_SVX_SVXERR_HXX
#define INCLUDED_SVX_SVXERR_HXX

#include <vcl/errcode.hxx>

// define ----------------------------------------------------------------

#define ERRCODE_SVX_LINGU_THESAURUSNOTEXISTS   ErrCode(1UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS)

#define ERRCODE_SVX_LINGU_LINGUNOTEXISTS       ErrCode(3UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_HYPHENNOTEXISTS      ErrCode(4UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_DICT_NOTREADABLE     ErrCode(5UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
#define ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE    ErrCode(6UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_GRAPHIC_NOTREADABLE        ErrCode(7UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )

#define ERRCODE_SVX_LINGU_NOLANGUAGE           ErrCode(9UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_FORMS_NOIOSERVICES         ErrCode(10UL | ERRCODE_AREA_SVX )
#define ERRCODE_SVX_FORMS_READWRITEFAILED      ErrCode(11UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_BULLETITEM_NOBULLET        ErrCode(12UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_MODIFIED_VBASIC_STORAGE    ErrCode(13UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_VBASIC_STORAGE_EXIST       ErrCode(14UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

/** Error message: "Wrong password." */
#define ERRCODE_SVX_WRONGPASS               ErrCode(15UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_NONE)

/** Error message: "Read error. Unsupported encryption method." */
#define ERRCODE_SVX_READ_FILTER_CRYPT       ErrCode(16UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Read error. Passwort encrypted Powerpoint documents..." */
#define ERRCODE_SVX_READ_FILTER_PPOINT      ErrCode(17UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Warning. Passwort protection is not supported when..." */
#define ERRCODE_SVX_EXPORT_FILTER_CRYPT      ErrCode(18UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK)


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
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC SvxErrorHandler : private SfxErrorHandler
{
public:
    SvxErrorHandler();
    static void ensure();
};

SVX_DLLPUBLIC const ErrMsgCode* getRID_SVXERRCODE();
SVX_DLLPUBLIC const ErrMsgCode* getRID_SVXERRCTX();

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
