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


#include <string.h>
#include <vector>
#include <osl/diagnose.h>
#include <xmloff/xmltoken.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>

#include <xmloff/attrlist.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

struct SvXMLTagAttribute_Impl
{
    SvXMLTagAttribute_Impl( const OUString &rName,
                         const OUString &rValue )
        : sName(rName),
        sValue(rValue)
    {
    }

    SvXMLTagAttribute_Impl( const SvXMLTagAttribute_Impl& r ) :
        sName(r.sName),
        sValue(r.sValue)
    {
    }

    OUString sName;
    OUString sValue;
};

struct SvXMLAttributeList_Impl
{
    SvXMLAttributeList_Impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }

    ::std::vector<struct SvXMLTagAttribute_Impl> vecAttribute;
    typedef ::std::vector<struct SvXMLTagAttribute_Impl>::size_type size_type;
};


sal_Int16 SAL_CALL SvXMLAttributeList::getLength()
{
    return sal::static_int_cast< sal_Int16 >(m_pImpl->vecAttribute.size());
}


SvXMLAttributeList::SvXMLAttributeList( const SvXMLAttributeList &r ) :
    cppu::WeakImplHelper<css::xml::sax::XAttributeList, css::util::XCloneable, css::lang::XUnoTunnel>(r),
    m_pImpl( new SvXMLAttributeList_Impl( *r.m_pImpl ) )
{
}

SvXMLAttributeList::SvXMLAttributeList( const uno::Reference<
        xml::sax::XAttributeList> & rAttrList )
    : m_pImpl( new SvXMLAttributeList_Impl),
      sType( GetXMLToken(XML_CDATA) )
{

    SvXMLAttributeList* pImpl =
        SvXMLAttributeList::getImplementation( rAttrList );

    if( pImpl )
        *m_pImpl = *(pImpl->m_pImpl);
    else
        AppendAttributeList( rAttrList );
}

OUString SAL_CALL SvXMLAttributeList::getNameByIndex(sal_Int16 i)
{
    return ( static_cast< SvXMLAttributeList_Impl::size_type >( i ) < m_pImpl->vecAttribute.size() ) ? m_pImpl->vecAttribute[i].sName : OUString();
}


OUString SAL_CALL SvXMLAttributeList::getTypeByIndex(sal_Int16)
{
    return sType;
}

OUString SAL_CALL  SvXMLAttributeList::getValueByIndex(sal_Int16 i)
{
    return ( static_cast< SvXMLAttributeList_Impl::size_type >( i ) < m_pImpl->vecAttribute.size() ) ? m_pImpl->vecAttribute[i].sValue : OUString();
}

OUString SAL_CALL SvXMLAttributeList::getTypeByName( const OUString& )
{
    return sType;
}

OUString SAL_CALL SvXMLAttributeList::getValueByName(const OUString& sName)
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


uno::Reference< css::util::XCloneable >  SvXMLAttributeList::createClone()
{
    uno::Reference< css::util::XCloneable >  r = new SvXMLAttributeList( *this );
    return r;
}


SvXMLAttributeList::SvXMLAttributeList()
    : m_pImpl( new SvXMLAttributeList_Impl ),
      sType( GetXMLToken(XML_CDATA) )
{
}


SvXMLAttributeList::~SvXMLAttributeList()
{
}


void SvXMLAttributeList::AddAttribute(  const OUString &sName ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.emplace_back( sName , sValue );
}

void SvXMLAttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();

    OSL_ASSERT( ! getLength() );
}

void SvXMLAttributeList::RemoveAttribute( const OUString& sName )
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}

void SvXMLAttributeList::AppendAttributeList( const uno::Reference< css::xml::sax::XAttributeList >  &r )
{
    OSL_ASSERT( r.is() );

    sal_Int16 nMax = r->getLength();
    SvXMLAttributeList_Impl::size_type nTotalSize =
        m_pImpl->vecAttribute.size() + nMax;
    m_pImpl->vecAttribute.reserve( nTotalSize );

    for( sal_Int16 i = 0 ; i < nMax ; ++i ) {
        m_pImpl->vecAttribute.emplace_back(
            r->getNameByIndex( i ) ,
            r->getValueByIndex( i ));
    }

    OSL_ASSERT( nTotalSize == static_cast<SvXMLAttributeList_Impl::size_type>(getLength()));
}

void SvXMLAttributeList::SetValueByIndex( sal_Int16 i,
        const OUString& rValue )
{
    if( static_cast< SvXMLAttributeList_Impl::size_type >( i )
            < m_pImpl->vecAttribute.size() )
    {
        m_pImpl->vecAttribute[i].sValue = rValue;
    }
}

void SvXMLAttributeList::RemoveAttributeByIndex( sal_Int16 i )
{
    if( static_cast< SvXMLAttributeList_Impl::size_type >( i )
            < m_pImpl->vecAttribute.size() )
        m_pImpl->vecAttribute.erase( m_pImpl->vecAttribute.begin() + i );
}

void SvXMLAttributeList::RenameAttributeByIndex( sal_Int16 i,
                                                 const OUString& rNewName )
{
    if( static_cast< SvXMLAttributeList_Impl::size_type >( i )
            < m_pImpl->vecAttribute.size() )
    {
        m_pImpl->vecAttribute[i].sName = rNewName;
    }
}

sal_Int16 SvXMLAttributeList::GetIndexByName( const OUString& rName ) const
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii =
        m_pImpl->vecAttribute.begin();

    for( sal_Int16 nIndex=0; ii!=m_pImpl->vecAttribute.end(); ++ii, ++nIndex )
    {
        if( (*ii).sName == rName )
        {
            return nIndex;
        }
    }
    return -1;
}

namespace
{
    class theSvXMLAttributeListUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvXMLAttributeListUnoTunnelId> {};
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLAttributeList::getUnoTunnelId() throw()
{
    return theSvXMLAttributeListUnoTunnelId::get().getSeq();
}

SvXMLAttributeList* SvXMLAttributeList::getImplementation( const uno::Reference< uno::XInterface >& xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
    {
        return
            reinterpret_cast<SvXMLAttributeList*>(
                sal::static_int_cast<sal_IntPtr>(
                    xUT->getSomething( SvXMLAttributeList::getUnoTunnelId())));
    }
    else
        return nullptr;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLAttributeList::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
