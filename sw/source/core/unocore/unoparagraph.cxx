/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoparagraph.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:32:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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

//#ifndef _COM_SUN_STAR_BEANS_SETPROPERTYTOLERANTFAILED_HPP_
//#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_GETPROPERTYTOLERANTRESULT_HPP_
//#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_TOLERANTPROPERTYSETRESULTTYPE_HPP_
//#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
//#endif
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
using ::rtl::OUString;

/******************************************************************
 * forward declarations
 ******************************************************************/

beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            SwUnoCrsr& rUnoCrsr,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertyMap& rMap,
                            sal_Bool &rAttrSetFetched )
                                throw( beans::UnknownPropertyException);

/******************************************************************
 * SwXParagraph
 ******************************************************************/
/* -----------------------------11.07.00 12:10--------------------------------

 ---------------------------------------------------------------------------*/
SwXParagraph* SwXParagraph::GetImplementation(uno::Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xParaTunnel( xRef, uno::UNO_QUERY);
    if(xParaTunnel.is())
        return reinterpret_cast< SwXParagraph * >(
                sal::static_int_cast< sal_IntPtr >( xParaTunnel->getSomething(SwXParagraph::getUnoTunnelId()) ));
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
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXParagraph::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXParagraph");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
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
uno::Sequence< OUString > SwXParagraph::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(8);
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
    xParentText(0),
    aLstnrCntnr( (text::XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    nSelectionStartPos(-1),
    nSelectionEndPos(-1),
    m_bIsDescriptor(TRUE)
{
}

/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr, sal_Int32 nSelStart, sal_Int32 nSelEnd) :
    SwClient(pCrsr),
    xParentText(pParent),
    aLstnrCntnr( (text::XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    nSelectionStartPos(nSelStart),
    nSelectionEndPos(nSelEnd),
    m_bIsDescriptor(FALSE)
{
}
/*-- 11.12.98 08:12:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::~SwXParagraph()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
uno::Reference< beans::XPropertySetInfo >  SwXParagraph::getPropertySetInfo(void)
                                            throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    uno::Sequence<uno::Any> aValues(1);
    aValues.getArray()[0] = aValue;
    SetPropertyValues_Impl( aPropertyNames, aValues );
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraph::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    uno::Sequence< uno::Any > aRet = GetPropertyValues_Impl(aPropertyNames );
    return aRet.getConstArray()[0];
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXParagraph::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const uno::Any* pValues = rValues.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        OUString sTmp;
        SwParaSelection aParaSel(pUnoCrsr);
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(!pMap)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            else
            {
                if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                    throw beans::PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

                SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet,
                                        sTmp, pValues[nProp], pMap);
                pMap++;
            }
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXParagraph::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Any > SAL_CALL SwXParagraph::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        uno::Any* pValues = aValues.getArray();
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
        const SwAttrSet& rAttrSet = ((SwTxtNode&)rTxtNode).GetSwAttrSet();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                if(!SwXParagraph::getDefaultTextContentValue(
                    pValues[nProp], pPropertyNames[nProp], pMap->nWID))
                {
                    BOOL bDone = FALSE;
                    beans::PropertyState eTemp;
                    bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                pMap, *pUnoCrsr, &(pValues[nProp]), eTemp, rTxtNode.GetTxtNode() );
                    if(!bDone)
                        pValues[nProp] = aPropSet.getPropertyValue(*pMap, rAttrSet);
                }
                ++pMap;
            }
            else
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
    else
        throw uno::RuntimeException();
    return aValues;
}
/* -----------------------------04.11.03 11:43--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Any > SwXParagraph::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames )
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property exception caught" ) ), static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "WrappedTargetException caught" ) ), static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:43--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------25.09.03 11:09--------------------------------

 ---------------------------------------------------------------------------*/

/* disabled for #i46921#

uno::Sequence< SetPropertyTolerantFailed > SAL_CALL SwXParagraph::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    if (rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
    SwAttrSet& rAttrSet = ((SwTxtNode&)rTxtNode).GetSwAttrSet();
    USHORT nAttrCount = rAttrSet.Count();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    sal_Int32 nVals = rValues.getLength();
    const uno::Any *pValue = rValues.getConstArray();

    sal_Int32 nFailed = 0;
    uno::Sequence< SetPropertyTolerantFailed > aFailed( nProps );
    SetPropertyTolerantFailed *pFailed = aFailed.getArray();

    // get entry to start with
    const SfxItemPropertyMap*   pStartEntry = aPropSet.getPropertyMap();

    OUString sTmp;
    SwParaSelection aParaSel( pUnoCrsr );
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        try
        {
            pFailed[ nFailed ].Name    = pProp[i];

            const SfxItemPropertyMap* pEntry =
                    SfxItemPropertyMap::GetByName( pStartEntry, pProp[i] );
            if (!pEntry)
                pFailed[ nFailed++ ].Result  = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            else
            {
                // set property value
                // (compare to SwXParagraph::setPropertyValues)
                if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
                    pFailed[ nFailed++ ].Result  = TolerantPropertySetResultType::PROPERTY_VETO;
                else
                {
                    SwXTextCursor::SetPropertyValue(
                                *pUnoCrsr, aPropSet, sTmp, pValue[i], pEntry );
                }

                // continue with search for next property after current entry
                // (property map and sequence of property names are sorted!)
                pStartEntry = ++pEntry;
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    aFailed.realloc( nFailed );
    return aFailed;
}


uno::Sequence< GetPropertyTolerantResult > SAL_CALL SwXParagraph::getPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< GetDirectPropertyTolerantResult > aTmpRes(
            GetPropertyValuesTolerant_Impl( rPropertyNames, sal_False ) );
    const GetDirectPropertyTolerantResult *pTmpRes = aTmpRes.getConstArray();

    // copy temporary result to final result type
    sal_Int32 nLen = aTmpRes.getLength();
    uno::Sequence< GetPropertyTolerantResult > aRes( nLen );
    GetPropertyTolerantResult *pRes = aRes.getArray();
    for (sal_Int32 i = 0;  i < nLen;  i++)
        *pRes++ = *pTmpRes++;
    return aRes;
}


uno::Sequence< GetDirectPropertyTolerantResult > SAL_CALL SwXParagraph::getDirectPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return GetPropertyValuesTolerant_Impl( rPropertyNames, sal_True );
}


uno::Sequence< GetDirectPropertyTolerantResult > SAL_CALL SwXParagraph::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        sal_Bool bDirectValuesOnly )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if (!pUnoCrsr)
        throw RuntimeException();
    SwTxtNode* pTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode().GetTxtNode();
    DBG_ASSERT( pTxtNode != NULL, "need text node" );

    // #i46786# Use SwAttrSet pointer for determining the state.
    //          Use the value SwAttrSet (from the paragraph OR the style)
    //          for determining the actual value(s).
    const SwAttrSet* pAttrSet = pTxtNode->GetpSwAttrSet();
    const SwAttrSet& rValueAttrSet = pTxtNode->GetSwAttrSet();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    uno::Sequence< GetDirectPropertyTolerantResult > aResult( nProps );
    GetDirectPropertyTolerantResult *pResult = aResult.getArray();
    sal_Int32 nIdx = 0;

    // get entry to start with
    const SfxItemPropertyMap *pStartEntry = aPropSet.getPropertyMap();

    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        DBG_ASSERT( nIdx < nProps, "index out ouf bounds" );
        GetDirectPropertyTolerantResult &rResult = pResult[nIdx];

        try
        {
            rResult.Name = pProp[i];

            const SfxItemPropertyMap *pEntry =
                    SfxItemPropertyMap::GetByName( pStartEntry, pProp[i] );
            if (!pEntry)  // property available?
                rResult.Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            else
            {
                // get property state
                // (compare to SwXParagraph::getPropertyState)
                sal_Bool bAttrSetFetched = sal_True;
                PropertyState eState = lcl_SwXParagraph_getPropertyState(
                            *pUnoCrsr, &pAttrSet, *pEntry, bAttrSetFetched );
                rResult.State  = eState;

//                if (bDirectValuesOnly  &&  PropertyState_DIRECT_VALUE != eState)
//                    rResult.Result = TolerantPropertySetResultType::NO_DIRECT_VALUE;
//                else
                rResult.Result = TolerantPropertySetResultType::UNKNOWN_FAILURE;
                if (!bDirectValuesOnly  ||  PropertyState_DIRECT_VALUE == eState)
                {
                    // get property value
                    // (compare to SwXParagraph::getPropertyValue(s))
                    uno::Any aValue;
                    if (!SwXParagraph::getDefaultTextContentValue(
                                aValue, pProp[i], pEntry->nWID ) )
                    {
                        // handle properties that are not part of the attribute
                        // and thus only pretendend to be paragraph attributes
                        BOOL bDone = FALSE;
                        PropertyState eTemp;
                        bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                    pEntry, *pUnoCrsr, &aValue, eTemp, pTxtNode );

                        // if not found try the real paragraph attributes...
                        if (!bDone)
                            aValue = aPropSet.getPropertyValue( *pEntry, rValueAttrSet );
                    }

                    rResult.Value  = aValue;
                    rResult.Result = TolerantPropertySetResultType::SUCCESS;

                    nIdx++;
                }
                // this assertion should never occur!
                DBG_ASSERT( nIdx < 1  ||  pResult[nIdx - 1].Result != TolerantPropertySetResultType::UNKNOWN_FAILURE,
                        "unknown failure while retrieving property" );

                // continue with search for next property after current entry
                // (property map and sequence of property names are sorted!)
                pStartEntry = ++pEntry;
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            rResult.Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            rResult.Result = TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            rResult.Result = TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            rResult.Result = TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    // resize to actually used size
    aResult.realloc( nIdx );

    return aResult;
}


*/

/* -----------------------------12.09.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::getDefaultTextContentValue(uno::Any& rAny, const OUString& rPropertyName, USHORT nWID)
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
        case FN_UNO_TEXT_WRAP:  rAny <<= text::WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= text::TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {   uno::Sequence<text::TextContentAnchorType> aTypes(1);
            text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
            rAny.setValue(&aTypes, ::getCppuType((uno::Sequence<text::TextContentAnchorType>*)0));
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
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName );
        if(!pMap)
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + *pNames, static_cast < cppu::OWeakObject * > ( this ) );
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)) ||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES)) ||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_TEXT_WRAP)))
                return;

        // Absatz selektieren
        SwParaSelection aParaSel(pUnoCrsr);
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                throw uno::RuntimeException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only:" ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            if(pMap->nWID < RES_FRMATR_END)
            {
                SvUShortsSort aWhichIds;
                aWhichIds.Insert(pMap->nWID);
                if(pMap->nWID < RES_PARATR_BEGIN)
                    pUnoCrsr->GetDoc()->ResetAttrs(*pUnoCrsr, sal_True, &aWhichIds);
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
                    pTemp->GetDoc()->ResetAttrs(*pTemp, sal_True, &aWhichIds);
                    delete pTemp;
                }
            }
            else
                SwUnoCursorHelper::resetCrsrPropertyValue(pMap, *pUnoCrsr);
        }
        else
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();

    return aRet;
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
                    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // SwXParagraph will only created in order to be inserteb by
    // 'insertTextContentBefore' or 'insertTextContentAfter' therefore
    // they cannot be attached
    throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXParagraph::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  aRet;
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    return ::getCppuType((uno::Reference<text::XTextRange>*)0);
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
uno::Reference< text::XText >  SwXParagraph::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 11.12.98 08:12:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXParagraph::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwParaSelection aSelection(pUnoCrsr);
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< text::XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 08:12:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXParagraph::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwParaSelection aSelection(pUnoCrsr);
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< text::XText >  xParent = getText();
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

