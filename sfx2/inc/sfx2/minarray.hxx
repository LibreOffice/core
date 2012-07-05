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
