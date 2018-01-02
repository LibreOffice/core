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
#include <svtools/ehdl.hxx>
#include <svx/svxdllapi.h>

#define ERRCODE_SVX_LINGU_LINGUNOTEXISTS      ErrCode( ErrCodeArea::Svx, ErrCodeClass::NotExists, 3 )
#define ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE   ErrCode( ErrCodeArea::Svx, ErrCodeClass::Write, 6 )
#define ERRCODE_SVX_GRAPHIC_NOTREADABLE       ErrCode( ErrCodeArea::Svx, ErrCodeClass::Read, 7 )
#define ERRCODE_SVX_LINGU_NOLANGUAGE          ErrCode( ErrCodeArea::Svx, ErrCodeClass::NotExists, 9 )
#define ERRCODE_SVX_MODIFIED_VBASIC_STORAGE   ErrCode( WarningFlag::Yes, ErrCodeArea::Svx, ErrCodeClass::Write, 13 )
#define ERRCODE_SVX_VBASIC_STORAGE_EXIST      ErrCode( WarningFlag::Yes, ErrCodeArea::Svx, ErrCodeClass::Write, 14 )
/** Error message: "Wrong password." */
#define ERRCODE_SVX_WRONGPASS                 ErrCode( ErrCodeArea::Svx, ErrCodeClass::NONE, 15)
/** Error message: "Read error. Unsupported encryption method." */
#define ERRCODE_SVX_READ_FILTER_CRYPT         ErrCode( ErrCodeArea::Svx, ErrCodeClass::Read, 16)
/** Error message: "Read error. Passwort encrypted Powerpoint documents..." */
#define ERRCODE_SVX_READ_FILTER_PPOINT        ErrCode( ErrCodeArea::Svx, ErrCodeClass::Read, 17)
/** Error message: "Warning. Passwort protection is not supported when..." */
#define ERRCODE_SVX_EXPORT_FILTER_CRYPT       ErrCode( WarningFlag::Yes, ErrCodeArea::Svx, ErrCodeClass::Export, 18)


// both codes will be used twice : with ErrCodeClass::Read- and ErrCodeClass::Write-bits
#define ERRCTX_SVX_LINGU_THESAURUS              1
#define ERRCTX_SVX_LINGU_SPELLING               2
#define ERRCTX_SVX_LINGU_HYPHENATION            3
#define ERRCTX_SVX_LINGU_DICTIONARY             4
#define ERRCTX_SVX_BACKGROUND                   5
#define ERRCTX_SVX_IMPORT_GRAPHIC               6

class SVX_DLLPUBLIC SvxErrorHandler : private SfxErrorHandler
{
public:
    SvxErrorHandler();
    static void ensure();
};

SVX_DLLPUBLIC extern const ErrMsgCode RID_SVXERRCODE[];
SVX_DLLPUBLIC extern const ErrMsgCode RID_SVXERRCTX[];

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
