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
#include "share.h"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void some_more(  t_throws_exc p )
{
    (*p)();
}

extern "C" void SAL_CALL start( t_throws_exc p )
{
    try
    {
        some_more( p );
    }
    catch (lang::IllegalArgumentException & exc)
    {
        OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        printf( "starter.cxx: caught IllegalArgumentException: %s\n", msg.getStr() );
    }
    catch (Exception & exc)
    {
        OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        printf( "starter.cxx: caught some UNO exc: %s\n", msg.getStr() );
    }
    catch (...)
    {
        printf( "starter.cxx: caught something\n" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
