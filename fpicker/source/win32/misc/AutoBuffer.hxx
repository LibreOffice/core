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

#ifndef _AUTO_BUFFER_HXX_
#define _AUTO_BUFFER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <rtl/ustring.hxx>

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
