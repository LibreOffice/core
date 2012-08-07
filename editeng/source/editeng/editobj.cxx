/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <comphelper/string.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <tools/stream.hxx>

#include "editeng/fieldupdater.hxx"
#include <editobj2.hxx>
#include <editeng/editdata.hxx>
#include <editattr.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/graph.hxx>
#include <svl/intitem.hxx>
#include <unotools/fontcvt.hxx>
#include <tools/tenccvt.hxx>

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

ContentInfo::ContentInfo( SfxItemPool& rPool ) : aParaAttribs( rPool, EE_PARA_START, EE_CHAR_END )
{
    eFamily = SFX_STYLE_FAMILY_PARA;
    pWrongs = NULL;
}

// the real Copy constructor is nonsens, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse )
    : aParaAttribs( rPoolToUse, EE_PARA_START, EE_CHAR_END )
    , pWrongs(0)
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );
    aText = rCopyFrom.GetText();
    aStyle = rCopyFrom.GetStyle();
    eFamily = rCopyFrom.GetFamily();

    for (size_t i = 0; i < rCopyFrom.aAttribs.size(); ++i)
    {
        const XEditAttribute& rAttr = rCopyFrom.aAttribs[i];
        XEditAttribute* pMyAttr = MakeXEditAttribute(
            rPoolToUse, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
        aAttribs.push_back(pMyAttr);
    }

    if ( rCopyFrom.GetWrongList() )
        pWrongs = rCopyFrom.GetWrongList()->Clone();
}

ContentInfo::~ContentInfo()
{
    XEditAttributesType::iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
        aParaAttribs.GetPool()->Remove(*it->GetItem());
    aAttribs.clear();

    delete pWrongs;
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

EditTextObject::EditTextObject( sal_uInt16 n)
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = n;
}

EditTextObject::EditTextObject( const EditTextObject& r )
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = r.nWhich;
}

EditTextObject::~EditTextObject()
{
    DBG_DTOR( EE_EditTextObject, 0 );
}

size_t EditTextObject::GetParagraphCount() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

String EditTextObject::GetText(size_t /* nParagraph */) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return String();
}

void EditTextObject::Insert(const EditTextObject& /* rObj */, size_t /* nPara */)
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

EditTextObject* EditTextObject::CreateTextObject(size_t /*nPara*/, size_t /*nParas*/) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::RemoveParagraph(size_t /*nPara*/)
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::HasPortionInfo() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

void EditTextObject::ClearPortionInfo()
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::HasOnlineSpellErrors() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

sal_Bool EditTextObject::HasCharAttribs( sal_uInt16 ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

void EditTextObject::GetCharAttribs( sal_uInt16 /*nPara*/, std::vector<EECharAttrib>& /*rLst*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

void EditTextObject::MergeParaAttribs( const SfxItemSet& /*rAttribs*/, sal_uInt16 /*nStart*/, sal_uInt16 /*nEnd*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::IsFieldObject() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

const SvxFieldItem* EditTextObject::GetField() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

sal_Bool EditTextObject::HasField( TypeId /*aType*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

SfxItemSet EditTextObject::GetParaAttribs(size_t /*nPara*/) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return SfxItemSet( *(SfxItemPool*)NULL );
}

void EditTextObject::SetParaAttribs(size_t /*nPara*/, const SfxItemSet& /*rAttribs*/)
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::RemoveCharAttribs( sal_uInt16 /*nWhich*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

sal_Bool EditTextObject::RemoveParaAttribs( sal_uInt16 /*nWhich*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

sal_Bool EditTextObject::HasStyleSheet( const XubString& /*rName*/, SfxStyleFamily /*eFamily*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

void EditTextObject::GetStyleSheet(size_t /*nPara*/, String& /*rName*/, SfxStyleFamily& /*eFamily*/) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

void EditTextObject::SetStyleSheet(size_t /*nPara*/, const String& /*rName*/, const SfxStyleFamily& /*eFamily*/)
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::ChangeStyleSheets( const XubString&, SfxStyleFamily,
                                            const XubString&, SfxStyleFamily )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

void EditTextObject::ChangeStyleSheetName( SfxStyleFamily /*eFamily*/,
                const XubString& /*rOldName*/, const XubString& /*rNewName*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_uInt16 EditTextObject::GetUserType() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::SetUserType( sal_uInt16 )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_uLong EditTextObject::GetObjectSettings() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::SetObjectSettings( sal_uLong )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

bool EditTextObject::IsVertical() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return false;
}

void EditTextObject::SetVertical( bool bVertical )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    ((BinTextObject*)this)->SetVertical( bVertical );
}

sal_uInt16 EditTextObject::GetScriptType() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return ((const BinTextObject*)this)->GetScriptType();
}


sal_Bool EditTextObject::Store( SvStream& rOStream ) const
{
    if ( rOStream.GetError() )
        return false;

    sal_Size nStartPos = rOStream.Tell();

    rOStream << (sal_uInt16)Which();

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

    DBG_ASSERT( ( nWhich == 0x22 /*EE_FORMAT_BIN300*/ ) || ( nWhich == EE_FORMAT_BIN ), "CreateTextObject: Unknown Object!" );

    if ( rIStream.GetError() )
        return NULL;

    EditTextObject* pTxtObj = NULL;
    switch ( nWhich )
    {
        case 0x22 /*BIN300*/:       pTxtObj = new BinTextObject( 0 );
                                    ((BinTextObject*)pTxtObj)->CreateData300( rIStream );
                                    break;
        case EE_FORMAT_BIN:         pTxtObj = new BinTextObject( pGlobalTextObjectPool );
                                    pTxtObj->CreateData( rIStream );
                                    break;
        default:
        {
            // If I do not know the format, I overwrite the contents:
            rIStream.SetError( EE_READWRITE_WRONGFORMAT );
        }
    }

    // Make sure that the stream is left at the correct place.
    sal_Size nFullSz = sizeof( nWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
    return pTxtObj;
}

void EditTextObject::StoreData( SvStream& ) const
{
    OSL_FAIL( "StoreData: Base class!" );
}

void EditTextObject::CreateData( SvStream& )
{
    OSL_FAIL( "CreateData: Base class!" );
}

sal_uInt16 EditTextObject::GetVersion() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

bool EditTextObject::operator==( const EditTextObject& rCompare ) const
{
    return static_cast< const BinTextObject* >( this )->operator==( static_cast< const BinTextObject& >( rCompare ) );
}

// #i102062#
bool EditTextObject::isWrongListEqual(const EditTextObject& rCompare) const
{
    return static_cast< const BinTextObject* >(this)->isWrongListEqual(static_cast< const BinTextObject& >(rCompare));
}

// from SfxItemPoolUser
void BinTextObject::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
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

BinTextObject::BinTextObject( SfxItemPool* pP ) :
    EditTextObject( EE_FORMAT_BIN ),
    SfxItemPoolUser()
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
    // be decoupled and deleted whcih would lead to crashes.
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
        pPool->AddSfxItemPoolUser(*this);
    }

    bVertical = false;
    bStoreUnicodeStrings = false;
    nScriptType = 0;
}

BinTextObject::BinTextObject( const BinTextObject& r ) :
    EditTextObject( r ),
    SfxItemPoolUser()
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
        pPool->AddSfxItemPoolUser(*this);
    }

    if ( bOwnerOfPool && pPool && r.pPool )
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    aContents.reserve(r.aContents.size());
    ContentInfosType::const_iterator it = r.aContents.begin(), itEnd = r.aContents.end();
    for (; it != itEnd; ++it)
        aContents.push_back(new ContentInfo(*it, *pPool));
}

BinTextObject::~BinTextObject()
{
    if(!bOwnerOfPool && pPool)
    {
        pPool->RemoveSfxItemPoolUser(*this);
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

sal_uInt16 BinTextObject::GetUserType() const
{
    return nUserType;
}

void BinTextObject::SetUserType( sal_uInt16 n )
{
    nUserType = n;
}

sal_uLong BinTextObject::GetObjectSettings() const
{
    return nObjSettings;
}

void BinTextObject::SetObjectSettings( sal_uLong n )
{
    nObjSettings = n;
}

bool BinTextObject::IsVertical() const
{
    return bVertical;
}

void BinTextObject::SetVertical( bool b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}

sal_uInt16 BinTextObject::GetScriptType() const
{
    return nScriptType;
}

void BinTextObject::SetScriptType( sal_uInt16 nType )
{
    nScriptType = nType;
}

EditTextObject* BinTextObject::Clone() const
{
    return new BinTextObject( *this );
}

XEditAttribute* BinTextObject::CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void BinTextObject::DestroyAttrib( XEditAttribute* pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
    delete pAttr;
}

BinTextObject::ContentInfosType& BinTextObject::GetContents()
{
    return aContents;
}

const BinTextObject::ContentInfosType& BinTextObject::GetContents() const
{
    return aContents;
}

ContentInfo* BinTextObject::CreateAndInsertContent()
{
    aContents.push_back(new ContentInfo(*pPool));
    return &aContents.back();
}

size_t BinTextObject::GetParagraphCount() const
{
    return aContents.size();
}

String BinTextObject::GetText(size_t nPara) const
{
    if (nPara >= aContents.size())
        return String();

    return aContents[nPara].GetText();
}

void BinTextObject::Insert(const EditTextObject& rObj, size_t nDestPara)
{
    DBG_ASSERT( rObj.Which() == EE_FORMAT_BIN, "UTO: unknown Textobjekt" );

    const BinTextObject& rBinObj = (const BinTextObject&)rObj;

    if (nDestPara > aContents.size())
        nDestPara = aContents.size();

    const ContentInfosType& rCIs = rBinObj.aContents;
    for (size_t i = 0, n = rCIs.size(); i < n; ++i)
    {
        const ContentInfo& rC = rCIs[i];
        size_t nPos = nDestPara + i;
        aContents.insert(
            aContents.begin()+nPos, new ContentInfo(rC, *GetPool()));
    }

    ClearPortionInfo();
}

EditTextObject* BinTextObject::CreateTextObject(size_t nPara, size_t nParas) const
{
    if (nPara >= aContents.size() || !nParas)
        return NULL;

    // Only split the Pool, when a the Pool is set externally.
    BinTextObject* pObj = new BinTextObject( bOwnerOfPool ? 0 : pPool );
    if ( bOwnerOfPool && pPool )
        pObj->GetPool()->SetDefaultMetric( pPool->GetMetric( DEF_METRIC ) );

    // If complete text is only one ScriptType, this is valid.
    // If text contains different ScriptTypes, this shouldn't be a problem...
    pObj->nScriptType = nScriptType;

    const size_t nEndPara = nPara+nParas-1;
    for (size_t i = nPara; i <= nEndPara; ++i)
    {
        const ContentInfo& rC = aContents[i];
        ContentInfo* pNew = new ContentInfo(rC, *pObj->GetPool());
        pObj->aContents.push_back(pNew);
    }
    return pObj;
}

void BinTextObject::RemoveParagraph(size_t nPara)
{
    DBG_ASSERT( nPara < aContents.size(), "BinTextObject::GetText: Paragraph does not exist!" );
    if (nPara >= aContents.size())
        return;

    ContentInfosType::iterator it = aContents.begin();
    std::advance(it, nPara);
    aContents.erase(it);
    ClearPortionInfo();
}

sal_Bool BinTextObject::HasPortionInfo() const
{
    return pPortionInfo ? true : false;
}

void BinTextObject::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        delete pPortionInfo;
        pPortionInfo = NULL;
    }
}

sal_Bool BinTextObject::HasOnlineSpellErrors() const
{
    ContentInfosType::const_iterator it = aContents.begin(), itEnd = aContents.end();
    for (; it != itEnd; ++it)
    {
        if ( it->GetWrongList() && !it->GetWrongList()->empty() )
            return true;
    }
    return false;
}

sal_Bool BinTextObject::HasCharAttribs( sal_uInt16 _nWhich ) const
{
    for (size_t nPara = aContents.size(); nPara; )
    {
        const ContentInfo& rC = aContents[--nPara];

        size_t nAttribs = rC.aAttribs.size();
        if ( nAttribs && !_nWhich )
            return true;

        for (size_t nAttr = nAttribs; nAttr; )
        {
            const XEditAttribute& rX = rC.aAttribs[--nAttr];
            if (rX.GetItem()->Which() == _nWhich)
                return true;
        }
    }
    return false;
}

void BinTextObject::GetCharAttribs( sal_uInt16 nPara, std::vector<EECharAttrib>& rLst ) const
{
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

void BinTextObject::MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    bool bChanged = false;

    for (size_t nPara = aContents.size(); nPara; )
    {
        ContentInfo& rC = aContents[--nPara];

        for ( sal_uInt16 nW = nStart; nW <= nEnd; nW++ )
        {
            if ( ( rC.GetParaAttribs().GetItemState( nW, false ) != SFX_ITEM_ON )
                    && ( rAttribs.GetItemState( nW, false ) == SFX_ITEM_ON ) )
            {
                rC.GetParaAttribs().Put( rAttribs.Get( nW ) );
                bChanged = true;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();
}

sal_Bool BinTextObject::IsFieldObject() const
{
    return BinTextObject::GetField() ? true : false;
}

const SvxFieldItem* BinTextObject::GetField() const
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

sal_Bool BinTextObject::HasField( TypeId aType ) const
{
    size_t nParagraphs = aContents.size();
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];
        size_t nAttrs = rC.aAttribs.size();
        for (size_t nAttr = 0; nAttr < nAttrs; ++nAttr)
        {
            const XEditAttribute& rAttr = rC.aAttribs[nAttr];
            if (rAttr.GetItem()->Which() == EE_FEATURE_FIELD)
            {
                if ( !aType )
                    return true;

                const SvxFieldData* pFldData = static_cast<const SvxFieldItem*>(rAttr.GetItem())->GetField();
                if ( pFldData && pFldData->IsA( aType ) )
                    return true;
            }
        }
    }
    return false;
}

SfxItemSet BinTextObject::GetParaAttribs(size_t nPara) const
{
    const ContentInfo& rC = aContents[nPara];
    return rC.GetParaAttribs();
}

void BinTextObject::SetParaAttribs(size_t nPara, const SfxItemSet& rAttribs)
{
    ContentInfo& rC = aContents[nPara];
    rC.GetParaAttribs().Set(rAttribs);
    ClearPortionInfo();
}

sal_Bool BinTextObject::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    sal_Bool bChanged = false;

    for ( sal_uInt16 nPara = aContents.size(); nPara; )
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

sal_Bool BinTextObject::RemoveParaAttribs( sal_uInt16 _nWhich )
{
    bool bChanged = false;

    for (size_t nPara = aContents.size(); nPara; )
    {
        ContentInfo& rC = aContents[--nPara];

        if ( !_nWhich )
        {
            if (rC.GetParaAttribs().Count())
                bChanged = true;
            rC.GetParaAttribs().ClearItem();
        }
        else
        {
            if (rC.GetParaAttribs().GetItemState(_nWhich) == SFX_ITEM_ON)
            {
                rC.GetParaAttribs().ClearItem(_nWhich);
                bChanged = true;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

sal_Bool BinTextObject::HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const
{
    size_t nParagraphs = aContents.size();
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];
        if (rC.GetFamily() == eFamily && rC.GetStyle() == rName)
            return true;
    }
    return false;
}

void BinTextObject::GetStyleSheet(size_t nPara, String& rName, SfxStyleFamily& rFamily) const
{
    if (nPara >= aContents.size())
        return;

    const ContentInfo& rC = aContents[nPara];
    rName = rC.GetStyle();
    rFamily = rC.GetFamily();
}

void BinTextObject::SetStyleSheet(size_t nPara, const String& rName, const SfxStyleFamily& rFamily)
{
    if (nPara >= aContents.size())
        return;

    ContentInfo& rC = aContents[nPara];
    rC.GetStyle() = rName;
    rC.GetFamily() = rFamily;
}

sal_Bool BinTextObject::ImpChangeStyleSheets(
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

sal_Bool BinTextObject::ChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    sal_Bool bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void BinTextObject::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const XubString& rOldName, const XubString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

editeng::FieldUpdater BinTextObject::GetFieldUpdater()
{
    return editeng::FieldUpdater(*this);
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

void BinTextObject::StoreData( SvStream& rOStream ) const
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
    rOStream << static_cast<sal_uInt16>(nParagraphs);

    sal_Unicode nUniChar = CH_FEATURE;
    char cFeatureConverted = rtl::OString(&nUniChar, 1, eEncoding).toChar();

    // The individual paragraphs ...
    for (size_t nPara = 0; nPara < nParagraphs; ++nPara)
    {
        const ContentInfo& rC = aContents[nPara];

        // Text...
        rtl::OStringBuffer aBuffer(rtl::OUStringToOString(rC.GetText(), eEncoding));

        // Symbols?
        bool bSymbolPara = false;
        if (rC.GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON)
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)rC.GetParaAttribs().Get(EE_CHAR_FONTINFO);
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                aBuffer = rtl::OStringBuffer(rtl::OUStringToOString(rC.GetText(), RTL_TEXTENCODING_SYMBOL));
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
                    rtl::OString aNew(rtl::OUStringToOString(aPart, rFontItem.GetCharSet()));
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
                        char cConv = rtl::OUStringToOString(rtl::OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
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
                    char cConv = rtl::OUStringToOString(rtl::OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                    if ( cConv )
                        aBuffer[nChar] = cConv;
                }
            }

            DestroyFontToSubsFontConverter( hConv );

        }


        // Convert CH_FEATURE to CH_FEATURE_OLD
        rtl::OString aText = aBuffer.makeStringAndClear().replace(cFeatureConverted, CH_FEATURE_OLD);
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
        for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
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

void BinTextObject::CreateData( SvStream& rIStream )
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
        rtl::OString aByteString = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStream);
        pC->GetText() = rtl::OStringToOUString(aByteString, eSrcEncoding);

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
                    sal_Unicode cChar = rtl::OUString(&cEncodedChar, 1,
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
                pC->GetText() = rtl::OStringToOUString(aByteString, RTL_TEXTENCODING_SYMBOL);
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
                    rtl::OString aPart(aByteString.copy(rAttr.GetStart(), rAttr.GetEnd()-rAttr.GetStart()));
                    rtl::OUString aNew(rtl::OStringToOUString(aPart, rFontItem.GetCharSet()));
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
            using comphelper::string::rtl_uString_alloc;

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
                    rC.GetText() = rtl::OUString(pStr, SAL_NO_ACQUIRE);
                }

                // StyleSheetName
                rIStream >> nL;
                if ( nL )
                {
                    rtl_uString *pStr = rtl_uString_alloc(nL);
                    rIStream.Read(pStr->buffer, nL*sizeof(sal_Unicode) );
                    rC.GetStyle() = rtl::OUString(pStr, SAL_NO_ACQUIRE);
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

sal_uInt16 BinTextObject::GetVersion() const
{
    return nVersion;
}

bool BinTextObject::operator==( const BinTextObject& rCompare ) const
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
bool BinTextObject::isWrongListEqual(const BinTextObject& rCompare) const
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

#define CHARSETMARKER   0x9999

void BinTextObject::CreateData300( SvStream& rIStream )
{
    // For forward compatibility.

    // First load the Pool...
    // Is always saved in the 300!
    GetPool()->Load( rIStream );

    // The number of paragraphs ...
    sal_uInt32 nParagraphs;
    rIStream >> nParagraphs;

    // The individual paragraphs...
    for ( sal_uLong nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // The Text...
        pC->GetText() = rIStream.ReadUniOrByteString(rIStream.GetStreamCharSet());

        // StyleName and Family...
        pC->GetStyle() = rIStream.ReadUniOrByteString(rIStream.GetStreamCharSet());
        sal_uInt16 nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Paragraph attributes ...
        pC->GetParaAttribs().Load( rIStream );

        // The number of attributes ...
        sal_uInt32 nAttribs;
        rIStream >> nAttribs;

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attribute
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( sal_uLong nAttr = 0; nAttr < nAttribs; nAttr++ )
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
                XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                pC->aAttribs.push_back(pAttr);
            }
        }
    }

    // Check whether a font was saved
    sal_uInt16 nCharSetMarker;
    rIStream >> nCharSetMarker;
    if ( nCharSetMarker == CHARSETMARKER )
    {
        sal_uInt16 nCharSet;
        rIStream >> nCharSet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
