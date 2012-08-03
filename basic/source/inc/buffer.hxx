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
    bool    Check( sal_uInt16 );
public:
    SbiBuffer( SbiParser*, short ); // increment
   ~SbiBuffer();
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    bool operator += (const String&);   // save basic-string
    bool operator += (sal_Int8);        // save character
    bool operator += (sal_Int16);       // save integer
    bool operator += (sal_uInt8);       // save character
    bool operator += (sal_uInt16);      // save integer
    bool operator += (sal_uInt32);      // save integer
    bool operator += (sal_Int32);       // save integer
    char*  GetBuffer();             // give out buffer (delete yourself!)
    char*  GetBufferPtr(){ return pBuf; }
    sal_uInt32 GetSize()                { return nOff; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
