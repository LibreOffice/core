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
