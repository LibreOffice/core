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
#include "precompiled_editeng.hxx"
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

SvXMLAttrContainerItem::SvXMLAttrContainerItem( USHORT _nWhich ) :
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
                    XubString &/*rText*/,
                    const IntlWrapper * /*pIntlWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

USHORT SvXMLAttrContainerItem::GetVersion( USHORT /*nFileFormatVersion*/ ) const
{
    // This item should never be stored
    return USHRT_MAX;
}

bool  SvXMLAttrContainerItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    Reference<XNameContainer> xContainer =
        new SvUnoAttributeContainer( new SvXMLAttrContainerData( *pImpl ) );

    rVal.setValue( &xContainer, ::getCppuType((Reference<XNameContainer>*)0) );
    return true;
}

bool SvXMLAttrContainerItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    Reference<XInterface> xRef;
    SvUnoAttributeContainer* pContainer = NULL;

    if( rVal.getValue() != NULL && rVal.getValueType().getTypeClass() == TypeClass_INTERFACE )
    {
        xRef = *(Reference<XInterface>*)rVal.getValue();
        Reference<XUnoTunnel> xTunnel(xRef, UNO_QUERY);
        if( xTunnel.is() )
            pContainer = (SvUnoAttributeContainer*)(ULONG)xTunnel->getSomething(SvUnoAttributeContainer::getUnoTunnelId());
    }

    if( pContainer )
    {
        delete pImpl;
        pImpl = new SvXMLAttrContainerData( * pContainer->GetContainerImpl() );
    }
    else
    {
        SvXMLAttrContainerData* pNewImpl = new SvXMLAttrContainerData;

        try
        {
            Reference<XNameContainer> xContainer( xRef, UNO_QUERY );
            if( !xContainer.is() )
                return FALSE;

            const Sequence< ::rtl::OUString > aNameSequence( xContainer->getElementNames() );
            const ::rtl::OUString* pNames = aNameSequence.getConstArray();
            const INT32 nCount = aNameSequence.getLength();
            Any aAny;
            AttributeData* pData;
            INT32 nAttr;

            for( nAttr = 0; nAttr < nCount; nAttr++ )
            {
                const ::rtl::OUString aName( *pNames++ );

                aAny = xContainer->getByName( aName );
                if( aAny.getValue() == NULL || aAny.getValueType() != ::getCppuType((AttributeData*)0) )
                    return FALSE;

                pData = (AttributeData*)aAny.getValue();
                sal_Int32 pos = aName.indexOf( sal_Unicode(':') );
                if( pos != -1 )
                {
                    const ::rtl::OUString aPrefix( aName.copy( 0, pos ));
                    const ::rtl::OUString aLName( aName.copy( pos+1 ));

                    if( pData->Namespace.getLength() == 0 )
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
            {
                delete pImpl;
                pImpl = pNewImpl;
            }
            else
            {
                delete pNewImpl;
                return false;
            }
        }
        catch(...)
        {
            delete pNewImpl;
            return false;
        }
    }
    return true;
}


BOOL SvXMLAttrContainerItem::AddAttr( const ::rtl::OUString& rLName,
                                        const ::rtl::OUString& rValue )
{
    return pImpl->AddAttr( rLName, rValue );
}

BOOL SvXMLAttrContainerItem::AddAttr( const ::rtl::OUString& rPrefix,
          const ::rtl::OUString& rNamespace, const ::rtl::OUString& rLName,
          const ::rtl::OUString& rValue )
{
    return pImpl->AddAttr( rPrefix, rNamespace, rLName, rValue );
}

USHORT SvXMLAttrContainerItem::GetAttrCount() const
{
    return (USHORT)pImpl->GetAttrCount();
}

::rtl::OUString SvXMLAttrContainerItem::GetAttrNamespace( USHORT i ) const
{
    return pImpl->GetAttrNamespace( i );
}

::rtl::OUString SvXMLAttrContainerItem::GetAttrPrefix( USHORT i ) const
{
    return pImpl->GetAttrPrefix( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetAttrLName( USHORT i ) const
{
    return pImpl->GetAttrLName( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetAttrValue( USHORT i ) const
{
    return pImpl->GetAttrValue( i );
}


USHORT SvXMLAttrContainerItem::GetFirstNamespaceIndex() const
{
    return pImpl->GetFirstNamespaceIndex();
}

USHORT SvXMLAttrContainerItem::GetNextNamespaceIndex( USHORT nIdx ) const
{
    return pImpl->GetNextNamespaceIndex( nIdx );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetNamespace( USHORT i ) const
{
    return pImpl->GetNamespace( i );
}

const ::rtl::OUString& SvXMLAttrContainerItem::GetPrefix( USHORT i ) const
{
    return pImpl->GetPrefix( i );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
