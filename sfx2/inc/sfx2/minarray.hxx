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

#if defined (ALPHA) && defined (UNX)
#define DEL_ARRAY( X )
#else
#define DEL_ARRAY( X ) X
#endif

#define DECL_OBJARRAY( ARR, T, nI, nG ) \
class ARR\
{\
private:\
    T*   pData;\
    sal_uInt16  nUsed;\
    sal_uInt8   nGrow;\
    sal_uInt8    nUnused;\
public:\
    ARR( sal_uInt8 nInitSize = nI, sal_uInt8 nGrowSize = nG );\
    ARR( const ARR& rOrig );\
    ~ARR();\
\
    ARR& operator= ( const ARR& rOrig );\
\
    const T& GetObject( sal_uInt16 nPos ) const; \
    T& GetObject( sal_uInt16 nPos ); \
\
    void Insert( sal_uInt16 nPos, ARR& rIns, sal_uInt16 nStart = 0, sal_uInt16 nEnd = USHRT_MAX );\
    void Insert( sal_uInt16 nPos, const T& rElem );\
    void Insert( sal_uInt16 nPos, const T& rElems, sal_uInt16 nLen );\
    void Append( const T& rElem );\
\
    sal_Bool Remove( const T& rElem );\
    sal_uInt16 Remove( sal_uInt16 nPos, sal_uInt16 nLen );\
\
    sal_uInt16 Count() const { return nUsed; }\
    T* operator*();\
    const T& operator[]( sal_uInt16 nPos ) const;\
    T& operator[]( sal_uInt16 nPos );\
\
    sal_Bool Contains( const T& rItem ) const;\
    void Clear() { Remove( 0, Count() ); }\
};\
\
inline void ARR::Insert( sal_uInt16 nPos, ARR& rIns, sal_uInt16 nStart, sal_uInt16 nEnd )\
{\
    Insert( nPos, *(rIns.pData+(sizeof(T)*nStart)), nStart-nEnd+1 );\
}\
\
inline void ARR::Insert( sal_uInt16 nPos, const T& rElem )\
{\
    Insert( nPos, rElem, 1 );\
}\
\
inline T* ARR::operator*()\
{\
    return ( nUsed==0 ? 0 : pData );\
} \
inline const T& ARR::operator[]( sal_uInt16 nPos ) const\
{\
    DBG_ASSERT( nPos < nUsed, "" ); \
    return *(pData+nPos);\
} \
inline T& ARR::operator [] (sal_uInt16 nPos) \
{\
    DBG_ASSERT( nPos < nUsed, "" ); \
    return *(pData+nPos); \
} \
inline const T& ARR::GetObject( sal_uInt16 nPos ) const { return operator[](nPos); } \
inline T& ARR::GetObject( sal_uInt16 nPos ) { return operator[](nPos); } \

#ifndef _lint
// String too long

#define IMPL_OBJARRAY( ARR, T ) \
ARR::ARR( sal_uInt8 nInitSize, sal_uInt8 nGrowSize ): \
    nUsed(0), \
    nGrow( nGrowSize ? nGrowSize : 1 ), \
    nUnused(nInitSize) \
{ \
    if ( nInitSize != 0 ) \
    { \
        size_t nBytes = nInitSize * sizeof(T); \
        pData = (T*) new char[ nBytes ]; \
        memset( pData, 0, nBytes ); \
    } \
    else \
        pData = 0; \
} \
\
ARR::ARR( const ARR& rOrig ) \
{ \
    nUsed = rOrig.nUsed; \
    nGrow = rOrig.nGrow; \
    nUnused = rOrig.nUnused; \
\
    if ( rOrig.pData != 0 ) \
    { \
        size_t nBytes = (nUsed + nUnused) * sizeof(T); \
        pData = (T*) new char [ nBytes ]; \
        memset( pData, 0, nBytes ); \
        for ( sal_uInt16 n = 0; n < nUsed; ++n ) \
            *(pData+n) = *(rOrig.pData+n); \
    } \
    else \
        pData = 0; \
} \
\
ARR::~ARR() \
{ \
    for ( sal_uInt16 n = 0; n < nUsed; ++n ) \
        ( pData+n )->T::~T(); \
    delete[] (char*) pData;\
} \
\
ARR& ARR::operator= ( const ARR& rOrig )\
{ \
    for ( sal_uInt16 n = 0; n < nUsed; ++n ) \
        ( pData+n )->T::~T(); \
    delete[] (char*) pData;\
\
    nUsed = rOrig.nUsed; \
    nGrow = rOrig.nGrow; \
    nUnused = rOrig.nUnused; \
\
    if ( rOrig.pData != 0 ) \
    { \
        size_t nBytes = (nUsed + nUnused) * sizeof(T); \
        pData = (T*) new char[ nBytes ]; \
        memset( pData, 0, nBytes ); \
        for ( sal_uInt16 n = 0; n < nUsed; ++n ) \
            *(pData+n) = *(rOrig.pData+n); \
    } \
    else \
        pData = 0; \
    return *this; \
} \
\
void ARR::Append( const T& aElem ) \
{ \
     \
    if ( nUnused == 0 ) \
    { \
        sal_uInt16 nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow; \
        size_t nBytes = nNewSize * sizeof(T); \
        T* pNewData = (T*) new char[ nBytes ]; \
        memset( pNewData, 0, nBytes ); \
        if ( pData ) \
        { \
            memcpy( pNewData, pData, nUsed * sizeof(T) ); \
            delete[] (char*) pData;\
        } \
        nUnused = (sal_uInt8)(nNewSize-nUsed); \
        pData = pNewData; \
    } \
\
     \
    pData[nUsed] = aElem; \
    ++nUsed; \
    --nUnused; \
} \
\
sal_uInt16 ARR::Remove( sal_uInt16 nPos, sal_uInt16 nLen ) \
{ \
    DBG_ASSERT( (nPos+nLen) < (nUsed+1), "" ); \
    DBG_ASSERT( nLen > 0, "" ); \
\
    nLen = Min( (sal_uInt16)(nUsed-nPos), (sal_uInt16)nLen ); \
\
    if ( nLen == 0 ) \
        return 0; \
\
    for ( sal_uInt16 n = nPos; n < (nPos+nLen); ++n ) \
        ( pData+n )->T::~T(); \
\
    if ( (nUsed-nLen) == 0 ) \
    { \
        delete[] (char*) pData;\
        pData = 0; \
        nUsed = 0; \
        nUnused = 0; \
        return nLen; \
    } \
\
    if ( (nUnused+nLen) >= nGrow ) \
    { \
        sal_uInt16 nNewUsed = nUsed-nLen; \
        sal_uInt16 nNewSize = ((nNewUsed+nGrow-1)/nGrow) * nGrow; \
        DBG_ASSERT( nNewUsed <= nNewSize && nNewUsed+nGrow > nNewSize, \
                    "shrink size computation failed" ); \
        size_t nBytes = nNewSize * sizeof(T); \
        T* pNewData = (T*) new char[ nBytes ]; \
        memset( pNewData, 0, nBytes ); \
        if ( nPos > 0 ) \
            memcpy( pNewData, pData, nPos * sizeof(T) ); \
        if ( nNewUsed != nPos ) \
            memcpy(pNewData+nPos, pData+nPos+nLen, (nNewUsed-nPos) * sizeof(T) ); \
        delete[] (char*) pData;\
        pData = pNewData; \
        nUsed = nNewUsed; \
        nUnused = (sal_uInt8)(nNewSize - nNewUsed); \
        return nLen; \
    } \
\
     \
    if ( nUsed-nPos-nLen > 0 ) \
    { \
        memmove(pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen) * sizeof(T));\
    } \
    nUsed = nUsed - nLen; \
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused + nLen); \
    return nLen; \
} \
\
sal_Bool ARR::Remove( const T& aElem ) \
{ \
    if ( nUsed == 0 ) \
        return sal_False; \
\
    const T *pIter = pData + nUsed - 1; \
    for ( sal_uInt16 n = 0; n < nUsed; ++n, --pIter ) \
        if ( *pIter == aElem ) \
        { \
            Remove(nUsed-n-1, 1); \
            return sal_True; \
        } \
    return sal_False; \
} \
\
sal_Bool ARR::Contains( const T& rItem ) const \
{ \
    if ( !nUsed ) \
        return sal_False; \
    for ( sal_uInt16 n = 0; n < nUsed; ++n ) \
    { \
        const T& r2ndItem = GetObject(n); \
        if ( r2ndItem == rItem ) \
            return sal_True; \
    } \
    return sal_False; \
} \
\
void ARR::Insert( sal_uInt16 nPos, const T& rElems, sal_uInt16 nLen ) \
{ \
    DBG_ASSERT( nPos <= nUsed, "" ); \
     \
    if ( nUnused == 0 ) \
    { \
        \
        /* auf die naechste Grow-Grenze aufgerundet vergroeszern */ \
        sal_uInt16 nNewSize; \
        for ( nNewSize = nUsed+nGrow; nNewSize < (nUsed + nLen); ++nNewSize ) \
            /* empty loop */; \
        size_t nBytes = nNewSize * sizeof(T); \
        T* pNewData = (T*) new char[ nBytes ]; \
        memset( pNewData, 0, nBytes ); \
        \
        if ( pData ) \
        { \
            DBG_ASSERT( nUsed < nNewSize, "" ); \
            memcpy( pNewData, pData, nUsed * sizeof(T) ); \
            delete (char*) pData;\
        } \
        nUnused = (sal_uInt8)(nNewSize-nUsed); \
        pData = pNewData; \
    } \
\
     \
    if ( nPos < nUsed ) \
    { \
        memmove(pData+nPos+nLen-1, pData+nPos-1, sizeof(T) * (nUsed-nPos)); \
    } \
\
    memmove(pData+nPos, &rElems, sizeof(T) * nLen); \
    nUsed = nUsed + nLen; \
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused - nLen); \
}

// _lint
#endif

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
    sal_Bool Replace( void* pOldElem, void* pNewElem );
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
   sal_Bool Replace( T aOldElem, T aNewElem ) {\
       return SfxPtrArr::Replace((void *)aOldElem, (void*) aNewElem);\
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

class ByteArr
{
private:
    char* pData;
    sal_uInt16 nUsed;
    sal_uInt8 nGrow;
    sal_uInt8 nUnused;
public:
    ByteArr( sal_uInt8 nInitSize = 0, sal_uInt8 nGrowSize = 8 );
    ByteArr( const ByteArr& rOrig );
    ~ByteArr();
    ByteArr& operator= ( const ByteArr& rOrig );
    char GetObject( sal_uInt16 nPos ) const { return operator[](nPos); }
    char& GetObject( sal_uInt16 nPos ) { return operator[](nPos); }
    void Insert( sal_uInt16 nPos, char rElem );
    void Append( char rElem );
    sal_Bool Remove( char rElem );
    sal_uInt16 Remove( sal_uInt16 nPos, sal_uInt16 nLen );
    sal_uInt16 Count() const { return nUsed; }
    char* operator*();
    char operator[]( sal_uInt16 nPos ) const;
    char& operator[]( sal_uInt16 nPos );
    sal_Bool Contains( const char rItem ) const;
    void Clear() { Remove( 0, Count() ); }
};

inline char* ByteArr::operator*()
{
    return ( nUsed==0 ? 0 : pData );
}

#define DECL_1BYTEARRAY(ARR, T, nI, nG)\
class ARR: public ByteArr\
{\
public:\
        ARR( sal_uInt8 nIni=nI, sal_uInt8 nGrow=nG ):\
            ByteArr(nIni,nGrow) \
        {}\
        ARR( const ARR& rOrig ):\
            ByteArr(rOrig) \
        {}\
        T GetObject( sal_uInt16 nPos ) const { return operator[](nPos); } \
        T& GetObject( sal_uInt16 nPos ) { return operator[](nPos); } \
        void Insert( sal_uInt16 nPos, T aElement ) {\
            ByteArr::Insert(nPos,(char)aElement);\
        }\
        void Append( T aElement ) {\
            ByteArr::Append((char)aElement);\
        }\
        void Remove( T aElement ) {\
            ByteArr::Remove((char)aElement);\
        }\
        void Remove( sal_uInt16 nPos, sal_uInt16 nLen = 1 ) {\
            ByteArr::Remove( nPos, nLen ); \
        }\
        T* operator *() {\
            return (T*) ByteArr::operator*();\
        }\
        T operator[]( sal_uInt16 nPos ) const { \
            return (T) ByteArr::operator[](nPos); } \
        T& operator[]( sal_uInt16 nPos ) { \
            return (T&) ByteArr::operator[](nPos); } \
        void Clear() { Remove( 0, Count() ); }\
};

class WordArr
{
private:
    short* pData;
    sal_uInt16 nUsed;
    sal_uInt8 nGrow;
    sal_uInt8 nUnused;
public:
    WordArr( sal_uInt8 nInitSize = 0, sal_uInt8 nGrowSize = 8 );
    WordArr( const WordArr& rOrig );
    ~WordArr();
    WordArr& operator= ( const WordArr& rOrig );
    short GetObject( sal_uInt16 nPos ) const { return operator[](nPos); }
    short& GetObject( sal_uInt16 nPos ) { return operator[](nPos); }
    void Insert( sal_uInt16 nPos, short rElem );
    void Append( short rElem );
    sal_Bool Remove( short rElem );
    sal_uInt16 Remove( sal_uInt16 nPos, sal_uInt16 nLen );
    sal_uInt16 Count() const { return nUsed; }
    short* operator*();
    short operator[]( sal_uInt16 nPos ) const;
    short& operator[]( sal_uInt16 nPos );
    sal_Bool Contains( const short rItem ) const;
    void Clear() { Remove( 0, Count() ); }
};

inline short* WordArr::operator*()
{
    return ( nUsed==0 ? 0 : pData );
}

#define DECL_2BYTEARRAY(ARR, T, nI, nG)\
class ARR: public WordArr\
{\
public:\
        ARR( sal_uInt8 nIni=nI, sal_uInt8 nGrowValue=nG ):\
            WordArr(nIni,nGrowValue) \
        {}\
        ARR( const ARR& rOrig ):\
            WordArr(rOrig) \
        {}\
        T GetObject( sal_uInt16 nPos ) const { return operator[](nPos); } \
        T& GetObject( sal_uInt16 nPos ) { return operator[](nPos); } \
        void Insert( sal_uInt16 nPos, T aElement ) {\
            WordArr::Insert(nPos,(short)aElement);\
        }\
        void Append( T aElement ) {\
            WordArr::Append((short)aElement);\
        }\
        void Remove( T aElement ) {\
            WordArr::Remove((short)aElement);\
        }\
        void Remove( sal_uInt16 nPos, sal_uInt16 nLen = 1 ) {\
            WordArr::Remove( nPos, nLen ); \
        }\
        T* operator *() {\
            return (T*) WordArr::operator*();\
        }\
        T operator[]( sal_uInt16 nPos ) const { \
            return (T) WordArr::operator[](nPos); } \
        T& operator[]( sal_uInt16 nPos ) { \
            return (T&) WordArr::operator[](nPos); } \
        void Clear() { Remove( 0, Count() ); }\
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
