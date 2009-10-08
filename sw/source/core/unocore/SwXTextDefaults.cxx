/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwXTextDefaults.cxx,v $
 * $Revision: 1.22 $
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
#include "precompiled_sw.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
#include <hintids.hxx>

#include <unomid.h>


using rtl::OUString;
using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

// declarations
void lcl_setPageDesc(SwDoc*, const uno::Any&, SfxItemSet& ); // from unoobj.cxx


SwXTextDefaults::SwXTextDefaults ( SwDoc * pNewDoc ) :
    aPropSet( aSwMapProvider.GetPropertyMap ( PROPERTY_MAP_TEXT_DEFAULT ) ),
    pDoc    ( pNewDoc )
{
}


SwXTextDefaults::~SwXTextDefaults ()
{
}


uno::Reference< XPropertySetInfo > SAL_CALL SwXTextDefaults::getPropertySetInfo(  )
        throw(RuntimeException)
{
    static uno::Reference < XPropertySetInfo > xRef = aPropSet.getPropertySetInfo();
    return xRef;
}


void SAL_CALL SwXTextDefaults::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
    if (RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId)
    {
        SfxItemSet aSet( pDoc->GetAttrPool(), RES_PAGEDESC, RES_PAGEDESC );
        aSet.Put(rItem);
        lcl_setPageDesc( pDoc, aValue, aSet );
        pDoc->SetDefault(aSet.Get(RES_PAGEDESC));
    }
    else if ((RES_PARATR_DROP == pMap->nWID && MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId) ||
             (RES_TXTATR_CHARFMT == pMap->nWID))
    {
        OUString uStyle;
        if(aValue >>= uStyle)
        {
            String sStyle;
            SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
            SwDocStyleSheet* pStyle =
                (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
            SwFmtDrop* pDrop = 0;
            SwFmtCharFmt *pCharFmt = 0;
            if(pStyle)
            {
                rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pStyle ) );
                if (RES_PARATR_DROP == pMap->nWID)
                {
                    pDrop = (SwFmtDrop*)rItem.Clone();   // because rItem ist const...
                    pDrop->SetCharFmt(xStyle->GetCharFmt());
                    pDoc->SetDefault(*pDrop);
                }
                else // RES_TXTATR_CHARFMT == pMap->nWID
                {
                    pCharFmt = (SwFmtCharFmt*)rItem.Clone();   // because rItem ist const...
                    pCharFmt->SetCharFmt(xStyle->GetCharFmt());
                    pDoc->SetDefault(*pCharFmt);
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
        pDoc->SetDefault(*pNewItem);
        delete pNewItem;
    }
}


Any SAL_CALL SwXTextDefaults::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
    rItem.QueryValue( aRet, pMap->nMemberId );
    return aRet;
}


void SAL_CALL SwXTextDefaults::addPropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::removePropertyChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::addVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}


void SAL_CALL SwXTextDefaults::removeVetoableChangeListener( const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}


// XPropertyState
PropertyState SAL_CALL SwXTextDefaults::getPropertyState( const OUString& rPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
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
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "setPropertyToDefault: property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SfxItemPool& rSet (pDoc->GetAttrPool());
    rSet.ResetPoolDefaultItem ( pMap->nWID );
}


Any SAL_CALL SwXTextDefaults::getPropertyDefault( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    Any aRet;
    SfxItemPool& rSet (pDoc->GetAttrPool());
    const SfxPoolItem *pItem = rSet.GetPoolDefaultItem ( pMap->nWID );
    pItem->QueryValue( aRet, pMap->nMemberId );
    return aRet;
}


rtl::OUString SAL_CALL SwXTextDefaults::getImplementationName(  )
    throw (RuntimeException)
{
    return C2U("SwXTextDefaults");
}


sal_Bool SAL_CALL SwXTextDefaults::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    return  rServiceName == C2U("com.sun.star.text.Defaults") ||
            rServiceName == C2U("com.sun.star.style.CharacterProperties") ||
            rServiceName == C2U("com.sun.star.style.CharacterPropertiesAsian") ||
            rServiceName == C2U("com.sun.star.style.CharacterPropertiesComplex") ||
            rServiceName == C2U("com.sun.star.style.ParagraphProperties") ||
            rServiceName == C2U("com.sun.star.style.ParagraphPropertiesAsian") ||
            rServiceName == C2U("com.sun.star.style.ParagraphPropertiesComplex");
}


uno::Sequence< ::rtl::OUString > SAL_CALL SwXTextDefaults::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    uno::Sequence< OUString > aRet(7);
    OUString* pArr = aRet.getArray();
    *pArr++ = C2U("com.sun.star.text.Defaults");
    *pArr++ = C2U("com.sun.star.style.CharacterProperties");
    *pArr++ = C2U("com.sun.star.style.CharacterPropertiesAsian");
    *pArr++ = C2U("com.sun.star.style.CharacterPropertiesComplex");
    *pArr++ = C2U("com.sun.star.style.ParagraphProperties");
    *pArr++ = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    *pArr++ = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}



