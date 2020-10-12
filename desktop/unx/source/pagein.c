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

#include "file_image.h"
#include "pagein.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#endif
#include <sys/types.h>

/* do_pagein */
static void do_pagein (const char * filename)
{
    int result;
    file_image image = FILE_IMAGE_INITIALIZER;

    if (file_image_open (&image, filename) != 0)
        return;

    if ((result = file_image_pagein (&image)) != 0)
    {
        fprintf (stderr, "file_image_pagein %s: %s\n", filename, strerror(result));
    }

    file_image_close (&image);
}

static int isRotational(char const * path)
{
    int ret = 1;
#ifdef LINUX
    FILE * fp = NULL;
    char fullpath[4096];
    struct stat out;
    int major, minor;
    char type;
    if (stat(path , &out) == -1)
        return ret;
    major = major(out.st_dev);
    minor = 0; /* minor(out.st_dev); only the device itself has a queue */
    sprintf(fullpath,"/sys/dev/block/%d:%d/queue/rotational",major,minor);
    if ((fp = fopen(fullpath, "r")) == NULL)
        return ret;
    if (fgets(&type, 1, fp))
        ret = type == '1';
    fclose(fp);
#endif
    return ret;
}

void pagein_execute(char const * path, char const * file)
{
    char fullpath[4096];
    char *p = NULL;
    FILE   * fp = NULL;
    if(!isRotational(path))
        return;
    memset(fullpath, 0, sizeof(fullpath));
    strncpy (fullpath, path, 3000);
    if (!(p = strrchr (fullpath, '/')))
        p = fullpath;
    else
        p++;
    strncpy(p, file, 1024);
    p[strlen(p)] = '\0';
    if ((fp = fopen (fullpath, "r")) == NULL)
    {

        fprintf (stderr, "fopen %s: %s\n", fullpath, strerror(errno));
        return;
    }
    while (fgets (p, 1024, fp) != NULL)
    {
        p[strlen(p) - 1] = '\0';

        /* paths relative to the location of the pagein file */
        do_pagein (fullpath);
    }
    fclose (fp);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
