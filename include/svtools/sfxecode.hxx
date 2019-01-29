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
#ifndef INCLUDED_SVTOOLS_SFXECODE_HXX
#define INCLUDED_SVTOOLS_SFXECODE_HXX

class ErrCode;

#define ERRCODE_SFX_NOTATEMPLATE            ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Format, 2)
#define ERRCODE_SFX_GENERAL                 ErrCode(ErrCodeArea::Sfx, ErrCodeClass::General, 3)
#define ERRCODE_SFX_DOLOADFAILED            ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Read, 4)
#define ERRCODE_SFX_ALREADYOPEN             ErrCode(ErrCodeArea::Sfx, ErrCodeClass::AlreadyExists, 10)
#define ERRCODE_SFX_WRONGPASSWORD           ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Read, 11)
#define ERRCODE_SFX_DOCUMENTREADONLY        ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Write, 12)
#define ERRCODE_SFX_OLEGENERAL              ErrCode(ErrCodeArea::Sfx, ErrCodeClass::NONE, 14)
#define ERRCODE_SFX_TEMPLATENOTFOUND        ErrCode(ErrCodeArea::Sfx, ErrCodeClass::NotExists, 15)

#define ERRCODE_SFX_CANTCREATECONTENT       ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Create, 21)
#define ERRCODE_SFX_INVALIDSYNTAX           ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Path, 36)
#define ERRCODE_SFX_CANTGETPASSWD           ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Read, 42)
#define ERRCODE_SFX_NOMOREDOCUMENTSALLOWED  ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 44)
#define ERRCODE_SFX_CANTCREATEBACKUP        ErrCode(ErrCodeArea::Sfx, ErrCodeClass::Create, 50)
#define ERRCODE_SFX_MACROS_SUPPORT_DISABLED ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 51)
#define ERRCODE_SFX_DOCUMENT_MACRO_DISABLED ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 52)
#define ERRCODE_SFX_SHARED_NOPASSWORDCHANGE ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 54)
#define ERRCODE_SFX_INCOMPLETE_ENCRYPTION   ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 55)
#define ERRCODE_SFX_DOCUMENT_MACRO_DISABLED_MAC \
                                            ErrCode(WarningFlag::Yes, ErrCodeArea::Sfx, ErrCodeClass::NONE, 56)
#define ERRCODE_SFX_FORMAT_ROWCOL           ErrCode(ErrCodeArea::Sfx, ErrCodeClass::NONE, 57)


// Various
#define ERRCTX_ERROR                    21
#define ERRCTX_WARNING                  22

// Document
#define ERRCTX_SFX_LOADTEMPLATE         1
#define ERRCTX_SFX_SAVEDOC              2
#define ERRCTX_SFX_SAVEASDOC            3
#define ERRCTX_SFX_DOCINFO              4
#define ERRCTX_SFX_DOCTEMPLATE          5
#define ERRCTX_SFX_MOVEORCOPYCONTENTS   6

// Application
#define ERRCTX_SFX_DOCMANAGER           50
#define ERRCTX_SFX_OPENDOC              51
#define ERRCTX_SFX_NEWDOCDIRECT         52
#define ERRCTX_SFX_NEWDOC               53

// Organizer
#define ERRCTX_SFX_CREATEOBJSH          70

// BASIC
#define ERRCTX_SFX_LOADBASIC            80

// Addressbook
#define ERRCTX_SFX_SEARCHADDRESS        90

#endif // INCLUDED_SVTOOLS_SFXECODE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
