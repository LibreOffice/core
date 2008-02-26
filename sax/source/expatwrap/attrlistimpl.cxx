/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attrlistimpl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:40:24 $
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

#include "attrlistimpl.hxx"

#include <vector>

#if OSL_DEBUG_LEVEL == 0
#  ifndef NDEBUG
#    define NDEBUG
#  endif
#endif
#include <assert.h>

#include <cppuhelper/weak.hxx>

using namespace ::std;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;


namespace sax_expatwrap {
struct TagAttribute
{
    TagAttribute()
        {}
    TagAttribute( const OUString &aName, const OUString &aType , const OUString &aValue )
    {
        this->sName     = aName;
        this->sType     = aType;
        this->sValue    = aValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeList_impl
{
    AttributeList_impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    vector<struct TagAttribute> vecAttribute;
};



sal_Int16 AttributeList::getLength(void) throw (RuntimeException)
{
    return static_cast<sal_Int16>(m_pImpl->vecAttribute.size());
}


AttributeList::AttributeList( const AttributeList &r ) :
    cppu::WeakImplHelper2<XAttributeList, XCloneable>()
{
    m_pImpl = new AttributeList_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeList::getNameByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeList::getTypeByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeList::getValueByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeList::getTypeByName( const OUString& sName ) throw (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeList::getValueByName(const OUString& sName) throw (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


Reference< XCloneable > AttributeList::createClone() throw (RuntimeException)
{
    AttributeList *p = new AttributeList( *this );
    return Reference< XCloneable > ( (XCloneable * ) p );
}



AttributeList::AttributeList()
{
    m_pImpl = new AttributeList_impl;
}



AttributeList::~AttributeList()
{
    delete m_pImpl;
}


void AttributeList::addAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

void AttributeList::clear()
{
    m_pImpl->vecAttribute.clear();
}

void AttributeList::removeAttribute( const OUString &sName )
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}


void AttributeList::setAttributeList( const Reference<  XAttributeList >  &r )
{
    assert( r.is() );

    sal_Int16 nMax = r->getLength();
    clear();
    m_pImpl->vecAttribute.reserve( nMax );

    for( int i = 0 ; i < nMax ; i ++ ) {
       m_pImpl->vecAttribute.push_back(
           TagAttribute(
               r->getNameByIndex( static_cast<sal_Int16>(i) ) ,
               r->getTypeByIndex( static_cast<sal_Int16>(i) ) ,
               r->getValueByIndex( static_cast<sal_Int16>(i) ) ) );
    }
    assert( nMax == getLength() );
}

}
