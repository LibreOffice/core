/*************************************************************************
 *
 *  $RCSfile: htags.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:40:50 $
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

/* NAME $Id: htags.cpp,v 1.1 2003-10-15 14:40:50 dvo Exp $
 * PURPOSE
 *
 * NOTES
 *
 * HISTORY
 *        frog - Aug 6, 1998: Created.
 */

#include "precompile.h"

#ifdef __GNUG__
#pragma implementaion "htags.h"
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
     if (!(fp = fopen(tname, "wb")))
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
