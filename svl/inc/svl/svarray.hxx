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

#ifndef _SVARRAY_HXX
#define _SVARRAY_HXX

/***********************************************************************
*
*   Hier folgt die Beschreibung fuer die exportierten Makros:
*
*       SV_DECL_PTRARR_SORT(nm, AE, IS, GS)
*       SV_IMPL_OP_PTRARR_SORT( nm,AE )
*           defieniere/implementiere ein Sort-Array mit Pointern, das nach
*           Objecten sortiert ist. Basiert auf einem PTRARR.
*           Sortierung mit Hilfe der Object-operatoren "<" und "=="
*
***********************************************************************/

#include "svl/svldllapi.h"

#include <limits.h>     // USHRT_MAX
#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <tools/solar.h>

typedef void* VoidPtr;
class SVL_DLLPUBLIC SvPtrarr
{
protected:
    VoidPtr    *pData;
    sal_uInt16 nFree;
    sal_uInt16 nA;

    void _resize(size_t n);

public:
    SvPtrarr( sal_uInt16= 0 );
    ~SvPtrarr() { rtl_freeMemory( pData ); }

    VoidPtr& operator[](sal_uInt16 nP) const { return *(pData+nP); }

    void Insert( const SvPtrarr * pI, sal_uInt16 nP,
                 sal_uInt16 nS = 0, sal_uInt16 nE = USHRT_MAX );

    VoidPtr& GetObject(sal_uInt16 nP) const { return (*this)[nP]; }

    void Insert( const VoidPtr& aE, sal_uInt16 nP );
    void Insert( const VoidPtr *pE, sal_uInt16 nL, sal_uInt16 nP );
    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1 );
    sal_uInt16 Count() const { return nA; }
    const VoidPtr* GetData() const { return (const VoidPtr*)pData; }
    sal_uInt16 GetPos( const VoidPtr & aE ) const;
};

// SORTARR - Begin

#define _SV_IMPL_SORTAR_ALG(nm, AE)\
sal_Bool nm::Insert( const AE & aE )\
{\
    sal_uInt16 nP;\
    sal_Bool bExist;\
    bExist = Seek_Entry( aE, &nP );\
    if( ! bExist )\
        nm##_SAR::Insert( aE, nP );\
    return !bExist;\
}\
\
void nm::Remove( sal_uInt16 nP, sal_uInt16 nL )\
{\
    if( nL )\
        nm##_SAR::Remove( nP, nL);\
}

#define SV_DECL_PTRARR_SORT(nm, AE, IS)\
class nm##_SAR: public SvPtrarr \
{\
public:\
    nm##_SAR( sal_uInt16 nIni=IS )\
        : SvPtrarr(nIni) {}\
    void Insert( const nm##_SAR *pI, sal_uInt16 nP, \
            sal_uInt16 nS = 0, sal_uInt16 nE = USHRT_MAX ) {\
        SvPtrarr::Insert((const SvPtrarr*)pI, nP, nS, nE);\
    }\
    void Insert( const AE & aE, sal_uInt16 nP ) {\
        SvPtrarr::Insert( (const VoidPtr &)aE, nP );\
    }\
    void Insert( const AE *pE, sal_uInt16 nL, sal_uInt16 nP ) {\
        SvPtrarr::Insert( (const VoidPtr*)pE, nL, nP );\
    }\
    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1) {\
        SvPtrarr::Remove(nP,nL);\
    }\
    const AE* GetData() const {\
        return (const AE*)SvPtrarr::GetData();\
    }\
    AE operator[]( sal_uInt16 nP )const  { \
        return (AE)SvPtrarr::operator[](nP); }\
    AE GetObject(sal_uInt16 nP) const { \
        return (AE)SvPtrarr::GetObject(nP); }\
    \
    sal_uInt16 GetPos( const AE & aE ) const { \
        return SvPtrarr::GetPos((const VoidPtr &)aE);\
    }\
    void DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL=1 );\
private:\
    nm##_SAR( const nm##_SAR& );\
    nm##_SAR& operator=( const nm##_SAR& );\
};\
\
class nm : private nm##_SAR \
{\
public:\
    nm(sal_uInt16 nSize = IS)\
        : nm##_SAR(nSize) {}\
    sal_Bool Insert( const AE& aE );\
    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1 );\
    sal_uInt16 Count() const  {   return nm##_SAR::Count(); }\
    const AE* GetData() const { return (const AE*)pData; }\
    AE operator[](sal_uInt16 nP) const {\
        return nm##_SAR::operator[]( nP );\
    }\
    AE GetObject(sal_uInt16 nP) const {\
        return nm##_SAR::GetObject( nP );\
    }\
    sal_Bool Seek_Entry( const AE aE, sal_uInt16* pP = 0 ) const;\
    void DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL=1 ); \
    sal_uInt16 GetPos( const AE& aE ) const { \
        return SvPtrarr::GetPos((const VoidPtr&)aE);\
    }\
private:\
    nm( const nm& );\
    nm& operator=( const nm& );\
};

#define SV_IMPL_OP_PTRARR_SORT( nm,AE )\
_SV_IMPL_SORTAR_ALG( nm,AE )\
void nm::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL ) { \
    if( nL ) {\
        OSL_ENSURE( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );\
        for( sal_uInt16 n=nP; n < nP + nL; n++ ) \
            delete *((AE*)pData+n); \
        SvPtrarr::Remove( nP, nL ); \
    } \
} \
sal_Bool nm::Seek_Entry( const AE aE, sal_uInt16* pP ) const\
{\
    register sal_uInt16 nO  = nm##_SAR::Count(),\
            nM, \
            nU = 0;\
    if( nO > 0 )\
    {\
        nO--;\
        while( nU <= nO )\
        {\
            nM = nU + ( nO - nU ) / 2;\
            if( *(*((AE*)pData + nM)) == *(aE) )\
            {\
                if( pP ) *pP = nM;\
                return sal_True;\
            }\
            else if( *(*((AE*)pData + nM)) < *(aE) )\
                nU = nM + 1;\
            else if( nM == 0 )\
            {\
                if( pP ) *pP = nU;\
                return sal_False;\
            }\
            else\
                nO = nM - 1;\
        }\
    }\
    if( pP ) *pP = nU;\
    return sal_False;\
}



#if defined(ICC) || defined(GCC) || (defined(WNT) && _MSC_VER >= 1400)
#define C40_INSERT( c, p, n ) Insert( (c const *&) p, n )
#else
#define C40_INSERT( c, p, n ) Insert( p, n )
#endif

#endif  //_SVARRAY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
