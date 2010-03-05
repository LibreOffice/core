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

/* NAME
 * PURPOSE
 *
 * NOTES
 *
 * HISTORY
 *        frog - Aug 6, 1998: Created.
 */

#include "precompile.h"

#ifdef __GNUG__
#pragma implementation "htags.h"
#endif

#include <string.h>

#include "hwplib.h"
#include "hwpfile.h"
#include "htags.h"
#include "himgutil.h"

bool HyperText::Read(HWPFile & hwpf)
{
    hwpf.Read1b(filename, 256);
    hwpf.Read2b(bookmark, 16);
    hwpf.Read1b(macro, 325);
    hwpf.Read1b(&type, 1);
    hwpf.Read1b(reserve, 3);
    if( type == 2 )
    {
        for( int i = 1; i < 256; i++)
        {
            filename[i-1] = filename[i];
            if( filename[i] == 0 )
                break;
        }
    }
    return true;
}


EmPicture::EmPicture(int tsize):size(tsize - 32)
{
    if (size <= 0)
        data = 0;
    else
        data = new uchar[size];
}
#ifdef WIN32
#define unlink _unlink
#endif
EmPicture::~EmPicture(void)
{
// clear temporary image file
    char *fname = (char *) GetEmbImgname(this);

    if (fname && access(fname, 0) == 0)
        unlink(fname);
    if (data)
        delete[]data;
};

bool EmPicture::Read(HWPFile & hwpf)
{
    if (size <= 0)
        return false;
    hwpf.Read1b(name, 16);
    hwpf.Read1b(type, 16);
    name[0] = 'H';
    name[1] = 'W';
    name[2] = 'P';
    if (hwpf.ReadBlock(data, size) == 0)
        return false;
    return true;
}


OlePicture::OlePicture(int tsize)
{
    size = tsize - 4;
    if (size <= 0)
        return;
#ifdef WIN32
     pis = 0L;
#else
     pis = new char[size];
#endif
};

OlePicture::~OlePicture(void)
{
#ifdef WIN32
     if( pis )
          pis->Release();
#else
     delete[] pis;
#endif
};

#define FILESTG_SIGNATURE_NORMAL 0xF8995568

bool OlePicture::Read(HWPFile & hwpf)
{
    if (size <= 0)
        return false;

// We process only FILESTG_SIGNATURE_NORMAL.
    hwpf.Read4b(&signature, 1);
    if (signature != FILESTG_SIGNATURE_NORMAL)
        return false;
#ifdef WIN32
     char *data;
     data = new char[size];
     if( data == 0 || hwpf.ReadBlock(data,size) == 0 )
          return false;
     FILE *fp;
     char tname[200];
     wchar_t wtname[200];
     tmpnam(tname);
     if (0 == (fp = fopen(tname, "wb")))
          return false;
     fwrite(data, size, 1, fp);
     fclose(fp);
     MultiByteToWideChar(CP_ACP, 0, tname, -1, wtname, 200);
     if( StgOpenStorage(wtname, NULL,
                     STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_TRANSACTED,
                     NULL, 0, &pis) != S_OK ) {
          pis = 0;
          unlink(tname);
          return false;
     }
     unlink(tname);
     delete [] data;
#else
    if (pis == 0 || hwpf.ReadBlock(pis, size) == 0)
        return false;
#endif

    return true;
}
