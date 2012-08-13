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
#ifndef _GLOBNAME_HXX
#define _GLOBNAME_HXX

#include <vector>

#include "tools/toolsdllapi.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/string.hxx>

struct ImpSvGlobalName
{
    sal_uInt8       szData[ 16 ];
    sal_uInt16      nRefCount;

    enum Empty { EMPTY };

                ImpSvGlobalName()
                {
                    nRefCount = 0;
                }
                ImpSvGlobalName( const ImpSvGlobalName & rObj );
                ImpSvGlobalName( Empty );

    sal_Bool    operator == ( const ImpSvGlobalName & rObj ) const;
};

#ifdef WNT
struct _GUID;
typedef struct _GUID GUID;
#else
struct GUID;
#endif

typedef GUID CLSID;
class SvStream;

class TOOLS_DLLPUBLIC SvGlobalName
{
    ImpSvGlobalName * pImp;
    void    NewImp();

public:
            SvGlobalName();
            SvGlobalName( const SvGlobalName & rObj )
            {
                pImp = rObj.pImp;
                pImp->nRefCount++;
            }
            SvGlobalName( ImpSvGlobalName * pImpP )
            {
                pImp = pImpP;
                pImp->nRefCount++;
            }
            SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                          sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                          sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

            // create SvGlobalName from a platform independent representation
            SvGlobalName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
            ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & operator << ( SvStream &, const SvGlobalName & );

    sal_Bool            operator < ( const SvGlobalName & rObj ) const;
    SvGlobalName &  operator += ( sal_uInt32 );
    SvGlobalName &  operator ++ () { return operator += ( 1 ); }

    sal_Bool    operator == ( const SvGlobalName & rObj ) const;
    sal_Bool    operator != ( const SvGlobalName & rObj ) const
            { return !(*this == rObj); }

    void    MakeFromMemory( void * pData );
    sal_Bool    MakeId( const String & rId );
    String  GetHexName() const;

                  SvGlobalName( const CLSID & rId );
    const CLSID & GetCLSID() const { return *(CLSID *)pImp->szData; }
    const sal_uInt8* GetBytes() const { return pImp->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
