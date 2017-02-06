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

#include <svx/svxids.hrc>
#include <editeng/memberids.hrc>
#include <swtypes.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include "poolfmt.hrc"
#include "poolfmt.hxx"
#include <fmtcol.hxx>
#include <unomap.hxx>
#include <unostyle.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <pagedesc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <charfmt.hxx>
#include <lineinfo.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <docstyle.hxx>
#include <fmtclds.hxx>
#include <editeng/brushitem.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/XFootnotesSettingsSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <o3tl/any.hxx>
#include <vcl/font.hxx>
#include <editeng/flstitem.hxx>
#include <vcl/metric.hxx>
#include <svtools/ctrltool.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <editeng/unofdesc.hxx>
#include <fmtornt.hxx>
#include <SwStyleNameMapper.hxx>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <numrule.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertyvalue.hxx>
#include <paratr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;

// Constants for the css::text::ColumnSeparatorStyle
#define API_COL_LINE_NONE               0
#define API_COL_LINE_SOLID              1
#define API_COL_LINE_DOTTED             2
#define API_COL_LINE_DASHED             3

#define WID_PREFIX                      0
#define WID_SUFFIX                      1
#define WID_NUMBERING_TYPE              2
#define WID_START_AT                    3
#define WID_FOOTNOTE_COUNTING           4
#define WID_PARAGRAPH_STYLE             5
#define WID_PAGE_STYLE                  6
#define WID_CHARACTER_STYLE             7
#define WID_POSITION_END_OF_DOC         8
#define WID_END_NOTICE                  9
#define WID_BEGIN_NOTICE                10
#define WID_ANCHOR_CHARACTER_STYLE      11

const SfxItemPropertySet* GetFootnoteSet()
{
    static const SfxItemPropertyMapEntry aFootnoteMap_Impl[] =
    {
        { OUString(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_BEGIN_NOTICE),          WID_BEGIN_NOTICE,       ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_CHAR_STYLE_NAME),       WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_END_NOTICE),            WID_END_NOTICE ,        ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_FOOTNOTE_COUNTING),     WID_FOOTNOTE_COUNTING,  ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_NUMBERING_TYPE),        WID_NUMBERING_TYPE,     ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { OUString(UNO_NAME_PAGE_STYLE_NAME),       WID_PAGE_STYLE,         ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_PARA_STYLE_NAME),       WID_PARAGRAPH_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_POSITION_END_OF_DOC),   WID_POSITION_END_OF_DOC,cppu::UnoType<bool>::get(), PROPERTY_NONE,         0},
        { OUString(UNO_NAME_PREFIX),                WID_PREFIX,             ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_START_AT),              WID_START_AT ,          ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { OUString(UNO_NAME_SUFFIX),                WID_SUFFIX,             ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aFootnoteSet_Impl(aFootnoteMap_Impl);
    return &aFootnoteSet_Impl;
}

const SfxItemPropertySet* GetEndnoteSet()
{
    static const SfxItemPropertyMapEntry aEndnoteMap_Impl[] =
    {
        { OUString(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_CHAR_STYLE_NAME),       WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_NUMBERING_TYPE),        WID_NUMBERING_TYPE,     ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { OUString(UNO_NAME_PAGE_STYLE_NAME),       WID_PAGE_STYLE,         ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_PARA_STYLE_NAME),       WID_PARAGRAPH_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_PREFIX),                WID_PREFIX,     ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_START_AT),              WID_START_AT ,          ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { OUString(UNO_NAME_SUFFIX),                WID_SUFFIX,     ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aEndnoteSet_Impl(aEndnoteMap_Impl);
    return &aEndnoteSet_Impl;
}

const SfxItemPropertySet* GetNumberingRulesSet()
{
    static const SfxItemPropertyMapEntry aNumberingRulesMap_Impl[] =
    {
        { OUString(UNO_NAME_IS_ABSOLUTE_MARGINS),       WID_IS_ABS_MARGINS, cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { OUString(UNO_NAME_IS_AUTOMATIC),              WID_IS_AUTOMATIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { OUString(UNO_NAME_IS_CONTINUOUS_NUMBERING),   WID_CONTINUOUS,     cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { OUString(UNO_NAME_NAME),                      WID_RULE_NAME   ,   ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,     0},
        { OUString(UNO_NAME_NUMBERING_IS_OUTLINE),      WID_IS_OUTLINE, cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { OUString(UNO_NAME_DEFAULT_LIST_ID),           WID_DEFAULT_LIST_ID, ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet  aNumberingRulesSet_Impl( aNumberingRulesMap_Impl );
    return &aNumberingRulesSet_Impl;
}

#define WID_NUM_ON                      0
#define WID_SEPARATOR_INTERVAL          1
#define WID_NUMBERING_TYPE              2
#define WID_NUMBER_POSITION             3
#define WID_DISTANCE                    4
#define WID_INTERVAL                    5
#define WID_SEPARATOR_TEXT              6
#define WID_COUNT_EMPTY_LINES           8
#define WID_COUNT_LINES_IN_FRAMES       9
#define WID_RESTART_AT_EACH_PAGE        10

const SfxItemPropertySet* GetLineNumberingSet()
{
    static const SfxItemPropertyMapEntry aLineNumberingMap_Impl[] =
    {
        { OUString(UNO_NAME_CHAR_STYLE_NAME),         WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_COUNT_EMPTY_LINES),       WID_COUNT_EMPTY_LINES , cppu::UnoType<bool>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_COUNT_LINES_IN_FRAMES),   WID_COUNT_LINES_IN_FRAMES, cppu::UnoType<bool>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_DISTANCE),                WID_DISTANCE       ,    ::cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_IS_ON),                   WID_NUM_ON,             cppu::UnoType<bool>::get()  ,          PROPERTY_NONE,     0},
        { OUString(UNO_NAME_INTERVAL),                WID_INTERVAL  ,       ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_SEPARATOR_TEXT),          WID_SEPARATOR_TEXT,   ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { OUString(UNO_NAME_NUMBER_POSITION),         WID_NUMBER_POSITION,    ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_NUMBERING_TYPE),          WID_NUMBERING_TYPE ,    ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { OUString(UNO_NAME_RESTART_AT_EACH_PAGE),    WID_RESTART_AT_EACH_PAGE, cppu::UnoType<bool>::get()  ,          PROPERTY_NONE,     0},
        { OUString(UNO_NAME_SEPARATOR_INTERVAL),      WID_SEPARATOR_INTERVAL, ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aLineNumberingSet_Impl(aLineNumberingMap_Impl);
    return &aLineNumberingSet_Impl;
}

static SwCharFormat* lcl_getCharFormat(SwDoc* pDoc, const uno::Any& aValue)
{
    SwCharFormat* pRet = nullptr;
    OUString uTmp;
    aValue >>= uTmp;
    OUString sCharFormat;
    SwStyleNameMapper::FillUIName(uTmp, sCharFormat, SwGetPoolIdFromName::ChrFmt, true);
    if (sCharFormat != SW_RESSTR(STR_POOLCOLL_STANDARD))
    {
        pRet = pDoc->FindCharFormatByName( sCharFormat );
    }
    if(!pRet)
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sCharFormat, SwGetPoolIdFromName::ChrFmt);
        if(USHRT_MAX != nId)
            pRet = pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );
    }
    return pRet;
}

static SwTextFormatColl* lcl_GetParaStyle(SwDoc* pDoc, const uno::Any& aValue)
{
    OUString uTmp;
    aValue >>= uTmp;
    OUString sParaStyle;
    SwStyleNameMapper::FillUIName(uTmp, sParaStyle, SwGetPoolIdFromName::TxtColl, true );
    SwTextFormatColl* pRet = pDoc->FindTextFormatCollByName( sParaStyle );
    if( !pRet  )
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sParaStyle, SwGetPoolIdFromName::TxtColl );
        if( USHRT_MAX != nId  )
            pRet = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nId );
    }
    return pRet;
}

static SwPageDesc* lcl_GetPageDesc(SwDoc* pDoc, const uno::Any& aValue)
{
    OUString uTmp;
    aValue >>= uTmp;
    OUString sPageDesc;
    SwStyleNameMapper::FillUIName(uTmp, sPageDesc, SwGetPoolIdFromName::PageDesc, true );
    SwPageDesc* pRet = pDoc->FindPageDesc( sPageDesc );
    if(!pRet)
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sPageDesc, SwGetPoolIdFromName::PageDesc);
        if(USHRT_MAX != nId)
            pRet = pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( nId );
    }
    return pRet;
}

// Numbering
const unsigned short aSvxToUnoAdjust[] =
{
    text::HoriOrientation::LEFT,   //3
    text::HoriOrientation::RIGHT,  //1
    USHRT_MAX,
    text::HoriOrientation::CENTER, //2
    USHRT_MAX,
    USHRT_MAX
};

const unsigned short aUnoToSvxAdjust[] =
{
    USHRT_MAX,
    SVX_ADJUST_RIGHT,       // 1
    SVX_ADJUST_CENTER,      // 3
    SVX_ADJUST_LEFT,        // 0
    USHRT_MAX,
    USHRT_MAX
};

OUString SwXFootnoteProperties::getImplementationName()
{
    return OUString("SwXFootnoteProperties");
}

sal_Bool SwXFootnoteProperties::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXFootnoteProperties::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.FootnoteSettings" };
    return aRet;
}

SwXFootnoteProperties::SwXFootnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetFootnoteSet())
{
}

SwXFootnoteProperties::~SwXFootnoteProperties()
{

}

uno::Reference< beans::XPropertySetInfo >  SwXFootnoteProperties::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
    return aRef;
}

void SwXFootnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwFootnoteInfo aFootnoteInfo(pDoc->GetFootnoteInfo());
            switch(pEntry->nWID)
            {
                case WID_PREFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFootnoteInfo.SetPrefix(uTmp);
                }
                break;
                case WID_SUFFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFootnoteInfo.SetSuffix(uTmp);
                }
                break;
                case WID_NUMBERING_TYPE:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if(nTmp >= 0 &&
                        (nTmp <= SVX_NUM_ARABIC ||
                            nTmp > SVX_NUM_BITMAP))
                        aFootnoteInfo.aFormat.SetNumberingType(nTmp);
                    else
                        throw lang::IllegalArgumentException();
                }
                break;
                case WID_START_AT:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aFootnoteInfo.nFootnoteOffset = nTmp;
                }
                break;
                case WID_FOOTNOTE_COUNTING:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    switch(nTmp)
                    {
                        case  FootnoteNumbering::PER_PAGE:
                            aFootnoteInfo.eNum = FTNNUM_PAGE;
                        break;
                        case  FootnoteNumbering::PER_CHAPTER:
                            aFootnoteInfo.eNum = FTNNUM_CHAPTER;
                        break;
                        case  FootnoteNumbering::PER_DOCUMENT:
                            aFootnoteInfo.eNum = FTNNUM_DOC;
                        break;
                    }
                }
                break;
                case WID_PARAGRAPH_STYLE:
                {
                    SwTextFormatColl* pColl = lcl_GetParaStyle(pDoc, aValue);
                    if(pColl)
                        aFootnoteInfo.SetFootnoteTextColl(*pColl);
                }
                break;
                case WID_PAGE_STYLE:
                {
                    SwPageDesc* pDesc = lcl_GetPageDesc(pDoc, aValue);
                    if(pDesc)
                        aFootnoteInfo.ChgPageDesc( pDesc );
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    SwCharFormat* pFormat = lcl_getCharFormat(pDoc, aValue);
                    if(pFormat)
                    {
                        if(pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE)
                            aFootnoteInfo.SetAnchorCharFormat(pFormat);
                        else
                            aFootnoteInfo.SetCharFormat(pFormat);
                    }
                }
                break;
                case WID_POSITION_END_OF_DOC:
                {
                    bool bVal = *o3tl::doAccess<bool>(aValue);
                    aFootnoteInfo.ePos = bVal ? FTNPOS_CHAPTER : FTNPOS_PAGE;
                }
                break;
                case WID_END_NOTICE:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFootnoteInfo.aQuoVadis = uTmp;
                }
                break;
                case WID_BEGIN_NOTICE:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFootnoteInfo.aErgoSum = uTmp;
                }
                break;
            }
            pDoc->SetFootnoteInfo(aFootnoteInfo);
        }
        else
            throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXFootnoteProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            const SwFootnoteInfo& rFootnoteInfo = pDoc->GetFootnoteInfo();
            switch(pEntry->nWID)
            {
                case WID_PREFIX:
                {
                    aRet <<= rFootnoteInfo.GetPrefix();
                }
                break;
                case WID_SUFFIX:
                {
                    aRet <<= rFootnoteInfo.GetSuffix();
                }
                break;
                case  WID_NUMBERING_TYPE :
                {
                    aRet <<= rFootnoteInfo.aFormat.GetNumberingType();
                }
                break;
                case  WID_START_AT:
                    aRet <<= (sal_Int16)rFootnoteInfo.nFootnoteOffset;
                break;
                case  WID_FOOTNOTE_COUNTING  :
                {
                    sal_Int16 nRet = 0;
                    switch(rFootnoteInfo.eNum)
                    {
                        case  FTNNUM_PAGE:
                            nRet = FootnoteNumbering::PER_PAGE;
                        break;
                        case  FTNNUM_CHAPTER:
                            nRet = FootnoteNumbering::PER_CHAPTER;
                        break;
                        case  FTNNUM_DOC:
                            nRet = FootnoteNumbering::PER_DOCUMENT;
                        break;
                    }
                    aRet <<= nRet;
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTextFormatColl* pColl = rFootnoteInfo.GetFootnoteTextColl();
                    OUString aString;
                    if(pColl)
                        aString = pColl->GetName();
                    SwStyleNameMapper::FillProgName(aString, aString, SwGetPoolIdFromName::TxtColl, true);
                    aRet <<= aString;
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    OUString aString;
                    if( rFootnoteInfo.KnowsPageDesc() )
                    {
                        SwStyleNameMapper::FillProgName(
                                rFootnoteInfo.GetPageDesc( *pDoc )->GetName(),
                                aString,
                                SwGetPoolIdFromName::PageDesc,
                                true);
                    }
                    aRet <<= aString;
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    OUString aString;
                    const SwCharFormat* pCharFormat = nullptr;
                    if( pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE )
                    {
                        if( rFootnoteInfo.GetAnchorCharFormatDep()->GetRegisteredIn() )
                            pCharFormat = rFootnoteInfo.GetAnchorCharFormat(*pDoc);
                    }
                    else
                    {
                        if( rFootnoteInfo.GetCharFormatDep()->GetRegisteredIn() )
                            pCharFormat = rFootnoteInfo.GetCharFormat(*pDoc);
                    }
                    if( pCharFormat )
                    {
                        SwStyleNameMapper::FillProgName(
                                pCharFormat->GetName(),
                                aString,
                                SwGetPoolIdFromName::ChrFmt,
                                true);
                    }
                    aRet <<= aString;
                }
                break;
                case  WID_POSITION_END_OF_DOC:
                    aRet <<= FTNPOS_CHAPTER == rFootnoteInfo.ePos;
                break;
                case  WID_END_NOTICE         :
                    aRet <<= rFootnoteInfo.aQuoVadis;
                break;
                case  WID_BEGIN_NOTICE       :
                    aRet <<= rFootnoteInfo.aErgoSum;
                break;
            }
        }
        else
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXFootnoteProperties::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFootnoteProperties::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFootnoteProperties::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFootnoteProperties::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

OUString SwXEndnoteProperties::getImplementationName()
{
    return OUString("SwXEndnoteProperties");
}

sal_Bool SwXEndnoteProperties::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXEndnoteProperties::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.FootnoteSettings" };
    return aRet;
}

SwXEndnoteProperties::SwXEndnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetEndnoteSet())
{
}

SwXEndnoteProperties::~SwXEndnoteProperties()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXEndnoteProperties::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
    return aRef;
}

void SwXEndnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwEndNoteInfo aEndInfo(pDoc->GetEndNoteInfo());
            switch(pEntry->nWID)
            {
                case WID_PREFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aEndInfo.SetPrefix(uTmp);
                }
                break;
                case WID_SUFFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aEndInfo.SetSuffix(uTmp);
                }
                break;
                case  WID_NUMBERING_TYPE :
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aEndInfo.aFormat.SetNumberingType(nTmp);
                }
                break;
                case  WID_START_AT:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aEndInfo.nFootnoteOffset = nTmp;
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTextFormatColl* pColl = lcl_GetParaStyle(pDoc, aValue);
                    if(pColl)
                        aEndInfo.SetFootnoteTextColl(*pColl);
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    SwPageDesc* pDesc = lcl_GetPageDesc(pDoc, aValue);
                    if(pDesc)
                        aEndInfo.ChgPageDesc( pDesc );
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case  WID_CHARACTER_STYLE    :
                {
                    SwCharFormat* pFormat = lcl_getCharFormat(pDoc, aValue);
                    if(pFormat)
                    {
                        if(pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE)
                            aEndInfo.SetAnchorCharFormat(pFormat);
                        else
                            aEndInfo.SetCharFormat(pFormat);
                    }
                }
                break;
            }
            pDoc->SetEndNoteInfo(aEndInfo);
        }
        else
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXEndnoteProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
            switch(pEntry->nWID)
            {
                case WID_PREFIX:
                    aRet <<= rEndInfo.GetPrefix();
                break;
                case WID_SUFFIX:
                    aRet <<= rEndInfo.GetSuffix();
                break;
                case  WID_NUMBERING_TYPE :
                    aRet <<= rEndInfo.aFormat.GetNumberingType();
                break;
                case  WID_START_AT:
                    aRet <<= (sal_Int16)rEndInfo.nFootnoteOffset;
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTextFormatColl* pColl = rEndInfo.GetFootnoteTextColl();
                    OUString aString;
                    if(pColl)
                        aString = pColl->GetName();
                    SwStyleNameMapper::FillProgName(
                            aString,
                            aString,
                            SwGetPoolIdFromName::TxtColl,
                            true);
                    aRet <<= aString;

                }
                break;
                case  WID_PAGE_STYLE :
                {
                    OUString aString;
                    if( rEndInfo.KnowsPageDesc() )
                    {
                        SwStyleNameMapper::FillProgName(
                            rEndInfo.GetPageDesc( *pDoc )->GetName(),
                            aString,
                            SwGetPoolIdFromName::PageDesc,
                            true );
                    }
                    aRet <<= aString;
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    OUString aString;
                    const SwCharFormat* pCharFormat = nullptr;
                    if( pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE )
                    {
                        if( rEndInfo.GetAnchorCharFormatDep()->GetRegisteredIn() )
                            pCharFormat = rEndInfo.GetAnchorCharFormat(*pDoc);
                    }
                    else
                    {
                        if( rEndInfo.GetCharFormatDep()->GetRegisteredIn() )
                            pCharFormat = rEndInfo.GetCharFormat(*pDoc);
                    }
                    if( pCharFormat )
                    {
                        SwStyleNameMapper::FillProgName(
                                pCharFormat->GetName(),
                                aString,
                                SwGetPoolIdFromName::ChrFmt,
                                true );
                    }
                    aRet <<= aString;
                }
                break;
            }
        }
        else
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXEndnoteProperties::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXEndnoteProperties::removePropertyChangeListener(const OUString& /*PropertyName*/,
        const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXEndnoteProperties::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXEndnoteProperties::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

OUString SwXLineNumberingProperties::getImplementationName()
{
    return OUString("SwXLineNumberingProperties");
}

sal_Bool SwXLineNumberingProperties::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXLineNumberingProperties::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.LineNumberingProperties" };
    return aRet;
}

SwXLineNumberingProperties::SwXLineNumberingProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetLineNumberingSet())
{
}

SwXLineNumberingProperties::~SwXLineNumberingProperties()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXLineNumberingProperties::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
    return aRef;
}

void SwXLineNumberingProperties::setPropertyValue(
    const OUString& rPropertyName, const Any& aValue)
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwLineNumberInfo  aFontMetric(pDoc->GetLineNumberInfo());
            switch(pEntry->nWID)
            {
                case WID_NUM_ON:
                {
                    bool bVal = *o3tl::doAccess<bool>(aValue);
                    aFontMetric.SetPaintLineNumbers(bVal);
                }
                break;
                case WID_CHARACTER_STYLE :
                {
                    SwCharFormat* pFormat = lcl_getCharFormat(pDoc, aValue);
                    if(pFormat)
                        aFontMetric.SetCharFormat(pFormat);
                }
                break;
                case WID_NUMBERING_TYPE  :
                {
                    SvxNumberType aNumType(aFontMetric.GetNumType());
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aNumType.SetNumberingType(nTmp);
                    aFontMetric.SetNumType(aNumType);
                }
                break;
                case WID_NUMBER_POSITION :
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    switch(nTmp)
                    {
                        case  style::LineNumberPosition::LEFT:
                             aFontMetric.SetPos(LINENUMBER_POS_LEFT); ;
                        break;
                        case style::LineNumberPosition::RIGHT :
                             aFontMetric.SetPos(LINENUMBER_POS_RIGHT);       ;
                        break;
                        case  style::LineNumberPosition::INSIDE:
                            aFontMetric.SetPos(LINENUMBER_POS_INSIDE);      ;
                        break;
                        case  style::LineNumberPosition::OUTSIDE:
                            aFontMetric.SetPos(LINENUMBER_POS_OUTSIDE);
                        break;
                    }
                }
                break;
                case WID_DISTANCE        :
                {
                    sal_Int32 nVal = 0;
                    aValue >>= nVal;
                    sal_Int32 nTmp = convertMm100ToTwip(nVal);
                    if (nTmp > USHRT_MAX)
                        nTmp = USHRT_MAX;
                    aFontMetric.SetPosFromLeft( static_cast< sal_uInt16 >(nTmp) );
                }
                break;
                case WID_INTERVAL   :
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if( nTmp > 0)
                        aFontMetric.SetCountBy(nTmp);
                }
                break;
                case WID_SEPARATOR_TEXT  :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFontMetric.SetDivider(uTmp);
                }
                break;
                case WID_SEPARATOR_INTERVAL:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if( nTmp >= 0)
                        aFontMetric.SetDividerCountBy(nTmp);
                }
                break;
                case WID_COUNT_EMPTY_LINES :
                {
                    bool bVal = *o3tl::doAccess<bool>(aValue);
                    aFontMetric.SetCountBlankLines(bVal);
                }
                break;
                case WID_COUNT_LINES_IN_FRAMES :
                {
                    bool bVal = *o3tl::doAccess<bool>(aValue);
                    aFontMetric.SetCountInFlys(bVal);
                }
                break;
                case WID_RESTART_AT_EACH_PAGE :
                {
                    bool bVal = *o3tl::doAccess<bool>(aValue);
                    aFontMetric.SetRestartEachPage(bVal);
                }
                break;
            }
            pDoc->SetLineNumberInfo(aFontMetric);
        }
        else
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
}

Any SwXLineNumberingProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(pDoc)
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            const SwLineNumberInfo& rInfo = pDoc->GetLineNumberInfo();
            switch(pEntry->nWID)
            {
                case WID_NUM_ON:
                    aRet <<= rInfo.IsPaintLineNumbers();
                break;
                case WID_CHARACTER_STYLE :
                {
                    OUString aString;
                    // return empty string if no char format is set
                    // otherwise it would be created here
                    if(rInfo.HasCharFormat())
                    {
                        SwStyleNameMapper::FillProgName(
                                    rInfo.GetCharFormat(pDoc->getIDocumentStylePoolAccess())->GetName(),
                                    aString,
                                    SwGetPoolIdFromName::ChrFmt,
                                    true);
                    }
                    aRet <<= aString;
                }
                break;
                case WID_NUMBERING_TYPE  :
                    aRet <<= rInfo.GetNumType().GetNumberingType();
                break;
                case WID_NUMBER_POSITION :
                {
                    sal_Int16 nRet = 0;
                    switch(rInfo.GetPos())
                    {
                        case  LINENUMBER_POS_LEFT:
                            nRet = style::LineNumberPosition::LEFT;
                        break;
                        case LINENUMBER_POS_RIGHT :
                            nRet = style::LineNumberPosition::RIGHT      ;
                        break;
                        case  LINENUMBER_POS_INSIDE:
                            nRet = style::LineNumberPosition::INSIDE     ;
                        break;
                        case LINENUMBER_POS_OUTSIDE :
                            nRet = style::LineNumberPosition::OUTSIDE    ;
                        break;
                    }
                    aRet <<= nRet;
                }
                break;
                case WID_DISTANCE        :
                {
                    sal_uInt32 nPos = rInfo.GetPosFromLeft();
                    if(USHRT_MAX == nPos)
                        nPos = 0;
                    aRet <<= static_cast < sal_Int32 >(convertTwipToMm100(nPos));
                }
                break;
                case WID_INTERVAL   :
                    aRet <<= (sal_Int16)rInfo.GetCountBy();
                break;
                case WID_SEPARATOR_TEXT  :
                    aRet <<= rInfo.GetDivider();
                break;
                case WID_SEPARATOR_INTERVAL:
                    aRet <<= (sal_Int16)rInfo.GetDividerCountBy();
                break;
                case WID_COUNT_EMPTY_LINES :
                    aRet <<= rInfo.IsCountBlankLines();
                break;
                case WID_COUNT_LINES_IN_FRAMES :
                    aRet <<= rInfo.IsCountInFlys();
                break;
                case WID_RESTART_AT_EACH_PAGE :
                    aRet <<= rInfo.IsRestartEachPage();
                break;
            }
        }
        else
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXLineNumberingProperties::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
OSL_FAIL("not implemented");
}

void SwXLineNumberingProperties::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
OSL_FAIL("not implemented");
}

void SwXLineNumberingProperties::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
OSL_FAIL("not implemented");
}

void SwXLineNumberingProperties::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
OSL_FAIL("not implemented");
}

static const char aInvalidStyle[] = "__XXX___invalid";

class SwXNumberingRules::Impl : public SwClient
{
private:
    SwXNumberingRules& m_rParent;
public:
    explicit Impl(SwXNumberingRules& rParent) : m_rParent(rParent) {}
protected:
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

bool SwXNumberingRules::isInvalidStyle(const OUString &rName)
{
    return rName == aInvalidStyle;
}

namespace
{
    class theSwXNumberingRulesUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXNumberingRulesUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXNumberingRules::getUnoTunnelId()
{
    return theSwXNumberingRulesUnoTunnelId::get().getSeq();
}

// return implementation specific data
sal_Int64 SwXNumberingRules::getSomething( const uno::Sequence< sal_Int8 > & rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

OUString SwXNumberingRules::getImplementationName()
{
    return OUString("SwXNumberingRules");
}

sal_Bool SwXNumberingRules::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXNumberingRules::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.NumberingRules" };
    return aRet;
}

SwXNumberingRules::SwXNumberingRules(const SwNumRule& rRule, SwDoc* doc) :
    m_pImpl(new SwXNumberingRules::Impl(*this)),
    pDoc(doc),
    pDocShell(nullptr),
    pNumRule(new SwNumRule(rRule)),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(true)
{
    // first organize the document - it is dependent on the set character formats
    // if no format is set, it should work as well
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        SwNumFormat rFormat(pNumRule->Get(i));
        SwCharFormat* pCharFormat = rFormat.GetCharFormat();
        if(pCharFormat)
        {
            pDoc = pCharFormat->GetDoc();
            break;
        }
    }
    if(pDoc)
        pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(&*m_pImpl);
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        m_sNewCharStyleNames[i] = aInvalidStyle;
        m_sNewBulletFontNames[i] = aInvalidStyle;
    }
}

SwXNumberingRules::SwXNumberingRules(SwDocShell& rDocSh) :
    m_pImpl(new SwXNumberingRules::Impl(*this)),
    pDoc(nullptr),
    pDocShell(&rDocSh),
    pNumRule(nullptr),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(false)
{
    pDocShell->GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(&*m_pImpl);
}

SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc) :
    m_pImpl(new SwXNumberingRules::Impl(*this)),
    pDoc(&rDoc),
    pDocShell(nullptr),
    pNumRule(nullptr),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(false)
{
    rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(&*m_pImpl);
    m_sCreatedNumRuleName = rDoc.GetUniqueNumRuleName();
#if OSL_DEBUG_LEVEL > 1
    const sal_uInt16 nIndex =
#endif
    rDoc.MakeNumRule( m_sCreatedNumRuleName, nullptr, false,
                      // #i89178#
                      numfunc::GetDefaultPositionAndSpaceMode() );
#if OSL_DEBUG_LEVEL > 1
    (void)nIndex;
#endif
}

SwXNumberingRules::~SwXNumberingRules()
{
    SolarMutexGuard aGuard;
    if(pDoc && !m_sCreatedNumRuleName.isEmpty())
        pDoc->DelNumRule( m_sCreatedNumRuleName );
    if( bOwnNumRuleCreated )
        delete pNumRule;
}

void SwXNumberingRules::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;
    if(nIndex < 0 || MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();

    auto rProperties = o3tl::tryAccess<uno::Sequence<beans::PropertyValue>>(
        rElement);
    if(!rProperties)
        throw lang::IllegalArgumentException();
    SwNumRule* pRule = nullptr;
    if(pNumRule)
        SwXNumberingRules::SetNumberingRuleByIndex( *pNumRule,
                            *rProperties, nIndex);
    else if(pDocShell)
    {
        // #i87650# - correction of cws swwarnings:
        SwNumRule aNumRule( *(pDocShell->GetDoc()->GetOutlineNumRule()) );
        SwXNumberingRules::SetNumberingRuleByIndex( aNumRule,
                            *rProperties, nIndex);
        // set character format if needed
        const SwCharFormats* pFormats = pDocShell->GetDoc()->GetCharFormats();
        const size_t nChCount = pFormats->size();
        for(sal_uInt16 i = 0; i < MAXLEVEL;i++)
        {
            SwNumFormat aFormat(aNumRule.Get( i ));
            if (!m_sNewCharStyleNames[i].isEmpty() &&
                m_sNewCharStyleNames[i] != UNO_NAME_CHARACTER_FORMAT_NONE &&
                (!aFormat.GetCharFormat() || aFormat.GetCharFormat()->GetName()!= m_sNewCharStyleNames[i]))
            {
                SwCharFormat* pCharFormat = nullptr;
                for(size_t j = 0; j< nChCount; ++j)
                {
                    SwCharFormat* pTmp = (*pFormats)[j];
                    if(pTmp->GetName() == m_sNewCharStyleNames[i])
                    {
                        pCharFormat = pTmp;
                        break;
                    }
                }
                if(!pCharFormat)
                {
                    SfxStyleSheetBase* pBase;
                    pBase = pDocShell->GetStyleSheetPool()->Find(m_sNewCharStyleNames[i],
                                                                    SfxStyleFamily::Char);
                    if(!pBase)
                        pBase = &pDocShell->GetStyleSheetPool()->Make(m_sNewCharStyleNames[i], SfxStyleFamily::Char);
                    pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

                }
                aFormat.SetCharFormat( pCharFormat );
                aNumRule.Set( i, aFormat );
            }
        }
        pDocShell->GetDoc()->SetOutlineNumRule( aNumRule );
    }
    else if(!pNumRule && pDoc && !m_sCreatedNumRuleName.isEmpty() &&
        nullptr != (pRule = pDoc->FindNumRulePtr( m_sCreatedNumRuleName )))
    {
        SwXNumberingRules::SetNumberingRuleByIndex( *pRule,
                            *rProperties, nIndex);

        pRule->Validate();
    }
    else
        throw uno::RuntimeException();
}

sal_Int32 SwXNumberingRules::getCount()
{
    return MAXLEVEL;
}

uno::Any SwXNumberingRules::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if(nIndex < 0 || MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();

    uno::Any aVal;
    const SwNumRule* pRule = pNumRule;
    if(!pRule && pDoc && !m_sCreatedNumRuleName.isEmpty())
        pRule = pDoc->FindNumRulePtr( m_sCreatedNumRuleName );
    if(pRule)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
                                        *pRule, nIndex);
        aVal <<= aRet;

    }
    else if(pDocShell)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
                *pDocShell->GetDoc()->GetOutlineNumRule(), nIndex);
        aVal <<= aRet;
    }
    else
        throw uno::RuntimeException();
    return aVal;
}

uno::Type SwXNumberingRules::getElementType()
{
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SwXNumberingRules::hasElements()
{
    return true;
}

uno::Sequence<beans::PropertyValue> SwXNumberingRules::GetNumberingRuleByIndex(
                const SwNumRule& rNumRule, sal_Int32 nIndex) const
{
    SolarMutexGuard aGuard;
    OSL_ENSURE( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    const SwNumFormat& rFormat = rNumRule.Get( (sal_uInt16)nIndex );

    SwCharFormat* pCharFormat = rFormat.GetCharFormat();
    OUString CharStyleName;
    if (pCharFormat)
        CharStyleName = pCharFormat->GetName();

    // Whether or not a style is present: the array entry overwrites this string
    if (!m_sNewCharStyleNames[nIndex].isEmpty() &&
        !SwXNumberingRules::isInvalidStyle(m_sNewCharStyleNames[nIndex]))
    {
        CharStyleName = m_sNewCharStyleNames[nIndex];
    }

    OUString aUString;
    if (pDocShell) // -> Chapter Numbering
    {
        // template name
        OUString sValue(SW_RES(STR_POOLCOLL_HEADLINE1 + nIndex));
        const SwTextFormatColls* pColls = pDocShell->GetDoc()->GetTextFormatColls();
        const size_t nCount = pColls->size();
        for(size_t i = 0; i < nCount; ++i)
        {
            SwTextFormatColl &rTextColl = *pColls->operator[](i);
            if(rTextColl.IsDefault())
                continue;

            const sal_Int16 nOutLevel = rTextColl.IsAssignedToListLevelOfOutlineStyle()
                                        ? static_cast<sal_Int16>(rTextColl.GetAssignedOutlineStyleLevel())
                                        : MAXLEVEL;
            if ( nOutLevel == nIndex )
            {
                sValue = rTextColl.GetName();
                break; // the style for the level in question has been found
            }
            else if( sValue==rTextColl.GetName() )
            {
                // if the default for the level is existing, but its
                // level is different, then it cannot be the default.
                sValue.clear();
            }
        }
        SwStyleNameMapper::FillProgName(sValue, aUString, SwGetPoolIdFromName::TxtColl, true);
    }

    return GetPropertiesForNumFormat(rFormat, CharStyleName, (pDocShell) ? & aUString : nullptr);

}

uno::Sequence<beans::PropertyValue> SwXNumberingRules::GetPropertiesForNumFormat(
        const SwNumFormat& rFormat, OUString const& rCharFormatName,
        OUString const*const pHeadingStyleName)
{
    bool bChapterNum = pHeadingStyleName != nullptr;

    std::vector<PropertyValue> aPropertyValues;
    aPropertyValues.reserve(32);
    //fill all properties into the array

    //adjust
    SvxAdjust eAdj = rFormat.GetNumAdjust();
    sal_Int16 nINT16 = aSvxToUnoAdjust[eAdj];
    aPropertyValues.push_back(comphelper::makePropertyValue("Adjust", nINT16));

    //parentnumbering
    nINT16 = rFormat.GetIncludeUpperLevels();
    aPropertyValues.push_back(comphelper::makePropertyValue("ParentNumbering", nINT16));

    //prefix
    OUString aUString = rFormat.GetPrefix();
    aPropertyValues.push_back(comphelper::makePropertyValue("Prefix", aUString));

    //suffix
    aUString = rFormat.GetSuffix();
    aPropertyValues.push_back(comphelper::makePropertyValue("Suffix", aUString));

    //char style name

    aUString.clear();
    SwStyleNameMapper::FillProgName( rCharFormatName, aUString, SwGetPoolIdFromName::ChrFmt, true );
    aPropertyValues.push_back(comphelper::makePropertyValue("CharStyleName", aUString));

    //startvalue
    nINT16 = rFormat.GetStart();
    aPropertyValues.push_back(comphelper::makePropertyValue("StartWith", nINT16));

    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        //leftmargin
        sal_Int32 nINT32 = convertTwipToMm100(rFormat.GetAbsLSpace());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_LEFT_MARGIN, nINT32));

        //chartextoffset
        nINT32 = convertTwipToMm100(rFormat.GetCharTextDistance());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_SYMBOL_TEXT_DISTANCE, nINT32));

        //firstlineoffset
        nINT32 = convertTwipToMm100(rFormat.GetFirstLineOffset());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_FIRST_LINE_OFFSET, nINT32));
    }

    // PositionAndSpaceMode
    nINT16 = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        nINT16 = PositionAndSpaceMode::LABEL_ALIGNMENT;
    }
    aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_POSITION_AND_SPACE_MODE, nINT16));

    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        // LabelFollowedBy
        nINT16 = LabelFollow::LISTTAB;
        if ( rFormat.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
        {
            nINT16 = LabelFollow::SPACE;
        }
        else if ( rFormat.GetLabelFollowedBy() == SvxNumberFormat::NOTHING )
        {
            nINT16 = LabelFollow::NOTHING;
        }
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_LABEL_FOLLOWED_BY, nINT16));

        // ListtabStopPosition
        sal_Int32 nINT32 = convertTwipToMm100(rFormat.GetListtabPos());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_LISTTAB_STOP_POSITION, nINT32));

        // FirstLineIndent
        nINT32 = convertTwipToMm100(rFormat.GetFirstLineIndent());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_FIRST_LINE_INDENT, nINT32));

        // IndentAt
        nINT32 = convertTwipToMm100(rFormat.GetIndentAt());
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_INDENT_AT, nINT32));
    }

    //numberingtype
    nINT16 = rFormat.GetNumberingType();
    aPropertyValues.push_back(comphelper::makePropertyValue("NumberingType", nINT16));

    if(!bChapterNum)
    {
        if(SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType())
        {
            //BulletId
            nINT16 = rFormat.GetBulletChar();
            aPropertyValues.push_back(comphelper::makePropertyValue("BulletId", nINT16));

            const vcl::Font* pFont = rFormat.GetBulletFont();

            //BulletChar
            aUString = OUString(rFormat.GetBulletChar());
            aPropertyValues.push_back(comphelper::makePropertyValue("BulletChar", aUString));

            //BulletFontName
            aUString = pFont ? pFont->GetStyleName() : OUString();
            aPropertyValues.push_back(comphelper::makePropertyValue("BulletFontName", aUString));

            //BulletFont
            if(pFont)
            {
                 awt::FontDescriptor aDesc;
                SvxUnoFontDescriptor::ConvertFromFont( *pFont, aDesc );
                aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_BULLET_FONT, aDesc));
            }
        }
        if(SVX_NUM_BITMAP == rFormat.GetNumberingType())
        {
            //GraphicURL
            const SvxBrushItem* pBrush = rFormat.GetBrush();
            if(pBrush)
            {
                Any aAny;
                pBrush->QueryValue( aAny, MID_GRAPHIC_URL );
                aAny >>= aUString;
            }
            else
                aUString.clear();
            aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_GRAPHIC_URL, aUString));

            //graphicbitmap
            const Graphic* pGraphic = nullptr;
            if(pBrush )
                pGraphic = pBrush->GetGraphic();
            if(pGraphic)
            {
                uno::Reference<awt::XBitmap> xBmp = VCLUnoHelper::CreateBitmap( pGraphic->GetBitmapEx() );
                aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_GRAPHIC_BITMAP, xBmp));
            }
             Size aSize = rFormat.GetGraphicSize();
            // #i101131#
            // adjust conversion due to type mismatch between <Size> and <awt::Size>
            awt::Size aAwtSize(convertTwipToMm100(aSize.Width()), convertTwipToMm100(aSize.Height()));
            aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_GRAPHIC_SIZE, aAwtSize));

            const SwFormatVertOrient* pOrient = rFormat.GetGraphicOrientation();
            if(pOrient)
            {
                uno::Any any;
                pOrient->QueryValue(any);
                aPropertyValues.push_back(PropertyValue(
                    UNO_NAME_VERT_ORIENT, -1, any, PropertyState_DIRECT_VALUE));
            }
        }
    }
    else
    {
        aUString = *pHeadingStyleName;
        aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_HEADING_STYLE_NAME, aUString));
    }

    return ::comphelper::containerToSequence(aPropertyValues);
}

static PropertyValue const* lcl_FindProperty(
    const char* cName, std::vector<PropertyValue const*> const& rPropertyValues)
{
    const OUString sCmp = OUString::createFromAscii(cName);
    for(const PropertyValue* pTemp : rPropertyValues)
    {
        if (sCmp == pTemp->Name)
            return pTemp;
    }
    return nullptr;
}

void SwXNumberingRules::SetNumberingRuleByIndex(
            SwNumRule& rNumRule,
            const uno::Sequence<beans::PropertyValue>& rProperties, sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    OSL_ENSURE( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    SwNumFormat aFormat(rNumRule.Get( (sal_uInt16)nIndex ));

    OUString sHeadingStyleName;
    OUString sParagraphStyleName;

    SetPropertiesToNumFormat(aFormat, m_sNewCharStyleNames[nIndex],
        &m_sNewBulletFontNames[nIndex],
        &sHeadingStyleName, &sParagraphStyleName,
        pDoc, pDocShell, rProperties);


    if (pDoc && !sParagraphStyleName.isEmpty())
    {
        const SwTextFormatColls* pColls = pDoc->GetTextFormatColls();
        const size_t nCount = pColls->size();
        for (size_t k = 0; k < nCount; ++k)
        {
            SwTextFormatColl &rTextColl = *((*pColls)[k]);
            if (rTextColl.GetName() == sParagraphStyleName)
                rTextColl.SetFormatAttr( SwNumRuleItem( rNumRule.GetName()));
        }
    }

    if (!sHeadingStyleName.isEmpty())
    {
        assert(pDocShell);
        const SwTextFormatColls* pColls = pDocShell->GetDoc()->GetTextFormatColls();
        const size_t nCount = pColls->size();
        for (size_t k = 0; k < nCount; ++k)
        {
            SwTextFormatColl &rTextColl = *((*pColls)[k]);
            if (rTextColl.IsDefault())
                continue;
            if (rTextColl.IsAssignedToListLevelOfOutlineStyle() &&
                rTextColl.GetAssignedOutlineStyleLevel() == nIndex &&
                rTextColl.GetName() != sHeadingStyleName)
            {
                rTextColl.DeleteAssignmentToListLevelOfOutlineStyle();
            }
            else if (rTextColl.GetName() == sHeadingStyleName)
            {
                rTextColl.AssignToListLevelOfOutlineStyle( nIndex );
            }
        }
    }

    rNumRule.Set(static_cast<sal_uInt16>(nIndex), aFormat);
}

void SwXNumberingRules::SetPropertiesToNumFormat(
        SwNumFormat & aFormat,
        OUString & rCharStyleName, OUString *const pBulletFontName,
        OUString *const pHeadingStyleName,
        OUString *const pParagraphStyleName,
        SwDoc *const pDoc, SwDocShell *const pDocShell,
        const uno::Sequence<beans::PropertyValue>& rProperties)
{
    // the order of the names is important!
    static const char* aNumPropertyNames[] =
    {
        "Adjust",                               //0
        "ParentNumbering",                      //1
        "Prefix",                               //2
        "Suffix",                               //3
        "CharStyleName",                        //4
        "StartWith",                            //5
        UNO_NAME_LEFT_MARGIN,                   //6
        UNO_NAME_SYMBOL_TEXT_DISTANCE,          //7
        UNO_NAME_FIRST_LINE_OFFSET,             //8
        UNO_NAME_POSITION_AND_SPACE_MODE, //9
        UNO_NAME_LABEL_FOLLOWED_BY,       //10
        UNO_NAME_LISTTAB_STOP_POSITION,   //11
        UNO_NAME_FIRST_LINE_INDENT,       //12
        UNO_NAME_INDENT_AT,               //13
        "NumberingType",                        //14
        UNO_NAME_PARAGRAPH_STYLE_NAME, //15
        // these are not in chapter numbering
        "BulletId",                             //16
        UNO_NAME_BULLET_FONT, //17
        "BulletFontName",                       //18
        "BulletChar",                           //19
        UNO_NAME_GRAPHIC_URL,    //20
        UNO_NAME_GRAPHIC_BITMAP, //21
        UNO_NAME_GRAPHIC_SIZE,   //22
        UNO_NAME_VERT_ORIENT,    //23
        // these are only in chapter numbering
        UNO_NAME_HEADING_STYLE_NAME, //24
        // these two are accepted but ignored for some reason
        "BulletRelSize",                         // 25
        "BulletColor"                            // 26
    };

    enum {
        NotInChapterFirst = 16,
        NotInChapterLast = 23,
        InChapterFirst = 24,
        InChapterLast = 24
    };

    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    std::vector<PropertyValue const*> aPropertyValues;
    bool bExcept = false;
    for(sal_Int32 i = 0; i < rProperties.getLength() && !bExcept; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        bExcept = true;
        for(size_t j = 0; j < SAL_N_ELEMENTS( aNumPropertyNames ); j++)
        {
            if (pDocShell &&
                j >= static_cast<size_t>(NotInChapterFirst) &&
                j <= static_cast<size_t>(NotInChapterLast))
                continue;
            if (!pDocShell &&
                j >= static_cast<size_t>(InChapterFirst) &&
                j <= static_cast<size_t>(InChapterLast))
                continue;
            if (rProp.Name.equalsAscii(aNumPropertyNames[j]))
            {
                bExcept = false;
                break;
            }
        }
        SAL_WARN_IF( bExcept, "sw.uno", "Unknown/incorrect property " << rProp.Name << ", failing" );
        aPropertyValues.push_back(& rProp);
    }

    bool bWrongArg = false;
    if(!bExcept)
       {
        SvxBrushItem* pSetBrush = nullptr;
        Size* pSetSize = nullptr;
        SwFormatVertOrient* pSetVOrient = nullptr;
        bool bCharStyleNameSet = false;

        for(size_t i = 0; i < SAL_N_ELEMENTS( aNumPropertyNames ) && !bExcept && !bWrongArg; ++i)
        {
            PropertyValue const*const pProp(
                    lcl_FindProperty(aNumPropertyNames[i], aPropertyValues));
            if (!pProp)
                continue;
            switch(i)
            {
                case 0: //"Adjust"
                {
                    sal_Int16 nValue = text::HoriOrientation::NONE;
                    pProp->Value >>= nValue;
                    if (nValue > text::HoriOrientation::NONE &&
                        nValue <= text::HoriOrientation::LEFT &&
                        USHRT_MAX != aUnoToSvxAdjust[nValue])
                    {
                        aFormat.SetNumAdjust((SvxAdjust)aUnoToSvxAdjust[nValue]);
                    }
                    else
                        bWrongArg = true;
                }
                break;
                case 1: //"ParentNumbering",
                {
                    sal_Int16 nSet = 0;
                    pProp->Value >>= nSet;
                    if(nSet >= 0 && MAXLEVEL >= nSet)
                        aFormat.SetIncludeUpperLevels( static_cast< sal_uInt8 >(nSet) );
                }
                break;
                case 2: //"Prefix",
                {
                    OUString uTmp;
                    pProp->Value >>= uTmp;
                    aFormat.SetPrefix(uTmp);
                }
                break;
                case 3: //"Suffix",
                {
                    OUString uTmp;
                    pProp->Value >>= uTmp;
                    aFormat.SetSuffix(uTmp);
                }
                break;
                case 4: //"CharStyleName",
                {
                    bCharStyleNameSet = true;
                    OUString uTmp;
                    pProp->Value >>= uTmp;
                    OUString sCharFormatName;
                    SwStyleNameMapper::FillUIName( uTmp, sCharFormatName, SwGetPoolIdFromName::ChrFmt, true );
                    if (sCharFormatName == UNO_NAME_CHARACTER_FORMAT_NONE)
                    {
                        rCharStyleName = aInvalidStyle;
                        aFormat.SetCharFormat(nullptr);
                    }
                    else if(pDocShell || pDoc)
                    {
                        SwDoc* pLocalDoc = pDoc ? pDoc : pDocShell->GetDoc();
                        const SwCharFormats* pFormats = pLocalDoc->GetCharFormats();
                        const size_t nChCount = pFormats->size();

                        SwCharFormat* pCharFormat = nullptr;
                        if (!sCharFormatName.isEmpty())
                        {
                            for(size_t j = 0; j< nChCount; ++j)
                            {
                                SwCharFormat* pTmp = (*pFormats)[j];
                                if(pTmp->GetName() == sCharFormatName)
                                {
                                    pCharFormat = pTmp;
                                    break;
                                }
                            }
                            if(!pCharFormat)
                            {

                                SfxStyleSheetBase* pBase;
                                SfxStyleSheetBasePool* pPool = pLocalDoc->GetDocShell()->GetStyleSheetPool();
                                pBase = pPool->Find(sCharFormatName, SfxStyleFamily::Char);
                                if(!pBase)
                                    pBase = &pPool->Make(sCharFormatName, SfxStyleFamily::Char);
                                pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();
                            }
                        }
                        aFormat.SetCharFormat( pCharFormat );
                        // #i51842#
                        // If the character format has been found its name should not be in the
                        // char style names array
                        rCharStyleName.clear();
                     }
                    else
                        rCharStyleName = sCharFormatName;
                }
                break;
                case 5: //"StartWith",
                {
                    sal_Int16 nVal = 0;
                    pProp->Value >>= nVal;
                    aFormat.SetStart(nVal);
                }
                break;
                case 6: //UNO_NAME_LEFT_MARGIN,
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    // #i23727# nValue can be negative
                    aFormat.SetAbsLSpace((short) convertMm100ToTwip(nValue));
                }
                break;
                case 7: //UNO_NAME_SYMBOL_TEXT_DISTANCE,
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    if(nValue >= 0)
                        aFormat.SetCharTextDistance((short) convertMm100ToTwip(nValue));
                    else
                        bWrongArg = true;
                }
                break;
                case 8: //UNO_NAME_FIRST_LINE_OFFSET,
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    // #i23727# nValue can be positive
                    nValue = convertMm100ToTwip(nValue);
                    aFormat.SetFirstLineOffset((short)nValue);
                }
                break;
                case 9: // UNO_NAME_POSITION_AND_SPACE_MODE
                {
                    sal_Int16 nValue = 0;
                    pProp->Value >>= nValue;
                    if ( nValue == 0 )
                    {
                        aFormat.SetPositionAndSpaceMode( SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
                    }
                    else if ( nValue == 1 )
                    {
                        aFormat.SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
                    }
                    else
                    {
                        bWrongArg = true;
                    }
                }
                break;
                case 10: // UNO_NAME_LABEL_FOLLOWED_BY
                {
                    sal_Int16 nValue = 0;
                    pProp->Value >>= nValue;
                    if ( nValue == 0 )
                    {
                        aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                    }
                    else if ( nValue == 1 )
                    {
                        aFormat.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                    }
                    else if ( nValue == 2 )
                    {
                        aFormat.SetLabelFollowedBy( SvxNumberFormat::NOTHING );
                    }
                    else
                    {
                        bWrongArg = true;
                    }
                }
                break;
                case 11: // UNO_NAME_LISTTAB_STOP_POSITION
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    nValue = convertMm100ToTwip(nValue);
                    if ( nValue >= 0 )
                    {
                        aFormat.SetListtabPos( nValue );
                    }
                    else
                    {
                        bWrongArg = true;
                    }
                }
                break;
                case 12: // UNO_NAME_FIRST_LINE_INDENT
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    nValue = convertMm100ToTwip(nValue);
                    aFormat.SetFirstLineIndent( nValue );
                }
                break;
                case 13: // UNO_NAME_INDENT_AT
                {
                    sal_Int32 nValue = 0;
                    pProp->Value >>= nValue;
                    nValue = convertMm100ToTwip(nValue);
                    aFormat.SetIndentAt( nValue );
                }
                break;
                case 14: //"NumberingType"
                {
                    sal_Int16 nSet = 0;
                    pProp->Value >>= nSet;
                    if(nSet >= 0)
                        aFormat.SetNumberingType(nSet);
                    else
                        bWrongArg = true;
                }
                break;
                case 15: //"ParagraphStyleName"
                {
                    if (pParagraphStyleName)
                    {
                        OUString uTmp;
                        pProp->Value >>= uTmp;
                        OUString sStyleName;
                        SwStyleNameMapper::FillUIName(uTmp, sStyleName, SwGetPoolIdFromName::TxtColl, true );
                        *pParagraphStyleName = sStyleName;
                    }
                }
                break;
                case 16: //"BulletId",
                {
                    assert( !pDocShell );
                    sal_Int16 nSet = 0;
                    if( pProp->Value >>= nSet )
                        aFormat.SetBulletChar(nSet);
                    else
                        bWrongArg = true;
                }
                break;
                case 17: //UNO_NAME_BULLET_FONT,
                {
                    assert( !pDocShell );
                    awt::FontDescriptor desc;
                    if (pProp->Value >>= desc)
                    {
                        // #i93725#
                        // do not accept "empty" font
                        if (!desc.Name.isEmpty())
                        {
                            vcl::Font aFont;
                            SvxUnoFontDescriptor::ConvertToFont(desc, aFont);
                            aFormat.SetBulletFont(&aFont);
                        }
                    }
                    else
                        bWrongArg = true;
                }
                break;
                case 18: //"BulletFontName",
                {
                    assert( !pDocShell );
                    OUString sBulletFontName;
                    pProp->Value >>= sBulletFontName;
                    SwDocShell* pLclDocShell = pDocShell ? pDocShell : pDoc ? pDoc->GetDocShell() : nullptr;
                    if( !sBulletFontName.isEmpty() && pLclDocShell )
                    {
                        const SvxFontListItem* pFontListItem =
                                static_cast<const SvxFontListItem* >(pLclDocShell
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST ));
                        const FontList*  pList = pFontListItem->GetFontList();
                        FontMetric aFontMetric = pList->Get(
                            sBulletFontName, WEIGHT_NORMAL, ITALIC_NONE);
                        vcl::Font aFont(aFontMetric);
                        aFormat.SetBulletFont(&aFont);
                    }
                    else if (pBulletFontName)
                        *pBulletFontName = sBulletFontName;
                }
                break;
                case 19: //"BulletChar",
                {
                    assert( !pDocShell );
                    OUString aChar;
                    pProp->Value >>= aChar;
                    if(aChar.getLength() == 1)
                    {
                        aFormat.SetBulletChar(aChar.toChar());
                    }
                    else if(aChar.isEmpty())
                    {
                        // If w:lvlText's value is null - set bullet char to zero
                        aFormat.SetBulletChar(sal_Unicode(0x0));
                    }
                    else
                    {
                        bWrongArg = true;
                    }
                }
                break;
                case 20: //UNO_NAME_GRAPHIC_URL,
                {
                    assert( !pDocShell );
                    OUString sBrushURL;
                    pProp->Value >>= sBrushURL;
                    if(!pSetBrush)
                    {
                        const SvxBrushItem* pOrigBrush = aFormat.GetBrush();
                        if(pOrigBrush)
                        {
                            pSetBrush = new SvxBrushItem(*pOrigBrush);
                        }
                        else
                            pSetBrush = new SvxBrushItem(OUString(), OUString(), GPOS_AREA, RES_BACKGROUND);
                    }
                    pSetBrush->PutValue( pProp->Value, MID_GRAPHIC_URL );
                }
                break;
                case 21: //UNO_NAME_GRAPHIC_BITMAP,
                {
                    assert( !pDocShell );
                    uno::Reference<awt::XBitmap> xBitmap;
                    if (pProp->Value >>= xBitmap)
                    {
                        if(!pSetBrush)
                        {
                            const SvxBrushItem* pOrigBrush = aFormat.GetBrush();
                            if(pOrigBrush)
                            {
                                pSetBrush = new SvxBrushItem(*pOrigBrush);
                            }
                            else
                                pSetBrush = new SvxBrushItem(OUString(), OUString(), GPOS_AREA, RES_BACKGROUND);
                        }

                        BitmapEx aBmp = VCLUnoHelper::GetBitmap(xBitmap);
                        Graphic aNewGr(aBmp);
                        pSetBrush->SetGraphic( aNewGr );
                    }
                    else
                        bWrongArg = true;
                }
                break;
                case 22: //UNO_NAME_GRAPHIC_SIZE,
                {
                    assert( !pDocShell );
                    if(!pSetSize)
                        pSetSize = new Size;
                    awt::Size size;
                    if (pProp->Value >>= size)
                    {
                        size.Width = convertMm100ToTwip(size.Width);
                        size.Height = convertMm100ToTwip(size.Height);
                        pSetSize->Width() = size.Width;
                        pSetSize->Height() = size.Height;
                    }
                    else
                        bWrongArg = true;
                }
                break;
                case 23: //VertOrient
                {
                    assert( !pDocShell );
                    if(!pSetVOrient)
                    {
                        if(aFormat.GetGraphicOrientation())
                            pSetVOrient = static_cast<SwFormatVertOrient*>(aFormat.GetGraphicOrientation()->Clone());
                        else
                            pSetVOrient = new SwFormatVertOrient;
                    }
                    pSetVOrient->PutValue(pProp->Value, MID_VERTORIENT_ORIENT);
                }
                break;
                case 24: //"HeadingStyleName"
                {
                    if (pHeadingStyleName)
                    {
                        OUString uTmp;
                        pProp->Value >>= uTmp;
                        OUString sStyleName;
                        SwStyleNameMapper::FillUIName(uTmp, sStyleName, SwGetPoolIdFromName::TxtColl, true );
                        *pHeadingStyleName = sStyleName;
                    }
                }
                break;
                case 25: // BulletRelSize - unsupported - only available in Impress
                break;
                case 26: // BulletColor - ignored too
                break;
            }
        }
        if(!bExcept && !bWrongArg && (pSetBrush || pSetSize || pSetVOrient))
        {
            if(!pSetBrush && aFormat.GetBrush())
                pSetBrush = new SvxBrushItem(*aFormat.GetBrush());

            if(pSetBrush)
            {
                if(!pSetVOrient && aFormat.GetGraphicOrientation())
                    pSetVOrient = new SwFormatVertOrient(*aFormat.GetGraphicOrientation());

                if(!pSetSize)
                {
                    pSetSize = new Size(aFormat.GetGraphicSize());
                    if(!pSetSize->Width() || !pSetSize->Height())
                    {
                        const Graphic* pGraphic = pSetBrush->GetGraphic();
                        if(pGraphic)
                            *pSetSize = ::GetGraphicSizeTwip(*pGraphic, nullptr);
                    }
                }
                sal_Int16 eOrient = pSetVOrient ?
                    (sal_Int16)pSetVOrient->GetVertOrient() : text::VertOrientation::NONE;
                aFormat.SetGraphicBrush( pSetBrush, pSetSize, text::VertOrientation::NONE == eOrient ? nullptr : &eOrient );
            }
        }
        if ((!bCharStyleNameSet || rCharStyleName.isEmpty())
            && aFormat.GetNumberingType() == NumberingType::BITMAP
            && !aFormat.GetCharFormat()
            && !SwXNumberingRules::isInvalidStyle(rCharStyleName))
        {
            OUString tmp;
            SwStyleNameMapper::FillProgName(RES_POOLCHR_BUL_LEVEL, tmp);
            rCharStyleName = tmp;
        }
        delete pSetBrush;
        delete pSetSize;
        delete pSetVOrient;
      }

    if(bWrongArg)
        throw lang::IllegalArgumentException();
    else if(bExcept)
        throw uno::RuntimeException();
}

uno::Reference< XPropertySetInfo > SwXNumberingRules::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
    return aRef;
}

void SwXNumberingRules::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    SolarMutexGuard aGuard;
    SwNumRule* pDocRule = nullptr;
    SwNumRule* pCreatedRule = nullptr;
    if(!pNumRule)
    {
        if(!pNumRule && pDocShell)
        {
            pDocRule = new SwNumRule(*pDocShell->GetDoc()->GetOutlineNumRule());
        }
        else if(pDoc && !m_sCreatedNumRuleName.isEmpty())
        {
            pCreatedRule = pDoc->FindNumRulePtr(m_sCreatedNumRuleName);
        }

    }
    if(!pNumRule && !pDocRule && !pCreatedRule)
        throw RuntimeException();

    if(rPropertyName == UNO_NAME_IS_AUTOMATIC)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        if(!pCreatedRule)
            pDocRule ? pDocRule->SetAutoRule(bVal) : pNumRule->SetAutoRule(bVal);
    }
    else if(rPropertyName == UNO_NAME_IS_CONTINUOUS_NUMBERING)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        pDocRule ? pDocRule->SetContinusNum(bVal) :
            pCreatedRule ? pCreatedRule->SetContinusNum(bVal) : pNumRule->SetContinusNum(bVal);
    }
    else if(rPropertyName == UNO_NAME_NAME)
    {
        delete pDocRule;
        throw IllegalArgumentException();
    }
    else if(rPropertyName == UNO_NAME_IS_ABSOLUTE_MARGINS)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        pDocRule ? pDocRule->SetAbsSpaces(bVal) :
            pCreatedRule ? pCreatedRule->SetAbsSpaces(bVal) : pNumRule->SetAbsSpaces(bVal);
    }
    else if(rPropertyName == UNO_NAME_NUMBERING_IS_OUTLINE)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        SwNumRuleType eNumRuleType = bVal ? OUTLINE_RULE : NUM_RULE;
        pDocRule ? pDocRule->SetRuleType(eNumRuleType) :
            pCreatedRule ? pCreatedRule->SetRuleType(eNumRuleType) : pNumRule->SetRuleType(eNumRuleType);
    }
    else if(rPropertyName == UNO_NAME_DEFAULT_LIST_ID)
    {
        delete pDocRule;
        throw IllegalArgumentException();
    }
    else
        throw UnknownPropertyException();

    if(pDocRule)
    {
        pDocShell->GetDoc()->SetOutlineNumRule(*pDocRule);
        delete pDocRule;
    }
    else if(pCreatedRule)
    {
        pCreatedRule->Validate();
    }
}

Any SwXNumberingRules::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;
    const SwNumRule* pRule = pNumRule;
    if(!pRule && pDocShell)
        pRule = pDocShell->GetDoc()->GetOutlineNumRule();
    else if(pDoc && !m_sCreatedNumRuleName.isEmpty())
        pRule = pDoc->FindNumRulePtr( m_sCreatedNumRuleName );
    if(!pRule)
        throw RuntimeException();

    if(rPropertyName == UNO_NAME_IS_AUTOMATIC)
    {
        aRet <<= pRule->IsAutoRule();
    }
    else if(rPropertyName == UNO_NAME_IS_CONTINUOUS_NUMBERING)
    {
        aRet <<= pRule->IsContinusNum();
    }
    else if(rPropertyName == UNO_NAME_NAME)
        aRet <<= pRule->GetName();
    else if(rPropertyName == UNO_NAME_IS_ABSOLUTE_MARGINS)
    {
        aRet <<= pRule->IsAbsSpaces();
    }
    else if(rPropertyName == UNO_NAME_NUMBERING_IS_OUTLINE)
    {
        aRet <<= pRule->IsOutlineRule();
    }
    else if(rPropertyName == UNO_NAME_DEFAULT_LIST_ID)
    {
        OSL_ENSURE( !pRule->GetDefaultListId().isEmpty(),
                "<SwXNumberingRules::getPropertyValue(..)> - no default list id found. Serious defect." );
        aRet <<= pRule->GetDefaultListId();
    }
    else
        throw UnknownPropertyException();
    return aRet;
}

void SwXNumberingRules::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXNumberingRules::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXNumberingRules::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
}

void SwXNumberingRules::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
}

OUString SwXNumberingRules::getName()
{
    if(pNumRule)
    {
        OUString aString;
        SwStyleNameMapper::FillProgName(pNumRule->GetName(), aString, SwGetPoolIdFromName::NumRule, true );
        return aString;
    }
    // consider chapter numbering <SwXNumberingRules>
    if ( pDocShell )
    {
        OUString aString;
        SwStyleNameMapper::FillProgName( pDocShell->GetDoc()->GetOutlineNumRule()->GetName(),
                                         aString, SwGetPoolIdFromName::NumRule, true );
        return aString;
    }
    return m_sCreatedNumRuleName;
}

void SwXNumberingRules::setName(const OUString& /*rName*/)
{
    RuntimeException aExcept;
    aExcept.Message = "readonly";
    throw aExcept;
}

void SwXNumberingRules::Impl::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        if(m_rParent.bOwnNumRuleCreated)
            delete m_rParent.pNumRule;
        m_rParent.pNumRule = nullptr;
        m_rParent.pDoc = nullptr;
    }
}

OUString SwXChapterNumbering::getImplementationName()
{
    return OUString("SwXChapterNumbering");
}

sal_Bool SwXChapterNumbering::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXChapterNumbering::getSupportedServiceNames()
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.ChapterNumbering";
    pArray[1] = "com.sun.star.text.NumberingRules";
    return aRet;
}

SwXChapterNumbering::SwXChapterNumbering(SwDocShell& rDocSh) :
    SwXNumberingRules(rDocSh)
{
}

SwXChapterNumbering::~SwXChapterNumbering()
{
}

OUString SwXTextColumns::getImplementationName()
{
    return OUString("SwXTextColumns");
}

sal_Bool SwXTextColumns::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextColumns::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextColumns" };
    return aRet;
}

SwXTextColumns::SwXTextColumns() :
    nReference(0),
    bIsAutomaticWidth(true),
    nAutoDistance(0),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_COLUMS)),
    nSepLineWidth(0),
    nSepLineColor(0), //black
    nSepLineHeightRelative(100),//full height
    nSepLineVertAlign(style::VerticalAlignment_MIDDLE),
    bSepLineIsOn(false),
    nSepLineStyle(API_COL_LINE_NONE) // None
{
}

SwXTextColumns::SwXTextColumns(const SwFormatCol& rFormatCol) :
    nReference(0),
    aTextColumns(rFormatCol.GetNumCols()),
    bIsAutomaticWidth(rFormatCol.IsOrtho()),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_COLUMS))
{
    const sal_uInt16 nItemGutterWidth = rFormatCol.GetGutterWidth();
    nAutoDistance = bIsAutomaticWidth ?
                        USHRT_MAX == nItemGutterWidth ? DEF_GUTTER_WIDTH : (sal_Int32)nItemGutterWidth
                        : 0;
    nAutoDistance = convertTwipToMm100(nAutoDistance);

    TextColumn* pColumns = aTextColumns.getArray();
    const SwColumns& rCols = rFormatCol.GetColumns();
    for(sal_Int32 i = 0; i < aTextColumns.getLength(); ++i)
    {
        const SwColumn* pCol = &rCols[i];

        pColumns[i].Width = pCol->GetWishWidth();
        nReference += pColumns[i].Width;
        pColumns[i].LeftMargin =    convertTwipToMm100(pCol->GetLeft ());
        pColumns[i].RightMargin =   convertTwipToMm100(pCol->GetRight());
    }
    if(!aTextColumns.getLength())
        nReference = USHRT_MAX;

    nSepLineWidth = rFormatCol.GetLineWidth();
    nSepLineColor = rFormatCol.GetLineColor().GetColor();
    nSepLineHeightRelative = rFormatCol.GetLineHeight();
    bSepLineIsOn = rFormatCol.GetLineAdj() != COLADJ_NONE;
    sal_Int8 nStyle = API_COL_LINE_NONE;
    switch (rFormatCol.GetLineStyle())
    {
        case table::BorderLineStyle::SOLID: nStyle = API_COL_LINE_SOLID; break;
        case table::BorderLineStyle::DOTTED: nStyle= API_COL_LINE_DOTTED; break;
        case table::BorderLineStyle::DASHED: nStyle= API_COL_LINE_DASHED; break;
        default: break;
    }
    nSepLineStyle = nStyle;
    switch(rFormatCol.GetLineAdj())
    {
        case COLADJ_TOP:    nSepLineVertAlign = style::VerticalAlignment_TOP;   break;
        case COLADJ_BOTTOM: nSepLineVertAlign = style::VerticalAlignment_BOTTOM;    break;
        case COLADJ_CENTER:
        case COLADJ_NONE:   nSepLineVertAlign = style::VerticalAlignment_MIDDLE;
    }
}

SwXTextColumns::~SwXTextColumns()
{
}

sal_Int32 SwXTextColumns::getReferenceValue()
{
    SolarMutexGuard aGuard;
    return nReference;
}

sal_Int16 SwXTextColumns::getColumnCount()
{
    SolarMutexGuard aGuard;
    return static_cast< sal_Int16>( aTextColumns.getLength() );
}

void SwXTextColumns::setColumnCount(sal_Int16 nColumns)
{
    SolarMutexGuard aGuard;
    if(nColumns <= 0)
        throw uno::RuntimeException();
    bIsAutomaticWidth = true;
    aTextColumns.realloc(nColumns);
     TextColumn* pCols = aTextColumns.getArray();
    nReference = USHRT_MAX;
    sal_Int32 nWidth = nReference / nColumns;
    sal_Int32 nDiff = nReference - nWidth * nColumns;
    sal_Int32 nDist = nAutoDistance / 2;
    for(sal_Int16 i = 0; i < nColumns; i++)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = i == 0 ? 0 : nDist;
        pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
    }
    pCols[nColumns - 1].Width += nDiff;
}

uno::Sequence< TextColumn > SwXTextColumns::getColumns()
{
    SolarMutexGuard aGuard;
    return aTextColumns;
}

void SwXTextColumns::setColumns(const uno::Sequence< TextColumn >& rColumns)
{
    SolarMutexGuard aGuard;
    sal_Int32 nReferenceTemp = 0;
    const TextColumn* prCols = rColumns.getConstArray();
    for(long i = 0; i < rColumns.getLength(); i++)
    {
        nReferenceTemp += prCols[i].Width;
    }
    bIsAutomaticWidth = false;
    nReference = !nReferenceTemp ? USHRT_MAX : nReferenceTemp;
    aTextColumns = rColumns;
}

uno::Reference< XPropertySetInfo > SwXTextColumns::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}

void SwXTextColumns::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
{
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    switch(pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
        {
            sal_Int32 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0)
                throw IllegalArgumentException();
            nSepLineWidth = convertMm100ToTwip(nTmp);
        }
        break;
        case WID_TXTCOL_LINE_COLOR:
            aValue >>= nSepLineColor;
        break;
        case WID_TXTCOL_LINE_STYLE:
        {
            aValue >>= nSepLineStyle;
        }
        break;
        case WID_TXTCOL_LINE_REL_HGT:
        {
            sal_Int8 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0)
                throw IllegalArgumentException();
            nSepLineHeightRelative = nTmp;
        }
        break;
        case WID_TXTCOL_LINE_ALIGN:
        {
            style::VerticalAlignment eAlign;
            if(!(aValue >>= eAlign) )
            {
                sal_Int8 nTmp = 0;
                if (! ( aValue >>= nTmp ) )
                    throw IllegalArgumentException();
                else
                    nSepLineVertAlign = nTmp;
            }
            else
                nSepLineVertAlign = static_cast< sal_Int8 >(eAlign);
        }
        break;
        case WID_TXTCOL_LINE_IS_ON:
            bSepLineIsOn = *o3tl::doAccess<bool>(aValue);
        break;
        case WID_TXTCOL_AUTO_DISTANCE:
        {
            sal_Int32 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0 || nTmp >= nReference)
                throw IllegalArgumentException();
            nAutoDistance = nTmp;
            sal_Int32 nColumns = aTextColumns.getLength();
            TextColumn* pCols = aTextColumns.getArray();
            sal_Int32 nDist = nAutoDistance / 2;
            for(sal_Int32 i = 0; i < nColumns; i++)
            {
                pCols[i].LeftMargin = i == 0 ? 0 : nDist;
                pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
            }
        }
        break;
    }
}

Any SwXTextColumns::getPropertyValue( const OUString& rPropertyName )
{
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    Any aRet;
    switch(pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
            aRet <<= static_cast < sal_Int32 >(convertTwipToMm100(nSepLineWidth));
        break;
        case WID_TXTCOL_LINE_COLOR:
            aRet <<= nSepLineColor;
        break;
        case WID_TXTCOL_LINE_STYLE:
            aRet <<= nSepLineStyle;
        break;
        case WID_TXTCOL_LINE_REL_HGT:
            aRet <<= nSepLineHeightRelative;
        break;
        case WID_TXTCOL_LINE_ALIGN:
            aRet <<= (style::VerticalAlignment)nSepLineVertAlign;
        break;
        case WID_TXTCOL_LINE_IS_ON:
            aRet <<= bSepLineIsOn;
        break;
        case WID_TXTCOL_IS_AUTOMATIC :
            aRet <<= bIsAutomaticWidth;
        break;
        case WID_TXTCOL_AUTO_DISTANCE:
            aRet <<= nAutoDistance;
        break;
    }
    return aRet;
}

void SwXTextColumns::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXTextColumns::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXTextColumns::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
}

void SwXTextColumns::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
{
}

namespace
{
    class theSwXTextColumnsUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextColumnsUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextColumns::getUnoTunnelId()
{
    return theSwXTextColumnsUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXTextColumns::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
