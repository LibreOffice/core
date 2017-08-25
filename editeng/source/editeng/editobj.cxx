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
#include <comphelper/string.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <tools/stream.hxx>

#include "editeng/fieldupdater.hxx"
#include "editeng/macros.hxx"
#include "editeng/section.hxx"
#include <editobj2.hxx>
#include <editeng/editdata.hxx>
#include <editattr.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brushitem.hxx>

#include <vcl/graph.hxx>
#include <svl/intitem.hxx>
#include "svl/sharedstringpool.hxx"
#include <unotools/fontcvt.hxx>
#include <tools/tenccvt.hxx>

#include <libxml/xmlwriter.h>
#include <algorithm>
#include <cassert>

#if DEBUG_EDIT_ENGINE
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace com::sun::star;


XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    // Create thw new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE )
{
    pItem = &rAttr;
    nStart = nS;
    nEnd = nE;
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
    OutputDevice* pRefDev, sal_uLong nPW, sal_uInt16 _nStretchX, sal_uInt16 _nStretchY) :
    aRefMapMode(pRefDev->GetMapMode()), nStretchX(_nStretchX), nStretchY(_nStretchY)
{
    nRefDevPtr = reinterpret_cast<sal_uIntPtr>(pRefDev);
    nPaperWidth = nPW;
    eRefDevType = pRefDev->GetOutDevType();
}

void XParaPortionList::push_back(XParaPortion* p)
{
    maList.push_back(std::unique_ptr<XParaPortion>(p));
}

const XParaPortion& XParaPortionList::operator [](size_t i) const
{
    return *maList[i].get();
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
        const XEditAttribute& rAttr = *aAttrib.get();
        XEditAttribute* pMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        maCharAttribs.push_back(std::unique_ptr<XEditAttribute>(pMyAttr));
    }

    if ( rCopyFrom.GetWrongList() )
        mpWrongs.reset(rCopyFrom.GetWrongList()->Clone());
}

ContentInfo::~ContentInfo()
{
    XEditAttributesType::iterator it = maCharAttribs.begin(), itEnd = maCharAttribs.end();
    for (; it != itEnd; ++it)
        aParaAttribs.GetPool()->Remove(*(*it)->GetItem());
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("ContentInfo"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("style"), BAD_CAST(aStyle.toUtf8().getStr()));
    xmlTextWriterStartElement(pWriter, BAD_CAST("text"));
    xmlTextWriterWriteString(pWriter, BAD_CAST(GetText().toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
    aParaAttribs.dumpAsXml(pWriter);
    for (size_t i=0; i<maCharAttribs.size(); ++i)
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("attribs"));
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("start"), "%" SAL_PRIdINT32, maCharAttribs[i]->GetStart());
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("end"), "%" SAL_PRIdINT32, maCharAttribs[i]->GetEnd());
        maCharAttribs[i]->GetItem()->dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
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

    XEditAttributesType::const_iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
    {
        const XEditAttribute& rAttr = *it;
        cout << "attribute: " << endl;
        cout << "  span: [begin=" << rAttr.GetStart() << ", end=" << rAttr.GetEnd() << "]" << endl;
        cout << "  feature: " << (rAttr.IsFeature() ? "yes":"no") << endl;
    }
}
#endif

bool ContentInfo::Equals( const ContentInfo& rCompare, bool bComparePool ) const
{
    if( (maText == rCompare.maText) &&
            (aStyle == rCompare.aStyle ) &&
            (maCharAttribs.size() == rCompare.maCharAttribs.size()) &&
            (eFamily == rCompare.eFamily ) &&
            aParaAttribs.Equals( rCompare.aParaAttribs, bComparePool ) )
    {
        for (size_t i = 0, n = maCharAttribs.size(); i < n; ++i)
        {
            if (!(*(maCharAttribs[i]) == *(rCompare.maCharAttribs[i])))
                return false;
        }

        return true;
    }

    return false;
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
    const OUString& rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    return mpImpl->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void EditTextObject::ChangeStyleSheetName(
    SfxStyleFamily eFamily, const OUString& rOldName, const OUString& rNewName)
{
    mpImpl->ChangeStyleSheetName(eFamily, rOldName, rNewName);
}

editeng::FieldUpdater EditTextObject::GetFieldUpdater()
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

bool EditTextObject::IsTopToBottom() const
{
    return mpImpl->IsTopToBottom();
}

void EditTextObject::SetVertical( bool bVertical, bool bTopToBottom )
{
    return mpImpl->SetVertical(bVertical, bTopToBottom);
}

SvtScriptType EditTextObject::GetScriptType() const
{
    return mpImpl->GetScriptType();
}


EditTextObject* EditTextObject::Clone() const
{
    return new EditTextObject(*this);
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
        xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    xmlTextWriterStartElement(pWriter, BAD_CAST("EditTextObject"));
    sal_Int32 nCount = GetParagraphCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        mpImpl->aContents[i]->dumpAsXml(pWriter);
    }
    xmlTextWriterEndElement(pWriter);

    if (bOwns)
    {
       xmlTextWriterEndDocument(pWriter);
       xmlFreeTextWriter(pWriter);
    }
}

// from SfxItemPoolUser
void EditTextObjectImpl::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
{
    if(!bOwnerOfPool && pPool == &rSfxItemPool)
    {
        // The pool we are based on gets destructed; get owner of pool by creating own one.
        // No need to call RemoveSfxItemPoolUser(), this is done from the pool's destructor
        // Base new pool on EditEnginePool; it would also be possible to clone the used
        // pool if needed, but only text attributes should be used.
        SfxItemPool* pNewPool = EditEngine::CreatePool();

        pNewPool->SetDefaultMetric(pPool->GetMetric(DEF_METRIC));

        ContentInfosType aReplaced;
        aReplaced.reserve(aContents.size());
        ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
        for (; it != itEnd; ++it)
            aReplaced.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*it->get(), *pNewPool)));
        aReplaced.swap(aContents);

        // set local variables
        pPool = pNewPool;
        bOwnerOfPool = true;
    }
}

#if DEBUG_EDIT_ENGINE
void EditTextObjectImpl::Dump() const
{
    ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
        it->Dump();
}
#endif

EditEngineItemPool* getEditEngineItemPool(SfxItemPool* pPool)
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

EditTextObjectImpl::EditTextObjectImpl( EditTextObject* pFront, SfxItemPool* pP ) :
    mpFront(pFront)
{
    nVersion = 0;
    nMetric = 0xFFFF;
    nUserType = OutlinerMode::DontKnow;
    nObjSettings = 0;
    pPortionInfo = nullptr;

    // #i101239# ensure target is a EditEngineItemPool, else
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

    bVertical = false;
    bIsTopToBottomVert = false;
    nScriptType = SvtScriptType::NONE;
}

EditTextObjectImpl::EditTextObjectImpl( EditTextObject* pFront, const EditTextObjectImpl& r ) :
    mpFront(pFront)
{
    nVersion = r.nVersion;
    nMetric = r.nMetric;
    nUserType = r.nUserType;
    nObjSettings = r.nObjSettings;
    bVertical = r.bVertical;
    bIsTopToBottomVert = r.bIsTopToBottomVert;
    nScriptType = r.nScriptType;
    pPortionInfo = nullptr;    // Do not copy PortionInfo

    if ( !r.bOwnerOfPool )
    {
        // reuse alien pool; this must be a EditEngineItemPool
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
    ContentInfosType::const_iterator it = r.aContents.begin(), itEnd = r.aContents.end();
    for (; it != itEnd; ++it)
        aContents.push_back(std::unique_ptr<ContentInfo>(new ContentInfo(*it->get(), *pPool)));
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
    ContentInfosType::iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
    {
        ContentInfo& rInfo = *it->get();
        rInfo.NormalizeString(rPool);
    }
}

std::vector<svl::SharedString> EditTextObjectImpl::GetSharedStrings() const
{
    std::vector<svl::SharedString> aSSs;
    aSSs.reserve(aContents.size());
    ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
    {
        const ContentInfo& rInfo = *it->get();
        aSSs.push_back(rInfo.GetSharedString());
    }
    return aSSs;
}

bool EditTextObjectImpl::IsVertical() const
{
    return bVertical;
}

bool EditTextObjectImpl::IsTopToBottom() const
{
    return bIsTopToBottomVert;
}

void EditTextObjectImpl::SetVertical( bool bVert, bool bTopToBottom)
{
    if (bVert != bVertical || bTopToBottom != (bVert && bIsTopToBottomVert))
    {
        bVertical = bVert;
        bIsTopToBottomVert = bVert && bTopToBottom;
        ClearPortionInfo();
    }
}


void EditTextObjectImpl::SetScriptType( SvtScriptType nType )
{
    nScriptType = nType;
}

XEditAttribute* EditTextObjectImpl::CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void EditTextObjectImpl::DestroyAttrib( XEditAttribute* pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
    delete pAttr;
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
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return OUString();

    return aContents[nPara]->GetText();
}

void EditTextObjectImpl::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        delete pPortionInfo;
        pPortionInfo = nullptr;
    }
}

bool EditTextObjectImpl::HasOnlineSpellErrors() const
{
    ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
    {
        if ( (*it)->GetWrongList() && !(*it)->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditTextObjectImpl::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    rLst.clear();
    const ContentInfo& rC = *aContents[nPara].get();
    for (const auto & aAttrib : rC.maCharAttribs)
    {
        const XEditAttribute& rAttr = *aAttrib.get();
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
        const ContentInfo& rC = *aContents[0].get();
        if (rC.GetText().getLength() == 1)
        {
            size_t nAttribs = rC.maCharAttribs.size();
            for (size_t nAttr = nAttribs; nAttr; )
            {
                const XEditAttribute& rX = *rC.maCharAttribs[--nAttr].get();
                if (rX.GetItem()->Which() == EE_FEATURE_FIELD)
                    return static_cast<const SvxFieldItem*>(rX.GetItem());
            }
        }
    }
    return nullptr;
}

const SvxFieldData* EditTextObjectImpl::GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return nullptr;

    const ContentInfo& rC = *aContents[nPara].get();
    if (nPos >= rC.maCharAttribs.size())
        // URL position is out-of-bound.
        return nullptr;

    ContentInfo::XEditAttributesType::const_iterator it = rC.maCharAttribs.begin(), itEnd = rC.maCharAttribs.end();
    size_t nCurPos = 0;
    for (; it != itEnd; ++it)
    {
        const XEditAttribute& rAttr = *it->get();
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
        const ContentInfo& rC = *aContents[nPara].get();
        size_t nAttrs = rC.maCharAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = *rC.maCharAttribs[nAttr].get();
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
    const ContentInfo& rC = *aContents[nPara].get();
    return rC.GetParaAttribs();
}

bool EditTextObjectImpl::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    bool bChanged = false;

    for ( size_t nPara = aContents.size(); nPara; )
    {
        ContentInfo& rC = *aContents[--nPara].get();

        for (size_t nAttr = rC.maCharAttribs.size(); nAttr; )
        {
            XEditAttribute& rAttr = *rC.maCharAttribs[--nAttr].get();
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
    typedef std::vector<size_t> SectionBordersType;
    typedef std::vector<SectionBordersType> ParagraphsType;
    ParagraphsType aParaBorders(aContents.size());

    // First pass: determine section borders for each paragraph.
    for (size_t nPara = 0; nPara < aContents.size(); ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara].get();
        SectionBordersType& rBorders = aParaBorders[nPara];
        rBorders.push_back(0);
        rBorders.push_back(rC.GetText().getLength());
        for (const auto & aAttrib : rC.maCharAttribs)
        {
            const XEditAttribute& rAttr = *aAttrib.get();
            const SfxPoolItem* pItem = rAttr.GetItem();
            if (!pItem)
                continue;

            rBorders.push_back(rAttr.GetStart());
            rBorders.push_back(rAttr.GetEnd());
        }
    }

    // Sort and remove duplicates for each paragraph.
    ParagraphsType::iterator it = aParaBorders.begin(), itEnd = aParaBorders.end();
    for (; it != itEnd; ++it)
    {
        SectionBordersType& rBorders = *it;
        std::sort(rBorders.begin(), rBorders.end());
        SectionBordersType::iterator itUniqueEnd = std::unique(rBorders.begin(), rBorders.end());
        rBorders.erase(itUniqueEnd, rBorders.end());
    }

    std::vector<editeng::Section> aAttrs;

    // Create storage for each section.  Note that this creates storage even
    // for unformatted sections.  The entries are sorted first by paragraph,
    // then by section positions.  They don't overlap with each other.
    it = aParaBorders.begin();
    for (; it != itEnd; ++it)
    {
        size_t nPara = distance(aParaBorders.begin(), it);
        const SectionBordersType& rBorders = *it;
        if (rBorders.size() == 1 && rBorders[0] == 0)
        {
            // Empty paragraph. Push an empty section.
            aAttrs.push_back(editeng::Section(nPara, 0, 0));
            continue;
        }

        SectionBordersType::const_iterator itBorder = rBorders.begin(), itBorderEnd = rBorders.end();
        size_t nPrev = *itBorder;
        size_t nCur;
        for (++itBorder; itBorder != itBorderEnd; ++itBorder, nPrev = nCur)
        {
            nCur = *itBorder;
            aAttrs.push_back(editeng::Section(nPara, nPrev, nCur));
        }
    }

    if (aAttrs.empty())
        return;

    // Go through all formatted paragraphs, and store format items.
    std::vector<editeng::Section>::iterator itAttr = aAttrs.begin();
    for (sal_Int32 nPara = 0; nPara < (sal_Int32)aContents.size(); ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara].get();

        itAttr = std::find_if(itAttr, aAttrs.end(), FindByParagraph(nPara));
        if (itAttr == aAttrs.end())
        {
            // This should never happen. There is a logic error somewhere...
            assert(false);
            return;
        }

        for (const auto & aAttrib : rC.maCharAttribs)
        {
            const XEditAttribute& rXAttr = *aAttrib.get();
            const SfxPoolItem* pItem = rXAttr.GetItem();
            if (!pItem)
                continue;

            sal_Int32 nStart = rXAttr.GetStart(), nEnd = rXAttr.GetEnd();
            std::vector<editeng::Section>::iterator itCurAttr = itAttr;

            // Find the container whose start position matches.
            itCurAttr = std::find_if(itCurAttr, aAttrs.end(), FindBySectionStart(nPara, nStart));
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
                auto iter(std::find_if(
                    rSecAttr.maAttributes.begin(), rSecAttr.maAttributes.end(),
                    [&pItem](SfxPoolItem const*const pIt)
                        { return pIt->Which() == pItem->Which(); }));
                if (rSecAttr.maAttributes.end() == iter)
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
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    const ContentInfo& rC = *aContents[nPara].get();
    rName = rC.GetStyle();
    rFamily = rC.GetFamily();
}

void EditTextObjectImpl::SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& rFamily)
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    ContentInfo& rC = *aContents[nPara].get();
    rC.SetStyle(rName);
    rC.SetFamily(rFamily);
}

bool EditTextObjectImpl::ImpChangeStyleSheets(
                    const OUString& rOldName, SfxStyleFamily eOldFamily,
                    const OUString& rNewName, SfxStyleFamily eNewFamily )
{
    const size_t nParagraphs = aContents.size();
    bool bChanges = false;

    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        ContentInfo& rC = *aContents[nPara].get();
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
                    const OUString& rOldName, SfxStyleFamily eOldFamily,
                    const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    bool bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void EditTextObjectImpl::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const OUString& rOldName, const OUString& rNewName )
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
            ( bIsTopToBottomVert != rCompare.bIsTopToBottomVert ) )
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
        const ContentInfo& rCandA = *aContents[i].get();
        const ContentInfo& rCandB = *rCompare.aContents[i].get();

        if(!rCandA.isWrongListEqual(rCandB))
        {
            return false;
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
