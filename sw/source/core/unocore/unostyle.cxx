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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/propertysequence.hxx>
#include <hintids.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <svl/hint.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <svl/listener.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svx/pageitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/wghtitem.hxx>

#include <autostyle_helper.hxx>
#include <pagedesc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <cmdid.h>
#include <unomid.h>
#include <unomap.hxx>
#include <unostyle.hxx>
#include <unosett.hxx>
#include <docsh.hxx>
#include <paratr.hxx>
#include <unoprnms.hxx>
#include <shellio.hxx>
#include <docstyle.hxx>
#include <unotextbodyhf.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <strings.hrc>
#include <poolfmt.hxx>
#include <unoevent.hxx>
#include <fmtruby.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <istyleaccess.hxx>
#include <fmtfsize.hxx>
#include <numrule.hxx>
#include <tblafmt.hxx>
#include <frameformats.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

#include <svl/stylepool.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/memberids.h>
#include <svx/unomid.hxx>
#include <svx/unoshape.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <swunohelper.hxx>
#include <svx/xbtmpit.hxx>

#include <ccoll.hxx>
#include <hints.hxx>
#include <uiitems.hxx>
#include <unoxstyle.hxx>

#include <cassert>
#include <memory>
#include <set>
#include <string_view>
#include <limits>

using namespace css;
using namespace css::io;
using namespace css::lang;
using namespace css::uno;

namespace {

    // these should really be constexprs, but MSVC still is apparently too stupid for them
    #define nPoolChrNormalRange (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN)
    #define nPoolChrHtmlRange   (RES_POOLCHR_HTML_END   - RES_POOLCHR_HTML_BEGIN)
    #define nPoolCollTextRange     ( RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN)
    #define nPoolCollListsRange    ( RES_POOLCOLL_LISTS_END    - RES_POOLCOLL_LISTS_BEGIN)
    #define nPoolCollExtraRange    ( RES_POOLCOLL_EXTRA_END    - RES_POOLCOLL_EXTRA_BEGIN)
    #define nPoolCollRegisterRange ( RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN)
    #define nPoolCollDocRange      ( RES_POOLCOLL_DOC_END      - RES_POOLCOLL_DOC_BEGIN)
    #define nPoolCollHtmlRange     ( RES_POOLCOLL_HTML_END     - RES_POOLCOLL_HTML_BEGIN)
    #define nPoolFrameRange ( RES_POOLFRM_END - RES_POOLFRM_BEGIN)
    #define nPoolPageRange  ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN)
    #define nPoolNumRange   ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN)
    #define nPoolCollListsStackedStart    ( nPoolCollTextRange)
    #define nPoolCollExtraStackedStart    ( nPoolCollListsStackedStart    + nPoolCollListsRange)
    #define nPoolCollRegisterStackedStart ( nPoolCollExtraStackedStart    + nPoolCollExtraRange)
    #define nPoolCollDocStackedStart      ( nPoolCollRegisterStackedStart + nPoolCollRegisterRange)
    #define nPoolCollHtmlStackedStart     ( nPoolCollDocStackedStart      + nPoolCollDocRange)
    using paragraphstyle_t = std::remove_const<decltype(style::ParagraphStyleCategory::TEXT)>::type;
    using collectionbits_t = sal_uInt16;
    struct ParagraphStyleCategoryEntry
    {
        paragraphstyle_t m_eCategory;
        SfxStyleSearchBits m_nSwStyleBits;
        collectionbits_t m_nCollectionBits;
        constexpr ParagraphStyleCategoryEntry(paragraphstyle_t eCategory, SfxStyleSearchBits nSwStyleBits, collectionbits_t nCollectionBits)
                : m_eCategory(eCategory)
                , m_nSwStyleBits(nSwStyleBits)
                , m_nCollectionBits(nCollectionBits)
            { }
    };

constexpr ParagraphStyleCategoryEntry sParagraphStyleCategoryEntries[]
{
        { style::ParagraphStyleCategory::TEXT,    SfxStyleSearchBits::SwText,    COLL_TEXT_BITS     },
        { style::ParagraphStyleCategory::CHAPTER, SfxStyleSearchBits::SwChapter, COLL_DOC_BITS      },
        { style::ParagraphStyleCategory::LIST,    SfxStyleSearchBits::SwList,    COLL_LISTS_BITS    },
        { style::ParagraphStyleCategory::INDEX,   SfxStyleSearchBits::SwIndex,   COLL_REGISTER_BITS },
        { style::ParagraphStyleCategory::EXTRA,   SfxStyleSearchBits::SwExtra,   COLL_EXTRA_BITS    },
        { style::ParagraphStyleCategory::HTML,    SfxStyleSearchBits::SwHtml,    COLL_HTML_BITS     }
};

} // namespace anonymous

class StyleFamilyEntry
{
public:
    template <SfxStyleFamily f> static StyleFamilyEntry Create(sal_uInt16 nPropMapType, SwGetPoolIdFromName aPoolId, OUString sName, TranslateId pResId)
    {
        return StyleFamilyEntry(f, nPropMapType, aPoolId, sName, pResId, GetCountOrName<f>, TranslateIndex<f>);
    }

    SfxStyleFamily family() const { return m_eFamily; }
    sal_uInt16 propMapType() const { return m_nPropMapType; }
    const uno::Reference<beans::XPropertySetInfo>& xPSInfo() const { return m_xPSInfo; }
    SwGetPoolIdFromName poolId() const { return m_aPoolId; }
    const OUString& name() const { return m_sName; }
    const TranslateId& resId() const { return m_pResId; }

    sal_Int32 getCountOrName(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex) const { return m_fGetCountOrName(rDoc, pString, nIndex); }
    sal_uInt16 translateIndex(const sal_uInt16 nIndex) const { return m_fTranslateIndex(nIndex); }

private:
    using GetCountOrName_t = sal_Int32 (*)(const SwDoc&, OUString*, sal_Int32);
    using TranslateIndex_t = sal_uInt16(*)(const sal_uInt16);
    SfxStyleFamily m_eFamily;
    sal_uInt16 m_nPropMapType;
    uno::Reference<beans::XPropertySetInfo> m_xPSInfo;
    SwGetPoolIdFromName m_aPoolId;
    OUString m_sName;
    TranslateId m_pResId;
    GetCountOrName_t m_fGetCountOrName;
    TranslateIndex_t m_fTranslateIndex;
    StyleFamilyEntry(SfxStyleFamily eFamily, sal_uInt16 nPropMapType, SwGetPoolIdFromName aPoolId, OUString sName, TranslateId pResId, GetCountOrName_t fGetCountOrName, TranslateIndex_t fTranslateIndex)
            : m_eFamily(eFamily)
            , m_nPropMapType(nPropMapType)
            , m_xPSInfo(aSwMapProvider.GetPropertySet(nPropMapType)->getPropertySetInfo())
            , m_aPoolId(aPoolId)
            , m_sName(std::move(sName))
            , m_pResId(pResId)
            , m_fGetCountOrName(fGetCountOrName)
            , m_fTranslateIndex(fTranslateIndex)
        { }
    template<SfxStyleFamily> static inline sal_Int32 GetCountOrName(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex);
    template<SfxStyleFamily> static inline sal_uInt16 TranslateIndex(const sal_uInt16 nIndex) { return nIndex; }
};

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Char>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    const sal_uInt16 nBaseCount = nPoolChrHtmlRange + nPoolChrNormalRange;
    nIndex -= nBaseCount;
    sal_Int32 nCount = 0;
    for(auto pFormat : *rDoc.GetCharFormats())
    {
        if(pFormat->IsDefault() && pFormat != rDoc.GetDfltCharFormat())
            continue;
        if(!IsPoolUserFormat(pFormat->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            // the default character format needs to be set to "Default!"
            if(rDoc.GetDfltCharFormat() == pFormat)
                *pString = SwResId(STR_POOLCHR_STANDARD);
            else
                *pString = pFormat->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nBaseCount;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Para>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    const sal_uInt16 nBaseCount = nPoolCollHtmlStackedStart + nPoolCollHtmlRange;
    nIndex -= nBaseCount;
    sal_Int32 nCount = 0;
    for(auto pColl : *rDoc.GetTextFormatColls())
    {
        if(pColl->IsDefault())
            continue;
        if(!IsPoolUserFormat(pColl->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pColl->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nBaseCount;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Frame>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolFrameRange;
    sal_Int32 nCount = 0;
    for(const auto pFormat : *rDoc.GetFrameFormats())
    {
        if(pFormat->IsDefault() || pFormat->IsAuto())
            continue;
        if(!IsPoolUserFormat(pFormat->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pFormat->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nPoolFrameRange;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Page>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolPageRange;
    sal_Int32 nCount = 0;
    const size_t nArrLen = rDoc.GetPageDescCnt();
    for(size_t i = 0; i < nArrLen; ++i)
    {
        const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
        if(!IsPoolUserFormat(rDesc.GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = rDesc.GetName();
            break;
        }
        ++nCount;
    }
    nCount += nPoolPageRange;
    return nCount;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Pseudo>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolNumRange;
    sal_Int32 nCount = 0;
    for(const auto pRule : rDoc.GetNumRuleTable())
    {
        if(pRule->IsAutoRule())
            continue;
        if(!IsPoolUserFormat(pRule->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pRule->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nPoolNumRange;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Table>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    if (!rDoc.HasTableStyles())
        return 0;

    const auto pAutoFormats = &rDoc.GetTableStyles();
    const sal_Int32 nCount = pAutoFormats->size();
    if (0 <= nIndex && nIndex < nCount)
        *pString = pAutoFormats->operator[](nIndex).GetName();

    return nCount;
}

template<>
sal_Int32 StyleFamilyEntry::GetCountOrName<SfxStyleFamily::Cell>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    const auto& rAutoFormats = rDoc.GetTableStyles();
    const auto& rTableTemplateMap = SwTableAutoFormat::GetTableTemplateMap();
    const sal_Int32 nUsedCellStylesCount = rAutoFormats.size() * rTableTemplateMap.size();
    const sal_Int32 nCount = nUsedCellStylesCount + rDoc.GetCellStyles().size();
    if (0 <= nIndex && nIndex < nCount)
    {
        if (nUsedCellStylesCount > nIndex)
        {
            const sal_Int32 nAutoFormat = nIndex / rTableTemplateMap.size();
            const sal_Int32 nBoxFormat = rTableTemplateMap[nIndex % rTableTemplateMap.size()];
            const SwTableAutoFormat& rTableFormat = rAutoFormats[nAutoFormat];
            SwStyleNameMapper::FillProgName(rTableFormat.GetName(), *pString, SwGetPoolIdFromName::TabStyle);
            *pString += rTableFormat.GetTableTemplateCellSubName(rTableFormat.GetBoxFormat(nBoxFormat));
        }
        else
            *pString = rDoc.GetCellStyles()[nIndex-nUsedCellStylesCount].GetName();
    }
    return nCount;
}

template<>
sal_uInt16 StyleFamilyEntry::TranslateIndex<SfxStyleFamily::Char>(const sal_uInt16 nIndex)
{
    static_assert(nPoolChrNormalRange > 0 && nPoolChrHtmlRange > 0, "invalid pool range");
    if (nIndex < nPoolChrNormalRange)
        return nIndex + RES_POOLCHR_NORMAL_BEGIN;
    else if (nIndex < (nPoolChrHtmlRange + nPoolChrNormalRange))
        return nIndex + RES_POOLCHR_HTML_BEGIN - nPoolChrNormalRange;
    throw lang::IndexOutOfBoundsException();
}

template<>
sal_uInt16 StyleFamilyEntry::TranslateIndex<SfxStyleFamily::Para>(const sal_uInt16 nIndex)
{
    static_assert(nPoolCollTextRange > 0 && nPoolCollListsRange > 0 && nPoolCollExtraRange > 0 && nPoolCollRegisterRange > 0 && nPoolCollDocRange > 0 && nPoolCollHtmlRange > 0, "weird pool range");
    if (nIndex < nPoolCollListsStackedStart)
        return nIndex + RES_POOLCOLL_TEXT_BEGIN;
    else if (nIndex < nPoolCollExtraStackedStart)
        return nIndex + RES_POOLCOLL_LISTS_BEGIN - nPoolCollListsStackedStart;
    else if (nIndex < nPoolCollRegisterStackedStart)
        return nIndex + RES_POOLCOLL_EXTRA_BEGIN - nPoolCollExtraStackedStart;
    else if (nIndex < nPoolCollDocStackedStart)
        return nIndex + RES_POOLCOLL_REGISTER_BEGIN - nPoolCollRegisterStackedStart;
    else if (nIndex < nPoolCollHtmlStackedStart)
        return nIndex + RES_POOLCOLL_DOC_BEGIN - nPoolCollDocStackedStart;
    else if (nIndex < nPoolCollHtmlStackedStart + nPoolCollTextRange)
        return nIndex + RES_POOLCOLL_HTML_BEGIN - nPoolCollHtmlStackedStart;
    throw lang::IndexOutOfBoundsException();
}

template<>
sal_uInt16 StyleFamilyEntry::TranslateIndex<SfxStyleFamily::Page>(const sal_uInt16 nIndex)
{
    if (nIndex < nPoolPageRange)
        return nIndex + RES_POOLPAGE_BEGIN;
    throw lang::IndexOutOfBoundsException();
}

template<>
sal_uInt16 StyleFamilyEntry::TranslateIndex<SfxStyleFamily::Frame>(const sal_uInt16 nIndex)
{
    if (nIndex < nPoolFrameRange)
        return nIndex + RES_POOLFRM_BEGIN;
    throw lang::IndexOutOfBoundsException();
}

template<>
sal_uInt16 StyleFamilyEntry::TranslateIndex<SfxStyleFamily::Pseudo>(const sal_uInt16 nIndex)
{
    if (nIndex < nPoolNumRange)
        return nIndex + RES_POOLNUMRULE_BEGIN;
    throw lang::IndexOutOfBoundsException();
}

static const std::vector<StyleFamilyEntry>& lcl_GetStyleFamilyEntries()
{
    static const std::vector<StyleFamilyEntry> our_pStyleFamilyEntries{
        StyleFamilyEntry::Create<SfxStyleFamily::Char>  (PROPERTY_MAP_CHAR_STYLE,  SwGetPoolIdFromName::ChrFmt,    "CharacterStyles", STR_STYLE_FAMILY_CHARACTER),
        StyleFamilyEntry::Create<SfxStyleFamily::Para>  (PROPERTY_MAP_PARA_STYLE,  SwGetPoolIdFromName::TxtColl,   "ParagraphStyles", STR_STYLE_FAMILY_PARAGRAPH),
        StyleFamilyEntry::Create<SfxStyleFamily::Page>  (PROPERTY_MAP_PAGE_STYLE,  SwGetPoolIdFromName::PageDesc,  "PageStyles",      STR_STYLE_FAMILY_PAGE),
        StyleFamilyEntry::Create<SfxStyleFamily::Frame> (PROPERTY_MAP_FRAME_STYLE, SwGetPoolIdFromName::FrmFmt,    "FrameStyles",     STR_STYLE_FAMILY_FRAME),
        StyleFamilyEntry::Create<SfxStyleFamily::Pseudo>(PROPERTY_MAP_NUM_STYLE,   SwGetPoolIdFromName::NumRule,   "NumberingStyles", STR_STYLE_FAMILY_NUMBERING),
        StyleFamilyEntry::Create<SfxStyleFamily::Table> (PROPERTY_MAP_TABLE_STYLE, SwGetPoolIdFromName::TabStyle,  "TableStyles",     STR_STYLE_FAMILY_TABLE),
        StyleFamilyEntry::Create<SfxStyleFamily::Cell>  (PROPERTY_MAP_CELL_STYLE,  SwGetPoolIdFromName::CellStyle, "CellStyles",      STR_STYLE_FAMILY_CELL),
    };
    return our_pStyleFamilyEntries;
}

class SwStyleBase_Impl
{
private:
    SwDoc& m_rDoc;
    const SwPageDesc* m_pOldPageDesc;
    rtl::Reference<SwDocStyleSheet> m_xNewBase;
    SfxItemSet* m_pItemSet;
    std::unique_ptr<SfxItemSet> m_pMyItemSet;
    OUString m_rStyleName;
    const SwAttrSet* m_pParentStyle;
public:
    SwStyleBase_Impl(SwDoc& rSwDoc, OUString aName, const SwAttrSet* pParentStyle)
        : m_rDoc(rSwDoc)
        , m_pOldPageDesc(nullptr)
        , m_pItemSet(nullptr)
        , m_rStyleName(std::move(aName))
        , m_pParentStyle(pParentStyle)
    { }

    rtl::Reference<SwDocStyleSheet>& getNewBase()
    {
        return m_xNewBase;
    }

    void setNewBase(SwDocStyleSheet* pNew)
    {
        m_xNewBase = pNew;
    }

    bool HasItemSet() const
    {
        return m_xNewBase.is();
    }

    SfxItemSet& GetItemSet()
    {
        assert(m_xNewBase.is());
        if(!m_pItemSet)
        {
            m_pMyItemSet.reset(new SfxItemSet(m_xNewBase->GetItemSet()));
            m_pItemSet = m_pMyItemSet.get();

            // set parent style to have the correct XFillStyle setting as XFILL_NONE
            if(!m_pItemSet->GetParent() && m_pParentStyle)
                m_pItemSet->SetParent(m_pParentStyle);
        }
        return *m_pItemSet;
    }

    const SwPageDesc* GetOldPageDesc();

    // still a hack, but a bit more explicit and with a proper scope
    struct ItemSetOverrider
    {
        SwStyleBase_Impl& m_rStyleBase;
        SfxItemSet* m_pOldSet;
        ItemSetOverrider(SwStyleBase_Impl& rStyleBase, SfxItemSet* pTemp)
                : m_rStyleBase(rStyleBase)
                , m_pOldSet(m_rStyleBase.m_pItemSet)
        { m_rStyleBase.m_pItemSet = pTemp; }
        ~ItemSetOverrider()
        { m_rStyleBase.m_pItemSet = m_pOldSet; };
    };
};


SfxStyleFamily SwXStyle::GetFamily() const
{return m_rEntry.family();}

OUString SwXStyle::getImplementationName()
{ return {"SwXStyle"}; };

sal_Bool SwXStyle::supportsService(const OUString& rServiceName)
{ return cppu::supportsService(this, rServiceName); };



class SwStyleProperties_Impl
{
    const SfxItemPropertyMap& mrMap;
    std::map<OUString, uno::Any> m_vPropertyValues;
public:
    explicit SwStyleProperties_Impl(const SfxItemPropertyMap& rMap)
        : mrMap(rMap)
    { }

    bool AllowsKey(std::u16string_view rName)
    {
        return mrMap.hasPropertyByName(rName);
    }
    bool SetProperty(const OUString& rName, const uno::Any& rValue)
    {
        if(!AllowsKey(rName))
            return false;
        m_vPropertyValues[rName] = rValue;
        return true;
    }
    void GetProperty(const OUString& rName, const uno::Any*& pAny)
    {
        if(!AllowsKey(rName))
        {
            pAny = nullptr;
            return;
        }
        pAny = &m_vPropertyValues[rName];
        return;
    }
    bool ClearProperty( const OUString& rName )
    {
        if(!AllowsKey(rName))
            return false;
        m_vPropertyValues[rName] = uno::Any();
        return true;
    }
    void ClearAllProperties( )
            { m_vPropertyValues.clear(); }
    void Apply(SwXStyle& rStyle)
    {
        for(const auto& rPropertyPair : m_vPropertyValues)
        {
            if(rPropertyPair.second.hasValue())
                rStyle.setPropertyValue(rPropertyPair.first, rPropertyPair.second);
        }
    }
};

static rtl::Reference<SwXStyle> CreateStyleCharOrParaOrPseudo(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName, SfxStyleFamily eFamily)
{
    return pBasePool ? new SwXStyle(pBasePool, eFamily, pDocShell->GetDoc(), sStyleName) : new SwXStyle(pDocShell->GetDoc(), eFamily, false);
}

static rtl::Reference<SwXFrameStyle> CreateStyleFrame(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
{
    return pBasePool ? new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), sStyleName) : new SwXFrameStyle(pDocShell->GetDoc());
}

static rtl::Reference<SwXPageStyle> CreateStylePage(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
{
    return pBasePool ? new SwXPageStyle(*pBasePool, pDocShell, sStyleName) : new SwXPageStyle(pDocShell);
}

static rtl::Reference<SwXTextTableStyle> CreateStyleTable(SwDocShell* pDocShell, const OUString& sStyleName)
{
    return SwXTextTableStyle::CreateXTextTableStyle(pDocShell, sStyleName);
}

static rtl::Reference<SwXTextCellStyle> CreateStyleCell(SwDocShell* pDocShell, const OUString& sStyleName)
{
    return SwXTextCellStyle::CreateXTextCellStyle(pDocShell, sStyleName);
}

namespace {

class XStyleFamily : public cppu::WeakImplHelper
<
    container::XNameContainer,
    lang::XServiceInfo,
    container::XIndexAccess,
    beans::XPropertySet
>
, public SfxListener
{
    const StyleFamilyEntry& m_rEntry;
    SfxStyleSheetBasePool* m_pBasePool;
    SwDocShell* m_pDocShell;

    SwXStyle* FindStyle(std::u16string_view rStyleName) const;
    sal_Int32 GetCountOrName(OUString* pString, sal_Int32 nIndex = SAL_MAX_INT32)
        { return m_rEntry.getCountOrName(*m_pDocShell->GetDoc(), pString, nIndex); };
    static const StyleFamilyEntry& InitEntry(SfxStyleFamily eFamily)
    {
        auto& entries = lcl_GetStyleFamilyEntries();
        const auto pEntry = std::find_if(entries.begin(), entries.end(),
                [eFamily] (const StyleFamilyEntry& e) { return e.family() == eFamily; });
        assert(pEntry != entries.end());
        return *pEntry;
    }
public:
    XStyleFamily(SwDocShell* pDocShell, const SfxStyleFamily eFamily)
        : m_rEntry(InitEntry(eFamily))
        , m_pBasePool(pDocShell->GetStyleSheetPool())
        , m_pDocShell(pDocShell)
    {
        if (m_pBasePool) //tdf#124142 html docs can have no styles
            StartListening(*m_pBasePool);
    }

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override
    {
        SolarMutexGuard aGuard;
        return GetCountOrName(nullptr);
    };
    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
        { return cppu::UnoType<style::XStyle>::get(); };
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        if(!m_pBasePool)
            throw uno::RuntimeException();
        return true;
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XNameContainer
    virtual void SAL_CALL insertByName(const OUString& Name, const uno::Any& Element) override;
    virtual void SAL_CALL replaceByName(const OUString& Name, const uno::Any& Element) override;
    virtual void SAL_CALL removeByName(const OUString& Name) override;

    //XPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override
        { return {}; };
    virtual void SAL_CALL setPropertyValue( const OUString&, const uno::Any&) override
        { SAL_WARN("sw.uno", "###unexpected!"); };
    virtual uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString&, const uno::Reference<beans::XPropertyChangeListener>&) override
        { SAL_WARN("sw.uno", "###unexpected!"); };
    virtual void SAL_CALL removePropertyChangeListener( const OUString&, const uno::Reference<beans::XPropertyChangeListener>&) override
        { SAL_WARN("sw.uno", "###unexpected!"); };
    virtual void SAL_CALL addVetoableChangeListener(const OUString&, const uno::Reference<beans::XVetoableChangeListener>&) override
        { SAL_WARN("sw.uno", "###unexpected!"); };
    virtual void SAL_CALL removeVetoableChangeListener(const OUString&, const uno::Reference<beans::XVetoableChangeListener>&) override
        { SAL_WARN("sw.uno", "###unexpected!"); };

    //SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override
    {
        if(rHint.GetId() == SfxHintId::Dying)
        {
            m_pBasePool = nullptr;
            m_pDocShell = nullptr;
            EndListening(rBC);
        }
    }

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
        { return {"XStyleFamily"}; };
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
        { return cppu::supportsService(this, rServiceName); };
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override
        { return { "com.sun.star.style.StyleFamily" }; }
};
}

OUString SwXStyleFamilies::getImplementationName()
    { return {"SwXStyleFamilies"}; }

sal_Bool SwXStyleFamilies::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames()
    { return { "com.sun.star.style.StyleFamilies" }; }

SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
        SwUnoCollection(rDocShell.GetDoc()),
        m_pDocShell(&rDocShell)
    { }

SwXStyleFamilies::~SwXStyleFamilies()
    { }

uno::Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    auto& entries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(entries.begin(), entries.end(),
        [&Name] (const StyleFamilyEntry& e) { return e.name() == Name; });
    if(pEntry == entries.end())
        throw container::NoSuchElementException();
    return getByIndex(pEntry - entries.begin());
}

uno::Sequence< OUString > SwXStyleFamilies::getElementNames()
{
    auto& entries(lcl_GetStyleFamilyEntries());
    uno::Sequence<OUString> aNames(entries.size());
    std::transform(entries.begin(), entries.end(),
            aNames.getArray(), [] (const StyleFamilyEntry& e) { return e.name(); });
    return aNames;
}

sal_Bool SwXStyleFamilies::hasByName(const OUString& Name)
{
    auto& entries(lcl_GetStyleFamilyEntries());
    return std::any_of(entries.begin(), entries.end(),
        [&Name] (const StyleFamilyEntry& e) { return e.name() == Name; });
}

sal_Int32 SwXStyleFamilies::getCount()
{
    return lcl_GetStyleFamilyEntries().size();
}

uno::Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
{
    auto& entries(lcl_GetStyleFamilyEntries());
    SolarMutexGuard aGuard;
    if(nIndex < 0 || o3tl::make_unsigned(nIndex) >= entries.size())
        throw lang::IndexOutOfBoundsException();
    if(!IsValid())
        throw uno::RuntimeException();
    auto eFamily = entries[nIndex].family();
    assert(eFamily != SfxStyleFamily::All);
    auto& rxFamily = m_vFamilies[eFamily];
    if(!rxFamily.is())
        rxFamily = new XStyleFamily(m_pDocShell, eFamily);
    return uno::Any(rxFamily);
}

uno::Type SwXStyleFamilies::getElementType()
{
    return cppu::UnoType<container::XNameContainer>::get();
}

sal_Bool SwXStyleFamilies::hasElements()
    { return true; }

void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
{
    SolarMutexGuard aGuard;
    if(!IsValid() || rURL.isEmpty())
        throw uno::RuntimeException();
    SwgReaderOption aOpt;
    aOpt.SetFrameFormats(true);
    aOpt.SetTextFormats(true);
    aOpt.SetPageDescs(true);
    aOpt.SetNumRules(true);
    aOpt.SetMerge(false);
    for(const auto& rProperty: aOptions)
    {
        bool bValue = false;
        if(rProperty.Value.getValueType() == cppu::UnoType<bool>::get())
            bValue = rProperty.Value.get<bool>();

        if(rProperty.Name == UNO_NAME_OVERWRITE_STYLES)
            aOpt.SetMerge(!bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_NUMBERING_STYLES)
            aOpt.SetNumRules(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_PAGE_STYLES)
            aOpt.SetPageDescs(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_FRAME_STYLES)
            aOpt.SetFrameFormats(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_TEXT_STYLES)
            aOpt.SetTextFormats(bValue);
        else if(rProperty.Name == "InputStream")
        {
            Reference<XInputStream> xInputStream;
            if (!(rProperty.Value >>= xInputStream))
                throw IllegalArgumentException("Parameter 'InputStream' could not be converted to "
                                               "type 'com::sun::star::io::XInputStream'",
                                               nullptr, 0);

            aOpt.SetInputStream(xInputStream);

        }
    }
    const ErrCodeMsg nErr = m_pDocShell->LoadStylesFromFile( rURL, aOpt, true );
    if(nErr)
        throw io::IOException();
}

uno::Sequence< beans::PropertyValue > SwXStyleFamilies::getStyleLoaderOptions()
{
    const uno::Any aVal(true);
    return comphelper::InitPropertySequence({
        { UNO_NAME_LOAD_TEXT_STYLES, aVal },
        { UNO_NAME_LOAD_FRAME_STYLES, aVal },
        { UNO_NAME_LOAD_PAGE_STYLES, aVal },
        { UNO_NAME_LOAD_NUMBERING_STYLES, aVal },
        { UNO_NAME_OVERWRITE_STYLES, aVal }
    });
}

static bool lcl_GetHeaderFooterItem(
        SfxItemSet const& rSet, std::u16string_view rPropName, bool const bFooter,
        SvxSetItem const*& o_rpItem)
{
    o_rpItem = rSet.GetItemIfSet(
        bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
        false);
    if (!o_rpItem &&
        rPropName == UNO_NAME_FIRST_IS_SHARED)
    {   // fdo#79269 header may not exist, check footer then
        o_rpItem = rSet.GetItemIfSet(
            (!bFooter) ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
            false);
    }
    return o_rpItem;
}

css::uno::Reference<css::style::XStyle> SwXStyleFamilies::CreateStyle(SfxStyleFamily eFamily, SwDoc& rDoc)
{
    switch (eFamily)
    {
        case SfxStyleFamily::Char:
        case SfxStyleFamily::Para:
        case SfxStyleFamily::Pseudo:
            return CreateStyleCharOrParaOrPseudo(eFamily, rDoc);
        case SfxStyleFamily::Page:
            return CreateStylePage(rDoc);
        case SfxStyleFamily::Frame:
            return CreateStyleFrame(rDoc);
        case SfxStyleFamily::Table:
            return CreateStyleTable(rDoc);
        case SfxStyleFamily::Cell:
            return CreateStyleCell(rDoc);
        default:
            assert(false);
            return nullptr;
    }
}

rtl::Reference<SwXStyle> SwXStyleFamilies::CreateStyleCharOrParaOrPseudo(SfxStyleFamily eFamily, SwDoc& rDoc)
{
    return ::CreateStyleCharOrParaOrPseudo(nullptr, rDoc.GetDocShell(), "", eFamily);
}

rtl::Reference<SwXPageStyle> SwXStyleFamilies::CreateStylePage(SwDoc& rDoc)
{
    return ::CreateStylePage(nullptr, rDoc.GetDocShell(), "");
}

rtl::Reference<SwXFrameStyle> SwXStyleFamilies::CreateStyleFrame(SwDoc& rDoc)
{
    return ::CreateStyleFrame(nullptr, rDoc.GetDocShell(), "");
}

rtl::Reference<SwXTextTableStyle> SwXStyleFamilies::CreateStyleTable(SwDoc& rDoc)
{
    return ::CreateStyleTable(rDoc.GetDocShell(), "");
}

rtl::Reference<SwXTextCellStyle> SwXStyleFamilies::CreateStyleCell(SwDoc& rDoc)
{
    return ::CreateStyleCell(rDoc.GetDocShell(), "");
}

// FIXME: Ugly special casing that should die.
uno::Reference<css::style::XStyle> SwXStyleFamilies::CreateStyleCondParagraph(SwDoc& rDoc)
    { return new SwXStyle(&rDoc, SfxStyleFamily::Para, true); };

uno::Any XStyleFamily::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if(nIndex < 0)
        throw lang::IndexOutOfBoundsException();
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    try
    {
        SwStyleNameMapper::FillUIName(m_rEntry.translateIndex(nIndex), sStyleName);
    } catch(...) {}
    if (sStyleName.isEmpty())
        GetCountOrName(&sStyleName, nIndex);
    if(sStyleName.isEmpty())
        throw lang::IndexOutOfBoundsException();
    return getByName(sStyleName);
}

uno::Any XStyleFamily::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.poolId());
    if(!m_pBasePool)
        throw uno::RuntimeException();
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName, m_rEntry.family());
    if(!pBase)
        throw container::NoSuchElementException(rName);
    uno::Reference<style::XStyle> xStyle = FindStyle(sStyleName);
    if(!xStyle.is())
        switch (m_rEntry.family())
        {
            case SfxStyleFamily::Char:
            case SfxStyleFamily::Para:
            case SfxStyleFamily::Pseudo:
                xStyle = ::CreateStyleCharOrParaOrPseudo(m_pBasePool, m_pDocShell, sStyleName, m_rEntry.family());
                break;
            case SfxStyleFamily::Page:
                xStyle = ::CreateStylePage(m_pBasePool, m_pDocShell, sStyleName);
                break;
            case SfxStyleFamily::Frame:
                xStyle = ::CreateStyleFrame(m_pBasePool, m_pDocShell, pBase->GetName());
                break;
            case SfxStyleFamily::Table:
                xStyle = ::CreateStyleTable(m_pDocShell, sStyleName);
                break;
            case SfxStyleFamily::Cell:
                xStyle = ::CreateStyleCell(m_pDocShell, sStyleName);
                break;
            default:
                assert(false);
        }
    return uno::Any(xStyle);
}

uno::Sequence<OUString> XStyleFamily::getElementNames()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    std::vector<OUString> vRet;
    std::unique_ptr<SfxStyleSheetIterator> pIt = m_pBasePool->CreateIterator(m_rEntry.family());
    for (SfxStyleSheetBase* pStyle = pIt->First(); pStyle; pStyle = pIt->Next())
    {
        OUString sName;
        SwStyleNameMapper::FillProgName(pStyle->GetName(), sName, m_rEntry.poolId());
        vRet.push_back(sName);
    }
    return comphelper::containerToSequence(vRet);
}

sal_Bool XStyleFamily::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.poolId());
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName, m_rEntry.family());
    return nullptr != pBase;
}

void XStyleFamily::insertByName(const OUString& rName, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.poolId());
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName, m_rEntry.family());
    if (pBase)
        throw container::ElementExistException();
    if(rElement.getValueType().getTypeClass() != uno::TypeClass_INTERFACE)
        throw lang::IllegalArgumentException();
    if (SwGetPoolIdFromName::CellStyle == m_rEntry.poolId())
    {
        // handle cell style
        uno::Reference<style::XStyle> xStyle = rElement.get<uno::Reference<style::XStyle>>();
        SwXTextCellStyle* pNewStyle = dynamic_cast<SwXTextCellStyle*>(xStyle.get());
        if (!pNewStyle)
            throw lang::IllegalArgumentException();

        pNewStyle->setName(sStyleName); // insertByName sets the element name
        m_pDocShell->GetDoc()->GetCellStyles().AddBoxFormat(*pNewStyle->GetBoxFormat(), sStyleName);
        pNewStyle->SetPhysical();
    }
    else if (SwGetPoolIdFromName::TabStyle == m_rEntry.poolId())
    {
        // handle table style
        uno::Reference<style::XStyle> xStyle = rElement.get<uno::Reference<style::XStyle>>();
        SwXTextTableStyle* pNewStyle = dynamic_cast<SwXTextTableStyle*>(xStyle.get());
        if (!pNewStyle)
            throw lang::IllegalArgumentException();

        pNewStyle->setName(sStyleName); // insertByName sets the element name
        m_pDocShell->GetDoc()->GetTableStyles().AddAutoFormat(*pNewStyle->GetTableFormat());
        pNewStyle->SetPhysical();
    }
    else
    {
        uno::Reference<lang::XUnoTunnel> xStyleTunnel = rElement.get<uno::Reference<lang::XUnoTunnel>>();
        SwXStyle* pNewStyle = comphelper::getFromUnoTunnel<SwXStyle>(xStyleTunnel);
        if (!pNewStyle || !pNewStyle->IsDescriptor() || pNewStyle->GetFamily() != m_rEntry.family())
            throw lang::IllegalArgumentException();

        SfxStyleSearchBits nMask = SfxStyleSearchBits::All;
        if(m_rEntry.family() == SfxStyleFamily::Para && !pNewStyle->IsConditional())
            nMask &= ~SfxStyleSearchBits::SwCondColl;
        auto pStyle = &m_pBasePool->Make(sStyleName, m_rEntry.family(), nMask);
        pNewStyle->SetDoc(m_pDocShell->GetDoc(), m_pBasePool);
        pNewStyle->SetStyleName(sStyleName);
        pStyle->SetParent(pNewStyle->GetParentStyleName());
        // after all, we still need to apply the properties of the descriptor
        pNewStyle->ApplyDescriptorProperties();
    }
}

void XStyleFamily::replaceByName(const OUString& rName, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.poolId());
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName, m_rEntry.family());
    // replacements only for userdefined styles
    if(!pBase)
        throw container::NoSuchElementException();
    if (SwGetPoolIdFromName::CellStyle == m_rEntry.poolId())
    {
        // handle cell styles, don't call on assigned cell styles (TableStyle child)
        OUString sParent;
        SwBoxAutoFormat* pBoxAutoFormat = SwXTextCellStyle::GetBoxAutoFormat(m_pDocShell, sStyleName, &sParent);
        if (pBoxAutoFormat && sParent.isEmpty())// if parent exists then this style is assigned to a table style. Don't replace.
        {
            uno::Reference<style::XStyle> xStyle = rElement.get<uno::Reference<style::XStyle>>();
            SwXTextCellStyle* pStyleToReplaceWith = dynamic_cast<SwXTextCellStyle*>(xStyle.get());
            if (!pStyleToReplaceWith)
                throw lang::IllegalArgumentException();

            pStyleToReplaceWith->setName(sStyleName);
            *pBoxAutoFormat = *pStyleToReplaceWith->GetBoxFormat();
            pStyleToReplaceWith->SetPhysical();
        }
    }
    else if (SwGetPoolIdFromName::TabStyle == m_rEntry.poolId())
    {
        // handle table styles
        SwTableAutoFormat* pTableAutoFormat = SwXTextTableStyle::GetTableAutoFormat(m_pDocShell, sStyleName);
        if (pTableAutoFormat)
        {
            uno::Reference<style::XStyle> xStyle = rElement.get<uno::Reference<style::XStyle>>();
            SwXTextTableStyle* pStyleToReplaceWith = dynamic_cast<SwXTextTableStyle*>(xStyle.get());
            if (!pStyleToReplaceWith)
                throw lang::IllegalArgumentException();

            pStyleToReplaceWith->setName(sStyleName);
            *pTableAutoFormat = *pStyleToReplaceWith->GetTableFormat();
            pStyleToReplaceWith->SetPhysical();
        }
    }
    else
    {
        if(!pBase->IsUserDefined())
            throw lang::IllegalArgumentException();
        //if there's an object available to this style then it must be invalidated
        uno::Reference<style::XStyle> xStyle = FindStyle(pBase->GetName());
        if(xStyle.is())
        {
            SwXStyle* pStyle = comphelper::getFromUnoTunnel<SwXStyle>(xStyle);
            if(pStyle)
                pStyle->Invalidate();
        }
        m_pBasePool->Remove(pBase);
        insertByName(rName, rElement);
    }
}

void XStyleFamily::removeByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sName;
    SwStyleNameMapper::FillUIName(rName, sName, m_rEntry.poolId());
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sName, m_rEntry.family());
    if(!pBase)
        throw container::NoSuchElementException();
    if (SwGetPoolIdFromName::CellStyle == m_rEntry.poolId())
    {
        // handle cell style
        m_pDocShell->GetDoc()->GetCellStyles().RemoveBoxFormat(rName);
    }
    else if (SwGetPoolIdFromName::TabStyle == m_rEntry.poolId())
    {
        // handle table style
        m_pDocShell->GetDoc()->GetTableStyles().EraseAutoFormat(rName);
    }
    else
        m_pBasePool->Remove(pBase);
}

uno::Any SAL_CALL XStyleFamily::getPropertyValue( const OUString& sPropertyName )
{
    if(sPropertyName != "DisplayName")
        throw beans::UnknownPropertyException( "unknown property: " + sPropertyName, getXWeak() );
    SolarMutexGuard aGuard;
    return uno::Any(SwResId(m_rEntry.resId()));
}


SwXStyle* XStyleFamily::FindStyle(std::u16string_view rStyleName) const
{
    SwXStyle* pFoundStyle = nullptr;
    m_pBasePool->ForAllListeners(
        [this, &pFoundStyle, &rStyleName] (SfxListener* pListener)
        {
            SwXStyle* pTempStyle = dynamic_cast<SwXStyle*>(pListener);
            if(pTempStyle && pTempStyle->GetFamily() == m_rEntry.family() && pTempStyle->GetStyleName() == rStyleName)
            {
                pFoundStyle = pTempStyle;
                return true; // break
            }
            return false;
        });
    return pFoundStyle;
}

static SwGetPoolIdFromName lcl_GetSwEnumFromSfxEnum(SfxStyleFamily eFamily)
{
    auto& entries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(entries.begin(), entries.end(),
            [eFamily] (const StyleFamilyEntry& e) { return e.family() == eFamily; });
    if(pEntry != entries.end())
        return pEntry->poolId();
    SAL_WARN("sw.uno", "someone asking for all styles in unostyle.cxx!" );
    return SwGetPoolIdFromName::ChrFmt;
}

const uno::Sequence<sal_Int8>& SwXStyle::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXStyleUnoTunnelId;
    return theSwXStyleUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SwXStyle::getSomething(const uno::Sequence<sal_Int8>& rId)
{
    return comphelper::getSomethingImpl(rId, this);
}


uno::Sequence< OUString > SwXStyle::getSupportedServiceNames()
{
    tools::Long nCount = 1;
    if(SfxStyleFamily::Para == m_rEntry.family())
    {
        nCount = 5;
        if(m_bIsConditional)
            nCount++;
    }
    else if(SfxStyleFamily::Char == m_rEntry.family())
        nCount = 5;
    else if(SfxStyleFamily::Page == m_rEntry.family())
        nCount = 3;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.Style";
    switch(m_rEntry.family())
    {
        case SfxStyleFamily::Char:
            pArray[1] = "com.sun.star.style.CharacterStyle";
            pArray[2] = "com.sun.star.style.CharacterProperties";
            pArray[3] = "com.sun.star.style.CharacterPropertiesAsian";
            pArray[4] = "com.sun.star.style.CharacterPropertiesComplex";
        break;
        case SfxStyleFamily::Page:
            pArray[1] = "com.sun.star.style.PageStyle";
            pArray[2] = "com.sun.star.style.PageProperties";
        break;
        case SfxStyleFamily::Para:
            pArray[1] = "com.sun.star.style.ParagraphStyle";
            pArray[2] = "com.sun.star.style.ParagraphProperties";
            pArray[3] = "com.sun.star.style.ParagraphPropertiesAsian";
            pArray[4] = "com.sun.star.style.ParagraphPropertiesComplex";
        if(m_bIsConditional)
            pArray[5] = "com.sun.star.style.ConditionalParagraphStyle";
        break;

        default:
            ;
    }
    return aRet;
}

static uno::Reference<beans::XPropertySet> lcl_InitStandardStyle(const SfxStyleFamily eFamily,  uno::Reference<container::XNameAccess> const & rxStyleFamily)
{
    using return_t = decltype(lcl_InitStandardStyle(eFamily, rxStyleFamily));
    if(eFamily != SfxStyleFamily::Para && eFamily != SfxStyleFamily::Page)
        return {};
    auto aResult(rxStyleFamily->getByName("Standard"));
    if(!aResult.has<return_t>())
        return {};
    return aResult.get<return_t>();
}

static uno::Reference<container::XNameAccess> lcl_InitStyleFamily(SwDoc* pDoc, const StyleFamilyEntry& rEntry)
{
    using return_t = decltype(lcl_InitStyleFamily(pDoc, rEntry));
    if(rEntry.family() != SfxStyleFamily::Char
            && rEntry.family() != SfxStyleFamily::Para
            && rEntry.family() != SfxStyleFamily::Page)
        return {};
    auto xModel(pDoc->GetDocShell()->GetBaseModel());
    uno::Reference<style::XStyleFamiliesSupplier> xFamilySupplier(xModel, uno::UNO_QUERY);
    auto xFamilies = xFamilySupplier->getStyleFamilies();
    auto aResult(xFamilies->getByName(rEntry.name()));
    if(!aResult.has<return_t>())
        return {};
    return aResult.get<return_t>();
}

static bool lcl_InitConditional(SfxStyleSheetBasePool* pBasePool, const SfxStyleFamily eFamily, const OUString& rStyleName)
{
    if(!pBasePool || eFamily != SfxStyleFamily::Para)
        return false;
    SfxStyleSheetBase* pBase = pBasePool->Find(rStyleName, eFamily);
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?" );
    if(!pBase)
        return false;
    const sal_uInt16 nId(SwStyleNameMapper::GetPoolIdFromUIName(rStyleName, SwGetPoolIdFromName::TxtColl));
    if(nId != USHRT_MAX)
        return ::IsConditionalByPoolId(nId);
    return RES_CONDTXTFMTCOLL == static_cast<SwDocStyleSheet*>(pBase)->GetCollection()->Which();
}

static const StyleFamilyEntry& lcl_GetStyleEntry(const SfxStyleFamily eFamily)
{
    auto& entries = lcl_GetStyleFamilyEntries();
    const auto pEntry = std::find_if(entries.begin(), entries.end(),
            [eFamily] (const StyleFamilyEntry& e) { return e.family() == eFamily; });
    assert(pEntry != entries.end());
    return *pEntry;
}

SwXStyle::SwXStyle(SwDoc* pDoc, SfxStyleFamily eFamily, bool bConditional)
    : m_pDoc(pDoc)
    , m_rEntry(lcl_GetStyleEntry(eFamily))
    , m_bIsDescriptor(true)
    , m_bIsConditional(bConditional)
    , m_pBasePool(nullptr)
    , m_xStyleFamily(lcl_InitStyleFamily(pDoc, m_rEntry))
    , m_xStyleData(lcl_InitStandardStyle(eFamily, m_xStyleFamily))
{
    assert(!m_bIsConditional || m_rEntry.family() == SfxStyleFamily::Para); // only paragraph styles are conditional
    // Register ourselves as a listener to the document (via the page descriptor)
    SvtListener::StartListening(pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
    m_pPropertiesImpl = std::make_unique<SwStyleProperties_Impl>(
            aSwMapProvider.GetPropertySet(m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE :  m_rEntry.propMapType())->getPropertyMap());
}

SwXStyle::SwXStyle(SfxStyleSheetBasePool* pPool, SfxStyleFamily eFamily, SwDoc* pDoc, const OUString& rStyleName)
    : m_pDoc(pDoc)
    , m_sStyleName(rStyleName)
    , m_rEntry(lcl_GetStyleEntry(eFamily))
    , m_bIsDescriptor(false)
    , m_bIsConditional(lcl_InitConditional(pPool, eFamily, rStyleName))
    , m_pBasePool(pPool)
{ }

SwXStyle::~SwXStyle()
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
        SfxListener::EndListening(*m_pBasePool);
    m_pPropertiesImpl.reset();
    SvtListener::EndListeningAll();
}

void SwXStyle::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pDoc = nullptr;
        m_xStyleData.clear();
        m_xStyleFamily.clear();
    }
}

OUString SwXStyle::getName()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        return m_sStyleName;
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase)
        throw uno::RuntimeException();
    OUString aString;
    SwStyleNameMapper::FillProgName(pBase->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( m_rEntry.family()));
    return aString;
}

void SwXStyle::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        m_sStyleName = rName;
        return;
    }
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase || !pBase->IsUserDefined())
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xTmp(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    if(!xTmp->SetName(rName))
        throw uno::RuntimeException();
    m_sStyleName = rName;
}

sal_Bool SwXStyle::isUserDefined()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    //if it is not found it must be non user defined
    return pBase && pBase->IsUserDefined();
}

sal_Bool SwXStyle::isInUse()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family(), SfxStyleSearchBits::Used);
    return pBase && pBase->IsUsed();
}

OUString SwXStyle::getParentStyle()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        return m_sParentStyleName;
    }
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    OUString aString;
    if(pBase)
        aString = pBase->GetParent();
    SwStyleNameMapper::FillProgName(aString, aString, lcl_GetSwEnumFromSfxEnum(m_rEntry.family()));
    return aString;
}

void SwXStyle::setParentStyle(const OUString& rParentStyle)
{
    SolarMutexGuard aGuard;
    OUString sParentStyle;
    SwStyleNameMapper::FillUIName(rParentStyle, sParentStyle, lcl_GetSwEnumFromSfxEnum ( m_rEntry.family()) );
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        m_sParentStyleName = sParentStyle;
        try
        {
            const auto aAny = m_xStyleFamily->getByName(sParentStyle);
            m_xStyleData = aAny.get<decltype(m_xStyleData)>();
        }
        catch(...)
        { }
        return;
    }
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    if(!pBase)
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    //make it a 'real' style - necessary for pooled styles
    xBase->GetItemSet();
    if(xBase->GetParent() != sParentStyle)
    {
        if(!xBase->SetParent(sParentStyle))
            throw uno::RuntimeException();
    }
}

uno::Reference<beans::XPropertySetInfo> SwXStyle::getPropertySetInfo()
{
    if(m_bIsConditional)
    {
        assert(m_rEntry.family() == SfxStyleFamily::Para);
        static auto xCondParaRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CONDITIONAL_PARA_STYLE)->getPropertySetInfo();
        return xCondParaRef;
    }
    return m_rEntry.xPSInfo();
}

void SwXStyle::ApplyDescriptorProperties()
{
    m_bIsDescriptor = false;
    m_xStyleData.clear();
    m_xStyleFamily.clear();
    m_pPropertiesImpl->Apply(*this);
}

const TranslateId STR_POOLPAGE_ARY[] =
{
    // Page styles
    STR_POOLPAGE_STANDARD,
    STR_POOLPAGE_FIRST,
    STR_POOLPAGE_LEFT,
    STR_POOLPAGE_RIGHT,
    STR_POOLPAGE_ENVELOPE,
    STR_POOLPAGE_REGISTER,
    STR_POOLPAGE_HTML,
    STR_POOLPAGE_FOOTNOTE,
    STR_POOLPAGE_ENDNOTE,
    STR_POOLPAGE_LANDSCAPE
};


const SwPageDesc* SwStyleBase_Impl::GetOldPageDesc()
{
    if(!m_pOldPageDesc)
    {
        SwPageDesc *pd = m_rDoc.FindPageDesc(m_rStyleName);
        if(pd)
            m_pOldPageDesc = pd;

        if(!m_pOldPageDesc)
        {
            for (size_t i = 0; i < SAL_N_ELEMENTS(STR_POOLPAGE_ARY); ++i)
            {
                if (SwResId(STR_POOLPAGE_ARY[i]) == m_rStyleName)
                {
                    m_pOldPageDesc = m_rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_BEGIN + i);
                    break;
                }
            }
        }
    }
    return m_pOldPageDesc;
}


static sal_uInt8 lcl_TranslateMetric(const SfxItemPropertyMapEntry& rEntry, SwDoc* pDoc, uno::Any& o_aValue)
{
    // check for needed metric translation
    if(!(rEntry.nMoreFlags & PropertyMoreFlags::METRIC_ITEM))
        return rEntry.nMemberId;
    // exception: If these ItemTypes are used, do not convert when these are negative
    // since this means they are intended as percent values
    if((XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
            && o_aValue.has<sal_Int32>()
            && o_aValue.get<sal_Int32>() < 0)
        return rEntry.nMemberId;
    if(!pDoc)
        return rEntry.nMemberId;

    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const MapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));
    if(eMapUnit != MapUnit::Map100thMM)
        SvxUnoConvertFromMM(eMapUnit, o_aValue);
    return rEntry.nMemberId;
}

template<>
void SwXStyle::SetPropertyValue<HINT_BEGIN>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& /*rPropSet*/, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    // default ItemSet handling
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
    aSet.SetParent(&rStyleSet);
    SfxItemPropertySet::setPropertyValue(rEntry, rValue, aSet);
    rStyleSet.Put(aSet);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_HIDDEN>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    bool bHidden = false;
    if(rValue >>= bHidden)
    {
        //make it a 'real' style - necessary for pooled styles
        o_rStyleBase.getNewBase()->GetItemSet();
        o_rStyleBase.getNewBase()->SetHidden(bHidden);
    }
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_STYLE_INTEROP_GRAB_BAG>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    o_rStyleBase.getNewBase()->GetItemSet();
    o_rStyleBase.getNewBase()->SetGrabBagItem(rValue);
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(XATTR_FILLGRADIENT)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    uno::Any aValue(rValue);
    const auto nMemberId(lcl_TranslateMetric(rEntry, m_pDoc, aValue));
    if(MID_NAME == nMemberId)
    {
        // add set commands for FillName items
        SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
        if(!aValue.has<OUString>())
            throw lang::IllegalArgumentException();
        SvxShape::SetFillAttribute(rEntry.nWID, aValue.get<OUString>(), rStyleSet);
    }
    else if(MID_BITMAP == nMemberId)
    {
        if(sal_uInt16(XATTR_FILLBITMAP) == rEntry.nWID)
        {
            Graphic aNullGraphic;
            SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
            XFillBitmapItem aXFillBitmapItem(std::move(aNullGraphic));
            aXFillBitmapItem.PutValue(aValue, nMemberId);
            rStyleSet.Put(aXFillBitmapItem);
        }
    }
    else
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_BACKGROUND)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    const std::unique_ptr<SvxBrushItem> aOriginalBrushItem(getSvxBrushItemFromSourceSet(rStyleSet, RES_BACKGROUND, true, m_pDoc->IsInXMLImport()));
    std::unique_ptr<SvxBrushItem> aChangedBrushItem(aOriginalBrushItem->Clone());

    uno::Any aValue(rValue);
    const auto nMemberId(lcl_TranslateMetric(rEntry, m_pDoc, aValue));
    aChangedBrushItem->PutValue(aValue, nMemberId);

    // 0xff is already the default - but if BackTransparent is set
    // to true, it must be applied in the item set on ODF import
    // to potentially override parent style, which is unknown yet
    if(*aChangedBrushItem == *aOriginalBrushItem && (MID_GRAPHIC_TRANSPARENT != nMemberId || !aValue.has<bool>() || !aValue.get<bool>()))
        return;

    setSvxBrushItemAsFillAttributesToTargetSet(*aChangedBrushItem, rStyleSet);
}
template<>
void SwXStyle::SetPropertyValue<OWN_ATTR_FILLBMP_MODE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    drawing::BitmapMode eMode;
    if(!(rValue >>= eMode))
    {
        if(!rValue.has<sal_Int32>())
            throw lang::IllegalArgumentException();
        eMode = static_cast<drawing::BitmapMode>(rValue.get<sal_Int32>());
    }
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    rStyleSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
    rStyleSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_PAPER_BIN)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& /*rPropSet*/, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if (!rValue.has<OUString>() && !rValue.has<sal_Int32>())
        throw lang::IllegalArgumentException();
    SfxPrinter* pPrinter = m_pDoc->getIDocumentDeviceAccess().getPrinter(true);
    using printeridx_t = decltype(pPrinter->GetPaperBinCount());
    printeridx_t nBin = std::numeric_limits<printeridx_t>::max();
    if(rValue.has<OUString>())
    {
        OUString sValue(rValue.get<OUString>());
        if(sValue == "[From printer settings]")
            nBin = std::numeric_limits<printeridx_t>::max()-1;
        else if(pPrinter)
        {
            for(printeridx_t i=0, nEnd = pPrinter->GetPaperBinCount(); i < nEnd; ++i)
            {
                if (sValue == pPrinter->GetPaperBinName(i))
                {
                    nBin = i;
                    break;
                }
            }
        }
    }
    else if (rValue.has<sal_Int32>() && pPrinter)
    {
        sal_Int32 nValue (rValue.get<sal_Int32>());
        nBin = pPrinter->GetPaperBinBySourceIndex(nValue);
    }

    if(nBin == std::numeric_limits<printeridx_t>::max())
        throw lang::IllegalArgumentException();

    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
    aSet.SetParent(&rStyleSet);
    SfxItemPropertySet::setPropertyValue(rEntry, uno::Any(static_cast<sal_Int8>(nBin == std::numeric_limits<printeridx_t>::max()-1 ? -1 : nBin)), aSet);
    rStyleSet.Put(aSet);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_NUM_RULES>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    auto xNumberTunnel(rValue.get<uno::Reference<container::XIndexReplace>>());
    if(!xNumberTunnel)
        throw lang::IllegalArgumentException();
    SwXNumberingRules* pSwXRules = dynamic_cast<SwXNumberingRules*>(xNumberTunnel.get());
    if(!pSwXRules)
        return;
    SwNumRule aSetRule(*pSwXRules->GetNumRule());
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        const SwNumFormat* pFormat = aSetRule.GetNumFormat(i);
        if(!pFormat)
            continue;
        SwNumFormat aFormat(*pFormat);
        const auto& rCharName(pSwXRules->GetNewCharStyleNames()[i]);
        if(!rCharName.isEmpty()
               && !SwXNumberingRules::isInvalidStyle(rCharName)
               && (!pFormat->GetCharFormat() || pFormat->GetCharFormat()->GetName() != rCharName))
        {
            auto pCharFormatIt(std::find_if(m_pDoc->GetCharFormats()->begin(), m_pDoc->GetCharFormats()->end(),
                    [&rCharName] (SwCharFormat* pF) { return pF->GetName() == rCharName; }));
            if(pCharFormatIt != m_pDoc->GetCharFormats()->end())
                aFormat.SetCharFormat(*pCharFormatIt);
            else if(m_pBasePool)
            {
                auto pBase(m_pBasePool->Find(rCharName, SfxStyleFamily::Char));
                if(!pBase)
                    pBase = &m_pBasePool->Make(rCharName, SfxStyleFamily::Char);
                aFormat.SetCharFormat(static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat());
            }
            else
                aFormat.SetCharFormat(nullptr);
        }
        // same for fonts:
        const auto& rBulletName(pSwXRules->GetBulletFontNames()[i]);
        if(!rBulletName.isEmpty()
                && !SwXNumberingRules::isInvalidStyle(rBulletName)
                && (!pFormat->GetBulletFont() || pFormat->GetBulletFont()->GetFamilyName() != rBulletName))
        {
            const auto pFontListItem(static_cast<const SvxFontListItem*>(m_pDoc->GetDocShell()->GetItem(SID_ATTR_CHAR_FONTLIST)));
            const auto pList(pFontListItem->GetFontList());
            vcl::Font aFont(pList->Get(rBulletName, WEIGHT_NORMAL, ITALIC_NONE));
            aFormat.SetBulletFont(&aFont);
        }
        aSetRule.Set(i, &aFormat);
    }
    o_rStyleBase.getNewBase()->SetNumRule(aSetRule);
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_OUTLINELEVEL)>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<sal_Int16>())
        return;
    const auto nLevel(rValue.get<sal_Int16>());
    if(0 <= nLevel && nLevel <= MAXLEVEL)
        o_rStyleBase.getNewBase()->GetCollection()->SetAttrOutlineLevel(nLevel);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_FOLLOW_STYLE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<OUString>())
        return;
    const auto sValue(rValue.get<OUString>());
    OUString aString;
    SwStyleNameMapper::FillUIName(sValue, aString, m_rEntry.poolId());
    o_rStyleBase.getNewBase()->SetFollow(aString);
}

template <>
void SwXStyle::SetPropertyValue<FN_UNO_LINK_STYLE>(const SfxItemPropertyMapEntry&,
                                                   const SfxItemPropertySet&,
                                                   const uno::Any& rValue,
                                                   SwStyleBase_Impl& o_rStyleBase)
{
    if (!rValue.has<OUString>())
        return;
    const auto sValue(rValue.get<OUString>());
    OUString aString;
    SwStyleNameMapper::FillUIName(sValue, aString, m_rEntry.poolId());
    o_rStyleBase.getNewBase()->SetLink(aString);
}

template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_PAGEDESC)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_PAGEDESC_PAGEDESCNAME != rEntry.nMemberId)
    {
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
        return;
    }
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    // special handling for RES_PAGEDESC
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    std::unique_ptr<SwFormatPageDesc> pNewDesc;
    if(const SwFormatPageDesc* pItem = rStyleSet.GetItemIfSet(RES_PAGEDESC))
        pNewDesc.reset(new SwFormatPageDesc(*pItem));
    else
        pNewDesc.reset(new SwFormatPageDesc);
    const auto sValue(rValue.get<OUString>());
    OUString sDescName;
    SwStyleNameMapper::FillUIName(sValue, sDescName, SwGetPoolIdFromName::PageDesc);
    if(pNewDesc->GetPageDesc() && pNewDesc->GetPageDesc()->GetName() == sDescName)
        return;
    if(sDescName.isEmpty())
    {
        rStyleSet.ClearItem(RES_BREAK);
        rStyleSet.Put(SwFormatPageDesc());
    }
    else
    {
        SwPageDesc* pPageDesc(SwPageDesc::GetByName(*m_pDoc, sDescName));
        if(!pPageDesc)
            throw lang::IllegalArgumentException();
        pNewDesc->RegisterToPageDesc(*pPageDesc);
        rStyleSet.Put(std::move(pNewDesc));
    }
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_TEXT_VERT_ADJUST)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(m_rEntry.family() != SfxStyleFamily::Page)
    {
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
        return;
    }
    if(!m_pDoc || !rValue.has<drawing::TextVerticalAdjust>() || !o_rStyleBase.GetOldPageDesc())
        return;
    SwPageDesc* pPageDesc = m_pDoc->FindPageDesc(o_rStyleBase.GetOldPageDesc()->GetName());
    if(pPageDesc)
        pPageDesc->SetVerticalAdjustment(rValue.get<drawing::TextVerticalAdjust>());
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_IS_AUTO_UPDATE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<bool>())
        throw lang::IllegalArgumentException();
    const bool bAuto(rValue.get<bool>());
    if(SfxStyleFamily::Para == m_rEntry.family())
        o_rStyleBase.getNewBase()->GetCollection()->SetAutoUpdateOnDirectFormat(bAuto);
    else if(SfxStyleFamily::Frame == m_rEntry.family())
        o_rStyleBase.getNewBase()->GetFrameFormat()->SetAutoUpdateOnDirectFormat(bAuto);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_PARA_STYLE_CONDITIONS>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    static_assert(COND_COMMAND_COUNT == 28, "invalid size of command count?");
    using expectedarg_t = uno::Sequence<beans::NamedValue>;
    if(!rValue.has<expectedarg_t>() || !m_pBasePool)
        throw lang::IllegalArgumentException();
    SwCondCollItem aCondItem;
    const auto aNamedValues = rValue.get<expectedarg_t>();
    for(const auto& rNamedValue : aNamedValues)
    {
        if(!rNamedValue.Value.has<OUString>())
            throw lang::IllegalArgumentException();

        const OUString sValue(rNamedValue.Value.get<OUString>());
        // get UI style name from programmatic style name
        OUString aStyleName;
        SwStyleNameMapper::FillUIName(sValue, aStyleName, lcl_GetSwEnumFromSfxEnum(m_rEntry.family()));

        // check for correct context and style name
        const auto nIdx(GetCommandContextIndex(rNamedValue.Name));
        if (nIdx == -1)
            throw lang::IllegalArgumentException();
        bool bStyleFound = false;
        for(auto pBase = m_pBasePool->First(SfxStyleFamily::Para); pBase; pBase = m_pBasePool->Next())
        {
            bStyleFound = pBase->GetName() == aStyleName;
            if (bStyleFound)
                break;
        }
        if (!bStyleFound)
            throw lang::IllegalArgumentException();
        aCondItem.SetStyle(&aStyleName, nIdx);
    }
    o_rStyleBase.GetItemSet().Put(aCondItem);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_CATEGORY>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!o_rStyleBase.getNewBase()->IsUserDefined() || !rValue.has<paragraphstyle_t>())
        throw lang::IllegalArgumentException();
    static const std::map<paragraphstyle_t, SfxStyleSearchBits> aUnoToCore = []{
        std::map<paragraphstyle_t, SfxStyleSearchBits> map;
        std::transform(std::begin(sParagraphStyleCategoryEntries), std::end(sParagraphStyleCategoryEntries), std::inserter(map, map.end()),
            [] (const ParagraphStyleCategoryEntry& rEntry) { return std::make_pair(rEntry.m_eCategory, rEntry.m_nSwStyleBits); });
        return map;
    }();
    const auto pUnoToCoreIt(aUnoToCore.find(rValue.get<paragraphstyle_t>()));
    if (pUnoToCoreIt == aUnoToCore.end())
        throw lang::IllegalArgumentException();
    o_rStyleBase.getNewBase()->SetMask( pUnoToCoreIt->second|SfxStyleSearchBits::UserDefined );
}
template<>
void SwXStyle::SetPropertyValue<SID_SWREGISTER_COLLECTION>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    OUString sName;
    rValue >>= sName;
    SwRegisterItem aReg(!sName.isEmpty());
    aReg.SetWhich(SID_SWREGISTER_MODE);
    o_rStyleBase.GetItemSet().Put(aReg);
    OUString aString;
    SwStyleNameMapper::FillUIName(sName, aString, SwGetPoolIdFromName::TxtColl);
    o_rStyleBase.GetItemSet().Put(SfxStringItem(SID_SWREGISTER_COLLECTION, aString ) );
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_TXTATR_CJK_RUBY)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_RUBY_CHARSTYLE != rEntry.nMemberId)
        return;
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    const auto sValue(rValue.get<OUString>());
    SfxItemSet& rStyleSet(o_rStyleBase.GetItemSet());
    std::unique_ptr<SwFormatRuby> pRuby;
    if(const SwFormatRuby* pRubyItem = rStyleSet.GetItemIfSet(RES_TXTATR_CJK_RUBY))
        pRuby.reset(new SwFormatRuby(*pRubyItem));
    else
        pRuby.reset(new SwFormatRuby(OUString()));
    OUString sStyle;
    SwStyleNameMapper::FillUIName(sValue, sStyle, SwGetPoolIdFromName::ChrFmt);
    pRuby->SetCharFormatName(sValue);
    pRuby->SetCharFormatId(0);
    if(!sValue.isEmpty())
    {
        const sal_uInt16 nId(SwStyleNameMapper::GetPoolIdFromUIName(sValue, SwGetPoolIdFromName::ChrFmt));
        pRuby->SetCharFormatId(nId);
    }
    rStyleSet.Put(std::move(pRuby));
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_DROP)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_DROPCAP_CHAR_STYLE_NAME != rEntry.nMemberId)
    {
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
        return;
    }
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    SfxItemSet& rStyleSet(o_rStyleBase.GetItemSet());
    std::unique_ptr<SwFormatDrop> pDrop;
    if(const SwFormatDrop* pDropItem = rStyleSet.GetItemIfSet(RES_PARATR_DROP))
        pDrop.reset(new SwFormatDrop(*pDropItem));
    else
        pDrop.reset(new SwFormatDrop);
    const auto sValue(rValue.get<OUString>());
    OUString sStyle;
    SwStyleNameMapper::FillUIName(sValue, sStyle, SwGetPoolIdFromName::ChrFmt);
    auto pStyle(static_cast<SwDocStyleSheet*>(m_pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SfxStyleFamily::Char)));
    //default character style must not be set as default format
    if(!pStyle || pStyle->GetCharFormat() == m_pDoc->GetDfltCharFormat() )
    {
        throw lang::IllegalArgumentException();
    }
    pDrop->SetCharFormat(pStyle->GetCharFormat());
    rStyleSet.Put(std::move(pDrop));
}
template<>
void SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_NUMRULE)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    uno::Any aValue(rValue);
    lcl_TranslateMetric(rEntry, m_pDoc, aValue);
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, o_rStyleBase);
    // --> OD 2006-10-18 #i70223#
    if(SfxStyleFamily::Para == m_rEntry.family() &&
            o_rStyleBase.getNewBase().is() && o_rStyleBase.getNewBase()->GetCollection() &&
            //rBase.getNewBase()->GetCollection()->GetOutlineLevel() < MAXLEVEL /* assigned to list level of outline style */) //#outline level,removed by zhaojianwei
            o_rStyleBase.getNewBase()->GetCollection()->IsAssignedToListLevelOfOutlineStyle())       ////<-end,add by zhaojianwei
    {
        OUString sNewNumberingRuleName;
        aValue >>= sNewNumberingRuleName;
        if(sNewNumberingRuleName.isEmpty() || sNewNumberingRuleName != m_pDoc->GetOutlineNumRule()->GetName())
            o_rStyleBase.getNewBase()->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
    }
}

void SwXStyle::SetStyleProperty(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& rBase)
{
    using propertytype_t = decltype(rEntry.nWID);
    using coresetter_t = std::function<void(SwXStyle&, const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, const uno::Any&, SwStyleBase_Impl&)>;
    static const std::map<propertytype_t, coresetter_t> aUnoToCore{
            // these explicit std::mem_fn() calls shouldn't be needed, but apparently MSVC is currently too stupid for C++11 again
            { FN_UNO_HIDDEN,                 std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_HIDDEN>)                 },
            { FN_UNO_STYLE_INTEROP_GRAB_BAG, std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_STYLE_INTEROP_GRAB_BAG>) },
            { XATTR_FILLGRADIENT,            std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(XATTR_FILLGRADIENT)>)            },
            { XATTR_FILLHATCH,               std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(XATTR_FILLGRADIENT)>)            },
            { XATTR_FILLBITMAP,              std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(XATTR_FILLGRADIENT)>)            },
            { XATTR_FILLFLOATTRANSPARENCE,   std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(XATTR_FILLGRADIENT)>)            },
            { RES_BACKGROUND,                std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_BACKGROUND)>)                },
            { OWN_ATTR_FILLBMP_MODE,         std::mem_fn(&SwXStyle::SetPropertyValue<OWN_ATTR_FILLBMP_MODE>)         },
            { RES_PAPER_BIN,                 std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_PAPER_BIN)>)                 },
            { FN_UNO_NUM_RULES,              std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_NUM_RULES>)              },
            { RES_PARATR_OUTLINELEVEL,       std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_OUTLINELEVEL)>)       },
            { FN_UNO_FOLLOW_STYLE,           std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_FOLLOW_STYLE>)           },
            { FN_UNO_LINK_STYLE,             std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_LINK_STYLE>)             },
            { RES_PAGEDESC,                  std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_PAGEDESC)>)                  },
            { RES_TEXT_VERT_ADJUST,          std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_TEXT_VERT_ADJUST)>)          },
            { FN_UNO_IS_AUTO_UPDATE,         std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_IS_AUTO_UPDATE>)         },
            { FN_UNO_PARA_STYLE_CONDITIONS,  std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_PARA_STYLE_CONDITIONS>)  },
            { FN_UNO_CATEGORY,               std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_CATEGORY>)               },
            { SID_SWREGISTER_COLLECTION,     std::mem_fn(&SwXStyle::SetPropertyValue<SID_SWREGISTER_COLLECTION>)     },
            { RES_TXTATR_CJK_RUBY,           std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_TXTATR_CJK_RUBY)>)           },
            { RES_PARATR_DROP,               std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_DROP)>)               },
            { RES_PARATR_NUMRULE,            std::mem_fn(&SwXStyle::SetPropertyValue<sal_uInt16(RES_PARATR_NUMRULE)>)            }
    };
    const auto pUnoToCoreIt(aUnoToCore.find(rEntry.nWID));
    if(pUnoToCoreIt != aUnoToCore.end())
        pUnoToCoreIt->second(*this, rEntry, rPropSet, rValue, rBase);
    else
    {
        // adapted switch logic to a more readable state; removed goto's and made
        // execution of standard setting of property in ItemSet dependent of this variable
        uno::Any aValue(rValue);
        lcl_TranslateMetric(rEntry, m_pDoc, aValue);
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, rBase);
    }
}

void SwXStyle::SetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
{
    if(!m_pDoc)
        throw uno::RuntimeException();
    sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    SwStyleBase_Impl aBaseImpl(*m_pDoc, m_sStyleName, &GetDoc()->GetDfltTextFormatColl()->GetAttrSet()); // add pDfltTextFormatColl as parent
    if(m_pBasePool)
    {
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
        SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
        if(!pBase)
            throw uno::RuntimeException();
        aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    }
    if(!aBaseImpl.getNewBase().is() && !m_bIsDescriptor)
        throw uno::RuntimeException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
    {
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(pNames[nProp]);
        if(!pEntry || (!m_bIsConditional && pNames[nProp] == UNO_NAME_PARA_STYLE_CONDITIONS))
            throw beans::UnknownPropertyException("Unknown property: " + pNames[nProp], getXWeak());
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], getXWeak());
        if(aBaseImpl.getNewBase().is())
            SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl);
        else if(!m_pPropertiesImpl->SetProperty(pNames[nProp], pValues[nProp]))
            throw lang::IllegalArgumentException();
    }

    if(aBaseImpl.HasItemSet())
        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
}

void SwXStyle::setPropertyValues(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
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

SfxStyleSheetBase* SwXStyle::GetStyleSheetBase()
{
    if(!m_pBasePool)
        return nullptr;
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());
    return pBase;
}
void SwXStyle::PrepareStyleBase(SwStyleBase_Impl& rBase)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        throw uno::RuntimeException();
    if(!rBase.getNewBase().is())
        rBase.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
}

template<>
uno::Any SwXStyle::GetStyleProperty<HINT_BEGIN>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase);
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_IS_PHYSICAL>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::Any(false);
    bool bPhys = static_cast<SwDocStyleSheet*>(pBase)->IsPhysical();
    // The standard character format is not existing physically
    if( bPhys && SfxStyleFamily::Char == GetFamily() &&
        static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat() &&
        static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat()->IsDefault() )
        bPhys = false;
    return uno::Any(bool(bPhys));
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_HIDDEN>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::Any(false);
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    return uno::Any(xBase->IsHidden());
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_STYLE_INTEROP_GRAB_BAG>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::Any();
    uno::Any aRet;
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    xBase->GetGrabBagItem(aRet);
    return aRet;
}
template<>
uno::Any SwXStyle::GetStyleProperty<sal_uInt16(RES_PAPER_BIN)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& /*rPropSet*/, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SfxItemSet& rSet = rBase.GetItemSet();
    uno::Any aValue;
    SfxItemPropertySet::getPropertyValue(rEntry, rSet, aValue);
    sal_Int8 nBin(aValue.get<sal_Int8>());
    if(nBin == -1)
        return uno::Any(OUString("[From printer settings]"));
    SfxPrinter* pPrinter = GetDoc()->getIDocumentDeviceAccess().getPrinter(false);
    if(!pPrinter)
        return uno::Any();
    return uno::Any(pPrinter->GetPaperBinName(nBin));
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_NUM_RULES>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SwNumRule* pRule = rBase.getNewBase()->GetNumRule();
    assert(pRule && "Where is the NumRule?");
    uno::Reference<container::XIndexReplace> xRules(new SwXNumberingRules(*pRule, GetDoc()));
    return uno::Any(xRules);
}
template<>
uno::Any SwXStyle::GetStyleProperty<sal_uInt16(RES_PARATR_OUTLINELEVEL)>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SAL_WARN_IF(SfxStyleFamily::Para != GetFamily(), "sw.uno", "only paras");
    return uno::Any(sal_Int16(rBase.getNewBase()->GetCollection()->GetAttrOutlineLevel()));
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_FOLLOW_STYLE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    OUString aString;
    SwStyleNameMapper::FillProgName(rBase.getNewBase()->GetFollow(), aString, lcl_GetSwEnumFromSfxEnum(GetFamily()));
    return uno::Any(aString);
}

template <>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_LINK_STYLE>(const SfxItemPropertyMapEntry&,
                                                       const SfxItemPropertySet&,
                                                       SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    OUString aString;
    SwStyleNameMapper::FillProgName(rBase.getNewBase()->GetLink(), aString,
                                    lcl_GetSwEnumFromSfxEnum(GetFamily()));
    return uno::Any(aString);
}

template<>
uno::Any SwXStyle::GetStyleProperty<sal_uInt16(RES_PAGEDESC)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    if(MID_PAGEDESC_PAGEDESCNAME != rEntry.nMemberId)
        return GetStyleProperty<HINT_BEGIN>(rEntry, rPropSet, rBase);
    // special handling for RES_PAGEDESC
    const SwFormatPageDesc* pItem =
        rBase.GetItemSet().GetItemIfSet(RES_PAGEDESC);
    if(!pItem)
        return uno::Any();
    const SwPageDesc* pDesc = pItem->GetPageDesc();
    if(!pDesc)
        return uno::Any();
    OUString aString;
    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, SwGetPoolIdFromName::PageDesc);
    return uno::Any(aString);
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_IS_AUTO_UPDATE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    switch(GetFamily())
    {
        case SfxStyleFamily::Para : return uno::Any(rBase.getNewBase()->GetCollection()->IsAutoUpdateOnDirectFormat());
        case SfxStyleFamily::Frame: return uno::Any(rBase.getNewBase()->GetFrameFormat()->IsAutoUpdateOnDirectFormat());
        default: return uno::Any();
    }
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_DISPLAY_NAME>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    return uno::Any(rBase.getNewBase()->GetName());
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_PARA_STYLE_CONDITIONS>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    static_assert(COND_COMMAND_COUNT == 28, "invalid size of command count?");
    uno::Sequence<beans::NamedValue> aSeq(COND_COMMAND_COUNT);
    sal_uInt16 nIndex = 0;
    for(auto& rNV : asNonConstRange(aSeq))
    {
        rNV.Name = GetCommandContextByIndex(nIndex++);
        rNV.Value <<= OUString();
    }
    SwFormat* pFormat = static_cast<SwDocStyleSheet*>(GetStyleSheetBase())->GetCollection();
    if(pFormat && RES_CONDTXTFMTCOLL == pFormat->Which())
    {
        const CommandStruct* pCmds = SwCondCollItem::GetCmds();
        beans::NamedValue* pSeq = aSeq.getArray();
        for(sal_uInt16 n = 0;  n < COND_COMMAND_COUNT; ++n)
        {
            const SwCollCondition* pCond = static_cast<SwConditionTextFormatColl*>(pFormat)->HasCondition(SwCollCondition(nullptr, pCmds[n].nCnd, pCmds[n].nSubCond));
            if(!pCond || !pCond->GetTextFormatColl())
                continue;
            // get programmatic style name from UI style name
            OUString aStyleName = pCond->GetTextFormatColl()->GetName();
            SwStyleNameMapper::FillProgName(aStyleName, aStyleName, lcl_GetSwEnumFromSfxEnum(GetFamily()));
            pSeq[n].Value <<= aStyleName;
        }
    }
    return uno::Any(aSeq);
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_CATEGORY>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    static const std::map<collectionbits_t, paragraphstyle_t> aUnoToCore = []{
        std::map<collectionbits_t, paragraphstyle_t> map;
        std::transform(std::begin(sParagraphStyleCategoryEntries), std::end(sParagraphStyleCategoryEntries), std::inserter(map, map.end()),
            [] (const ParagraphStyleCategoryEntry& rEntry) { return std::make_pair(rEntry.m_nCollectionBits, rEntry.m_eCategory); });
        return map;
    }();
    const sal_uInt16 nPoolId = rBase.getNewBase()->GetCollection()->GetPoolFormatId();
    const auto pUnoToCoreIt(aUnoToCore.find(COLL_GET_RANGE_BITS & nPoolId));
    if(pUnoToCoreIt == aUnoToCore.end())
        return uno::Any(sal_Int16(-1));
    return uno::Any(pUnoToCoreIt->second);
}
template<>
uno::Any SwXStyle::GetStyleProperty<SID_SWREGISTER_COLLECTION>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SwPageDesc *pPageDesc = rBase.getNewBase()->GetPageDesc();
    if(!pPageDesc)
        return uno::Any(OUString());
    const SwTextFormatColl* pCol = pPageDesc->GetRegisterFormatColl();
    if(!pCol)
        return uno::Any(OUString());
    OUString aName;
    SwStyleNameMapper::FillProgName(pCol->GetName(), aName, SwGetPoolIdFromName::TxtColl);
    return uno::Any(aName);
}
template<>
uno::Any SwXStyle::GetStyleProperty<sal_uInt16(RES_BACKGROUND)>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SfxItemSet& rSet = rBase.GetItemSet();
    const std::unique_ptr<SvxBrushItem> aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));
    uno::Any aResult;
    if(!aOriginalBrushItem->QueryValue(aResult, rEntry.nMemberId))
        SAL_WARN("sw.uno", "error getting attribute from RES_BACKGROUND.");
    return aResult;
}
template<>
uno::Any SwXStyle::GetStyleProperty<OWN_ATTR_FILLBMP_MODE>(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SfxItemSet& rSet = rBase.GetItemSet();
    if (rSet.Get(XATTR_FILLBMP_TILE).GetValue())
        return uno::Any(drawing::BitmapMode_REPEAT);
    if (rSet.Get(XATTR_FILLBMP_STRETCH).GetValue())
        return uno::Any(drawing::BitmapMode_STRETCH);
    return uno::Any(drawing::BitmapMode_NO_REPEAT);
}
template<>
uno::Any SwXStyle::GetStyleProperty<HINT_BEGIN>(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& /*rPropSet*/, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SfxItemSet& rSet = rBase.GetItemSet();
    uno::Any aResult;
    SfxItemPropertySet::getPropertyValue(rEntry, rSet, aResult);
    //
    // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
    if(rEntry.aType == cppu::UnoType<sal_Int16>::get() && aResult.getValueType() == cppu::UnoType<sal_Int32>::get())
        aResult <<= static_cast<sal_Int16>(aResult.get<sal_Int32>());
    // check for needed metric translation
    if(rEntry.nMoreFlags & PropertyMoreFlags::METRIC_ITEM && GetDoc())
    {
        const SfxItemPool& rPool = GetDoc()->GetAttrPool();
        const MapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));
        bool bAllowedConvert(true);
        // exception: If these ItemTypes are used, do not convert when these are negative
        // since this means they are intended as percent values
        if(XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
            bAllowedConvert = !aResult.has<sal_Int32>() || aResult.get<sal_Int32>() > 0;
        if(eMapUnit != MapUnit::Map100thMM && bAllowedConvert)
            SvxUnoConvertToMM(eMapUnit, aResult);
    }
    return aResult;
}

uno::Any SwXStyle::GetStyleProperty_Impl(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    using propertytype_t = decltype(rEntry.nWID);
    using coresetter_t = std::function<uno::Any(SwXStyle&, const SfxItemPropertyMapEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)>;
    static const std::map<propertytype_t, coresetter_t> aUnoToCore{
            // these explicit std::mem_fn() calls shouldn't be needed, but apparently MSVC is currently too stupid for C++11 again
            { FN_UNO_IS_PHYSICAL,            std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_IS_PHYSICAL>)            },
            { FN_UNO_HIDDEN,                 std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_HIDDEN>)                 },
            { FN_UNO_STYLE_INTEROP_GRAB_BAG, std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_STYLE_INTEROP_GRAB_BAG>) },
            { RES_PAPER_BIN,                 std::mem_fn(&SwXStyle::GetStyleProperty<sal_uInt16(RES_PAPER_BIN)>)                 },
            { FN_UNO_NUM_RULES,              std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_NUM_RULES>)              },
            { RES_PARATR_OUTLINELEVEL,       std::mem_fn(&SwXStyle::GetStyleProperty<sal_uInt16(RES_PARATR_OUTLINELEVEL)>)       },
            { FN_UNO_FOLLOW_STYLE,           std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_FOLLOW_STYLE>)           },
            { FN_UNO_LINK_STYLE,             std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_LINK_STYLE>)             },
            { RES_PAGEDESC,                  std::mem_fn(&SwXStyle::GetStyleProperty<sal_uInt16(RES_PAGEDESC)>)                  },
            { FN_UNO_IS_AUTO_UPDATE,         std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_IS_AUTO_UPDATE>)         },
            { FN_UNO_DISPLAY_NAME,           std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_DISPLAY_NAME>)           },
            { FN_UNO_PARA_STYLE_CONDITIONS,  std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_PARA_STYLE_CONDITIONS>)  },
            { FN_UNO_CATEGORY,               std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_CATEGORY>)               },
            { SID_SWREGISTER_COLLECTION,     std::mem_fn(&SwXStyle::GetStyleProperty<SID_SWREGISTER_COLLECTION>)     },
            { RES_BACKGROUND,                std::mem_fn(&SwXStyle::GetStyleProperty<sal_uInt16(RES_BACKGROUND)>)                },
            { OWN_ATTR_FILLBMP_MODE,         std::mem_fn(&SwXStyle::GetStyleProperty<OWN_ATTR_FILLBMP_MODE>)         }
    };
    const auto pUnoToCoreIt(aUnoToCore.find(rEntry.nWID));
    if(pUnoToCoreIt != aUnoToCore.end())
        return pUnoToCoreIt->second(*this, rEntry, rPropSet, rBase);
    return GetStyleProperty<HINT_BEGIN>(rEntry, rPropSet, rBase);
}

uno::Any SwXStyle::GetPropertyValue_Impl(const SfxItemPropertySet* pPropSet, SwStyleBase_Impl& rBase, const OUString& rPropertyName)
{
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rPropertyName);
    if(!pEntry || (!m_bIsConditional && rPropertyName == UNO_NAME_PARA_STYLE_CONDITIONS))
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, getXWeak());
    if(m_pBasePool)
        return GetStyleProperty_Impl(*pEntry, *pPropSet, rBase);
    const uno::Any* pAny = nullptr;
    m_pPropertiesImpl->GetProperty(rPropertyName, pAny);
    if(pAny->hasValue())
        return *pAny;
    uno::Any aValue;
    switch(m_rEntry.family())
    {
        case SfxStyleFamily::Pseudo:
            throw uno::RuntimeException("No default value for: " + rPropertyName);
        break;
        case SfxStyleFamily::Para:
        case SfxStyleFamily::Page:
            aValue = m_xStyleData->getPropertyValue(rPropertyName);
        break;
        case SfxStyleFamily::Char:
        case SfxStyleFamily::Frame:
        {
            if(pEntry->nWID < POOLATTR_BEGIN || pEntry->nWID >= RES_UNKNOWNATR_END)
                throw uno::RuntimeException("No default value for: " + rPropertyName);
            SwFormat* pFormat;
            if(m_rEntry.family() == SfxStyleFamily::Char)
                pFormat = m_pDoc->GetDfltCharFormat();
            else
                pFormat = m_pDoc->GetDfltFrameFormat();
            const SwAttrPool* pPool = pFormat->GetAttrSet().GetPool();
            const SfxPoolItem& rItem = pPool->GetUserOrPoolDefaultItem(pEntry->nWID);
            rItem.QueryValue(aValue, pEntry->nMemberId);
        }
        break;
        default:
        ;
    }
    return aValue;
}

uno::Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pBasePool && !m_bIsDescriptor)
        throw uno::RuntimeException();
    sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName, &m_pDoc->GetDfltTextFormatColl()->GetAttrSet()); // add pDfltTextFormatColl as parent
    return GetPropertyValue_Impl(pPropSet, aBase, rPropertyName);
}

uno::Sequence<uno::Any> SwXStyle::getPropertyValues(const uno::Sequence<OUString>& rPropertyNames)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pBasePool && !m_bIsDescriptor)
        throw uno::RuntimeException();
    sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName, &m_pDoc->GetDfltTextFormatColl()->GetAttrSet()); // add pDfltTextFormatColl as parent
    uno::Sequence<uno::Any> aValues(rPropertyNames.getLength());
    auto aValuesRange = asNonConstRange(aValues);
    // workaround for bad designed API
    try
    {
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
            aValuesRange[nProp] = GetPropertyValue_Impl(pPropSet, aBase, rPropertyNames[nProp]);
    }
    catch(beans::UnknownPropertyException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException("Unknown property exception caught",
                getXWeak(), anyEx );
    }
    catch(lang::WrappedTargetException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("WrappedTargetException caught",
                getXWeak(), anyEx );
    }
    return aValues;
}

void SwXStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl(aProperties, aValues);
}

beans::PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aNames{rPropertyName};
    uno::Sequence<beans::PropertyState> aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

// allow to retarget the SfxItemSet working on, default correctly. Only
// use pSourceSet below this point (except in header/footer processing)
static const SfxItemSet* lcl_GetItemsetForProperty(const SfxItemSet& rSet, SfxStyleFamily eFamily, std::u16string_view rPropertyName)
{
    if(eFamily != SfxStyleFamily::Page)
        return &rSet;
    const bool isFooter = o3tl::starts_with(rPropertyName, u"Footer");
    if(!isFooter && !o3tl::starts_with(rPropertyName, u"Header") && rPropertyName != UNO_NAME_FIRST_IS_SHARED)
        return &rSet;
    const SvxSetItem* pSetItem;
    if(!lcl_GetHeaderFooterItem(rSet, rPropertyName, isFooter, pSetItem))
        return nullptr;
    return &pSetItem->GetItemSet();
}
uno::Sequence<beans::PropertyState> SwXStyle::getPropertyStates(const uno::Sequence<OUString>& rPropertyNames)
{
    SolarMutexGuard aGuard;
    uno::Sequence<beans::PropertyState> aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    if(!m_pBasePool)
        throw uno::RuntimeException();
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName, m_rEntry.family());

    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase)
        throw uno::RuntimeException();

    const OUString* pNames = rPropertyNames.getConstArray();
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const SfxItemSet& rSet = xStyle->GetItemSet();

    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); ++i)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(sPropName);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, getXWeak());

        if (FN_UNO_NUM_RULES == pEntry->nWID || FN_UNO_FOLLOW_STYLE == pEntry->nWID
            || pEntry->nWID == FN_UNO_LINK_STYLE)
        {
            // handle NumRules first, done
            pStates[i] = beans::PropertyState_DIRECT_VALUE;
            continue;
        }
        const SfxItemSet* pSourceSet = lcl_GetItemsetForProperty(rSet, m_rEntry.family(), sPropName);
        if(!pSourceSet)
        {
            // if no SetItem, value is ambiguous and we are done
            pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
            continue;
        }
        switch(pEntry->nWID)
        {
            case OWN_ATTR_FILLBMP_MODE:
            {
                if(SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                    || SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_TILE, false))
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                }
            }
            break;
            case XATTR_FILLSTYLE:
            {
                if (m_rEntry.family() == SfxStyleFamily::Frame
                    && xStyle->GetFrameFormat()->DerivedFrom() == GetDoc()->GetDfltFrameFormat())
                {   // tdf#156155 mpDfltFrameFormat is the parent, but because
                    // it IsDefault() it is not enumerated/exported as a style
                    // to ODF, so export its one important value here.
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = SfxItemPropertySet::getPropertyState(*pEntry, *pSourceSet);
                }
            }
            break;
            case RES_BACKGROUND:
            {
                // for FlyFrames we need to mark the used properties from type RES_BACKGROUND
                // as beans::PropertyState_DIRECT_VALUE to let users of this property call
                // getPropertyValue where the member properties will be mapped from the
                // fill attributes to the according SvxBrushItem entries
                if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*pSourceSet, pEntry->nMemberId))
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                }
            }
            break;
            default:
            {
                pStates[i] = SfxItemPropertySet::getPropertyState(*pEntry, *pSourceSet);

                if(SfxStyleFamily::Page == m_rEntry.family() && SID_ATTR_PAGE_SIZE == pEntry->nWID && beans::PropertyState_DIRECT_VALUE == pStates[i])
                {
                    const SvxSizeItem& rSize = rSet.Get(SID_ATTR_PAGE_SIZE);
                    sal_uInt8 nMemberId = pEntry->nMemberId & 0x7f;

                    if((LONG_MAX == rSize.GetSize().Width() && (MID_SIZE_WIDTH == nMemberId || MID_SIZE_SIZE == nMemberId)) ||
                        (LONG_MAX == rSize.GetSize().Height() && MID_SIZE_HEIGHT == nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
            }
        }
    }
    return aRet;
}

void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
{
    const uno::Sequence<OUString> aSequence(&rPropertyName, 1);
    setPropertiesToDefault(aSequence);
}

static SwFormat* lcl_GetFormatForStyle(SwDoc const * pDoc, const rtl::Reference<SwDocStyleSheet>& xStyle, const SfxStyleFamily eFamily)
{
    if(!xStyle.is())
        return nullptr;
    switch(eFamily)
    {
        case SfxStyleFamily::Char: return xStyle->GetCharFormat();
        case SfxStyleFamily::Para: return xStyle->GetCollection();
        case SfxStyleFamily::Frame: return xStyle->GetFrameFormat();
        case SfxStyleFamily::Page:
        {
            SwPageDesc* pDesc(pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName()));
            if(pDesc)
                return &pDesc->GetMaster();
        }
        break;
        default: ;
    }
    return nullptr;
}

void SAL_CALL SwXStyle::setPropertiesToDefault(const uno::Sequence<OUString>& aPropertyNames)
{
    SolarMutexGuard aGuard;
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(GetStyleSheetBase())));
    SwFormat* pTargetFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.family());
    if(!pTargetFormat)
    {
        if(!m_bIsDescriptor)
            return;
        for(const auto& rName : aPropertyNames)
            m_pPropertiesImpl->ClearProperty(rName);
        return;
    }
    const sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    for(const auto& rName : aPropertyNames)
    {
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rName);
        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rName, getXWeak());
        if (pEntry->nWID == FN_UNO_FOLLOW_STYLE || pEntry->nWID == FN_UNO_LINK_STYLE
            || pEntry->nWID == FN_UNO_NUM_RULES)
            throw uno::RuntimeException("Cannot reset: " + rName, getXWeak());
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException("setPropertiesToDefault: property is read-only: " + rName, getXWeak());
        if(pEntry->nWID == RES_PARATR_OUTLINELEVEL)
        {
            static_cast<SwTextFormatColl*>(pTargetFormat)->DeleteAssignmentToListLevelOfOutlineStyle();
            continue;
        }
        pTargetFormat->ResetFormatAttr(pEntry->nWID);
        if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
        {
            //
            SwDoc* pDoc = pTargetFormat->GetDoc();
            SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aSet(pDoc->GetAttrPool());
            aSet.SetParent(&pTargetFormat->GetAttrSet());

            aSet.ClearItem(XATTR_FILLBMP_STRETCH);
            aSet.ClearItem(XATTR_FILLBMP_TILE);

            pTargetFormat->SetFormatAttr(aSet);
        }
    }
}

void SAL_CALL SwXStyle::setAllPropertiesToDefault()
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        m_pPropertiesImpl->ClearAllProperties();
        return;
    }
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(GetStyleSheetBase())));
    if(!xStyle.is())
        throw uno::RuntimeException();
    if(SfxStyleFamily::Page == m_rEntry.family())
    {
        size_t nPgDscPos(0);
        SwPageDesc* pDesc = m_pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName(), &nPgDscPos);
        SwFormat* pPageFormat(nullptr);
        if(pDesc)
        {
            pPageFormat = &pDesc->GetMaster();
            pDesc->SetUseOn(UseOnPage::All);
        }
        else
            pPageFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.family());
        SwPageDesc& rPageDesc = m_pDoc->GetPageDesc(nPgDscPos);
        rPageDesc.ResetAllMasterAttr();

        pPageFormat->SetPageFormatToDefault();
        SwPageDesc* pStdPgDsc = m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD);
        std::shared_ptr<SwFormatFrameSize> aFrameSz(std::make_shared<SwFormatFrameSize>(SwFrameSize::Fixed));

        if(RES_POOLPAGE_STANDARD == rPageDesc.GetPoolFormatId())
        {
            if(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))
            {
                const Size aPhysSize( SvxPaperInfo::GetPaperSize(
                    static_cast<Printer*>(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))));
                aFrameSz->SetSize(aPhysSize);
            }
            else
            {
                aFrameSz->SetSize(SvxPaperInfo::GetDefaultPaperSize());
            }

        }
        else
        {
            aFrameSz.reset(pStdPgDsc->GetMaster().GetFrameSize().Clone());
        }

        if(pStdPgDsc->GetLandscape())
        {
            SwTwips nTmp = aFrameSz->GetHeight();
            aFrameSz->SetHeight(aFrameSz->GetWidth());
            aFrameSz->SetWidth(nTmp);
        }

        pPageFormat->SetFormatAttr(*aFrameSz);
        m_pDoc->ChgPageDesc(nPgDscPos, m_pDoc->GetPageDesc(nPgDscPos));
        return;
    }
    if(SfxStyleFamily::Para == m_rEntry.family())
    {
        if(xStyle->GetCollection())
            xStyle->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
    }
    SwFormat* const pTargetFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.family());
    if(!pTargetFormat)
        return;
    pTargetFormat->ResetAllFormatAttr();
}

uno::Sequence<uno::Any> SAL_CALL SwXStyle::getPropertyDefaults(const uno::Sequence<OUString>& aPropertyNames)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet(nCount);
    if(!nCount)
        return aRet;
    auto pRet = aRet.getArray();
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    const sal_uInt16 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.propMapType();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

    const SfxItemSet &rSet = xStyle->GetItemSet(), *pParentSet = rSet.GetParent();
    for(sal_Int32 i = 0; i < nCount; ++i)
    {
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(aPropertyNames[i]);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + aPropertyNames[i], getXWeak());
        // these cannot be in an item set, especially not the
        // parent set, so the default value is void
        if (pEntry->nWID >= RES_UNKNOWNATR_END)
            continue;

        if(pParentSet)
        {
            aSwMapProvider.GetPropertySet(nPropSetId)->getPropertyValue(aPropertyNames[i], *pParentSet, pRet[i]);
        }
        else if(pEntry->nWID != rSet.GetPool()->GetSlotId(pEntry->nWID))
        {
            const SfxPoolItem& rItem = rSet.GetPool()->GetUserOrPoolDefaultItem(pEntry->nWID);
            rItem.QueryValue(pRet[i], pEntry->nMemberId);
        }
    }
    return aRet;
}

uno::Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
{
    const uno::Sequence<OUString> aSequence(&rPropertyName, 1);
    return getPropertyDefaults(aSequence)[0];
}

void SwXStyle::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if((rHint.GetId() == SfxHintId::Dying) || (rHint.GetId() == SfxHintId::StyleSheetErased))
    {
        m_pBasePool = nullptr;
        SfxListener::EndListening(rBC);
    }
    else if(rHint.GetId() == SfxHintId::StyleSheetChanged)
    {
        SfxStyleSheetBasePool& rBP = static_cast<SfxStyleSheetBasePool&>(rBC);
        SfxStyleSheetBase* pOwnBase = rBP.Find(m_sStyleName, m_rEntry.family());
        if(!pOwnBase)
        {
            SfxListener::EndListening(rBC);
            Invalidate();
        }
    }
}

void SwXStyle::Invalidate()
{
    m_sStyleName.clear();
    m_pBasePool = nullptr;
    m_pDoc = nullptr;
    m_xStyleData.clear();
    m_xStyleFamily.clear();
}

SwXPageStyle::SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, const OUString& rStyleName)
    : SwXStyle(&rPool, SfxStyleFamily::Page, pDocSh->GetDoc(), rStyleName)
{ }

SwXPageStyle::SwXPageStyle(SwDocShell* pDocSh)
    : SwXStyle(pDocSh->GetDoc(), SfxStyleFamily::Page)
{ }

void SwXStyle::PutItemToSet(const SvxSetItem* pSetItem, const SfxItemPropertySet& rPropSet, const SfxItemPropertyMapEntry& rEntry, const uno::Any& rVal, SwStyleBase_Impl& rBaseImpl)
{
    // create a new SvxSetItem and get it's ItemSet as new target
    std::unique_ptr<SvxSetItem> pNewSetItem(pSetItem->Clone());
    SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

    // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
    rSetSet.SetParent(&m_pDoc->GetDfltFrameFormat()->GetAttrSet());

    // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
    // default method to set the property
    {
        SwStyleBase_Impl::ItemSetOverrider o(rBaseImpl, &rSetSet);
        SetStyleProperty(rEntry, rPropSet, rVal, rBaseImpl);
    }

    // reset parent at ItemSet from SetItem
    rSetSet.SetParent(nullptr);

    // set the new SvxSetItem at the real target and delete it
    rBaseImpl.GetItemSet().Put(std::move(pNewSetItem));
}

void SwXPageStyle::SetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBaseImpl(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); // add pDfltFrameFormat as parent
    if(!m_pBasePool)
    {
        if(!IsDescriptor())
            throw uno::RuntimeException();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
            if(!m_pPropertiesImpl->SetProperty(rPropertyNames[nProp], rValues[nProp]))
                throw lang::IllegalArgumentException();
        return;
    }
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
    {
        const OUString& rPropName = rPropertyNames[nProp];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rPropName);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rPropName, getXWeak());
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropName, getXWeak());

        const bool bHeader(rPropName.startsWith("Header"));
        const bool bFooter(rPropName.startsWith("Footer"));
        const bool bFirstIsShared(rPropName == UNO_NAME_FIRST_IS_SHARED);
        if(bHeader || bFooter || bFirstIsShared)
        {
            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // it is a Header/Footer entry, access the SvxSetItem containing it's information
                    const SvxSetItem* pSetItem = nullptr;
                    if (lcl_GetHeaderFooterItem(aBaseImpl.GetItemSet(), rPropName, bFooter, pSetItem))
                    {
                        PutItemToSet(pSetItem, *pPropSet, *pEntry, rValues[nProp], aBaseImpl);

                        if (pEntry->nWID == SID_ATTR_PAGE_SHARED_FIRST)
                        {
                            // Need to add this to the other as well
                            pSetItem = aBaseImpl.GetItemSet().GetItemIfSet(
                                        bFooter ? SID_ATTR_PAGE_HEADERSET : SID_ATTR_PAGE_FOOTERSET,
                                        false);
                            if (pSetItem)
                            {
                                PutItemToSet(pSetItem, *pPropSet, *pEntry, rValues[nProp], aBaseImpl);
                            }
                        }
                    }
                    else if(pEntry->nWID == SID_ATTR_PAGE_ON && rValues[nProp].get<bool>())
                    {
                        // Header/footer gets switched on, create defaults and the needed SfxSetItem
                        SfxItemSetFixed
                            <RES_FRMATR_BEGIN,RES_FRMATR_END - 1,            // [82

                            // FillAttribute support
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,              // [1014

                            SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,    // [10023
                            SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,          // [10051
                            SID_ATTR_PAGE_ON,SID_ATTR_PAGE_SHARED,          // [10060
                            SID_ATTR_PAGE_SHARED_FIRST,SID_ATTR_PAGE_SHARED_FIRST>
                                aTempSet(*aBaseImpl.GetItemSet().GetPool());

                        // set correct parent to get the XFILL_NONE FillStyle as needed
                        aTempSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_ON, true));
                        constexpr tools::Long constTwips_5mm = o3tl::toTwips(5, o3tl::Length::mm);
                        aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(constTwips_5mm, constTwips_5mm)));
                        aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                        aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, true));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED_FIRST, true));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, true));

                        SvxSetItem aNewSetItem(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, aTempSet);
                        aBaseImpl.GetItemSet().Put(aNewSetItem);
                    }
                }
                continue;
                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                if(bFirstIsShared) // only special handling for headers/footers here
                    break;
                {
                    const SvxSetItem* pSetItem =
                        aBaseImpl.GetItemSet().GetItemIfSet(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false);

                    if(pSetItem)
                    {
                        // create a new SvxSetItem and get it's ItemSet as new target
                        std::unique_ptr<SvxSetItem> pNewSetItem(pSetItem->Clone());
                        SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

                        // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
                        rSetSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                        // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
                        // default method to set the property
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBaseImpl, &rSetSet);
                            SetStyleProperty(*pEntry, *pPropSet, rValues[nProp], aBaseImpl);
                        }

                        // reset parent at ItemSet from SetItem
                        rSetSet.SetParent(nullptr);

                        // set the new SvxSetItem at the real target and delete it
                        aBaseImpl.GetItemSet().Put(std::move(pNewSetItem));
                    }
                }
                continue;
                default: ;
            }
        }
        switch(pEntry->nWID)
        {
            case SID_ATTR_PAGE_DYNAMIC:
            case SID_ATTR_PAGE_SHARED:
            case SID_ATTR_PAGE_SHARED_FIRST:
            case SID_ATTR_PAGE_ON:
            case RES_HEADER_FOOTER_EAT_SPACING:
                // these slots are exclusive to Header/Footer, thus this is an error
                throw beans::UnknownPropertyException("Unknown property: " + rPropName, getXWeak());
            case FN_UNO_HEADER:
            case FN_UNO_HEADER_LEFT:
            case FN_UNO_HEADER_RIGHT:
            case FN_UNO_HEADER_FIRST:
            case FN_UNO_FOOTER:
            case FN_UNO_FOOTER_LEFT:
            case FN_UNO_FOOTER_RIGHT:
            case FN_UNO_FOOTER_FIRST:
                throw lang::IllegalArgumentException();
            case FN_PARAM_FTN_INFO:
            {
                const SwPageFootnoteInfoItem& rItem = aBaseImpl.GetItemSet().Get(FN_PARAM_FTN_INFO);
                std::unique_ptr<SfxPoolItem> pNewFootnoteItem(rItem.Clone());
                if(!pNewFootnoteItem->PutValue(rValues[nProp], pEntry->nMemberId))
                    throw lang::IllegalArgumentException();
                aBaseImpl.GetItemSet().Put(std::move(pNewFootnoteItem));
                break;
            }
            default:
            {
                SetStyleProperty(*pEntry, *pPropSet, rValues[nProp], aBaseImpl);
                break;
            }
        }
    }

    if(aBaseImpl.HasItemSet())
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

        if (undoGuard.UndoWasEnabled())
        {
            // Fix i64460: as long as Undo of page styles with header/footer causes trouble...
            GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
        }

        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
    }
}

void SwXPageStyle::setPropertyValues(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl(rPropertyNames, rValues);
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

static rtl::Reference<SwXHeadFootText> lcl_makeHeaderFooter(const sal_uInt16 nRes, const bool bHeader, SwFrameFormat const*const pFrameFormat)
{
    if (!pFrameFormat)
        return nullptr;
    const SfxItemSet& rSet = pFrameFormat->GetAttrSet();
    const SfxPoolItem* pItem;
    if(SfxItemState::SET != rSet.GetItemState(nRes, true, &pItem))
        return nullptr;
    SwFrameFormat* const pHeadFootFormat = bHeader
        ? static_cast<SwFormatHeader*>(const_cast<SfxPoolItem*>(pItem))->GetHeaderFormat()
        : static_cast<SwFormatFooter*>(const_cast<SfxPoolItem*>(pItem))->GetFooterFormat();
    if(!pHeadFootFormat)
        return nullptr;
    return SwXHeadFootText::CreateXHeadFootText(*pHeadFootFormat, bHeader);
}

uno::Sequence<uno::Any> SwXPageStyle::GetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    sal_Int32 nLength = rPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet (nLength);
    auto aRetRange = asNonConstRange(aRet);
    if(!m_pBasePool)
    {
        if(!IsDescriptor())
            throw uno::RuntimeException();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
        {
            const uno::Any* pAny = nullptr;
            m_pPropertiesImpl->GetProperty(rPropertyNames[nProp], pAny);
            if (!pAny->hasValue())
                aRetRange[nProp] = m_xStyleData->getPropertyValue(rPropertyNames[nProp]);
            else
                aRetRange[nProp] = *pAny;
        }
        return aRet;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBase(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); // add pDfltFrameFormat as parent
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    for(sal_Int32 nProp = 0; nProp < nLength; ++nProp)
    {
        const OUString& rPropName = rPropertyNames[nProp];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rPropName);

        if (!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rPropName, getXWeak() );
        const bool bHeader(rPropName.startsWith("Header"));
        const bool bFooter(rPropName.startsWith("Footer"));
        const bool bFirstIsShared(rPropName == UNO_NAME_FIRST_IS_SHARED);
        if(bHeader || bFooter || bFirstIsShared)
        {
            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // slot is a Header/Footer slot
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SvxSetItem* pSetItem;

                    if (lcl_GetHeaderFooterItem(rSet, rPropName, bFooter, pSetItem))
                    {
                        // get from SfxItemSet of the corresponding SfxSetItem
                        const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBase, &const_cast< SfxItemSet& >(rSetSet));
                            aRetRange[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
                        }
                    }
                    else if(pEntry->nWID == SID_ATTR_PAGE_ON)
                    {
                        // header/footer is not available, thus off. Default is <false>, though
                        aRetRange[nProp] <<= false;
                    }
                }
                continue;
                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                if(bFirstIsShared) // only special handling for headers/footers here
                    break;
                {
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SvxSetItem* pSetItem =
                        rSet.GetItemIfSet(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false);
                    if(pSetItem)
                    {
                        // set at SfxItemSet of the corresponding SfxSetItem
                        const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBase, &const_cast<SfxItemSet&>(rSetSet));
                            aRetRange[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
                        }
                    }
                }
                continue;
                default: ;
            }
        }
        switch(pEntry->nWID)
        {
            // these slots are exclusive to Header/Footer, thus this is an error
            case SID_ATTR_PAGE_DYNAMIC:
            case SID_ATTR_PAGE_SHARED:
            case SID_ATTR_PAGE_SHARED_FIRST:
            case SID_ATTR_PAGE_ON:
            case RES_HEADER_FOOTER_EAT_SPACING:
                throw beans::UnknownPropertyException( "Unknown property: " + rPropName, getXWeak() );
            case FN_UNO_HEADER:
            case FN_UNO_HEADER_LEFT:
            case FN_UNO_HEADER_FIRST:
            case FN_UNO_HEADER_RIGHT:
            case FN_UNO_FOOTER:
            case FN_UNO_FOOTER_LEFT:
            case FN_UNO_FOOTER_FIRST:
            case FN_UNO_FOOTER_RIGHT:
            {
                bool bLeft(false);
                bool bFirst(false);
                sal_uInt16 nRes = 0;
                switch(pEntry->nWID)
                {
                    case FN_UNO_HEADER:       nRes = RES_HEADER; break;
                    case FN_UNO_HEADER_LEFT:  nRes = RES_HEADER; bLeft = true;  break;
                    case FN_UNO_HEADER_FIRST: nRes = RES_HEADER; bFirst = true; break;
                    case FN_UNO_HEADER_RIGHT: nRes = RES_HEADER; break;
                    case FN_UNO_FOOTER:       nRes = RES_FOOTER; break;
                    case FN_UNO_FOOTER_LEFT:  nRes = RES_FOOTER; bLeft = true;  break;
                    case FN_UNO_FOOTER_FIRST: nRes = RES_FOOTER; bFirst = true; break;
                    case FN_UNO_FOOTER_RIGHT: nRes = RES_FOOTER; break;
                    default: ;
                }

                const SwPageDesc* pDesc = aBase.GetOldPageDesc();
                assert(pDesc);
                const SwFrameFormat* pFrameFormat = nullptr;
                bool bShare = (nRes == RES_HEADER && pDesc->IsHeaderShared()) || (nRes == RES_FOOTER && pDesc->IsFooterShared());
                bool bShareFirst = pDesc->IsFirstShared();
                // TextLeft returns the left content if there is one,
                // Text and TextRight return the master content.
                // TextRight does the same as Text and is for
                // compatibility only.
                if(bLeft && !bShare)
                    pFrameFormat = &pDesc->GetLeft();
                else if(bFirst && !bShareFirst)
                {
                    pFrameFormat = &pDesc->GetFirstMaster();
                    // no need to make GetFirstLeft() accessible
                    // since it is always shared
                }
                else
                    pFrameFormat = &pDesc->GetMaster();
                const uno::Reference<text::XText> xRet = lcl_makeHeaderFooter(nRes, nRes == RES_HEADER, pFrameFormat);
                if (xRet.is())
                    aRetRange[nProp] <<= xRet;
            }
            break;
            case FN_PARAM_FTN_INFO:
            {
                rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
                const SfxItemSet& rSet = xStyle->GetItemSet();
                const SfxPoolItem& rItem = rSet.Get(FN_PARAM_FTN_INFO);
                rItem.QueryValue(aRetRange[nProp], pEntry->nMemberId);
            }
            break;
            default:
                aRetRange[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
        }
    }
    return aRet;
}

uno::Sequence<uno::Any> SwXPageStyle::getPropertyValues(const uno::Sequence<OUString>& rPropertyNames)
{
    SolarMutexGuard aGuard;
    uno::Sequence<uno::Any> aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl(rPropertyNames);
    }
    catch(beans::UnknownPropertyException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("Unknown property exception caught",
                getXWeak(), anyEx );
    }
    catch(lang::WrappedTargetException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("WrappedTargetException caught",
                getXWeak(), anyEx );
    }

    return aValues;
}

uno::Any SwXPageStyle::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties)[0];
}

void SwXPageStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);

    // Trick: if the Domain Mapper changes the props of shared header/footer,
    // store the old ones in time for later use.
    const bool bIsHeader = rPropertyName == UNO_NAME_HEADER_IS_SHARED;
    const bool bIsFooter = rPropertyName == UNO_NAME_FOOTER_IS_SHARED;
    if ((bIsFooter || bIsHeader) && rValue == uno::Any(true))
    {
        // Find the matching page descriptor
        for (size_t i = 0; i < GetDoc()->GetPageDescCnt(); i++)
        {
            auto pPageDesc = &GetDoc()->GetPageDesc(i);
            // If we have the right page descriptor stash the necessary formats in import time.
            if (pPageDesc->GetName() == GetStyleName())
            {
                auto pLeftHeader = pPageDesc->GetLeft().GetHeader().GetHeaderFormat();
                if (bIsHeader && pLeftHeader)
                {
                    pPageDesc->StashFrameFormat(pPageDesc->GetLeft(), true, true, false);
                    pPageDesc->StashFrameFormat(pPageDesc->GetFirstMaster(), true, false, true);
                    pPageDesc->StashFrameFormat(pPageDesc->GetFirstLeft(), true, true, true);
                }
                auto pLeftFooter = pPageDesc->GetLeft().GetFooter().GetFooterFormat();
                if (bIsFooter && pLeftFooter)
                {
                    pPageDesc->StashFrameFormat(pPageDesc->GetLeft(), false, true, false);
                    pPageDesc->StashFrameFormat(pPageDesc->GetFirstMaster(), false, false, true);
                    pPageDesc->StashFrameFormat(pPageDesc->GetFirstLeft(), false, true, true);
                }
            }
        }
    }
    // And set the props... as we did it before.
    SetPropertyValues_Impl(aProperties, aValues);
}

SwXFrameStyle::SwXFrameStyle(SwDoc *pDoc)
    : SwXFrameStyle_Base(pDoc, SfxStyleFamily::Frame, false)
{ }

void SwXFrameStyle::SetItem(sal_uInt16 eAtr, const SfxPoolItem& rItem)
{
    assert(eAtr >= RES_FRMATR_BEGIN && eAtr < RES_FRMATR_END);
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        return;
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    SfxItemSet& rStyleSet = xStyle->GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), sal_uInt16(eAtr), sal_uInt16(eAtr));
    aSet.Put(rItem);
    xStyle->SetItemSet(aSet);
}

const SfxPoolItem* SwXFrameStyle::GetItem(sal_uInt16 eAtr)
{
    assert(eAtr >= RES_FRMATR_BEGIN && eAtr < RES_FRMATR_END);
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        return nullptr;
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    return &xStyle->GetItemSet().Get(eAtr);
}

uno::Reference<container::XNameReplace> SwXFrameStyle::getEvents()
{
    return new SwFrameStyleEventDescriptor(*this);
}

// Already implemented autostyle families: 3
#define AUTOSTYLE_FAMILY_COUNT 3
const IStyleAccess::SwAutoStyleFamily aAutoStyleByIndex[] =
{
    IStyleAccess::AUTO_STYLE_CHAR,
    IStyleAccess::AUTO_STYLE_RUBY,
    IStyleAccess::AUTO_STYLE_PARA
};

class SwAutoStylesEnumImpl
{
    std::vector<std::shared_ptr<SfxItemSet>> mAutoStyles;
    std::vector<std::shared_ptr<SfxItemSet>>::iterator m_aIter;
    SwDoc& m_rDoc;
    IStyleAccess::SwAutoStyleFamily m_eFamily;
public:
    SwAutoStylesEnumImpl( SwDoc& rInitDoc, IStyleAccess::SwAutoStyleFamily eFam );
    bool hasMoreElements() { return m_aIter != mAutoStyles.end(); }
    std::shared_ptr<SfxItemSet> const & nextElement() { return *(m_aIter++); }
    IStyleAccess::SwAutoStyleFamily getFamily() const { return m_eFamily; }
    SwDoc& getDoc() const { return m_rDoc; }
};

SwXAutoStyles::SwXAutoStyles(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()), m_pDocShell( &rDocShell )
{
}

SwXAutoStyles::~SwXAutoStyles()
{
}

sal_Int32 SwXAutoStyles::getCount()
{
    return AUTOSTYLE_FAMILY_COUNT;
}

uno::Any SwXAutoStyles::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(nIndex < 0 || nIndex >= AUTOSTYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(!IsValid())
        throw uno::RuntimeException();

    uno::Reference< style::XAutoStyleFamily >  aRef;
    IStyleAccess::SwAutoStyleFamily nType = aAutoStyleByIndex[nIndex];
    switch( nType )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            if(!m_xAutoCharStyles.is())
                m_xAutoCharStyles = new SwXAutoStyleFamily(m_pDocShell, nType);
            aRef = m_xAutoCharStyles;
        }
        break;
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            if(!m_xAutoRubyStyles.is())
                m_xAutoRubyStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
            aRef = m_xAutoRubyStyles;
        }
        break;
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            if(!m_xAutoParaStyles.is())
                m_xAutoParaStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
            aRef = m_xAutoParaStyles;
        }
        break;

        default:
            ;
    }
    aRet <<= aRef;

    return aRet;
}

uno::Type SwXAutoStyles::getElementType(  )
{
    return cppu::UnoType<style::XAutoStyleFamily>::get();
}

sal_Bool SwXAutoStyles::hasElements(  )
{
    return true;
}

uno::Any SwXAutoStyles::getByName(const OUString& Name)
{
    uno::Any aRet;
    if(Name == "CharacterStyles")
        aRet = getByIndex(0);
    else if(Name == "RubyStyles")
        aRet = getByIndex(1);
    else if(Name == "ParagraphStyles")
        aRet = getByIndex(2);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXAutoStyles::getElementNames()
{
    uno::Sequence< OUString > aNames(AUTOSTYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = "CharacterStyles";
    pNames[1] = "RubyStyles";
    pNames[2] = "ParagraphStyles";
    return aNames;
}

sal_Bool SwXAutoStyles::hasByName(const OUString& Name)
{
    if( Name == "CharacterStyles" ||
        Name == "RubyStyles" ||
        Name == "ParagraphStyles" )
        return true;
    else
        return false;
}

SwXAutoStyleFamily::SwXAutoStyleFamily(SwDocShell* pDocSh, IStyleAccess::SwAutoStyleFamily nFamily) :
    m_pDocShell( pDocSh ), m_eFamily(nFamily)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    StartListening(pDocSh->GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
}

SwXAutoStyleFamily::~SwXAutoStyleFamily()
{
}

void SwXAutoStyleFamily::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        m_pDocShell = nullptr;
}

std::shared_ptr<SfxItemSet>
PropValuesToAutoStyleItemSet(SwDoc& rDoc, IStyleAccess::SwAutoStyleFamily eFamily,
                             const uno::Sequence<beans::PropertyValue>& Values, SwAttrSet& aSet)
{
    const SfxItemPropertySet* pPropSet = nullptr;
    switch( eFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_RUBY_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARA_AUTO_STYLE);
            break;
        }
        default: ;
    }

    if( !pPropSet)
        throw uno::RuntimeException();

    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == eFamily);

    if(!bTakeCareOfDrawingLayerFillStyle)
    {
        for( const beans::PropertyValue& rValue : Values )
        {
            try
            {
                pPropSet->setPropertyValue( rValue.Name, rValue.Value, aSet );
            }
            catch (const beans::UnknownPropertyException &)
            {
                OSL_FAIL( "Unknown property" );
            }
            catch (const lang::IllegalArgumentException &)
            {
                OSL_FAIL( "Illegal argument" );
            }
        }
    }
    else
    {
        // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
        // to make cases in RES_BACKGROUND work correct; target *is* a style
        // where this is the case
        aSet.SetParent(&rDoc.GetDfltTextFormatColl()->GetAttrSet());

        // here the used DrawingLayer FillStyles are imported when family is
        // equal to IStyleAccess::AUTO_STYLE_PARA, thus we will need to serve the
        // used slots functionality here to do this correctly
        const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

        for( const beans::PropertyValue& rValue : Values )
        {
            const OUString& rPropName = rValue.Name;
            uno::Any aValue(rValue.Value);
            const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rPropName);

            if (!pEntry)
            {
                SAL_WARN("sw.core", "SwXAutoStyleFamily::insertStyle: Unknown property: " << rPropName);
                continue;
            }

            const sal_uInt8 nMemberId(pEntry->nMemberId);
            bool bDone(false);

            // check for needed metric translation
            if(pEntry->nMoreFlags & PropertyMoreFlags::METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aValue >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = rDoc.GetAttrPool();
                    const MapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != MapUnit::Map100thMM)
                    {
                        SvxUnoConvertFromMM(eMapUnit, aValue);
                    }
                }
            }

            switch(pEntry->nWID)
            {
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLBITMAP:
                case XATTR_FILLFLOATTRANSPARENCE:
                // not yet needed; activate when LineStyle support may be added
                // case XATTR_LINESTART:
                // case XATTR_LINEEND:
                // case XATTR_LINEDASH:
                {
                    if(MID_NAME == nMemberId)
                    {
                        // add set commands for FillName items
                        OUString aTempName;

                        if(!(aValue >>= aTempName))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        SvxShape::SetFillAttribute(pEntry->nWID, aTempName, aSet);
                        bDone = true;
                    }
                    else if (MID_BITMAP == nMemberId)
                    {
                        if(XATTR_FILLBITMAP == pEntry->nWID)
                        {
                            Graphic aNullGraphic;
                            XFillBitmapItem aXFillBitmapItem(std::move(aNullGraphic));

                            aXFillBitmapItem.PutValue(aValue, nMemberId);
                            aSet.Put(aXFillBitmapItem);
                            bDone = true;
                        }
                    }

                    break;
                }
                case RES_BACKGROUND:
                {
                    const std::unique_ptr<SvxBrushItem> aOriginalBrushItem(getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, true, rDoc.IsInXMLImport()));
                    std::unique_ptr<SvxBrushItem> aChangedBrushItem(aOriginalBrushItem->Clone());

                    aChangedBrushItem->PutValue(aValue, nMemberId);

                    if(*aChangedBrushItem != *aOriginalBrushItem)
                    {
                        setSvxBrushItemAsFillAttributesToTargetSet(*aChangedBrushItem, aSet);
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    drawing::BitmapMode eMode;

                    if(!(aValue >>= eMode))
                    {
                        sal_Int32 nMode = 0;

                        if(!(aValue >>= nMode))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        eMode = static_cast<drawing::BitmapMode>(nMode);
                    }

                    aSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
                    aSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));

                    bDone = true;
                    break;
                }
                default: break;
            }

            if(!bDone)
            {
                try
                {
                    pPropSet->setPropertyValue( rPropName, aValue, aSet );
                }
                catch (const beans::UnknownPropertyException &)
                {
                    OSL_FAIL( "Unknown property" );
                }
                catch (const lang::IllegalArgumentException &)
                {
                    OSL_FAIL( "Illegal argument" );
                }
            }
        }

        // clear parent again
        aSet.SetParent(nullptr);
    }

    // need to ensure uniqueness of evtl. added NameOrIndex items
    // currently in principle only needed when bTakeCareOfDrawingLayerFillStyle,
    // but does not hurt and is easily forgotten later eventually, so keep it
    // as common case
    rDoc.CheckForUniqueItemForLineFillNameOrIndex(aSet);

    return rDoc.GetIStyleAccess().cacheAutomaticStyle(aSet, eFamily);
}

uno::Reference< style::XAutoStyle > SwXAutoStyleFamily::insertStyle(
    const uno::Sequence< beans::PropertyValue >& Values )
{
    if (!m_pDocShell)
    {
        throw uno::RuntimeException();
    }

    WhichRangesContainer pRange;
    switch (m_eFamily)
    {
    case IStyleAccess::AUTO_STYLE_CHAR:
    {
        pRange = aCharAutoFormatSetRange;
        break;
    }
    case IStyleAccess::AUTO_STYLE_RUBY:
    {
        pRange = WhichRangesContainer(RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY);
        break;
    }
    case IStyleAccess::AUTO_STYLE_PARA:
    {
        pRange = aTextNodeSetRange; // checked, already added support for [XATTR_FILL_FIRST, XATTR_FILL_LAST]
        break;
    }
    default:
        throw uno::RuntimeException();
    }

    SwAttrSet aEmptySet(m_pDocShell->GetDoc()->GetAttrPool(), pRange);
    auto xSet = PropValuesToAutoStyleItemSet(*m_pDocShell->GetDoc(), m_eFamily, Values, aEmptySet);

    uno::Reference<style::XAutoStyle> xRet = new SwXAutoStyle(m_pDocShell->GetDoc(), std::move(xSet), m_eFamily);

    return xRet;
}

uno::Reference< container::XEnumeration > SwXAutoStyleFamily::createEnumeration(  )
{
    if( !m_pDocShell )
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >
        (new SwXAutoStylesEnumerator( *m_pDocShell->GetDoc(), m_eFamily ));
}

uno::Type SwXAutoStyleFamily::getElementType(  )
{
    return cppu::UnoType<style::XAutoStyle>::get();
}

sal_Bool SwXAutoStyleFamily::hasElements(  )
{
    return false;
}

SwAutoStylesEnumImpl::SwAutoStylesEnumImpl( SwDoc& rInitDoc, IStyleAccess::SwAutoStyleFamily eFam )
: m_rDoc( rInitDoc ), m_eFamily( eFam )
{
    // special case for ruby auto styles:
    if ( IStyleAccess::AUTO_STYLE_RUBY == eFam )
    {
        std::set< std::pair< sal_uInt16, text::RubyAdjust > > aRubyMap;
        SwAttrPool& rAttrPool = m_rDoc.GetAttrPool();

        // do this in two phases otherwise we invalidate the iterators when we insert into the pool
        std::vector<const SwFormatRuby*> vRubyItems;
        ItemSurrogates aSurrogates;
        rAttrPool.GetItemSurrogates(aSurrogates, RES_TXTATR_CJK_RUBY);
        for (const SfxPoolItem* pItem : aSurrogates)
        {
            auto pRubyItem = dynamic_cast<const SwFormatRuby*>(pItem);
            if ( pRubyItem && pRubyItem->GetTextRuby() )
                vRubyItems.push_back(pRubyItem);
        }
        for (const SwFormatRuby* pRubyItem : vRubyItems)
        {
            std::pair< sal_uInt16, text::RubyAdjust > aPair( pRubyItem->GetPosition(), pRubyItem->GetAdjustment() );
            if ( aRubyMap.insert( aPair ).second )
            {
                auto pItemSet = std::make_shared<SfxItemSetFixed<RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY>>( rAttrPool );
                pItemSet->Put( *pRubyItem );
                mAutoStyles.push_back( pItemSet );
            }
        }
    }
    else
    {
        m_rDoc.GetIStyleAccess().getAllStyles( mAutoStyles, m_eFamily );
    }

    m_aIter = mAutoStyles.begin();
}

SwXAutoStylesEnumerator::SwXAutoStylesEnumerator( SwDoc& rDoc, IStyleAccess::SwAutoStyleFamily eFam )
: m_pImpl( new SwAutoStylesEnumImpl( rDoc, eFam ) )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    StartListening(rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
}

SwXAutoStylesEnumerator::~SwXAutoStylesEnumerator()
{
}

void SwXAutoStylesEnumerator::Notify( const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        m_pImpl.reset();
}

sal_Bool SwXAutoStylesEnumerator::hasMoreElements(  )
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    return m_pImpl->hasMoreElements();
}

uno::Any SwXAutoStylesEnumerator::nextElement(  )
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    uno::Any aRet;
    if( m_pImpl->hasMoreElements() )
    {
        uno::Reference< style::XAutoStyle > xAutoStyle = new SwXAutoStyle(&m_pImpl->getDoc(),
                                                        m_pImpl->nextElement(), m_pImpl->getFamily());
        aRet <<= xAutoStyle;
    }
    return aRet;
}

// SwXAutoStyle with the family IStyleAccess::AUTO_STYLE_PARA (or
// PROPERTY_MAP_PARA_AUTO_STYLE) now uses DrawingLayer FillStyles to allow
// unified paragraph background fill, thus the UNO API implementation has to
// support the needed slots for these. This seems to be used only for reading
// (no setPropertyValue implementation here), so maybe specialized for saving
// the Writer Doc to ODF

SwXAutoStyle::SwXAutoStyle(
    SwDoc* pDoc,
    std::shared_ptr<SfxItemSet> pInitSet,
    IStyleAccess::SwAutoStyleFamily eFam)
:   mpSet(std::move(pInitSet)),
    meFamily(eFam),
    mrDoc(*pDoc)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    //StartListening(mrDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
}

SwXAutoStyle::~SwXAutoStyle()
{
}

void SwXAutoStyle::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        mpSet.reset();
}

uno::Reference< beans::XPropertySetInfo > SwXAutoStyle::getPropertySetInfo(  )
{
    uno::Reference< beans::XPropertySetInfo >  xRet;
    switch( meFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            static const auto xCharRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE)->getPropertySetInfo();
            xRet = xCharRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            static const auto xRubyRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_RUBY_AUTO_STYLE)->getPropertySetInfo();
            xRet = xRubyRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            static const auto xParaRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARA_AUTO_STYLE)->getPropertySetInfo();
            xRet = xParaRef;
        }
        break;

        default:
            ;
    }

    return xRet;
}

void SwXAutoStyle::setPropertyValue( const OUString& /*rPropertyName*/, const uno::Any& /*rValue*/ )
{
}

uno::Any SwXAutoStyle::getPropertyValue( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];
}

void SwXAutoStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                              const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXAutoStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                 const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
}

void SwXAutoStyle::addVetoableChangeListener( const OUString& /*PropertyName*/,
                                              const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}

void SwXAutoStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/,
                                                 const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}

void SwXAutoStyle::setPropertyValues(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Sequence< uno::Any >& /*aValues*/ )
{
}

uno::Sequence< uno::Any > SwXAutoStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
{
    if( !mpSet )
    {
        throw uno::RuntimeException();
    }

    // query_item
    sal_uInt16 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const OUString* pNames = rPropertyNames.getConstArray();

    const sal_Int32 nLen(rPropertyNames.getLength());
    uno::Sequence< uno::Any > aRet( nLen );
    uno::Any* pValues = aRet.getArray();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, getXWeak() );
        }

        uno::Any aTarget;
        bool bDone(false);

        if ( RES_TXTATR_AUTOFMT == pEntry->nWID || RES_AUTO_STYLE == pEntry->nWID )
        {
            OUString sName(StylePool::nameOf( mpSet ));
            aTarget <<= sName;
            bDone = true;
        }
        else if(bTakeCareOfDrawingLayerFillStyle)
        {
            // add support for DrawingLayer FillStyle slots
            switch(pEntry->nWID)
            {
                case RES_BACKGROUND:
                {
                    const std::unique_ptr<SvxBrushItem> aOriginalBrushItem(getSvxBrushItemFromSourceSet(*mpSet, RES_BACKGROUND));

                    if(!aOriginalBrushItem->QueryValue(aTarget, pEntry->nMemberId))
                    {
                        OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    if (mpSet->Get(XATTR_FILLBMP_TILE).GetValue())
                    {
                        aTarget <<= drawing::BitmapMode_REPEAT;
                    }
                    else if (mpSet->Get(XATTR_FILLBMP_STRETCH).GetValue())
                    {
                        aTarget <<= drawing::BitmapMode_STRETCH;
                    }
                    else
                    {
                        aTarget <<= drawing::BitmapMode_NO_REPEAT;
                    }

                    bDone = true;
                    break;
                }
            }
        }

        if(!bDone)
        {
            SfxItemPropertySet::getPropertyValue( *pEntry, *mpSet, aTarget );
        }

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            if(pEntry->aType == cppu::UnoType<sal_Int16>::get() && pEntry->aType != aTarget.getValueType())
            {
                // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                sal_Int32 nValue = 0;
                if (aTarget >>= nValue)
                {
                    aTarget <<= static_cast<sal_Int16>(nValue);
                }
            }

            // check for needed metric translation
            if(pEntry->nMoreFlags & PropertyMoreFlags::METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aTarget >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = mrDoc.GetAttrPool();
                    const MapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != MapUnit::Map100thMM)
                    {
                        SvxUnoConvertToMM(eMapUnit, aTarget);
                    }
                }
            }
        }

        // add value
        pValues[i] = aTarget;
    }

    return aRet;
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyValues (
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
        css::uno::Any exc = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("Unknown property exception caught", getXWeak(), exc );
    }
    catch (lang::WrappedTargetException &)
    {
        css::uno::Any exc = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("WrappedTargetException caught", getXWeak(), exc );
    }

    return aValues;
}

void SwXAutoStyle::addPropertiesChangeListener(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
}

void SwXAutoStyle::removePropertiesChangeListener(
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
}

void SwXAutoStyle::firePropertiesChangeEvent(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
}

beans::PropertyState SwXAutoStyle::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

void SwXAutoStyle::setPropertyToDefault( const OUString& /*PropertyName*/ )
{
}

uno::Any SwXAutoStyle::getPropertyDefault( const OUString& rPropertyName )
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXAutoStyle::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames )
{
    if (!mpSet)
    {
        throw uno::RuntimeException();
    }

    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();

    sal_uInt16 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, getXWeak() );
        }

        bool bDone(false);

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            // DrawingLayer PropertyStyle support
            switch(pEntry->nWID)
            {
                case OWN_ATTR_FILLBMP_MODE:
                {
                    if(SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                        || SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_TILE, false))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                    }

                    bDone = true;
                    break;
                }
                case RES_BACKGROUND:
                {
                    if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*mpSet,
                            pEntry->nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                    bDone = true;

                    break;
                }
            }
        }

        if(!bDone)
        {
            pStates[i] = SfxItemPropertySet::getPropertyState(*pEntry, *mpSet );
        }
    }

    return aRet;
}

void SwXAutoStyle::setAllPropertiesToDefault(  )
{
}

void SwXAutoStyle::setPropertiesToDefault(
        const uno::Sequence< OUString >& /*rPropertyNames*/ )
{
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyDefaults(
        const uno::Sequence< OUString >& /*aPropertyNames*/ )
{
    return { };
}

uno::Sequence< beans::PropertyValue > SwXAutoStyle::getProperties()
{
    if( !mpSet )
        throw uno::RuntimeException();
    SolarMutexGuard aGuard;
    std::vector< beans::PropertyValue > aPropertyVector;

    sal_uInt16 nPropSetId = 0;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

    SfxItemSet& rSet = *mpSet;
    SfxItemIter aIter(rSet);

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        const sal_uInt16 nWID = pItem->Which();

        // TODO: Optimize - and fix! the old iteration filled each WhichId
        // only once but there are more properties than WhichIds
        for( const auto pEntry : rMap.getPropertyEntries() )
        {
            if ( pEntry->nWID == nWID )
            {
                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = pEntry->aName;
                pItem->QueryValue( aPropertyValue.Value, pEntry->nMemberId );
                aPropertyVector.push_back( aPropertyValue );
            }
        }
    }

    const sal_Int32 nCount = aPropertyVector.size();
    uno::Sequence< beans::PropertyValue > aRet( nCount );
    beans::PropertyValue* pProps = aRet.getArray();

    for ( int i = 0; i < nCount; ++i, pProps++ )
    {
        *pProps = aPropertyVector[i];
    }

    return aRet;
}

SwXTextTableStyle::SwXTextTableStyle(SwDocShell* pDocShell, SwTableAutoFormat* pTableAutoFormat) :
    m_pDocShell(pDocShell), m_pTableAutoFormat(pTableAutoFormat), m_bPhysical(true)
{
    UpdateCellStylesMapping();
}

SwXTextTableStyle::SwXTextTableStyle(SwDocShell* pDocShell, const OUString& rTableAutoFormatName) :
    m_pDocShell(pDocShell), m_pTableAutoFormat_Impl(new SwTableAutoFormat(rTableAutoFormatName)), m_bPhysical(false)
{
    m_pTableAutoFormat = m_pTableAutoFormat_Impl.get();
    UpdateCellStylesMapping();
}

rtl::Reference<SwXTextTableStyle> SwXTextTableStyle::CreateXTextTableStyle(SwDocShell* pDocShell, const OUString& rTableAutoFormatName)
{
    SolarMutexGuard aGuard;
    rtl::Reference<SwXTextTableStyle> xTextTableStyle;
    SwTableAutoFormat* pAutoFormat = GetTableAutoFormat(pDocShell, rTableAutoFormatName);
    if (pAutoFormat && pAutoFormat->GetName() == rTableAutoFormatName)
    {
        xTextTableStyle = pAutoFormat->GetXObject();
        if (!xTextTableStyle.is())
        {
            xTextTableStyle.set(new SwXTextTableStyle(pDocShell, pAutoFormat));
            pAutoFormat->SetXObject(xTextTableStyle);
        }
    }

    // If corresponding AutoFormat doesn't exist create a non physical style.
    if (!xTextTableStyle.is())
    {
        xTextTableStyle.set(new SwXTextTableStyle(pDocShell, rTableAutoFormatName));
        SAL_INFO("sw.uno", "creating SwXTextTableStyle for non existing SwTableAutoFormat");
    }

    return xTextTableStyle;
}

void SwXTextTableStyle::UpdateCellStylesMapping()
{
    const std::vector<sal_Int32> aTableTemplateMap = SwTableAutoFormat::GetTableTemplateMap();
    assert(aTableTemplateMap.size() == STYLE_COUNT && "can not map SwTableAutoFormat to a SwXTextTableStyle");
    for (sal_Int32 i=0; i<STYLE_COUNT; ++i)
    {
        SwBoxAutoFormat* pBoxFormat = &m_pTableAutoFormat->GetBoxFormat(aTableTemplateMap[i]);
        rtl::Reference<SwXTextCellStyle> xCellStyle(pBoxFormat->GetXObject());
        if (!xCellStyle.is())
        {
            xCellStyle.set(new SwXTextCellStyle(m_pDocShell, pBoxFormat, m_pTableAutoFormat->GetName()));
            pBoxFormat->SetXObject(xCellStyle);
        }
        m_aCellStyles[i] = xCellStyle;
    }
}

const CellStyleNameMap& SwXTextTableStyle::GetCellStyleNameMap()
{
    static CellStyleNameMap const aMap
    {
        { "first-row"     , FIRST_ROW_STYLE },
        { "last-row"      , LAST_ROW_STYLE },
        { "first-column"  , FIRST_COLUMN_STYLE },
        { "last-column"   , LAST_COLUMN_STYLE },
        { "body"          , BODY_STYLE },
        { "even-rows"     , EVEN_ROWS_STYLE },
        { "odd-rows"      , ODD_ROWS_STYLE },
        { "even-columns"  , EVEN_COLUMNS_STYLE },
        { "odd-columns"   , ODD_COLUMNS_STYLE },
        { "background"    , BACKGROUND_STYLE },
        // loext namespace
        { "first-row-start-column"  , FIRST_ROW_START_COLUMN_STYLE },
        { "first-row-end-column"    , FIRST_ROW_END_COLUMN_STYLE },
        { "last-row-start-column"   , LAST_ROW_START_COLUMN_STYLE },
        { "last-row-end-column"     , LAST_ROW_END_COLUMN_STYLE },
        { "first-row-even-column"   , FIRST_ROW_EVEN_COLUMN_STYLE },
        { "last-row-even-column"    , LAST_ROW_EVEN_COLUMN_STYLE },
    };
    return aMap;
}

SwTableAutoFormat* SwXTextTableStyle::GetTableFormat()
{
    return m_pTableAutoFormat;
}

SwTableAutoFormat* SwXTextTableStyle::GetTableAutoFormat(SwDocShell* pDocShell, std::u16string_view sName)
{
    const size_t nStyles = pDocShell->GetDoc()->GetTableStyles().size();
    for(size_t i=0; i < nStyles; ++i)
    {
        SwTableAutoFormat* pAutoFormat = &pDocShell->GetDoc()->GetTableStyles()[i];
        if (pAutoFormat->GetName() == sName)
        {
            return pAutoFormat;
        }
    }
    // not found
    return nullptr;
}

void SwXTextTableStyle::SetPhysical()
{
    if (!m_bPhysical)
    {
        // find table format in doc
        SwTableAutoFormat* pTableAutoFormat = GetTableAutoFormat(m_pDocShell, m_pTableAutoFormat->GetName());
        if (pTableAutoFormat)
        {
            m_bPhysical = true;
            /// take care of children, make SwXTextCellStyles use new core SwBoxAutoFormats
            const std::vector<sal_Int32> aTableTemplateMap = SwTableAutoFormat::GetTableTemplateMap();
            for (size_t i=0; i<aTableTemplateMap.size(); ++i)
            {
                SwBoxAutoFormat* pOldBoxFormat = &m_pTableAutoFormat->GetBoxFormat(aTableTemplateMap[i]);
                rtl::Reference<SwXTextCellStyle> xCellStyle(pOldBoxFormat->GetXObject());
                if (!xCellStyle.is())
                    continue;
                SwBoxAutoFormat& rNewBoxFormat = pTableAutoFormat->GetBoxFormat(aTableTemplateMap[i]);
                xCellStyle->SetBoxFormat(&rNewBoxFormat);
                rNewBoxFormat.SetXObject(xCellStyle);
            }
            m_pTableAutoFormat_Impl = nullptr;
            m_pTableAutoFormat = pTableAutoFormat;
            m_pTableAutoFormat->SetXObject(this);
        }
        else
            SAL_WARN("sw.uno", "setting style physical, but SwTableAutoFormat in document not found");
    }
    else
        SAL_WARN("sw.uno", "calling SetPhysical on a physical SwXTextTableStyle");
}

// XStyle
sal_Bool SAL_CALL SwXTextTableStyle::isUserDefined()
{
    SolarMutexGuard aGuard;
    // only first style is not user defined
    if (m_pDocShell->GetDoc()->GetTableStyles()[0].GetName() == m_pTableAutoFormat->GetName())
        return false;

    return true;
}

sal_Bool SAL_CALL SwXTextTableStyle::isInUse()
{
    SolarMutexGuard aGuard;
    if (!m_bPhysical)
        return false;

    for (const SwTableFormat* pFormat : *m_pDocShell->GetDoc()->GetTableFrameFormats())
    {
        if(pFormat->IsUsed())
        {
            SwTable* pTable = SwTable::FindTable(pFormat);
            if(pTable->GetTableStyleName() == m_pTableAutoFormat->GetName())
                return true;
        }
    }
    return false;
}

OUString SAL_CALL SwXTextTableStyle::getParentStyle()
{
    return OUString();
}

void SAL_CALL SwXTextTableStyle::setParentStyle(const OUString& /*aParentStyle*/)
{ }

//XNamed
OUString SAL_CALL SwXTextTableStyle::getName()
{
    SolarMutexGuard aGuard;
    OUString sProgName;
    SwStyleNameMapper::FillProgName(m_pTableAutoFormat->GetName(), sProgName, SwGetPoolIdFromName::TabStyle);
    return sProgName;
}

void SAL_CALL SwXTextTableStyle::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    m_pTableAutoFormat->SetName(rName);
}

//XPropertySet
css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL SwXTextTableStyle::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xRef(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_STYLE)->getPropertySetInfo());
    return xRef;
}

void SAL_CALL SwXTextTableStyle::setPropertyValue(const OUString& /*rPropertyName*/, const css::uno::Any& /*aValue*/)
{
    SAL_WARN("sw.uno", "not implemented");
}

css::uno::Any SAL_CALL SwXTextTableStyle::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    bool bIsRow = false;

    if (rPropertyName == UNO_NAME_TABLE_FIRST_ROW_END_COLUMN)
        bIsRow = m_pTableAutoFormat->FirstRowEndColumnIsRow();
    else if (rPropertyName == UNO_NAME_TABLE_FIRST_ROW_START_COLUMN)
        bIsRow = m_pTableAutoFormat->FirstRowStartColumnIsRow();
    else if (rPropertyName == UNO_NAME_TABLE_LAST_ROW_END_COLUMN)
        bIsRow = m_pTableAutoFormat->LastRowEndColumnIsRow();
    else if (rPropertyName == UNO_NAME_TABLE_LAST_ROW_START_COLUMN)
        bIsRow = m_pTableAutoFormat->LastRowStartColumnIsRow();
    else if (rPropertyName == UNO_NAME_DISPLAY_NAME)
        return uno::Any(m_pTableAutoFormat->GetName());
    else
        throw css::beans::UnknownPropertyException(rPropertyName);

    return uno::Any(bIsRow ? OUString("row") : OUString("column"));
}

void SAL_CALL SwXTextTableStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*xListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextTableStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextTableStyle::addVetoableChangeListener( const OUString& /*PropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextTableStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

//XNameAccess
uno::Any SAL_CALL SwXTextTableStyle::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    const CellStyleNameMap& rMap = GetCellStyleNameMap();
    CellStyleNameMap::const_iterator iter = rMap.find(rName);
    if(iter == rMap.end())
        throw css::container::NoSuchElementException();

    auto nIdx = (*iter).second;
    return css::uno::Any(uno::Reference(cppu::getXWeak(m_aCellStyles[nIdx].get())));
}

css::uno::Sequence<OUString> SAL_CALL SwXTextTableStyle::getElementNames()
{
    return comphelper::mapKeysToSequence(GetCellStyleNameMap());
}

sal_Bool SAL_CALL SwXTextTableStyle::hasByName(const OUString& rName)
{
    const CellStyleNameMap& rMap = GetCellStyleNameMap();
    CellStyleNameMap::const_iterator iter = rMap.find(rName);
    return iter != rMap.end();
}

//XNameContainer
void SAL_CALL SwXTextTableStyle::insertByName(const OUString& /*Name*/, const uno::Any& /*Element*/)
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextTableStyle::replaceByName(const OUString& rName, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;
    const CellStyleNameMap& rMap = GetCellStyleNameMap();
    CellStyleNameMap::const_iterator iter = rMap.find(rName);
    if(iter == rMap.end())
        throw container::NoSuchElementException();
    const sal_Int32 nCellStyle = iter->second;

    rtl::Reference<SwXTextCellStyle> xStyleToReplaceWith = dynamic_cast<SwXTextCellStyle*>(rElement.get<uno::Reference<style::XStyle>>().get());
    if (!xStyleToReplaceWith.is())
        throw lang::IllegalArgumentException();

    // replace only with physical ...
    if (!xStyleToReplaceWith->IsPhysical())
        throw lang::IllegalArgumentException();

    const auto& rTableTemplateMap = SwTableAutoFormat::GetTableTemplateMap();
    const sal_Int32 nBoxFormat = rTableTemplateMap[nCellStyle];

    // move SwBoxAutoFormat to dest. SwTableAutoFormat
    m_pTableAutoFormat->SetBoxFormat(*xStyleToReplaceWith->GetBoxFormat(), nBoxFormat);
    // remove unassigned SwBoxAutoFormat, which is not anymore in use anyways
    m_pDocShell->GetDoc()->GetCellStyles().RemoveBoxFormat(xStyleToReplaceWith->getName());
    // make SwXTextCellStyle use new, moved SwBoxAutoFormat
    xStyleToReplaceWith->SetBoxFormat(&m_pTableAutoFormat->GetBoxFormat(nBoxFormat));
    m_pTableAutoFormat->GetBoxFormat(nBoxFormat).SetXObject(xStyleToReplaceWith);
    // make this SwXTextTableStyle use new SwXTextCellStyle
    m_aCellStyles[nCellStyle] = xStyleToReplaceWith;
}

void SAL_CALL SwXTextTableStyle::removeByName(const OUString& /*Name*/)
{
    SAL_WARN("sw.uno", "not implemented");
}

//XElementAccess
uno::Type SAL_CALL SAL_CALL SwXTextTableStyle::getElementType()
{
    return cppu::UnoType<style::XStyle>::get();
}

sal_Bool SAL_CALL SAL_CALL SwXTextTableStyle::hasElements()
{
    return true;
}

//XServiceInfo
OUString SAL_CALL SwXTextTableStyle::getImplementationName()
{
    return {"SwXTextTableStyle"};
}

sal_Bool SAL_CALL SwXTextTableStyle::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL SwXTextTableStyle::getSupportedServiceNames()
{
    return {"com.sun.star.style.Style"};
}

// SwXTextCellStyle
SwXTextCellStyle::SwXTextCellStyle(SwDocShell* pDocShell, SwBoxAutoFormat* pBoxAutoFormat, OUString sParentStyle) :
    m_pDocShell(pDocShell),
    m_pBoxAutoFormat(pBoxAutoFormat),
    m_sParentStyle(std::move(sParentStyle)),
    m_bPhysical(true)
{ }

SwXTextCellStyle::SwXTextCellStyle(SwDocShell* pDocShell, OUString sName) :
    m_pDocShell(pDocShell),
    m_pBoxAutoFormat_Impl(std::make_shared<SwBoxAutoFormat>()),
    m_sName(std::move(sName)),
    m_bPhysical(false)
{
    m_pBoxAutoFormat = m_pBoxAutoFormat_Impl.get();
}

SwBoxAutoFormat* SwXTextCellStyle::GetBoxFormat()
{
    return m_pBoxAutoFormat;
}

void SwXTextCellStyle::SetBoxFormat(SwBoxAutoFormat* pBoxFormat)
{
    if (m_bPhysical)
        m_pBoxAutoFormat = pBoxFormat;
    else
        SAL_INFO("sw.uno", "trying to call SwXTextCellStyle::SetBoxFormat on non physical style");
}

void SwXTextCellStyle::SetPhysical()
{
    if (!m_bPhysical)
    {
        SwBoxAutoFormat* pBoxAutoFormat = GetBoxAutoFormat(m_pDocShell, m_sName, &m_sParentStyle);
        if (pBoxAutoFormat)
        {
            m_bPhysical = true;
            m_pBoxAutoFormat_Impl = nullptr;
            m_pBoxAutoFormat = pBoxAutoFormat;
            m_pBoxAutoFormat->SetXObject(this);
        }
        else
            SAL_WARN("sw.uno", "setting style physical, but SwBoxAutoFormat in document not found");
    }
    else
        SAL_WARN("sw.uno", "calling SetPhysical on a physical SwXTextCellStyle");
}

bool SwXTextCellStyle::IsPhysical() const
{
    return m_bPhysical;
}

SwBoxAutoFormat* SwXTextCellStyle::GetBoxAutoFormat(SwDocShell* pDocShell, std::u16string_view sName, OUString* pParentName)
{
    if (sName.empty())
        return nullptr;

    SwBoxAutoFormat* pBoxAutoFormat = pDocShell->GetDoc()->GetCellStyles().GetBoxFormat(sName);
    if (!pBoxAutoFormat)
    {
        sal_Int32 nTemplateIndex;
        OUString sParentName;
        std::u16string_view sCellSubName;

        size_t nSeparatorIndex = sName.rfind('.');
        if (nSeparatorIndex == std::u16string_view::npos)
            return nullptr;

        sParentName = sName.substr(0, nSeparatorIndex);
        sCellSubName = sName.substr(nSeparatorIndex+1);
        nTemplateIndex = o3tl::toInt32(sCellSubName)-1; // -1 because cell styles names start from 1, but internally are indexed from 0
        if (0 > nTemplateIndex)
            return nullptr;

        const auto& rTableTemplateMap = SwTableAutoFormat::GetTableTemplateMap();
        if (rTableTemplateMap.size() <= o3tl::make_unsigned(nTemplateIndex))
            return nullptr;

        SwStyleNameMapper::FillUIName(sParentName, sParentName, SwGetPoolIdFromName::TabStyle);
        SwTableAutoFormat* pTableAutoFormat = pDocShell->GetDoc()->GetTableStyles().FindAutoFormat(sParentName);
        if (!pTableAutoFormat)
            return nullptr;

        if (pParentName)
            *pParentName = sParentName;
        sal_uInt32 nBoxIndex = rTableTemplateMap[nTemplateIndex];
        pBoxAutoFormat = &pTableAutoFormat->GetBoxFormat(nBoxIndex);
    }

    return pBoxAutoFormat;
}

rtl::Reference<SwXTextCellStyle> SwXTextCellStyle::CreateXTextCellStyle(SwDocShell* pDocShell, const OUString& sName)
{
    rtl::Reference<SwXTextCellStyle> xTextCellStyle;

    if (!sName.isEmpty()) // create a cell style for a physical box
    {
        OUString sParentName;
        SwBoxAutoFormat* pBoxFormat = GetBoxAutoFormat(pDocShell, sName, &sParentName);

        // something went wrong but we don't want a crash
        if (!pBoxFormat)
        {
            // return a default-dummy style to prevent crash
            static SwBoxAutoFormat aDefaultBoxFormat;
            pBoxFormat = &aDefaultBoxFormat;
        }

        xTextCellStyle = pBoxFormat->GetXObject();
        if (!xTextCellStyle.is())
        {
            xTextCellStyle.set(new SwXTextCellStyle(pDocShell, pBoxFormat, sParentName));
            pBoxFormat->SetXObject(xTextCellStyle);
        }
    }
    else // create a non physical style
        xTextCellStyle.set(new SwXTextCellStyle(pDocShell, sName));

    return xTextCellStyle;
}

// XStyle
sal_Bool SAL_CALL SwXTextCellStyle::isUserDefined()
{
    SolarMutexGuard aGuard;
    // if this cell belong to first table style then its default style
    if (&m_pDocShell->GetDoc()->GetTableStyles()[0] == m_pDocShell->GetDoc()->GetTableStyles().FindAutoFormat(m_sParentStyle))
        return false;

    return true;
}

sal_Bool SAL_CALL SwXTextCellStyle::isInUse()
{
    SolarMutexGuard aGuard;
    uno::Reference<style::XStyleFamiliesSupplier> xFamiliesSupplier(m_pDocShell->GetModel(), uno::UNO_QUERY);
    if (!xFamiliesSupplier.is())
        return false;

    uno::Reference<container::XNameAccess> xFamilies = xFamiliesSupplier->getStyleFamilies();
    if (!xFamilies.is())
        return false;

    uno::Reference<container::XNameAccess> xTableStyles;
    xFamilies->getByName("TableStyles") >>= xTableStyles;
    if (!xTableStyles.is())
        return false;

    uno::Reference<style::XStyle> xStyle;
    xTableStyles->getByName(m_sParentStyle) >>= xStyle;
    if (!xStyle.is())
        return false;

    return xStyle->isInUse();
}

OUString SAL_CALL SwXTextCellStyle::getParentStyle()
{
    // Do not return name of the parent (which is a table style) because the parent should be a cell style.
    return OUString();
}

void SAL_CALL SwXTextCellStyle::setParentStyle(const OUString& /*sParentStyle*/)
{
    // Changing parent to one which is unaware of it will lead to a something unexpected. getName() rely on a parent.
    SAL_INFO("sw.uno", "Changing SwXTextCellStyle parent");
}

//XNamed
OUString SAL_CALL SwXTextCellStyle::getName()
{
    SolarMutexGuard aGuard;
    OUString sName;

    // if style is physical then we request a name from doc
    if (m_bPhysical)
    {
        SwTableAutoFormat* pTableFormat = m_pDocShell->GetDoc()->GetTableStyles().FindAutoFormat(m_sParentStyle);
        if (!pTableFormat)
        {
            // if auto format is not found as a child of table formats, look in SwDoc cellstyles
            sName = m_pDocShell->GetDoc()->GetCellStyles().GetBoxFormatName(*m_pBoxAutoFormat);
        }
        else
        {
            OUString sParentStyle;
            SwStyleNameMapper::FillProgName(m_sParentStyle, sParentStyle, SwGetPoolIdFromName::TabStyle);
            sName = sParentStyle + pTableFormat->GetTableTemplateCellSubName(*m_pBoxAutoFormat);
        }
    }
    else
        sName = m_sName;

    return sName;
}

void SAL_CALL SwXTextCellStyle::setName(const OUString& sName)
{
    SolarMutexGuard aGuard;
    // if style is physical then we can not rename it.
    if (!m_bPhysical)
        m_sName = sName;
    // change name if style is unassigned (name is not generated automatically)
    m_pDocShell->GetDoc()->GetCellStyles().ChangeBoxFormatName(getName(), sName);
}

//XPropertySet
css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL SwXTextCellStyle::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xRef(aSwMapProvider.GetPropertySet(PROPERTY_MAP_CELL_STYLE)->getPropertySetInfo());
    return xRef;
}

void SAL_CALL SwXTextCellStyle::setPropertyValue(const OUString& rPropertyName, const css::uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertyMapEntry *const pEntry = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CELL_STYLE)->getPropertyMap().getByName(rPropertyName);
    if(pEntry)
    {
        switch(pEntry->nWID)
        {
            case RES_BACKGROUND:
            {
                SvxBrushItem rBrush = m_pBoxAutoFormat->GetBackground();
                rBrush.PutValue(aValue, 0);
                m_pBoxAutoFormat->SetBackground(rBrush);
                return;
            }
            case RES_BOX:
            {
                SvxBoxItem rBox = m_pBoxAutoFormat->GetBox();
                rBox.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetBox(rBox);
                return;
            }
            case RES_VERT_ORIENT:
            {
                SwFormatVertOrient rVertOrient = m_pBoxAutoFormat->GetVerticalAlignment();
                rVertOrient.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetVerticalAlignment(rVertOrient);
                return;
            }
            case RES_FRAMEDIR:
            {
                SvxFrameDirectionItem rDirItem = m_pBoxAutoFormat->GetTextOrientation();
                rDirItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetTextOrientation(rDirItem);
                return;
            }
            case RES_BOXATR_FORMAT:
            {
                sal_uInt32 nKey;
                if (aValue >>= nKey)
                {
                    // FIXME: It's not working for old "automatic" currency formats, which are still in use by autotbl.fmt.
                    // Scenario:
                    // 1) Mark all styles present by default in autotbl.fmt as default.
                    // 2) convert all currencies present in autotbl.fmt before calling this code
                    const SvNumberformat* pNumFormat = m_pDocShell->GetDoc()->GetNumberFormatter()->GetEntry(nKey);
                    if (pNumFormat)
                        m_pBoxAutoFormat->SetValueFormat(pNumFormat->GetFormatstring(), pNumFormat->GetLanguage(), GetAppLanguage());
                }
                return;
            }
            // Paragraph attributes
            case RES_PARATR_ADJUST:
            {
                SvxAdjustItem rAdjustItem = m_pBoxAutoFormat->GetAdjust();
                rAdjustItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetAdjust(rAdjustItem);
                return;
            }
            case RES_CHRATR_COLOR:
            {
                SvxColorItem rColorItem = m_pBoxAutoFormat->GetColor();
                rColorItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetColor(rColorItem);
                return;
            }
            case RES_CHRATR_SHADOWED:
            {
                SvxShadowedItem rShadowedItem = m_pBoxAutoFormat->GetShadowed();
                bool bValue = false; aValue >>= bValue;
                rShadowedItem.SetValue(bValue);
                m_pBoxAutoFormat->SetShadowed(rShadowedItem);
                return;
            }
            case RES_CHRATR_CONTOUR:
            {
                SvxContourItem rContourItem = m_pBoxAutoFormat->GetContour();
                bool bValue = false; aValue >>= bValue;
                rContourItem.SetValue(bValue);
                m_pBoxAutoFormat->SetContour(rContourItem);
                return;
            }
            case RES_CHRATR_CROSSEDOUT:
            {
                SvxCrossedOutItem rCrossedOutItem = m_pBoxAutoFormat->GetCrossedOut();
                rCrossedOutItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCrossedOut(rCrossedOutItem);
                return;
            }
            case RES_CHRATR_UNDERLINE:
            {
                SvxUnderlineItem rUnderlineItem = m_pBoxAutoFormat->GetUnderline();
                rUnderlineItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetUnderline(rUnderlineItem);
                return;
            }
            case RES_CHRATR_FONTSIZE:
            {
                SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetHeight();
                rFontHeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetHeight(rFontHeightItem);
                return;
            }
            case RES_CHRATR_WEIGHT:
            {
                SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetWeight();
                rWeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetWeight(rWeightItem);
                return;
            }
            case RES_CHRATR_POSTURE:
            {
                SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetPosture();
                rPostureItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetPosture(rPostureItem);
                return;
            }
            case RES_CHRATR_FONT:
            {
                SvxFontItem rFontItem = m_pBoxAutoFormat->GetFont();
                rFontItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetFont(rFontItem);
                return;
            }
            case RES_CHRATR_CJK_FONTSIZE:
            {
                SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCJKHeight();
                rFontHeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCJKHeight(rFontHeightItem);
                return;
            }
            case RES_CHRATR_CJK_WEIGHT:
            {
                SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetCJKWeight();
                rWeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCJKWeight(rWeightItem);
                return;
            }
            case RES_CHRATR_CJK_POSTURE:
            {
                SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetCJKPosture();
                rPostureItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCJKPosture(rPostureItem);
                return;
            }
            case RES_CHRATR_CJK_FONT:
            {
                SvxFontItem rFontItem = m_pBoxAutoFormat->GetCJKFont();
                rFontItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCJKFont(rFontItem);
                return;
            }
            case RES_CHRATR_CTL_FONTSIZE:
            {
                SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCTLHeight();
                rFontHeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCTLHeight(rFontHeightItem);
                return;
            }
            case RES_CHRATR_CTL_WEIGHT:
            {
                SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetCTLWeight();
                rWeightItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCTLWeight(rWeightItem);
                return;
            }
            case RES_CHRATR_CTL_POSTURE:
            {
                SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetCTLPosture();
                rPostureItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCTLPosture(rPostureItem);
                return;
            }
            case RES_CHRATR_CTL_FONT:
            {
                SvxFontItem rFontItem = m_pBoxAutoFormat->GetCTLFont();
                rFontItem.PutValue(aValue, pEntry->nMemberId);
                m_pBoxAutoFormat->SetCTLFont(rFontItem);
                return;
            }
            default:
                SAL_WARN("sw.uno", "SwXTextCellStyle unknown nWID");
                throw css::uno::RuntimeException();
        }
    }

    throw css::beans::UnknownPropertyException(rPropertyName);
}

css::uno::Any SAL_CALL SwXTextCellStyle::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    const SfxItemPropertyMapEntry *const pEntry = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CELL_STYLE)->getPropertyMap().getByName(rPropertyName);
    if(pEntry)
    {
        switch(pEntry->nWID)
        {
            case RES_BACKGROUND:
            {
                const SvxBrushItem& rBrush = m_pBoxAutoFormat->GetBackground();
                rBrush.QueryValue(aRet);
                return aRet;
            }
            case RES_BOX:
            {
                const SvxBoxItem& rBox = m_pBoxAutoFormat->GetBox();
                rBox.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_VERT_ORIENT:
            {
                const SwFormatVertOrient& rVertOrient = m_pBoxAutoFormat->GetVerticalAlignment();
                rVertOrient.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_FRAMEDIR:
            {
                const SvxFrameDirectionItem& rDirItem = m_pBoxAutoFormat->GetTextOrientation();
                rDirItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_BOXATR_FORMAT:
            {
                OUString sFormat;
                LanguageType eLng, eSys;
                m_pBoxAutoFormat->GetValueFormat(sFormat, eLng, eSys);
                if(!sFormat.isEmpty())
                {
                    SvNumFormatType nType; bool bNew; sal_Int32 nCheckPos;
                    sal_uInt32 nKey = m_pDocShell->GetDoc()->GetNumberFormatter()->GetIndexPuttingAndConverting(sFormat, eLng, eSys, nType, bNew, nCheckPos);
                    aRet <<= nKey;
                }
                return aRet;
            }
            // Paragraph attributes
            case RES_PARATR_ADJUST:
            {
                const SvxAdjustItem& rAdjustItem = m_pBoxAutoFormat->GetAdjust();
                rAdjustItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_COLOR:
            {
                const SvxColorItem& rColorItem = m_pBoxAutoFormat->GetColor();
                rColorItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_SHADOWED:
            {
                const SvxShadowedItem& rShadowedItem = m_pBoxAutoFormat->GetShadowed();
                aRet <<= rShadowedItem.GetValue();
                return aRet;
            }
            case RES_CHRATR_CONTOUR:
            {
                const SvxContourItem& rContourItem = m_pBoxAutoFormat->GetContour();
                aRet <<= rContourItem.GetValue();
                return aRet;
            }
            case RES_CHRATR_CROSSEDOUT:
            {
                const SvxCrossedOutItem& rCrossedOutItem = m_pBoxAutoFormat->GetCrossedOut();
                rCrossedOutItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_UNDERLINE:
            {
                const SvxUnderlineItem& rUnderlineItem = m_pBoxAutoFormat->GetUnderline();
                rUnderlineItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_FONTSIZE:
            {
                const SvxFontHeightItem& rFontHeightItem = m_pBoxAutoFormat->GetHeight();
                rFontHeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_WEIGHT:
            {
                const SvxWeightItem& rWeightItem = m_pBoxAutoFormat->GetWeight();
                rWeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_POSTURE:
            {
                const SvxPostureItem& rPostureItem = m_pBoxAutoFormat->GetPosture();
                rPostureItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_FONT:
            {
                const SvxFontItem rFontItem = m_pBoxAutoFormat->GetFont();
                rFontItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CJK_FONTSIZE:
            {
                const SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCJKHeight();
                rFontHeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CJK_WEIGHT:
            {
                const SvxWeightItem& rWeightItem = m_pBoxAutoFormat->GetCJKWeight();
                rWeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CJK_POSTURE:
            {
                const SvxPostureItem& rPostureItem = m_pBoxAutoFormat->GetCJKPosture();
                rPostureItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CJK_FONT:
            {
                const SvxFontItem rFontItem = m_pBoxAutoFormat->GetCJKFont();
                rFontItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CTL_FONTSIZE:
            {
                const SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCTLHeight();
                rFontHeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CTL_WEIGHT:
            {
                const SvxWeightItem& rWeightItem = m_pBoxAutoFormat->GetCTLWeight();
                rWeightItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CTL_POSTURE:
            {
                const SvxPostureItem& rPostureItem = m_pBoxAutoFormat->GetCTLPosture();
                rPostureItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            case RES_CHRATR_CTL_FONT:
            {
                const SvxFontItem rFontItem = m_pBoxAutoFormat->GetCTLFont();
                rFontItem.QueryValue(aRet, pEntry->nMemberId);
                return aRet;
            }
            default:
                SAL_WARN("sw.uno", "SwXTextCellStyle unknown nWID");
                throw css::uno::RuntimeException();
        }
    }

    throw css::beans::UnknownPropertyException(rPropertyName);
}

void SAL_CALL SwXTextCellStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*xListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextCellStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextCellStyle::addVetoableChangeListener( const OUString& /*PropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

void SAL_CALL SwXTextCellStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*aListener*/ )
{
    SAL_WARN("sw.uno", "not implemented");
}

//XPropertyState
css::beans::PropertyState SAL_CALL SwXTextCellStyle::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aNames { rPropertyName };
    uno::Sequence<beans::PropertyState> aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

css::uno::Sequence<css::beans::PropertyState> SAL_CALL SwXTextCellStyle::getPropertyStates(const css::uno::Sequence<OUString>& aPropertyNames)
{
    SolarMutexGuard aGuard;
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const SwBoxAutoFormat& rDefaultBoxFormat = SwTableAutoFormat::GetDefaultBoxFormat();
    const SfxItemPropertyMap& rMap = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CELL_STYLE)->getPropertyMap();
    const OUString* pNames = aPropertyNames.getConstArray();
    for(sal_Int32 i=0; i < aPropertyNames.getLength(); ++i)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertyMapEntry* pEntry = rMap.getByName(sPropName);
        if(pEntry)
        {
            uno::Any aAny1, aAny2;
            switch(pEntry->nWID)
            {
                case RES_BACKGROUND:
                    m_pBoxAutoFormat->GetBackground().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetBackground().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_BOX:
                    m_pBoxAutoFormat->GetBox().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetBox().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_VERT_ORIENT:
                    m_pBoxAutoFormat->GetVerticalAlignment().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetVerticalAlignment().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_FRAMEDIR:
                    m_pBoxAutoFormat->GetTextOrientation().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetTextOrientation().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_BOXATR_FORMAT:
                {
                    OUString sFormat;
                    LanguageType eLng, eSys;
                    m_pBoxAutoFormat->GetValueFormat(sFormat, eLng, eSys);
                    pStates[i] = sFormat.isEmpty() ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                }
                case RES_PARATR_ADJUST:
                    m_pBoxAutoFormat->GetAdjust().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetAdjust().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_COLOR:
                    m_pBoxAutoFormat->GetColor().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetColor().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_SHADOWED:
                    m_pBoxAutoFormat->GetShadowed().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetShadowed().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CONTOUR:
                    m_pBoxAutoFormat->GetContour().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetContour().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CROSSEDOUT:
                    m_pBoxAutoFormat->GetCrossedOut().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCrossedOut().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_UNDERLINE:
                    m_pBoxAutoFormat->GetUnderline().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetUnderline().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_FONTSIZE:
                    m_pBoxAutoFormat->GetHeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetHeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_WEIGHT:
                    m_pBoxAutoFormat->GetWeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetWeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_POSTURE:
                    m_pBoxAutoFormat->GetPosture().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetPosture().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_FONT:
                    m_pBoxAutoFormat->GetFont().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetFont().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CJK_FONTSIZE:
                    m_pBoxAutoFormat->GetCJKHeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCJKHeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CJK_WEIGHT:
                    m_pBoxAutoFormat->GetCJKWeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCJKWeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CJK_POSTURE:
                    m_pBoxAutoFormat->GetCJKPosture().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCJKPosture().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CJK_FONT:
                    m_pBoxAutoFormat->GetCJKFont().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCJKFont().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CTL_FONTSIZE:
                    m_pBoxAutoFormat->GetCTLHeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCTLHeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CTL_WEIGHT:
                    m_pBoxAutoFormat->GetCTLWeight().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCTLWeight().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CTL_POSTURE:
                    m_pBoxAutoFormat->GetCTLPosture().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCTLPosture().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                case RES_CHRATR_CTL_FONT:
                    m_pBoxAutoFormat->GetCTLFont().QueryValue(aAny1, pEntry->nMemberId);
                    rDefaultBoxFormat.GetCTLFont().QueryValue(aAny2, pEntry->nMemberId);
                    pStates[i] = aAny1 == aAny2 ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                    break;
                default:
                    // fallthrough to DIRECT_VALUE, to export properties for which getPropertyStates is not implemented
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    SAL_WARN("sw.uno", "SwXTextCellStyle getPropertyStates unknown nWID");
            }
        }
        else
        {
            SAL_WARN("sw.uno", "SwXTextCellStyle unknown property:" + sPropName);
            throw css::beans::UnknownPropertyException(sPropName);
        }
    }
    return aRet;
}

void SAL_CALL SwXTextCellStyle::setPropertyToDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    const SwBoxAutoFormat& rDefaultBoxFormat = SwTableAutoFormat::GetDefaultBoxFormat();
    const SfxItemPropertyMap& rMap = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CELL_STYLE)->getPropertyMap();
    const SfxItemPropertyMapEntry* pEntry = rMap.getByName(rPropertyName);
    if(!pEntry)
        return;

    uno::Any aAny;
    switch(pEntry->nWID)
    {
        case RES_BACKGROUND:
        {
            SvxBrushItem rBrush = m_pBoxAutoFormat->GetBackground();
            rDefaultBoxFormat.GetBackground().QueryValue(aAny, pEntry->nMemberId);
            rBrush.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetBackground(rBrush);
            break;
        }
        case RES_BOX:
        {
            SvxBoxItem rBox = m_pBoxAutoFormat->GetBox();
            rDefaultBoxFormat.GetBox().QueryValue(aAny, pEntry->nMemberId);
            rBox.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetBox(rBox);
            break;
        }
        case RES_VERT_ORIENT:
        {
            SwFormatVertOrient rVertOrient = m_pBoxAutoFormat->GetVerticalAlignment();
            rDefaultBoxFormat.GetVerticalAlignment().QueryValue(aAny, pEntry->nMemberId);
            rVertOrient.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetVerticalAlignment(rVertOrient);
            break;
        }
        case RES_FRAMEDIR:
        {
            SvxFrameDirectionItem rFrameDirectionItem = m_pBoxAutoFormat->GetTextOrientation();
            rDefaultBoxFormat.GetTextOrientation().QueryValue(aAny, pEntry->nMemberId);
            rFrameDirectionItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetTextOrientation(rFrameDirectionItem);
            break;
        }
        case RES_BOXATR_FORMAT:
        {
            OUString sFormat;
            LanguageType eLng, eSys;
            rDefaultBoxFormat.GetValueFormat(sFormat, eLng, eSys);
            m_pBoxAutoFormat->SetValueFormat(sFormat, eLng, eSys);
            break;
        }
        case RES_PARATR_ADJUST:
        {
            SvxAdjustItem rAdjustItem = m_pBoxAutoFormat->GetAdjust();
            rDefaultBoxFormat.GetAdjust().QueryValue(aAny, pEntry->nMemberId);
            rAdjustItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetAdjust(rAdjustItem);
            break;
        }
        case RES_CHRATR_COLOR:
        {
            SvxColorItem rColorItem = m_pBoxAutoFormat->GetColor();
            rDefaultBoxFormat.GetColor().QueryValue(aAny, pEntry->nMemberId);
            rColorItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetColor(rColorItem);
            break;
        }
        case RES_CHRATR_SHADOWED:
        {
            SvxShadowedItem rShadowedItem = m_pBoxAutoFormat->GetShadowed();
            rDefaultBoxFormat.GetShadowed().QueryValue(aAny, pEntry->nMemberId);
            rShadowedItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetShadowed(rShadowedItem);
            break;
        }
        case RES_CHRATR_CONTOUR:
        {
            SvxContourItem rContourItem = m_pBoxAutoFormat->GetContour();
            rDefaultBoxFormat.GetContour().QueryValue(aAny, pEntry->nMemberId);
            rContourItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetContour(rContourItem);
            break;
        }
        case RES_CHRATR_CROSSEDOUT:
        {
            SvxCrossedOutItem rCrossedOutItem = m_pBoxAutoFormat->GetCrossedOut();
            rDefaultBoxFormat.GetCrossedOut().QueryValue(aAny, pEntry->nMemberId);
            rCrossedOutItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCrossedOut(rCrossedOutItem);
            break;
        }
        case RES_CHRATR_UNDERLINE:
        {
            SvxUnderlineItem rUnderlineItem = m_pBoxAutoFormat->GetUnderline();
            rDefaultBoxFormat.GetUnderline().QueryValue(aAny, pEntry->nMemberId);
            rUnderlineItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetUnderline(rUnderlineItem);
            break;
        }
        case RES_CHRATR_FONTSIZE:
        {
            SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetHeight();
            rDefaultBoxFormat.GetHeight().QueryValue(aAny, pEntry->nMemberId);
            rFontHeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetHeight(rFontHeightItem);
            break;
        }
        case RES_CHRATR_WEIGHT:
        {
            SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetWeight();
            rDefaultBoxFormat.GetWeight().QueryValue(aAny, pEntry->nMemberId);
            rWeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetWeight(rWeightItem);
            break;
        }
        case RES_CHRATR_POSTURE:
        {
            SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetPosture();
            rDefaultBoxFormat.GetPosture().QueryValue(aAny, pEntry->nMemberId);
            rPostureItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetPosture(rPostureItem);
            break;
        }
        case RES_CHRATR_FONT:
        {
            SvxFontItem rFontItem = m_pBoxAutoFormat->GetFont();
            rDefaultBoxFormat.GetFont().QueryValue(aAny, pEntry->nMemberId);
            rFontItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetFont(rFontItem);
            break;
        }
        case RES_CHRATR_CJK_FONTSIZE:
        {
            SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCJKHeight();
            rDefaultBoxFormat.GetCJKHeight().QueryValue(aAny, pEntry->nMemberId);
            rFontHeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCJKHeight(rFontHeightItem);
            break;
        }
        case RES_CHRATR_CJK_WEIGHT:
        {
            SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetCJKWeight();
            rDefaultBoxFormat.GetCJKWeight().QueryValue(aAny, pEntry->nMemberId);
            rWeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCJKWeight(rWeightItem);
            break;
        }
        case RES_CHRATR_CJK_POSTURE:
        {
            SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetCJKPosture();
            rDefaultBoxFormat.GetCJKPosture().QueryValue(aAny, pEntry->nMemberId);
            rPostureItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCJKPosture(rPostureItem);
            break;
        }
        case RES_CHRATR_CJK_FONT:
        {
            SvxFontItem rFontItem = m_pBoxAutoFormat->GetCJKFont();
            rDefaultBoxFormat.GetCJKFont().QueryValue(aAny, pEntry->nMemberId);
            rFontItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCJKFont(rFontItem);
            break;
        }
        case RES_CHRATR_CTL_FONTSIZE:
        {
            SvxFontHeightItem rFontHeightItem = m_pBoxAutoFormat->GetCTLHeight();
            rDefaultBoxFormat.GetCTLHeight().QueryValue(aAny, pEntry->nMemberId);
            rFontHeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCTLHeight(rFontHeightItem);
            break;
        }
        case RES_CHRATR_CTL_WEIGHT:
        {
            SvxWeightItem rWeightItem = m_pBoxAutoFormat->GetCTLWeight();
            rDefaultBoxFormat.GetCTLWeight().QueryValue(aAny, pEntry->nMemberId);
            rWeightItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCTLWeight(rWeightItem);
            break;
        }
        case RES_CHRATR_CTL_POSTURE:
        {
            SvxPostureItem rPostureItem = m_pBoxAutoFormat->GetCTLPosture();
            rDefaultBoxFormat.GetCTLPosture().QueryValue(aAny, pEntry->nMemberId);
            rPostureItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCTLPosture(rPostureItem);
            break;
        }
        case RES_CHRATR_CTL_FONT:
        {
            SvxFontItem rFontItem = m_pBoxAutoFormat->GetCTLFont();
            rDefaultBoxFormat.GetCTLFont().QueryValue(aAny, pEntry->nMemberId);
            rFontItem.PutValue(aAny, pEntry->nMemberId);
            m_pBoxAutoFormat->SetCTLFont(rFontItem);
            break;
        }
        default:
            SAL_WARN("sw.uno", "SwXTextCellStyle setPropertyToDefault unknown nWID");
    }
}

css::uno::Any SAL_CALL SwXTextCellStyle::getPropertyDefault(const OUString& /*aPropertyName*/)
{
    SAL_WARN("sw.uno", "not implemented");
    uno::Any aRet;
    return aRet;
}

//XServiceInfo
OUString SAL_CALL SwXTextCellStyle::getImplementationName()
{
    return {"SwXTextCellStyle"};
}

sal_Bool SAL_CALL SwXTextCellStyle::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL SwXTextCellStyle::getSupportedServiceNames()
{
    return {"com.sun.star.style.Style"};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
