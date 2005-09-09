/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransformerActions.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:57:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#define _XMLOFF_TRANSFORMERACTIONS_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#include <hash_map>

#ifndef _XMLOFF_TRANSFORMERACTIONINIT_HXX
#include "TransformerActionInit.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTION_HXX
#include "TransformerAction.hxx"
#endif

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
    public ::std::hash_map< NameKey_Impl, TransformerAction_Impl,
                            NameHash_Impl, NameHash_Impl >
{
public:
    XMLTransformerActions( XMLTransformerActionInit *pInit );
    ~XMLTransformerActions();

    void Add( XMLTransformerActionInit *pInit );
};

#endif  //  _XMLOFF_TRANSFORMERACTIONS_HXX
