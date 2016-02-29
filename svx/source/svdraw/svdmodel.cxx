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


#include <svx/svdmodel.hxx>

#include <cassert>
#include <math.h>

#include <osl/endian.h>
#include <rtl/strbuf.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/configmgr.hxx>

#include <svl/whiter.hxx>
#include <svx/xit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>

#include <svx/xtable.hxx>

#include "svx/svditer.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/textchain.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdoole2.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include "svdoutlinercache.hxx"

#include "svx/xflclit.hxx"
#include "svx/xlnclit.hxx"

#include "officecfg/Office/Common.hxx"
#include "editeng/fontitem.hxx"
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/outlobj.hxx>
#include "editeng/forbiddencharacterstable.hxx"
#include <svl/zforlist.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>

#include <tools/tenccvt.hxx>
#include <unotools/syslocale.hxx>

#include <svx/sdr/properties/properties.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <memory>
#include <libxml/xmlwriter.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


struct SdrModelImpl
{
    SfxUndoManager* mpUndoManager;
    SdrUndoFactory* mpUndoFactory;
};


void SdrModel::ImpCtor(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* _pEmbeddedHelper,
    bool bUseExtColorTable, bool bLoadRefCounts)
{
    mpImpl = new SdrModelImpl;
    mpImpl->mpUndoManager=nullptr;
    mpImpl->mpUndoFactory=nullptr;
    mbInDestruction = false;
    aObjUnit=SdrEngineDefaults::GetMapFraction();
    eObjUnit=SdrEngineDefaults::GetMapUnit();
    eUIUnit=FUNIT_MM;
    aUIScale=Fraction(1,1);
    nUIUnitKomma=0;
    bUIOnlyKomma=false;
    pLayerAdmin=nullptr;
    pItemPool=pPool;
    bMyPool=false;
    m_pEmbeddedHelper=_pEmbeddedHelper;
    pDrawOutliner=nullptr;
    pHitTestOutliner=nullptr;
    pRefOutDev=nullptr;
    mpLibreOfficeKitCallback = nullptr;
    mpLibreOfficeKitData = nullptr;
    mbTiledSearching = false;
    nProgressAkt=0;
    nProgressMax=0;
    nProgressOfs=0;
    pDefaultStyleSheet=nullptr;
    mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = nullptr;
    pLinkManager=nullptr;
    pUndoStack=nullptr;
    pRedoStack=nullptr;
    nMaxUndoCount=16;
    mnUniqueCommentID=0;
    pAktUndoGroup=nullptr;
    nUndoLevel=0;
    mbUndoEnabled=true;
    nProgressPercent=0;
    nLoadVersion=0;
    bExtColorTable=false;
    mbChanged = false;
    bInfoChanged=false;
    bPagNumsDirty=false;
    bMPgNumsDirty=false;
    bTransportContainer = false;
    bSavePortable=false;
    bSaveCompressed=false;
    bSaveNative=false;
    bSwapGraphics=false;
    nSwapGraphicsMode=SdrSwapGraphicsMode::DEFAULT;
    bSaveOLEPreview=false;
    bPasteResize=false;
    bNoBitmapCaching=false;
    bReadOnly=false;
    nStreamNumberFormat=SvStreamEndian::BIG;
    nDefaultTabulator=0;
    mpNumberFormatter = nullptr;
    bTransparentTextFrames=false;
    bStarDrawPreviewMode = false;
    nStarDrawPreviewMasterPageNum = SDRPAGE_NOTFOUND;
    mpForbiddenCharactersTable = nullptr;
    mbModelLocked = false;
    mpOutlinerCache = nullptr;
    mbKernAsianPunctuation = false;
    mbAddExtLeading = false;
    mnHandoutPageCount = 0;
    nReserveUInt6 = 0;
    nReserveUInt7 = 0;

    mbDisableTextEditUsesCommonUndoManager = false;

    if (!utl::ConfigManager::IsAvoidConfig())
        mnCharCompressType =
            officecfg::Office::Common::AsianLayout::CompressCharacterDistance::
            get();
    else
        mnCharCompressType = 0;

#ifdef OSL_LITENDIAN
    nStreamNumberFormat=SvStreamEndian::LITTLE;
#endif
    bExtColorTable=bUseExtColorTable;

    if ( pPool == nullptr )
    {
        pItemPool=new SdrItemPool(nullptr, bLoadRefCounts);
        // Outliner doesn't have its own Pool, so use the EditEngine's
        SfxItemPool* pOutlPool=EditEngine::CreatePool( bLoadRefCounts );
        // OutlinerPool as SecondaryPool of SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
        // remember that I created both pools myself
        bMyPool=true;
    }
    pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);

// using static SdrEngineDefaults only if default SvxFontHeight item is not available
    const SfxPoolItem* pPoolItem = pItemPool->GetPoolDefaultItem( EE_CHAR_FONTHEIGHT );
    if ( pPoolItem )
        nDefTextHgt = static_cast<const SvxFontHeightItem*>(pPoolItem)->GetHeight();
    else
        nDefTextHgt = SdrEngineDefaults::GetFontHeight();

    pItemPool->SetPoolDefaultItem( makeSdrTextWordWrapItem( false ) );

    SetTextDefaults();
    pLayerAdmin=new SdrLayerAdmin;
    pLayerAdmin->SetModel(this);
    ImpSetUIUnit();

    // can't create DrawOutliner OnDemand, because I can't get the Pool,
    // then (only from 302 onwards!)
    pDrawOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, *this);
    ImpSetOutlinerDefaults(pDrawOutliner, true);

    pHitTestOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, *this);
    ImpSetOutlinerDefaults(pHitTestOutliner, true);

    /* Start Text Chaining related code */
    // Initialize Chaining Outliner
    pChainingOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, *this );
    ImpSetOutlinerDefaults(pChainingOutliner, true);

    // Make a TextChain
    pTextChain = new TextChain;
    /* End Text Chaining related code */

    ImpCreateTables();
}

SdrModel::SdrModel():
    maMaPag(),
    maPages()
{
    ImpCtor(nullptr, nullptr, false, LOADREFCOUNTS);
}

SdrModel::SdrModel(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, bool bLoadRefCounts):
    maMaPag(),
    maPages()
{
    ImpCtor(pPool,pPers,bUseExtColorTable, bLoadRefCounts);
}

SdrModel::SdrModel(const OUString& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, bool bLoadRefCounts):
    maMaPag(),
    maPages(),
    aTablePath(rPath)
{
    ImpCtor(pPool,pPers,bUseExtColorTable, bLoadRefCounts);
}

SdrModel::~SdrModel()
{

    mbInDestruction = true;

    Broadcast(SdrHint(HINT_MODELCLEARED));

    delete mpOutlinerCache;

    ClearUndoBuffer();
#ifdef DBG_UTIL
    if(pAktUndoGroup)
    {
        OStringBuffer aStr("In the Dtor of the SdrModel there is an open Undo left: \"");
        aStr.append(OUStringToOString(pAktUndoGroup->GetComment(), osl_getThreadTextEncoding()))
            .append('\"');
        OSL_FAIL(aStr.getStr());
    }
#endif
    delete pAktUndoGroup;

    ClearModel(true);

    delete pLayerAdmin;

    delete pTextChain;
    // Delete DrawOutliner only after deleting ItemPool, because ItemPool
    // references Items of the DrawOutliner!
    delete pChainingOutliner;
    delete pHitTestOutliner;
    delete pDrawOutliner;

    // delete StyleSheetPool, derived classes should not do this since
    // the DrawingEngine may need it in its destructor
    if( mxStyleSheetPool.is() )
    {
        Reference< XComponent > xComponent( dynamic_cast< cppu::OWeakObject* >( mxStyleSheetPool.get() ), UNO_QUERY );
        if( xComponent.is() ) try
        {
            xComponent->dispose();
        }
        catch( RuntimeException& )
        {
        }
        mxStyleSheetPool.clear();
    }

    if (bMyPool)
    {
        // delete Pools if they're mine
        SfxItemPool* pOutlPool=pItemPool->GetSecondaryPool();
        SfxItemPool::Free(pItemPool);
        // OutlinerPool has to be deleted after deleting ItemPool, because
        // ItemPool contains SetItems that themselves reference Items from OutlinerPool.
        SfxItemPool::Free(pOutlPool);
    }

    if( mpForbiddenCharactersTable )
        mpForbiddenCharactersTable->release();

    delete mpNumberFormatter;

    delete mpImpl->mpUndoFactory;
    delete mpImpl;
}

void SdrModel::SetSwapGraphics( bool bSwap )
{
    bSwapGraphics = bSwap;
}

bool SdrModel::IsReadOnly() const
{
    return bReadOnly;
}

void SdrModel::SetReadOnly(bool bYes)
{
    bReadOnly=bYes;
}


void SdrModel::SetMaxUndoActionCount(sal_uIntPtr nCount)
{
    if (nCount<1) nCount=1;
    nMaxUndoCount=nCount;
    if (pUndoStack!=nullptr) {
        while (pUndoStack->size()>nMaxUndoCount) {
            delete pUndoStack->back();
            pUndoStack->pop_back();
        }
    }
}

void SdrModel::ClearUndoBuffer()
{
    if (pUndoStack!=nullptr) {
        while (!pUndoStack->empty()) {
            delete pUndoStack->back();
            pUndoStack->pop_back();
        }
        delete pUndoStack;
        pUndoStack=nullptr;
    }
    if (pRedoStack!=nullptr) {
        while (!pRedoStack->empty()) {
            delete pRedoStack->back();
            pRedoStack->pop_back();
        }
        delete pRedoStack;
        pRedoStack=nullptr;
    }
}

bool SdrModel::HasUndoActions() const
{
    return pUndoStack && !pUndoStack->empty();
}

bool SdrModel::HasRedoActions() const
{
    return pRedoStack && !pRedoStack->empty();
}

bool SdrModel::Undo()
{
    bool bRet = false;
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Undo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo = HasUndoActions() ? pUndoStack->front() : nullptr;
        if(pDo!=nullptr)
        {
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Undo();
            if(pRedoStack==nullptr)
                pRedoStack=new std::deque<SfxUndoAction*>;
            SfxUndoAction* p = pUndoStack->front();
            pUndoStack->pop_front();
            pRedoStack->push_front(p);
            mbUndoEnabled = bWasUndoEnabled;
        }
    }
    return bRet;
}

bool SdrModel::Redo()
{
    bool bRet = false;
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo = HasRedoActions() ? pRedoStack->front() : nullptr;
        if(pDo!=nullptr)
        {
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Redo();
            if(pUndoStack==nullptr)
                pUndoStack=new std::deque<SfxUndoAction*>;
            SfxUndoAction* p = pRedoStack->front();
            pRedoStack->pop_front();
            pUndoStack->push_front(p);
            mbUndoEnabled = bWasUndoEnabled;
        }
    }
    return bRet;
}

bool SdrModel::Repeat(SfxRepeatTarget& rView)
{
    bool bRet = false;
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo = HasUndoActions() ? pUndoStack->front() : nullptr;
        if(pDo!=nullptr)
        {
            if(pDo->CanRepeat(rView))
            {
                pDo->Repeat(rView);
                bRet = true;
            }
        }
    }
    return bRet;
}

void SdrModel::ImpPostUndoAction(SdrUndoAction* pUndo)
{
    DBG_ASSERT( mpImpl->mpUndoManager == nullptr, "svx::SdrModel::ImpPostUndoAction(), method not supported with application undo manager!" );
    if( IsUndoEnabled() )
    {
        if (aUndoLink.IsSet())
        {
            aUndoLink.Call(pUndo);
        }
        else
        {
            if (pUndoStack==nullptr)
                pUndoStack=new std::deque<SfxUndoAction*>;
            pUndoStack->push_front(pUndo);
            while (pUndoStack->size()>nMaxUndoCount)
            {
                delete pUndoStack->back();
                pUndoStack->pop_back();
            }
            if (pRedoStack!=nullptr)
                pRedoStack->clear();
        }
    }
    else
    {
        delete pUndo;
    }
}

void SdrModel::BegUndo()
{
    if( mpImpl->mpUndoManager )
    {
        mpImpl->mpUndoManager->EnterListAction("","");
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        if(pAktUndoGroup==nullptr)
        {
            pAktUndoGroup = new SdrUndoGroup(*this);
            nUndoLevel=1;
        }
        else
        {
            nUndoLevel++;
        }
    }
}

void SdrModel::BegUndo(const OUString& rComment)
{
    if( mpImpl->mpUndoManager )
    {
        mpImpl->mpUndoManager->EnterListAction( rComment, "" );
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (nUndoLevel==1)
        {
            pAktUndoGroup->SetComment(rComment);
        }
    }
}

void SdrModel::BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc)
{
    if( mpImpl->mpUndoManager )
    {
        OUString aComment(rComment);
        if( !aComment.isEmpty() && !rObjDescr.isEmpty() )
        {
            aComment = aComment.replaceFirst("%1", rObjDescr);
        }
        mpImpl->mpUndoManager->EnterListAction( aComment,"" );
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (nUndoLevel==1)
        {
            pAktUndoGroup->SetComment(rComment);
            pAktUndoGroup->SetObjDescription(rObjDescr);
            pAktUndoGroup->SetRepeatFunction(eFunc);
        }
    }
}

void SdrModel::EndUndo()
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::EndUndo(): UndoLevel is already 0!");
    if( mpImpl->mpUndoManager )
    {
        if( nUndoLevel )
        {
            nUndoLevel--;
            mpImpl->mpUndoManager->LeaveListAction();
        }
    }
    else
    {
        if(pAktUndoGroup!=nullptr && IsUndoEnabled())
        {
            nUndoLevel--;
            if(nUndoLevel==0)
            {
                if(pAktUndoGroup->GetActionCount()!=0)
                {
                    SdrUndoAction* pUndo=pAktUndoGroup;
                    pAktUndoGroup=nullptr;
                    ImpPostUndoAction(pUndo);
                }
                else
                {
                    // was empty
                    delete pAktUndoGroup;
                    pAktUndoGroup=nullptr;
                }
            }
        }
    }
}

void SdrModel::SetUndoComment(const OUString& rComment)
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel is already 0!");

    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else if( IsUndoEnabled() )
    {
        if(nUndoLevel==1)
        {
            pAktUndoGroup->SetComment(rComment);
        }
    }
}

void SdrModel::SetUndoComment(const OUString& rComment, const OUString& rObjDescr)
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel is already 0!");
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else
    {
        if (nUndoLevel==1)
        {
            pAktUndoGroup->SetComment(rComment);
            pAktUndoGroup->SetObjDescription(rObjDescr);
        }
    }
}

void SdrModel::AddUndo(SdrUndoAction* pUndo)
{
    if( mpImpl->mpUndoManager )
    {
        mpImpl->mpUndoManager->AddUndoAction( pUndo );
    }
    else if( !IsUndoEnabled() )
    {
        delete pUndo;
    }
    else
    {
        if (pAktUndoGroup!=nullptr)
        {
            pAktUndoGroup->AddAction(pUndo);
        }
        else
        {
            ImpPostUndoAction(pUndo);
        }
    }
}

void SdrModel::EnableUndo( bool bEnable )
{
    if( mpImpl->mpUndoManager )
    {
        mpImpl->mpUndoManager->EnableUndo( bEnable );
    }
    else
    {
        mbUndoEnabled = bEnable;
    }
}

bool SdrModel::IsUndoEnabled() const
{
    if( mpImpl->mpUndoManager )
    {
        return mpImpl->mpUndoManager->IsUndoEnabled();
    }
    else
    {
        return mbUndoEnabled;
    }
}

void SdrModel::ImpCreateTables()
{
    for( int i = 0; i < XPROPERTY_LIST_COUNT; i++ )
    {
        if( !bExtColorTable || i != XCOLOR_LIST )
            maProperties[i] = XPropertyList::CreatePropertyList (
                (XPropertyListType) i, aTablePath, ""/*TODO?*/ );
    }
}

void SdrModel::ClearModel(bool bCalledFromDestructor)
{
    if(bCalledFromDestructor)
    {
        mbInDestruction = true;
    }

    sal_Int32 i;
    // delete all drawing pages
    sal_Int32 nCount=GetPageCount();
    for (i=nCount-1; i>=0; i--)
    {
        DeletePage( (sal_uInt16)i );
    }
    maPages.clear();
    PageListChanged();

    // delete all Masterpages
    nCount=GetMasterPageCount();
    for(i=nCount-1; i>=0; i--)
    {
        DeleteMasterPage( (sal_uInt16)i );
    }
    maMaPag.clear();
    MasterPageListChanged();

    pLayerAdmin->ClearLayer();
}

SdrModel* SdrModel::AllocModel() const
{
    SdrModel* pModel=new SdrModel;
    pModel->SetScaleUnit(eObjUnit,aObjUnit);
    return pModel;
}

SdrPage* SdrModel::AllocPage(bool bMasterPage)
{
    return new SdrPage(*this,bMasterPage);
}

void SdrModel::SetTextDefaults() const
{
    SetTextDefaults( pItemPool, nDefTextHgt );
}

void SdrModel::SetTextDefaults( SfxItemPool* pItemPool, sal_uIntPtr nDefTextHgt )
{
    // set application-language specific dynamic pool language defaults
    SvxFontItem aSvxFontItem( EE_CHAR_FONTINFO) ;
    SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
    SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
    sal_uInt16 nLanguage;
    if (!utl::ConfigManager::IsAvoidConfig())
        nLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    else
        nLanguage = LANGUAGE_ENGLISH_US;

    // get DEFAULTFONT_LATIN_TEXT and set at pool as dynamic default
    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::LATIN_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItem.SetFamily(aFont.GetFamily());
    aSvxFontItem.SetFamilyName(aFont.GetFamilyName());
    aSvxFontItem.SetStyleName(OUString());
    aSvxFontItem.SetPitch( aFont.GetPitch());
    aSvxFontItem.SetCharSet( aFont.GetCharSet() );
    pItemPool->SetPoolDefaultItem(aSvxFontItem);

    // get DEFAULTFONT_CJK_TEXT and set at pool as dynamic default
    vcl::Font aFontCJK(OutputDevice::GetDefaultFont(DefaultFontType::CJK_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCJK.SetFamily( aFontCJK.GetFamily());
    aSvxFontItemCJK.SetFamilyName(aFontCJK.GetFamilyName());
    aSvxFontItemCJK.SetStyleName(OUString());
    aSvxFontItemCJK.SetPitch( aFontCJK.GetPitch());
    aSvxFontItemCJK.SetCharSet( aFontCJK.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCJK);

    // get DEFAULTFONT_CTL_TEXT and set at pool as dynamic default
    vcl::Font aFontCTL(OutputDevice::GetDefaultFont(DefaultFontType::CTL_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCTL.SetFamily(aFontCTL.GetFamily());
    aSvxFontItemCTL.SetFamilyName(aFontCTL.GetFamilyName());
    aSvxFontItemCTL.SetStyleName(OUString());
    aSvxFontItemCTL.SetPitch( aFontCTL.GetPitch() );
    aSvxFontItemCTL.SetCharSet( aFontCTL.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCTL);

    // set dynamic FontHeight defaults
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CTL ) );

    // set FontColor defaults
    pItemPool->SetPoolDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor(), EE_CHAR_COLOR) );
}

SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
{
    pDrawOutliner->SetTextObj(pObj);
    return *pDrawOutliner;
}

SdrOutliner& SdrModel::GetChainingOutliner(const SdrTextObj* pObj) const
{
    pChainingOutliner->SetTextObj(pObj);
    return *pChainingOutliner;
}

const SdrTextObj* SdrModel::GetFormattingTextObj() const
{
    if (pDrawOutliner!=nullptr) {
        return pDrawOutliner->GetTextObj();
    }
    return nullptr;
}

void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, bool bInit )
{
    // Initialization of the Outliners for drawing text and HitTest
    if( bInit )
    {
        pOutliner->EraseVirtualDevice();
        pOutliner->SetUpdateMode(false);
        pOutliner->SetEditTextObjectPool(pItemPool);
        pOutliner->SetDefTab(nDefaultTabulator);
    }

    pOutliner->SetRefDevice(GetRefDevice());
    Outliner::SetForbiddenCharsTable(GetForbiddenCharsTable());
    pOutliner->SetAsianCompressionMode( mnCharCompressType );
    pOutliner->SetKernAsianPunctuation( IsKernAsianPunctuation() );
    pOutliner->SetAddExtLeading( IsAddExtLeading() );

    if ( !GetRefDevice() )
    {
        MapMode aMapMode(eObjUnit, Point(0,0), aObjUnit, aObjUnit);
        pOutliner->SetRefMapMode(aMapMode);
    }
}

void SdrModel::SetRefDevice(OutputDevice* pDev)
{
    pRefOutDev=pDev;
    ImpSetOutlinerDefaults( pDrawOutliner );
    ImpSetOutlinerDefaults( pHitTestOutliner );
    RefDeviceChanged();
}

void SdrModel::registerLibreOfficeKitCallback(LibreOfficeKitCallback pCallback, void* pData)
{
    mpLibreOfficeKitCallback = pCallback;
    mpLibreOfficeKitData = pData;
}

void SdrModel::libreOfficeKitCallback(int nType, const char* pPayload) const
{
    // Per-view callbacks should always invoke SfxViewShell::libreOfficeKitViewCallback().
    assert(!comphelper::LibreOfficeKit::isViewCallback());

    if (mbTiledSearching)
    {
        switch (nType)
        {
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
            return;
        }
    }

    if (mpLibreOfficeKitCallback)
        mpLibreOfficeKitCallback(nType, pPayload, mpLibreOfficeKitData);
}

void SdrModel::setTiledSearching(bool bTiledSearching)
{
    assert(!comphelper::LibreOfficeKit::isViewCallback());
    mbTiledSearching = bTiledSearching;
}

bool SdrModel::isTiledSearching() const
{
    assert(!comphelper::LibreOfficeKit::isViewCallback());
    return mbTiledSearching;
}

void* SdrModel::getLibreOfficeKitData() const
{
    return mpLibreOfficeKitData;
}

void SdrModel::ImpReformatAllTextObjects()
{
    if( isLocked() )
        return;

    sal_uInt16 nCount=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nCount; nNum++) {
        GetMasterPage(nNum)->ReformatAllTextObjects();
    }
    nCount=GetPageCount();
    for (nNum=0; nNum<nCount; nNum++) {
        GetPage(nNum)->ReformatAllTextObjects();
    }
}

/*  steps over all available pages and sends notify messages to
    all edge objects that are connected to other objects so that
    they may reposition themselves
*/
void SdrModel::ImpReformatAllEdgeObjects()
{
    if( isLocked() )
        return;

    sal_uInt16 nCount=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nCount; nNum++)
    {
        GetMasterPage(nNum)->ReformatAllEdgeObjects();
    }
    nCount=GetPageCount();
    for (nNum=0; nNum<nCount; nNum++)
    {
        GetPage(nNum)->ReformatAllEdgeObjects();
    }
}

uno::Reference<embed::XStorage> SdrModel::GetDocumentStorage() const
{
    uno::Reference<document::XStorageBasedDocument> const xSBD(
            const_cast<SdrModel*>(this)->getUnoModel(), uno::UNO_QUERY);
    if (!xSBD.is())
    {
        SAL_WARN("svx", "no UNO model");
        return nullptr;
    }
    return xSBD->getDocumentStorage();
}

uno::Reference<io::XInputStream>
SdrModel::GetDocumentStream( OUString const& rURL,
                ::comphelper::LifecycleProxy & rProxy) const
{
    uno::Reference<embed::XStorage> const xStorage(GetDocumentStorage());
    if (!xStorage.is())
    {
        SAL_WARN("svx", "no storage?");
        return nullptr;
    }
    try {
        uno::Reference<io::XStream> const xStream(
            ::comphelper::OStorageHelper::GetStreamAtPackageURL(
                xStorage, rURL, embed::ElementModes::READ, rProxy));
        return (xStream.is()) ? xStream->getInputStream() : nullptr;
    }
    catch (container::NoSuchElementException const&)
    {
        SAL_INFO("svx", "not found");
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("svx", "exception: '" << e.Message << "'");
    }
    return nullptr;
}

// convert template attributes from the string into "hard" attributes
void SdrModel::BurnInStyleSheetAttributes()
{
    sal_uInt16 nCount=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nCount; nNum++) {
        GetMasterPage(nNum)->BurnInStyleSheetAttributes();
    }
    nCount=GetPageCount();
    for (nNum=0; nNum<nCount; nNum++) {
        GetPage(nNum)->BurnInStyleSheetAttributes();
    }
}

void SdrModel::RefDeviceChanged()
{
    Broadcast(SdrHint(HINT_REFDEVICECHG));
    ImpReformatAllTextObjects();
}

void SdrModel::SetDefaultFontHeight(sal_uIntPtr nVal)
{
    if (nVal!=nDefTextHgt) {
        nDefTextHgt=nVal;
        Broadcast(SdrHint(HINT_DEFFONTHGTCHG));
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetDefaultTabulator(sal_uInt16 nVal)
{
    if (nDefaultTabulator!=nVal) {
        nDefaultTabulator=nVal;
        Outliner& rOutliner=GetDrawOutliner();
        rOutliner.SetDefTab(nVal);
        Broadcast(SdrHint(HINT_DEFAULTTABCHG));
        ImpReformatAllTextObjects();
    }
}

void SdrModel::ImpSetUIUnit()
{
    if(0 == aUIScale.GetNumerator() || 0 == aUIScale.GetDenominator())
    {
        aUIScale = Fraction(1,1);
    }

    // set start values
    nUIUnitKomma = 0;
    sal_Int64 nMul(1);
    sal_Int64 nDiv(1);

    // normalize on meters resp. inch
    switch (eObjUnit)
    {
        case MAP_100TH_MM   : nUIUnitKomma+=5; break;
        case MAP_10TH_MM    : nUIUnitKomma+=4; break;
        case MAP_MM         : nUIUnitKomma+=3; break;
        case MAP_CM         : nUIUnitKomma+=2; break;
        case MAP_1000TH_INCH: nUIUnitKomma+=3; break;
        case MAP_100TH_INCH : nUIUnitKomma+=2; break;
        case MAP_10TH_INCH  : nUIUnitKomma+=1; break;
        case MAP_INCH       : nUIUnitKomma+=0; break;
        case MAP_POINT      : nDiv=72;     break;          // 1Pt   = 1/72"
        case MAP_TWIP       : nDiv=144; nUIUnitKomma++; break; // 1Twip = 1/1440"
        case MAP_PIXEL      : break;
        case MAP_SYSFONT    : break;
        case MAP_APPFONT    : break;
        case MAP_RELATIVE   : break;
        default: break;
    } // switch

    // 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
    // 1 furlong = 10 chains  =  7.920" =   201.168,0mm
    // 1 chain   =  4 poles   =    792" =    20.116,8mm
    // 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
    // 1 yd      =  3 ft      =     36" =       914,4mm
    // 1 ft      = 12 "       =      1" =       304,8mm
    switch (eUIUnit)
    {
        case FUNIT_NONE   : break;
        // metric
        case FUNIT_100TH_MM: nUIUnitKomma-=5; break;
        case FUNIT_MM     : nUIUnitKomma-=3; break;
        case FUNIT_CM     : nUIUnitKomma-=2; break;
        case FUNIT_M      : nUIUnitKomma+=0; break;
        case FUNIT_KM     : nUIUnitKomma+=3; break;
        // Inch
        case FUNIT_TWIP   : nMul=144; nUIUnitKomma--;  break;  // 1Twip = 1/1440"
        case FUNIT_POINT  : nMul=72;     break;            // 1Pt   = 1/72"
        case FUNIT_PICA   : nMul=6;      break;            // 1Pica = 1/6"
        case FUNIT_INCH   : break;                         // 1"    = 1"
        case FUNIT_FOOT   : nDiv*=12;    break;            // 1Ft   = 12"
        case FUNIT_MILE   : nDiv*=6336; nUIUnitKomma++; break; // 1mile = 63360"
        // other
        case FUNIT_CUSTOM : break;
        case FUNIT_PERCENT: nUIUnitKomma+=2; break;
        // TODO: Add code to handle the following if needed (added to remove warning)
        case FUNIT_CHAR   : break;
        case FUNIT_LINE   : break;
        case FUNIT_PIXEL  : break;
        case FUNIT_DEGREE : break;
        case FUNIT_SECOND : break;
        case FUNIT_MILLISECOND : break;
    } // switch

    // check if mapping is from metric to inch and adapt
    const bool bMapInch(IsInch(eObjUnit));
    const bool bUIMetr(IsMetric(eUIUnit));

    if (bMapInch && bUIMetr)
    {
        nUIUnitKomma += 4;
        nMul *= 254;
    }

    // check if mapping is from inch to metric and adapt
    const bool bMapMetr(IsMetric(eObjUnit));
    const bool bUIInch(IsInch(eUIUnit));

    if (bMapMetr && bUIInch)
    {
        nUIUnitKomma -= 4;
        nDiv *= 254;
    }

    // use temporary fraction for reduction (fallback to 32bit here),
    // may need to be changed in the future, too
    if(1 != nMul || 1 != nDiv)
    {
        const Fraction aTemp(static_cast< long >(nMul), static_cast< long >(nDiv));
        nMul = aTemp.GetNumerator();
        nDiv = aTemp.GetDenominator();
    }

    // #i89872# take Unit of Measurement into account
    if(1 != aUIScale.GetDenominator() || 1 != aUIScale.GetNumerator())
    {
        // divide by UIScale
        nMul *= aUIScale.GetDenominator();
        nDiv *= aUIScale.GetNumerator();
    }

    // shorten trailing zeros for dividend
    while(0 == (nMul % 10))
    {
        nUIUnitKomma--;
        nMul /= 10;
    }

    // shorten trailing zeros for divisor
    while(0 == (nDiv % 10))
    {
        nUIUnitKomma++;
        nDiv /= 10;
    }

    // end preparations, set member values
    aUIUnitFact = Fraction(sal_Int32(nMul), sal_Int32(nDiv));
    bUIOnlyKomma = (nMul == nDiv);
    TakeUnitStr(eUIUnit, aUIUnitStr);
}

void SdrModel::SetScaleUnit(MapUnit eMap, const Fraction& rFrac)
{
    if (eObjUnit!=eMap || aObjUnit!=rFrac) {
        eObjUnit=eMap;
        aObjUnit=rFrac;
        pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetScaleUnit(MapUnit eMap)
{
    if (eObjUnit!=eMap) {
        eObjUnit=eMap;
        pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetScaleFraction(const Fraction& rFrac)
{
    if (aObjUnit!=rFrac) {
        aObjUnit=rFrac;
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit)
{
    if (eUIUnit!=eUnit) {
        eUIUnit=eUnit;
        ImpSetUIUnit();
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIScale(const Fraction& rScale)
{
    if (aUIScale!=rScale) {
        aUIScale=rScale;
        ImpSetUIUnit();
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit, const Fraction& rScale)
{
    if (eUIUnit!=eUnit || aUIScale!=rScale) {
        eUIUnit=eUnit;
        aUIScale=rScale;
        ImpSetUIUnit();
        ImpReformatAllTextObjects();
    }
}

void SdrModel::TakeUnitStr(FieldUnit eUnit, OUString& rStr)
{
    switch(eUnit)
    {
        default:
        case FUNIT_NONE   :
        case FUNIT_CUSTOM :
        {
            rStr.clear();
            break;
        }
        case FUNIT_100TH_MM:
        {
            rStr = "/100mm";
            break;
        }
        case FUNIT_MM     :
        {
            rStr = "mm";
            break;
        }
        case FUNIT_CM     :
        {
            rStr = "cm";
            break;
        }
        case FUNIT_M      :
        {
            rStr = "m";
            break;
        }
        case FUNIT_KM     :
        {
            rStr ="km";
            break;
        }
        case FUNIT_TWIP   :
        {
            rStr = "twip";
            break;
        }
        case FUNIT_POINT  :
        {
            rStr = "pt";
            break;
        }
        case FUNIT_PICA   :
        {
            rStr = "pica";
            break;
        }
        case FUNIT_INCH   :
        {
            rStr = "\"";
            break;
        }
        case FUNIT_FOOT   :
        {
            rStr = "ft";
            break;
        }
        case FUNIT_MILE   :
        {
            rStr = "mile(s)";
            break;
        }
        case FUNIT_PERCENT:
        {
            rStr = "%";
            break;
        }
    }
}

void SdrModel::TakeMetricStr(long nVal, OUString& rStr, bool bNoUnitChars, sal_Int32 nNumDigits) const
{
    // #i22167#
    // change to double precision usage to not lose decimal places
    const bool bNegative(nVal < 0L);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc(aSysLoc.GetLocaleData());
    double fLocalValue(double(nVal) * double(aUIUnitFact));

    if(bNegative)
    {
        fLocalValue = -fLocalValue;
    }

    if( -1 == nNumDigits )
    {
        nNumDigits = LocaleDataWrapper::getNumDigits();
    }

    sal_Int32 nKomma(nUIUnitKomma);

    if(nKomma > nNumDigits)
    {
        const sal_Int32 nDiff(nKomma - nNumDigits);
        const double fFactor(pow(10.0, static_cast<const int>(nDiff)));

        fLocalValue /= fFactor;
        nKomma = nNumDigits;
    }
    else if(nKomma < nNumDigits)
    {
        const sal_Int32 nDiff(nNumDigits - nKomma);
        const double fFactor(pow(10.0, static_cast<const int>(nDiff)));

        fLocalValue *= fFactor;
        nKomma = nNumDigits;
    }

    OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(fLocalValue + 0.5));

    if(nKomma < 0)
    {
        // negative nKomma (decimal point) means: add zeros
        sal_Int32 nCount(-nKomma);

        for(sal_Int32 i=0; i<nCount; i++)
            aBuf.append('0');

        nKomma = 0;
    }

    // the second condition needs to be <= since inside this loop
    // also the leading zero is inserted.
    if (nKomma > 0 && aBuf.getLength() <= nKomma)
    {
        // if necessary, add zeros before the decimal point
        sal_Int32 nCount = nKomma - aBuf.getLength();

        if(nCount >= 0 && LocaleDataWrapper::isNumLeadingZero())
            nCount++;

        for(sal_Int32 i=0; i<nCount; i++)
            aBuf.insert(0, '0');
    }

    sal_Unicode cDec( rLoc.getNumDecimalSep()[0] );

    // insert KommaChar (decimal point character)
    sal_Int32 nVorKomma = aBuf.getLength() - nKomma;

    if(nKomma > 0)
        aBuf.insert(nVorKomma, cDec);

    if(!LocaleDataWrapper::isNumTrailingZeros())
    {
        // Remove all trailing zeros.
        while (!aBuf.isEmpty() && aBuf[aBuf.getLength()-1] == '0')
            aBuf.remove(aBuf.getLength()-1, 1);

        // Remove decimal if it's the last character.
        if (!aBuf.isEmpty() && aBuf[aBuf.getLength()-1] == cDec)
            aBuf.remove(aBuf.getLength()-1, 1);
    }

    // if necessary, add separators before every third digit
    if( nVorKomma > 3 )
    {
        OUString aThoSep( rLoc.getNumThousandSep() );
        if ( !aThoSep.isEmpty() )
        {
            sal_Unicode cTho( aThoSep[0] );
            sal_Int32 i(nVorKomma - 3);

            while(i > 0)
            {
                aBuf.insert(i, cTho);
                i -= 3;
            }
        }
    }

    if (aBuf.isEmpty())
        aBuf.append("0");

    if(bNegative)
    {
        aBuf.insert(0, "-");
    }

    if(!bNoUnitChars)
        aBuf.append(aUIUnitStr);

    rStr = aBuf.makeStringAndClear();
}

void SdrModel::TakeAngleStr(long nAngle, OUString& rStr, bool bNoDegChar)
{
    bool bNeg = nAngle < 0;

    if(bNeg)
        nAngle = -nAngle;

    OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(nAngle));

    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();
    sal_Int32 nCount = 2;

    if(LocaleDataWrapper::isNumLeadingZero())
        nCount++;

    while(aBuf.getLength() < nCount)
        aBuf.insert(0, '0');

    aBuf.insert(aBuf.getLength()-2, rLoc.getNumDecimalSep()[0]);

    if(bNeg)
        aBuf.insert(0, '-');

    if(!bNoDegChar)
        aBuf.append(DEGREE_CHAR);

    rStr = aBuf.makeStringAndClear();
}

void SdrModel::TakePercentStr(const Fraction& rVal, OUString& rStr, bool bNoPercentChar)
{
    sal_Int32 nMul(rVal.GetNumerator());
    sal_Int32 nDiv(rVal.GetDenominator());
    bool bNeg(nMul < 0);

    if(nDiv < 0)
        bNeg = !bNeg;

    if(nMul < 0)
        nMul = -nMul;

    if(nDiv < 0)
        nDiv = -nDiv;

    nMul *= 100;
    nMul += nDiv/2;
    nMul /= nDiv;

    rStr = OUString::number(nMul);

    if(bNeg)
        rStr = "-" + rStr;

    if(!bNoPercentChar)
        rStr += "%";
}

void SdrModel::SetChanged(bool bFlg)
{
    mbChanged = bFlg;
}

void SdrModel::RecalcPageNums(bool bMaster)
{
    if(bMaster)
    {
        sal_uInt16 nCount=sal_uInt16(maMaPag.size());
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            SdrPage* pPg=maMaPag[i];
            pPg->SetPageNum(i);
        }
        bMPgNumsDirty=false;
    }
    else
    {
        sal_uInt16 nCount=sal_uInt16(maPages.size());
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            SdrPage* pPg=maPages[i];
            pPg->SetPageNum(i);
        }
        bPagNumsDirty=false;
    }
}

void SdrModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nCount=GetPageCount();
    if (nPos>nCount) nPos=nCount;
    maPages.insert(maPages.begin()+nPos,pPage);
    PageListChanged();
    pPage->SetInserted();
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nCount) bPagNumsDirty=true;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPage);
    Broadcast(aHint);
}

void SdrModel::DeletePage(sal_uInt16 nPgNum)
{
    SdrPage* pPg=RemovePage(nPgNum);
    delete pPg;
}

SdrPage* SdrModel::RemovePage(sal_uInt16 nPgNum)
{
    SdrPage* pPg=maPages[nPgNum];
    maPages.erase(maPages.begin()+nPgNum);
    PageListChanged();
    if (pPg!=nullptr) {
        pPg->SetInserted(false);
    }
    bPagNumsDirty=true;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
    return pPg;
}

void SdrModel::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    SdrPage* pPg=maPages[nPgNum];
    if (pPg!=nullptr) {
        maPages.erase(maPages.begin()+nPgNum); // shortcut to avoid two broadcasts
        PageListChanged();
        pPg->SetInserted(false);
        InsertPage(pPg,nNewPos);
    }
    else
        RemovePage(nPgNum);
}

void SdrModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nCount=GetMasterPageCount();
    if (nPos>nCount) nPos=nCount;
    maMaPag.insert(maMaPag.begin()+nPos,pPage);
    MasterPageListChanged();
    pPage->SetInserted();
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nCount) {
        bMPgNumsDirty=true;
    }
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPage);
    Broadcast(aHint);
}

void SdrModel::DeleteMasterPage(sal_uInt16 nPgNum)
{
    SdrPage* pPg=RemoveMasterPage(nPgNum);
    delete pPg;
}

SdrPage* SdrModel::RemoveMasterPage(sal_uInt16 nPgNum)
{
    SdrPage* pRetPg=maMaPag[nPgNum];
    maMaPag.erase(maMaPag.begin()+nPgNum);
    MasterPageListChanged();

    if(pRetPg)
    {
        // Now delete the links from the normal drawing pages to the deleted master page.
        sal_uInt16 nPageAnz(GetPageCount());

        for(sal_uInt16 np(0); np < nPageAnz; np++)
        {
            GetPage(np)->TRG_ImpMasterPageRemoved(*pRetPg);
        }

        pRetPg->SetInserted(false);
    }

    bMPgNumsDirty=true;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pRetPg);
    Broadcast(aHint);
    return pRetPg;
}

void SdrModel::MoveMasterPage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    SdrPage* pPg=maMaPag[nPgNum];
    maMaPag.erase(maMaPag.begin()+nPgNum);
    MasterPageListChanged();
    if (pPg!=nullptr) {
        pPg->SetInserted(false);
        maMaPag.insert(maMaPag.begin()+nNewPos,pPg);
        MasterPageListChanged();
    }
    bMPgNumsDirty=true;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
}


void SdrModel::CopyPages(sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                         sal_uInt16 nDestPos,
                         bool bUndo, bool bMoveNoCopy)
{
    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

    if( bUndo )
        BegUndo(ImpGetResStr(STR_UndoMergeModel));

    sal_uInt16 nPageAnz=GetPageCount();
    sal_uInt16 nMaxPage=nPageAnz;

    if (nMaxPage!=0)
        nMaxPage--;
    if (nFirstPageNum>nMaxPage)
        nFirstPageNum=nMaxPage;
    if (nLastPageNum>nMaxPage)
        nLastPageNum =nMaxPage;
    bool bReverse=nLastPageNum<nFirstPageNum;
    if (nDestPos>nPageAnz)
        nDestPos=nPageAnz;

    // at first, save the pointers of the affected pages in an array
    sal_uInt16 nPageNum=nFirstPageNum;
    sal_uInt16 nCopyAnz=((!bReverse)?(nLastPageNum-nFirstPageNum):(nFirstPageNum-nLastPageNum))+1;
    std::unique_ptr<SdrPage*[]> pPagePtrs(new SdrPage*[nCopyAnz]);
    sal_uInt16 nCopyNum;
    for(nCopyNum=0; nCopyNum<nCopyAnz; nCopyNum++)
    {
        pPagePtrs[nCopyNum]=GetPage(nPageNum);
        if (bReverse)
            nPageNum--;
        else
            nPageNum++;
    }

    // now copy the pages
    sal_uInt16 nDestNum=nDestPos;
    for (nCopyNum=0; nCopyNum<nCopyAnz; nCopyNum++)
    {
        SdrPage* pPg=pPagePtrs[nCopyNum];
        sal_uInt16 nPageNum2=pPg->GetPageNum();
        if (!bMoveNoCopy)
        {
            const SdrPage* pPg1=GetPage(nPageNum2);
            pPg=pPg1->Clone();
            InsertPage(pPg,nDestNum);
            if (bUndo)
                AddUndo(GetSdrUndoFactory().CreateUndoCopyPage(*pPg));
            nDestNum++;
        }
        else
        {
            // TODO: Move is untested!
            if (nDestNum>nPageNum2)
                nDestNum--;

            if(bUndo)
                AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*GetPage(nPageNum2),nPageNum2,nDestNum));

            pPg=RemovePage(nPageNum2);
            InsertPage(pPg,nDestNum);
            nDestNum++;
        }

        if(bReverse)
            nPageNum2--;
        else
            nPageNum2++;
    }

    pPagePtrs.reset();
    if(bUndo)
        EndUndo();
}

void SdrModel::Merge(SdrModel& rSourceModel,
                     sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                     sal_uInt16 nDestPos,
                     bool bMergeMasterPages, bool bAllMasterPages,
                     bool bUndo, bool bTreadSourceAsConst)
{
    if (&rSourceModel==this)
    {
        CopyPages(nFirstPageNum,nLastPageNum,nDestPos,bUndo,!bTreadSourceAsConst);
        return;
    }

    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

    if (bUndo)
        BegUndo(ImpGetResStr(STR_UndoMergeModel));

    sal_uInt16 nSrcPageAnz=rSourceModel.GetPageCount();
    sal_uInt16 nSrcMasterPageAnz=rSourceModel.GetMasterPageCount();
    sal_uInt16 nDstMasterPageAnz=GetMasterPageCount();
    bool bInsPages=(nFirstPageNum<nSrcPageAnz || nLastPageNum<nSrcPageAnz);
    sal_uInt16 nMaxSrcPage=nSrcPageAnz; if (nMaxSrcPage!=0) nMaxSrcPage--;
    if (nFirstPageNum>nMaxSrcPage) nFirstPageNum=nMaxSrcPage;
    if (nLastPageNum>nMaxSrcPage)  nLastPageNum =nMaxSrcPage;
    bool bReverse=nLastPageNum<nFirstPageNum;

    std::unique_ptr<sal_uInt16[]> pMasterMap;
    std::unique_ptr<bool[]> pMasterNeed;
    sal_uInt16    nMasterNeed=0;
    if (bMergeMasterPages && nSrcMasterPageAnz!=0) {
        // determine which MasterPages from rSrcModel we need
        pMasterMap.reset(new sal_uInt16[nSrcMasterPageAnz]);
        pMasterNeed.reset(new bool[nSrcMasterPageAnz]);
        memset(pMasterMap.get(),0xFF,nSrcMasterPageAnz*sizeof(sal_uInt16));
        if (bAllMasterPages) {
            memset(pMasterNeed.get(), true, nSrcMasterPageAnz * sizeof(bool));
        } else {
            memset(pMasterNeed.get(), false, nSrcMasterPageAnz * sizeof(bool));
            sal_uInt16 nAnf= bReverse ? nLastPageNum : nFirstPageNum;
            sal_uInt16 nEnd= bReverse ? nFirstPageNum : nLastPageNum;
            for (sal_uInt16 i=nAnf; i<=nEnd; i++) {
                const SdrPage* pPg=rSourceModel.GetPage(i);
                if(pPg->TRG_HasMasterPage())
                {
                    SdrPage& rMasterPage = pPg->TRG_GetMasterPage();
                    sal_uInt16 nMPgNum(rMasterPage.GetPageNum());

                    if(nMPgNum < nSrcMasterPageAnz)
                    {
                        pMasterNeed[nMPgNum] = true;
                    }
                }
            }
        }
        // now determine the Mapping of the MasterPages
        sal_uInt16 nAktMaPagNum=nDstMasterPageAnz;
        for (sal_uInt16 i=0; i<nSrcMasterPageAnz; i++) {
            if (pMasterNeed[i]) {
                pMasterMap[i]=nAktMaPagNum;
                nAktMaPagNum++;
                nMasterNeed++;
            }
        }
    }

    // get the MasterPages
    if (pMasterMap && pMasterNeed && nMasterNeed!=0) {
        for (sal_uInt16 i=nSrcMasterPageAnz; i>0;) {
            i--;
            if (pMasterNeed[i]) {
                SdrPage* pPg=nullptr;
                if (bTreadSourceAsConst) {
                    const SdrPage* pPg1=rSourceModel.GetMasterPage(i);
                    pPg=pPg1->Clone();
                } else {
                    pPg=rSourceModel.RemoveMasterPage(i);
                }
                if (pPg!=nullptr) {
                    // Now append all of them to the end of the DstModel.
                    // Don't use InsertMasterPage(), because everything is
                    // inconsistent until all are in.
                    maMaPag.insert(maMaPag.begin()+nDstMasterPageAnz, pPg);
                    MasterPageListChanged();
                    pPg->SetInserted();
                    pPg->SetModel(this);
                    bMPgNumsDirty=true;
                    if (bUndo) AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pPg));
                } else {
                    OSL_FAIL("SdrModel::Merge(): MasterPage not found in SourceModel.");
                }
            }
        }
    }

    // get the drawing pages
    if (bInsPages) {
        sal_uInt16 nSourcePos=nFirstPageNum;
        sal_uInt16 nMergeCount=sal_uInt16(std::abs((long)((long)nFirstPageNum-nLastPageNum))+1);
        if (nDestPos>GetPageCount()) nDestPos=GetPageCount();
        while (nMergeCount>0) {
            SdrPage* pPg=nullptr;
            if (bTreadSourceAsConst) {
                const SdrPage* pPg1=rSourceModel.GetPage(nSourcePos);
                pPg=pPg1->Clone();
            } else {
                pPg=rSourceModel.RemovePage(nSourcePos);
            }
            if (pPg!=nullptr) {
                InsertPage(pPg,nDestPos);
                if (bUndo) AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pPg));

                if(pPg->TRG_HasMasterPage())
                {
                    SdrPage& rMasterPage = pPg->TRG_GetMasterPage();
                    sal_uInt16 nMaPgNum(rMasterPage.GetPageNum());

                    if (bMergeMasterPages)
                    {
                        sal_uInt16 nNeuNum(0xFFFF);

                        if(pMasterMap)
                        {
                            nNeuNum = pMasterMap[nMaPgNum];
                        }

                        if(nNeuNum != 0xFFFF)
                        {
                            // tdf#90357 here pPg and the to-be-set new masterpage are parts of the new model
                            // already, but the currently set masterpage is part of the old model. Remove master
                            // page from already cloned page to prevent creating wrong undo action that can
                            // eventually crash the app.
                            // Do *not* remove it directly after cloning - the old masterpage is still needed
                            // later to find the new to-be-set masterpage.
                            pPg->TRG_ClearMasterPage();

                            if(bUndo)
                            {
                                AddUndo(GetSdrUndoFactory().CreateUndoPageChangeMasterPage(*pPg));
                            }

                            pPg->TRG_SetMasterPage(*GetMasterPage(nNeuNum));
                        }
                        DBG_ASSERT(nNeuNum!=0xFFFF,"SdrModel::Merge(): Something is crooked with the mapping of the MasterPages.");
                    } else {
                        if (nMaPgNum>=nDstMasterPageAnz) {
                            // This is outside of the original area of the MasterPage of the DstModel.
                            pPg->TRG_ClearMasterPage();
                        }
                    }
                }

            } else {
                OSL_FAIL("SdrModel::Merge(): Drawing page not found in SourceModel.");
            }
            nDestPos++;
            if (bReverse) nSourcePos--;
            else if (bTreadSourceAsConst) nSourcePos++;
            nMergeCount--;
        }
    }

    pMasterMap.reset();
    pMasterNeed.reset();

    bMPgNumsDirty=true;
    bPagNumsDirty=true;

    SetChanged();
    // TODO: Missing: merging and mapping of layers
    // at the objects as well as at the MasterPageDescriptors
    if (bUndo) EndUndo();
}

void SdrModel::SetStarDrawPreviewMode(bool bPreview)
{
    if (!bPreview && bStarDrawPreviewMode && GetPageCount())
    {
        // Resetting is not allowed, because the Model might not be loaded completely
        DBG_ASSERT(false,"SdrModel::SetStarDrawPreviewMode(): Resetting not allowed, because Model might not be complete.");
    }
    else
    {
        bStarDrawPreviewMode = bPreview;
    }
}

uno::Reference< uno::XInterface > SdrModel::getUnoModel()
{
    if( !mxUnoModel.is() )
        mxUnoModel = createUnoModel();

    return mxUnoModel;
}

void SdrModel::setUnoModel( css::uno::Reference< css::uno::XInterface > xModel )
{
    mxUnoModel = xModel;
}

uno::Reference< uno::XInterface > SdrModel::createUnoModel()
{
    OSL_FAIL( "SdrModel::createUnoModel() - base implementation should not be called!" );
    css::uno::Reference< css::uno::XInterface > xInt;
    return xInt;
}

void SdrModel::setLock( bool bLock )
{
    if( mbModelLocked != bLock )
    {
        // #i120437# need to set first, else ImpReformatAllEdgeObjects will do nothing
        mbModelLocked = bLock;

        if( !bLock )
        {
            ImpReformatAllEdgeObjects();
        }
    }
}


void SdrModel::MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel )
{
    assert(pNewModel != nullptr);
    if( pSourceSet && pDestSet && (pSourceSet != pDestSet ) )
    {
        SfxWhichIter aWhichIter(*pSourceSet);
        sal_uInt16 nWhich(aWhichIter.FirstWhich());
        const SfxPoolItem *pPoolItem;

        while(nWhich)
        {
            if(SfxItemState::SET == pSourceSet->GetItemState(nWhich, false, &pPoolItem))
            {
                const SfxPoolItem* pResultItem = nullptr;

                switch( nWhich )
                {
                case XATTR_FILLBITMAP:
                    pResultItem = static_cast<const XFillBitmapItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINEDASH:
                    pResultItem = static_cast<const XLineDashItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINESTART:
                    pResultItem = static_cast<const XLineStartItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINEEND:
                    pResultItem = static_cast<const XLineEndItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLGRADIENT:
                    pResultItem = static_cast<const XFillGradientItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLFLOATTRANSPARENCE:
                    // allow all kinds of XFillFloatTransparenceItem to be set
                    pResultItem = static_cast<const XFillFloatTransparenceItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLHATCH:
                    pResultItem = static_cast<const XFillHatchItem*>(pPoolItem)->checkForUniqueItem( pNewModel );
                    break;
                }

                // set item
                if( pResultItem )
                {
                    pDestSet->Put(*pResultItem);
                    delete pResultItem;
                }
                else
                    pDestSet->Put(*pPoolItem);
            }
            nWhich = aWhichIter.NextWhich();
        }
    }
}


void SdrModel::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
{
    if( mpForbiddenCharactersTable )
        mpForbiddenCharactersTable->release();

    mpForbiddenCharactersTable = xForbiddenChars.get();

    if( mpForbiddenCharactersTable )
        mpForbiddenCharactersTable->acquire();

    ImpSetOutlinerDefaults( pDrawOutliner );
    ImpSetOutlinerDefaults( pHitTestOutliner );
}


void SdrModel::SetCharCompressType( sal_uInt16 nType )
{
    if( nType != mnCharCompressType )
    {
        mnCharCompressType = nType;
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
    }
}

void SdrModel::SetKernAsianPunctuation( bool bEnabled )
{
    if( mbKernAsianPunctuation != bEnabled )
    {
        mbKernAsianPunctuation = bEnabled;
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
    }
}

void SdrModel::SetAddExtLeading( bool bEnabled )
{
    if( mbAddExtLeading != bEnabled )
    {
        mbAddExtLeading = bEnabled;
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
    }
}

void SdrModel::ReformatAllTextObjects()
{
    ImpReformatAllTextObjects();
}

SdrOutliner* SdrModel::createOutliner( sal_uInt16 nOutlinerMode )
{
    if( nullptr == mpOutlinerCache )
        mpOutlinerCache = new SdrOutlinerCache(this);

    return mpOutlinerCache->createOutliner( nOutlinerMode );
}

std::vector<SdrOutliner*> SdrModel::GetActiveOutliners() const
{
    std::vector< SdrOutliner* > aRet(mpOutlinerCache ? mpOutlinerCache->GetActiveOutliners() : std::vector< SdrOutliner* >());
    aRet.push_back(pDrawOutliner);
    aRet.push_back(pHitTestOutliner);

    return aRet;
}

void SdrModel::disposeOutliner( SdrOutliner* pOutliner )
{
    if( mpOutlinerCache )
    {
        mpOutlinerCache->disposeOutliner( pOutliner );
    }
    else
    {
        delete pOutliner;
    }
}

SvxNumType SdrModel::GetPageNumType() const
{
    return SVX_ARABIC;
}

const SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum) const
{
    DBG_ASSERT(nPgNum < maPages.size(), "SdrModel::GetPage: Access out of range (!)");
    return nPgNum < maPages.size() ? maPages[nPgNum] : nullptr;
}

SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum)
{
    DBG_ASSERT(nPgNum < maPages.size(), "SdrModel::GetPage: Access out of range (!)");
    return nPgNum < maPages.size() ? maPages[nPgNum] : nullptr;
}

sal_uInt16 SdrModel::GetPageCount() const
{
    return sal_uInt16(maPages.size());
}

void SdrModel::PageListChanged()
{
}

TextChain *SdrModel::GetTextChain() const
{
    return pTextChain;
}

const SdrPage* SdrModel::GetMasterPage(sal_uInt16 nPgNum) const
{
    DBG_ASSERT(nPgNum < maMaPag.size(), "SdrModel::GetMasterPage: Access out of range (!)");
    return maMaPag[nPgNum];
}

SdrPage* SdrModel::GetMasterPage(sal_uInt16 nPgNum)
{
    DBG_ASSERT(nPgNum < maMaPag.size(), "SdrModel::GetMasterPage: Access out of range (!)");
    return maMaPag[nPgNum];
}

sal_uInt16 SdrModel::GetMasterPageCount() const
{
    return sal_uInt16(maMaPag.size());
}

void SdrModel::MasterPageListChanged()
{
}

void SdrModel::SetSdrUndoManager( SfxUndoManager* pUndoManager )
{
    mpImpl->mpUndoManager = pUndoManager;
}

SfxUndoManager* SdrModel::GetSdrUndoManager() const
{
    return mpImpl->mpUndoManager;
}

SdrUndoFactory& SdrModel::GetSdrUndoFactory() const
{
    if( !mpImpl->mpUndoFactory )
        mpImpl->mpUndoFactory = new SdrUndoFactory;
    return *mpImpl->mpUndoFactory;
}

void SdrModel::SetSdrUndoFactory( SdrUndoFactory* pUndoFactory )
{
    if( pUndoFactory && (pUndoFactory != mpImpl->mpUndoFactory) )
    {
        delete mpImpl->mpUndoFactory;
        mpImpl->mpUndoFactory = pUndoFactory;
    }
}

void SdrModel::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sdrModel"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    sal_uInt16 nPageCount = GetPageCount();
    for (sal_uInt16 i = 0; i < nPageCount; ++i)
    {
        if (const SdrPage* pPage = GetPage(i))
            pPage->dumpAsXml(pWriter);
    }

    xmlTextWriterEndElement(pWriter);
}

namespace
{
    class theSdrModelUnoTunnelImplementationId : public rtl::Static< UnoTunnelIdInit, theSdrModelUnoTunnelImplementationId > {};
}

const css::uno::Sequence< sal_Int8 >& SdrModel::getUnoTunnelImplementationId()
{
    return theSdrModelUnoTunnelImplementationId::get().getSeq();
}


SdrHint::SdrHint(SdrHintKind eNewHint)
:   mpPage(nullptr),
    mpObj(nullptr),
    meHint(eNewHint)
{
}

SdrHint::SdrHint(const SdrObject& rNewObj)
:   mpPage(rNewObj.GetPage()),
    mpObj(&rNewObj),
    meHint(HINT_OBJCHG)
{
    maRectangle = rNewObj.GetLastBoundRect();
}

void SdrHint::SetPage(const SdrPage* pNewPage)
{
    mpPage = pNewPage;
}

void SdrHint::SetObject(const SdrObject* pNewObj)
{
    mpObj = pNewObj;
}

void SdrHint::SetKind(SdrHintKind eNewKind)
{
    meHint = eNewKind;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
