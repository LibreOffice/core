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

#include <tools/toolsdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>

struct SAL_WARN_UNUSED SvGUID
{
    sal_uInt32 Data1;
    sal_uInt16 Data2;
    sal_uInt16 Data3;
    sal_uInt8  Data4[8];
};

class SvStream;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC SvGlobalName
{
public:
    SvGlobalName() = default;
    SvGlobalName(const SvGlobalName& rObj) = default;

    SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                  sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                  sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

    // create SvGlobalName from a platform independent representation
    SvGlobalName( const css::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName( const SvGUID & rId );

    SvGlobalName & operator = ( const SvGlobalName & rObj ) = default;

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & WriteSvGlobalName( SvStream &, const SvGlobalName & );

    bool          operator < ( const SvGlobalName & rObj ) const;

    bool          operator == ( const SvGlobalName & rObj ) const;
    bool          operator != ( const SvGlobalName & rObj ) const
                      { return !(*this == rObj); }

    void          MakeFromMemory( void const * pData );
    bool          MakeId( const OUString & rId );
    OUString      GetHexName() const;

    const SvGUID& GetCLSID() const { return m_aData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    css::uno::Sequence < sal_Int8 > GetByteSequence() const;

private:
    SvGUID m_aData = {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
