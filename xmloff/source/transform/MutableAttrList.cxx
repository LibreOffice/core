/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string.h>

#include <osl/mutex.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/attrlist.hxx>
#include <comphelper/servicehelper.hxx>
#include "MutableAttrList.hxx"


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
        XAttributeList> & rAttrList, bool bClone ) :
    m_xAttrList( rAttrList.is() ? rAttrList : new SvXMLAttributeList ),
    m_pMutableAttrList( nullptr )
{
    if( bClone )
        GetMutableAttrList();
}


XMLMutableAttributeList::~XMLMutableAttributeList()
{
    m_xAttrList = nullptr;
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
    throw( RuntimeException, std::exception )
{
    if( rId.getLength() == 16 &&
        0 == memcmp( getUnoTunnelId().getConstArray(),
                                rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

sal_Int16 SAL_CALL XMLMutableAttributeList::getLength()
        throw( RuntimeException, std::exception )
{
    return m_xAttrList->getLength();
}


OUString SAL_CALL XMLMutableAttributeList::getNameByIndex(sal_Int16 i)
        throw( RuntimeException, std::exception )
{
    return m_xAttrList->getNameByIndex( i );
}


OUString SAL_CALL XMLMutableAttributeList::getTypeByIndex(sal_Int16 i)
        throw( RuntimeException, std::exception )
{
    return m_xAttrList->getTypeByIndex( i );
}

OUString SAL_CALL  XMLMutableAttributeList::getValueByIndex(sal_Int16 i)
    throw( RuntimeException, std::exception )
{
    return m_xAttrList->getValueByIndex( i );
}

OUString SAL_CALL XMLMutableAttributeList::getTypeByName(
        const OUString& rName )
        throw( RuntimeException, std::exception )
{
    return m_xAttrList->getTypeByName( rName );
}

OUString SAL_CALL XMLMutableAttributeList::getValueByName(
        const OUString& rName)
        throw( RuntimeException, std::exception )
{
    return m_xAttrList->getValueByName( rName );
}


Reference< XCloneable > XMLMutableAttributeList::createClone()
        throw( RuntimeException, std::exception )
{
    // A cloned list will be a read only list!
    Reference< XCloneable >  r = new SvXMLAttributeList( m_xAttrList );
    return r;
}

void XMLMutableAttributeList::SetValueByIndex( sal_Int16 i,
                                               const OUString& rValue )
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
