/*************************************************************************
 *
 *  $RCSfile: unoparagraph.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2001-04-03 07:28:56 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <cmdid.h>
#ifndef _UNOMID_H
#include <unomid.h>
#endif

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOCRSRHELPER_HXX
#include <unocrsrhelper.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>

#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::drawing;

using namespace ::rtl;

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
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
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
         sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXParagraph::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(4);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Paragraph");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
     pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
     pArray[3] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph() :
    aLstnrCntnr( (XTextRange*)this),
    xParentText(0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(TRUE)
{
}

/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr) :
    SwClient(pCrsr),
    xParentText(pParent),
    aLstnrCntnr( (XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(FALSE)
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
    DBG_ASSERT(m_bIsDescriptor, "Paragraph is not a descriptor")
    if(m_bIsDescriptor)
    {
        m_bIsDescriptor = FALSE;
        pCrsr->Add(this);
        xParentText = pParent;
        if(m_sText.getLength())
        {
            try { setString(m_sText); }
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
                if(pMap->nFlags & PropertyAttribute::READONLY)
                    throw IllegalArgumentException();
                {
                    IllegalArgumentException aArg;
                    aArg.Message = pPropertyNames[nProp];
                    throw aArg;
                }
                SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet,
                                        sTmp, pValues[nProp], pMap);
            }
            else
            {
                UnknownPropertyException aExcept;
                aExcept.Message = pPropertyNames[nProp];
                throw aExcept;
            }
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< Any > aValues(rPropertyNames.getLength());
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        Any* pValues = aValues.getArray();
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            SwParaSelection aParaSel(pUnoCrsr);
            SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
            SwAttrSet& rAttrSet = ((SwTxtNode&)rTxtNode).GetSwAttrSet();
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                if(!SwXParagraph::getDefaultTextContentValue(
                    pValues[nProp], pPropertyNames[nProp], pMap->nWID))
                {
                    BOOL bDone = FALSE;
                    PropertyState eTemp;
                    bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                pMap, *pUnoCrsr, rAttrSet, pValues[nProp], eTemp );
                    if(!bDone)
                        pValues[nProp] = aPropSet.getPropertyValue(*pMap, rAttrSet);
                }
            }
            else
            {
                UnknownPropertyException aExcept;
                aExcept.Message = pPropertyNames[nProp];
                throw aExcept;
            }
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
        if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE))
            nWID = FN_UNO_ANCHOR_TYPE;
        else if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES))
            nWID = FN_UNO_ANCHOR_TYPES;
        else if(0 == rPropertyName.compareToAscii(UNO_NAME_TEXT_WRAP))
            nWID = FN_UNO_TEXT_WRAP;
        else
            return FALSE;
    }

    switch(nWID)
    {
        case FN_UNO_TEXT_WRAP:  rAny <<= WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {   Sequence<TextContentAnchorType> aTypes(1);
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
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
//-----------------------------------------------------------------------------
beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            SwUnoCrsr& rUnoCrsr,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertyMap& rMap )
                                throw( beans::UnknownPropertyException)
{
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    if( FN_UNO_ANCHOR_TYPES == rMap.nWID  ||
            RES_ANCHOR == rMap.nWID && MID_ANCHOR_ANCHORTYPE == rMap.nMemberId ||
            RES_SURROUND == rMap.nWID  && MID_SURROUND_SURROUNDTYPE == rMap.nMemberId )
        return eRet;

    if(!(*ppSet))
    {
        SwNode& rTxtNode = rUnoCrsr.GetPoint()->nNode.GetNode();
        (*ppSet) = ((SwTxtNode&)rTxtNode).GetpSwAttrSet();
    }
    if(rMap.nWID == FN_UNO_NUM_RULES)
    {
        //wenn eine Numerierung gesetzt ist, dann hier herausreichen, sonst nichts tun
        SwUnoCursorHelper::getNumberingProperty( rUnoCrsr, eRet );
    }
    else if(rMap.nWID == FN_UNO_PARA_STYLE ||
            rMap.nWID == FN_UNO_PARA_CONDITIONAL_STYLE_NAME )
    {
        SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(
            rUnoCrsr, rMap.nWID == FN_UNO_PARA_CONDITIONAL_STYLE_NAME);
        if( !pFmt )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else if(rMap.nWID == FN_UNO_PAGE_STYLE)
    {
        String sVal = SwUnoCursorHelper::GetCurPageStyle( rUnoCrsr );
        if( !sVal.Len() )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else if((*ppSet) && SFX_ITEM_SET == (*ppSet)->GetItemState(rMap.nWID, FALSE))
        eRet = beans::PropertyState_DIRECT_VALUE;
    else
        eRet = beans::PropertyState_DEFAULT_VALUE;

    return eRet;
}

/*-- 05.03.99 11:37:30---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXParagraph::getPropertyState(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName );
        if(!pMap)
        {
            UnknownPropertyException aExcept;
            aExcept.Message = rPropertyName;
            throw aExcept;
        }
        eRet = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet, *pMap );
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    const OUString* pNames = PropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        for(sal_Int32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd; i++ )
        {
            pMap = SfxItemPropertyMap::GetByName( pMap, pNames[i] );
            if(!pMap)
            {
                UnknownPropertyException aExcept;
                aExcept.Message = pNames[i];
                throw aExcept;
            }
            pStates[i] = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet,*pMap);
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE) ||
            0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES)||
                0 == rPropertyName.compareToAscii(UNO_NAME_TEXT_WRAP))
            return;

        // Absatz selektieren
        SwParaSelection aParaSel(pUnoCrsr);
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
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
            throw beans::UnknownPropertyException();
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
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
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
            throw beans::UnknownPropertyException();
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

}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        // Absatz selektieren
        SwParaSelection aSelection(pUnoCrsr);
        pUnoCrsr->GetDoc()->DelFullPara(*pUnoCrsr);
        pUnoCrsr->Remove(this);
        aLstnrCntnr.Disposing();
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        aRef = new SwXTextPortionEnumeration(*pUnoCrsr, xParentText);
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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

