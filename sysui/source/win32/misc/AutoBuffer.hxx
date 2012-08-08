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

#ifndef _AUTO_BUFFER_HXX_
#define _AUTO_BUFFER_HXX_

#include <sal/types.h>

#include <rtl/ustring>

//-------------------------------------------------------------
// A simple unicode buffer management class, the class itself
// is responsible for the allocated unicode buffer, any
// modification of the buffer size outside the class may lead
// to undefined behaviour
//-------------------------------------------------------------

class CAutoUnicodeBuffer
{
public:

    // if bLazyCreation is true the buffer will be created
    // when someone wants to fill the buffer
    CAutoUnicodeBuffer( size_t size, sal_Bool bLazyCreation = sal_False );
    ~CAutoUnicodeBuffer( );

    // resizes the buffer
    sal_Bool SAL_CALL resize( size_t new_size );

    // zeros the buffer
    void SAL_CALL empty( );

    // fills the buffer with a given content
    sal_Bool SAL_CALL fill( const sal_Unicode* pContent, size_t nLen );

    // returns the size of the buffer
    size_t SAL_CALL size( ) const;

    // conversion operator
    operator sal_Unicode*( );

    // address operator
    sal_Unicode* operator&( );

    const sal_Unicode* operator&( ) const;

private:
    void SAL_CALL init( );

private:
    size_t m_buffSize; // the number of unicode chars
    sal_Unicode* m_pBuff;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
