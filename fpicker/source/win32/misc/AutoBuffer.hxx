/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AutoBuffer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:14:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _AUTO_BUFFER_HXX_
#define _AUTO_BUFFER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
