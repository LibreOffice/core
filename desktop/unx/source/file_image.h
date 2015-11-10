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

#ifndef INCLUDED_DESKTOP_UNX_SOURCE_FILE_IMAGE_H
#define INCLUDED_DESKTOP_UNX_SOURCE_FILE_IMAGE_H

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

#define FILE_IMAGE_INITIALIZER { NULL, 0 }


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
