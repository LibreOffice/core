/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htags.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:39:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* NAME $Id: htags.h,v 1.2 2005-09-07 16:39:15 rt Exp $
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
