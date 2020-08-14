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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX

#include <rtl/ustring.hxx>
#include <xmloff/namespacemap.hxx>
#include "TransformerActionInit.hxx"
#include "TransformerAction.hxx"
#include <boost/functional/hash.hpp>
#include <unordered_map>

struct NameKey_Impl
{
    sal_uInt16 m_nPrefix;
    OUString m_aLocalName;

    NameKey_Impl( sal_uInt16 nPrfx, const OUString& rLclNm ) :
        m_nPrefix( nPrfx ),
        m_aLocalName( rLclNm )
    {
    }

    NameKey_Impl() :
        m_nPrefix( XML_NAMESPACE_UNKNOWN )
    {
    }

    void SetLocalName( ::xmloff::token::XMLTokenEnum eLclNm )
    {
        m_aLocalName = ::xmloff::token::GetXMLToken( eLclNm );
    }
};

struct NameHash_Impl
{
    inline size_t operator()( const NameKey_Impl& r ) const;
    inline bool operator()( const NameKey_Impl& r1,
                               const NameKey_Impl& r2 ) const;
};

inline size_t NameHash_Impl::operator()( const NameKey_Impl& r ) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, r.m_nPrefix);
    boost::hash_combine(seed, r.m_aLocalName.hashCode());
    return seed;
}

inline bool NameHash_Impl::operator()(
        const NameKey_Impl& r1,
        const NameKey_Impl& r2 ) const
{
    return r1.m_nPrefix == r2.m_nPrefix && r1.m_aLocalName == r2.m_aLocalName;
}

struct TransformerAction_Impl
{
    sal_uInt32 m_nActionType;
    sal_uInt32 m_nParam1;
    sal_uInt32 m_nParam2;
    sal_uInt32 m_nParam3;

    TransformerAction_Impl() :
        m_nActionType( XML_TACTION_EOT ),
        m_nParam1( 0 ),
        m_nParam2( 0 ),
        m_nParam3( 0 )
    {
    }

    sal_uInt16 GetQNamePrefixFromParam1() const
    {
        return static_cast< sal_uInt16 >( m_nParam1 >> 16 );
    }

    sal_uInt16 GetQNamePrefixFromParam2() const
    {
        return static_cast< sal_uInt16 >( m_nParam2 >> 16 );
    }

    sal_uInt16 GetQNamePrefixFromParam3() const
    {
        return static_cast< sal_uInt16 >( m_nParam3 >> 16 );
    }

    ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam1() const
    {
        return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam1 & 0xffff );
    }

    ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam2() const
    {
        return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam2 & 0xffff );
    }

    ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam3() const
    {
        return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam3 & 0xffff );
    }

};

class XMLTransformerActions :
    public std::unordered_map< NameKey_Impl, TransformerAction_Impl,
                            NameHash_Impl, NameHash_Impl >
{
public:
    explicit XMLTransformerActions( XMLTransformerActionInit const *pInit );
    ~XMLTransformerActions();

    void Add( XMLTransformerActionInit const *pInit );
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
