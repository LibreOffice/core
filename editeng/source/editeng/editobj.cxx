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

#include <memory>

#include <o3tl/safeint.hxx>
#include <sal/log.hxx>

#include <editeng/macros.hxx>
#include <editeng/section.hxx>
#include "editobj2.hxx"
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>

#include <svl/sharedstringpool.hxx>

#include <libxml/xmlwriter.h>
#include <algorithm>
#include <cassert>

#if DEBUG_EDIT_ENGINE
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace com::sun::star;


static XEditAttribute MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    // Create the new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    return XEditAttribute( rNew, nStart, nEnd );
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE )
    : pItem(&rAttr)
    , nStart(nS)
    , nEnd(nE)
{
}

bool XEditAttribute::IsFeature() const
{
    sal_uInt16 nWhich = pItem->Which();
    return  ((nWhich >= EE_FEATURE_START) && (nWhich <=  EE_FEATURE_END));
}

void XEditAttribute::SetItem(const SfxPoolItem& rNew)
{
    pItem = &rNew;
}

XParaPortionList::XParaPortionList(OutputDevice* pRefDev, sal_uInt32 nPW,
            double fFontScaleX, double fFontScaleY,
            double fSpacingScaleX, double fSpacingScaleY)
    : pRefDevPtr(pRefDev)
    , mfFontScaleX(fFontScaleX)
    , mfFontScaleY(fFontScaleY)
    , mfSpacingScaleX(fSpacingScaleX)
    , mfSpacingScaleY(fSpacingScaleY)
    , nPaperWidth(nPW)
{
}

void XParaPortionList::push_back(XParaPortion* p)
{
    maList.push_back(std::unique_ptr<XParaPortion>(p));
}

const XParaPortion& XParaPortionList::operator [](size_t i) const
{
    return *maList[i];
}

ContentInfo::ContentInfo( SfxItemPool& rPool ) :
    eFamily(SfxStyleFamily::Para),
    aParaAttribs(rPool)
{
}

// the real Copy constructor is nonsense, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse ) :
    maText(rCopyFrom.maText),
    aStyle(rCopyFrom.aStyle),
    eFamily(rCopyFrom.eFamily),
    aParaAttribs(rPoolToUse)
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );

    for (const XEditAttribute & rAttr : rCopyFrom.maCharAttribs)
    {
        XEditAttribute aMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        maCharAttribs.push_back(aMyAttr);
    }

    if ( rCopyFrom.GetWrongList() )
        mpWrongs.reset(rCopyFrom.GetWrongList()->Clone());
}

ContentInfo::~ContentInfo()
{
    for (auto const& charAttrib : maCharAttribs)
        aParaAttribs.GetPool()->Remove(*charAttrib.GetItem());
    maCharAttribs.clear();
}

void ContentInfo::NormalizeString( svl::SharedStringPool& rPool )
{
    maText = rPool.intern(OUString(maText.getData()));
}


OUString ContentInfo::GetText() const
{
    rtl_uString* p = const_cast<rtl_uString*>(maText.getData());
    return OUString(p);
}

sal_Int32 ContentInfo::GetTextLen() const
{
    const rtl_uString* p = maText.getData();
    return p->length;
}

void ContentInfo::SetText( const OUString& rStr )
{
    maText = svl::SharedString(rStr.pData, nullptr);
}

void ContentInfo::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ContentInfo"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("style"), BAD_CAST(aStyle.toUtf8().getStr()));
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("text"));
    OUString aText = GetText();
    // TODO share code with sax_fastparser::FastSaxSerializer::write().
    (void)xmlTextWriterWriteString(pWriter, BAD_CAST(aText.replaceAll("\x01", "&#1;").toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    aParaAttribs.dumpAsXml(pWriter);
    for (auto const& rCharAttribs : maCharAttribs)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("attribs"));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("start"), "%" SAL_PRIdINT32, rCharAttribs.GetStart());
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("end"), "%" SAL_PRIdINT32, rCharAttribs.GetEnd());
        rCharAttribs.GetItem()->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

const WrongList* ContentInfo::GetWrongList() const
{
    return mpWrongs.get();
}

void ContentInfo::SetWrongList( WrongList* p )
{
    mpWrongs.reset(p);
}

// #i102062#
bool ContentInfo::isWrongListEqual(const ContentInfo& rCompare) const
{
    if(GetWrongList() == rCompare.GetWrongList())
        return true;

    if(!GetWrongList() || !rCompare.GetWrongList())
        return false;

    return (*GetWrongList() == *rCompare.GetWrongList());
}

#if DEBUG_EDIT_ENGINE
void ContentInfo::Dump() const
{
    cout << "--" << endl;
    cout << "text: '" << OUString(const_cast<rtl_uString*>(maText.getData())) << "'" << endl;
    cout << "style: '" << aStyle << "'" << endl;

    for (auto const& attrib : aAttribs)
    {
        cout << "attribute: " << endl;
        cout << "  span: [begin=" << attrib.GetStart() << ", end=" << attrib.GetEnd() << "]" << endl;
        cout << "  feature: " << (attrib.IsFeature() ? "yes":"no") << endl;
    }
}
#endif

bool ContentInfo::Equals(const ContentInfo& rCompare, bool bComparePool) const
{
    return maText == rCompare.maText && aStyle == rCompare.aStyle && eFamily == rCompare.eFamily
           && aParaAttribs.Equals(rCompare.aParaAttribs, bComparePool)
           && maCharAttribs == rCompare.maCharAttribs;
}

EditTextObject::~EditTextObject() = default;

std::unique_ptr<EditTextObject> EditTextObjectImpl::Clone() const
{
    return std::make_unique<EditTextObjectImpl>(*this);
}

bool EditTextObject::Equals( const EditTextObject& rCompare ) const
{
    return toImpl(*this).Equals(toImpl(rCompare), false /*bComparePool*/);
}

void EditTextObjectImpl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("editTextObject.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        (void)xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("EditTextObject"));
    sal_Int32 nCount = GetParagraphCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        maContents[i]->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);

    if (bOwns)
    {
       (void)xmlTextWriterEndDocument(pWriter);
       xmlFreeTextWriter(pWriter);
    }
}

#if DEBUG_EDIT_ENGINE
void EditTextObjectImpl::Dump() const
{
    for (auto const& content : maContents)
        content.Dump();
}
#endif

static rtl::Reference<SfxItemPool> getEditEngineItemPool(SfxItemPool* pPool, MapUnit eDefaultMetric)
{
    // #i101239# ensure target is an EditEngineItemPool, so that at
    // pool destruction time of an alien pool, the pool is still alive.
    // When registering would happen at an alien pool which just uses an
    // EditEngineItemPool as some sub-pool, that pool could already
    // be decoupled and deleted which would lead to crashes.
    for (; pPool; pPool = pPool->GetSecondaryPool())
        if (dynamic_cast<EditEngineItemPool*>(pPool))
            return pPool;

    auto pRetval = EditEngine::CreatePool();
    pRetval->SetDefaultMetric(eDefaultMetric);
    return pRetval;
}

EditTextObjectImpl::EditTextObjectImpl(SfxItemPool* pP, MapUnit eDefaultMetric, bool bVertical,
    TextRotation eRotation, SvtScriptType eScriptType)
    : mpPool(getEditEngineItemPool(pP, eDefaultMetric))
    , meUserType(OutlinerMode::DontKnow)
    , meScriptType(eScriptType)
    , meRotation(eRotation)
    , meMetric(eDefaultMetric)
    , mbVertical(bVertical)
{
}

EditTextObjectImpl::EditTextObjectImpl( const EditTextObjectImpl& r )
    : mpPool(r.mpPool)
    , meUserType(r.meUserType)
    , meScriptType(r.meScriptType)
    , meRotation(r.meRotation)
    , meMetric(r.meMetric)
    , mbVertical(r.mbVertical)
{
    // Do not copy PortionInfo

    maContents.reserve(r.maContents.size());
    for (auto const& content : r.maContents)
        maContents.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*content, *mpPool)));
}

EditTextObjectImpl::~EditTextObjectImpl()
{
    ClearPortionInfo();

    // Remove contents before deleting the pool instance since each content
    // has to access the pool instance in its destructor.
    maContents.clear();
}


void EditTextObjectImpl::SetUserType( OutlinerMode n )
{
    meUserType = n;
}

void EditTextObjectImpl::NormalizeString( svl::SharedStringPool& rPool )
{
    for (auto const& content : maContents)
    {
        ContentInfo& rInfo = *content;
        rInfo.NormalizeString(rPool);
    }
}

std::vector<svl::SharedString> EditTextObjectImpl::GetSharedStrings() const
{
    std::vector<svl::SharedString> aSSs;
    aSSs.reserve(maContents.size());
    for (auto const& content : maContents)
    {
        const ContentInfo& rInfo = *content;
        aSSs.push_back(rInfo.GetSharedString());
    }
    return aSSs;
}

bool EditTextObjectImpl::IsEffectivelyVertical() const
{
    return (mbVertical && meRotation == TextRotation::NONE) ||
        (!mbVertical && meRotation != TextRotation::NONE);
}

bool EditTextObjectImpl::IsTopToBottom() const
{
    return (mbVertical && meRotation == TextRotation::NONE) ||
        (!mbVertical && meRotation == TextRotation::TOPTOBOTTOM);
}

void EditTextObjectImpl::SetVertical( bool bVert)
{
    if (bVert != mbVertical)
    {
        mbVertical = bVert;
        ClearPortionInfo();
    }
}

bool EditTextObjectImpl::GetVertical() const
{
    return mbVertical;
}

void EditTextObjectImpl::SetRotation(TextRotation nRotation)
{
    if (meRotation != nRotation)
    {
        meRotation = nRotation;
        ClearPortionInfo();
    }
}

TextRotation EditTextObjectImpl::GetRotation() const
{
    return meRotation;
}

XEditAttribute EditTextObjectImpl::CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    return MakeXEditAttribute( *mpPool, rItem, nStart, nEnd );
}

void EditTextObjectImpl::DestroyAttrib( const XEditAttribute& rAttr )
{
    mpPool->Remove( *rAttr.GetItem() );
}


ContentInfo* EditTextObjectImpl::CreateAndInsertContent()
{
    maContents.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*mpPool)));
    return maContents.back().get();
}

sal_Int32 EditTextObjectImpl::GetParagraphCount() const
{
    size_t nSize = maContents.size();
    if (nSize > EE_PARA_MAX_COUNT)
    {
        SAL_WARN( "editeng", "EditTextObjectImpl::GetParagraphCount - overflow " << nSize);
        return EE_PARA_MAX_COUNT;
    }
    return static_cast<sal_Int32>(nSize);
}

OUString EditTextObjectImpl::GetText(sal_Int32 nPara) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return OUString();

    return maContents[nPara]->GetText();
}

sal_Int32 EditTextObjectImpl::GetTextLen(sal_Int32 nPara ) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return 0;

    return maContents[nPara]->GetTextLen();
}

void EditTextObjectImpl::ClearPortionInfo()
{
    mpPortionInfo.reset();
}

bool EditTextObjectImpl::HasOnlineSpellErrors() const
{
    for (auto const& content : maContents)
    {
        if ( content->GetWrongList() && !content->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditTextObjectImpl::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return;

    rLst.clear();
    const ContentInfo& rC = *maContents[nPara];
    for (const XEditAttribute & rAttr : rC.maCharAttribs)
    {
        EECharAttrib aEEAttr(rAttr.GetStart(), rAttr.GetEnd(), rAttr.GetItem());
        rLst.push_back(aEEAttr);
    }
}

bool EditTextObjectImpl::IsFieldObject() const
{
    return GetField() != nullptr;
}

const SvxFieldItem* EditTextObjectImpl::GetField() const
{
    if (maContents.size() == 1)
    {
        const ContentInfo& rC = *maContents[0];
        if (rC.GetText().getLength() == 1)
        {
            size_t nAttribs = rC.maCharAttribs.size();
            for (size_t nAttr = nAttribs; nAttr; )
            {
                const XEditAttribute& rX = rC.maCharAttribs[--nAttr];
                if (rX.GetItem()->Which() == EE_FEATURE_FIELD)
                    return static_cast<const SvxFieldItem*>(rX.GetItem());
            }
        }
    }
    return nullptr;
}

const SvxFieldData* EditTextObjectImpl::GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return nullptr;

    const ContentInfo& rC = *maContents[nPara];
    if (nPos >= rC.maCharAttribs.size())
        // URL position is out-of-bound.
        return nullptr;

    size_t nCurPos = 0;
    for (XEditAttribute const& rAttr : rC.maCharAttribs)
    {
        if (rAttr.GetItem()->Which() != EE_FEATURE_FIELD)
            // Skip attributes that are not fields.
            continue;

        const SvxFieldItem* pField = static_cast<const SvxFieldItem*>(rAttr.GetItem());
        const SvxFieldData* pFldData = pField->GetField();
        if (nType != text::textfield::Type::UNSPECIFIED && nType != pFldData->GetClassId())
            // Field type doesn't match. Skip it.  UNSPECIFIED matches all field types.
            continue;

        if (nCurPos == nPos)
            // Found it!
            return pFldData;

        ++nCurPos;
    }

    return nullptr; // field not found.
}

bool EditTextObjectImpl::HasField( sal_Int32 nType ) const
{
    size_t nParagraphs = maContents.size();
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = *maContents[nPara];
        size_t nAttrs = rC.maCharAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = rC.maCharAttribs[nAttr];
            if (rAttr.GetItem()->Which() != EE_FEATURE_FIELD)
                continue;

            if (nType == text::textfield::Type::UNSPECIFIED)
                // Match any field type.
                return true;

            const SvxFieldData* pFldData = static_cast<const SvxFieldItem*>(rAttr.GetItem())->GetField();
            if (pFldData && pFldData->GetClassId() == nType)
                return true;
        }
    }
    return false;
}

const SfxItemSet& EditTextObjectImpl::GetParaAttribs(sal_Int32 nPara) const
{
    const ContentInfo& rC = *maContents[nPara];
    return rC.GetParaAttribs();
}

bool EditTextObjectImpl::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    bool bChanged = false;

    for ( size_t nPara = maContents.size(); nPara; )
    {
        ContentInfo& rC = *maContents[--nPara];

        for (size_t nAttr = rC.maCharAttribs.size(); nAttr; )
        {
            XEditAttribute& rAttr = rC.maCharAttribs[--nAttr];
            if ( !_nWhich || (rAttr.GetItem()->Which() == _nWhich) )
            {
                mpPool->Remove(*rAttr.GetItem());
                rC.maCharAttribs.erase(rC.maCharAttribs.begin()+nAttr);
                bChanged = true;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

namespace {

class FindByParagraph
{
    sal_Int32 mnPara;
public:
    explicit FindByParagraph(sal_Int32 nPara) : mnPara(nPara) {}
    bool operator() (const editeng::Section& rAttr) const
    {
        return rAttr.mnParagraph == mnPara;
    }
};

class FindBySectionStart
{
    sal_Int32 mnPara;
    sal_Int32 mnStart;
public:
    FindBySectionStart(sal_Int32 nPara, sal_Int32 nStart) : mnPara(nPara), mnStart(nStart) {}
    bool operator() (const editeng::Section& rAttr) const
    {
        return rAttr.mnParagraph == mnPara && rAttr.mnStart == mnStart;
    }
};

}

void EditTextObjectImpl::GetAllSections( std::vector<editeng::Section>& rAttrs ) const
{
    std::vector<editeng::Section> aAttrs;
    aAttrs.reserve(maContents.size());
    std::vector<size_t> aBorders;

    for (size_t nPara = 0; nPara < maContents.size(); ++nPara)
    {
        aBorders.clear();
        const ContentInfo& rC = *maContents[nPara];
        aBorders.push_back(0);
        aBorders.push_back(rC.GetText().getLength());
        for (const XEditAttribute & rAttr : rC.maCharAttribs)
        {
            const SfxPoolItem* pItem = rAttr.GetItem();
            if (!pItem)
                continue;

            aBorders.push_back(rAttr.GetStart());
            aBorders.push_back(rAttr.GetEnd());
        }

        // Sort and remove duplicates for each paragraph.
        std::sort(aBorders.begin(), aBorders.end());
        auto itUniqueEnd = std::unique(aBorders.begin(), aBorders.end());
        aBorders.erase(itUniqueEnd, aBorders.end());

        // Create storage for each section.  Note that this creates storage even
        // for unformatted sections.  The entries are sorted first by paragraph,
        // then by section positions.  They don't overlap with each other.

        if (aBorders.size() == 1 && aBorders[0] == 0)
        {
            // Empty paragraph. Push an empty section.
            aAttrs.emplace_back(nPara, 0, 0);
            continue;
        }

        auto itBorder = aBorders.begin(), itBorderEnd = aBorders.end();
        size_t nPrev = *itBorder;
        size_t nCur;
        for (++itBorder; itBorder != itBorderEnd; ++itBorder, nPrev = nCur)
        {
            nCur = *itBorder;
            aAttrs.emplace_back(nPara, nPrev, nCur);
        }
    }

    if (aAttrs.empty())
        return;

    // Go through all formatted paragraphs, and store format items.
    std::vector<editeng::Section>::iterator itAttr = aAttrs.begin();
    for (sal_Int32 nPara = 0; nPara < static_cast<sal_Int32>(maContents.size()); ++nPara)
    {
        const ContentInfo& rC = *maContents[nPara];

        itAttr = std::find_if(itAttr, aAttrs.end(), FindByParagraph(nPara));
        if (itAttr == aAttrs.end())
        {
            // This should never happen. There is a logic error somewhere...
            assert(false);
            return;
        }

        for (const XEditAttribute & rXAttr : rC.maCharAttribs)
        {
            const SfxPoolItem* pItem = rXAttr.GetItem();
            if (!pItem)
                continue;

            sal_Int32 nStart = rXAttr.GetStart(), nEnd = rXAttr.GetEnd();

            // Find the container whose start position matches.
            std::vector<editeng::Section>::iterator itCurAttr = std::find_if(itAttr, aAttrs.end(), FindBySectionStart(nPara, nStart));
            if (itCurAttr == aAttrs.end())
            {
                // This should never happen. There is a logic error somewhere...
                assert(false);
                return;
            }

            for (; itCurAttr != aAttrs.end() && itCurAttr->mnParagraph == nPara && itCurAttr->mnEnd <= nEnd; ++itCurAttr)
            {
                editeng::Section& rSecAttr = *itCurAttr;
                // serious bug: will cause duplicate attributes to be exported
                if (std::none_of(rSecAttr.maAttributes.begin(), rSecAttr.maAttributes.end(),
                    [&pItem](SfxPoolItem const*const pIt)
                        { return pIt->Which() == pItem->Which(); }))
                {
                    rSecAttr.maAttributes.push_back(pItem);
                }
                else
                {
                    SAL_WARN("editeng", "GetAllSections(): duplicate attribute suppressed");
                }
            }
        }
    }

    rAttrs.swap(aAttrs);
}

void EditTextObjectImpl::GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& rFamily) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return;

    const ContentInfo& rC = *maContents[nPara];
    rName = rC.GetStyle();
    rFamily = rC.GetFamily();
}

void EditTextObjectImpl::SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& rFamily)
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= maContents.size())
        return;

    ContentInfo& rC = *maContents[nPara];
    rC.SetStyle(rName);
    rC.SetFamily(rFamily);
}

bool EditTextObjectImpl::ImpChangeStyleSheets(
                    std::u16string_view rOldName, SfxStyleFamily eOldFamily,
                    const OUString& rNewName, SfxStyleFamily eNewFamily )
{
    const size_t nParagraphs = maContents.size();
    bool bChanges = false;

    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        ContentInfo& rC = *maContents[nPara];
        if ( rC.GetFamily() == eOldFamily )
        {
            if ( rC.GetStyle() == rOldName )
            {
                rC.SetStyle(rNewName);
                rC.SetFamily(eNewFamily);
                bChanges = true;
            }
        }
    }
    return bChanges;
}

bool EditTextObjectImpl::ChangeStyleSheets(
                    std::u16string_view rOldName, SfxStyleFamily eOldFamily,
                    const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    bool bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void EditTextObjectImpl::ChangeStyleSheetName( SfxStyleFamily eFamily,
                std::u16string_view rOldName, const OUString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

bool EditTextObjectImpl::operator==( const EditTextObject& rCompare ) const
{
    return Equals(toImpl(rCompare), true);
}

bool EditTextObjectImpl::Equals( const EditTextObjectImpl& rCompare, bool bComparePool ) const
{
    if( this == &rCompare )
        return true;

    if(     ( bComparePool && mpPool != rCompare.mpPool ) ||
            ( meMetric != rCompare.meMetric ) ||
            ( meUserType!= rCompare.meUserType ) ||
            ( meScriptType != rCompare.meScriptType ) ||
            ( mbVertical != rCompare.mbVertical ) ||
            ( meRotation != rCompare.meRotation ) )
        return false;

    return std::equal(
        maContents.begin(), maContents.end(), rCompare.maContents.begin(), rCompare.maContents.end(),
        [bComparePool](const auto& c1, const auto& c2) { return c1->Equals(*c2, bComparePool); });
}

// #i102062#
bool EditTextObjectImpl::isWrongListEqual(const EditTextObject& rComp) const
{
    const EditTextObjectImpl& rCompare = toImpl(rComp);
    return std::equal(
        maContents.begin(), maContents.end(), rCompare.maContents.begin(), rCompare.maContents.end(),
        [](const auto& c1, const auto& c2) { return c1->isWrongListEqual(*c2); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
