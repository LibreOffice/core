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

#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <xmloff/xmlcnimp.hxx>
#include <xmloff/unoatrcn.hxx>
#include <editeng/xmlcnitm.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;


TYPEINIT1(SvXMLAttrContainerItem, SfxPoolItem);

SvXMLAttrContainerItem::SvXMLAttrContainerItem( sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich )
{
    pImpl = new SvXMLAttrContainerData;
}

SvXMLAttrContainerItem::SvXMLAttrContainerItem(
                                        const SvXMLAttrContainerItem& rItem ) :
    SfxPoolItem( rItem )
{
    pImpl = new SvXMLAttrContainerData( *rItem.pImpl );
}

SvXMLAttrContainerItem::~SvXMLAttrContainerItem()
{
    delete pImpl;
}

int SvXMLAttrContainerItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( rItem.ISA(SvXMLAttrContainerItem),
               "SvXMLAttrContainerItem::operator ==(): Bad type");
    return *pImpl == *((const SvXMLAttrContainerItem&)rItem).pImpl;
}

int SvXMLAttrContainerItem::Compare( const SfxPoolItem &/*rWith*/ ) const
{
    DBG_ASSERT( !this, "not yet implemented" );

    return 0;
}

SfxItemPresentation SvXMLAttrContainerItem::GetPresentation(
                    SfxItemPresentation /*ePresentation*/,
                    SfxMapUnit /*eCoreMetric*/,
                    SfxMapUnit /*ePresentationMetric*/,
                    OUString & /*rText*/,
                    const IntlWrapper * /*pIntlWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_uInt16 SvXMLAttrContainerItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    // This item should never be stored
    return USHRT_MAX;
}

bool SvXMLAttrContainerItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    Reference<XNameContainer> xContainer =
        new SvUnoAttributeContainer( new SvXMLAttrContainerData( *pImpl ) );

    rVal.setValue( &xContainer, ::getCppuType((Reference<XNameContainer>*)0) );
    return true;
}

bool SvXMLAttrContainerItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    Reference<XInterface> xRef;
    SvUnoAttributeContainer* pContainer = NULL;

    if( rVal.getValue() != NULL && rVal.getValueType().getTypeClass() == TypeClass_INTERFACE )
    {
        xRef = *(Reference<XInterface>*)rVal.getValue();
        Reference<XUnoTunnel> xTunnel(xRef, UNO_QUERY);
        if( xTunnel.is() )
            pContainer = (SvUnoAttributeContainer*)(sal_uLong)xTunnel->getSomething(SvUnoAttributeContainer::getUnoTunnelId());
    }

    if( pContainer )
    {
        delete pImpl;
        pImpl = new SvXMLAttrContainerData( * pContainer->GetContainerImpl() );
    }
    else
    {
        std::auto_ptr<SvXMLAttrContainerData> pNewImpl(new SvXMLAttrContainerData);

        try
        {
            Reference<XNameContainer> xContainer( xRef, UNO_QUERY );
            if( !xContainer.is() )
                return sal_False;

            const Sequence< ::rtl::OUString > aNameSequence( xContainer->getElementNames() );
            const ::rtl::OUString* pNames = aNameSequence.getConstArray();
            const sal_Int32 nCount = aNameSequence.getLength();
            Any aAny;
            AttributeData* pData;
            sal_Int32 nAttr;

            for( nAttr = 0; nAttr < nCount; nAttr++ )
            {
                const ::rtl::OUString aName( *pNames++ );

                aAny = xContainer->getByName( aName );
                if( aAny.getValue() == NULL || aAny.getValueType() != ::getCppuType((AttributeData*)0) )
                    return sal_False;

                pData = (AttributeData*)aAny.getValue();
                sal_Int32 pos = aName.indexOf( sal_Unicode(':') );
                if( pos != -1 )
                {
                    const ::rtl::OUString aPrefix( aName.copy( 0, pos ));
                    const ::rtl::OUString aLName( aName.copy( pos+1 ));

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
                pImpl = pNewImpl.release();
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


sal_Bool SvXMLAttrContainerItem::AddAttr( const ::rtl::OUString& rLName,
                                        const ::rtl::OUString& rValue )
{
    return pImpl->AddAttr( rLName, rValue );
}

sal_Bool SvXMLAttrContainerItem::AddAttr( const ::rtl::OUString& rPrefix,
          const ::rtl::OUString& rNamespace, const ::rtl::OUString& rLName,
          const ::rtl::OUString& rValue )
{
    return pImpl->AddAttr( rPrefix, rNamespace, rLName, rValue );
}

sal_uInt16 SvXMLAttrContainerItem::GetAttrCount() const
{
    return (sal_uInt16)pImpl->GetAttrCount();
}

::rtl::OUString SvXMLAttrContainerItem::GetAttrNamespace( sal_uInt16 i ) const
{
    return pImpl->GetAttrNamespace( i );
}

::rtl::OUString SvXMLAttrContainerItem::GetAttrPrefix( sal_uInt16 i ) const
{
    return pImpl->GetAttrPrefix( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetAttrLName( sal_uInt16 i ) const
{
    return pImpl->GetAttrLName( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetAttrValue( sal_uInt16 i ) const
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

const ::rtl::OUString& SvXMLAttrContainerItem::GetNamespace( sal_uInt16 i ) const
{
    return pImpl->GetNamespace( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetPrefix( sal_uInt16 i ) const
{
    return pImpl->GetPrefix( i );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
