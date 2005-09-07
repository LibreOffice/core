/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: himgutil.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:35:23 $
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

/* NAME $Id: himgutil.h,v 1.2 2005-09-07 16:35:23 rt Exp $
 * PURPOSE
 *   WIN32 등의 xv image 라이브러리가 없는 경우,
 *   이미지 파일의 형식을 반환하는데 사용한다.
 * NOTES
 *
 * HISTORY
 *        frog - Dec 23, 1998: Created.
 */

#ifndef _HIMGUTIL_H_
#define _HIMGUTIL_H_

/**
 * Graphics file format identifiers.
 */
#define RFT_ERROR    -1                           /* couldn't open file, or whatever... */
#define RFT_UNKNOWN   0
#define RFT_GIF       1
#define RFT_PM        2
#define RFT_PBM       3
#define RFT_XBM       4
#define RFT_SUNRAS    5
#define RFT_BMP       6
#define RFT_UTAHRLE   7
#define RFT_IRIS      8
#define RFT_PCX       9
#define RFT_JFIF     10
#define RFT_TIFF     11
#define RFT_PDSVICAR 12
#define RFT_COMPRESS 13
#define RFT_PS       14
#define RFT_IFF      15
#define RFT_TARGA    16
#define RFT_XPM      17
#define RFT_XWD      18
#define RFT_FITS     19
#define RFT_WMF      20

// function declaration
struct EmPicture;

int  ReadFileType(const char *fname);
const char *GetPictureFilename(const char *picname);
/**
 * Extract the name from given object
 * @param empic Object having name to extract
 * @returns Name of embeded picture
 */
const char *GetEmbImgname(const EmPicture *empic);
#endif                                            /* _HIMGUTIL_H_ */
