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

/* NAME $Id: htags.h,v 1.3 2008-04-10 12:08:00 rt Exp $
 * PURPOSE
 *
 * NOTES
 *
 * HISTORY
 *        frog - Aug 6, 1998: Created.
 */

#ifndef _HTAGS_H_
#define _HTAGS_H_

#ifdef __GNUG__
#pragma interface
#endif

class HWPFile;
/**
 * @short Embeded image
 */
struct EmPicture
{
    int   size;
    char  name[16];
    char  type[16];
    uchar *data;

    EmPicture(int size);
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
    ulong signature;
#ifdef WIN32
     IStorage *pis;
#else
     char *pis;
#endif
    OlePicture(int tsize);
    ~OlePicture(void);

    bool Read(HWPFile& hwpf);
};
#endif                                            /* _HTAGS_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
