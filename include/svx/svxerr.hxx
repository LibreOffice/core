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

#define ERRCODE_SVX_LINGU_THESAURUSNOTEXISTS  ErrCode( ErrCodeArea::Svx,  1UL | ERRCODE_CLASS_NOTEXISTS)
#define ERRCODE_SVX_LINGU_LINGUNOTEXISTS      ErrCode( ErrCodeArea::Svx,  3UL | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_HYPHENNOTEXISTS     ErrCode( ErrCodeArea::Svx,  4UL | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_DICT_NOTREADABLE    ErrCode( ErrCodeArea::Svx,  5UL | ERRCODE_CLASS_READ )
#define ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE   ErrCode( ErrCodeArea::Svx,  6UL | ERRCODE_CLASS_WRITE )
#define ERRCODE_SVX_GRAPHIC_NOTREADABLE       ErrCode( ErrCodeArea::Svx,  7UL | ERRCODE_CLASS_READ )
#define ERRCODE_SVX_LINGU_NOLANGUAGE          ErrCode( ErrCodeArea::Svx,  9UL | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_FORMS_NOIOSERVICES        ErrCode( ErrCodeArea::Svx, 10UL )
#define ERRCODE_SVX_FORMS_READWRITEFAILED     ErrCode( ErrCodeArea::Svx, 11UL )
#define ERRCODE_SVX_BULLETITEM_NOBULLET       ErrCode( ErrCodeArea::Svx, 12UL )
#define ERRCODE_SVX_MODIFIED_VBASIC_STORAGE   ErrCode( ErrCodeArea::Svx, 13UL | ERRCODE_CLASS_WRITE | ERRCODE_WARNING_MASK )
#define ERRCODE_SVX_VBASIC_STORAGE_EXIST      ErrCode( ErrCodeArea::Svx, 14UL | ERRCODE_CLASS_WRITE | ERRCODE_WARNING_MASK )
/** Error message: "Wrong password." */
#define ERRCODE_SVX_WRONGPASS                 ErrCode( ErrCodeArea::Svx, 15UL | ERRCODE_CLASS_NONE)
/** Error message: "Read error. Unsupported encryption method." */
#define ERRCODE_SVX_READ_FILTER_CRYPT         ErrCode( ErrCodeArea::Svx, 16UL | ERRCODE_CLASS_READ)
/** Error message: "Read error. Passwort encrypted Powerpoint documents..." */
#define ERRCODE_SVX_READ_FILTER_PPOINT        ErrCode( ErrCodeArea::Svx, 17UL | ERRCODE_CLASS_READ)
/** Error message: "Warning. Passwort protection is not supported when..." */
#define ERRCODE_SVX_EXPORT_FILTER_CRYPT       ErrCode( ErrCodeArea::Svx, 18UL | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK)


// both codes will be used twice : with ERRCODE_CLASS_READ- and ERRCODE_CLASS_WRITE-bits
#define ERRCTX_SVX_LINGU_THESAURUS              1
#define ERRCTX_SVX_LINGU_SPELLING               2
#define ERRCTX_SVX_LINGU_HYPHENATION            3
#define ERRCTX_SVX_LINGU_DICTIONARY             4
#define ERRCTX_SVX_BACKGROUND                   5
#define ERRCTX_SVX_IMPORT_GRAPHIC               6

// class SvxErrorHandler -------------------------------------------------

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
