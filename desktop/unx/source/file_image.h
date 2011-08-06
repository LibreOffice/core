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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
