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

#include <editeng/fieldupdater.hxx>
#include <editeng/macros.hxx>
#include <editeng/section.hxx>
#include "editobj2.hxx"
#include <editeng/editdata.hxx>
#include <editattr.hxx>
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


static std::unique_ptr<XEditAttribute> MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    // Create the new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    std::unique_ptr<XEditAttribute> pNew(new XEditAttribute( rNew, nStart, nEnd ));
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE )
    : pItem(&rAttr)
    , nStart(nS)
    , nEnd(nE)
{
}

XEditAttribute::~XEditAttribute()
{
    pItem = nullptr;  // belongs to the Pool.
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

XParaPortionList::XParaPortionList(
    OutputDevice* pRefDev, sal_uInt32 nPW, sal_uInt16 _nStretchX, sal_uInt16 _nStretchY)
    : pRefDevPtr(pRefDev)
    , nStretchX(_nStretchX)
    , nStretchY(_nStretchY)
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
    aParaAttribs(rPool, svl::Items<EE_PARA_START, EE_CHAR_END>{})
{
}

// the real Copy constructor is nonsense, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse ) :
    maText(rCopyFrom.maText),
    aStyle(rCopyFrom.aStyle),
    eFamily(rCopyFrom.eFamily),
    aParaAttribs(rPoolToUse, svl::Items<EE_PARA_START, EE_CHAR_END>{})
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );

    for (const auto & aAttrib : rCopyFrom.maCharAttribs)
    {
        const XEditAttribute& rAttr = *aAttrib;
        std::unique_ptr<XEditAttribute> pMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        maCharAttribs.push_back(std::move(pMyAttr));
    }

    if ( rCopyFrom.GetWrongList() )
        mpWrongs.reset(rCopyFrom.GetWrongList()->Clone());
}

ContentInfo::~ContentInfo()
{
    for (auto const& charAttrib : maCharAttribs)
        aParaAttribs.GetPool()->Remove(*charAttrib->GetItem());
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

void ContentInfo::SetText( const OUString& rStr )
{
    maText = svl::SharedString(rStr.pData, nullptr);
}

void ContentInfo::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ContentInfo"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("style"), BAD_CAST(aStyle.toUtf8().getStr()));
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("text"));
    (void)xmlTextWriterWriteString(pWriter, BAD_CAST(GetText().toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    aParaAttribs.dumpAsXml(pWriter);
    for (size_t i=0; i<maCharAttribs.size(); ++i)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("attribs"));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("start"), "%" SAL_PRIdINT32, maCharAttribs[i]->GetStart());
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("end"), "%" SAL_PRIdINT32, maCharAttribs[i]->GetEnd());
        maCharAttribs[i]->GetItem()->dumpAsXml(pWriter);
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
           && std::equal(maCharAttribs.cbegin(), maCharAttribs.cend(),
                         rCompare.maCharAttribs.cbegin(), rCompare.maCharAttribs.cend(),
                         [](const std::unique_ptr<XEditAttribute>& pAttribute1,
                            const std::unique_ptr<XEditAttribute>& pAttribute2) -> bool {
                             return *pAttribute1 == *pAttribute2;
                         });
}

EditTextObject::EditTextObject( SfxItemPool* pPool ) :
    mpImpl(new EditTextObjectImpl(this, pPool))
{
}

EditTextObject::EditTextObject( const EditTextObject& r ) :
    SfxItemPoolUser(),
    mpImpl(new EditTextObjectImpl(this, *r.mpImpl))
{
}

EditTextObject::~EditTextObject()
{
}

sal_Int32 EditTextObject::GetParagraphCount() const
{
    return mpImpl->GetParagraphCount();
}

OUString EditTextObject::GetText(sal_Int32 nPara) const
{
    return mpImpl->GetText(nPara);
}

void EditTextObject::ClearPortionInfo()
{
    mpImpl->ClearPortionInfo();
}

bool EditTextObject::HasOnlineSpellErrors() const
{
    return mpImpl->HasOnlineSpellErrors();
}

void EditTextObject::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    mpImpl->GetCharAttribs(nPara, rLst);
}

bool EditTextObject::IsFieldObject() const
{
    return mpImpl->IsFieldObject();
}

const SvxFieldItem* EditTextObject::GetField() const
{
    return mpImpl->GetField();
}

const SvxFieldData* EditTextObject::GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const
{
    return mpImpl->GetFieldData(nPara, nPos, nType);
}

bool EditTextObject::HasField( sal_Int32 nType ) const
{
    return mpImpl->HasField(nType);
}

const SfxItemSet& EditTextObject::GetParaAttribs(sal_Int32 nPara) const
{
    return mpImpl->GetParaAttribs(nPara);
}

bool EditTextObject::RemoveCharAttribs( sal_uInt16 nWhich )
{
    return mpImpl->RemoveCharAttribs(nWhich);
}

void EditTextObject::GetAllSections( std::vector<editeng::Section>& rAttrs ) const
{
    mpImpl->GetAllSections(rAttrs);
}

void EditTextObject::GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const
{
    mpImpl->GetStyleSheet(nPara, rName, eFamily);
}

void EditTextObject::SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily)
{
    mpImpl->SetStyleSheet(nPara, rName, eFamily);
}

bool EditTextObject::ChangeStyleSheets(
    std::u16string_view rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    return mpImpl->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void EditTextObject::ChangeStyleSheetName(
    SfxStyleFamily eFamily, std::u16string_view rOldName, const OUString& rNewName)
{
    mpImpl->ChangeStyleSheetName(eFamily, rOldName, rNewName);
}

editeng::FieldUpdater EditTextObject::GetFieldUpdater() const
{
    return mpImpl->GetFieldUpdater();
}

void EditTextObject::NormalizeString( svl::SharedStringPool& rPool )
{
    mpImpl->NormalizeString(rPool);
}

std::vector<svl::SharedString> EditTextObject::GetSharedStrings() const
{
    return mpImpl->GetSharedStrings();
}

const SfxItemPool* EditTextObject::GetPool() const
{
    return mpImpl->GetPool();
}

OutlinerMode EditTextObject::GetUserType() const
{
    return mpImpl->GetUserType();
}

void EditTextObject::SetUserType( OutlinerMode n )
{
    mpImpl->SetUserType(n);
}

bool EditTextObject::IsVertical() const
{
    return mpImpl->IsVertical();
}

bool EditTextObject::GetDirectVertical() const
{
    return mpImpl->GetDirectVertical();
}

bool EditTextObject::IsTopToBottom() const
{
    return mpImpl->IsTopToBottom();
}

void EditTextObject::SetVertical( bool bVertical )
{
    return mpImpl->SetVertical(bVertical);
}

void EditTextObject::SetRotation( TextRotation nRotation )
{
    mpImpl->SetRotation(nRotation);
}

TextRotation EditTextObject::GetRotation() const
{
    return mpImpl->GetRotation();
}

SvtScriptType EditTextObject::GetScriptType() const
{
    return mpImpl->GetScriptType();
}


std::unique_ptr<EditTextObject> EditTextObject::Clone() const
{
    return std::unique_ptr<EditTextObject>(new EditTextObject(*this));
}

bool EditTextObject::operator==( const EditTextObject& rCompare ) const
{
    return mpImpl->operator==(*rCompare.mpImpl);
}

bool EditTextObject::Equals( const EditTextObject& rCompare ) const
{
    return mpImpl->Equals(*rCompare.mpImpl, false/*bComparePool*/);
}

// #i102062#
bool EditTextObject::isWrongListEqual(const EditTextObject& rCompare) const
{
    return mpImpl->isWrongListEqual(*rCompare.mpImpl);
}

void EditTextObject::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
{
    mpImpl->ObjectInDestruction(rSfxItemPool);
}

#if DEBUG_EDIT_ENGINE
void EditTextObject::Dump() const
{
    mpImpl->Dump();
}
#endif

void EditTextObject::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("editTextObject.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("EditTextObject"));
    sal_Int32 nCount = GetParagraphCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        mpImpl->aContents[i]->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);

    if (bOwns)
    {
       (void)xmlTextWriterEndDocument(pWriter);
       xmlFreeTextWriter(pWriter);
    }
}

// from SfxItemPoolUser
void EditTextObjectImpl::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
{
    if(bOwnerOfPool || pPool != &rSfxItemPool)
        return;

    // The pool we are based on gets destructed; get owner of pool by creating own one.
    // No need to call RemoveSfxItemPoolUser(), this is done from the pool's destructor
    // Base new pool on EditEnginePool; it would also be possible to clone the used
    // pool if needed, but only text attributes should be used.
    SfxItemPool* pNewPool = EditEngine::CreatePool();

    pNewPool->SetDefaultMetric(pPool->GetMetric(DEF_METRIC));

    ContentInfosType aReplaced;
    aReplaced.reserve(aContents.size());
    for (auto const& content : aContents)
        aReplaced.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*content, *pNewPool)));
    aReplaced.swap(aContents);

    // set local variables
    pPool = pNewPool;
    bOwnerOfPool = true;
}

#if DEBUG_EDIT_ENGINE
void EditTextObjectImpl::Dump() const
{
    for (auto const& content : aContents)
        content.Dump();
}
#endif

static EditEngineItemPool* getEditEngineItemPool(SfxItemPool* pPool)
{
    EditEngineItemPool* pRetval = dynamic_cast< EditEngineItemPool* >(pPool);

    while(!pRetval && pPool && pPool->GetSecondaryPool())
    {
        pPool = pPool->GetSecondaryPool();

        if(pPool)
        {
            pRetval = dynamic_cast< EditEngineItemPool* >(pPool);
        }
    }

    return pRetval;
}

EditTextObjectImpl::EditTextObjectImpl( EditTextObject* pFront, SfxItemPool* pP )
    : mpFront(pFront)
    , nMetric(0xFFFF)
    , nUserType(OutlinerMode::DontKnow)
    , nScriptType(SvtScriptType::NONE)
    , bVertical(false)
    , mnRotation(TextRotation::NONE)
{
    // #i101239# ensure target is an EditEngineItemPool, else
    // fallback to pool ownership. This is needed to ensure that at
    // pool destruction time of an alien pool, the pool is still alive.
    // When registering would happen at an alien pool which just uses an
    // EditEngineItemPool as some sub-pool, that pool could already
    // be decoupled and deleted which would lead to crashes.
    pPool = getEditEngineItemPool(pP);

    if ( pPool )
    {
        bOwnerOfPool = false;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool =  true;
    }

    if(!bOwnerOfPool && pPool)
    {
        // it is sure now that the pool is an EditEngineItemPool
        pPool->AddSfxItemPoolUser(*mpFront);
    }
}

EditTextObjectImpl::EditTextObjectImpl( EditTextObject* pFront, const EditTextObjectImpl& r )
    : mpFront(pFront)
    , nMetric(r.nMetric)
    , nUserType(r.nUserType)
    , nScriptType(r.nScriptType)
    , bVertical(r.bVertical)
    , mnRotation(r.mnRotation)
{
    // Do not copy PortionInfo

    if ( !r.bOwnerOfPool )
    {
        // reuse alien pool; this must be an EditEngineItemPool
        // since there is no other way to construct a BinTextObject
        // than it's regular constructor where that is ensured
        pPool = r.pPool;
        bOwnerOfPool = false;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool = true;

    }

    if (!bOwnerOfPool)
    {
        // it is sure now that the pool is an EditEngineItemPool
        pPool->AddSfxItemPoolUser(*mpFront);
    }

    if (bOwnerOfPool && r.pPool)
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    aContents.reserve(r.aContents.size());
    for (auto const& content : r.aContents)
        aContents.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*content, *pPool)));
}

EditTextObjectImpl::~EditTextObjectImpl()
{
    if(!bOwnerOfPool && pPool)
    {
        pPool->RemoveSfxItemPoolUser(*mpFront);
    }

    ClearPortionInfo();

    // Remove contents before deleting the pool instance since each content
    // has to access the pool instance in its destructor.
    aContents.clear();
    if ( bOwnerOfPool )
    {
        SfxItemPool::Free(pPool);
    }
}


void EditTextObjectImpl::SetUserType( OutlinerMode n )
{
    nUserType = n;
}

void EditTextObjectImpl::NormalizeString( svl::SharedStringPool& rPool )
{
    for (auto const& content : aContents)
    {
        ContentInfo& rInfo = *content;
        rInfo.NormalizeString(rPool);
    }
}

std::vector<svl::SharedString> EditTextObjectImpl::GetSharedStrings() const
{
    std::vector<svl::SharedString> aSSs;
    aSSs.reserve(aContents.size());
    for (auto const& content : aContents)
    {
        const ContentInfo& rInfo = *content;
        aSSs.push_back(rInfo.GetSharedString());
    }
    return aSSs;
}

bool EditTextObjectImpl::IsVertical() const
{
    return (bVertical && mnRotation == TextRotation::NONE) ||
        (!bVertical && mnRotation != TextRotation::NONE);
}

bool EditTextObjectImpl::IsTopToBottom() const
{
    return (bVertical && mnRotation == TextRotation::NONE) ||
        (!bVertical && mnRotation == TextRotation::TOPTOBOTTOM);
}

void EditTextObjectImpl::SetVertical( bool bVert)
{
    if (bVert != bVertical)
    {
        bVertical = bVert;
        ClearPortionInfo();
    }
}

bool EditTextObjectImpl::GetDirectVertical() const
{
    return bVertical;
}

void EditTextObjectImpl::SetRotation(TextRotation nRotation)
{
    if (mnRotation != nRotation)
    {
        mnRotation = nRotation;
        ClearPortionInfo();
    }
}

TextRotation EditTextObjectImpl::GetRotation() const
{
    return mnRotation;
}


void EditTextObjectImpl::SetScriptType( SvtScriptType nType )
{
    nScriptType = nType;
}

std::unique_ptr<XEditAttribute> EditTextObjectImpl::CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void EditTextObjectImpl::DestroyAttrib( std::unique_ptr<XEditAttribute> pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
}


ContentInfo* EditTextObjectImpl::CreateAndInsertContent()
{
    aContents.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*pPool)));
    return aContents.back().get();
}

sal_Int32 EditTextObjectImpl::GetParagraphCount() const
{
    size_t nSize = aContents.size();
    if (nSize > EE_PARA_MAX_COUNT)
    {
        SAL_WARN( "editeng", "EditTextObjectImpl::GetParagraphCount - overflow " << nSize);
        return EE_PARA_MAX_COUNT;
    }
    return static_cast<sal_Int32>(nSize);
}

OUString EditTextObjectImpl::GetText(sal_Int32 nPara) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= aContents.size())
        return OUString();

    return aContents[nPara]->GetText();
}

void EditTextObjectImpl::ClearPortionInfo()
{
    pPortionInfo.reset();
}

bool EditTextObjectImpl::HasOnlineSpellErrors() const
{
    for (auto const& content : aContents)
    {
        if ( content->GetWrongList() && !content->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditTextObjectImpl::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= aContents.size())
        return;

    rLst.clear();
    const ContentInfo& rC = *aContents[nPara];
    for (const auto & aAttrib : rC.maCharAttribs)
    {
        const XEditAttribute& rAttr = *aAttrib;
        EECharAttrib aEEAttr;
        aEEAttr.pAttr = rAttr.GetItem();
        aEEAttr.nStart = rAttr.GetStart();
        aEEAttr.nEnd = rAttr.GetEnd();
        rLst.push_back(aEEAttr);
    }
}

bool EditTextObjectImpl::IsFieldObject() const
{
    return GetField() != nullptr;
}

const SvxFieldItem* EditTextObjectImpl::GetField() const
{
    if (aContents.size() == 1)
    {
        const ContentInfo& rC = *aContents[0];
        if (rC.GetText().getLength() == 1)
        {
            size_t nAttribs = rC.maCharAttribs.size();
            for (size_t nAttr = nAttribs; nAttr; )
            {
                const XEditAttribute& rX = *rC.maCharAttribs[--nAttr];
                if (rX.GetItem()->Which() == EE_FEATURE_FIELD)
                    return static_cast<const SvxFieldItem*>(rX.GetItem());
            }
        }
    }
    return nullptr;
}

const SvxFieldData* EditTextObjectImpl::GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= aContents.size())
        return nullptr;

    const ContentInfo& rC = *aContents[nPara];
    if (nPos >= rC.maCharAttribs.size())
        // URL position is out-of-bound.
        return nullptr;

    size_t nCurPos = 0;
    for (auto const& charAttrib : rC.maCharAttribs)
    {
        const XEditAttribute& rAttr = *charAttrib;
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
    size_t nParagraphs = aContents.size();
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara];
        size_t nAttrs = rC.maCharAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = *rC.maCharAttribs[nAttr];
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
    const ContentInfo& rC = *aContents[nPara];
    return rC.GetParaAttribs();
}

bool EditTextObjectImpl::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    bool bChanged = false;

    for ( size_t nPara = aContents.size(); nPara; )
    {
        ContentInfo& rC = *aContents[--nPara];

        for (size_t nAttr = rC.maCharAttribs.size(); nAttr; )
        {
            XEditAttribute& rAttr = *rC.maCharAttribs[--nAttr];
            if ( !_nWhich || (rAttr.GetItem()->Which() == _nWhich) )
            {
                pPool->Remove(*rAttr.GetItem());
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
    std::vector<std::vector<size_t>> aParaBorders(aContents.size());

    // First pass: determine section borders for each paragraph.
    for (size_t nPara = 0; nPara < aContents.size(); ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara];
        std::vector<size_t>& rBorders = aParaBorders[nPara];
        rBorders.push_back(0);
        rBorders.push_back(rC.GetText().getLength());
        for (const auto & aAttrib : rC.maCharAttribs)
        {
            const XEditAttribute& rAttr = *aAttrib;
            const SfxPoolItem* pItem = rAttr.GetItem();
            if (!pItem)
                continue;

            rBorders.push_back(rAttr.GetStart());
            rBorders.push_back(rAttr.GetEnd());
        }
    }

    // Sort and remove duplicates for each paragraph.
    for (auto & paraBorders : aParaBorders)
    {
        std::sort(paraBorders.begin(), paraBorders.end());
        auto itUniqueEnd = std::unique(paraBorders.begin(), paraBorders.end());
        paraBorders.erase(itUniqueEnd, paraBorders.end());
    }

    std::vector<editeng::Section> aAttrs;

    // Create storage for each section.  Note that this creates storage even
    // for unformatted sections.  The entries are sorted first by paragraph,
    // then by section positions.  They don't overlap with each other.
    size_t nPara1 = 0;
    for (auto const& paraBorders : aParaBorders)
    {
        if (paraBorders.size() == 1 && paraBorders[0] == 0)
        {
            // Empty paragraph. Push an empty section.
            aAttrs.emplace_back(nPara1, 0, 0);
            ++nPara1;
            continue;
        }

        auto itBorder = paraBorders.begin(), itBorderEnd = paraBorders.end();
        size_t nPrev = *itBorder;
        size_t nCur;
        for (++itBorder; itBorder != itBorderEnd; ++itBorder, nPrev = nCur)
        {
            nCur = *itBorder;
            aAttrs.emplace_back(nPara1, nPrev, nCur);
        }
        ++nPara1;
    }

    if (aAttrs.empty())
        return;

    // Go through all formatted paragraphs, and store format items.
    std::vector<editeng::Section>::iterator itAttr = aAttrs.begin();
    for (sal_Int32 nPara = 0; nPara < static_cast<sal_Int32>(aContents.size()); ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara];

        itAttr = std::find_if(itAttr, aAttrs.end(), FindByParagraph(nPara));
        if (itAttr == aAttrs.end())
        {
            // This should never happen. There is a logic error somewhere...
            assert(false);
            return;
        }

        for (const auto & aAttrib : rC.maCharAttribs)
        {
            const XEditAttribute& rXAttr = *aAttrib;
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
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= aContents.size())
        return;

    const ContentInfo& rC = *aContents[nPara];
    rName = rC.GetStyle();
    rFamily = rC.GetFamily();
}

void EditTextObjectImpl::SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& rFamily)
{
    if (nPara < 0 || o3tl::make_unsigned(nPara) >= aContents.size())
        return;

    ContentInfo& rC = *aContents[nPara];
    rC.SetStyle(rName);
    rC.SetFamily(rFamily);
}

bool EditTextObjectImpl::ImpChangeStyleSheets(
                    std::u16string_view rOldName, SfxStyleFamily eOldFamily,
                    const OUString& rNewName, SfxStyleFamily eNewFamily )
{
    const size_t nParagraphs = aContents.size();
    bool bChanges = false;

    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        ContentInfo& rC = *aContents[nPara];
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

bool EditTextObjectImpl::operator==( const EditTextObjectImpl& rCompare ) const
{
    return Equals( rCompare, true);
}

bool EditTextObjectImpl::Equals( const EditTextObjectImpl& rCompare, bool bComparePool ) const
{
    if( this == &rCompare )
        return true;

    if( ( aContents.size() != rCompare.aContents.size() ) ||
            ( bComparePool && pPool != rCompare.pPool ) ||
            ( nMetric != rCompare.nMetric ) ||
            ( nUserType!= rCompare.nUserType ) ||
            ( nScriptType != rCompare.nScriptType ) ||
            ( bVertical != rCompare.bVertical ) ||
            ( mnRotation != rCompare.mnRotation ) )
        return false;

    for (size_t i = 0, n = aContents.size(); i < n; ++i)
    {
        if (!(aContents[i]->Equals( *(rCompare.aContents[i]), bComparePool)))
            return false;
    }

    return true;
}

// #i102062#
bool EditTextObjectImpl::isWrongListEqual(const EditTextObjectImpl& rCompare) const
{
    if (aContents.size() != rCompare.aContents.size())
    {
        return false;
    }

    for (size_t i = 0, n = aContents.size(); i < n; ++i)
    {
        const ContentInfo& rCandA = *aContents[i];
        const ContentInfo& rCandB = *rCompare.aContents[i];

        if(!rCandA.isWrongListEqual(rCandB))
        {
            return false;
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
