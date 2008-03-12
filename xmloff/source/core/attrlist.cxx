/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attrlist.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:28:45 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <vector>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>

#if OSL_DEBUG_LEVEL == 0
#  ifndef NDEBUG
#    define NDEBUG
#  endif
#endif
#include <assert.h>

#include <xmloff/attrlist.hxx>

using ::rtl::OUString;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

struct SvXMLTagAttribute_Impl
{
    SvXMLTagAttribute_Impl(){}
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

    SvXMLAttributeList_Impl( const SvXMLAttributeList_Impl& r ) :
            vecAttribute( r.vecAttribute )
    {
    }

    ::std::vector<struct SvXMLTagAttribute_Impl> vecAttribute;
    typedef ::std::vector<struct SvXMLTagAttribute_Impl>::size_type size_type;
};



sal_Int16 SAL_CALL SvXMLAttributeList::getLength(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return sal::static_int_cast< sal_Int16 >(m_pImpl->vecAttribute.size());
}


SvXMLAttributeList::SvXMLAttributeList( const SvXMLAttributeList &r ) :
    cppu::WeakImplHelper3<com::sun::star::xml::sax::XAttributeList, com::sun::star::util::XCloneable, com::sun::star::lang::XUnoTunnel>(r),
    m_pImpl( new SvXMLAttributeList_Impl( *r.m_pImpl ) )
{
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
    if( static_cast< SvXMLAttributeList_Impl::size_type >( i )
            < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString SAL_CALL SvXMLAttributeList::getTypeByIndex(sal_Int16) throw( ::com::sun::star::uno::RuntimeException )
{
    return sType;
}

OUString SAL_CALL  SvXMLAttributeList::getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    if( static_cast< SvXMLAttributeList_Impl::size_type >( i )
            < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString SAL_CALL SvXMLAttributeList::getTypeByName( const OUString& ) throw( ::com::sun::star::uno::RuntimeException )
{
    return sType;
}

OUString SAL_CALL SvXMLAttributeList::getValueByName(const OUString& sName) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
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


void SvXMLAttributeList::AddAttribute(  const OUString &sName ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( SvXMLTagAttribute_Impl( sName , sValue ) );
}

void SvXMLAttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();

    assert( ! getLength() );
}

void SvXMLAttributeList::RemoveAttribute( const OUString sName )
{
    ::std::vector<struct SvXMLTagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}


void SvXMLAttributeList::SetAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    Clear();
    AppendAttributeList( r );
}

void SvXMLAttributeList::AppendAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    assert( r.is() );

    sal_Int16 nMax = r->getLength();
    SvXMLAttributeList_Impl::size_type nTotalSize =
        m_pImpl->vecAttribute.size() + nMax;
    m_pImpl->vecAttribute.reserve( nTotalSize );

    for( sal_Int16 i = 0 ; i < nMax ; ++i ) {
        m_pImpl->vecAttribute.push_back( SvXMLTagAttribute_Impl(
            r->getNameByIndex( i ) ,
            r->getValueByIndex( i )));
    }

    assert( nTotalSize == (SvXMLAttributeList_Impl::size_type)getLength());
}

void SvXMLAttributeList::SetValueByIndex( sal_Int16 i,
        const ::rtl::OUString& rValue )
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
    {
        return
            reinterpret_cast<SvXMLAttributeList*>(
                sal::static_int_cast<sal_IntPtr>(
                    xUT->getSomething( SvXMLAttributeList::getUnoTunnelId())));
    }
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
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}


