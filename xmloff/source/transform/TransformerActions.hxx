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

#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#define _XMLOFF_TRANSFORMERACTIONS_HXX

#include <rtl/ustring.hxx>
#include <xmloff/nmspmap.hxx>

#include <boost/unordered_map.hpp>
#include "TransformerActionInit.hxx"
#include "TransformerAction.hxx"

struct NameKey_Impl
{
    sal_uInt16 m_nPrefix;
    ::rtl::OUString m_aLocalName;

    inline NameKey_Impl( sal_uInt16 nPrfx,
                         ::xmloff::token::XMLTokenEnum eLclNm ) :
        m_nPrefix( nPrfx ),
        m_aLocalName( ::xmloff::token::GetXMLToken( eLclNm ) )
    {
    }

    inline NameKey_Impl( sal_uInt16 nPrfx, const ::rtl::OUString& rLclNm ) :
        m_nPrefix( nPrfx ),
        m_aLocalName( rLclNm )
    {
    }

    inline NameKey_Impl() :
        m_nPrefix( XML_NAMESPACE_UNKNOWN )
    {
    }

    inline void SetLocalName( ::xmloff::token::XMLTokenEnum eLclNm )
    {
        m_aLocalName = ::xmloff::token::GetXMLToken( eLclNm );
    }
};

// -----------------------------------------------------------------------------

struct NameHash_Impl
{
    inline size_t operator()( const NameKey_Impl& r ) const;
    inline bool operator()( const NameKey_Impl& r1,
                               const NameKey_Impl& r2 ) const;
};

inline size_t NameHash_Impl::operator()( const NameKey_Impl& r ) const
{
    return static_cast< size_t >( r.m_nPrefix ) +
           static_cast< size_t >( r.m_aLocalName.hashCode() );
}

inline bool NameHash_Impl::operator()(
        const NameKey_Impl& r1,
        const NameKey_Impl& r2 ) const
{
    return r1.m_nPrefix == r2.m_nPrefix && r1.m_aLocalName == r2.m_aLocalName;
}

// -----------------------------------------------------------------------------

struct TransformerAction_Impl
{
    sal_uInt32 m_nActionType;
    sal_uInt32 m_nParam1;
    sal_uInt32 m_nParam2;
    sal_uInt32 m_nParam3;

    inline TransformerAction_Impl( sal_uInt32 nActnTp, sal_uInt32 nPrm1,
                                   sal_uInt32 nPrm2, sal_uInt32 nPrm3 ) :
        m_nActionType( nActnTp ),
        m_nParam1( nPrm1 ),
        m_nParam2( nPrm2 ),
        m_nParam3( nPrm3 )
    {

    }
    inline TransformerAction_Impl() :
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


// -----------------------------------------------------------------------------

class XMLTransformerActions :
    public ::boost::unordered_map< NameKey_Impl, TransformerAction_Impl,
                            NameHash_Impl, NameHash_Impl >
{
public:
    XMLTransformerActions( XMLTransformerActionInit *pInit );
    ~XMLTransformerActions();

    void Add( XMLTransformerActionInit *pInit );
};

#endif  //  _XMLOFF_TRANSFORMERACTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
