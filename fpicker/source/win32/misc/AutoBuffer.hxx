/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
