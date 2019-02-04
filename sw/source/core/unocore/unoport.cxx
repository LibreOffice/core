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
#include <cppuhelper/exc_hlp.hxx>
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
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

void SwXTextPortion::init(const SwUnoCursor* pPortionCursor)
{
    m_pUnoCursor = pPortionCursor->GetDoc()->CreateUnoCursor(*pPortionCursor->GetPoint());
    if (pPortionCursor->HasMark())
    {
        m_pUnoCursor->SetMark();
        *m_pUnoCursor->GetMark() = *pPortionCursor->GetMark();
    }
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
        uno::Reference< text::XText > const& rParent,
        SwTextPortionType eType)
    : m_pPropSet(aSwMapProvider.GetPropertySet(
        (PORTION_REDLINE_START == eType ||
         PORTION_REDLINE_END   == eType)
            ?  PROPERTY_MAP_REDLINE_PORTION
            :  PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(rParent)
    , m_aDepends(*this)
    , m_pFrameFormat(nullptr)
    , m_ePortionType(eType)
    , m_bIsCollapsed(false)
{
    init( pPortionCursor);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
    uno::Reference< text::XText > const& rParent,
    SwFrameFormat& rFormat )
    : m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(rParent)
    , m_aDepends(*this)
    , m_pFrameFormat(&rFormat)
    , m_ePortionType(PORTION_FRAME)
    , m_bIsCollapsed(false)
{
    m_aDepends.StartListening(&rFormat);
    init( pPortionCursor);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
    SwTextRuby const& rAttr,
    uno::Reference< text::XText > const& xParent,
    bool bIsEnd )
    : m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(xParent)
    , m_pRubyText   ( bIsEnd ? nullptr : new uno::Any )
    , m_pRubyStyle  ( bIsEnd ? nullptr : new uno::Any )
    , m_pRubyAdjust ( bIsEnd ? nullptr : new uno::Any )
    , m_pRubyIsAbove( bIsEnd ? nullptr : new uno::Any )
    , m_pRubyPosition( bIsEnd ? nullptr : new uno::Any )
    , m_aDepends(*this)
    , m_pFrameFormat(nullptr)
    , m_ePortionType( bIsEnd ? PORTION_RUBY_END : PORTION_RUBY_START )
    , m_bIsCollapsed(false)
{
    init( pPortionCursor);

    if (!bIsEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        rItem.QueryValue(*m_pRubyText);
        rItem.QueryValue(*m_pRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*m_pRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*m_pRubyIsAbove, MID_RUBY_ABOVE);
        rItem.QueryValue(*m_pRubyPosition, MID_RUBY_POSITION);
    }
}

SwXTextPortion::~SwXTextPortion()
{
    SolarMutexGuard aGuard;
    m_pUnoCursor.reset(nullptr);
    m_aDepends.EndListeningAll();
}

uno::Reference< text::XText >  SwXTextPortion::getText()
{
    return m_xParentText;
}

uno::Reference< text::XTextRange >  SwXTextPortion::getStart()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCursor& rUnoCursor = GetCursor();

    SwPaM aPam(*rUnoCursor.Start());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextPortion::getEnd()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCursor& rUnoCursor = GetCursor();

    SwPaM aPam(*rUnoCursor.End());
    uno::Reference< text::XText > xParent = getText();
    xRet = new SwXTextRange(aPam, xParent);
    return xRet;
}

OUString SwXTextPortion::getString()
{
    SolarMutexGuard aGuard;
    OUString aText;
    SwUnoCursor& rUnoCursor = GetCursor();

    // TextPortions are always within a paragraph
    SwTextNode* pTextNd = rUnoCursor.GetNode().GetTextNode();
    if ( pTextNd )
    {
        const sal_Int32 nStt = rUnoCursor.Start()->nContent.GetIndex();
        aText = pTextNd->GetExpandText(nullptr, nStt,
                rUnoCursor.End()->nContent.GetIndex() - nStt );
    }
    return aText;
}

void SwXTextPortion::setString(const OUString& aString)
{
    SolarMutexGuard aGuard;
    SwUnoCursor& rUnoCursor = GetCursor();

    SwUnoCursorHelper::SetString(rUnoCursor, aString);
}

uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    //! PropertySetInfo for text portion extensions
    static uno::Reference< beans::XPropertySetInfo >
            xTextPorExtRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS)->getPropertySetInfo();
    //! PropertySetInfo for redline portions
    static uno::Reference< beans::XPropertySetInfo >
            xRedlPorRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_REDLINE_PORTION)->getPropertySetInfo();

    return (PORTION_REDLINE_START == m_ePortionType ||
            PORTION_REDLINE_END   == m_ePortionType) ? xRedlPorRef : xTextPorExtRef;
}

void SwXTextPortion::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    SwUnoCursor& rUnoCursor = GetCursor();

    SwUnoCursorHelper::SetPropertyValue(rUnoCursor, *m_pPropSet,
            rPropertyName, aValue);
}

void SwXTextPortion::GetPropertyValue(
        uno::Any &rVal,
        const SfxItemPropertySimpleEntry& rEntry,
        SwUnoCursor *pUnoCursor,
        std::unique_ptr<SfxItemSet> &pSet )
{
    OSL_ENSURE( pUnoCursor, "UNO cursor missing" );
    if (!pUnoCursor)
        return;
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
                pRet = nullptr;
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
                    rVal <<= m_bIsCollapsed;
                break;
                default:
                break;
            }
        }
        break;
        case FN_UNO_IS_START:
        {
            bool bStart = true, bPut = true;
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
                    bStart = false;
                break;
                default:
                    bPut = false;
            }
            if(bPut)
                rVal <<= bStart;
        }
        break;
        case RES_TXTATR_CJK_RUBY:
        {
            const uno::Any* pToSet = nullptr;
            switch(rEntry.nMemberId)
            {
                case MID_RUBY_TEXT :    pToSet = m_pRubyText.get();   break;
                case MID_RUBY_ADJUST :  pToSet = m_pRubyAdjust.get(); break;
                case MID_RUBY_CHARSTYLE:pToSet = m_pRubyStyle.get();  break;
                case MID_RUBY_ABOVE :   pToSet = m_pRubyIsAbove.get();break;
                case MID_RUBY_POSITION: pToSet = m_pRubyPosition.get();break;
            }
            if(pToSet)
                rVal = *pToSet;
        }
        break;
        default:
            beans::PropertyState eTemp;
            bool bDone = SwUnoCursorHelper::getCursorPropertyValue(
                                rEntry, *pUnoCursor, &rVal, eTemp );
            if(!bDone)
            {
                if(!pSet)
                {
                    pSet = std::make_unique<SfxItemSet>(
                        pUnoCursor->GetDoc()->GetAttrPool(),
                        svl::Items<
                            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                            RES_UNKNOWNATR_CONTAINER,
                                RES_UNKNOWNATR_CONTAINER>{});
                    SwUnoCursorHelper::GetCursorAttr(*pUnoCursor, *pSet);
                }
                m_pPropSet->getPropertyValue(rEntry, *pSet, rVal);
            }
    }
}

uno::Sequence< uno::Any > SwXTextPortion::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
{
    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    uno::Any *pValues = aValues.getArray();
    SwUnoCursor& rUnoCursor = GetCursor();

    {
        std::unique_ptr<SfxItemSet> pSet;
        // get starting point for the look-up, either the provided one or else
        // from the beginning of the map
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if(!pEntry)
                throw beans::UnknownPropertyException( "Unknown property: " + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            GetPropertyValue( pValues[nProp], *pEntry, &rUnoCursor, pSet );
        }
    }
    return aValues;
}

uno::Any SwXTextPortion::getPropertyValue(
    const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aPropertyNames { rPropertyName };
    return GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}

void SwXTextPortion::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
{
    SwUnoCursor& rUnoCursor = GetCursor();

    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const uno::Any* pValues = rValues.getConstArray();
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        uno::Sequence< beans::PropertyValue > aValues( rPropertyNames.getLength() );
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if (!pEntry)
                throw beans::UnknownPropertyException( "Unknown property: " + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ("Property is read-only: " + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            aValues[nProp].Name = pPropertyNames[nProp];
            aValues[nProp].Value = pValues[nProp];
        }
        SwUnoCursorHelper::SetPropertyValues( rUnoCursor, *m_pPropSet, aValues );
    }
}

void SwXTextPortion::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
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
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("Unknown property exception caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }
    catch (lang::WrappedTargetException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("WrappedTargetException caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }

    return aValues;
}

/* disabled for #i46921# */
uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL SwXTextPortion::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
{
    SolarMutexGuard aGuard;

    if (rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();
    SwUnoCursor& rUnoCursor = GetCursor();

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
                                rUnoCursor, *m_pPropSet, pProp[i], pValue[i] );
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
{
    SolarMutexGuard aGuard;

    uno::Sequence< beans::GetDirectPropertyTolerantResult > aTmpRes(
            GetPropertyValuesTolerant_Impl( rPropertyNames, false ) );
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
{
    SolarMutexGuard aGuard;
    return GetPropertyValuesTolerant_Impl( rPropertyNames, true );
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SwXTextPortion::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        bool bDirectValuesOnly )
{
    SolarMutexGuard aGuard;

    SwUnoCursor& rUnoCursor = GetCursor();

    std::vector< beans::GetDirectPropertyTolerantResult > aResultVector;

    try
    {
        sal_Int32 nProps = rPropertyNames.getLength();
        const OUString *pProp = rPropertyNames.getConstArray();

        std::unique_ptr<SfxItemSet> pSet;

        const SfxItemPropertyMap& rPropMap = m_pPropSet->getPropertyMap();


        uno::Sequence< beans::PropertyState > aPropertyStates =
            SwUnoCursorHelper::GetPropertyStates(
                rUnoCursor, *m_pPropSet,
                rPropertyNames,
                SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT );
        const beans::PropertyState* pPropertyStates = aPropertyStates.getConstArray();

        for (sal_Int32 i = 0;  i < nProps;  ++i)
        {
            beans::GetDirectPropertyTolerantResult aResult;
            try
            {
                aResult.Name = pProp[i];
                if(pPropertyStates[i] == beans::PropertyState::PropertyState_MAKE_FIXED_SIZE)     // property unknown?
                {
                    if( bDirectValuesOnly )
                        continue;
                    else
                        aResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
                }
                else
                {
                    const SfxItemPropertySimpleEntry* pEntry = rPropMap.getByName( pProp[i] );
                    if (!pEntry)
                        throw beans::UnknownPropertyException( "Unknown property: " + pProp[i], static_cast < cppu::OWeakObject * > ( this ) );
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
                        GetPropertyValue( aResult.Value, *pEntry, &rUnoCursor, pSet );
                        aResult.Result = beans::TolerantPropertySetResultType::SUCCESS;
                        aResultVector.push_back( aResult );
                    }
                }
            }
            catch (const beans::UnknownPropertyException &)
            {
                // should not occur because property was searched for before
                OSL_FAIL( "unexpected exception caught" );
                aResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            }
            catch (const lang::IllegalArgumentException &)
            {
                aResult.Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
            }
            catch (const beans::PropertyVetoException &)
            {
                aResult.Result = beans::TolerantPropertySetResultType::PROPERTY_VETO;
            }
            catch (const lang::WrappedTargetException &)
            {
                aResult.Result = beans::TolerantPropertySetResultType::WRAPPED_TARGET;
            }
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }

    return comphelper::containerToSequence(aResultVector);
}

void SwXTextPortion::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{}

void SwXTextPortion::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{}

void SwXTextPortion::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{}

void SwXTextPortion::addPropertyChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextPortion::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXTextPortion::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCursor& rUnoCursor = GetCursor();

    if (GetTextPortionType() == PORTION_RUBY_START &&
        rPropertyName.startsWith("Ruby"))
    {
        eRet = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        eRet = SwUnoCursorHelper::GetPropertyState(rUnoCursor, *m_pPropSet,
                rPropertyName);
    }
    return eRet;
}

uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
{
    SolarMutexGuard aGuard;
    SwUnoCursor& rUnoCursor = GetCursor();

    uno::Sequence< beans::PropertyState > aRet =
        SwUnoCursorHelper::GetPropertyStates(rUnoCursor, *m_pPropSet,
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
{
    SolarMutexGuard aGuard;
    SwUnoCursor& rUnoCursor = GetCursor();

    SwUnoCursorHelper::SetPropertyToDefault(
            rUnoCursor, *m_pPropSet, rPropertyName);
}

uno::Any SwXTextPortion::getPropertyDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwUnoCursor& rUnoCursor = GetCursor();

    aRet = SwUnoCursorHelper::GetPropertyDefault(rUnoCursor, *m_pPropSet,
                rPropertyName);
    return aRet;
}

uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& /*aServiceName*/)
{
    SolarMutexGuard aGuard;
    SwUnoCursor& rUnoCursor = GetCursor();

    return SwXParaFrameEnumeration::Create(rUnoCursor, PARAFRAME_PORTION_CHAR, m_pFrameFormat);
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
{
    return { "com.sun.star.text.TextContent" };
}

OUString SwXTextPortion::getImplementationName()
{
    return { "SwXTextPortion" };
}

sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames()
{
    return { "com.sun.star.text.TextPortion",
            "com.sun.star.style.CharacterProperties",
            "com.sun.star.style.CharacterPropertiesAsian",
            "com.sun.star.style.CharacterPropertiesComplex",
            "com.sun.star.style.ParagraphProperties",
            "com.sun.star.style.ParagraphPropertiesAsian",
            "com.sun.star.style.ParagraphPropertiesComplex" };
}

void SwXTextPortion::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (auto pLegacyHint = dynamic_cast<const sw::LegacyModifyHint*>(&rHint))
    {
        ClientModify(this, pLegacyHint->m_pOld, pLegacyHint->m_pNew);
        if(!m_aDepends.IsListeningTo(m_pFrameFormat))
            m_pFrameFormat = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
