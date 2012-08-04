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

#include <string.h>

#include <osl/mutex.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/attrlist.hxx>
#include <comphelper/servicehelper.hxx>
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

namespace
{
    class theXMLMutableAttributeListUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theXMLMutableAttributeListUnoTunnelId> {};
}

// XUnoTunnel & co
const Sequence< sal_Int8 > & XMLMutableAttributeList::getUnoTunnelId() throw()
{
    return theXMLMutableAttributeListUnoTunnelId::get().getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL XMLMutableAttributeList::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == memcmp( getUnoTunnelId().getConstArray(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
