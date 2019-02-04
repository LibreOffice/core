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

#include <memory>
#include <string.h>
#include <com/sun/star/xml/AttributeData.hpp>
#include <o3tl/any.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <limits.h>

#include <xmloff/xmlcnimp.hxx>

#include <xmloff/unoatrcn.hxx>

using namespace ::com::sun::star;

// Interface implementation

uno::Reference< uno::XInterface >  SvUnoAttributeContainer_CreateInstance()
{
    return *(new SvUnoAttributeContainer);
}

SvUnoAttributeContainer::SvUnoAttributeContainer( std::unique_ptr<SvXMLAttrContainerData> pContainer)
: mpContainer( std::move( pContainer ) )
{
    if( !mpContainer )
        mpContainer = std::make_unique<SvXMLAttrContainerData>();
}

// container::XElementAccess
uno::Type SAL_CALL SvUnoAttributeContainer::getElementType()
{
    return cppu::UnoType<xml::AttributeData>::get();
}

sal_Bool SAL_CALL SvUnoAttributeContainer::hasElements()
{
    return mpContainer->GetAttrCount() != 0;
}

sal_uInt16 SvUnoAttributeContainer::getIndexByName(const OUString& aName ) const
{
    const sal_uInt16 nAttrCount = mpContainer->GetAttrCount();

    sal_Int32 nPos = aName.indexOf( ':' );
    if( nPos == -1 )
    {
        for( sal_uInt16 nAttr = 0; nAttr < nAttrCount; nAttr++ )
        {
            if( mpContainer->GetAttrLName(nAttr) == aName &&
                mpContainer->GetAttrPrefix(nAttr).isEmpty() )
                return nAttr;
        }
    }
    else
    {
        const OUString aPrefix( aName.copy( 0L, nPos ) );
        const OUString aLName( aName.copy( nPos+1 ) );

        for( sal_uInt16 nAttr = 0; nAttr < nAttrCount; nAttr++ )
        {
            if( mpContainer->GetAttrLName(nAttr) == aLName &&
                mpContainer->GetAttrPrefix(nAttr) == aPrefix )
                return nAttr;
        }
    }

    return USHRT_MAX;
}

namespace
{
    class theSvUnoAttributeContainerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvUnoAttributeContainerUnoTunnelId> {};
}

const css::uno::Sequence< sal_Int8 > & SvUnoAttributeContainer::getUnoTunnelId() throw()
{
    return theSvUnoAttributeContainerUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SvUnoAttributeContainer::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

// container::XNameAccess
uno::Any SAL_CALL SvUnoAttributeContainer::getByName(const OUString& aName)
{
    sal_uInt16 nAttr = getIndexByName(aName );

    if( nAttr == USHRT_MAX )
        throw container::NoSuchElementException();

    xml::AttributeData aData;
    aData.Namespace = mpContainer->GetAttrNamespace(nAttr);
    aData.Type = "CDATA";
    aData.Value = mpContainer->GetAttrValue(nAttr);

    return uno::Any(aData);
}

uno::Sequence< OUString > SAL_CALL SvUnoAttributeContainer::getElementNames()
{
    const sal_uInt16 nAttrCount = mpContainer->GetAttrCount();

    uno::Sequence< OUString > aElementNames( static_cast<sal_Int32>(nAttrCount) );
    OUString *pNames = aElementNames.getArray();

    for( sal_uInt16 nAttr = 0; nAttr < nAttrCount; nAttr++ )
    {
        OUStringBuffer sBuffer( mpContainer->GetAttrPrefix(nAttr) );
        if( !sBuffer.isEmpty() )
            sBuffer.append( ':' );
        sBuffer.append( mpContainer->GetAttrLName(nAttr) );
        *pNames++ = sBuffer.makeStringAndClear();
    }

    return aElementNames;
}

sal_Bool SAL_CALL SvUnoAttributeContainer::hasByName(const OUString& aName)
{
    return getIndexByName(aName ) != USHRT_MAX;
}

// container::XNameReplace
void SAL_CALL SvUnoAttributeContainer::replaceByName(const OUString& aName, const uno::Any& aElement)
{
    if( auto pData = o3tl::tryAccess<xml::AttributeData>(aElement) )
    {
        sal_uInt16 nAttr = getIndexByName(aName );
        if( nAttr == USHRT_MAX )
            throw container::NoSuchElementException();

        sal_Int32 nPos = aName.indexOf( ':' );
        if( nPos != -1 )
        {
            const OUString aPrefix( aName.copy( 0L, nPos ));
            const OUString aLName( aName.copy( nPos+1 ));

            if( pData->Namespace.isEmpty() )
            {
                if( mpContainer->SetAt( nAttr, aPrefix, aLName, pData->Value ) )
                    return;
            }
            else
            {
                if( mpContainer->SetAt( nAttr, aPrefix, pData->Namespace, aLName, pData->Value ) )
                    return;
            }
        }
        else
        {
            if( pData->Namespace.isEmpty() )
            {
                if( mpContainer->SetAt( nAttr, aName, pData->Value ) )
                    return;
            }
        }
    }

    throw lang::IllegalArgumentException();
}

// container::XNameContainer
void SAL_CALL SvUnoAttributeContainer::insertByName(const OUString& aName, const uno::Any& aElement)
{
    auto pData = o3tl::tryAccess<xml::AttributeData>(aElement);
    if( !pData )
        throw lang::IllegalArgumentException();

    sal_uInt16 nAttr = getIndexByName(aName );
    if( nAttr != USHRT_MAX )
        throw container::ElementExistException();

    sal_Int32 nPos = aName.indexOf( ':' );
    if( nPos != -1 )
    {
        const OUString aPrefix( aName.copy( 0L, nPos ));
        const OUString aLName( aName.copy( nPos+1 ));

        if( pData->Namespace.isEmpty() )
        {
            if( mpContainer->AddAttr( aPrefix, aLName, pData->Value ) )
                return;
        }
        else
        {
            if( mpContainer->AddAttr( aPrefix, pData->Namespace, aLName, pData->Value ) )
                return;
        }
    }
    else
    {
        if( pData->Namespace.isEmpty() )
        {
            if( mpContainer->AddAttr( aName, pData->Value ) )
                return;
        }
    }
}

void SAL_CALL SvUnoAttributeContainer::removeByName(const OUString& Name)
{
    sal_uInt16 nAttr = getIndexByName(Name);
    if( nAttr == USHRT_MAX )
        throw container::NoSuchElementException();

    mpContainer->Remove( nAttr );
}

//XServiceInfo
OUString SAL_CALL SvUnoAttributeContainer::getImplementationName()
{
    return OUString( "SvUnoAttributeContainer" );
}

uno::Sequence< OUString > SvUnoAttributeContainer::getSupportedServiceNames()
{
    OUString aSN( "com.sun.star.xml.AttributeContainer" );
    uno::Sequence< OUString > aNS( &aSN, 1 );
    return aNS;
}

sal_Bool SvUnoAttributeContainer::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
