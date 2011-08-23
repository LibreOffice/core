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

#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _XMLOFF_XMLCNIMP_HXX
#include <bf_xmloff/xmlcnimp.hxx>
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include <bf_xmloff/unoatrcn.hxx>
#endif
#include "xmlcnitm.hxx"
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;

// ------------------------------------------------------------------------

/*N*/ TYPEINIT1(SvXMLAttrContainerItem, SfxPoolItem);

/*N*/ SvXMLAttrContainerItem::SvXMLAttrContainerItem( USHORT nWhich ) :
/*N*/ 	SfxPoolItem( nWhich )
/*N*/ {
/*N*/ 	pImpl = new SvXMLAttrContainerData;
/*N*/ }

/*N*/ SvXMLAttrContainerItem::SvXMLAttrContainerItem(
/*N*/ 										const SvXMLAttrContainerItem& rItem ) :
/*N*/ 	SfxPoolItem( rItem )
/*N*/ {
/*N*/ 	pImpl = new SvXMLAttrContainerData( *rItem.pImpl );
/*N*/ }

/*N*/ SvXMLAttrContainerItem::~SvXMLAttrContainerItem()
/*N*/ {
/*N*/ 	delete pImpl;
/*N*/ }

/*N*/ int SvXMLAttrContainerItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( rItem.ISA(SvXMLAttrContainerItem),
/*N*/ 			   "SvXMLAttrContainerItem::operator ==(): Bad type");
/*N*/ 	return *pImpl == *((const SvXMLAttrContainerItem&)rItem).pImpl;
/*N*/ }



/*N*/ USHORT SvXMLAttrContainerItem::GetVersion( USHORT nFileFormatVersion ) const
/*N*/ {
/*N*/ 	// This item should never be stored
/*N*/ 	return USHRT_MAX;
/*N*/ }

/*N*/ BOOL  SvXMLAttrContainerItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	Reference<XNameContainer> xContainer =
/*N*/ 		new SvUnoAttributeContainer( new SvXMLAttrContainerData( *pImpl ) );
/*N*/ 
/*N*/ 	rVal.setValue( &xContainer, ::getCppuType((Reference<XNameContainer>*)0) );
/*N*/ 	return TRUE;
/*N*/ }
/*N*/ BOOL SvXMLAttrContainerItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*NBFF*/{
/*NBFF*/ 	Reference<XInterface> xRef;
/*NBFF*/ 	SvUnoAttributeContainer* pContainer = NULL;
/*NBFF*/ 
/*NBFF*/ 	if( rVal.getValue() != NULL && rVal.getValueType().getTypeClass() == TypeClass_INTERFACE )
/*NBFF*/ 	{
/*NBFF*/ 		xRef = *(Reference<XInterface>*)rVal.getValue();
/*NBFF*/ 		Reference<XUnoTunnel> xTunnel(xRef, UNO_QUERY);
/*NBFF*/ 		if( xTunnel.is() )
/*NBFF*/ 			pContainer = (SvUnoAttributeContainer*)xTunnel->getSomething(SvUnoAttributeContainer::getUnoTunnelId());
/*NBFF*/ 	}
/*NBFF*/ 
/*NBFF*/ 	if( pContainer )
/*NBFF*/ 	{
/*NBFF*/ 		delete pImpl;
/*NBFF*/ 		pImpl = new SvXMLAttrContainerData( * pContainer->GetContainerImpl() );
/*NBFF*/ 	}
/*NBFF*/ 	else
/*NBFF*/ 	{
/*NBFF*/ 		SvXMLAttrContainerData* pNewImpl = new SvXMLAttrContainerData;
/*NBFF*/ 
/*NBFF*/ 		try
/*NBFF*/ 		{
/*NBFF*/ 			Reference<XNameContainer> xContainer( xRef, UNO_QUERY );
/*NBFF*/ 			if( !xContainer.is() )
/*NBFF*/ 				return FALSE;
/*NBFF*/ 
/*NBFF*/ 			const Sequence< OUString > aNameSequence( xContainer->getElementNames() );
/*NBFF*/ 			const OUString* pNames = aNameSequence.getConstArray();
/*NBFF*/ 			const INT32 nCount = aNameSequence.getLength();
/*NBFF*/ 			Any aAny;
/*NBFF*/ 			AttributeData* pData;
/*NBFF*/ 
/*NBFF*/ 			INT32 nAttr; for( nAttr = 0; nAttr < nCount; nAttr++ )
/*NBFF*/ 			{
/*NBFF*/ 				const OUString aName( *pNames++ );
/*NBFF*/ 
/*NBFF*/ 				aAny = xContainer->getByName( aName );
/*NBFF*/ 				if( aAny.getValue() == NULL || aAny.getValueType() != ::getCppuType((AttributeData*)0) )
/*NBFF*/ 					return FALSE;
/*NBFF*/ 
/*NBFF*/ 				pData = (AttributeData*)aAny.getValue();
/*NBFF*/ 				USHORT pos = aName.indexOf( sal_Unicode(':') );
/*NBFF*/ 				if( pos != -1 )
/*NBFF*/ 				{
/*NBFF*/ 					const OUString aPrefix( aName.copy( 0, pos ));
/*NBFF*/ 					const OUString aLName( aName.copy( pos+1 ));
/*NBFF*/ 
/*NBFF*/ 					if( pData->Namespace.getLength() == 0 )
/*NBFF*/ 					{
/*NBFF*/ 						if( !pNewImpl->AddAttr( aPrefix, aLName, pData->Value ) )
/*NBFF*/ 							break;
/*NBFF*/ 					}
/*NBFF*/ 					else
/*NBFF*/ 					{
/*NBFF*/ 						if( !pNewImpl->AddAttr( aPrefix, pData->Namespace, aLName, pData->Value ) )
/*NBFF*/ 							break;
/*NBFF*/ 					}
/*NBFF*/ 				}
/*NBFF*/ 				else
/*NBFF*/ 				{
/*NBFF*/ 					if( !pNewImpl->AddAttr( aName, pData->Value ) )
/*NBFF*/ 						break;
/*NBFF*/ 				}
/*NBFF*/ 			}
/*NBFF*/ 
/*NBFF*/ 			if( nAttr == nCount )
/*NBFF*/ 			{
/*NBFF*/ 				delete pImpl;
/*NBFF*/ 				pImpl = pNewImpl;
/*NBFF*/ 				return FALSE;
/*NBFF*/ 			}
/*NBFF*/ 			else
/*NBFF*/ 			{
/*NBFF*/ 				delete pNewImpl;
/*NBFF*/ 			}
/*NBFF*/ 		}
/*NBFF*/ 		catch(...)
/*NBFF*/ 		{
/*NBFF*/ 			delete pNewImpl;
/*NBFF*/ 			return FALSE;
/*NBFF*/ 		}
/*NBFF*/ 	}
/*NBFF*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL SvXMLAttrContainerItem::AddAttr( const OUString& rLName,
/*N*/ 								  	  const OUString& rValue )
/*N*/ {
/*N*/ 	return pImpl->AddAttr( rLName, rValue );
/*N*/ }

/*N*/ BOOL SvXMLAttrContainerItem::AddAttr( const OUString& rPrefix,
/*N*/ 		  const OUString& rNamespace, const OUString& rLName,
/*N*/ 		  const OUString& rValue )
/*N*/ {
/*N*/ 	return pImpl->AddAttr( rPrefix, rNamespace, rLName, rValue );
/*N*/ }

/*N*/ USHORT SvXMLAttrContainerItem::GetAttrCount() const
/*N*/ {
/*N*/ 	return (USHORT)pImpl->GetAttrCount();
/*N*/ }

/*N*/ OUString SvXMLAttrContainerItem::GetAttrNamespace( USHORT i ) const
/*N*/ {
/*N*/ 	return pImpl->GetAttrNamespace( i );
/*N*/ }

/*N*/ OUString SvXMLAttrContainerItem::GetAttrPrefix( USHORT i ) const
/*N*/ {
/*N*/ 	return pImpl->GetAttrPrefix( i );
/*N*/ }

/*N*/ const OUString& SvXMLAttrContainerItem::GetAttrLName( USHORT i ) const
/*N*/ {
/*N*/ 	return pImpl->GetAttrLName( i );
/*N*/ }

/*N*/ const OUString& SvXMLAttrContainerItem::GetAttrValue( USHORT i ) const
/*N*/ {
/*N*/ 	return pImpl->GetAttrValue( i );
/*N*/ }


/*N*/ USHORT SvXMLAttrContainerItem::GetFirstNamespaceIndex() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*N*/ }

/*N*/ USHORT SvXMLAttrContainerItem::GetNextNamespaceIndex( USHORT nIdx ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001
/*N*/ }

/*N*/ const OUString& SvXMLAttrContainerItem::GetNamespace( USHORT i ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return  OUString::createFromAscii (""); //STRIP001
/*N*/ }

/*N*/ const OUString& SvXMLAttrContainerItem::GetPrefix( USHORT i ) const
/*N*/ {
/*N*/ 		DBG_BF_ASSERT(0, "STRIP"); return ::rtl::OUString::createFromAscii ("");//STRIP001 /*N*/ 	return pImpl->GetPrefix( i );
/*N*/ }


}
