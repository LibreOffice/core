/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file_image.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:43:34 $
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

#ifndef INCLUDED_FILE_IMAGE_H
#define INCLUDED_FILE_IMAGE_H

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** file_image.
 */
struct file_image_st
{
    void * m_base;
    size_t m_size;
};

typedef struct file_image_st file_image;

#define FILE_IMAGE_INITIALIZER { 0, 0 }


/** file_image_open.
 */
int file_image_open (
    file_image * image,
    const char * filename);


/** file_image_pagein.
 */
int file_image_pagein (
    file_image * image);


/** file_image_close.
 */
int file_image_close (
    file_image * image);


/** Epilog.
 */
#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_ODEP_IMAGE_H */
