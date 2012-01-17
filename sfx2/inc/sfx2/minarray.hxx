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
#ifndef _SFXVARARR_HXX
#define _SFXVARARR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <limits.h>
#include <string.h>
#include <tools/solar.h>
#include <tools/debug.hxx>

class SFX2_DLLPUBLIC SfxPtrArr
{
private:
    void** pData;
    sal_uInt16 nUsed;
    sal_uInt8 nGrow;
    sal_uInt8 nUnused;
public:
    SfxPtrArr( sal_uInt8 nInitSize = 0, sal_uInt8 nGrowSize = 8 );
    SfxPtrArr( const SfxPtrArr& rOrig );
    ~SfxPtrArr();
    SfxPtrArr& operator= ( const SfxPtrArr& rOrig );
    void* GetObject( sal_uInt16 nPos ) const { return operator[](nPos); }
    void*& GetObject( sal_uInt16 nPos ) { return operator[](nPos); }
    void Insert( sal_uInt16 nPos, void* rElem );
    void Append( void* rElem );
    sal_Bool Remove( void* rElem );
    sal_uInt16 Remove( sal_uInt16 nPos, sal_uInt16 nLen );
    sal_uInt16 Count() const { return nUsed; }
    inline void** operator*();
    inline void* operator[]( sal_uInt16 nPos ) const;
    inline void*& operator[]( sal_uInt16 nPos );
    sal_Bool Contains( const void* rItem ) const;
    void Clear() { Remove( 0, Count() ); }
};

inline void** SfxPtrArr::operator*()
{
    return ( nUsed==0 ? 0 : pData );
}

inline void* SfxPtrArr::operator[]( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

inline void*& SfxPtrArr::operator [] (sal_uInt16 nPos)
{
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}


#define DECL_PTRARRAY(ARR, T, nI, nG)\
class ARR: public SfxPtrArr\
{\
public:\
   ARR( sal_uInt8 nIni=nI, sal_uInt8 nGrowValue=nG ):\
       SfxPtrArr(nIni,nGrowValue) \
   {}\
   ARR( const ARR& rOrig ):\
       SfxPtrArr(rOrig) \
   {}\
   T GetObject( sal_uInt16 nPos ) const { return operator[](nPos); } \
   T& GetObject( sal_uInt16 nPos ) { return operator[](nPos); } \
   void Insert( sal_uInt16 nPos, T aElement ) {\
       SfxPtrArr::Insert(nPos,(void *)aElement);\
   }\
   void Append( T aElement ) {\
       SfxPtrArr::Append((void *)aElement);\
   }\
   void Remove( T aElement ) {\
       SfxPtrArr::Remove((void*)aElement);\
   }\
   void Remove( sal_uInt16 nPos, sal_uInt16 nLen = 1 ) {\
       SfxPtrArr::Remove( nPos, nLen ); \
   }\
   T* operator *() {\
       return (T*) SfxPtrArr::operator*();\
   }\
   T operator[]( sal_uInt16 nPos ) const { \
       return (T) SfxPtrArr::operator[](nPos); } \
   T& operator[]( sal_uInt16 nPos ) { \
       return (T&) SfxPtrArr::operator[](nPos); } \
   void Clear() { Remove( 0, Count() ); }\
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
