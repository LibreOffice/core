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

#ifndef INCLUDED_UUI_INC_IDS_HXX
#define INCLUDED_UUI_INC_IDS_HXX

#include <vcl/errcode.hxx>

#define ERRCODE_UUI_IO_ABORT                                    ErrCode(ERRCODE_AREA_UUI + 0)
#define ERRCODE_UUI_IO_ACCESSDENIED                             ErrCode(ERRCODE_AREA_UUI + 1)
#define ERRCODE_UUI_IO_ALREADYEXISTS                            ErrCode(ERRCODE_AREA_UUI + 2)
#define ERRCODE_UUI_IO_BADCRC                                   ErrCode(ERRCODE_AREA_UUI + 3)
#define ERRCODE_UUI_IO_CANTCREATE                               ErrCode(ERRCODE_AREA_UUI + 4)
#define ERRCODE_UUI_IO_CANTREAD                                 ErrCode(ERRCODE_AREA_UUI + 5)
#define ERRCODE_UUI_IO_CANTSEEK                                 ErrCode(ERRCODE_AREA_UUI + 6)
#define ERRCODE_UUI_IO_CANTTELL                                 ErrCode(ERRCODE_AREA_UUI + 7)
#define ERRCODE_UUI_IO_CANTWRITE                                ErrCode(ERRCODE_AREA_UUI + 8)
#define ERRCODE_UUI_IO_CURRENTDIR                               ErrCode(ERRCODE_AREA_UUI + 9)
#define ERRCODE_UUI_IO_NOTREADY                                 ErrCode(ERRCODE_AREA_UUI + 10)
#define ERRCODE_UUI_IO_NOTSAMEDEVICE                            ErrCode(ERRCODE_AREA_UUI + 11)
#define ERRCODE_UUI_IO_GENERAL                                  ErrCode(ERRCODE_AREA_UUI + 12)
#define ERRCODE_UUI_IO_INVALIDACCESS                            ErrCode(ERRCODE_AREA_UUI + 13)
#define ERRCODE_UUI_IO_INVALIDCHAR                              ErrCode(ERRCODE_AREA_UUI + 14)
#define ERRCODE_UUI_IO_INVALIDDEVICE                            ErrCode(ERRCODE_AREA_UUI + 15)
#define ERRCODE_UUI_IO_INVALIDLENGTH                            ErrCode(ERRCODE_AREA_UUI + 16)
#define ERRCODE_UUI_IO_INVALIDPARAMETER                         ErrCode(ERRCODE_AREA_UUI + 17)
#define ERRCODE_UUI_IO_ISWILDCARD                               ErrCode(ERRCODE_AREA_UUI + 18)
#define ERRCODE_UUI_IO_LOCKVIOLATION                            ErrCode(ERRCODE_AREA_UUI + 19)
#define ERRCODE_UUI_IO_MISPLACEDCHAR                            ErrCode(ERRCODE_AREA_UUI + 20)
#define ERRCODE_UUI_IO_NAMETOOLONG                              ErrCode(ERRCODE_AREA_UUI + 21)
#define ERRCODE_UUI_IO_NOTEXISTS                                ErrCode(ERRCODE_AREA_UUI + 22)
#define ERRCODE_UUI_IO_NOTEXISTSPATH                            ErrCode(ERRCODE_AREA_UUI + 23)
#define ERRCODE_UUI_IO_NOTSUPPORTED                             ErrCode(ERRCODE_AREA_UUI + 24)
#define ERRCODE_UUI_IO_NOTADIRECTORY                            ErrCode(ERRCODE_AREA_UUI + 25)
#define ERRCODE_UUI_IO_NOTAFILE                                 ErrCode(ERRCODE_AREA_UUI + 26)
#define ERRCODE_UUI_IO_OUTOFSPACE                               ErrCode(ERRCODE_AREA_UUI + 27)
#define ERRCODE_UUI_IO_TOOMANYOPENFILES                         ErrCode(ERRCODE_AREA_UUI + 28)
#define ERRCODE_UUI_IO_OUTOFMEMORY                              ErrCode(ERRCODE_AREA_UUI + 29)
#define ERRCODE_UUI_IO_PENDING                                  ErrCode(ERRCODE_AREA_UUI + 30)
#define ERRCODE_UUI_IO_RECURSIVE                                ErrCode(ERRCODE_AREA_UUI + 31)
#define ERRCODE_UUI_IO_UNKNOWN                                  ErrCode(ERRCODE_AREA_UUI + 32)
#define ERRCODE_UUI_IO_WRITEPROTECTED                           ErrCode(ERRCODE_AREA_UUI + 33)
#define ERRCODE_UUI_IO_WRONGFORMAT                              ErrCode(ERRCODE_AREA_UUI + 34)
#define ERRCODE_UUI_IO_WRONGVERSION                             ErrCode(ERRCODE_AREA_UUI + 35)
#define ERRCODE_UUI_IO_NOTEXISTS_VOLUME                         ErrCode(ERRCODE_AREA_UUI + 36)
#define ERRCODE_UUI_IO_NOTEXISTS_FOLDER                         ErrCode(ERRCODE_AREA_UUI + 37)
#define ERRCODE_UUI_WRONGJAVA                                   ErrCode(ERRCODE_AREA_UUI + 38)
#define ERRCODE_UUI_WRONGJAVA_VERSION                           ErrCode(ERRCODE_AREA_UUI + 39)
#define ERRCODE_UUI_WRONGJAVA_MIN                               ErrCode(ERRCODE_AREA_UUI + 40)
#define ERRCODE_UUI_WRONGJAVA_VERSION_MIN                       ErrCode(ERRCODE_AREA_UUI + 41)
#define ERRCODE_UUI_BADPARTNERSHIP                              ErrCode(ERRCODE_AREA_UUI + 42)
#define ERRCODE_UUI_BADPARTNERSHIP_NAME                         ErrCode(ERRCODE_AREA_UUI + 43)
#define ERRCODE_UUI_IO_NOTREADY_VOLUME                          ErrCode(ERRCODE_AREA_UUI + 44)
#define ERRCODE_UUI_IO_NOTREADY_REMOVABLE                       ErrCode(ERRCODE_AREA_UUI + 45)
#define ERRCODE_UUI_IO_NOTREADY_VOLUME_REMOVABLE                ErrCode(ERRCODE_AREA_UUI + 46)
#define ERRCODE_UUI_WRONGMEDIUM                                 ErrCode(ERRCODE_AREA_UUI + 47)
#define ERRCODE_UUI_IO_CANTCREATE_NONAME                        ErrCode(ERRCODE_AREA_UUI + 48)
#define ERRCODE_UUI_IO_TARGETALREADYEXISTS                      ErrCode(ERRCODE_AREA_UUI + 49)
#define ERRCODE_UUI_IO_UNSUPPORTEDOVERWRITE                     ErrCode(ERRCODE_AREA_UUI + 50)
#define ERRCODE_UUI_IO_BROKENPACKAGE                            ErrCode(ERRCODE_AREA_UUI + 51)
#define ERRCODE_UUI_IO_BROKENPACKAGE_CANTREPAIR                 ErrCode(ERRCODE_AREA_UUI + 52)
#define ERRCODE_UUI_CONFIGURATION_BROKENDATA_NOREMOVE           ErrCode(ERRCODE_AREA_UUI + 53)
#define ERRCODE_UUI_CONFIGURATION_BROKENDATA_WITHREMOVE         ErrCode(ERRCODE_AREA_UUI + 54)
#define ERRCODE_UUI_CONFIGURATION_BACKENDMISSING                ErrCode(ERRCODE_AREA_UUI + 55)
#define ERRCODE_UUI_CONFIGURATION_BACKENDMISSING_WITHRECOVER    ErrCode(ERRCODE_AREA_UUI + 56)
#define ERRCODE_UUI_INVALID_XFORMS_SUBMISSION_DATA              ErrCode(ERRCODE_AREA_UUI + 57)
#define ERRCODE_UUI_IO_MODULESIZEEXCEEDED                       ErrCode(ERRCODE_AREA_UUI + 58)
#define ERRCODE_UUI_LOCKING_LOCKED                              ErrCode(ERRCODE_AREA_UUI + 59)
#define ERRCODE_UUI_LOCKING_LOCKED_SELF                         ErrCode(ERRCODE_AREA_UUI + 60)
#define ERRCODE_UUI_LOCKING_NOT_LOCKED                          ErrCode(ERRCODE_AREA_UUI + 61)
#define ERRCODE_UUI_LOCKING_LOCK_EXPIRED                        ErrCode(ERRCODE_AREA_UUI + 62)
#define ERRCODE_UUI_CANNOT_ACTIVATE_FACTORY                     ErrCode(ERRCODE_AREA_UUI + 63)
#define ERRCODE_AREA_UUI_UNKNOWNAUTH    25000
#define SSLWARN_TYPE_DOMAINMISMATCH     10
#define SSLWARN_TYPE_EXPIRED            20
#define SSLWARN_TYPE_INVALID            30

#define ERRCODE_UUI_UNKNOWNAUTH_UNTRUSTED ErrCode(ERRCODE_AREA_UUI_UNKNOWNAUTH + 1)

#define ERRCODE_UUI_SSLWARN_EXPIRED_1 (ERRCODE_AREA_UUI_UNKNOWNAUTH + SSLWARN_TYPE_EXPIRED + 1)
#define TITLE_UUI_SSLWARN_EXPIRED (ERRCODE_AREA_UUI_UNKNOWNAUTH + SSLWARN_TYPE_EXPIRED + 3)

#define ERRCODE_UUI_SSLWARN_DOMAINMISMATCH_1 (ERRCODE_AREA_UUI_UNKNOWNAUTH +  SSLWARN_TYPE_DOMAINMISMATCH + 1)
#define TITLE_UUI_SSLWARN_DOMAINMISMATCH (ERRCODE_AREA_UUI_UNKNOWNAUTH + SSLWARN_TYPE_DOMAINMISMATCH + 3)

#define ERRCODE_UUI_SSLWARN_INVALID_1 (ERRCODE_AREA_UUI_UNKNOWNAUTH + SSLWARN_TYPE_INVALID + 1)
#define TITLE_UUI_SSLWARN_INVALID (ERRCODE_AREA_UUI_UNKNOWNAUTH + SSLWARN_TYPE_INVALID + 3)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
