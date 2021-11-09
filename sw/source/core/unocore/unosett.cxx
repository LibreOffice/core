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

#include <editeng/editids.hrc>
#include <swtypes.hxx>
#include <unomid.h>
#include <hintids.hxx>
#include <strings.hrc>
#include <poolfmt.hxx>
#include <fmtcol.hxx>
#include <unomap.hxx>
//#include <unostyle.hxx>
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
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <o3tl/any.hxx>
#include <o3tl/enumarray.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/font.hxx>
#include <editeng/flstitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/graph.hxx>
#include <vcl/GraphicLoader.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/svapp.hxx>
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
#include <svl/itemprop.hxx>
#include <svl/listener.hxx>
#include <paratr.hxx>
#include <sal/log.hxx>
#include <numeric>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;


namespace
{
    SvtBroadcaster& GetPageDescNotifier(SwDoc* pDoc)
    {
        return pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier();
    }
}

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
#define WID_NUM_ON                      12
#define WID_SEPARATOR_INTERVAL          13
#define WID_NUMBER_POSITION             14
#define WID_DISTANCE                    15
#define WID_INTERVAL                    16
#define WID_SEPARATOR_TEXT              17
#define WID_COUNT_EMPTY_LINES           18
#define WID_COUNT_LINES_IN_FRAMES       19
#define WID_RESTART_AT_EACH_PAGE        20


static const SfxItemPropertySet* GetFootnoteSet()
{
    static const SfxItemPropertyMapEntry aFootnoteMap_Impl[] =
    {
        { UNO_NAME_ANCHOR_CHAR_STYLE_NAME,WID_ANCHOR_CHARACTER_STYLE, ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_BEGIN_NOTICE,          WID_BEGIN_NOTICE,       ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_STYLE_NAME,       WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_END_NOTICE,            WID_END_NOTICE ,        ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_FOOTNOTE_COUNTING,     WID_FOOTNOTE_COUNTING,  ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_NUMBERING_TYPE,        WID_NUMBERING_TYPE,     ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { UNO_NAME_PAGE_STYLE_NAME,       WID_PAGE_STYLE,         ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_PARA_STYLE_NAME,       WID_PARAGRAPH_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_POSITION_END_OF_DOC,   WID_POSITION_END_OF_DOC,cppu::UnoType<bool>::get(), PROPERTY_NONE,         0},
        { UNO_NAME_PREFIX,                WID_PREFIX,             ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_START_AT,              WID_START_AT ,          ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { UNO_NAME_SUFFIX,                WID_SUFFIX,             ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aFootnoteSet_Impl(aFootnoteMap_Impl);
    return &aFootnoteSet_Impl;
}

static const SfxItemPropertySet* GetEndnoteSet()
{
    static const SfxItemPropertyMapEntry aEndnoteMap_Impl[] =
    {
        { UNO_NAME_ANCHOR_CHAR_STYLE_NAME,WID_ANCHOR_CHARACTER_STYLE, ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_STYLE_NAME,       WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_NUMBERING_TYPE,        WID_NUMBERING_TYPE,     ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { UNO_NAME_PAGE_STYLE_NAME,       WID_PAGE_STYLE,         ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_PARA_STYLE_NAME,       WID_PARAGRAPH_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_PREFIX,                WID_PREFIX,     ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_START_AT,              WID_START_AT ,          ::cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,         0},
        { UNO_NAME_SUFFIX,                WID_SUFFIX,     ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aEndnoteSet_Impl(aEndnoteMap_Impl);
    return &aEndnoteSet_Impl;
}

static const SfxItemPropertySet* GetNumberingRulesSet()
{
    static const SfxItemPropertyMapEntry aNumberingRulesMap_Impl[] =
    {
        { UNO_NAME_IS_ABSOLUTE_MARGINS,       WID_IS_ABS_MARGINS, cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { UNO_NAME_IS_AUTOMATIC,              WID_IS_AUTOMATIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { UNO_NAME_IS_CONTINUOUS_NUMBERING,   WID_CONTINUOUS,     cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { UNO_NAME_NAME,                      WID_RULE_NAME   ,   ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,     0},
        { UNO_NAME_NUMBERING_IS_OUTLINE,      WID_IS_OUTLINE, cppu::UnoType<bool>::get(),            PROPERTY_NONE,     0},
        { UNO_NAME_DEFAULT_LIST_ID,           WID_DEFAULT_LIST_ID, ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet  aNumberingRulesSet_Impl( aNumberingRulesMap_Impl );
    return &aNumberingRulesSet_Impl;
}

static const SfxItemPropertySet* GetLineNumberingSet()
{
    static const SfxItemPropertyMapEntry aLineNumberingMap_Impl[] =
    {
        { UNO_NAME_CHAR_STYLE_NAME,         WID_CHARACTER_STYLE,    ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_COUNT_EMPTY_LINES,       WID_COUNT_EMPTY_LINES , cppu::UnoType<bool>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_COUNT_LINES_IN_FRAMES,   WID_COUNT_LINES_IN_FRAMES, cppu::UnoType<bool>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_DISTANCE,                WID_DISTANCE       ,    ::cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_IS_ON,                   WID_NUM_ON,             cppu::UnoType<bool>::get()  ,          PROPERTY_NONE,     0},
        { UNO_NAME_INTERVAL,                WID_INTERVAL  ,       ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_SEPARATOR_TEXT,          WID_SEPARATOR_TEXT,   ::cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_NUMBER_POSITION,         WID_NUMBER_POSITION,    ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_NUMBERING_TYPE,          WID_NUMBERING_TYPE ,    ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { UNO_NAME_RESTART_AT_EACH_PAGE,    WID_RESTART_AT_EACH_PAGE, cppu::UnoType<bool>::get()  ,          PROPERTY_NONE,     0},
        { UNO_NAME_SEPARATOR_INTERVAL,      WID_SEPARATOR_INTERVAL, ::cppu::UnoType<sal_Int16>::get(),PROPERTY_NONE,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
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
    SwStyleNameMapper::FillUIName(uTmp, sCharFormat, SwGetPoolIdFromName::ChrFmt);
    if (sCharFormat != SwResId(STR_POOLCHR_STANDARD))
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
    SwStyleNameMapper::FillUIName(uTmp, sParaStyle, SwGetPoolIdFromName::TxtColl );
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
    SwStyleNameMapper::FillUIName(uTmp, sPageDesc, SwGetPoolIdFromName::PageDesc );
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
const o3tl::enumarray<SvxAdjust, unsigned short> aSvxToUnoAdjust
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
    static_cast<unsigned short>(SvxAdjust::Right),       // 1
    static_cast<unsigned short>(SvxAdjust::Center),      // 3
    static_cast<unsigned short>(SvxAdjust::Left),        // 0
    USHRT_MAX,
    USHRT_MAX
};

OUString SwXFootnoteProperties::getImplementationName()
{
    return "SwXFootnoteProperties";
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
    m_pDoc(pDc),
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
    if(!m_pDoc)
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
    if(!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if ( pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SwFootnoteInfo aFootnoteInfo(m_pDoc->GetFootnoteInfo());
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
            if(!(nTmp >= 0 &&
                (nTmp <= SVX_NUM_ARABIC ||
                    nTmp > SVX_NUM_BITMAP)))
                throw lang::IllegalArgumentException();

            aFootnoteInfo.m_aFormat.SetNumberingType(static_cast<SvxNumType>(nTmp));

        }
        break;
        case WID_START_AT:
        {
            sal_Int16 nTmp = 0;
            aValue >>= nTmp;
            aFootnoteInfo.m_nFootnoteOffset = nTmp;
        }
        break;
        case WID_FOOTNOTE_COUNTING:
        {
            sal_Int16 nTmp = 0;
            aValue >>= nTmp;
            switch(nTmp)
            {
                case  FootnoteNumbering::PER_PAGE:
                    aFootnoteInfo.m_eNum = FTNNUM_PAGE;
                break;
                case  FootnoteNumbering::PER_CHAPTER:
                    aFootnoteInfo.m_eNum = FTNNUM_CHAPTER;
                break;
                case  FootnoteNumbering::PER_DOCUMENT:
                    aFootnoteInfo.m_eNum = FTNNUM_DOC;
                break;
            }
        }
        break;
        case WID_PARAGRAPH_STYLE:
        {
            SwTextFormatColl* pColl = lcl_GetParaStyle(m_pDoc, aValue);
            if(pColl)
                aFootnoteInfo.SetFootnoteTextColl(*pColl);
        }
        break;
        case WID_PAGE_STYLE:
        {
            SwPageDesc* pDesc = lcl_GetPageDesc(m_pDoc, aValue);
            if(pDesc)
                aFootnoteInfo.ChgPageDesc( pDesc );
        }
        break;
        case WID_ANCHOR_CHARACTER_STYLE:
        case WID_CHARACTER_STYLE:
        {
            SwCharFormat* pFormat = lcl_getCharFormat(m_pDoc, aValue);
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
            aFootnoteInfo.m_ePos = bVal ? FTNPOS_CHAPTER : FTNPOS_PAGE;
        }
        break;
        case WID_END_NOTICE:
        {
            OUString uTmp;
            aValue >>= uTmp;
            aFootnoteInfo.m_aQuoVadis = uTmp;
        }
        break;
        case WID_BEGIN_NOTICE:
        {
            OUString uTmp;
            aValue >>= uTmp;
            aFootnoteInfo.m_aErgoSum = uTmp;
        }
        break;
    }
    m_pDoc->SetFootnoteInfo(aFootnoteInfo);


}

uno::Any SwXFootnoteProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!m_pDoc)
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
    if(!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SwFootnoteInfo& rFootnoteInfo = m_pDoc->GetFootnoteInfo();
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
            aRet <<= static_cast<sal_Int16>(rFootnoteInfo.m_aFormat.GetNumberingType());
        }
        break;
        case  WID_START_AT:
            aRet <<= static_cast<sal_Int16>(rFootnoteInfo.m_nFootnoteOffset);
        break;
        case  WID_FOOTNOTE_COUNTING  :
        {
            sal_Int16 nRet = 0;
            switch(rFootnoteInfo.m_eNum)
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
            SwStyleNameMapper::FillProgName(aString, aString, SwGetPoolIdFromName::TxtColl);
            aRet <<= aString;
        }
        break;
        case  WID_PAGE_STYLE :
        {
            OUString aString;
            if( rFootnoteInfo.KnowsPageDesc() )
            {
                SwStyleNameMapper::FillProgName(
                        rFootnoteInfo.GetPageDesc( *m_pDoc )->GetName(),
                        aString,
                        SwGetPoolIdFromName::PageDesc);
            }
            aRet <<= aString;
        }
        break;
        case WID_ANCHOR_CHARACTER_STYLE:
        case WID_CHARACTER_STYLE:
        {
            OUString aString;
            const SwCharFormat* pCharFormat = rFootnoteInfo.GetCurrentCharFormat(pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE);
            if( pCharFormat )
            {
                SwStyleNameMapper::FillProgName(
                        pCharFormat->GetName(),
                        aString,
                        SwGetPoolIdFromName::ChrFmt);
            }
            aRet <<= aString;
        }
        break;
        case  WID_POSITION_END_OF_DOC:
            aRet <<= FTNPOS_CHAPTER == rFootnoteInfo.m_ePos;
        break;
        case  WID_END_NOTICE         :
            aRet <<= rFootnoteInfo.m_aQuoVadis;
        break;
        case  WID_BEGIN_NOTICE       :
            aRet <<= rFootnoteInfo.m_aErgoSum;
        break;
    }


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
    return "SwXEndnoteProperties";
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
    m_pDoc(pDc),
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
    if(!m_pDoc)
        return;

    const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
    if(!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if ( pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SwEndNoteInfo aEndInfo(m_pDoc->GetEndNoteInfo());
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
            aEndInfo.m_aFormat.SetNumberingType(static_cast<SvxNumType>(nTmp));
        }
        break;
        case  WID_START_AT:
        {
            sal_Int16 nTmp = 0;
            aValue >>= nTmp;
            aEndInfo.m_nFootnoteOffset = nTmp;
        }
        break;
        case  WID_PARAGRAPH_STYLE    :
        {
            SwTextFormatColl* pColl = lcl_GetParaStyle(m_pDoc, aValue);
            if(pColl)
                aEndInfo.SetFootnoteTextColl(*pColl);
        }
        break;
        case  WID_PAGE_STYLE :
        {
            SwPageDesc* pDesc = lcl_GetPageDesc(m_pDoc, aValue);
            if(pDesc)
                aEndInfo.ChgPageDesc( pDesc );
        }
        break;
        case WID_ANCHOR_CHARACTER_STYLE:
        case  WID_CHARACTER_STYLE    :
        {
            SwCharFormat* pFormat = lcl_getCharFormat(m_pDoc, aValue);
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
    m_pDoc->SetEndNoteInfo(aEndInfo);
}

uno::Any SwXEndnoteProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(m_pDoc)
    {
        const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
        if(!pEntry)
            throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        const SwEndNoteInfo& rEndInfo = m_pDoc->GetEndNoteInfo();
        switch(pEntry->nWID)
        {
            case WID_PREFIX:
                aRet <<= rEndInfo.GetPrefix();
            break;
            case WID_SUFFIX:
                aRet <<= rEndInfo.GetSuffix();
            break;
            case  WID_NUMBERING_TYPE :
                aRet <<= static_cast<sal_Int16>(rEndInfo.m_aFormat.GetNumberingType());
            break;
            case  WID_START_AT:
                aRet <<= static_cast<sal_Int16>(rEndInfo.m_nFootnoteOffset);
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
                        SwGetPoolIdFromName::TxtColl);
                aRet <<= aString;

            }
            break;
            case  WID_PAGE_STYLE :
            {
                OUString aString;
                if( rEndInfo.KnowsPageDesc() )
                {
                    SwStyleNameMapper::FillProgName(
                        rEndInfo.GetPageDesc( *m_pDoc )->GetName(),
                        aString,
                        SwGetPoolIdFromName::PageDesc);
                }
                aRet <<= aString;
            }
            break;
            case WID_ANCHOR_CHARACTER_STYLE:
            case WID_CHARACTER_STYLE:
            {
                OUString aString;
                const SwCharFormat* pCharFormat = rEndInfo.GetCurrentCharFormat( pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE );
                if( pCharFormat )
                {
                    SwStyleNameMapper::FillProgName(
                            pCharFormat->GetName(),
                            aString,
                            SwGetPoolIdFromName::ChrFmt);
                }
                aRet <<= aString;
            }
            break;
        }

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
    return "SwXLineNumberingProperties";
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
    m_pDoc(pDc),
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
    if(!m_pDoc)
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
    if(!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if ( pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SwLineNumberInfo  aFontMetric(m_pDoc->GetLineNumberInfo());
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
            SwCharFormat* pFormat = lcl_getCharFormat(m_pDoc, aValue);
            if(pFormat)
                aFontMetric.SetCharFormat(pFormat);
        }
        break;
        case WID_NUMBERING_TYPE  :
        {
            SvxNumberType aNumType(aFontMetric.GetNumType());
            sal_Int16 nTmp = 0;
            aValue >>= nTmp;
            aNumType.SetNumberingType(static_cast<SvxNumType>(nTmp));
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
                     aFontMetric.SetPos(LINENUMBER_POS_LEFT);
                break;
                case style::LineNumberPosition::RIGHT :
                     aFontMetric.SetPos(LINENUMBER_POS_RIGHT);
                break;
                case  style::LineNumberPosition::INSIDE:
                    aFontMetric.SetPos(LINENUMBER_POS_INSIDE);
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
            sal_Int32 nTmp = o3tl::toTwips(nVal, o3tl::Length::mm100);
            if (nTmp > SAL_MAX_UINT16)
                nTmp = SAL_MAX_UINT16;
            aFontMetric.SetPosFromLeft(nTmp);
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
    m_pDoc->SetLineNumberInfo(aFontMetric);
}

Any SwXLineNumberingProperties::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(!m_pDoc)
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropertySet->getPropertyMap().getByName( rPropertyName );
    if(!pEntry)
        throw UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const SwLineNumberInfo& rInfo = m_pDoc->GetLineNumberInfo();
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
                            rInfo.GetCharFormat(m_pDoc->getIDocumentStylePoolAccess())->GetName(),
                            aString,
                            SwGetPoolIdFromName::ChrFmt);
            }
            aRet <<= aString;
        }
        break;
        case WID_NUMBERING_TYPE  :
            aRet <<= static_cast<sal_Int16>(rInfo.GetNumType().GetNumberingType());
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
            aRet <<= static_cast<sal_Int16>(rInfo.GetCountBy());
        break;
        case WID_SEPARATOR_TEXT  :
            aRet <<= rInfo.GetDivider();
        break;
        case WID_SEPARATOR_INTERVAL:
            aRet <<= static_cast<sal_Int16>(rInfo.GetDividerCountBy());
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

constexpr OUStringLiteral aInvalidStyle = u"__XXX___invalid";

class SwXNumberingRules::Impl
    : public SvtListener
{
    SwXNumberingRules& m_rParent;
    virtual void Notify(const SfxHint&) override;
    public:
        explicit Impl(SwXNumberingRules& rParent) : m_rParent(rParent) {}
};

bool SwXNumberingRules::isInvalidStyle(std::u16string_view rName)
{
    return rName == aInvalidStyle;
}

namespace
{
}

const uno::Sequence< sal_Int8 > & SwXNumberingRules::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXNumberingRulesUnoTunnelId;
    return theSwXNumberingRulesUnoTunnelId.getSeq();
}

// return implementation specific data
sal_Int64 SwXNumberingRules::getSomething( const uno::Sequence< sal_Int8 > & rId )
{
    return comphelper::getSomethingImpl(rId, this);
}

OUString SwXNumberingRules::getImplementationName()
{
    return "SwXNumberingRules";
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
    m_pDoc(doc),
    m_pDocShell(nullptr),
    m_pNumRule(new SwNumRule(rRule)),
    m_pPropertySet(GetNumberingRulesSet()),
    m_bOwnNumRuleCreated(true)
{
    // first organize the document - it is dependent on the set character formats
    // if no format is set, it should work as well
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        SwNumFormat rFormat(m_pNumRule->Get(i));
        SwCharFormat* pCharFormat = rFormat.GetCharFormat();
        if(pCharFormat)
        {
            m_pDoc = pCharFormat->GetDoc();
            break;
        }
    }
    if(m_pDoc)
        m_pImpl->StartListening(GetPageDescNotifier(m_pDoc));
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        m_sNewCharStyleNames[i] = aInvalidStyle;
        m_sNewBulletFontNames[i] = aInvalidStyle;
    }
}

SwXNumberingRules::SwXNumberingRules(SwDocShell& rDocSh) :
    m_pImpl(new SwXNumberingRules::Impl(*this)),
    m_pDoc(nullptr),
    m_pDocShell(&rDocSh),
    m_pNumRule(nullptr),
    m_pPropertySet(GetNumberingRulesSet()),
    m_bOwnNumRuleCreated(false)
{
    m_pImpl->StartListening(GetPageDescNotifier(m_pDocShell->GetDoc()));
}

SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc) :
    m_pImpl(new SwXNumberingRules::Impl(*this)),
    m_pDoc(&rDoc),
    m_pDocShell(nullptr),
    m_pNumRule(nullptr),
    m_pPropertySet(GetNumberingRulesSet()),
    m_bOwnNumRuleCreated(false)
{
    m_pImpl->StartListening(GetPageDescNotifier(&rDoc));
    m_sCreatedNumRuleName = rDoc.GetUniqueNumRuleName();
    rDoc.MakeNumRule( m_sCreatedNumRuleName, nullptr, false,
                      // #i89178#
                      numfunc::GetDefaultPositionAndSpaceMode() );
}

SwXNumberingRules::~SwXNumberingRules()
{
    SolarMutexGuard aGuard;
    if(m_pDoc && !m_sCreatedNumRuleName.isEmpty())
        m_pDoc->DelNumRule( m_sCreatedNumRuleName );
    if( m_bOwnNumRuleCreated )
        delete m_pNumRule;
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
    if(m_pNumRule)
        SwXNumberingRules::SetNumberingRuleByIndex( *m_pNumRule,
                            *rProperties, nIndex);
    else if(m_pDocShell)
    {
        // #i87650# - correction of cws warnings:
        SwNumRule aNumRule( *(m_pDocShell->GetDoc()->GetOutlineNumRule()) );
        SwXNumberingRules::SetNumberingRuleByIndex( aNumRule,
                            *rProperties, nIndex);
        // set character format if needed
        // this code appears to be dead - except when a style is assigned for BITMAP numbering?
        const SwCharFormats* pFormats = m_pDocShell->GetDoc()->GetCharFormats();
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
                    pBase = m_pDocShell->GetStyleSheetPool()->Find(m_sNewCharStyleNames[i],
                                                                    SfxStyleFamily::Char);
                    if(!pBase)
                        pBase = &m_pDocShell->GetStyleSheetPool()->Make(m_sNewCharStyleNames[i], SfxStyleFamily::Char);
                    pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

                }
                aFormat.SetCharFormat( pCharFormat );
                aNumRule.Set( i, aFormat );
            }
        }
        m_pDocShell->GetDoc()->SetOutlineNumRule( aNumRule );
    }
    else if(m_pDoc && !m_sCreatedNumRuleName.isEmpty() &&
        nullptr != (pRule = m_pDoc->FindNumRulePtr( m_sCreatedNumRuleName )))
    {
        SwXNumberingRules::SetNumberingRuleByIndex( *pRule,
                            *rProperties, nIndex);

        pRule->Validate(*m_pDoc);
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
    const SwNumRule* pRule = m_pNumRule;
    if(!pRule && m_pDoc && !m_sCreatedNumRuleName.isEmpty())
        pRule = m_pDoc->FindNumRulePtr( m_sCreatedNumRuleName );
    if(pRule)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
                                        *pRule, nIndex);
        aVal <<= aRet;

    }
    else if(m_pDocShell)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
                *m_pDocShell->GetDoc()->GetOutlineNumRule(), nIndex);
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

const TranslateId STR_POOLCOLL_HEADLINE_ARY[]
{
    STR_POOLCOLL_HEADLINE1,
    STR_POOLCOLL_HEADLINE2,
    STR_POOLCOLL_HEADLINE3,
    STR_POOLCOLL_HEADLINE4,
    STR_POOLCOLL_HEADLINE5,
    STR_POOLCOLL_HEADLINE6,
    STR_POOLCOLL_HEADLINE7,
    STR_POOLCOLL_HEADLINE8,
    STR_POOLCOLL_HEADLINE9,
    STR_POOLCOLL_HEADLINE10
};

uno::Sequence<beans::PropertyValue> SwXNumberingRules::GetNumberingRuleByIndex(
                const SwNumRule& rNumRule, sal_Int32 nIndex) const
{
    SolarMutexGuard aGuard;
    OSL_ENSURE( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    const SwNumFormat& rFormat = rNumRule.Get( o3tl::narrowing<sal_uInt16>(nIndex) );

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
    if (m_pDocShell) // -> Chapter Numbering
    {
        // template name
        OUString sValue(SwResId(STR_POOLCOLL_HEADLINE_ARY[nIndex]));
        const SwTextFormatColls* pColls = m_pDocShell->GetDoc()->GetTextFormatColls();
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
        SwStyleNameMapper::FillProgName(sValue, aUString, SwGetPoolIdFromName::TxtColl);
    }

    OUString referer;
    if (m_pDoc != nullptr) {
        auto const sh = m_pDoc->GetPersist();
        if (sh != nullptr && sh->HasName()) {
            referer = sh->GetMedium()->GetName();
        }
    }
    return GetPropertiesForNumFormat(
        rFormat, CharStyleName, m_pDocShell ? & aUString : nullptr, referer);

}

uno::Sequence<beans::PropertyValue> SwXNumberingRules::GetPropertiesForNumFormat(
        const SwNumFormat& rFormat, OUString const& rCharFormatName,
        OUString const*const pHeadingStyleName, OUString const & referer)
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

    //listformat
    if (rFormat.HasListFormat())
    {
        aPropertyValues.push_back(comphelper::makePropertyValue("ListFormat", rFormat.GetListFormat()));
    }

    //char style name
    aUString.clear();
    SwStyleNameMapper::FillProgName( rCharFormatName, aUString, SwGetPoolIdFromName::ChrFmt);
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
        else if ( rFormat.GetLabelFollowedBy() == SvxNumberFormat::NEWLINE )
        {
            nINT16 = LabelFollow::NEWLINE;
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
            sal_UCS4 cBullet = rFormat.GetBulletChar();

            //BulletId
            nINT16 = cBullet;
            aPropertyValues.push_back(comphelper::makePropertyValue("BulletId", nINT16));

            std::optional<vcl::Font> pFont = rFormat.GetBulletFont();

            //BulletChar
            aUString = OUString(&cBullet, 1);
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
        if (SVX_NUM_BITMAP == rFormat.GetNumberingType())
        {
            const SvxBrushItem* pBrush = rFormat.GetBrush();
            const Graphic* pGraphic = pBrush ? pBrush->GetGraphic(referer) : nullptr;
            if (pGraphic)
            {
                //GraphicBitmap
                uno::Reference<awt::XBitmap> xBitmap(pGraphic->GetXGraphic(), uno::UNO_QUERY);
                aPropertyValues.push_back(comphelper::makePropertyValue(UNO_NAME_GRAPHIC_BITMAP, xBitmap));
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
                aPropertyValues.emplace_back(
                    UNO_NAME_VERT_ORIENT, -1, any, PropertyState_DIRECT_VALUE);
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

void SwXNumberingRules::SetNumberingRuleByIndex(
            SwNumRule& rNumRule,
            const uno::Sequence<beans::PropertyValue>& rProperties, sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    OSL_ENSURE( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    SwNumFormat aFormat(rNumRule.Get( o3tl::narrowing<sal_uInt16>(nIndex) ));

    OUString sHeadingStyleName;
    OUString sParagraphStyleName;

    SetPropertiesToNumFormat(aFormat, m_sNewCharStyleNames[nIndex],
        &m_sNewBulletFontNames[nIndex],
        &sHeadingStyleName, &sParagraphStyleName,
        m_pDoc, m_pDocShell, rProperties);


    if (m_pDoc && !sParagraphStyleName.isEmpty())
    {
        const SwTextFormatColls* pColls = m_pDoc->GetTextFormatColls();
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
        assert(m_pDocShell);
        const SwTextFormatColls* pColls = m_pDocShell->GetDoc()->GetTextFormatColls();
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

    rNumRule.Set(o3tl::narrowing<sal_uInt16>(nIndex), aFormat);
}

void SwXNumberingRules::SetPropertiesToNumFormat(
        SwNumFormat & aFormat,
        OUString & rCharStyleName, OUString *const pBulletFontName,
        OUString *const pHeadingStyleName,
        OUString *const pParagraphStyleName,
        SwDoc *const pDoc,
        SwDocShell *const pDocShell,
        const uno::Sequence<beans::PropertyValue>& rProperties)
{
    assert(pDoc == nullptr || pDocShell == nullptr); // can't be both ordinary and chapter numbering

    bool bWrongArg = false;
    std::unique_ptr<SvxBrushItem> pSetBrush;
    std::unique_ptr<Size> pSetSize;
    std::unique_ptr<SwFormatVertOrient> pSetVOrient;
    bool bCharStyleNameSet = false;

    for (const beans::PropertyValue& rProp : rProperties)
    {
        if (rProp.Name == UNO_NAME_ADJUST)
        {
            sal_Int16 nValue = text::HoriOrientation::NONE;
            rProp.Value >>= nValue;
            if (nValue > text::HoriOrientation::NONE &&
                nValue <= text::HoriOrientation::LEFT &&
                USHRT_MAX != aUnoToSvxAdjust[nValue])
            {
                aFormat.SetNumAdjust(static_cast<SvxAdjust>(aUnoToSvxAdjust[nValue]));
            }
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_PARENT_NUMBERING)
        {
            sal_Int16 nSet = 0;
            rProp.Value >>= nSet;
            if(nSet >= 0 && MAXLEVEL >= nSet)
                aFormat.SetIncludeUpperLevels( static_cast< sal_uInt8 >(nSet) );
        }
        else if (rProp.Name == UNO_NAME_PREFIX)
        {
            OUString uTmp;
            rProp.Value >>= uTmp;
            aFormat.SetPrefix(uTmp);
        }
        else if (rProp.Name == UNO_NAME_SUFFIX)
        {
            OUString uTmp;
            rProp.Value >>= uTmp;
            aFormat.SetSuffix(uTmp);
        }
        else if (rProp.Name == UNO_NAME_CHAR_STYLE_NAME)
        {
            bCharStyleNameSet = true;
            OUString uTmp;
            rProp.Value >>= uTmp;
            OUString sCharFormatName;
            SwStyleNameMapper::FillUIName( uTmp, sCharFormatName, SwGetPoolIdFromName::ChrFmt );
            SwDoc *const pLocalDoc = pDocShell ? pDocShell->GetDoc() : pDoc;
            if (sCharFormatName == UNO_NAME_CHARACTER_FORMAT_NONE)
            {
                rCharStyleName = aInvalidStyle;
                aFormat.SetCharFormat(nullptr);
            }
            else if (pLocalDoc)
            {
                SwCharFormat* pCharFormat = nullptr;
                if (!sCharFormatName.isEmpty())
                {
                    pCharFormat = pLocalDoc->FindCharFormatByName(sCharFormatName);
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
        else if (rProp.Name == UNO_NAME_START_WITH)
        {
            sal_Int16 nVal = 0;
            rProp.Value >>= nVal;
            aFormat.SetStart(nVal);
        }
        else if (rProp.Name == UNO_NAME_LEFT_MARGIN)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            // #i23727# nValue can be negative
            aFormat.SetAbsLSpace(o3tl::toTwips(nValue, o3tl::Length::mm100));
        }
        else if (rProp.Name == UNO_NAME_SYMBOL_TEXT_DISTANCE)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            if (nValue >= 0)
                aFormat.SetCharTextDistance(o3tl::toTwips(nValue, o3tl::Length::mm100));
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_FIRST_LINE_OFFSET)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            // #i23727# nValue can be positive
            nValue = o3tl::toTwips(nValue, o3tl::Length::mm100);
            aFormat.SetFirstLineOffset(nValue);
        }
        else if (rProp.Name == UNO_NAME_POSITION_AND_SPACE_MODE)
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
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
        else if (rProp.Name == UNO_NAME_LABEL_FOLLOWED_BY)
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            if ( nValue == LabelFollow::LISTTAB )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }
            else if ( nValue == LabelFollow::SPACE )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::SPACE );
            }
            else if ( nValue == LabelFollow::NOTHING )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::NOTHING );
            }
            else if ( nValue == LabelFollow::NEWLINE )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::NEWLINE );
            }
            else
            {
                bWrongArg = true;
            }
        }
        else if (rProp.Name == UNO_NAME_LISTTAB_STOP_POSITION)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            nValue = o3tl::toTwips(nValue, o3tl::Length::mm100);
            if ( nValue >= 0 )
            {
                aFormat.SetListtabPos( nValue );
            }
            else
            {
                bWrongArg = true;
            }
        }
        else if (rProp.Name == UNO_NAME_FIRST_LINE_INDENT)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            nValue = o3tl::toTwips(nValue, o3tl::Length::mm100);
            aFormat.SetFirstLineIndent( nValue );
        }
        else if (rProp.Name == UNO_NAME_INDENT_AT)
        {
            sal_Int32 nValue = 0;
            rProp.Value >>= nValue;
            nValue = o3tl::toTwips(nValue, o3tl::Length::mm100);
            aFormat.SetIndentAt( nValue );
        }
        else if (rProp.Name == UNO_NAME_NUMBERING_TYPE)
        {
            sal_Int16 nSet = 0;
            rProp.Value >>= nSet;
            if(nSet >= 0)
                aFormat.SetNumberingType(static_cast<SvxNumType>(nSet));
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_PARAGRAPH_STYLE_NAME)
        {
            if (pParagraphStyleName)
            {
                OUString uTmp;
                rProp.Value >>= uTmp;
                OUString sStyleName;
                SwStyleNameMapper::FillUIName(uTmp, sStyleName, SwGetPoolIdFromName::TxtColl );
                *pParagraphStyleName = sStyleName;
            }
        }
        else if (rProp.Name == UNO_NAME_BULLET_ID)
        {
            sal_Int16 nSet = 0;
            if( rProp.Value >>= nSet )
                aFormat.SetBulletChar(nSet);
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_BULLET_FONT)
        {
            awt::FontDescriptor desc;
            if (rProp.Value >>= desc)
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
        else if (rProp.Name == UNO_NAME_BULLET_FONT_NAME)
        {
            OUString sBulletFontName;
            rProp.Value >>= sBulletFontName;
            SwDocShell *const pLclDocShell = pDocShell ? pDocShell : pDoc ? pDoc->GetDocShell() : nullptr;
            if (!sBulletFontName.isEmpty() && pLclDocShell)
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
        else if (rProp.Name == UNO_NAME_BULLET_CHAR)
        {
            OUString aChar;
            rProp.Value >>= aChar;
            if (aChar.isEmpty())
            {
                // If w:lvlText's value is null - set bullet char to zero
                aFormat.SetBulletChar(u'\0');
            }
            else
            {
                sal_Int32 nIndexUtf16 = 0;
                sal_UCS4 cBullet = aChar.iterateCodePoints(&nIndexUtf16);
                if (aChar.getLength() == nIndexUtf16)
                    aFormat.SetBulletChar(cBullet);
                else
                    bWrongArg = true;
            }
        }
        else if (rProp.Name == UNO_NAME_GRAPHIC)
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            if (rProp.Value >>= xGraphic)
            {
                if (!pSetBrush)
                {
                    const SvxBrushItem* pOrigBrush = aFormat.GetBrush();
                    if(pOrigBrush)
                        pSetBrush.reset(new SvxBrushItem(*pOrigBrush));
                    else
                        pSetBrush.reset(new SvxBrushItem(OUString(), OUString(), GPOS_AREA, RES_BACKGROUND));
                }
                Graphic aGraphic(xGraphic);
                pSetBrush->SetGraphic(aGraphic);
            }
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_GRAPHIC_BITMAP)
        {
            uno::Reference<awt::XBitmap> xBitmap;
            if (rProp.Value >>= xBitmap)
            {
                if(!pSetBrush)
                {
                    const SvxBrushItem* pOrigBrush = aFormat.GetBrush();
                    if(pOrigBrush)
                        pSetBrush.reset(new SvxBrushItem(*pOrigBrush));
                    else
                        pSetBrush.reset(new SvxBrushItem(OUString(), OUString(), GPOS_AREA, RES_BACKGROUND));
                }

                uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
                Graphic aGraphic(xGraphic);
                pSetBrush->SetGraphic(aGraphic);
            }
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_GRAPHIC_SIZE)
        {
            if(!pSetSize)
                pSetSize.reset(new Size);
            awt::Size size;
            if (rProp.Value >>= size)
            {
                pSetSize->setWidth(o3tl::toTwips(size.Width, o3tl::Length::mm100));
                pSetSize->setHeight(o3tl::toTwips(size.Height, o3tl::Length::mm100));
            }
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_VERT_ORIENT)
        {
            if(!pSetVOrient)
            {
                if(aFormat.GetGraphicOrientation())
                    pSetVOrient.reset(aFormat.GetGraphicOrientation()->Clone());
                else
                    pSetVOrient.reset(new SwFormatVertOrient);
            }
            pSetVOrient->PutValue(rProp.Value, MID_VERTORIENT_ORIENT);
        }
        else if (rProp.Name == UNO_NAME_HEADING_STYLE_NAME
                && pDocShell) // only on chapter numbering
        {
            if (pHeadingStyleName)
            {
                OUString uTmp;
                rProp.Value >>= uTmp;
                OUString sStyleName;
                SwStyleNameMapper::FillUIName(uTmp, sStyleName, SwGetPoolIdFromName::TxtColl );
                *pHeadingStyleName = sStyleName;
            }
        }
        else if (rProp.Name == UNO_NAME_BULLET_REL_SIZE)
        {
            // BulletRelSize - unsupported - only available in Impress
        }
        else if (rProp.Name == UNO_NAME_BULLET_COLOR)
        {
            // BulletColor - ignored too
        }
        else if (rProp.Name == UNO_NAME_GRAPHIC_URL)
        {
            OUString aURL;
            if (rProp.Value >>= aURL)
            {
                if(!pSetBrush)
                {
                    const SvxBrushItem* pOrigBrush = aFormat.GetBrush();
                    if(pOrigBrush)
                        pSetBrush.reset(new SvxBrushItem(*pOrigBrush));
                    else
                        pSetBrush.reset(new SvxBrushItem(OUString(), OUString(), GPOS_AREA, RES_BACKGROUND));
                }

                Graphic aGraphic = vcl::graphic::loadFromURL(aURL);
                if (!aGraphic.IsNone())
                    pSetBrush->SetGraphic(aGraphic);
            }
            else
                bWrongArg = true;
        }
        else if (rProp.Name == UNO_NAME_LIST_FORMAT)
        {
            OUString uTmp;
            rProp.Value >>= uTmp;
            aFormat.SetListFormat(uTmp);
        }
        else
        {
            // Invalid property name
            SAL_WARN("sw.uno", "Unknown/incorrect property " << rProp.Name << ", failing");
            throw uno::RuntimeException("Unknown/incorrect property " + rProp.Name);
        }
    }
    if(!bWrongArg && (pSetBrush || pSetSize || pSetVOrient))
    {
        if(!pSetBrush && aFormat.GetBrush())
            pSetBrush.reset(new SvxBrushItem(*aFormat.GetBrush()));

        if(pSetBrush)
        {
            if(!pSetVOrient && aFormat.GetGraphicOrientation())
                pSetVOrient.reset( new SwFormatVertOrient(*aFormat.GetGraphicOrientation()) );

            if(!pSetSize)
            {
                pSetSize.reset(new Size(aFormat.GetGraphicSize()));
                if(!pSetSize->Width() || !pSetSize->Height())
                {
                    const Graphic* pGraphic = pSetBrush->GetGraphic();
                    if(pGraphic)
                        *pSetSize = ::GetGraphicSizeTwip(*pGraphic, nullptr);
                }
            }
            sal_Int16 eOrient = pSetVOrient ?
                pSetVOrient->GetVertOrient() : text::VertOrientation::NONE;
            aFormat.SetGraphicBrush( pSetBrush.get(), pSetSize.get(), text::VertOrientation::NONE == eOrient ? nullptr : &eOrient );
        }
    }
    if ((!bCharStyleNameSet || rCharStyleName.isEmpty())
        && aFormat.GetNumberingType() == NumberingType::BITMAP
        && !aFormat.GetCharFormat()
        && !SwXNumberingRules::isInvalidStyle(rCharStyleName))
    {
        OUString tmp;
        SwStyleNameMapper::FillProgName(RES_POOLCHR_BULLET_LEVEL, tmp);
        rCharStyleName = tmp;
    }

    if(bWrongArg)
        throw lang::IllegalArgumentException();
}

uno::Reference< XPropertySetInfo > SwXNumberingRules::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
    return aRef;
}

void SwXNumberingRules::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwNumRule> pDocRule;
    SwNumRule* pCreatedRule = nullptr;
    if(!m_pNumRule)
    {
        if(m_pDocShell)
        {
            pDocRule.reset(new SwNumRule(*m_pDocShell->GetDoc()->GetOutlineNumRule()));
        }
        else if(m_pDoc && !m_sCreatedNumRuleName.isEmpty())
        {
            pCreatedRule = m_pDoc->FindNumRulePtr(m_sCreatedNumRuleName);
        }

    }
    if(!m_pNumRule && !pDocRule && !pCreatedRule)
        throw RuntimeException();

    if(rPropertyName == UNO_NAME_IS_AUTOMATIC)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        if(!pCreatedRule)
            pDocRule ? pDocRule->SetAutoRule(bVal) : m_pNumRule->SetAutoRule(bVal);
    }
    else if(rPropertyName == UNO_NAME_IS_CONTINUOUS_NUMBERING)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        pDocRule ? pDocRule->SetContinusNum(bVal) :
            pCreatedRule ? pCreatedRule->SetContinusNum(bVal) : m_pNumRule->SetContinusNum(bVal);
    }
    else if(rPropertyName == UNO_NAME_NAME)
    {
        throw IllegalArgumentException();
    }
    else if(rPropertyName == UNO_NAME_IS_ABSOLUTE_MARGINS)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        pDocRule ? pDocRule->SetAbsSpaces(bVal) :
            pCreatedRule ? pCreatedRule->SetAbsSpaces(bVal) : m_pNumRule->SetAbsSpaces(bVal);
    }
    else if(rPropertyName == UNO_NAME_NUMBERING_IS_OUTLINE)
    {
        bool bVal = *o3tl::doAccess<bool>(rValue);
        SwNumRuleType eNumRuleType = bVal ? OUTLINE_RULE : NUM_RULE;
        pDocRule ? pDocRule->SetRuleType(eNumRuleType) :
            pCreatedRule ? pCreatedRule->SetRuleType(eNumRuleType) : m_pNumRule->SetRuleType(eNumRuleType);
    }
    else if(rPropertyName == UNO_NAME_DEFAULT_LIST_ID)
    {
        throw IllegalArgumentException();
    }
    else
        throw UnknownPropertyException(rPropertyName);

    if(pDocRule)
    {
        assert(m_pDocShell);
        m_pDocShell->GetDoc()->SetOutlineNumRule(*pDocRule);
        pDocRule.reset();
    }
    else if(pCreatedRule)
    {
        pCreatedRule->Validate(*m_pDoc);
    }
}

Any SwXNumberingRules::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;
    const SwNumRule* pRule = m_pNumRule;
    if(!pRule && m_pDocShell)
        pRule = m_pDocShell->GetDoc()->GetOutlineNumRule();
    else if(m_pDoc && !m_sCreatedNumRuleName.isEmpty())
        pRule = m_pDoc->FindNumRulePtr( m_sCreatedNumRuleName );
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
        throw UnknownPropertyException(rPropertyName);
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
    if(m_pNumRule)
    {
        OUString aString;
        SwStyleNameMapper::FillProgName(m_pNumRule->GetName(), aString, SwGetPoolIdFromName::NumRule );
        return aString;
    }
    // consider chapter numbering <SwXNumberingRules>
    if ( m_pDocShell )
    {
        OUString aString;
        SwStyleNameMapper::FillProgName( m_pDocShell->GetDoc()->GetOutlineNumRule()->GetName(),
                                         aString, SwGetPoolIdFromName::NumRule );
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

void SwXNumberingRules::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        if(m_rParent.m_bOwnNumRuleCreated)
            delete m_rParent.m_pNumRule;
        m_rParent.m_pNumRule = nullptr;
        m_rParent.m_pDoc = nullptr;
    }
}

OUString SwXChapterNumbering::getImplementationName()
{
    return "SwXChapterNumbering";
}

sal_Bool SwXChapterNumbering::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXChapterNumbering::getSupportedServiceNames()
{
    return { "com.sun.star.text.ChapterNumbering", "com.sun.star.text.NumberingRules" };
}

SwXChapterNumbering::SwXChapterNumbering(SwDocShell& rDocSh) :
    SwXNumberingRules(rDocSh)
{
}

SwXChapterNumbering::~SwXChapterNumbering()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
