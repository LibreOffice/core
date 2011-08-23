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

#include "precompile.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "hwplib.h"
#include "htags.h"
#include "himgutil.h"


static int ImageMagicType(const uchar * magicno)
{
    int rv = RFT_UNKNOWN;

    if (strncmp((char *) magicno, "GIF87a", 6) == 0 ||
        strncmp((char *) magicno, "GIF89a", 6) == 0)
        rv = RFT_GIF;
    else if (strncmp((char *) magicno, "VIEW", 4) == 0 ||
        strncmp((char *) magicno, "WEIV", 4) == 0)
        rv = RFT_PM;
    else if (magicno[0] == 'P' && magicno[1] >= '1' && magicno[1] <= '6')
        rv = RFT_PBM;
/* note: have to check XPM before XBM, as first 2 chars are the same */
    else if (strncmp((char *) magicno, "/* XPM */", 9) == 0)
        rv = RFT_XPM;
    else if (strncmp((char *) magicno, "#define", 7) == 0 ||
        (magicno[0] == '/' && magicno[1] == '*'))
        rv = RFT_XBM;
    else if (magicno[0] == 0x59 && (magicno[1] & 0x7f) == 0x26 &&
        magicno[2] == 0x6a && (magicno[3] & 0x7f) == 0x15)
        rv = RFT_SUNRAS;
    else if (magicno[0] == 'B' && magicno[1] == 'M')
        rv = RFT_BMP;
    else if (magicno[0] == 0x52 && magicno[1] == 0xcc)
        rv = RFT_UTAHRLE;
    else if ((magicno[0] == 0x01 && magicno[1] == 0xda) ||
        (magicno[0] == 0xda && magicno[1] == 0x01))
        rv = RFT_IRIS;
    else if (magicno[0] == 0x1f && magicno[1] == 0x9d)
        rv = RFT_COMPRESS;
    else if (magicno[0] == 0x0a && magicno[1] <= 5)
        rv = RFT_PCX;
    else if (strncmp((char *) magicno, "FORM", 4) == 0 &&
        strncmp((char *) magicno + 8, "ILBM", 4) == 0)
        rv = RFT_IFF;
    else if (magicno[0] == 0 && magicno[1] == 0 &&
        magicno[2] == 2 && magicno[3] == 0 &&
        magicno[4] == 0 && magicno[5] == 0 &&
        magicno[6] == 0 && magicno[7] == 0)
        rv = RFT_TARGA;
    else if (magicno[4] == 0x00 && magicno[5] == 0x00 &&
        magicno[6] == 0x00 && magicno[7] == 0x07)
        rv = RFT_XWD;
    else if (strncmp((char *) magicno, "SIMPLE  ", 8) == 0 &&
        magicno[29] == 'T')
        rv = RFT_FITS;
    else if (magicno[0] == 0xff && magicno[1] == 0xd8 && magicno[2] == 0xff)
        rv = RFT_JFIF;
    else if ((magicno[0] == 'M' && magicno[1] == 'M') ||
        (magicno[0] == 'I' && magicno[1] == 'I'))
        rv = RFT_TIFF;
    else if (strncmp((char *) magicno, "%!", 2) == 0 ||
        strncmp((char *) magicno, "\004%!", 3) == 0)
        rv = RFT_PS;
    else if (strncmp((char *) magicno, "\xd7\xcd\xc6\x9a", 4) == 0 ||
        strncmp((char *) magicno, "\x01\x00\x09\x00\x00\x03", 6) == 0)
        rv = RFT_WMF;

    return rv;
}

const char *GetEmbImgname(const EmPicture * empic)
{
    static char fname[256];
    char *ptr;
    const char *ext;

    if (tmpnam(fname) == NULL)
        return NULL;
    if (!empic || !empic->name[0] || (0 == (ptr = strrchr(fname, DIRSEP))))
        return NULL;
    switch (ImageMagicType((uchar *) empic->data))
    {
        case RFT_GIF:
            ext = "gif";
            break;
        case RFT_PM:
            ext = "pm";
            break;
        case RFT_PBM:
            ext = "pbm";
            break;
        case RFT_XBM:
            ext = "xbm";
            break;
        case RFT_SUNRAS:
            ext = "ras";
            break;
        case RFT_BMP:
            ext = "bmp";
            break;
        case RFT_UTAHRLE:
            ext = "rle";
            break;
        case RFT_PCX:
            ext = "pcx";
            break;
        case RFT_JFIF:
            ext = "jpg";
            break;
        case RFT_TIFF:
            ext = "tif";
            break;
        case RFT_PDSVICAR:
            ext = "pds";
            break;
        case RFT_PS:
            ext = "ps";
            break;
        case RFT_IFF:
            ext = "iff";
            break;
        case RFT_TARGA:
            ext = "tga";
            break;
        case RFT_XPM:
            ext = "xpm";
            break;
        case RFT_XWD:
            ext = "xwd";
            break;
        case RFT_WMF:
            ext = "wmf";
            break;
        default:
            ext = "666";
            break;
    };
    sprintf(ptr + 1, "mz_%s.%s", empic->name,ext);
    return fname;
}
