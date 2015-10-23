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
#ifndef INCLUDED_TOOLS_GLOBNAME_HXX
#define INCLUDED_TOOLS_GLOBNAME_HXX

#include <vector>

#include <tools/toolsdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <o3tl/cow_wrapper.hxx>

struct SvGUID
{
    sal_uInt32 Data1;
    sal_uInt16 Data2;
    sal_uInt16 Data3;
    sal_uInt8  Data4[8];
};

struct ImpSvGlobalName
{
    struct SvGUID   szData;

    ImpSvGlobalName(const SvGUID &rData)
        : szData(rData)
    {
    }
    ImpSvGlobalName(sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
              sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
              sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15);
    ImpSvGlobalName( const ImpSvGlobalName & rObj );
    ImpSvGlobalName();

    bool        operator == ( const ImpSvGlobalName & rObj ) const;
};

class SvStream;

class TOOLS_DLLPUBLIC SvGlobalName
{
    ::o3tl::cow_wrapper< ImpSvGlobalName > pImp;

public:
    SvGlobalName();
    SvGlobalName( const SvGlobalName & rObj ) :
        pImp( rObj.pImp )
    {
    }

    SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                  sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                  sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

    // create SvGlobalName from a platform independent representation
    SvGlobalName( const css::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName( const SvGUID & rId );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
    ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & WriteSvGlobalName( SvStream &, const SvGlobalName & );

    bool          operator < ( const SvGlobalName & rObj ) const;
    SvGlobalName& operator += ( sal_uInt32 );
    SvGlobalName& operator ++ () { return operator += ( 1 ); }

    bool          operator == ( const SvGlobalName & rObj ) const;
    bool          operator != ( const SvGlobalName & rObj ) const
                      { return !(*this == rObj); }

    void          MakeFromMemory( void * pData );
    bool          MakeId( const OUString & rId );
    OUString      GetHexName() const;

    const SvGUID& GetCLSID() const { return pImp->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    css::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
