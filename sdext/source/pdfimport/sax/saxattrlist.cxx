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


#include "saxattrlist.hxx"

namespace pdfi
{

SaxAttrList::SaxAttrList( const std::unordered_map< OUString, OUString, OUStringHash >& rMap )
{
    m_aAttributes.reserve(rMap.size());
    for( std::unordered_map< OUString,
                        OUString,
                        OUStringHash >::const_iterator it = rMap.begin();
         it != rMap.end(); ++it )
    {
        m_aIndexMap[ it->first ] = m_aAttributes.size();
        m_aAttributes.push_back( AttrEntry( it->first, it->second ) );
    }
}

SaxAttrList::SaxAttrList( const SaxAttrList& rClone ) :
    cppu::WeakImplHelper<css::xml::sax::XAttributeList, css::util::XCloneable>(rClone),
    m_aAttributes( rClone.m_aAttributes ),
    m_aIndexMap( rClone.m_aIndexMap )
{
}

SaxAttrList::~SaxAttrList()
{
}

namespace {
    static OUString getCDATAString()
    {
        return OUString( "CDATA" );
    }
}

sal_Int16 SAL_CALL SaxAttrList::getLength() throw(std::exception)
{
    return sal_Int16(m_aAttributes.size());
}
OUString SAL_CALL SaxAttrList::getNameByIndex( sal_Int16 i_nIndex ) throw(std::exception)
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? m_aAttributes[i_nIndex].m_aName : OUString();
}

OUString SAL_CALL SaxAttrList::getTypeByIndex( sal_Int16 i_nIndex) throw(std::exception)
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? getCDATAString() : OUString();
}

OUString SAL_CALL SaxAttrList::getTypeByName( const OUString& i_rName ) throw(std::exception)
{
    return (m_aIndexMap.find( i_rName ) != m_aIndexMap.end()) ? getCDATAString() : OUString();
}

OUString SAL_CALL SaxAttrList::getValueByIndex( sal_Int16 i_nIndex ) throw(std::exception)
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? m_aAttributes[i_nIndex].m_aValue : OUString();
}

OUString SAL_CALL SaxAttrList::getValueByName(const OUString& i_rName) throw(std::exception)
{
    std::unordered_map< OUString, size_t, OUStringHash >::const_iterator it = m_aIndexMap.find( i_rName );
    return (it != m_aIndexMap.end()) ? m_aAttributes[it->second].m_aValue : OUString();
}

css::uno::Reference< css::util::XCloneable > SAL_CALL SaxAttrList::createClone() throw(std::exception)
{
    return new SaxAttrList( *this );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
