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

#ifndef _BUFFER_HXX
#define _BUFFER_HXX

#include <tools/solar.h>
#include <tools/string.hxx>

class SbiParser;

class SbiBuffer {
    SbiParser* pParser;             // for error messages
    char*   pBuf;
    char*   pCur;
    sal_uInt32  nOff;
    sal_uInt32  nSize;
    short   nInc;
    sal_Bool    Check( sal_uInt16 );
public:
    SbiBuffer( SbiParser*, short ); // increment
   ~SbiBuffer();
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    void Align( sal_Int32 );
    sal_Bool Add( const void*, sal_uInt16 );
    sal_Bool operator += (const String&);   // save basic-string
    sal_Bool operator += (sal_Int8);        // save character
    sal_Bool operator += (sal_Int16);       // save integer
    sal_Bool operator += (sal_uInt8);       // save character
    sal_Bool operator += (sal_uInt16);      // save integer
    sal_Bool operator += (sal_uInt32);      // save integer
    sal_Bool operator += (sal_Int32);       // save integer
    char*  GetBuffer();             // give out buffer (delete yourself!)
    char*  GetBufferPtr(){ return pBuf; }
    sal_uInt32 GetSize()                { return nOff; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
