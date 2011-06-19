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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "saxattrlist.hxx"

namespace pdfi
{

SaxAttrList::SaxAttrList( const boost::unordered_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >& rMap )
{
    m_aAttributes.reserve(rMap.size());
    for( boost::unordered_map< rtl::OUString,
                        rtl::OUString,
                        rtl::OUStringHash >::const_iterator it = rMap.begin();
         it != rMap.end(); ++it )
    {
        m_aIndexMap[ it->first ] = m_aAttributes.size();
        m_aAttributes.push_back( AttrEntry( it->first, it->second ) );
    }
}

SaxAttrList::SaxAttrList( const SaxAttrList& rClone ) :
    cppu::WeakImplHelper2<com::sun::star::xml::sax::XAttributeList, com::sun::star::util::XCloneable>(rClone),
    m_aAttributes( rClone.m_aAttributes ),
    m_aIndexMap( rClone.m_aIndexMap )
{
}

SaxAttrList::~SaxAttrList()
{
}

namespace {
    static const rtl::OUString& getCDATAString()
    {
        static rtl::OUString aStr( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ) );
        return aStr;
    }
}

sal_Int16 SAL_CALL SaxAttrList::getLength() throw()
{
    return sal_Int16(m_aAttributes.size());
}
rtl::OUString SAL_CALL SaxAttrList::getNameByIndex( sal_Int16 i_nIndex ) throw()
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? m_aAttributes[i_nIndex].m_aName : rtl::OUString();
}

rtl::OUString SAL_CALL SaxAttrList::getTypeByIndex( sal_Int16 i_nIndex) throw()
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? getCDATAString() : rtl::OUString();
}

rtl::OUString SAL_CALL SaxAttrList::getTypeByName( const ::rtl::OUString& i_rName ) throw()
{
    return (m_aIndexMap.find( i_rName ) != m_aIndexMap.end()) ? getCDATAString() : rtl::OUString();
}

rtl::OUString SAL_CALL SaxAttrList::getValueByIndex( sal_Int16 i_nIndex ) throw()
{
    return (i_nIndex < sal_Int16(m_aAttributes.size())) ? m_aAttributes[i_nIndex].m_aValue : rtl::OUString();
}

rtl::OUString SAL_CALL SaxAttrList::getValueByName(const ::rtl::OUString& i_rName) throw()
{
    boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it = m_aIndexMap.find( i_rName );
    return (it != m_aIndexMap.end()) ? m_aAttributes[it->second].m_aValue : rtl::OUString();
}

com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL SaxAttrList::createClone() throw()
{
    return new SaxAttrList( *this );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
