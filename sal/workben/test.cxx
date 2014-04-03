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

#include <stdio.h>

#if (defined UNX)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    void test_int64();
    test_int64();

    void test_profile(void);
    test_profile();

    void test_OString();
    test_OString();

/*      void test_OWString(); */
/*      test_OWString(); */

/*      void test_OStringBuffer(); */
/*      test_OStringBuffer(); */

/*      void test_OWStringBuffer(); */
/*      test_OWStringBuffer(); */

/*      void test_OString2OWStringAndViceVersa(); */
/*      test_OString2OWStringAndViceVersa(); */

    void test_uuid();
    test_uuid();

    return(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
