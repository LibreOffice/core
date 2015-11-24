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

#ifndef INCLUDED_HWPFILTER_SOURCE_HTAGS_H
#define INCLUDED_HWPFILTER_SOURCE_HTAGS_H

class HWPFile;
/**
 * @short Embedded image
 */
struct EmPicture
{
    size_t size;
    char  name[16];
    char  type[16];
    uchar *data;

    explicit EmPicture(size_t size);
    ~EmPicture(void);

    bool Read(HWPFile& hwpf);
};
/**
 * @short HyperText
 */
struct HyperText
{
    kchar filename[256];
    hchar bookmark[16];
    char  macro[325];
    uchar type;
    char reserve[3];
    bool Read(HWPFile& hwpf);
};
/**
 * @short Win32 OLE object
 */
struct OlePicture
{
    int   size;
    uint signature;
#ifdef WIN32
    IStorage *pis;
#else
    char *pis;
#endif
    explicit OlePicture(int tsize);
    ~OlePicture(void);

    bool Read(HWPFile& hwpf);
};
#endif // INCLUDED_HWPFILTER_SOURCE_HTAGS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
