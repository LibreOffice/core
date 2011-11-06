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



#include "attrlistimpl.hxx"

#include <vector>

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

}
