/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributelist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:49:55 $
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
#include "precompiled_comphelper.hxx"

#ifndef _ATTRIBUTE_LIST_HXX
#include <comphelper/attributelist.hxx>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

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
    if( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
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
    if( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

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

