/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_hwpfilter.hxx"

#include <assert.h>
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include "attributes.hxx"

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
