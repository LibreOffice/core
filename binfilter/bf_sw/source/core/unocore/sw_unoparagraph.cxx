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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <cmdid.h>
#include <unomid.h>

#include <errhdl.hxx>

#include <unoobj.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <unoprnms.hxx>
#include <unocrsrhelper.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <bf_svtools/svstdarr.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::drawing;


using rtl::OUString;

/******************************************************************
 * SwXParagraph
 ******************************************************************/
/* -----------------------------11.07.00 12:10--------------------------------

 ---------------------------------------------------------------------------*/
SwXParagraph* SwXParagraph::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xParaTunnel( xRef, uno::UNO_QUERY);
    if(xParaTunnel.is())
        return (SwXParagraph*)xParaTunnel->getSomething(SwXParagraph::getUnoTunnelId());
    return 0;
}
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXParagraph::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXParagraph::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXParagraph::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParagraph");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextContent") ||
        sServiceName.EqualsAscii("com.sun.star.text.Paragraph") ||
         sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesAsian")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesComplex")||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesComplex");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXParagraph::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(8);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Paragraph");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    pArray[7] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph() :
    aLstnrCntnr( (XTextRange*)this),
    xParentText(0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(TRUE),
    nSelectionStartPos(-1),
    nSelectionEndPos(-1)
{
}

/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr, sal_Int32 nSelStart, sal_Int32 nSelEnd) :
    SwClient(pCrsr),
    xParentText(pParent),
    aLstnrCntnr( (XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(FALSE),
    nSelectionStartPos(nSelStart),
    nSelectionEndPos(nSelEnd)
{
}
/*-- 11.12.98 08:12:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::~SwXParagraph()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;
}
/* -----------------------------11.07.00 14:48--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::attachToText(SwXText* pParent, SwUnoCrsr* pCrsr)
{
    DBG_ASSERT(m_bIsDescriptor, "Paragraph is not a descriptor");
    if(m_bIsDescriptor)
    {
        m_bIsDescriptor = FALSE;
        pCrsr->Add(this);
        xParentText = pParent;
        if(m_sText.getLength())
        {
            try	{ setString(m_sText); }
            catch(...){}
            m_sText = OUString();
        }
    }
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXParagraph::getPropertySetInfo(void)
                                            throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
        WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    Sequence<Any> aValues(1);
    aValues.getArray()[0] = aValue;
    setPropertyValues(aPropertyNames, aValues);
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraph::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    Sequence< Any > aRet = getPropertyValues(aPropertyNames );
    return aRet.getConstArray()[0];
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::setPropertyValues(
    const Sequence< OUString >& rPropertyNames,
    const Sequence< Any >& aValues )
        throw(PropertyVetoException, IllegalArgumentException,
                        WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const Any* pValues = aValues.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        OUString sTmp;
        SwParaSelection aParaSel(pUnoCrsr);
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                if ( pMap->nFlags & PropertyAttribute::READONLY)
                    throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

                SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet,
                                        sTmp, pValues[nProp], pMap);
                pMap++;
            }
            else
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
    else
        throw uno::RuntimeException();
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > SwXParagraph::getPropertyValues(
    const Sequence< OUString >& rPropertyNames )
        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< Any > aValues(rPropertyNames.getLength());
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        Any* pValues = aValues.getArray();
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
        SwAttrSet& rAttrSet = ((SwTxtNode&)rTxtNode).GetSwAttrSet();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                if(!SwXParagraph::getDefaultTextContentValue(
                    pValues[nProp], pPropertyNames[nProp], pMap->nWID))
                {
                    BOOL bDone = FALSE;
                    PropertyState eTemp;
                    bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                pMap, *pUnoCrsr, &(pValues[nProp]), eTemp, rTxtNode.GetTxtNode() );
                    if(!bDone)
                        pValues[nProp] = aPropSet.getPropertyValue(*pMap, rAttrSet);
                }
                ++pMap;
            }
            else
                throw RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
    else
        throw RuntimeException();
    return aValues;
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::addPropertiesChangeListener(
    const Sequence< OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::removePropertiesChangeListener(
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::firePropertiesChangeEvent(
    const Sequence< OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{}
/* -----------------------------12.09.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::getDefaultTextContentValue(Any& rAny, const OUString& rPropertyName, USHORT nWID)
{
    if(!nWID)
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)))
            nWID = FN_UNO_ANCHOR_TYPE;
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES)))
            nWID = FN_UNO_ANCHOR_TYPES;
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_TEXT_WRAP)))
            nWID = FN_UNO_TEXT_WRAP;
        else
            return FALSE;
    }

    switch(nWID)
    {
        case FN_UNO_TEXT_WRAP:  rAny <<= WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {	Sequence<TextContentAnchorType> aTypes(1);
            TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
            rAny.setValue(&aTypes, ::getCppuType((uno::Sequence<TextContentAnchorType>*)0));
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addPropertyChangeListener(
    const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
//-----------------------------------------------------------------------------
beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            SwUnoCrsr& rUnoCrsr,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertyMap& rMap,
                            sal_Bool &rAttrSetFetched )
                                throw( beans::UnknownPropertyException)
{
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;

    if(!(*ppSet) && !rAttrSetFetched )
    {
        SwNode& rTxtNode = rUnoCrsr.GetPoint()->nNode.GetNode();
        (*ppSet) = ((SwTxtNode&)rTxtNode).GetpSwAttrSet();
        rAttrSetFetched = sal_True;
    }
    switch( rMap.nWID )
    {
    case FN_UNO_NUM_RULES:
        //wenn eine Numerierung gesetzt ist, dann hier herausreichen, sonst nichts tun
        SwUnoCursorHelper::getNumberingProperty( rUnoCrsr, eRet, NULL );
        break;
    case FN_UNO_ANCHOR_TYPES:
        break;
    case RES_ANCHOR:
        if ( MID_SURROUND_SURROUNDTYPE != rMap.nMemberId )
            goto lcl_SwXParagraph_getPropertyStateDEFAULT;
        break;
    case RES_SURROUND:
        if ( MID_ANCHOR_ANCHORTYPE != rMap.nMemberId )
            goto lcl_SwXParagraph_getPropertyStateDEFAULT;
        break;
    case FN_UNO_PARA_STYLE:
    case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        {
            SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(
                rUnoCrsr, rMap.nWID == FN_UNO_PARA_CONDITIONAL_STYLE_NAME);
            eRet = pFmt ? beans::PropertyState_DIRECT_VALUE
                        : beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
    case FN_UNO_PAGE_STYLE:
        {
            String sVal;
            SwUnoCursorHelper::GetCurPageStyle( rUnoCrsr, sVal );
            eRet = sVal.Len() ? beans::PropertyState_DIRECT_VALUE
                              : beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
    lcl_SwXParagraph_getPropertyStateDEFAULT:
    default:
        if((*ppSet) && SFX_ITEM_SET == (*ppSet)->GetItemState(rMap.nWID, FALSE))
            eRet = beans::PropertyState_DIRECT_VALUE;
        break;
    }
    return eRet;
}

/*-- 05.03.99 11:37:30---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXParagraph::getPropertyState(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName );
        if(!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        sal_Bool bDummy = sal_False;
        eRet = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet, *pMap,
                                                     bDummy );
    }
    else
        throw uno::RuntimeException();
    return eRet;
}
/*-- 05.03.99 11:37:32---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Sequence< beans::PropertyState > SwXParagraph::getPropertyStates(
        const uno::Sequence< OUString >& PropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const OUString* pNames = PropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        sal_Bool bAttrSetFetched = sal_False;
        for(sal_Int32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd; i++,++pStates,++pMap,++pNames )
        {
            pMap = SfxItemPropertyMap::GetByName( pMap, *pNames );
            if(!pMap)
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + *pNames, static_cast < cppu::OWeakObject * > ( this ) );
            if (bAttrSetFetched && !pSet &&
                pMap->nWID >= RES_CHRATR_BEGIN &&
                pMap->nWID <= RES_UNKNOWNATR_END )
                *pStates = beans::PropertyState_DEFAULT_VALUE;
            else
                *pStates = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet,*pMap, bAttrSetFetched );
        }
    }
    else
        throw uno::RuntimeException();

    return aRet;
}
/*-- 05.03.99 11:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setPropertyToDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)) ||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES)) ||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_TEXT_WRAP)))
                return;

        // Absatz selektieren
        SwParaSelection aParaSel(pUnoCrsr);
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw RuntimeException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only:" ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            if(pMap->nWID < RES_FRMATR_END)
            {
                SvUShortsSort aWhichIds;
                aWhichIds.Insert(pMap->nWID);
                if(pMap->nWID < RES_PARATR_BEGIN)
                    pUnoCrsr->GetDoc()->ResetAttr(*pUnoCrsr, sal_True, &aWhichIds);
                else
                {
                    //fuer Absatzattribute muss die Selektion jeweils auf
                    //Absatzgrenzen erweitert werden
                    SwPosition aStart = *pUnoCrsr->Start();
                    SwPosition aEnd = *pUnoCrsr->End();
                    SwUnoCrsr* pTemp = pUnoCrsr->GetDoc()->CreateUnoCrsr(aStart, sal_False);
                    if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
                    {
                        pTemp->MovePara(fnParaCurr, fnParaStart);
                    }
                    pTemp->SetMark();
                    *pTemp->GetPoint() = aEnd;
                    //pTemp->Exchange();
                    SwXTextCursor::SelectPam(*pTemp, sal_True);
                    if(!SwUnoCursorHelper::IsEndOfPara(*pTemp))
                    {
                        pTemp->MovePara(fnParaCurr, fnParaEnd);
                    }
                    pTemp->GetDoc()->ResetAttr(*pTemp, sal_True, &aWhichIds);
                    delete pTemp;
                }
            }
            else
                SwUnoCursorHelper::resetCrsrPropertyValue(pMap, *pUnoCrsr);
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
}
/*-- 05.03.99 11:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraph::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
            return aRet;

        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if(pMap->nWID < RES_FRMATR_END)
            {
                const SfxPoolItem& rDefItem =
                    pUnoCrsr->GetDoc()->GetAttrPool().GetDefaultItem(pMap->nWID);
                rDefItem.QueryValue(aRet, pMap->nMemberId);
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();

    return aRet;
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::attach(const uno::Reference< XTextRange > & xTextRange)
                    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    // SwXParagraph will only created in order to be inserteb by
    // 'insertTextContentBefore' or 'insertTextContentAfter' therefore
    // they cannot be attached
    throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        // Absatz selektieren
        SwParaSelection aSelection(pUnoCrsr);
        aRet = new SwXTextRange(*pUnoCrsr, xParentText);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 08:12:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        // Absatz selektieren
        {
            SwParaSelection aSelection(pUnoCrsr);
            pUnoCrsr->GetDoc()->DelFullPara(*pUnoCrsr);
        }
        aLstnrCntnr.Disposing();
        delete pUnoCrsr;
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 08:12:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:53---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXParagraph::createEnumeration(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        aRef = new SwXTextPortionEnumeration(*pUnoCrsr, xParentText, nSelectionStartPos, nSelectionEndPos);
    else
        throw uno::RuntimeException();
    return aRef;

}
/*-- 11.12.98 08:12:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXParagraph::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/*-- 11.12.98 08:12:54---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXParagraph::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(((SwXParagraph*)this)->GetCrsr())
        return sal_True;
    else
        return sal_False;
}
/*-- 11.12.98 08:12:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXParagraph::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 11.12.98 08:12:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getStart(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 08:12:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getEnd(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 08:12:56---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXParagraph::getString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwParaSelection aSelection(pUnoCrsr);
        SwXTextCursor::getTextFromPam(*pUnoCrsr, aRet);
    }
    else if(IsDescriptor())
        aRet = m_sText;
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 08:12:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setString(const OUString& aString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    if(pUnoCrsr)
    {
        if(!SwUnoCursorHelper::IsStartOfPara(*pUnoCrsr))
            pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
        SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);
        if(pUnoCrsr->GetNode()->GetTxtNode()->GetTxt().Len())
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
        SwXTextCursor::SetString(*pUnoCrsr, aString);
        SwXTextCursor::SelectPam(*pUnoCrsr, sal_False);
    }
    else if(IsDescriptor())
        m_sText = aString;
    else
        throw uno::RuntimeException();

}
/* -----------------23.03.99 12:49-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXParagraph::createContentEnumeration(const OUString& rServiceName)
    throw( uno::RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr || COMPARE_EQUAL != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw uno::RuntimeException();

    uno::Reference< container::XEnumeration >  xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_PARAGRAPH);
    return xRet;
}
/* -----------------23.03.99 12:49-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXParagraph::getAvailableServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 08:12:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
