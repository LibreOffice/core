/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributes.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:52:00 $
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

#include <assert.h>
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _CONFIGMGR_MISC_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

//using namespace ::std;

struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const OUString &rName, const OUString &rType , const OUString &rValue )
    {
        sName     = rName;
        sType     = rType;
        sValue    = rValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeListImpl_impl
{
    AttributeListImpl_impl()
    {
// performance improvement during adding
        vecAttribute.reserve(20);
    }
    std::vector<struct TagAttribute> vecAttribute;
};

sal_Int16 SAL_CALL AttributeListImpl::getLength(void) throw (RuntimeException)
{
    return (sal_Int16)m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r ) :
cppu::WeakImplHelper1<com::sun::star::xml::sax::XAttributeList>( r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}


OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw (RuntimeException)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw (RuntimeException)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}


OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw (RuntimeException)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}


OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw (RuntimeException)
{
    std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ )
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sType;
        }
    }
    return OUString();
}


OUString AttributeListImpl::getValueByName(const OUString& sName) throw (RuntimeException)
{
    std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ )
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sValue;
        }
    }
    return OUString();
}


AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}


AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const OUString &sName ,
const OUString &sType ,
const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}


void AttributeListImpl::clear()
{
    std::vector<struct TagAttribute> dummy;
    m_pImpl->vecAttribute.swap( dummy );

    assert( ! getLength() );
}
