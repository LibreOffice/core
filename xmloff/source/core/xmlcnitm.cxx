/*************************************************************************
 *
 *  $RCSfile: xmlcnitm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:19:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#include "xmlcnitm.hxx"
#include "xmlcnimp.hxx"
#include "unoatrcn.hxx"

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;

SV_IMPL_PTRARR( OUStringsDtor2_Impl, OUStringPtr )


SvXMLAttrContainerItem_Impl::SvXMLAttrContainerItem_Impl(
                            const SvXMLAttrContainerItem_Impl& rImpl ) :
    aNamespaceMap( rImpl.aNamespaceMap )
{
    USHORT nCount = rImpl.aLNames.Count();
    for( USHORT i=0; i<nCount; i++ )
    {
        aPrefixPoss.Insert( rImpl.aPrefixPoss[i], i );
        aLNames.Insert( new OUString( *rImpl.aLNames[i] ), i );
        aValues.Insert( new OUString( *rImpl.aValues[i] ), i );
    }
}

SvXMLAttrContainerItem_Impl::SvXMLAttrContainerItem_Impl()
{
}

int SvXMLAttrContainerItem_Impl::operator ==(
                        const SvXMLAttrContainerItem_Impl& rCmp ) const
{
    BOOL bRet = aLNames.Count() == rCmp.aLNames.Count() &&
                aNamespaceMap == rCmp.aNamespaceMap;
    if( bRet )
    {
        USHORT nCount = aLNames.Count();
        USHORT i;
        for( i=0; bRet && i < nCount; i++ )
            bRet = aPrefixPoss[i] == rCmp.aPrefixPoss[i];

        if( bRet )
        {
            for( i=0; bRet && i < nCount; i++ )
                bRet = *aLNames[i] == *rCmp.aLNames[i] &&
                       *aValues[i] == *rCmp.aValues[i];
        }
    }

    return (int)bRet;
}

BOOL SvXMLAttrContainerItem_Impl::AddAttr( const OUString& rLName,
                                              const OUString& rValue )
{
    aPrefixPoss.Insert( USHRT_MAX, aPrefixPoss.Count() );
    aLNames.Insert( new OUString(rLName), aLNames.Count() );
    aValues.Insert( new OUString(rValue), aValues.Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerItem_Impl::AddAttr( const OUString& rPrefix,
                                              const OUString& rNamespace,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    USHORT nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    aLNames.Insert( new OUString(rLName), aLNames.Count() );
    aValues.Insert( new OUString(rValue), aValues.Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerItem_Impl::AddAttr( const OUString& rPrefix,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    USHORT nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return FALSE;

    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    aLNames.Insert( new OUString(rLName), aLNames.Count() );
    aValues.Insert( new OUString(rValue), aValues.Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerItem_Impl::SetAt( USHORT i,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    *aLNames[i] = rLName;
    *aValues[i] = rValue;
    aPrefixPoss[i] = USHRT_MAX;

    return TRUE;
}

BOOL SvXMLAttrContainerItem_Impl::SetAt( USHORT i,
            const rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    USHORT nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    if( USHRT_MAX == nPos )
        return FALSE;

    *aLNames[i] = rLName;
    *aValues[i] = rValue;
    aPrefixPoss[i] = nPos;

    return TRUE;
}

BOOL SvXMLAttrContainerItem_Impl::SetAt( USHORT i,
            const rtl::OUString& rPrefix,
            const rtl::OUString& rLName,
            const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    USHORT nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return FALSE;

    *aLNames[i] = rLName;
    *aValues[i] = rValue;
    aPrefixPoss[i] = nPos;

    return TRUE;
}

void SvXMLAttrContainerItem_Impl::Remove( USHORT i )
{
    if( i < GetAttrCount() )
    {
        delete aLNames[i];
        aLNames.Remove( i );
        delete aValues[i];
        aValues.Remove( i );
        aPrefixPoss.Remove( i );
    }
    else
    {
        DBG_ERROR( "illegal index" );
    }
}

// ------------------------------------------------------------------------

TYPEINIT1(SvXMLAttrContainerItem, SfxPoolItem);

SvXMLAttrContainerItem::SvXMLAttrContainerItem( USHORT nWhich ) :
    SfxPoolItem( nWhich )
{
    pImpl = new SvXMLAttrContainerItem_Impl;
}

SvXMLAttrContainerItem::SvXMLAttrContainerItem(
                                        const SvXMLAttrContainerItem& rItem ) :
    SfxPoolItem( rItem )
{
    pImpl = new SvXMLAttrContainerItem_Impl( *rItem.pImpl );
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

int SvXMLAttrContainerItem::Compare( const SfxPoolItem &rWith ) const
{
    DBG_ASSERT( !this, "not yet implemented" );

    return 0;
}

SfxItemPresentation SvXMLAttrContainerItem::GetPresentation(
                    SfxItemPresentation ePresentation,
                    SfxMapUnit eCoreMetric,
                    SfxMapUnit ePresentationMetric,
                    XubString &rText,
                    const International *pInternational ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

USHORT SvXMLAttrContainerItem::GetVersion( USHORT nFileFormatVersion ) const
{
    // This item should never be stored
    return USHRT_MAX;
}

BOOL  SvXMLAttrContainerItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    Reference<XNameContainer> xContainer =
        new SvUnoAttributeContainer( new SvXMLAttrContainerItem_Impl( *pImpl ) );

    rVal.setValue( &xContainer, ::getCppuType((Reference<XNameContainer>*)0) );
    return TRUE;
}
BOOL SvXMLAttrContainerItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    Reference<XInterface> xRef;
    SvUnoAttributeContainer* pContainer = NULL;

    if( rVal.getValue() != NULL && rVal.getValueType().getTypeClass() == TypeClass_INTERFACE )
    {
        xRef = *(Reference<XInterface>*)rVal.getValue();
        Reference<XUnoTunnel> xTunnel(xRef, UNO_QUERY);
        pContainer = (SvUnoAttributeContainer*)xTunnel->getSomething(SvUnoAttributeContainer::getUnoTunnelId());
    }

    if( pContainer )
    {
        delete pImpl;
        pImpl = new SvXMLAttrContainerItem_Impl( * pContainer->GetContainerImpl() );
    }
    else
    {
        SvXMLAttrContainerItem_Impl* pNewImpl = new SvXMLAttrContainerItem_Impl;

        try
        {
            Reference<XNameContainer> xContainer( xRef, UNO_QUERY );
            if( !xContainer.is() )
                return FALSE;

            const Sequence< OUString > aNameSequence( xContainer->getElementNames() );
            const OUString* pNames = aNameSequence.getConstArray();
            const INT32 nCount = aNameSequence.getLength();
            Any aAny;
            AttributeData* pData;

            for( INT32 nAttr = 0; nAttr < nCount; nAttr++ )
            {
                const OUString aName( *pNames++ );

                aAny = xContainer->getByName( aName );
                if( aAny.getValue() == NULL || aAny.getValueType() != ::getCppuType((AttributeData*)0) )
                    return FALSE;

                pData = (AttributeData*)aAny.getValue();
                USHORT pos = aName.indexOf( sal_Unicode(':') );
                if( pos != -1 )
                {
                    const OUString aPrefix( aName.copy( 0, pos ));
                    const OUString aLName( aName.copy( pos+1 ));

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
                return FALSE;
            }
            else
            {
                delete pNewImpl;
            }
        }
        catch(...)
        {
            delete pNewImpl;
            return FALSE;
        }
    }
    return TRUE;
}


BOOL SvXMLAttrContainerItem::AddAttr( const OUString& rLName,
                                        const OUString& rValue )
{
    return pImpl->AddAttr( rLName, rValue );
}

BOOL SvXMLAttrContainerItem::AddAttr( const OUString& rPrefix,
          const OUString& rNamespace, const OUString& rLName,
          const OUString& rValue )
{
    return pImpl->AddAttr( rPrefix, rNamespace, rLName, rValue );
}

USHORT SvXMLAttrContainerItem::GetAttrCount() const
{
    return (USHORT)pImpl->GetAttrCount();
}

OUString SvXMLAttrContainerItem::GetAttrNamespace( USHORT i ) const
{
    return pImpl->GetAttrNamespace( i );
}

OUString SvXMLAttrContainerItem::GetAttrPrefix( USHORT i ) const
{
    return pImpl->GetAttrPrefix( i );
}

const OUString& SvXMLAttrContainerItem::GetAttrLName( USHORT i ) const
{
    return pImpl->GetAttrLName( i );
}

const OUString& SvXMLAttrContainerItem::GetAttrValue( USHORT i ) const
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

const OUString& SvXMLAttrContainerItem::GetNamespace( USHORT i ) const
{
    return pImpl->GetNamespace( i );
}

const OUString& SvXMLAttrContainerItem::GetPrefix( USHORT i ) const
{
    return pImpl->GetPrefix( i );
}


