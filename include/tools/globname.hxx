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
#pragma once

#include <tools/toolsdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <o3tl/cow_wrapper.hxx>

struct SAL_WARN_UNUSED SvGUID
{
    sal_uInt32 Data1;
    sal_uInt16 Data2;
    sal_uInt16 Data3;
    sal_uInt8  Data4[8];
};

struct SAL_WARN_UNUSED ImpSvGlobalName
{
    struct SvGUID   szData = {};

    ImpSvGlobalName(const SvGUID &rData)
        : szData(rData)
    {
    }
    ImpSvGlobalName(sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
              sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
              sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15);
    ImpSvGlobalName( const ImpSvGlobalName & rObj );
    ImpSvGlobalName() = default;

    bool        operator == ( const ImpSvGlobalName & rObj ) const;
};

class SvStream;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC SvGlobalName
{
    ::o3tl::cow_wrapper< ImpSvGlobalName > mpImpl;

public:
    SvGlobalName();
    SvGlobalName( const SvGlobalName & rObj ) :
        mpImpl( rObj.mpImpl )
    {
    }
    SvGlobalName( SvGlobalName && rObj ) noexcept :
        mpImpl( std::move(rObj.mpImpl) )
    {
    }

    SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                  sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                  sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

    // create SvGlobalName from a platform independent representation
    SvGlobalName( const css::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName( const SvGUID & rId );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
    SvGlobalName & operator = ( SvGlobalName && rObj ) noexcept;
    ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & WriteSvGlobalName( SvStream &, const SvGlobalName & );

    bool          operator < ( const SvGlobalName & rObj ) const;

    bool          operator == ( const SvGlobalName & rObj ) const;
    bool          operator != ( const SvGlobalName & rObj ) const
                      { return !(*this == rObj); }

    void          MakeFromMemory( void const * pData );
    bool          MakeId( const OUString & rId );
    OUString      GetHexName() const;

    const SvGUID& GetCLSID() const { return mpImpl->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    css::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

namespace std
{
    /** Specialise std::optional template for the case where we are wrapping a o3tl::cow_wrapper
        type, and we can make the pointer inside the cow_wrapper act as an empty value,
        and save ourselves some storage */
    template<>
    class optional<SvGlobalName> final : public o3tl::cow_optional<SvGlobalName>
    {
    public:
        using cow_optional::cow_optional; // inherit constructors
        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;
        ~optional();
        void reset();
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
