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
#include "precompiled_comphelper.hxx"
#include <comphelper/attributelist.hxx>
#include <vos/diagnose.hxx>

#include <vector>

using namespace rtl;
using namespace osl;
using namespace com::sun::star;

namespace comphelper {

struct TagAttribute_Impl
{
    TagAttribute_Impl(){}
    TagAttribute_Impl( const OUString &aName, const OUString &aType,
                         const OUString &aValue )
    {
        this->sName     = aName;
        this->sType     = aType;
        this->sValue    = aValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeList_Impl
{
    AttributeList_Impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    ::std::vector<struct TagAttribute_Impl> vecAttribute;
};

sal_Int16 SAL_CALL AttributeList::getLength(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return (sal_Int16)(m_pImpl->vecAttribute.size());
}

OUString SAL_CALL AttributeList::getNameByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size()) ) ? m_pImpl->vecAttribute[i].sName : OUString();
}

OUString SAL_CALL AttributeList::getTypeByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    if( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString SAL_CALL  AttributeList::getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) ? m_pImpl->vecAttribute[i].sValue : OUString();
}

OUString SAL_CALL AttributeList::getTypeByName( const OUString& sName ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString SAL_CALL AttributeList::getValueByName(const OUString& sName) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


AttributeList::AttributeList()
{
    m_pImpl = new AttributeList_Impl;
}



AttributeList::~AttributeList()
{
    delete m_pImpl;
}

void AttributeList::AddAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute_Impl( sName , sType , sValue ) );
}

void AttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();

    VOS_ENSURE( ! getLength(), "Length > 0 after AttributeList::Clear!");
}

void AttributeList::RemoveAttribute( const OUString sName )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}


void AttributeList::SetAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    Clear();
    AppendAttributeList( r );
}

void AttributeList::AppendAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    VOS_ENSURE( r.is(), "r isn't!" );

    sal_Int32 nMax = r->getLength();
    sal_Int32 nTotalSize = m_pImpl->vecAttribute.size() + nMax;
    m_pImpl->vecAttribute.reserve( nTotalSize );

    for( sal_Int16 i = 0 ; i < nMax ; i ++ ) {
        m_pImpl->vecAttribute.push_back( TagAttribute_Impl(
            r->getNameByIndex( i ) ,
            r->getTypeByIndex( i ) ,
            r->getValueByIndex( i )));
    }

    VOS_ENSURE( nTotalSize == getLength(), "nTotalSize != getLength()");
}

} // namespace comphelper

