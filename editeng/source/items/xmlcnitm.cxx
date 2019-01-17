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
#include <climits>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <o3tl/any.hxx>
#include <xmloff/xmlcnimp.hxx>
#include <xmloff/unoatrcn.hxx>
#include <editeng/xmlcnitm.hxx>
#include <tools/debug.hxx>
#include <tools/solar.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;


SvXMLAttrContainerItem::SvXMLAttrContainerItem( sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich ),
    pImpl( new SvXMLAttrContainerData )
{
}

SvXMLAttrContainerItem::SvXMLAttrContainerItem(
                                        const SvXMLAttrContainerItem& rItem ) :
    SfxPoolItem( rItem ),
    pImpl( new SvXMLAttrContainerData( *rItem.pImpl ) )
{
}

SvXMLAttrContainerItem::~SvXMLAttrContainerItem()
{
}

bool SvXMLAttrContainerItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( dynamic_cast< const SvXMLAttrContainerItem* >(&rItem) !=  nullptr,
               "SvXMLAttrContainerItem::operator ==(): Bad type");
    return *pImpl == *static_cast<const SvXMLAttrContainerItem&>(rItem).pImpl;
}

bool SvXMLAttrContainerItem::GetPresentation(
                    SfxItemPresentation /*ePresentation*/,
                    MapUnit /*eCoreMetric*/,
                    MapUnit /*ePresentationMetric*/,
                    OUString & /*rText*/,
                    const IntlWrapper& /*rIntlWrapper*/ ) const
{
    return false;
}

sal_uInt16 SvXMLAttrContainerItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    // This item should never be stored
    return USHRT_MAX;
}

bool SvXMLAttrContainerItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    Reference<XNameContainer> xContainer
        = new SvUnoAttributeContainer(std::make_unique<SvXMLAttrContainerData>(*pImpl));

    rVal <<= xContainer;
    return true;
}

bool SvXMLAttrContainerItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    SvUnoAttributeContainer* pContainer = nullptr;

    Reference<XUnoTunnel> xTunnel(rVal, UNO_QUERY);
    if( xTunnel.is() )
        pContainer = reinterpret_cast<SvUnoAttributeContainer*>(static_cast<sal_uLong>(xTunnel->getSomething(SvUnoAttributeContainer::getUnoTunnelId())));

    if( pContainer )
    {
        pImpl.reset( new SvXMLAttrContainerData( * pContainer->GetContainerImpl() ) );
    }
    else
    {
        std::unique_ptr<SvXMLAttrContainerData> pNewImpl(new SvXMLAttrContainerData);

        try
        {
            Reference<XNameContainer> xContainer( rVal, UNO_QUERY );
            if( !xContainer.is() )
                return false;

            const Sequence< OUString > aNameSequence( xContainer->getElementNames() );
            const OUString* pNames = aNameSequence.getConstArray();
            const sal_Int32 nCount = aNameSequence.getLength();
            Any aAny;
            sal_Int32 nAttr;

            for( nAttr = 0; nAttr < nCount; nAttr++ )
            {
                const OUString aName( *pNames++ );

                aAny = xContainer->getByName( aName );
                auto pData = o3tl::tryAccess<AttributeData>(aAny);
                if( !pData )
                    return false;

                sal_Int32 pos = aName.indexOf( ':' );
                if( pos != -1 )
                {
                    const OUString aPrefix( aName.copy( 0, pos ));
                    const OUString aLName( aName.copy( pos+1 ));

                    if( pData->Namespace.isEmpty() )
                    {
                        if( !pNewImpl->AddAttr( aPrefix, aLName, pData->Value ) )
                            break;
                    }
                    else
                    {
                        if( !pNewImpl->AddAttr( aPrefix, pData->Namespace, aLName, pData->Value ) )
                            break;
                    }
                }
                else
                {
                    if( !pNewImpl->AddAttr( aName, pData->Value ) )
                        break;
                }
            }

            if( nAttr == nCount )
                pImpl = std::move(pNewImpl);
            else
                return false;
        }
        catch(...)
        {
            return false;
        }
    }
    return true;
}


bool SvXMLAttrContainerItem::AddAttr( const OUString& rLName,
                                        const OUString& rValue )
{
    return pImpl->AddAttr( rLName, rValue );
}

bool SvXMLAttrContainerItem::AddAttr( const OUString& rPrefix,
          const OUString& rNamespace, const OUString& rLName,
          const OUString& rValue )
{
    return pImpl->AddAttr( rPrefix, rNamespace, rLName, rValue );
}

sal_uInt16 SvXMLAttrContainerItem::GetAttrCount() const
{
    return static_cast<sal_uInt16>(pImpl->GetAttrCount());
}

OUString SvXMLAttrContainerItem::GetAttrNamespace( sal_uInt16 i ) const
{
    return pImpl->GetAttrNamespace( i );
}

OUString SvXMLAttrContainerItem::GetAttrPrefix( sal_uInt16 i ) const
{
    return pImpl->GetAttrPrefix( i );
}

const OUString& SvXMLAttrContainerItem::GetAttrLName( sal_uInt16 i ) const
{
    return pImpl->GetAttrLName( i );
}

const OUString& SvXMLAttrContainerItem::GetAttrValue( sal_uInt16 i ) const
{
    return pImpl->GetAttrValue( i );
}


sal_uInt16 SvXMLAttrContainerItem::GetFirstNamespaceIndex() const
{
    return pImpl->GetFirstNamespaceIndex();
}

sal_uInt16 SvXMLAttrContainerItem::GetNextNamespaceIndex( sal_uInt16 nIdx ) const
{
    return pImpl->GetNextNamespaceIndex( nIdx );
}

const OUString& SvXMLAttrContainerItem::GetNamespace( sal_uInt16 i ) const
{
    return pImpl->GetNamespace( i );
}

const OUString& SvXMLAttrContainerItem::GetPrefix( sal_uInt16 i ) const
{
    return pImpl->GetPrefix( i );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
