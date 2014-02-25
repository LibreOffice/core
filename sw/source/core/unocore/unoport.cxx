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

#include <unoport.hxx>

#include <cmdid.h>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>

#include <unocrsrhelper.hxx>
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
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>


using namespace ::com::sun::star;

/******************************************************************
 * SwXTextPortion
 ******************************************************************/

class SwXTextPortion::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    ::cppu::OInterfaceContainerHelper m_EventListeners;

    Impl() : m_EventListeners(m_Mutex) { }
};

void SwXTextPortion::init(const SwUnoCrsr* pPortionCursor)
{
    SwUnoCrsr* pUnoCursor =
        pPortionCursor->GetDoc()->CreateUnoCrsr(*pPortionCursor->GetPoint());
    if (pPortionCursor->HasMark())
    {
        pUnoCursor->SetMark();
        *pUnoCursor->GetMark() = *pPortionCursor->GetMark();
    }
    pUnoCursor->Add(this);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCrsr* pPortionCrsr,
        uno::Reference< text::XText > const& rParent,
        SwTextPortionType eType)
    : m_pImpl(new Impl)
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
    init( pPortionCrsr);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCrsr* pPortionCrsr,
    uno::Reference< text::XText > const& rParent,
    SwFrmFmt& rFmt )
    : m_pImpl(new Impl)
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
    init( pPortionCrsr);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCrsr* pPortionCrsr,
    SwTxtRuby const& rAttr,
    uno::Reference< text::XText > const& xParent,
    sal_Bool bIsEnd )
    : m_pImpl(new Impl)
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
    init( pPortionCrsr);

    if (!bIsEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        rItem.QueryValue(*m_pRubyText, MID_RUBY_TEXT);
        rItem.QueryValue(*m_pRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*m_pRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*m_pRubyIsAbove, MID_RUBY_ABOVE);
    }
}

SwXTextPortion::~SwXTextPortion()
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    delete pUnoCrsr;
}

uno::Reference< text::XText >  SwXTextPortion::getText()
throw( uno::RuntimeException, std::exception )
{
    return m_xParentText;
}

uno::Reference< text::XTextRange >  SwXTextPortion::getStart()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwPaM aPam(*pUnoCrsr->Start());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextPortion::getEnd()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwPaM aPam(*pUnoCrsr->End());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}

OUString SwXTextPortion::getString()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString aTxt;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    // TextPortions are always within a paragraph
    SwTxtNode* pTxtNd = pUnoCrsr->GetNode()->GetTxtNode();
    if ( pTxtNd )
    {
        const sal_Int32 nStt = pUnoCrsr->Start()->nContent.GetIndex();
        aTxt = pTxtNd->GetExpandTxt( nStt,
                pUnoCrsr->End()->nContent.GetIndex() - nStt );
    }
    return aTxt;
}

void SwXTextPortion::setString(const OUString& aString) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetString(*pUnoCrsr, aString);
}

uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
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

void SwXTextPortion::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetPropertyValue(*pUnoCrsr, *m_pPropSet,
            rPropertyName, aValue);
}

void SwXTextPortion::GetPropertyValue(
        uno::Any &rVal,
        const SfxItemPropertySimpleEntry& rEntry,
        SwUnoCrsr *pUnoCrsr,
        SfxItemSet *&pSet )
{
    OSL_ENSURE( pUnoCrsr, "UNO cursor missing" );
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
                case PORTION_REFMARK_END:   pRet = UNO_NAME_REFERENCE_MARK;break;
                case PORTION_TOXMARK_START:
                case PORTION_TOXMARK_END:   pRet = UNO_NAME_DOCUMENT_INDEX_MARK;break;
                case PORTION_BOOKMARK_START :
                case PORTION_BOOKMARK_END : pRet = UNO_NAME_BOOKMARK;break;
                case PORTION_REDLINE_START:
                case PORTION_REDLINE_END:   pRet = "Redline";break;
                case PORTION_RUBY_START:
                case PORTION_RUBY_END:      pRet = "Ruby";break;
                case PORTION_SOFT_PAGEBREAK:pRet = "SoftPageBreak";break;
                case PORTION_META:          pRet = UNO_NAME_META; break;
                case PORTION_FIELD_START:pRet = "TextFieldStart";break;
                case PORTION_FIELD_END:pRet = "TextFieldEnd";break;
                case PORTION_FIELD_START_END:pRet = "TextFieldStartEnd";break;
                case PORTION_ANNOTATION:
                    pRet = "Annotation";
                    break;
                case PORTION_ANNOTATION_END:
                    pRet = "AnnotationEnd";
                    break;
                default:
                    pRet = 0;
                }

                OUString sRet;
                if( pRet )
                    sRet = OUString::createFromAscii( pRet );
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
                bool bDone = SwUnoCursorHelper::getCrsrPropertyValue(
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
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if(pEntry)
            {
                GetPropertyValue( pValues[nProp], *pEntry, pUnoCrsr, pSet );
            }
            else
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
        delete pSet;
    }
    return aValues;
}

uno::Any SwXTextPortion::getPropertyValue(
    const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    return GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}

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
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        uno::Sequence< beans::PropertyValue > aValues( rPropertyNames.getLength() );
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if (!pEntry)
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ("Property is read-only: " + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            aValues[nProp].Name = pPropertyNames[nProp];
            aValues[nProp].Value = pValues[nProp];
        }
        SwUnoCursorHelper::SetPropertyValues( *pUnoCrsr, *m_pPropSet, aValues );
    }
}

void SwXTextPortion::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

uno::Sequence< uno::Any > SwXTextPortion::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames )
        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}

/* disabled for #i46921# */
uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL SwXTextPortion::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

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

    const SfxItemPropertyMap& rPropMap = m_pPropSet->getPropertyMap();

    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        try
        {
            pFailed[ nFailed ].Name    = pProp[i];

            const SfxItemPropertySimpleEntry* pEntry = rPropMap.getByName( pProp[i] );
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
            OSL_FAIL( "unexpected exception caught" );
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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return GetPropertyValuesTolerant_Impl( rPropertyNames, sal_True );
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL SwXTextPortion::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        sal_Bool bDirectValuesOnly )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwUnoCrsr* pUnoCrsr = this->GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    SfxItemSet *pSet = 0;

    const SfxItemPropertyMap& rPropMap = m_pPropSet->getPropertyMap();

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
                  const SfxItemPropertySimpleEntry* pEntry = rPropMap.getByName( pProp[i] );
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
            OSL_FAIL( "unexpected exception caught" );
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

void SwXTextPortion::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException, std::exception)
{}

void SwXTextPortion::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException, std::exception)
{}

void SwXTextPortion::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException, std::exception)
{}

void SwXTextPortion::addPropertyChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXTextPortion::getPropertyState(const OUString& rPropertyName)
            throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    if (GetTextPortionType() == PORTION_RUBY_START &&
        rPropertyName.startsWith("Ruby"))
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

uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
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
            if (pNames[nProp].startsWith("Ruby"))
                pStates[nProp] = beans::PropertyState_DIRECT_VALUE;
        }
    }
    return aRet;
}

void SwXTextPortion::setPropertyToDefault(const OUString& rPropertyName)
                throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    SwUnoCursorHelper::SetPropertyToDefault(
            *pUnoCrsr, *m_pPropSet, rPropertyName);
}

uno::Any SwXTextPortion::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    aRet = SwUnoCursorHelper::GetPropertyDefault(*pUnoCrsr, *m_pPropSet,
                rPropertyName);
    return aRet;
}

void SwXTextPortion::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    // SwXTextPortion cannot be created at the factory therefore
    // they cannot be attached
    throw uno::RuntimeException();
}

uno::Reference< text::XTextRange >  SwXTextPortion::getAnchor()
throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    aRet = new SwXTextRange(*pUnoCrsr, m_xParentText);
    return aRet;
}

void SwXTextPortion::dispose()
throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    setString(OUString());
    pUnoCrsr->Remove(this);
}

void SAL_CALL SwXTextPortion::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXTextPortion::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& /*aServiceName*/)
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    uno::Reference< container::XEnumeration >  xRet =
        new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_CHAR,
                m_pFrameFmt);
    return xRet;

}

namespace
{
    class theSwXTextPortionUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextPortionUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextPortion::getUnoTunnelId()
{
    return theSwXTextPortionUnoTunnelId::get().getSeq();
}

sal_Int64 SwXTextPortion::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextContent";
    return aRet;
}

OUString SwXTextPortion::getImplementationName()
throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextPortion");
}

sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames()
throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    uno::Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextPortion";
    pArray[1] = "com.sun.star.style.CharacterProperties";
    pArray[2] = "com.sun.star.style.CharacterPropertiesAsian";
    pArray[3] = "com.sun.star.style.CharacterPropertiesComplex";
    pArray[4] = "com.sun.star.style.ParagraphProperties";
    pArray[5] = "com.sun.star.style.ParagraphPropertiesAsian";
    pArray[6] = "com.sun.star.style.ParagraphPropertiesComplex";
    return aRet;
}

void SwXTextPortion::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (!m_FrameDepend.GetRegisteredIn())
    {
        m_pFrameFmt = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
