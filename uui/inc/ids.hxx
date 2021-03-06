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

#pragma once

#include <vcl/errcode.hxx>

#define ERRCODE_UUI_IO_ABORT                                    ErrCode(ErrCodeArea::Uui, 0)
#define ERRCODE_UUI_IO_ACCESSDENIED                             ErrCode(ErrCodeArea::Uui, 1)
#define ERRCODE_UUI_IO_ALREADYEXISTS                            ErrCode(ErrCodeArea::Uui, 2)
#define ERRCODE_UUI_IO_BADCRC                                   ErrCode(ErrCodeArea::Uui, 3)
#define ERRCODE_UUI_IO_CANTCREATE                               ErrCode(ErrCodeArea::Uui, 4)
#define ERRCODE_UUI_IO_CANTREAD                                 ErrCode(ErrCodeArea::Uui, 5)
#define ERRCODE_UUI_IO_CANTSEEK                                 ErrCode(ErrCodeArea::Uui, 6)
#define ERRCODE_UUI_IO_CANTTELL                                 ErrCode(ErrCodeArea::Uui, 7)
#define ERRCODE_UUI_IO_CANTWRITE                                ErrCode(ErrCodeArea::Uui, 8)
#define ERRCODE_UUI_IO_CURRENTDIR                               ErrCode(ErrCodeArea::Uui, 9)
#define ERRCODE_UUI_IO_NOTREADY                                 ErrCode(ErrCodeArea::Uui, 10)
#define ERRCODE_UUI_IO_NOTSAMEDEVICE                            ErrCode(ErrCodeArea::Uui, 11)
#define ERRCODE_UUI_IO_GENERAL                                  ErrCode(ErrCodeArea::Uui, 12)
#define ERRCODE_UUI_IO_INVALIDACCESS                            ErrCode(ErrCodeArea::Uui, 13)
#define ERRCODE_UUI_IO_INVALIDCHAR                              ErrCode(ErrCodeArea::Uui, 14)
#define ERRCODE_UUI_IO_INVALIDDEVICE                            ErrCode(ErrCodeArea::Uui, 15)
#define ERRCODE_UUI_IO_INVALIDLENGTH                            ErrCode(ErrCodeArea::Uui, 16)
#define ERRCODE_UUI_IO_INVALIDPARAMETER                         ErrCode(ErrCodeArea::Uui, 17)
#define ERRCODE_UUI_IO_ISWILDCARD                               ErrCode(ErrCodeArea::Uui, 18)
#define ERRCODE_UUI_IO_LOCKVIOLATION                            ErrCode(ErrCodeArea::Uui, 19)
#define ERRCODE_UUI_IO_MISPLACEDCHAR                            ErrCode(ErrCodeArea::Uui, 20)
#define ERRCODE_UUI_IO_NAMETOOLONG                              ErrCode(ErrCodeArea::Uui, 21)
#define ERRCODE_UUI_IO_NOTEXISTS                                ErrCode(ErrCodeArea::Uui, 22)
#define ERRCODE_UUI_IO_NOTEXISTSPATH                            ErrCode(ErrCodeArea::Uui, 23)
#define ERRCODE_UUI_IO_NOTSUPPORTED                             ErrCode(ErrCodeArea::Uui, 24)
#define ERRCODE_UUI_IO_NOTADIRECTORY                            ErrCode(ErrCodeArea::Uui, 25)
#define ERRCODE_UUI_IO_NOTAFILE                                 ErrCode(ErrCodeArea::Uui, 26)
#define ERRCODE_UUI_IO_OUTOFSPACE                               ErrCode(ErrCodeArea::Uui, 27)
#define ERRCODE_UUI_IO_TOOMANYOPENFILES                         ErrCode(ErrCodeArea::Uui, 28)
#define ERRCODE_UUI_IO_OUTOFMEMORY                              ErrCode(ErrCodeArea::Uui, 29)
#define ERRCODE_UUI_IO_PENDING                                  ErrCode(ErrCodeArea::Uui, 30)
#define ERRCODE_UUI_IO_RECURSIVE                                ErrCode(ErrCodeArea::Uui, 31)
#define ERRCODE_UUI_IO_UNKNOWN                                  ErrCode(ErrCodeArea::Uui, 32)
#define ERRCODE_UUI_IO_WRITEPROTECTED                           ErrCode(ErrCodeArea::Uui, 33)
#define ERRCODE_UUI_IO_WRONGFORMAT                              ErrCode(ErrCodeArea::Uui, 34)
#define ERRCODE_UUI_IO_WRONGVERSION                             ErrCode(ErrCodeArea::Uui, 35)
#define ERRCODE_UUI_IO_NOTEXISTS_VOLUME                         ErrCode(ErrCodeArea::Uui, 36)
#define ERRCODE_UUI_IO_NOTEXISTS_FOLDER                         ErrCode(ErrCodeArea::Uui, 37)
#define ERRCODE_UUI_WRONGJAVA                                   ErrCode(ErrCodeArea::Uui, 38)
#define ERRCODE_UUI_WRONGJAVA_VERSION                           ErrCode(ErrCodeArea::Uui, 39)
#define ERRCODE_UUI_WRONGJAVA_MIN                               ErrCode(ErrCodeArea::Uui, 40)
#define ERRCODE_UUI_WRONGJAVA_VERSION_MIN                       ErrCode(ErrCodeArea::Uui, 41)
#define ERRCODE_UUI_BADPARTNERSHIP                              ErrCode(ErrCodeArea::Uui, 42)
#define ERRCODE_UUI_BADPARTNERSHIP_NAME                         ErrCode(ErrCodeArea::Uui, 43)
#define ERRCODE_UUI_IO_NOTREADY_VOLUME                          ErrCode(ErrCodeArea::Uui, 44)
#define ERRCODE_UUI_IO_NOTREADY_REMOVABLE                       ErrCode(ErrCodeArea::Uui, 45)
#define ERRCODE_UUI_IO_NOTREADY_VOLUME_REMOVABLE                ErrCode(ErrCodeArea::Uui, 46)
#define ERRCODE_UUI_WRONGMEDIUM                                 ErrCode(ErrCodeArea::Uui, 47)
#define ERRCODE_UUI_IO_CANTCREATE_NONAME                        ErrCode(ErrCodeArea::Uui, 48)
#define ERRCODE_UUI_IO_TARGETALREADYEXISTS                      ErrCode(ErrCodeArea::Uui, 49)
#define ERRCODE_UUI_IO_UNSUPPORTEDOVERWRITE                     ErrCode(ErrCodeArea::Uui, 50)
#define ERRCODE_UUI_IO_BROKENPACKAGE                            ErrCode(ErrCodeArea::Uui, 51)
#define ERRCODE_UUI_IO_BROKENPACKAGE_CANTREPAIR                 ErrCode(ErrCodeArea::Uui, 52)
#define ERRCODE_UUI_CONFIGURATION_BROKENDATA_NOREMOVE           ErrCode(ErrCodeArea::Uui, 53)
#define ERRCODE_UUI_CONFIGURATION_BROKENDATA_WITHREMOVE         ErrCode(ErrCodeArea::Uui, 54)
#define ERRCODE_UUI_CONFIGURATION_BACKENDMISSING                ErrCode(ErrCodeArea::Uui, 55)
#define ERRCODE_UUI_CONFIGURATION_BACKENDMISSING_WITHRECOVER    ErrCode(ErrCodeArea::Uui, 56)
#define ERRCODE_UUI_INVALID_XFORMS_SUBMISSION_DATA              ErrCode(ErrCodeArea::Uui, 57)
#define ERRCODE_UUI_IO_MODULESIZEEXCEEDED                       ErrCode(ErrCodeArea::Uui, 58)
#define ERRCODE_UUI_LOCKING_LOCKED                              ErrCode(ErrCodeArea::Uui, 59)
#define ERRCODE_UUI_LOCKING_LOCKED_SELF                         ErrCode(ErrCodeArea::Uui, 60)
#define ERRCODE_UUI_LOCKING_NOT_LOCKED                          ErrCode(ErrCodeArea::Uui, 61)
#define ERRCODE_UUI_LOCKING_LOCK_EXPIRED                        ErrCode(ErrCodeArea::Uui, 62)
#define ERRCODE_UUI_CANNOT_ACTIVATE_FACTORY                     ErrCode(ErrCodeArea::Uui, 63)
#define ERRCODE_UUI_IO_EXOTICFILEFORMAT                         ErrCode(ErrCodeArea::Uui, 64)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
