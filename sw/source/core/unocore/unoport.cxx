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
#include <utility>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <unocrsrhelper.hxx>
#include <unoparaframeenum.hxx>
#include <unotextrange.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <txtatr.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

void SwXTextPortion::init(const SwUnoCursor* pPortionCursor)
{
    m_pUnoCursor = pPortionCursor->GetDoc().CreateUnoCursor(*pPortionCursor->GetPoint());
    if (pPortionCursor->HasMark())
    {
        m_pUnoCursor->SetMark();
        *m_pUnoCursor->GetMark() = *pPortionCursor->GetMark();
    }
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
        uno::Reference< SwXText > xParent,
        SwTextPortionType eType)
    : m_pPropSet(aSwMapProvider.GetPropertySet(
        (PORTION_REDLINE_START == eType ||
         PORTION_REDLINE_END   == eType)
            ?  PROPERTY_MAP_REDLINE_PORTION
            :  PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(std::move(xParent))
    , m_pFrameFormat(nullptr)
    , m_ePortionType(eType != PORTION_LIST_AUTOFMT ? eType : PORTION_TEXT)
    , m_bIsCollapsed(false)
    , m_bIsListAutoFormat(false)
{
    if (eType == PORTION_LIST_AUTOFMT)
    {
        m_bIsListAutoFormat = true;
    }
    init( pPortionCursor);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
    uno::Reference< SwXText > xParent,
    SwFrameFormat& rFormat )
    : m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(std::move(xParent))
    , m_pFrameFormat(&rFormat)
    , m_ePortionType(PORTION_FRAME)
    , m_bIsCollapsed(false)
    , m_bIsListAutoFormat(false)
{
    StartListening(rFormat.GetNotifier());
    init( pPortionCursor);
}

SwXTextPortion::SwXTextPortion(
    const SwUnoCursor* pPortionCursor,
    SwTextRuby const& rAttr,
    uno::Reference< SwXText >  xParent,
    bool bIsEnd )
    : m_pPropSet(aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS))
    , m_xParentText(std::move(xParent))
    , m_pFrameFormat(nullptr)
    , m_ePortionType( bIsEnd ? PORTION_RUBY_END : PORTION_RUBY_START )
    , m_bIsCollapsed(false)
    , m_bIsListAutoFormat(false)
{
    if (!bIsEnd)
    {
        m_oRubyText.emplace();
        m_oRubyStyle.emplace();
        m_oRubyAdjust.emplace();
        m_oRubyIsAbove.emplace();
        m_oRubyPosition.emplace();
    }
    init( pPortionCursor);

    if (!bIsEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        rItem.QueryValue(*m_oRubyText);
        rItem.QueryValue(*m_oRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*m_oRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*m_oRubyIsAbove, MID_RUBY_ABOVE);
        rItem.QueryValue(*m_oRubyPosition, MID_RUBY_POSITION);
    }
}

SwXTextPortion::~SwXTextPortion()
{
    SolarMutexGuard aGuard;
    m_pUnoCursor.reset(nullptr);
    EndListeningAll();
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
    SwTextNode* pTextNd = rUnoCursor.GetPointNode().GetTextNode();
    if ( pTextNd )
    {
        const sal_Int32 nStt = rUnoCursor.Start()->GetContentIndex();
        aText = pTextNd->GetExpandText(nullptr, nStt,
                rUnoCursor.End()->GetContentIndex() - nStt,
                false, false, false, ExpandMode::ExpandFootnote);
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
        const SfxItemPropertyMapEntry& rEntry,
        SwUnoCursor *pUnoCursor,
        std::unique_ptr<SfxItemSet> &pSet )
{
    static constexpr OUStringLiteral TEXT = u"Text";
    static constexpr OUStringLiteral TEXTFIELD = u"TextField";
    static constexpr OUStringLiteral FRAME = u"Frame";
    static constexpr OUStringLiteral FOOTNOTE = u"Footnote";
    static constexpr OUStringLiteral REDLINE = u"Redline";
    static constexpr OUStringLiteral RUBY = u"Ruby";
    static constexpr OUStringLiteral SOFTPAGEBREAK = u"SoftPageBreak";
    static constexpr OUStringLiteral TEXTFIELDSTART = u"TextFieldStart";
    static constexpr OUStringLiteral TEXTFIELDSEPARATOR = u"TextFieldSeparator";
    static constexpr OUStringLiteral TEXTFIELDEND = u"TextFieldEnd";
    static constexpr OUStringLiteral TEXTFIELDSTARTEND = u"TextFieldStartEnd";
    static constexpr OUStringLiteral ANNOTATION = u"Annotation";
    static constexpr OUStringLiteral ANNOTATIONEND = u"AnnotationEnd";
    static constexpr OUStringLiteral LINEBREAK = u"LineBreak";

    OSL_ENSURE( pUnoCursor, "UNO cursor missing" );
    if (!pUnoCursor)
        return;
    switch(rEntry.nWID)
    {
        case FN_UNO_TEXT_PORTION_TYPE:
        {
            OUString sRet;
            switch (m_ePortionType)
            {
            case PORTION_TEXT:           sRet = TEXT; break;
            case PORTION_FIELD:          sRet = TEXTFIELD; break;
            case PORTION_FRAME:          sRet = FRAME; break;
            case PORTION_FOOTNOTE:       sRet = FOOTNOTE; break;
            case PORTION_REFMARK_START:
            case PORTION_REFMARK_END:    sRet = UNO_NAME_REFERENCE_MARK; break;
            case PORTION_TOXMARK_START:
            case PORTION_TOXMARK_END:    sRet = UNO_NAME_DOCUMENT_INDEX_MARK; break;
            case PORTION_BOOKMARK_START:
            case PORTION_BOOKMARK_END :  sRet = UNO_NAME_BOOKMARK; break;
            case PORTION_REDLINE_START:
            case PORTION_REDLINE_END:    sRet = REDLINE; break;
            case PORTION_RUBY_START:
            case PORTION_RUBY_END:       sRet = RUBY; break;
            case PORTION_SOFT_PAGEBREAK: sRet = SOFTPAGEBREAK; break;
            case PORTION_META:           sRet = UNO_NAME_META; break;
            case PORTION_FIELD_START:    sRet = TEXTFIELDSTART; break;
            case PORTION_FIELD_SEP:      sRet = TEXTFIELDSEPARATOR; break;
            case PORTION_FIELD_END:      sRet = TEXTFIELDEND; break;
            case PORTION_FIELD_START_END:sRet = TEXTFIELDSTARTEND; break;
            case PORTION_ANNOTATION:     sRet = ANNOTATION; break;
            case PORTION_ANNOTATION_END: sRet = ANNOTATIONEND; break;
            case PORTION_LINEBREAK:      sRet = LINEBREAK; break;
            case PORTION_CONTENT_CONTROL:sRet = UNO_NAME_CONTENT_CONTROL; break;
            default: break;
            }

            rVal <<= sRet;
        }
        break;
        case FN_UNO_CONTROL_CHARACTER: // obsolete!
        break;
        case FN_UNO_DOCUMENT_INDEX_MARK:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xTOXMark);
        break;
        case FN_UNO_REFERENCE_MARK:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xRefMark);
        break;
        case FN_UNO_BOOKMARK:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xBookmark);
        break;
        case FN_UNO_FOOTNOTE:
            rVal <<= uno::Reference<css::text::XFootnote>(m_xFootnote);
        break;
        case FN_UNO_TEXT_FIELD:
            rVal <<= m_xTextField;
        break;
        case FN_UNO_META:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xMeta);
        break;
        case FN_UNO_LINEBREAK:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xLineBreak);
            break;
        case FN_UNO_CONTENT_CONTROL:
            rVal <<= uno::Reference<css::text::XTextContent>(m_xContentControl);
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
                case PORTION_FIELD_SEP:
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
                case PORTION_FIELD_SEP:
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
            const std::optional<uno::Any>* pToSet = nullptr;
            switch(rEntry.nMemberId)
            {
                case MID_RUBY_TEXT :    pToSet = &m_oRubyText;   break;
                case MID_RUBY_ADJUST :  pToSet = &m_oRubyAdjust; break;
                case MID_RUBY_CHARSTYLE:pToSet = &m_oRubyStyle;  break;
                case MID_RUBY_ABOVE :   pToSet = &m_oRubyIsAbove;break;
                case MID_RUBY_POSITION: pToSet = &m_oRubyPosition;break;
            }
            if(pToSet && *pToSet)
                rVal = **pToSet;
        }
        break;
        default:
            beans::PropertyState eTemp;
            bool bDone = false;
            if (m_bIsListAutoFormat)
            {
                SwTextNode* pTextNode = pUnoCursor->GetPointNode().GetTextNode();
                std::shared_ptr<SfxItemSet> pListSet
                    = pTextNode->GetAttr(RES_PARATR_LIST_AUTOFMT).GetStyleHandle();
                if (pListSet)
                {
                    SfxItemPropertySet::getPropertyValue(rEntry, *pListSet, rVal);
                    bDone = true;
                }
            }
            if (!bDone)
            {
                bDone = SwUnoCursorHelper::getCursorPropertyValue(
                                    rEntry, *pUnoCursor, &rVal, eTemp );
            }
            if(!bDone)
            {
                if(!pSet)
                {
                    pSet = std::make_unique<SfxItemSetFixed<
                            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                            RES_UNKNOWNATR_CONTAINER,
                                RES_UNKNOWNATR_CONTAINER>>(pUnoCursor->GetDoc().GetAttrPool());
                    SwUnoCursorHelper::GetCursorAttr(*pUnoCursor, *pSet);
                }
                SfxItemPropertySet::getPropertyValue(rEntry, *pSet, rVal);
            }
    }
}

uno::Sequence< uno::Any > SwXTextPortion::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
{
    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aValues(nLength);
    uno::Any *pValues = aValues.getArray();
    SwUnoCursor& rUnoCursor = GetCursor();

    {
        std::unique_ptr<SfxItemSet> pSet;
        // get starting point for the look-up, either the provided one or else
        // from the beginning of the map
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            const SfxItemPropertyMapEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if(!pEntry)
                throw beans::UnknownPropertyException( "Unknown property: " + pPropertyNames[nProp], getXWeak() );
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
    if (rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException(u"lengths do not match"_ustr,
                                             getXWeak(), -1);

    SwUnoCursor& rUnoCursor = GetCursor();

    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const uno::Any* pValues = rValues.getConstArray();
        const SfxItemPropertyMap& rMap = m_pPropSet->getPropertyMap();
        uno::Sequence< beans::PropertyValue > aValues( rPropertyNames.getLength() );
        auto aValuesRange = asNonConstRange(aValues);
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            const SfxItemPropertyMapEntry* pEntry = rMap.getByName(pPropertyNames[nProp]);
            if (!pEntry)
                throw beans::UnknownPropertyException( "Unknown property: " + pPropertyNames[nProp], getXWeak() );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ("Property is read-only: " + pPropertyNames[nProp], getXWeak() );

            aValuesRange[nProp].Name = pPropertyNames[nProp];
            aValuesRange[nProp].Value = pValues[nProp];
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
        throw lang::WrappedTargetRuntimeException(u"Unknown property exception caught"_ustr,
                getXWeak(), anyEx );
    }
    catch (lang::WrappedTargetException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(u"WrappedTargetException caught"_ustr,
                getXWeak(), anyEx );
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

            const SfxItemPropertyMapEntry* pEntry = rPropMap.getByName( pProp[i] );
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
            TOOLS_WARN_EXCEPTION( "sw", "" );
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

    const uno::Sequence< beans::GetDirectPropertyTolerantResult > aTmpRes(
            GetPropertyValuesTolerant_Impl( rPropertyNames, false ) );

    // copy temporary result to final result type
    sal_Int32 nLen = aTmpRes.getLength();
    uno::Sequence< beans::GetPropertyTolerantResult > aRes( nLen );
    std::copy(aTmpRes.begin(), aTmpRes.end(), aRes.getArray());
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


        uno::Sequence< beans::PropertyState > aPropertyStates;
        if (m_bIsListAutoFormat)
        {
            SwTextNode* pTextNode = rUnoCursor.GetPointNode().GetTextNode();
            std::shared_ptr<SfxItemSet> pListSet
                = pTextNode->GetAttr(RES_PARATR_LIST_AUTOFMT).GetStyleHandle();
            if (pListSet)
            {
                std::vector<beans::PropertyState> aStates;
                for (const auto& rPropertyName : rPropertyNames)
                {
                    aStates.push_back(m_pPropSet->getPropertyState(rPropertyName, *pListSet));
                }
                aPropertyStates = comphelper::containerToSequence(aStates);
            }
        }
        if (!aPropertyStates.hasElements())
        {
            aPropertyStates =
                SwUnoCursorHelper::GetPropertyStates(
                    rUnoCursor, *m_pPropSet,
                    rPropertyNames,
                    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT );
        }
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
                    const SfxItemPropertyMapEntry* pEntry = rPropMap.getByName( pProp[i] );
                    if (!pEntry)
                        throw beans::UnknownPropertyException( "Unknown property: " + pProp[i], getXWeak() );
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
                TOOLS_WARN_EXCEPTION( "sw", "unexpected exception caught" );
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

uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames()
{
    return { u"com.sun.star.text.TextContent"_ustr };
}

OUString SwXTextPortion::getImplementationName()
{
    return { u"SwXTextPortion"_ustr };
}

sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames()
{
    return { u"com.sun.star.text.TextPortion"_ustr,
            u"com.sun.star.style.CharacterProperties"_ustr,
            u"com.sun.star.style.CharacterPropertiesAsian"_ustr,
            u"com.sun.star.style.CharacterPropertiesComplex"_ustr,
            u"com.sun.star.style.ParagraphProperties"_ustr,
            u"com.sun.star.style.ParagraphPropertiesAsian"_ustr,
            u"com.sun.star.style.ParagraphPropertiesComplex"_ustr };
}

void SwXTextPortion::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        m_pFrameFormat = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
