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
#include "precompiled_sw.hxx"


#include <cmdid.h>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>

#include <unocrsrhelper.hxx>
#include <unoport.hxx>
#include <unoparaframeenum.hxx>
#include <unotextrange.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <txtatr.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <frmfmt.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>


using namespace ::com::sun::star;
using ::rtl::OUString;


/******************************************************************
 * SwXTextPortion
 ******************************************************************/

static void init(SwXTextPortion & rPortion, const SwUnoCrsr* pPortionCursor)
{
    SwUnoCrsr* pUnoCursor =
        pPortionCursor->GetDoc()->CreateUnoCrsr(*pPortionCursor->GetPoint());
    if (pPortionCursor->HasMark())
    {
        pUnoCursor->SetMark();
        *pUnoCursor->GetMark() = *pPortionCursor->GetMark();
    }
    pUnoCursor->Add(& rPortion);
}

/*-- 11.12.98 09:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr,
        uno::Reference< text::XText > const& rParent,
        SwTextPortionType eType)
    : m_ListenerContainer( static_cast<text::XTextRange*>(this) )
    , m_pPropSet(aSwMapProvider.GetPropertySet(
        (PORTION_REDLINE_START == eType ||
         PORTION_REDLINE_END   == eType)
            ?  PROPERTY_MAP_REDLINE_PORTION
            :  PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(rParent)
    , m_pRubyText(0)
    , m_pRubyStyle(0)
    , m_pRubyAdjust(0)
    , m_pRubyIsAbove(0)
    , m_FrameDepend(this, 0)
    , m_pFrameFmt(0)
    , m_ePortionType(eType)
    , m_bIsCollapsed(false)
{
    init(*this, pPortionCrsr);
}

/* -----------------24.03.99 16:30-------------------
 *
 * --------------------------------------------------*/
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr,
        uno::Reference< text::XText > const& rParent,
        SwFrmFmt& rFmt )
    : m_ListenerContainer( static_cast<text::XTextRange*>(this) )
    , m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(rParent)
    , m_pRubyText(0)
    , m_pRubyStyle(0)
    , m_pRubyAdjust(0)
    , m_pRubyIsAbove(0)
    , m_FrameDepend(this, &rFmt)
    , m_pFrameFmt(&rFmt)
    , m_ePortionType(PORTION_FRAME)
    , m_bIsCollapsed(false)
{
    init(*this, pPortionCrsr);
}

/* -----------------------------19.02.01 10:52--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr,
                    SwTxtRuby const& rAttr,
                    uno::Reference< text::XText > const& xParent,
                    sal_Bool bIsEnd )
    : m_ListenerContainer( static_cast<text::XTextRange*>(this) )
    , m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(xParent)
    , m_pRubyText   ( bIsEnd ? 0 : new uno::Any )
    , m_pRubyStyle  ( bIsEnd ? 0 : new uno::Any )
    , m_pRubyAdjust ( bIsEnd ? 0 : new uno::Any )
    , m_pRubyIsAbove( bIsEnd ? 0 : new uno::Any )
    , m_FrameDepend(this, 0)
    , m_pFrameFmt(0)
    , m_ePortionType( bIsEnd ? PORTION_RUBY_END : PORTION_RUBY_START )
    , m_bIsCollapsed(false)
{
    init(*this, pPortionCrsr);

    if (!bIsEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        rItem.QueryValue(*m_pRubyText, MID_RUBY_TEXT);
        rItem.QueryValue(*m_pRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*m_pRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*m_pRubyIsAbove, MID_RUBY_ABOVE);
    }
}

/*-- 11.12.98 09:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortion::~SwXTextPortion()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    delete pUnoCrsr;
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText >  SwXTextPortion::getText()
throw( uno::RuntimeException )
{
    return m_xParentText;
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getStart()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwPaM aPam(*pUnoCrsr->Start());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getEnd()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwPaM aPam(*pUnoCrsr->End());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextPortion::getString()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString aTxt;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    // TextPortions are always within a paragraph
    SwTxtNode* pTxtNd = pUnoCrsr->GetNode()->GetTxtNode();
    if ( pTxtNd )
    {
        xub_StrLen nStt = pUnoCrsr->Start()->nContent.GetIndex();
        aTxt = pTxtNd->GetExpandTxt( nStt,
                pUnoCrsr->End()->nContent.GetIndex() - nStt );
    }
    return aTxt;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetString(*pUnoCrsr, aString);
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    //! PropertySetInfo for text portion extensions
    static uno::Reference< beans::XPropertySetInfo >
            xTxtPorExtRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS)->getPropertySetInfo();
    //! PropertySetInfo for redline portions
    static uno::Reference< beans::XPropertySetInfo >
            xRedlPorRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_REDLINE_PORTION)->getPropertySetInfo();

    return (PORTION_REDLINE_START == m_ePortionType ||
            PORTION_REDLINE_END   == m_ePortionType) ? xRedlPorRef : xTxtPorExtRef;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetPropertyValue(*pUnoCrsr, *m_pPropSet,
            rPropertyName, aValue);
}
/*-- 04.11.03 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::GetPropertyValue(
        uno::Any &rVal,
        const SfxItemPropertySimpleEntry& rEntry,
        SwUnoCrsr *pUnoCrsr,
        SfxItemSet *&pSet )
{
    DBG_ASSERT( pUnoCrsr, "UNO cursor missing" );
    if (!pUnoCrsr)
        return;
    if(pUnoCrsr)
    {
        switch(rEntry.nWID)
        {
            case FN_UNO_TEXT_PORTION_TYPE:
            {
                const char* pRet;
                switch (m_ePortionType)
                {
                case PORTION_TEXT:          pRet = "Text";break;
                case PORTION_FIELD:         pRet = "TextField";break;
                case PORTION_FRAME:         pRet = "Frame";break;
                case PORTION_FOOTNOTE:      pRet = "Footnote";break;
                case PORTION_REFMARK_START:
                case PORTION_REFMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_REFERENCE_MARK);break;
                case PORTION_TOXMARK_START:
                case PORTION_TOXMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_DOCUMENT_INDEX_MARK);break;
                case PORTION_BOOKMARK_START :
                case PORTION_BOOKMARK_END : pRet = SW_PROP_NAME_STR(UNO_NAME_BOOKMARK);break;
                case PORTION_REDLINE_START:
                case PORTION_REDLINE_END:   pRet = "Redline";break;
                case PORTION_RUBY_START:
                case PORTION_RUBY_END:      pRet = "Ruby";break;
                case PORTION_SOFT_PAGEBREAK:pRet = "SoftPageBreak";break;
                case PORTION_META:          pRet = SW_PROP_NAME_STR(UNO_NAME_META); break;
                case PORTION_FIELD_START:pRet = "TextFieldStart";break;
                case PORTION_FIELD_END:pRet = "TextFieldEnd";break;
                case PORTION_FIELD_START_END:pRet = "TextFieldStartEnd";break;
                default:
                    pRet = 0;
                }

                OUString sRet;
                if( pRet )
                    sRet = C2U( pRet );
                rVal <<= sRet;
            }
            break;
            case FN_UNO_CONTROL_CHARACTER: // obsolete!
            break;
            case FN_UNO_DOCUMENT_INDEX_MARK:
                rVal <<= m_xTOXMark;
            break;
            case FN_UNO_REFERENCE_MARK:
                rVal <<= m_xRefMark;
            break;
            case FN_UNO_BOOKMARK:
                rVal <<= m_xBookmark;
            break;
            case FN_UNO_FOOTNOTE:
                rVal <<= m_xFootnote;
            break;
            case FN_UNO_TEXT_FIELD:
                rVal <<= m_xTextField;
            break;
            case FN_UNO_META:
                rVal <<= m_xMeta;
            break;
            case FN_UNO_IS_COLLAPSED:
            {
                switch (m_ePortionType)
                {
                    case PORTION_REFMARK_START:
                    case PORTION_BOOKMARK_START :
                    case PORTION_TOXMARK_START:
                    case PORTION_REFMARK_END:
                    case PORTION_TOXMARK_END:
                    case PORTION_BOOKMARK_END :
                    case PORTION_REDLINE_START :
                    case PORTION_REDLINE_END :
                    case PORTION_RUBY_START:
                    case PORTION_RUBY_END:
                    case PORTION_FIELD_START:
                    case PORTION_FIELD_END:
                        rVal.setValue(&m_bIsCollapsed, ::getBooleanCppuType());
                    break;
                    default:
                    break;
                }
            }
            break;
            case FN_UNO_IS_START:
            {
                sal_Bool bStart = sal_True, bPut = sal_True;
                switch (m_ePortionType)
                {
                    case PORTION_REFMARK_START:
                    case PORTION_BOOKMARK_START:
                    case PORTION_TOXMARK_START:
                    case PORTION_REDLINE_START:
                    case PORTION_RUBY_START:
                    case PORTION_FIELD_START:
                    break;

                    case PORTION_REFMARK_END:
                    case PORTION_TOXMARK_END:
                    case PORTION_BOOKMARK_END:
                    case PORTION_REDLINE_END:
                    case PORTION_RUBY_END:
                    case PORTION_FIELD_END:
                        bStart = sal_False;
                    break;
                    default:
                        bPut = sal_False;
                }
                if(bPut)
                    rVal.setValue(&bStart, ::getBooleanCppuType());
            }
            break;
            case RES_TXTATR_CJK_RUBY:
            {
                const uno::Any* pToSet = 0;
                switch(rEntry.nMemberId)
                {
                    case MID_RUBY_TEXT :    pToSet = m_pRubyText.get();   break;
                    case MID_RUBY_ADJUST :  pToSet = m_pRubyAdjust.get(); break;
                    case MID_RUBY_CHARSTYLE:pToSet = m_pRubyStyle.get();  break;
                    case MID_RUBY_ABOVE :   pToSet = m_pRubyIsAbove.get();break;
                }
                if(pToSet)
                    rVal = *pToSet;
            }
            break;
            default:
                beans::PropertyState eTemp;
                sal_Bool bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                    rEntry, *pUnoCrsr, &(rVal), eTemp );
                if(!bDone)
                {
                    if(!pSet)
                    {
                        pSet = new SfxItemSet(pUnoCrsr->GetDoc()->GetAttrPool(),
                            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                            RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                            0L);
                        SwUnoCursorHelper::GetCrsrAttr(*pUnoCrsr, *pSet);
                    }
                    m_pPropSet->getPropertyValue(rEntry, *pSet, rVal);
                }
        }
    }
}


uno::Sequence< uno::Any > SAL_CALL SwXTextPortion::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    uno::Any *pValues = aValues.getArray();
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    {
        SfxItemSet *pSet = 0;
        // get startting pount fo the look-up, either the provided one or else
        // from the beginning of the map
        const SfxItemPropertyMap*   pMap = m_pPropSet->getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = pMap->getByName(pPropertyNames[nProp]);
            if(pEntry)
            {
                GetPropertyValue( pValues[nProp], *pEntry, pUnoCrsr, pSet );
            }
            else
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
        delete pSet;
    }
    return aValues;
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortion::getPropertyValue(
    const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    return GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextPortion::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const uno::Any* pValues = rValues.getConstArray();
        const SfxItemPropertyMap* pMap = m_pPropSet->getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = pMap->getByName(pPropertyNames[nProp]);
            if (!pEntry)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            SwUnoCursorHelper::SetPropertyValue( *pUnoCrsr, *m_pPropSet,
                     pPropertyNames[nProp], pValues[nProp]);
        }
    }
}

void SwXTextPortion::setPropertyValues(
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
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Any > SwXTextPortion::getPropertyValues(
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
/* -----------------------------29.09.03 11:44--------------------------------

 ---------------------------------------------------------------------------*/

/* disabled for #i46921# */

uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL SwXTextPortion::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();
    SwUnoCrsr* pUnoCrsr = this->GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    //sal_Int32 nVals = rValues.getLength();
    const uno::Any *pValue = rValues.getConstArray();

    sal_Int32 nFailed = 0;
    uno::Sequence< beans::SetPropertyTolerantFailed > aFailed( nProps );
    beans::SetPropertyTolerantFailed *pFailed = aFailed.getArray();

    const SfxItemPropertyMap* pPropMap = m_pPropSet->getPropertyMap();

    OUString sTmp;
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        try
        {
            pFailed[ nFailed ].Name    = pProp[i];

            const SfxItemPropertySimpleEntry* pEntry = pPropMap->getByName( pProp[i] );
            if (!pEntry)
                pFailed[ nFailed++ ].Result  = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            else
            {
                // set property value
                // (compare to SwXTextPortion::setPropertyValues)
                if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
                    pFailed[ nFailed++ ].Result  = beans::TolerantPropertySetResultType::PROPERTY_VETO;
                else
                {
                    SwUnoCursorHelper::SetPropertyValue(
                                *pUnoCrsr, *m_pPropSet, pProp[i], pValue[i] );
                }
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            pFailed[ nFailed++ ].Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            pFailed[ nFailed++ ].Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            pFailed[ nFailed++ ].Result = beans::TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            pFailed[ nFailed++ ].Result = beans::TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    aFailed.realloc( nFailed );
    return aFailed;
}


uno::Sequence< beans::GetPropertyTolerantResult > SAL_CALL SwXTextPortion::getPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< beans::GetDirectPropertyTolerantResult > aTmpRes(
            GetPropertyValuesTolerant_Impl( rPropertyNames, sal_False ) );
    const beans::GetDirectPropertyTolerantResult *pTmpRes = aTmpRes.getConstArray();

    // copy temporary result to final result type
    sal_Int32 nLen = aTmpRes.getLength();
    uno::Sequence< beans::GetPropertyTolerantResult > aRes( nLen );
    beans::GetPropertyTolerantResult *pRes = aRes.getArray();
    for (sal_Int32 i = 0;  i < nLen;  i++)
        *pRes++ = *pTmpRes++;
    return aRes;
}


uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL SwXTextPortion::getDirectPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return GetPropertyValuesTolerant_Impl( rPropertyNames, sal_True );
}


uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL SwXTextPortion::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        sal_Bool bDirectValuesOnly )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr* pUnoCrsr = this->GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    SfxItemSet *pSet = 0;

    const SfxItemPropertyMap* pPropMap = m_pPropSet->getPropertyMap();

    uno::Sequence< beans::PropertyState > aPropertyStates =
        SwUnoCursorHelper::GetPropertyStates(
            *pUnoCrsr, *m_pPropSet,
            rPropertyNames,
            SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT );
    const beans::PropertyState* pPropertyStates = aPropertyStates.getConstArray();

    std::vector< beans::GetDirectPropertyTolerantResult > aResultVector;
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        beans::GetDirectPropertyTolerantResult aResult;
        try
        {
            aResult.Name = pProp[i];
            if(pPropertyStates[i] == beans::PropertyState_MAKE_FIXED_SIZE)     // property unknown?
            {
                if( bDirectValuesOnly )
                    continue;
                else
                    aResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            }
            else
            {
                  const SfxItemPropertySimpleEntry* pEntry = pPropMap->getByName( pProp[i] );
                aResult.State  = pPropertyStates[i];

                aResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_FAILURE;
                //#i104499# ruby portion attributes need special handling:
                if( pEntry->nWID == RES_TXTATR_CJK_RUBY &&
                    m_ePortionType == PORTION_RUBY_START )
                {
                        aResult.State = beans::PropertyState_DIRECT_VALUE;
                }
                if (!bDirectValuesOnly  ||  beans::PropertyState_DIRECT_VALUE == aResult.State)
                {
                    // get property value
                    // (compare to SwXTextPortion::getPropertyValue(s))
                    GetPropertyValue( aResult.Value, *pEntry, pUnoCrsr, pSet );
                    aResult.Result = beans::TolerantPropertySetResultType::SUCCESS;
                    aResultVector.push_back( aResult );
                }
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            aResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            aResult.Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            aResult.Result = beans::TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            aResult.Result = beans::TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }
    delete pSet;

    uno::Sequence< beans::GetDirectPropertyTolerantResult > aResult( aResultVector.size() );
    std::vector< beans::GetDirectPropertyTolerantResult >::const_iterator aIt = aResultVector.begin();
    beans::GetDirectPropertyTolerantResult *pResult = aResult.getArray();
    for( sal_Int32 nResult = 0; nResult < aResult.getLength(); ++nResult )
    {
        pResult[nResult] = *aIt;
        ++aIt;
    }

    return aResult;
}


/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addPropertyChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 08.03.99 09:41:43---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXTextPortion::getPropertyState(const OUString& rPropertyName)
            throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    if (GetTextPortionType() == PORTION_RUBY_START &&
        !rPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
    {
        eRet = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        eRet = SwUnoCursorHelper::GetPropertyState(*pUnoCrsr, *m_pPropSet,
                rPropertyName);
    }
    return eRet;
}
/*-- 08.03.99 09:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    uno::Sequence< beans::PropertyState > aRet =
        SwUnoCursorHelper::GetPropertyStates(*pUnoCrsr, *m_pPropSet,
                rPropertyNames, SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION);

    if(GetTextPortionType() == PORTION_RUBY_START)
    {
        const OUString* pNames = rPropertyNames.getConstArray();
        beans::PropertyState* pStates = aRet.getArray();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength();nProp++)
        {
           if(!pNames[nProp].compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
                pStates[nProp] = beans::PropertyState_DIRECT_VALUE;
        }
    }
    return aRet;
}
/*-- 08.03.99 09:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setPropertyToDefault(const OUString& rPropertyName)
                throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetPropertyToDefault(
            *pUnoCrsr, *m_pPropSet, rPropertyName);
}
/*-- 08.03.99 09:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortion::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    aRet = SwUnoCursorHelper::GetPropertyDefault(*pUnoCrsr, *m_pPropSet,
                rPropertyName);
    return aRet;
}

/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // SwXTextPortion cannot be created at the factory therefore
    // they cannot be attached
    throw uno::RuntimeException();
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getAnchor()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    aRet = new SwXTextRange(*pUnoCrsr, m_xParentText);
    return aRet;
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::dispose()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    setString(aEmptyStr);
    pUnoCrsr->Remove(this);
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetRegisteredIn())
        throw uno::RuntimeException();

    m_ListenerContainer.AddListener(aListener);
}
/*-- 11.12.98 09:57:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!GetRegisteredIn() || !m_ListenerContainer.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& /*aServiceName*/)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    uno::Reference< container::XEnumeration >  xRet =
        new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_CHAR,
                m_pFrameFmt);
    return xRet;

}
/* -----------------------------04.07.01 08:52--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextPortion::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------04.07.01 08:52--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SwXTextPortion::getSomething( const uno::Sequence< sal_Int8 >& rId )
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
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/* -----------------25.03.99 10:30-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextPortion::getImplementationName()
throw( uno::RuntimeException )
{
    return C2U("SwXTextPortion");
}
/* -----------------25.03.99 10:30-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Bool bRet = sal_False;
    if(!rServiceName.compareToAscii("com.sun.star.text.TextPortion") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesComplex") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesComplex"))
    {
        bRet = sal_True;
    }

    return bRet;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames()
throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    uno::Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortion");
    pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}
/*-- 11.12.98 09:57:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (!m_FrameDepend.GetRegisteredIn())
    {
        m_pFrameFmt = 0;
    }
}

