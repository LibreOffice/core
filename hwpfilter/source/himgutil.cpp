/*************************************************************************
 *
 *  $RCSfile: himgutil.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:39:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 1998 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 1998 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "precompile.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "hwplib.h"
#include "htags.h"
#include "himgutil.h"

const char *GetPictureFilename(const char *picname)
{
#ifdef _WIN32
    return picname;
#else                                         /* UNIX */
    static char fname[256], *ptr;

    if (picname[1] == ':')
        ptr = strcpy(fname, picname + 3);
    else
        ptr = strcpy(fname, picname);

    while (*ptr)
    {
        if (*ptr == '\\')
            *ptr = DIRSEP;
        ptr++;
    }
    if (access(fname, 0) != -1)
        return fname;
    else if ((ptr = strrchr(fname, DIRSEP)) && access(ptr + 1, 0) != -1)
        return ptr + 1;

// 소문자로
    ptr = fname;
    while (*ptr)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }
    if (access(fname, 0) != -1)
        return fname;
    else if ((ptr = strrchr(fname, '/')) && access(ptr + 1, 0) != -1)
        return ptr + 1;
    return 0;
#endif                                        /* !_WIN32 */
}


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


int ReadFileType(const char *fname)
{
    FILE *fp;
    uchar magicno[30];                            /* first 30 bytes of file */
    int n;

    if (!fname)
        return RFT_ERROR;                         /* shouldn't happen */

    fp = fopen(fname, "rb");
    if (!fp)
        return RFT_ERROR;

    n = fread(magicno, 1, 30, fp);
    fclose(fp);

    if (n < 30)
        return RFT_UNKNOWN;                       /* files less than 30 bytes long... */

    return ImageMagicType(magicno);
}


const char *GetEmbImgname(const EmPicture * empic)
{
    static char fname[256];
    char *ptr, *ext;

    tmpnam(fname);
    if (!empic || !empic->name[0] || !(ptr = strrchr(fname, DIRSEP)))
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
    sprintf(ptr + 1, "mz_%s.%s", empic->name, ext);
    return fname;
}
