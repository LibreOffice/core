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

#ifndef CPV_NEW_DEL_H
#define CPV_NEW_DEL_H


void *              New(
                        intt                i_nSize );
void                Delete(
                        void *              io_pMemory );

/* at ASSIGN_NEW ptr must be an L-Value. */
#define ASSIGN_NEW( ptr, type ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr ); }

#define ASSIGN_NEW1( ptr, type, param1 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1 ); }

#define ASSIGN_NEW2( ptr, type, param1, param2 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1, param2 ); }

#define ASSIGN_NEW3( ptr, type, param1, param2, param3 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1, param2, param3 ); }

#define DELETE( ptr, type ) \
    { type##_DTOR( ptr ); Delete(ptr); }

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
