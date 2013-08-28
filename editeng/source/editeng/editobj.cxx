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

#include <vcl/graph.hxx>
#include <svl/intitem.hxx>
#include <unotools/fontcvt.hxx>
#include <tools/tenccvt.hxx>

#if DEBUG_EDIT_ENGINE
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace com::sun::star;

DBG_NAME( EE_EditTextObject )
DBG_NAME( XEditAttribute )

//--------------------------------------------------------------

XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    // Create thw new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE )
{
    DBG_CTOR( XEditAttribute, 0 );
    pItem = &rAttr;
    nStart = nS;
    nEnd = nE;
}

XEditAttribute::~XEditAttribute()
{
    DBG_DTOR( XEditAttribute, 0 );
    pItem = 0;  // belongs to the Pool.
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
    nRefDevPtr = (sal_uIntPtr)pRefDev; nPaperWidth = nPW;
    eRefDevType = pRefDev->GetOutDevType();
}

void XParaPortionList::push_back(XParaPortion* p)
{
    maList.push_back(p);
}

const XParaPortion& XParaPortionList::operator [](size_t i) const
{
    return maList[i];
}

ContentInfo::ContentInfo( SfxItemPool& rPool ) :
    eFamily(SFX_STYLE_FAMILY_PARA),
    aParaAttribs(rPool, EE_PARA_START, EE_CHAR_END)
{
}

// the real Copy constructor is nonsens, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse ) :
    aText(rCopyFrom.aText),
    aStyle(rCopyFrom.aStyle),
    eFamily(rCopyFrom.eFamily),
    aParaAttribs(rPoolToUse, EE_PARA_START, EE_CHAR_END)
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );

    for (size_t i = 0; i < rCopyFrom.aAttribs.size(); ++i)
    {
        const XEditAttribute& rAttr = rCopyFrom.aAttribs[i];
        XEditAttribute* pMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        aAttribs.push_back(pMyAttr);
    }

    if ( rCopyFrom.GetWrongList() )
        mpWrongs.reset(rCopyFrom.GetWrongList()->Clone());
}

ContentInfo::~ContentInfo()
{
    XEditAttributesType::iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
        aParaAttribs.GetPool()->Remove(*it->GetItem());
    aAttribs.clear();
}

WrongList* ContentInfo::GetWrongList() const
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
    cout << "text: '" << aText << "'" << endl;
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
    if( (aText == rCompare.aText) &&
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
    delete mpImpl;
}

sal_Int32 EditTextObject::GetParagraphCount() const
{
    return mpImpl->GetParagraphCount();
}

String EditTextObject::GetText(sal_Int32 nPara) const
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

sal_uInt16 EditTextObject::GetScriptType() const
{
    return mpImpl->GetScriptType();
}


bool EditTextObject::Store( SvStream& rOStream ) const
{
    if ( rOStream.GetError() )
        return false;

    sal_Size nStartPos = rOStream.Tell();

    sal_uInt16 nWhich = static_cast<sal_uInt16>(EE_FORMAT_BIN);
    rOStream << nWhich;

    sal_uInt32 nStructSz = 0;
    rOStream << nStructSz;

    StoreData( rOStream );

    sal_Size nEndPos = rOStream.Tell();
    nStructSz = nEndPos - nStartPos - sizeof( nWhich ) - sizeof( nStructSz );
    rOStream.Seek( nStartPos + sizeof( nWhich ) );
    rOStream << nStructSz;
    rOStream.Seek( nEndPos );

    return rOStream.GetError() ? false : true;
}

EditTextObject* EditTextObject::Create( SvStream& rIStream, SfxItemPool* pGlobalTextObjectPool )
{
    sal_uLong nStartPos = rIStream.Tell();

    // First check what type of Object...
    sal_uInt16 nWhich;
    rIStream >> nWhich;

    sal_uInt32 nStructSz;
    rIStream >> nStructSz;

    if (nWhich != EE_FORMAT_BIN)
    {
        // Unknown object we no longer support.
        rIStream.SetError(EE_READWRITE_WRONGFORMAT);
        return NULL;
    }

    if ( rIStream.GetError() )
        return NULL;

    EditTextObject* pTxtObj = new EditTextObject(pGlobalTextObjectPool);;
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
            aReplaced.push_back(new ContentInfo(*it, *pNewPool));
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
    pPortionInfo = 0;

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
    nScriptType = 0;
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
    pPortionInfo = NULL;    // Do not copy PortionInfo
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
        bOwnerOfPool =  true;

    }

    if(!bOwnerOfPool && pPool)
    {
        // it is sure now that the pool is an EditEngineItemPool
        pPool->AddSfxItemPoolUser(*mpFront);
    }

    if ( bOwnerOfPool && pPool && r.pPool )
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    aContents.reserve(r.aContents.size());
    ContentInfosType::const_iterator it = r.aContents.begin(), itEnd = r.aContents.end();
    for (; it != itEnd; ++it)
        aContents.push_back(new ContentInfo(*it, *pPool));
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

sal_uInt16 EditTextObjectImpl::GetUserType() const
{
    return nUserType;
}

void EditTextObjectImpl::SetUserType( sal_uInt16 n )
{
    nUserType = n;
}

bool EditTextObjectImpl::IsVertical() const
{
    return bVertical;
}

void EditTextObjectImpl::SetVertical( bool b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}

sal_uInt16 EditTextObjectImpl::GetScriptType() const
{
    return nScriptType;
}

void EditTextObjectImpl::SetScriptType( sal_uInt16 nType )
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

EditTextObjectImpl::ContentInfosType& EditTextObjectImpl::GetContents()
{
    return aContents;
}

const EditTextObjectImpl::ContentInfosType& EditTextObjectImpl::GetContents() const
{
    return aContents;
}

ContentInfo* EditTextObjectImpl::CreateAndInsertContent()
{
    aContents.push_back(new ContentInfo(*pPool));
    return &aContents.back();
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

String EditTextObjectImpl::GetText(sal_Int32 nPara) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return String();

    return aContents[nPara].GetText();
}

void EditTextObjectImpl::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        delete pPortionInfo;
        pPortionInfo = NULL;
    }
}

bool EditTextObjectImpl::HasOnlineSpellErrors() const
{
    ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
    {
        if ( it->GetWrongList() && !it->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditTextObjectImpl::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    rLst.clear();
    const ContentInfo& rC = aContents[nPara];
    for (size_t nAttr = 0; nAttr < rC.aAttribs.size(); ++nAttr)
    {
        const XEditAttribute& rAttr = rC.aAttribs[nAttr];
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
    return GetField() ? true : false;
}

const SvxFieldItem* EditTextObjectImpl::GetField() const
{
    if (aContents.size() == 1)
    {
        const ContentInfo& rC = aContents[0];
        if (rC.GetText().Len() == 1)
        {
            size_t nAttribs = rC.aAttribs.size();
            for (size_t nAttr = nAttribs; nAttr; )
            {
                const XEditAttribute& rX = rC.aAttribs[--nAttr];
                if (rX.GetItem()->Which() == EE_FEATURE_FIELD)
                    return static_cast<const SvxFieldItem*>(rX.GetItem());
            }
        }
    }
    return 0;
}

const SvxFieldData* EditTextObjectImpl::GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return NULL;

    const ContentInfo& rC = aContents[nPara];
    if (nPos >= rC.aAttribs.size())
        // URL position is out-of-bound.
        return NULL;

    ContentInfo::XEditAttributesType::const_iterator it = rC.aAttribs.begin(), itEnd = rC.aAttribs.end();
    size_t nCurPos = 0;
    for (; it != itEnd; ++it)
    {
        const XEditAttribute& rAttr = *it;
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

    return NULL; // field not found.
}

bool EditTextObjectImpl::HasField( sal_Int32 nType ) const
{
    size_t nParagraphs = aContents.size();
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];
        size_t nAttrs = rC.aAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = rC.aAttribs[nAttr];
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
    const ContentInfo& rC = aContents[nPara];
    return rC.GetParaAttribs();
}

void EditTextObjectImpl::SetParaAttribs(sal_Int32 nPara, const SfxItemSet& rAttribs)
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    ContentInfo& rC = aContents[nPara];
    rC.GetParaAttribs().Set(rAttribs);
    ClearPortionInfo();
}

bool EditTextObjectImpl::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    bool bChanged = false;

    for ( size_t nPara = aContents.size(); nPara; )
    {
        ContentInfo& rC = aContents[--nPara];

        for (size_t nAttr = rC.aAttribs.size(); nAttr; )
        {
            XEditAttribute& rAttr = rC.aAttribs[--nAttr];
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

class FindByParagraph : std::unary_function<editeng::Section, bool>
{
    size_t mnPara;
public:
    FindByParagraph(size_t nPara) : mnPara(nPara) {}
    bool operator() (const editeng::Section& rAttr) const
    {
        return rAttr.mnParagraph == mnPara;
    }
};

class FindBySectionStart : std::unary_function<editeng::Section, bool>
{
    size_t mnPara;
    size_t mnStart;
public:
    FindBySectionStart(size_t nPara, size_t nStart) : mnPara(nPara), mnStart(nStart) {}
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
        const ContentInfo& rC = aContents[nPara];
        SectionBordersType& rBorders = aParaBorders[nPara];
        rBorders.push_back(0);
        rBorders.push_back(rC.GetText().Len());
        for (size_t nAttr = 0; nAttr < rC.aAttribs.size(); ++nAttr)
        {
            const XEditAttribute& rAttr = rC.aAttribs[nAttr];
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
    for (size_t nPara = 0; nPara < aContents.size(); ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];

        itAttr = std::find_if(itAttr, aAttrs.end(), FindByParagraph(nPara));
        if (itAttr == aAttrs.end())
            // This should never happen. There is a logic error somewhere...
            return;

        for (size_t i = 0; i < rC.aAttribs.size(); ++i)
        {
            const XEditAttribute& rXAttr = rC.aAttribs[i];
            const SfxPoolItem* pItem = rXAttr.GetItem();
            if (!pItem)
                continue;

            size_t nStart = rXAttr.GetStart(), nEnd = rXAttr.GetEnd();
            std::vector<editeng::Section>::iterator itCurAttr = itAttr;

            // Find the container whose start position matches.
            itCurAttr = std::find_if(itCurAttr, aAttrs.end(), FindBySectionStart(nPara, nStart));
            if (itCurAttr == aAttrs.end())
                // This should never happen. There is a logic error somewhere...
                return;

            for (; itCurAttr != aAttrs.end() && itCurAttr->mnParagraph == nPara && itCurAttr->mnEnd <= nEnd; ++itCurAttr)
            {
                editeng::Section& rSecAttr = *itCurAttr;
                rSecAttr.maAttributes.push_back(pItem);
            }
        }
    }

    rAttrs.swap(aAttrs);
}

void EditTextObjectImpl::GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& rFamily) const
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    const ContentInfo& rC = aContents[nPara];
    rName = rC.GetStyle();
    rFamily = rC.GetFamily();
}

void EditTextObjectImpl::SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& rFamily)
{
    if (nPara < 0 || static_cast<size_t>(nPara) >= aContents.size())
        return;

    ContentInfo& rC = aContents[nPara];
    rC.GetStyle() = rName;
    rC.GetFamily() = rFamily;
}

bool EditTextObjectImpl::ImpChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    const size_t nParagraphs = aContents.size();
    bool bChanges = false;

    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        ContentInfo& rC = aContents[nPara];
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
    sal_Bool bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void EditTextObjectImpl::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const OUString& rOldName, const OUString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

editeng::FieldUpdater EditTextObjectImpl::GetFieldUpdater()
{
    return editeng::FieldUpdater(*mpFront);
}

namespace {

class FindAttribByChar : public std::unary_function<XEditAttribute, bool>
{
    sal_uInt16 mnWhich;
    sal_uInt16 mnChar;
public:
    FindAttribByChar(sal_uInt16 nWhich, sal_uInt16 nChar) : mnWhich(nWhich), mnChar(nChar) {}
    bool operator() (const XEditAttribute& rAttr) const
    {
        return (rAttr.GetItem()->Which() == mnWhich) && (rAttr.GetStart() <= mnChar) && (rAttr.GetEnd() > mnChar);
    }
};

}

void EditTextObjectImpl::StoreData( SvStream& rOStream ) const
{
    sal_uInt16 nVer = 602;
    rOStream << nVer;

    rOStream << static_cast<sal_Bool>(bOwnerOfPool);

    // First store the pool, later only the Surregate
    if ( bOwnerOfPool )
    {
        GetPool()->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
        GetPool()->Store( rOStream );
    }

    // Store Current text encoding ...
    rtl_TextEncoding eEncoding = GetSOStoreTextEncoding( osl_getThreadTextEncoding() );
    rOStream << (sal_uInt16) eEncoding;

    // The number of paragraphs ...
    size_t nParagraphs = aContents.size();
    // FIXME: this truncates, check usage of stream and if it can be changed,
    // i.e. is not persistent, adapt this and reader.
    sal_uInt16 nParagraphs_Stream = static_cast<sal_uInt16>(nParagraphs);
    rOStream << nParagraphs_Stream;

    sal_Unicode nUniChar = CH_FEATURE;
    char cFeatureConverted = OString(&nUniChar, 1, eEncoding).toChar();

    // The individual paragraphs ...
    for (size_t nPara = 0; nPara < nParagraphs_Stream; ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];

        // Text...
        OStringBuffer aBuffer(OUStringToOString(rC.GetText(), eEncoding));

        // Symbols?
        bool bSymbolPara = false;
        if (rC.GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON)
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)rC.GetParaAttribs().Get(EE_CHAR_FONTINFO);
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                aBuffer = OStringBuffer(OUStringToOString(rC.GetText(), RTL_TEXTENCODING_SYMBOL));
                bSymbolPara = true;
            }
        }
        for (size_t nA = 0; nA < rC.aAttribs.size(); ++nA)
        {
            const XEditAttribute& rAttr = rC.aAttribs[nA];

            if (rAttr.GetItem()->Which() == EE_CHAR_FONTINFO)
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)*rAttr.GetItem();
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    String aPart( rC.GetText(), rAttr.GetStart(), rAttr.GetEnd() - rAttr.GetStart() );
                    OString aNew(OUStringToOString(aPart, rFontItem.GetCharSet()));
                    aBuffer.remove(rAttr.GetStart(), rAttr.GetEnd() - rAttr.GetStart());
                    aBuffer.insert(rAttr.GetStart(), aNew);
                }

                // Convert StarSymbol back to StarBats
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    // Don't create a new Attrib with StarBats font, MBR changed the
                    // SvxFontItem::Store() to store StarBats instead of StarSymbol!
                    for (sal_uInt16 nChar = rAttr.GetStart(); nChar < rAttr.GetEnd(); ++nChar)
                    {
                        sal_Unicode cOld = rC.GetText().GetChar( nChar );
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

        FontToSubsFontConverter hConv = NULL;
        if (rC.GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON)
        {
            hConv = CreateFontToSubsFontConverter( ((const SvxFontItem&)rC.GetParaAttribs().Get( EE_CHAR_FONTINFO )).GetFamilyName(), FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        }
        if ( hConv )
        {
            for ( sal_uInt16 nChar = 0; nChar < rC.GetText().Len(); nChar++ )
            {
                const ContentInfo::XEditAttributesType& rAttribs = rC.aAttribs;
                ContentInfo::XEditAttributesType::const_iterator it =
                    std::find_if(rAttribs.begin(), rAttribs.end(),
                                 FindAttribByChar(EE_CHAR_FONTINFO, nChar));

                if (it == rAttribs.end())
                {
                    sal_Unicode cOld = rC.GetText().GetChar( nChar );
                    char cConv = OUStringToOString(OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                    if ( cConv )
                        aBuffer[nChar] = cConv;
                }
            }

            DestroyFontToSubsFontConverter( hConv );

        }


        // Convert CH_FEATURE to CH_FEATURE_OLD
        OString aText = aBuffer.makeStringAndClear().replace(cFeatureConverted, CH_FEATURE_OLD);
        write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOStream, aText);

        // StyleName and Family...
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStream, rC.GetStyle(), eEncoding);
        rOStream << static_cast<sal_uInt16>(rC.GetFamily());

        // Paragraph attributes ...
        rC.GetParaAttribs().Store( rOStream );

        // The number of attributes ...
        size_t nAttribs = rC.aAttribs.size();
        rOStream << static_cast<sal_uInt16>(nAttribs);

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attribute
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for (size_t nAttr = 0; nAttr < nAttribs; ++nAttr)
        {
            const XEditAttribute& rX = rC.aAttribs[nAttr];

            rOStream << rX.GetItem()->Which();
            GetPool()->StoreSurrogate(rOStream, rX.GetItem());
            rOStream << rX.GetStart();
            rOStream << rX.GetEnd();
        }
    }

    rOStream << nMetric;

    rOStream << nUserType;
    rOStream << nObjSettings;

    rOStream << static_cast<sal_Bool>(bVertical);
    rOStream << nScriptType;

    rOStream << static_cast<sal_Bool>(bStoreUnicodeStrings);
    if ( bStoreUnicodeStrings )
    {
        for ( size_t nPara = 0; nPara < nParagraphs_Stream; nPara++ )
        {
            const ContentInfo& rC = aContents[nPara];
            sal_uInt16 nL = rC.GetText().Len();
            rOStream << nL;
            rOStream.Write(rC.GetText().GetBuffer(), nL*sizeof(sal_Unicode));

            // StyleSheetName must be Unicode too!
            // Copy/Paste from EA3 to BETA or from BETA to EA3 not possible, not needed...
            // If needed, change nL back to sal_uLong and increase version...
            nL = rC.GetStyle().Len();
            rOStream << nL;
            rOStream.Write(rC.GetStyle().GetBuffer(), nL*sizeof(sal_Unicode));
        }
    }
}

void EditTextObjectImpl::CreateData( SvStream& rIStream )
{
    rIStream >> nVersion;

    // The text object was first created with the current setting of
    // pTextObjectPool.
    sal_Bool bOwnerOfCurrent = bOwnerOfPool;
    sal_Bool b;
    rIStream >> b;
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
    rIStream >> nCharSet;

    rtl_TextEncoding eSrcEncoding = GetSOLoadTextEncoding( (rtl_TextEncoding)nCharSet );

    // The number of paragraphs ...
    sal_uInt16 nParagraphs;
    rIStream >> nParagraphs;

    // The individual paragraphs ...
    for ( sal_uLong nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // The Text...
        OString aByteString = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStream);
        pC->GetText() = OStringToOUString(aByteString, eSrcEncoding);

        // StyleName and Family...
        pC->GetStyle() = rIStream.ReadUniOrByteString(eSrcEncoding);
        sal_uInt16 nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Paragraph attributes ...
        pC->GetParaAttribs().Load( rIStream );

        // The number of attributes ...
        sal_uInt16 nTmp16;
        rIStream >> nTmp16;
        size_t nAttribs = nTmp16;

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attributes
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        size_t nAttr;
        for (nAttr = 0; nAttr < nAttribs; ++nAttr)
        {
            sal_uInt16 _nWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> _nWhich;
            _nWhich = pPool->GetNewWhich( _nWhich );
            pItem = pPool->LoadSurrogate( rIStream, _nWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                if ( pItem->Which() == EE_FEATURE_NOTCONV )
                {
                    sal_Char cEncodedChar = aByteString[nStart];
                    sal_Unicode cChar = OUString(&cEncodedChar, 1,
                        ((SvxCharSetColorItem*)pItem)->GetCharSet()).toChar();
                    pC->GetText().SetChar(nStart, cChar);
                }
                else
                {
                    XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                    pC->aAttribs.push_back(pAttr);

                    if ( ( _nWhich >= EE_FEATURE_START ) && ( _nWhich <= EE_FEATURE_END ) )
                    {
                        // Convert CH_FEATURE to CH_FEATURE_OLD
                        DBG_ASSERT( (sal_uInt8) aByteString[nStart] == CH_FEATURE_OLD, "CreateData: CH_FEATURE expected!" );
                        if ( (sal_uInt8) aByteString[nStart] == CH_FEATURE_OLD )
                            pC->GetText().SetChar( nStart, CH_FEATURE );
                    }
                }
            }
        }

        // But check for paragraph and character symbol attribs here,
        // FinishLoad will not be called in OpenOffice Calc, no StyleSheets...

        sal_Bool bSymbolPara = false;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                pC->GetText() = OStringToOUString(aByteString, RTL_TEXTENCODING_SYMBOL);
                bSymbolPara = true;
            }
        }

        for (nAttr = pC->aAttribs.size(); nAttr; )
        {
            const XEditAttribute& rAttr = pC->aAttribs[--nAttr];
            if ( rAttr.GetItem()->Which() == EE_CHAR_FONTINFO )
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)*rAttr.GetItem();
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    OString aPart(aByteString.copy(rAttr.GetStart(), rAttr.GetEnd()-rAttr.GetStart()));
                    OUString aNew(OStringToOUString(aPart, rFontItem.GetCharSet()));
                    pC->GetText().Erase( rAttr.GetStart(), rAttr.GetEnd()-rAttr.GetStart() );
                    pC->GetText().Insert( aNew, rAttr.GetStart() );
                }

                // Convert StarMath and StarBats to StarSymbol
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    SvxFontItem aNewFontItem( rFontItem );
                    aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );

                    // Replace the existing attribute with a new one.
                    XEditAttribute* pNewAttr = CreateAttrib(aNewFontItem, rAttr.GetStart(), rAttr.GetEnd());

                    pPool->Remove(*rAttr.GetItem());
                    pC->aAttribs.erase(pC->aAttribs.begin()+nAttr);
                    pC->aAttribs.insert(pC->aAttribs.begin()+nAttr, pNewAttr);

                    for ( sal_uInt16 nChar = pNewAttr->GetStart(); nChar < pNewAttr->GetEnd(); nChar++ )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }


        // Convert StarMath and StarBats to StarSymbol
        // Maybe old symbol font as paragraph attribute?
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
            if ( hConv )
            {
                SvxFontItem aNewFontItem( rFontItem );
                aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );
                pC->GetParaAttribs().Put( aNewFontItem );

                for ( sal_uInt16 nChar = 0; nChar < pC->GetText().Len(); nChar++ )
                {
                    const ContentInfo::XEditAttributesType& rAttribs = pC->aAttribs;
                    ContentInfo::XEditAttributesType::const_iterator it =
                        std::find_if(rAttribs.begin(), rAttribs.end(),
                                     FindAttribByChar(EE_CHAR_FONTINFO, nChar));

                    if (it == rAttribs.end())
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
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
        rIStream >> nTmpMetric;
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
        rIStream >> nUserType;
        rIStream >> nObjSettings;
    }

    if ( nVersion >= 601 )
    {
        sal_Bool bTmp;
        rIStream >> bTmp;
        bVertical = bTmp;
    }

    if ( nVersion >= 602 )
    {
        rIStream >> nScriptType;

        sal_Bool bUnicodeStrings;
        rIStream >> bUnicodeStrings;
        if ( bUnicodeStrings )
        {
            for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
            {
                ContentInfo& rC = aContents[nPara];
                sal_uInt16 nL;

                // Text
                rIStream >> nL;
                if ( nL )
                {
                    rtl_uString *pStr = rtl_uString_alloc(nL);
                    rIStream.Read(pStr->buffer, nL*sizeof(sal_Unicode));
                    rC.GetText() = OUString(pStr, SAL_NO_ACQUIRE);
                }

                // StyleSheetName
                rIStream >> nL;
                if ( nL )
                {
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
            ContentInfo& rC = aContents[i];
            const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(rC.GetParaAttribs().Get(EE_PARA_LRSPACE));
            if ( rLRSpace.GetTxtLeft() && ( rC.GetParaAttribs().GetItemState( EE_PARA_TABS ) == SFX_ITEM_ON ) )
            {
                const SvxTabStopItem& rTabs = static_cast<const SvxTabStopItem&>(rC.GetParaAttribs().Get(EE_PARA_TABS));
                SvxTabStopItem aNewTabs( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
                for ( sal_uInt16 t = 0; t < rTabs.Count(); t++ )
                {
                    const SvxTabStop& rT = rTabs[ t ];
                    aNewTabs.Insert( SvxTabStop( rT.GetTabPos() - rLRSpace.GetTxtLeft(),
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
        const ContentInfo& rCandA = aContents[i];
        const ContentInfo& rCandB = rCompare.aContents[i];

        if(!rCandA.isWrongListEqual(rCandB))
        {
            return false;
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
