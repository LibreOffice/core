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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "test_ftpurl.hxx"
#include <stdio.h>
#include <unistd.h>



int main(int argc,char* argv[])
{

    typedef int (*INT_FUNC)(void);
    INT_FUNC tests[] = { test_ftpurl,
                   test_ftpparent,
                   test_ftpproperties,
                   test_ftpopen,
                   test_ftplist,
                   0 };  // don't remove ending zero

    int err_level = 0;

    fprintf(stderr,"-------       Testing       ----------\n");

    int i = -1;
    do {
        INT_FUNC f = tests[++i];
        if(f) {
            err_level += (*f)();
            fprintf(stderr,".");
        } else
            break;
    } while(true);

    if(err_level) {
        fprintf(stderr,"number of failed tests: %d\n",err_level);
        fprintf(stderr,"----------------------------------\n");
    } else
        fprintf(stderr,"no errors\n");

    return err_level;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
