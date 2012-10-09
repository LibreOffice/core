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


#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/stlunosequence.hxx>

#include <SwXTextDefaults.hxx>
#include <SwStyleNameMapper.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
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
using rtl::OUString;


SwXTextDefaults::SwXTextDefaults ( SwDoc * pNewDoc ) :
    m_pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_DEFAULT ) ),
    m_pDoc   ( pNewDoc )
{
}


SwXTextDefaults::~SwXTextDefaults ()
{
}


uno::Reference< XPropertySetInfo > SAL_CALL SwXTextDefaults::getPropertySetInfo(  )
        throw(RuntimeException)
{
    static uno::Reference < XPropertySetInfo > xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}


void SAL_CALL SwXTextDefaults::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    if (RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId)
    {
        SfxItemSet aSet( m_pDoc->GetAttrPool(), RES_PAGEDESC, RES_PAGEDESC );
        aSet.Put(rItem);
        SwUnoCursorHelper::SetPageDesc( aValue, *m_pDoc, aSet );
        m_pDoc->SetDefault(aSet.Get(RES_PAGEDESC));
    }
    else if ((RES_PARATR_DROP == pMap->nWID && MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId) ||
             (RES_TXTATR_CHARFMT == pMap->nWID))
    {
        OUString uStyle;
        if(aValue >>= uStyle)
        {
            String sStyle;
            SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
            SwDocStyleSheet* pStyle =
                (SwDocStyleSheet*)m_pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
            SwFmtDrop* pDrop = 0;
            SwFmtCharFmt *pCharFmt = 0;
            if(pStyle)
            {
                rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pStyle ) );
                if (RES_PARATR_DROP == pMap->nWID)
                {
                    pDrop = (SwFmtDrop*)rItem.Clone();   // because rItem ist const...
                    pDrop->SetCharFmt(xStyle->GetCharFmt());
                    m_pDoc->SetDefault(*pDrop);
                }
                else // RES_TXTATR_CHARFMT == pMap->nWID
                {
                    pCharFmt = (SwFmtCharFmt*)rItem.Clone();   // because rItem ist const...
                    pCharFmt->SetCharFmt(xStyle->GetCharFmt());
                    m_pDoc->SetDefault(*pCharFmt);
                }
            }
            else
                throw lang::IllegalArgumentException();
            delete pDrop;
            delete pCharFmt;
        }
        else
            throw lang::IllegalArgumentException();
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
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    rItem.QueryValue( aRet, pMap->nMemberId );
    return aRet;
}


void SAL_CALL SwXTextDefaults::addPropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::removePropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::addVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::removeVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL ( "not implemented" );
}


// XPropertyState
PropertyState SAL_CALL SwXTextDefaults::getPropertyState( const OUString& rPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = m_pDoc->GetDefault(pMap->nWID);
    if (IsStaticDefaultItem ( &rItem ) )
        eRet = PropertyState_DEFAULT_VALUE;
    return eRet;
}


Sequence< PropertyState > SAL_CALL SwXTextDefaults::getPropertyStates( const Sequence< OUString >& rPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
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
        throw(UnknownPropertyException, RuntimeException)
{
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "setPropertyToDefault: property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SfxItemPool& rSet (m_pDoc->GetAttrPool());
    rSet.ResetPoolDefaultItem ( pMap->nWID );
}


Any SAL_CALL SwXTextDefaults::getPropertyDefault( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if (!m_pDoc)
        throw RuntimeException();
    const SfxItemPropertySimpleEntry *pMap = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    SfxItemPool& rSet (m_pDoc->GetAttrPool());
    const SfxPoolItem *pItem = rSet.GetPoolDefaultItem ( pMap->nWID );
    pItem->QueryValue( aRet, pMap->nMemberId );
    return aRet;
}


rtl::OUString SAL_CALL SwXTextDefaults::getImplementationName(  )
    throw (RuntimeException)
{
    return OUString("SwXTextDefaults");
}


sal_Bool SAL_CALL SwXTextDefaults::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSeq(getSupportedServiceNames());
    return std::find(comphelper::stl_begin(aSeq), comphelper::stl_end(aSeq), rServiceName) != comphelper::stl_end(aSeq);
}


uno::Sequence< ::rtl::OUString > SAL_CALL SwXTextDefaults::getSupportedServiceNames(  )
    throw (RuntimeException)
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
