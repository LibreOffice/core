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


#include <vector>

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <xmloff/xmltoken.hxx>
#include <cppuhelper/implbase.hxx>

#include <xmloff/attrlist.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

sal_Int16 SAL_CALL SvXMLAttributeList::getLength()
{
    return sal::static_int_cast< sal_Int16 >(vecAttribute.size());
}


SvXMLAttributeList::SvXMLAttributeList( const SvXMLAttributeList &r ) :
    cppu::WeakImplHelper<css::xml::sax::XAttributeList, css::util::XCloneable, css::lang::XUnoTunnel>(r),
    vecAttribute( r.vecAttribute )
{
}

SvXMLAttributeList::SvXMLAttributeList( const uno::Reference< xml::sax::XAttributeList> & rAttrList )
{
    SvXMLAttributeList* pImpl =
        comphelper::getUnoTunnelImplementation<SvXMLAttributeList>( rAttrList );

    if( pImpl )
        vecAttribute = pImpl->vecAttribute;
    else
        AppendAttributeList( rAttrList );
}

OUString SAL_CALL SvXMLAttributeList::getNameByIndex(sal_Int16 i)
{
    assert( o3tl::make_unsigned(i) < vecAttribute.size() );
    return ( o3tl::make_unsigned( i ) < vecAttribute.size() ) ? vecAttribute[i].sName : OUString();
}


OUString SAL_CALL SvXMLAttributeList::getTypeByIndex(sal_Int16)
{
    return "CDATA";
}

OUString SAL_CALL  SvXMLAttributeList::getValueByIndex(sal_Int16 i)
{
    assert( o3tl::make_unsigned(i) < vecAttribute.size() );
    return ( o3tl::make_unsigned( i ) < vecAttribute.size() ) ? vecAttribute[i].sValue : OUString();
}

OUString SAL_CALL SvXMLAttributeList::getTypeByName( const OUString& )
{
    return "CDATA";
}

OUString SAL_CALL SvXMLAttributeList::getValueByName(const OUString& sName)
{
    auto ii = std::find_if(vecAttribute.begin(), vecAttribute.end(),
        [&sName](SvXMLTagAttribute_Impl& rAttr) { return rAttr.sName == sName; });

    if (ii != vecAttribute.end())
        return (*ii).sValue;

    return OUString();
}


uno::Reference< css::util::XCloneable >  SvXMLAttributeList::createClone()
{
    uno::Reference< css::util::XCloneable >  r = new SvXMLAttributeList( *this );
    return r;
}


SvXMLAttributeList::SvXMLAttributeList()
{
    vecAttribute.reserve(20); // performance improvement during adding
}


SvXMLAttributeList::~SvXMLAttributeList()
{
}


void SvXMLAttributeList::AddAttribute(  const OUString &sName ,
                                        const OUString &sValue )
{
    assert( !sName.isEmpty() && "empty attribute name is invalid");
    assert( std::count(sName.getStr(), sName.getStr() + sName.getLength(), u':') <= 1 && "too many colons");
    vecAttribute.emplace_back( SvXMLTagAttribute_Impl { sName , sValue } );
}

void SvXMLAttributeList::Clear()
{
    vecAttribute.clear();
}

void SvXMLAttributeList::RemoveAttribute( const OUString& sName )
{
    auto ii = std::find_if(vecAttribute.begin(), vecAttribute.end(),
        [&sName](SvXMLTagAttribute_Impl& rAttr) { return rAttr.sName == sName; });

    if (ii != vecAttribute.end())
        vecAttribute.erase( ii );
}

void SvXMLAttributeList::AppendAttributeList( const uno::Reference< css::xml::sax::XAttributeList >  &r )
{
    OSL_ASSERT( r.is() );

    sal_Int16 nMax = r->getLength();
    sal_Int16 nTotalSize = vecAttribute.size() + nMax;
    vecAttribute.reserve( nTotalSize );

    for( sal_Int16 i = 0 ; i < nMax ; ++i ) {
        OUString sName = r->getNameByIndex( i );
        assert( !sName.isEmpty() && "empty attribute name is invalid");
        assert( std::count(sName.getStr(), sName.getStr() + sName.getLength(), u':') <= 1 && "too many colons");
        vecAttribute.emplace_back(SvXMLTagAttribute_Impl { sName, r->getValueByIndex( i ) });
    }

    OSL_ASSERT( nTotalSize == getLength() );
}

void SvXMLAttributeList::SetValueByIndex( sal_Int16 i,
        const OUString& rValue )
{
    assert( o3tl::make_unsigned(i) < vecAttribute.size() );
    if( o3tl::make_unsigned( i ) < vecAttribute.size() )
    {
        vecAttribute[i].sValue = rValue;
    }
}

void SvXMLAttributeList::RemoveAttributeByIndex( sal_Int16 i )
{
    assert( o3tl::make_unsigned(i) < vecAttribute.size() );
    if( o3tl::make_unsigned( i ) < vecAttribute.size() )
        vecAttribute.erase( vecAttribute.begin() + i );
}

void SvXMLAttributeList::RenameAttributeByIndex( sal_Int16 i,
                                                 const OUString& rNewName )
{
    assert( o3tl::make_unsigned(i) < vecAttribute.size() );
    if( o3tl::make_unsigned( i ) < vecAttribute.size() )
    {
        vecAttribute[i].sName = rNewName;
    }
}

sal_Int16 SvXMLAttributeList::GetIndexByName( const OUString& rName ) const
{
    auto ii = std::find_if(vecAttribute.begin(), vecAttribute.end(),
        [&rName](const SvXMLTagAttribute_Impl& rAttr) { return rAttr.sName == rName; });

    if (ii != vecAttribute.end())
        return static_cast<sal_Int16>(std::distance(vecAttribute.begin(), ii));

    return -1;
}

// XUnoTunnel & co
UNO3_GETIMPLEMENTATION_IMPL(SvXMLAttributeList)


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
