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

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <vcl/svapp.hxx>

#include <SwXTextDefaults.hxx>
#include <SwStyleNameMapper.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <docstyle.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <paratr.hxx>
#include <unoprnms.hxx>
#include <unocrsrhelper.hxx>
#include <hintids.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SwXTextDefaults::SwXTextDefaults ( SwDoc * pNewDoc ) :
    m_pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_DEFAULT ) ),
    m_pDoc   ( pNewDoc )
{
}

SwXTextDefaults::~SwXTextDefaults ()
{
}

uno::Reference< XPropertySetInfo > SAL_CALL SwXTextDefaults::getPropertySetInfo(  )
{
    static uno::Reference < XPropertySetInfo > xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SAL_CALL SwXTextDefaults::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
{
    SolarMutexGuard aGuard;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException ( "Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    if (RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId)
    {
        SfxItemSet aSet( m_pDoc->GetAttrPool(), svl::Items<RES_PAGEDESC, RES_PAGEDESC>{} );
        aSet.Put(rItem);
        SwUnoCursorHelper::SetPageDesc( aValue, *m_pDoc, aSet );
        m_pDoc->SetDefault(aSet.Get(RES_PAGEDESC));
    }
    else if ((RES_PARATR_DROP == pMap->nWID && MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId) ||
             (RES_TXTATR_CHARFMT == pMap->nWID))
    {
        OUString uStyle;
        if(!(aValue >>= uStyle))
            throw lang::IllegalArgumentException();

        OUString sStyle;
        SwStyleNameMapper::FillUIName(uStyle, sStyle, SwGetPoolIdFromName::ChrFmt, true );
        SwDocStyleSheet* pStyle =
            static_cast<SwDocStyleSheet*>(m_pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SfxStyleFamily::Char));
        SwFormatDrop* pDrop = nullptr;
        SwFormatCharFormat *pCharFormat = nullptr;
        if(!pStyle)
            throw lang::IllegalArgumentException();

        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *pStyle ) );
        if (xStyle->GetCharFormat() == m_pDoc->GetDfltCharFormat())
            return; // don't SetCharFormat with formats from mpDfltCharFormat

        if (RES_PARATR_DROP == pMap->nWID)
        {
            pDrop = static_cast<SwFormatDrop*>(rItem.Clone());   // because rItem is const...
            pDrop->SetCharFormat(xStyle->GetCharFormat());
            m_pDoc->SetDefault(*pDrop);
        }
        else // RES_TXTATR_CHARFMT == pMap->nWID
        {
            pCharFormat = static_cast<SwFormatCharFormat*>(rItem.Clone());   // because rItem is const...
            pCharFormat->SetCharFormat(xStyle->GetCharFormat());
            m_pDoc->SetDefault(*pCharFormat);
        }

        delete pDrop;
        delete pCharFormat;
    }
    else
    {
        SfxPoolItem * pNewItem = rItem.Clone();
        pNewItem->PutValue( aValue, pMap->nMemberId);
        m_pDoc->SetDefault(*pNewItem);
        delete pNewItem;
    }
}

Any SAL_CALL SwXTextDefaults::getPropertyValue( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    rItem.QueryValue( aRet, pMap->nMemberId );
    return aRet;
}

void SAL_CALL SwXTextDefaults::addPropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
    OSL_FAIL ( "not implemented" );
}

void SAL_CALL SwXTextDefaults::removePropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
    OSL_FAIL ( "not implemented" );
}

void SAL_CALL SwXTextDefaults::addVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
    OSL_FAIL ( "not implemented" );
}

void SAL_CALL SwXTextDefaults::removeVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
    OSL_FAIL ( "not implemented" );
}

// XPropertyState
PropertyState SAL_CALL SwXTextDefaults::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    if (IsStaticDefaultItem ( &rItem ) )
        eRet = PropertyState_DEFAULT_VALUE;
    return eRet;
}

Sequence< PropertyState > SAL_CALL SwXTextDefaults::getPropertyStates( const Sequence< OUString >& rPropertyNames )
{
    const sal_Int32 nCount = rPropertyNames.getLength();
    const OUString * pNames = rPropertyNames.getConstArray();
    Sequence < PropertyState > aRet ( nCount );
    PropertyState *pState = aRet.getArray();

    for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++)
        pState[nIndex] = getPropertyState( pNames[nIndex] );

    return aRet;
}

void SAL_CALL SwXTextDefaults::setPropertyToDefault( const OUString& rPropertyName )
{
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw RuntimeException( "setPropertyToDefault: property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SfxItemPool& rSet (m_pDoc->GetAttrPool());
    rSet.ResetPoolDefaultItem ( pMap->nWID );
}

Any SAL_CALL SwXTextDefaults::getPropertyDefault( const OUString& rPropertyName )
{
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    SfxItemPool& rSet (m_pDoc->GetAttrPool());
    SfxPoolItem const*const pItem = rSet.GetPoolDefaultItem(pMap->nWID);
    if (pItem)
    {
        pItem->QueryValue( aRet, pMap->nMemberId );
    }
    return aRet;
}

OUString SAL_CALL SwXTextDefaults::getImplementationName(  )
{
    return OUString("SwXTextDefaults");
}

sal_Bool SAL_CALL SwXTextDefaults::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwXTextDefaults::getSupportedServiceNames(  )
{
    uno::Sequence< OUString > aRet(7);
    OUString* pArr = aRet.getArray();
    *pArr++ = "com.sun.star.text.Defaults";
    *pArr++ = "com.sun.star.style.CharacterProperties";
    *pArr++ = "com.sun.star.style.CharacterPropertiesAsian";
    *pArr++ = "com.sun.star.style.CharacterPropertiesComplex";
    *pArr++ = "com.sun.star.style.ParagraphProperties";
    *pArr++ = "com.sun.star.style.ParagraphPropertiesAsian";
    *pArr++ = "com.sun.star.style.ParagraphPropertiesComplex";
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
