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

#include <vcl/graphic.hxx>
#include <svl/intitem.hxx>
#include "svl/sharedstringpool.hxx"
#include <unotools/fontcvt.hxx>
#include <tools/tenccvt.hxx>

#include <libxml/xmlwriter.h>
#include <algorithm>

#if DEBUG_EDIT_ENGINE
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace com::sun::star;


XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    // Create thw new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE )
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
    eFamily(SFX_STYLE_FAMILY_PARA),
    aParaAttribs(rPool, EE_PARA_START, EE_CHAR_END)
{
}

// the real Copy constructor is nonsense, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse ) :
    maText(rCopyFrom.maText),
    aStyle(rCopyFrom.aStyle),
    eFamily(rCopyFrom.eFamily),
    aParaAttribs(rPoolToUse, EE_PARA_START, EE_CHAR_END)
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );

    for (size_t i = 0; i < rCopyFrom.aAttribs.size(); ++i)
    {
        const XEditAttribute& rAttr = *rCopyFrom.aAttribs[i].get();
        XEditAttribute* pMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        aAttribs.push_back(std::unique_ptr<XEditAttribute>(pMyAttr));
    }

    if ( rCopyFrom.GetWrongList() )
        mpWrongs.reset(rCopyFrom.GetWrongList()->Clone());
}

ContentInfo::~ContentInfo()
{
    XEditAttributesType::iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
        aParaAttribs.GetPool()->Remove(*(*it)->GetItem());
    aAttribs.clear();
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

bool ContentInfo::operator==( const ContentInfo& rCompare ) const
{
    if( (maText == rCompare.maText) &&
            (aStyle == rCompare.aStyle ) &&
            (aAttribs.size() == rCompare.aAttribs.size()) &&
            (eFamily == rCompare.eFamily ) &&
            (aParaAttribs == rCompare.aParaAttribs ) )
    {
        for (size_t i = 0, n = aAttribs.size(); i < n; ++i)
        {
            if (aAttribs[i] != rCompare.aAttribs[i])
                return false;
        }

        return true;
    }

    return false;
}

bool ContentInfo::operator!=(const ContentInfo& rCompare) const
{
    return !operator==(rCompare);
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

sal_uInt16 EditTextObject::GetUserType() const
{
    return mpImpl->GetUserType();
}

void EditTextObject::SetUserType( sal_uInt16 n )
{
    mpImpl->SetUserType(n);
}

bool EditTextObject::IsVertical() const
{
    return mpImpl->IsVertical();
}

void EditTextObject::SetVertical( bool bVertical )
{
    return mpImpl->SetVertical(bVertical);
}

SvtScriptType EditTextObject::GetScriptType() const
{
    return mpImpl->GetScriptType();
}


void EditTextObject::Store( SvStream& rOStream ) const
{
    if ( rOStream.GetError() )
        return;

    sal_Size nStartPos = rOStream.Tell();

    sal_uInt16 nWhich = static_cast<sal_uInt16>(EE_FORMAT_BIN);
    rOStream.WriteUInt16( nWhich );

    sal_uInt32 nStructSz = 0;
    rOStream.WriteUInt32( nStructSz );

    StoreData( rOStream );

    sal_Size nEndPos = rOStream.Tell();
    nStructSz = nEndPos - nStartPos - sizeof( nWhich ) - sizeof( nStructSz );
    rOStream.Seek( nStartPos + sizeof( nWhich ) );
    rOStream.WriteUInt32( nStructSz );
    rOStream.Seek( nEndPos );
}

EditTextObject* EditTextObject::Create( SvStream& rIStream )
{
    sal_Size nStartPos = rIStream.Tell();

    // First check what type of Object...
    sal_uInt16 nWhich;
    rIStream.ReadUInt16( nWhich );

    sal_uInt32 nStructSz;
    rIStream.ReadUInt32( nStructSz );

    if (nWhich != EE_FORMAT_BIN)
    {
        // Unknown object we no longer support.
        rIStream.SetError(EE_READWRITE_WRONGFORMAT);
        return nullptr;
    }

    if ( rIStream.GetError() )
        return nullptr;

    EditTextObject* pTxtObj = new EditTextObject(nullptr);
    pTxtObj->CreateData(rIStream);

    // Make sure that the stream is left at the correct place.
    sal_Size nFullSz = sizeof( nWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
    return pTxtObj;
}

void EditTextObject::StoreData( SvStream& rStrm ) const
{
    mpImpl->StoreData(rStrm);
}

void EditTextObject::CreateData( SvStream& rStrm )
{
    mpImpl->CreateData(rStrm);
}

EditTextObject* EditTextObject::Clone() const
{
    return new EditTextObject(*this);
}

bool EditTextObject::operator==( const EditTextObject& rCompare ) const
{
    return mpImpl->operator==(*rCompare.mpImpl);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("editTextObject"));
    sal_Int32 nCount = GetParagraphCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("paragraph"));
        xmlTextWriterWriteString(pWriter, BAD_CAST(GetText(i).toUtf8().getStr()));
        xmlTextWriterEndElement(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
}

// from SfxItemPoolUser
void EditTextObjectImpl::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
{
    if(!bOwnerOfPool && pPool && pPool == &rSfxItemPool)
    {
        // The pool we are based on gets destructed; get owner of pool by creating own one.
        // No need to call RemoveSfxItemPoolUser(), this is done from the pool's destructor
        // Base new pool on EditEnginePool; it would also be possible to clone the used
        // pool if needed, but only text attributes should be used.
        SfxItemPool* pNewPool = EditEngine::CreatePool();

        if(pPool)
        {
            pNewPool->SetDefaultMetric(pPool->GetMetric(DEF_METRIC));
        }

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
    nUserType = 0;
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
    bStoreUnicodeStrings = false;
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
    nScriptType = r.nScriptType;
    pPortionInfo = nullptr;    // Do not copy PortionInfo
    bStoreUnicodeStrings = false;

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


void EditTextObjectImpl::SetUserType( sal_uInt16 n )
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


void EditTextObjectImpl::SetVertical( bool b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}


void EditTextObjectImpl::SetScriptType( SvtScriptType nType )
{
    nScriptType = nType;
}

XEditAttribute* EditTextObjectImpl::CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
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
    for (size_t nAttr = 0; nAttr < rC.aAttribs.size(); ++nAttr)
    {
        const XEditAttribute& rAttr = *rC.aAttribs[nAttr].get();
        EECharAttrib aEEAttr;
        aEEAttr.pAttr = rAttr.GetItem();
        aEEAttr.nPara = nPara;
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
            size_t nAttribs = rC.aAttribs.size();
            for (size_t nAttr = nAttribs; nAttr; )
            {
                const XEditAttribute& rX = *rC.aAttribs[--nAttr].get();
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
    if (nPos >= rC.aAttribs.size())
        // URL position is out-of-bound.
        return nullptr;

    ContentInfo::XEditAttributesType::const_iterator it = rC.aAttribs.begin(), itEnd = rC.aAttribs.end();
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
        size_t nAttrs = rC.aAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = *rC.aAttribs[nAttr].get();
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

        for (size_t nAttr = rC.aAttribs.size(); nAttr; )
        {
            XEditAttribute& rAttr = *rC.aAttribs[--nAttr].get();
            if ( !_nWhich || (rAttr.GetItem()->Which() == _nWhich) )
            {
                pPool->Remove(*rAttr.GetItem());
                rC.aAttribs.erase(rC.aAttribs.begin()+nAttr);
                bChanged = true;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

namespace {

class FindByParagraph : public std::unary_function<editeng::Section, bool>
{
    sal_Int32 mnPara;
public:
    explicit FindByParagraph(sal_Int32 nPara) : mnPara(nPara) {}
    bool operator() (const editeng::Section& rAttr) const
    {
        return rAttr.mnParagraph == mnPara;
    }
};

class FindBySectionStart : public std::unary_function<editeng::Section, bool>
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
        for (size_t nAttr = 0; nAttr < rC.aAttribs.size(); ++nAttr)
        {
            const XEditAttribute& rAttr = *rC.aAttribs[nAttr].get();
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

        for (size_t i = 0; i < rC.aAttribs.size(); ++i)
        {
            const XEditAttribute& rXAttr = *rC.aAttribs[i].get();
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
    rC.GetStyle() = rName;
    rC.GetFamily() = rFamily;
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
                rC.GetStyle() = rNewName;
                rC.GetFamily() = eNewFamily;
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


namespace {

class FindAttribByChar : public std::unary_function<std::unique_ptr<XEditAttribute>, bool>
{
    sal_uInt16 mnWhich;
    sal_uInt16 mnChar;
public:
    FindAttribByChar(sal_uInt16 nWhich, sal_uInt16 nChar) : mnWhich(nWhich), mnChar(nChar) {}
    bool operator() (const std::unique_ptr<XEditAttribute>& rAttr) const
    {
        return (rAttr->GetItem()->Which() == mnWhich) && (rAttr->GetStart() <= mnChar) && (rAttr->GetEnd() > mnChar);
    }
};

}

void EditTextObjectImpl::StoreData( SvStream& rOStream ) const
{
    sal_uInt16 nVer = 602;
    rOStream.WriteUInt16( nVer );

    rOStream.WriteBool( bOwnerOfPool );

    // First store the pool, later only the Surregate
    if ( bOwnerOfPool )
    {
        GetPool()->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
        GetPool()->Store( rOStream );
    }

    // Store Current text encoding ...
    rtl_TextEncoding eEncoding = GetSOStoreTextEncoding( osl_getThreadTextEncoding() );
    rOStream.WriteUInt16( eEncoding );

    // The number of paragraphs ...
    size_t nParagraphs = aContents.size();
    // FIXME: this truncates, check usage of stream and if it can be changed,
    // i.e. is not persistent, adapt this and reader.
    sal_uInt16 nParagraphs_Stream = static_cast<sal_uInt16>(nParagraphs);
    rOStream.WriteUInt16( nParagraphs_Stream );

    sal_Unicode nUniChar = CH_FEATURE;
    char cFeatureConverted = OString(&nUniChar, 1, eEncoding).toChar();

    // The individual paragraphs ...
    for (size_t nPara = 0; nPara < nParagraphs_Stream; ++nPara)
    {
        const ContentInfo& rC = *aContents[nPara].get();

        // Text...
        OStringBuffer aBuffer(OUStringToOString(rC.GetText(), eEncoding));

        // Symbols?
        bool bSymbolPara = false;
        if (rC.GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SfxItemState::SET)
        {
            const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(rC.GetParaAttribs().Get(EE_CHAR_FONTINFO));
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                aBuffer = OStringBuffer(OUStringToOString(rC.GetText(), RTL_TEXTENCODING_SYMBOL));
                bSymbolPara = true;
            }
        }
        for (size_t nA = 0; nA < rC.aAttribs.size(); ++nA)
        {
            const XEditAttribute& rAttr = *rC.aAttribs[nA].get();

            if (rAttr.GetItem()->Which() == EE_CHAR_FONTINFO)
            {
                const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(*rAttr.GetItem());
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    OUString aPart = rC.GetText().copy( rAttr.GetStart(), rAttr.GetEnd() - rAttr.GetStart() );
                    OString aNew(OUStringToOString(aPart, rFontItem.GetCharSet()));
                    aBuffer.remove(rAttr.GetStart(), rAttr.GetEnd() - rAttr.GetStart());
                    aBuffer.insert(rAttr.GetStart(), aNew);
                }

                // Convert StarSymbol back to StarBats
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FontToSubsFontFlags::EXPORT | FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    // Don't create a new Attrib with StarBats font, MBR changed the
                    // SvxFontItem::Store() to store StarBats instead of StarSymbol!
                    for (sal_uInt16 nChar = rAttr.GetStart(); nChar < rAttr.GetEnd(); ++nChar)
                    {
                        sal_Unicode cOld = rC.GetText()[ nChar ];
                        char cConv = OUStringToOString(OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                        if ( cConv )
                            aBuffer[nChar] = cConv;
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }

        // Convert StarSymbol back to StarBats
        // StarSymbol as paragraph attribute or in StyleSheet?

        FontToSubsFontConverter hConv = nullptr;
        if (rC.GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SfxItemState::SET)
        {
            hConv = CreateFontToSubsFontConverter( static_cast<const SvxFontItem&>(rC.GetParaAttribs().Get( EE_CHAR_FONTINFO )).GetFamilyName(), FontToSubsFontFlags::EXPORT | FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
        }
        if ( hConv )
        {
            for ( sal_uInt16 nChar = 0; nChar < rC.GetText().getLength(); nChar++ )
            {
                const ContentInfo::XEditAttributesType& rAttribs = rC.aAttribs;
                if ( std::none_of(rAttribs.begin(), rAttribs.end(),
                                  FindAttribByChar(EE_CHAR_FONTINFO, nChar)) )
                {
                    sal_Unicode cOld = rC.GetText()[ nChar ];
                    char cConv = OUStringToOString(OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                    if ( cConv )
                        aBuffer[nChar] = cConv;
                }
            }

            DestroyFontToSubsFontConverter( hConv );

        }


        // Convert CH_FEATURE to CH_FEATURE_OLD
        OString aText = aBuffer.makeStringAndClear().replace(cFeatureConverted, CH_FEATURE_OLD);
        write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, aText);

        // StyleName and Family...
        write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, rC.GetStyle(), eEncoding);
        rOStream.WriteUInt16( rC.GetFamily() );

        // Paragraph attributes ...
        rC.GetParaAttribs().Store( rOStream );

        // The number of attributes ...
        size_t nAttribs = rC.aAttribs.size();
        rOStream.WriteUInt16( nAttribs );

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attribute
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for (size_t nAttr = 0; nAttr < nAttribs; ++nAttr)
        {
            const XEditAttribute& rX = *rC.aAttribs[nAttr].get();

            rOStream.WriteUInt16( rX.GetItem()->Which() );
            GetPool()->StoreSurrogate(rOStream, rX.GetItem());
            rOStream.WriteUInt16( rX.GetStart() );
            rOStream.WriteUInt16( rX.GetEnd() );
        }
    }

    rOStream.WriteUInt16( nMetric );

    rOStream.WriteUInt16( nUserType );
    rOStream.WriteUInt32( nObjSettings );

    rOStream.WriteBool( bVertical );
    rOStream.WriteUInt16( static_cast<sal_uInt16>(nScriptType) );

    rOStream.WriteBool( bStoreUnicodeStrings );
    if ( bStoreUnicodeStrings )
    {
        for ( size_t nPara = 0; nPara < nParagraphs_Stream; nPara++ )
        {
            const ContentInfo& rC = *aContents[nPara].get();
            sal_uInt16 nL = rC.GetText().getLength();
            rOStream.WriteUInt16( nL );
            rOStream.Write(rC.GetText().getStr(), nL*sizeof(sal_Unicode));

            // StyleSheetName must be Unicode too!
            // Copy/Paste from EA3 to BETA or from BETA to EA3 not possible, not needed...
            // If needed, change nL back to sal_uLong and increase version...
            nL = rC.GetStyle().getLength();
            rOStream.WriteUInt16( nL );
            rOStream.Write(rC.GetStyle().getStr(), nL*sizeof(sal_Unicode));
        }
    }
}

void EditTextObjectImpl::CreateData( SvStream& rIStream )
{
    rIStream.ReadUInt16( nVersion );

    // The text object was first created with the current setting of
    // pTextObjectPool.
    bool bOwnerOfCurrent = bOwnerOfPool;
    bool b;
    rIStream.ReadCharAsBool( b );
    bOwnerOfPool = b;

    if ( bOwnerOfCurrent && !bOwnerOfPool )
    {
        // A global Pool was used, but not handed over to me, but I need it!
        OSL_FAIL( "Give me the global TextObjectPool!" );
        return;
    }
    else if ( !bOwnerOfCurrent && bOwnerOfPool )
    {
        // A global Pool should be used, but this Textobject has its own.
        pPool = EditEngine::CreatePool();
    }

    if ( bOwnerOfPool )
        GetPool()->Load( rIStream );

    // CharSet, in which it was saved:
    sal_uInt16 nCharSet;
    rIStream.ReadUInt16( nCharSet );

    rtl_TextEncoding eSrcEncoding = GetSOLoadTextEncoding( (rtl_TextEncoding)nCharSet );

    // The number of paragraphs ...
    sal_uInt16 nParagraphs(0);
    rIStream.ReadUInt16( nParagraphs );

    const size_t nMinParaRecordSize = 6 + eSrcEncoding == RTL_TEXTENCODING_UNICODE ? 4 : 2;
    const size_t nMaxParaRecords = rIStream.remainingSize() / nMinParaRecordSize;
    if (nParagraphs > nMaxParaRecords)
    {
        SAL_WARN("editeng", "Parsing error: " << nMaxParaRecords <<
                 " max possible entries, but " << nParagraphs<< " claimed, truncating");
        nParagraphs = nMaxParaRecords;
    }

    // The individual paragraphs ...
    for ( sal_uLong nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // The Text...
        OString aByteString = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStream);
        pC->SetText(OStringToOUString(aByteString, eSrcEncoding));

        // StyleName and Family...
        pC->GetStyle() = rIStream.ReadUniOrByteString(eSrcEncoding);
        sal_uInt16 nStyleFamily(0);
        rIStream.ReadUInt16( nStyleFamily );
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Paragraph attributes ...
        pC->GetParaAttribs().Load( rIStream );

        // The number of attributes ...
        sal_uInt16 nTmp16(0);
        rIStream.ReadUInt16( nTmp16 );
        size_t nAttribs = nTmp16;

        const size_t nMinRecordSize(10);
        const size_t nMaxRecords = rIStream.remainingSize() / nMinRecordSize;
        if (nAttribs > nMaxRecords)
        {
            SAL_WARN("editeng", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nAttribs << " claimed, truncating");
            nAttribs = nMaxRecords;
        }

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attributes
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for (size_t nAttr = 0; nAttr < nAttribs; ++nAttr)
        {
            sal_uInt16 _nWhich(0), nStart(0), nEnd(0);
            const SfxPoolItem* pItem;

            rIStream.ReadUInt16( _nWhich );
            _nWhich = pPool->GetNewWhich( _nWhich );
            pItem = pPool->LoadSurrogate( rIStream, _nWhich, 0 );
            rIStream.ReadUInt16( nStart );
            rIStream.ReadUInt16( nEnd );
            if ( pItem )
            {
                if ( pItem->Which() == EE_FEATURE_NOTCONV )
                {
                    sal_Char cEncodedChar = aByteString[nStart];
                    sal_Unicode cChar = OUString(&cEncodedChar, 1,
                        static_cast<const SvxCharSetColorItem*>(pItem)->GetCharSet()).toChar();
                    pC->SetText(pC->GetText().replaceAt(nStart, 1, OUString(cChar)));
                }
                else
                {
                    XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                    pC->aAttribs.push_back(std::unique_ptr<XEditAttribute>(pAttr));

                    if ( ( _nWhich >= EE_FEATURE_START ) && ( _nWhich <= EE_FEATURE_END ) )
                    {
                        // Convert CH_FEATURE to CH_FEATURE_OLD
                        DBG_ASSERT( (sal_uInt8) aByteString[nStart] == CH_FEATURE_OLD, "CreateData: CH_FEATURE expected!" );
                        if ( (sal_uInt8) aByteString[nStart] == CH_FEATURE_OLD )
                            pC->SetText(pC->GetText().replaceAt(nStart, 1, OUString(CH_FEATURE)));
                    }
                }
            }
        }

        // But check for paragraph and character symbol attribs here,
        // FinishLoad will not be called in OpenOffice Calc, no StyleSheets...

        bool bSymbolPara = false;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SfxItemState::SET )
        {
            const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(pC->GetParaAttribs().Get( EE_CHAR_FONTINFO ));
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                pC->SetText(OStringToOUString(aByteString, RTL_TEXTENCODING_SYMBOL));
                bSymbolPara = true;
            }
        }

        for (size_t nAttr = pC->aAttribs.size(); nAttr; )
        {
            const XEditAttribute& rAttr = *pC->aAttribs[--nAttr].get();
            if ( rAttr.GetItem()->Which() == EE_CHAR_FONTINFO )
            {
                const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(*rAttr.GetItem());
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    OString aPart(aByteString.copy(rAttr.GetStart(), rAttr.GetEnd()-rAttr.GetStart()));
                    OUString aNew(OStringToOUString(aPart, rFontItem.GetCharSet()));
                    pC->SetText(pC->GetText().replaceAt(rAttr.GetStart(), rAttr.GetEnd()-rAttr.GetStart(), aNew));
                }

                // Convert StarMath and StarBats to StarSymbol
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FontToSubsFontFlags::IMPORT | FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    SvxFontItem aNewFontItem( rFontItem );
                    aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );

                    // Replace the existing attribute with a new one.
                    XEditAttribute* pNewAttr = CreateAttrib(aNewFontItem, rAttr.GetStart(), rAttr.GetEnd());

                    pPool->Remove(*rAttr.GetItem());
                    pC->aAttribs.erase(pC->aAttribs.begin()+nAttr);
                    pC->aAttribs.insert(pC->aAttribs.begin()+nAttr, std::unique_ptr<XEditAttribute>(pNewAttr));

                    for ( sal_uInt16 nChar = pNewAttr->GetStart(); nChar < pNewAttr->GetEnd(); nChar++ )
                    {
                        sal_Unicode cOld = pC->GetText()[ nChar ];
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->SetText(pC->GetText().replaceAt(nChar, 1, OUString(cConv)));
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }


        // Convert StarMath and StarBats to StarSymbol
        // Maybe old symbol font as paragraph attribute?
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SfxItemState::SET )
        {
            const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(pC->GetParaAttribs().Get( EE_CHAR_FONTINFO ));
            FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FontToSubsFontFlags::IMPORT | FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
            if ( hConv )
            {
                SvxFontItem aNewFontItem( rFontItem );
                aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );
                pC->GetParaAttribs().Put( aNewFontItem );

                for ( sal_uInt16 nChar = 0; nChar < pC->GetText().getLength(); nChar++ )
                {
                    const ContentInfo::XEditAttributesType& rAttribs = pC->aAttribs;
                    if ( std::none_of(rAttribs.begin(), rAttribs.end(),
                                      FindAttribByChar(EE_CHAR_FONTINFO, nChar)) )
                    {
                        sal_Unicode cOld = pC->GetText()[ nChar ];
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->SetText(pC->GetText().replaceAt(nChar, 1, OUString(cConv)));
                    }
                }

                DestroyFontToSubsFontConverter( hConv );
            }
        }
    }

    // From 400 also the DefMetric:
    if ( nVersion >= 400 )
    {
        sal_uInt16 nTmpMetric;
        rIStream.ReadUInt16( nTmpMetric );
        if ( nVersion >= 401 )
        {
            // In the 400 there was a bug in text objects with the own Pool,
            // therefore evaluate only from 401
            nMetric = nTmpMetric;
            if ( bOwnerOfPool && pPool && ( nMetric != 0xFFFF ) )
                pPool->SetDefaultMetric( (SfxMapUnit)nMetric );
        }
    }

    if ( nVersion >= 600 )
    {
        rIStream.ReadUInt16( nUserType );
        rIStream.ReadUInt32( nObjSettings );
    }

    if ( nVersion >= 601 )
    {
        bool bTmp(false);
        rIStream.ReadCharAsBool( bTmp );
        bVertical = bTmp;
    }

    if ( nVersion >= 602 )
    {
        sal_uInt16 aTmp16;
        rIStream.ReadUInt16( aTmp16 );
        nScriptType = static_cast<SvtScriptType>(aTmp16);

        bool bUnicodeStrings(false);
        rIStream.ReadCharAsBool( bUnicodeStrings );
        if ( bUnicodeStrings )
        {
            for (sal_uInt16 nPara = 0; nPara < nParagraphs; ++nPara)
            {
                ContentInfo& rC = *aContents[nPara].get();
                sal_uInt16 nL(0);

                // Text
                rIStream.ReadUInt16(nL);
                if (nL)
                {
                    size_t nMaxElementsPossible = rIStream.remainingSize() / sizeof(sal_Unicode);
                    if (nL > nMaxElementsPossible)
                    {
                        SAL_WARN("editeng", "Parsing error: " << nMaxElementsPossible <<
                                 " max possible entries, but " << nL << " claimed, truncating");
                        nL = nMaxElementsPossible;
                    }

                    rtl_uString *pStr = rtl_uString_alloc(nL);
                    rIStream.Read(pStr->buffer, nL*sizeof(sal_Unicode));
                    rC.SetText((OUString(pStr, SAL_NO_ACQUIRE)));

                    nL = 0;
                }

                // StyleSheetName
                rIStream.ReadUInt16( nL );
                if ( nL )
                {
                    size_t nMaxElementsPossible = rIStream.remainingSize() / sizeof(sal_Unicode);
                    if (nL > nMaxElementsPossible)
                    {
                        SAL_WARN("editeng", "Parsing error: " << nMaxElementsPossible <<
                                 " max possible entries, but " << nL << " claimed, truncating");
                        nL = nMaxElementsPossible;
                    }

                    rtl_uString *pStr = rtl_uString_alloc(nL);
                    rIStream.Read(pStr->buffer, nL*sizeof(sal_Unicode) );
                    rC.GetStyle() = OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }


    // from 500 the tabs are interpreted differently: TabPos + LI, previously only TabPos.
    // Works only if tab positions are set, not when DefTab.
    if ( nVersion < 500 )
    {
        for (size_t i = 0, n = aContents.size(); i < n; ++i)
        {
            ContentInfo& rC = *aContents[i].get();
            const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(rC.GetParaAttribs().Get(EE_PARA_LRSPACE));
            if ( rLRSpace.GetTextLeft() && ( rC.GetParaAttribs().GetItemState( EE_PARA_TABS ) == SfxItemState::SET ) )
            {
                const SvxTabStopItem& rTabs = static_cast<const SvxTabStopItem&>(rC.GetParaAttribs().Get(EE_PARA_TABS));
                SvxTabStopItem aNewTabs( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
                for ( sal_uInt16 t = 0; t < rTabs.Count(); t++ )
                {
                    const SvxTabStop& rT = rTabs[ t ];
                    aNewTabs.Insert( SvxTabStop( rT.GetTabPos() - rLRSpace.GetTextLeft(),
                                rT.GetAdjustment(), rT.GetDecimal(), rT.GetFill() ) );
                }
                rC.GetParaAttribs().Put( aNewTabs );
            }
        }
    }
}

bool EditTextObjectImpl::operator==( const EditTextObjectImpl& rCompare ) const
{
    if( this == &rCompare )
        return true;

    if( ( aContents.size() != rCompare.aContents.size() ) ||
            ( pPool != rCompare.pPool ) ||
            ( nMetric != rCompare.nMetric ) ||
            ( nUserType!= rCompare.nUserType ) ||
            ( nScriptType != rCompare.nScriptType ) ||
            ( bVertical != rCompare.bVertical ) )
        return false;

    for (size_t i = 0, n = aContents.size(); i < n; ++i)
    {
        if (aContents[i] != rCompare.aContents[i])
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
