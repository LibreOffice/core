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

#include <vector>

#include "xmltoken.hxx"
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "attrlist.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

struct SvXMLTagAttribute_Impl
{
    SvXMLTagAttribute_Impl(){}
    SvXMLTagAttribute_Impl( const OUString &rName,
                         const OUString &rValue )
        : sName(rName),
        sValue(rValue)
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
};



sal_Int16 SAL_CALL SvXMLAttributeList::getLength(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return m_pImpl->vecAttribute.size();
}


SvXMLAttributeList::SvXMLAttributeList( const SvXMLAttributeList &r )
{
    m_pImpl = new SvXMLAttributeList_Impl;
    *m_pImpl = *(r.m_pImpl);
}

SvXMLAttributeList::SvXMLAttributeList( const uno::Reference< 
        xml::sax::XAttributeList> & rAttrList )
    : sType( GetXMLToken(XML_CDATA) )
{
    m_pImpl = new SvXMLAttributeList_Impl;

    SvXMLAttributeList* pImpl =
        SvXMLAttributeList::getImplementation( rAttrList );

    if( pImpl )
        *m_pImpl = *(pImpl->m_pImpl);
    else
        AppendAttributeList( rAttrList );
}

OUString SAL_CALL SvXMLAttributeList::getNameByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString SAL_CALL SvXMLAttributeList::getTypeByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    return sType;
}

OUString SAL_CALL  SvXMLAttributeList::getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString SAL_CALL SvXMLAttributeList::getTypeByName( const OUString& sName ) throw( ::com::sun::star::uno::RuntimeException )
{
    return sType;
}

OUString SAL_CALL SvXMLAttributeList::getValueByName(const OUString& sName) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


uno::Reference< ::com::sun::star::util::XCloneable >  SvXMLAttributeList::createClone() throw( ::com::sun::star::uno::RuntimeException )
{
    uno::Reference< ::com::sun::star::util::XCloneable >  r = new SvXMLAttributeList( *this );
    return r;
}


SvXMLAttributeList::SvXMLAttributeList()
    : sType( GetXMLToken(XML_CDATA) )
{
    m_pImpl = new SvXMLAttributeList_Impl;
}



SvXMLAttributeList::~SvXMLAttributeList()
{
    delete m_pImpl;
}


void SvXMLAttributeList::AddAttribute( 	const OUString &sName ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( SvXMLTagAttribute_Impl( sName , sValue ) );
}

void SvXMLAttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();

    OSL_ASSERT( ! getLength() );
}

void SvXMLAttributeList::RemoveAttribute( const OUString sName )
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}

void SvXMLAttributeList::AppendAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    OSL_ASSERT( r.is() );

    sal_Int32 nMax = r->getLength();
    sal_Int32 nTotalSize = m_pImpl->vecAttribute.size() + nMax;
    m_pImpl->vecAttribute.reserve( nTotalSize );

    for( sal_Int32 i = 0 ; i < nMax ; i ++ ) {
        m_pImpl->vecAttribute.push_back( SvXMLTagAttribute_Impl(
            r->getNameByIndex( i ) ,
            r->getValueByIndex( i )));
    }

    OSL_ASSERT( nTotalSize == getLength());
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLAttributeList::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvXMLAttributeList* SvXMLAttributeList::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
        return (SvXMLAttributeList*)xUT->getSomething( SvXMLAttributeList::getUnoTunnelId() );
    else
        return NULL;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLAttributeList::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
