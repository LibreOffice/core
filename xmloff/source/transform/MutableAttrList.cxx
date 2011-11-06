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
#include "precompiled_xmloff.hxx"
#include <osl/mutex.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <xmloff/attrlist.hxx>
#include "MutableAttrList.hxx"

using ::rtl::OUString;

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SvXMLAttributeList *XMLMutableAttributeList::GetMutableAttrList()
{
    if( !m_pMutableAttrList )
    {
        m_pMutableAttrList = new SvXMLAttributeList( m_xAttrList );
        m_xAttrList = m_pMutableAttrList;
    }

    return m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList() :
    m_pMutableAttrList( new SvXMLAttributeList )
{
    m_xAttrList = m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList( const Reference<
        XAttributeList> & rAttrList, sal_Bool bClone ) :
    m_xAttrList( rAttrList.is() ? rAttrList : new SvXMLAttributeList ),
    m_pMutableAttrList( 0 )
{
    if( bClone )
        GetMutableAttrList();
}


XMLMutableAttributeList::~XMLMutableAttributeList()
{
    m_xAttrList = 0;
}


// XUnoTunnel & co
const Sequence< sal_Int8 > & XMLMutableAttributeList::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// XUnoTunnel
sal_Int64 SAL_CALL XMLMutableAttributeList::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

sal_Int16 SAL_CALL XMLMutableAttributeList::getLength(void)
        throw( RuntimeException )
{
    return m_xAttrList->getLength();
}


OUString SAL_CALL XMLMutableAttributeList::getNameByIndex(sal_Int16 i)
        throw( RuntimeException )
{
    return m_xAttrList->getNameByIndex( i );
}


OUString SAL_CALL XMLMutableAttributeList::getTypeByIndex(sal_Int16 i)
        throw( RuntimeException )
{
    return m_xAttrList->getTypeByIndex( i );
}

OUString SAL_CALL  XMLMutableAttributeList::getValueByIndex(sal_Int16 i)
    throw( RuntimeException )
{
    return m_xAttrList->getValueByIndex( i );
}

OUString SAL_CALL XMLMutableAttributeList::getTypeByName(
        const OUString& rName )
        throw( RuntimeException )
{
    return m_xAttrList->getTypeByName( rName );
}

OUString SAL_CALL XMLMutableAttributeList::getValueByName(
        const OUString& rName)
        throw( RuntimeException )
{
    return m_xAttrList->getValueByName( rName );
}


Reference< XCloneable > XMLMutableAttributeList::createClone()
        throw( RuntimeException )
{
    // A cloned list will be a read only list!
    Reference< XCloneable >  r = new SvXMLAttributeList( m_xAttrList );
    return r;
}

void XMLMutableAttributeList::SetValueByIndex( sal_Int16 i,
                                               const ::rtl::OUString& rValue )
{
    GetMutableAttrList()->SetValueByIndex( i, rValue );
}

void XMLMutableAttributeList::AddAttribute( const OUString &rName ,
                                            const OUString &rValue )
{
    GetMutableAttrList()->AddAttribute( rName, rValue );
}

void XMLMutableAttributeList::RemoveAttributeByIndex( sal_Int16 i )
{
    GetMutableAttrList()->RemoveAttributeByIndex( i );
}

void XMLMutableAttributeList::RenameAttributeByIndex( sal_Int16 i,
                                                      const OUString& rNewName )
{
    GetMutableAttrList()->RenameAttributeByIndex( i, rNewName );
}

void XMLMutableAttributeList::AppendAttributeList(
        const Reference< ::com::sun::star::xml::sax::XAttributeList >& r )
{
    GetMutableAttrList()->AppendAttributeList( r );
}

sal_Int16 XMLMutableAttributeList::GetIndexByName( const OUString& rName ) const
{
    sal_Int16 nIndex = -1;
    if( m_pMutableAttrList )
    {
        nIndex = m_pMutableAttrList->GetIndexByName( rName );
    }
    else
    {
        sal_Int16 nCount = m_xAttrList->getLength();
        for( sal_Int16 i=0; nIndex==-1 && i<nCount ; ++i )
        {
            if( m_xAttrList->getNameByIndex(i) == rName )
                nIndex = i;
        }
    }
    return nIndex;
}
