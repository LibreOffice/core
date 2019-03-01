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
#include <sal/log.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/whiter.hxx>
#include <svl/asiancfg.hxx>
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
#include <svx/svditer.hxx>
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
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svdoutlinercache.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <officecfg/Office/Common.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/forbiddencharacterstable.hxx>
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
#include <sfx2/viewsh.hxx>
#include <o3tl/enumrange.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


struct SdrModelImpl
{
    SfxUndoManager* mpUndoManager;
    SdrUndoFactory* mpUndoFactory;

    bool mbAnchoredTextOverflowLegacy; // tdf#99729 compatibility flag
};


void SdrModel::ImpCtor(
    SfxItemPool* pPool,
    ::comphelper::IEmbeddedHelper* _pEmbeddedHelper,
    bool bDisablePropertyFiles)
{
    mpImpl.reset(new SdrModelImpl);
    mpImpl->mpUndoManager=nullptr;
    mpImpl->mpUndoFactory=nullptr;
    mpImpl->mbAnchoredTextOverflowLegacy = false;
    mbInDestruction = false;
    aObjUnit=SdrEngineDefaults::GetMapFraction();
    eObjUnit=SdrEngineDefaults::GetMapUnit();
    eUIUnit=FieldUnit::MM;
    aUIScale=Fraction(1,1);
    nUIUnitDecimalMark=0;
    pLayerAdmin=nullptr;
    pItemPool=pPool;
    bMyPool=false;
    m_pEmbeddedHelper=_pEmbeddedHelper;
    pDrawOutliner=nullptr;
    pHitTestOutliner=nullptr;
    pRefOutDev=nullptr;
    pDefaultStyleSheet=nullptr;
    mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = nullptr;
    pLinkManager=nullptr;
    pUndoStack=nullptr;
    pRedoStack=nullptr;
    nMaxUndoCount=16;
    pCurrentUndoGroup=nullptr;
    nUndoLevel=0;
    mbUndoEnabled=true;
    mbChanged = false;
    bPagNumsDirty=false;
    bMPgNumsDirty=false;
    bTransportContainer = false;
    bSwapGraphics=false;
    bPasteResize=false;
    bReadOnly=false;
    nDefaultTabulator=0;
    bTransparentTextFrames=false;
    bStarDrawPreviewMode = false;
    mpForbiddenCharactersTable = nullptr;
    mbModelLocked = false;
    mpOutlinerCache = nullptr;
    mbKernAsianPunctuation = false;
    mbAddExtLeading = false;
    mnHandoutPageCount = 0;

    mbDisableTextEditUsesCommonUndoManager = false;

    if (!utl::ConfigManager::IsFuzzing())
        mnCharCompressType = static_cast<CharCompressType>(officecfg::Office::Common::AsianLayout::CompressCharacterDistance::
            get());
    else
        mnCharCompressType = CharCompressType::NONE;

    if ( pPool == nullptr )
    {
        pItemPool=new SdrItemPool(nullptr);
        // Outliner doesn't have its own Pool, so use the EditEngine's
        SfxItemPool* pOutlPool=EditEngine::CreatePool();
        // OutlinerPool as SecondaryPool of SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
        // remember that I created both pools myself
        bMyPool=true;
    }
    pItemPool->SetDefaultMetric(eObjUnit);

// using static SdrEngineDefaults only if default SvxFontHeight item is not available
    const SfxPoolItem* pPoolItem = pItemPool->GetPoolDefaultItem( EE_CHAR_FONTHEIGHT );
    if ( pPoolItem )
        mnDefTextHgt = static_cast<const SvxFontHeightItem*>(pPoolItem)->GetHeight();
    else
        mnDefTextHgt = SdrEngineDefaults::GetFontHeight();

    pItemPool->SetPoolDefaultItem( makeSdrTextWordWrapItem( false ) );

    SetTextDefaults();
    pLayerAdmin.reset(new SdrLayerAdmin);
    pLayerAdmin->SetModel(this);
    ImpSetUIUnit();

    // can't create DrawOutliner OnDemand, because I can't get the Pool,
    // then (only from 302 onwards!)
    pDrawOutliner = SdrMakeOutliner(OutlinerMode::TextObject, *this);
    ImpSetOutlinerDefaults(pDrawOutliner.get(), true);

    pHitTestOutliner = SdrMakeOutliner(OutlinerMode::TextObject, *this);
    ImpSetOutlinerDefaults(pHitTestOutliner.get(), true);

    /* Start Text Chaining related code */
    // Initialize Chaining Outliner
    pChainingOutliner = SdrMakeOutliner( OutlinerMode::TextObject, *this );
    ImpSetOutlinerDefaults(pChainingOutliner.get(), true);

    // Make a TextChain
    pTextChain.reset(new TextChain);
    /* End Text Chaining related code */

    ImpCreateTables(bDisablePropertyFiles || utl::ConfigManager::IsFuzzing());
}

SdrModel::SdrModel(
    SfxItemPool* pPool,
    ::comphelper::IEmbeddedHelper* pPers,
    bool bDisablePropertyFiles)
:
#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    maAllIncarnatedObjects(),
#endif
    maMaPag(),
    maPages()
{
    ImpCtor(pPool,pPers,bDisablePropertyFiles);
}

SdrModel::~SdrModel()
{

    mbInDestruction = true;

    Broadcast(SdrHint(SdrHintKind::ModelCleared));

    mpOutlinerCache.reset();

    ClearUndoBuffer();
#ifdef DBG_UTIL
    SAL_WARN_IF(pCurrentUndoGroup, "svx", "In the Dtor of the SdrModel there is an open Undo left: \""
                    << pCurrentUndoGroup->GetComment() << '\"');
#endif
    pCurrentUndoGroup.reset();

    ClearModel(true);

#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    if(!maAllIncarnatedObjects.empty())
    {
        SAL_WARN("svx","SdrModel::~SdrModel: Not all incarnations of SdrObjects deleted, possible memory leak (!)");
        // copy to std::vector - calling SdrObject::Free will change maAllIncarnatedObjects
        const std::vector< const SdrObject* > maRemainingObjects(maAllIncarnatedObjects.begin(), maAllIncarnatedObjects.end());
        for(auto pSdrObject : maRemainingObjects)
        {
            SdrObject* pCandidate(const_cast<SdrObject*>(pSdrObject));
            SdrObject::Free(pCandidate);
        }
    }
#endif

    pLayerAdmin.reset();

    pTextChain.reset();
    // Delete DrawOutliner only after deleting ItemPool, because ItemPool
    // references Items of the DrawOutliner!
    pChainingOutliner.reset();
    pHitTestOutliner.reset();
    pDrawOutliner.reset();

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

    mpForbiddenCharactersTable.reset();

    delete mpImpl->mpUndoFactory;
}

void SdrModel::SetSwapGraphics()
{
    bSwapGraphics = true;
}

bool SdrModel::IsReadOnly() const
{
    return bReadOnly;
}

void SdrModel::SetReadOnly(bool bYes)
{
    bReadOnly=bYes;
}


void SdrModel::SetMaxUndoActionCount(sal_uInt32 nCount)
{
    if (nCount<1) nCount=1;
    nMaxUndoCount=nCount;
    if (pUndoStack) {
        while (pUndoStack->size()>nMaxUndoCount) {
            pUndoStack->pop_back();
        }
    }
}

void SdrModel::ClearUndoBuffer()
{
    pUndoStack.reset();
    pRedoStack.reset();
}

bool SdrModel::HasUndoActions() const
{
    return pUndoStack && !pUndoStack->empty();
}

bool SdrModel::HasRedoActions() const
{
    return pRedoStack && !pRedoStack->empty();
}

void SdrModel::Undo()
{
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Undo(), method not supported with application undo manager!");
    }
    else
    {
        if(HasUndoActions())
        {
            SfxUndoAction* pDo = pUndoStack->front().get();
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Undo();
            if(!pRedoStack)
                pRedoStack.reset(new std::deque<std::unique_ptr<SfxUndoAction>>);
            std::unique_ptr<SfxUndoAction> p = std::move(pUndoStack->front());
            pUndoStack->pop_front();
            pRedoStack->emplace_front(std::move(p));
            mbUndoEnabled = bWasUndoEnabled;
        }
    }
}

void SdrModel::Redo()
{
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        if(HasRedoActions())
        {
            SfxUndoAction* pDo = pRedoStack->front().get();
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Redo();
            if(!pUndoStack)
                pUndoStack.reset(new std::deque<std::unique_ptr<SfxUndoAction>>);
            std::unique_ptr<SfxUndoAction> p = std::move(pRedoStack->front());
            pRedoStack->pop_front();
            pUndoStack->emplace_front(std::move(p));
            mbUndoEnabled = bWasUndoEnabled;
        }
    }
}

void SdrModel::Repeat(SfxRepeatTarget& rView)
{
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        if(HasUndoActions())
        {
            SfxUndoAction* pDo =  pUndoStack->front().get();
            if(pDo->CanRepeat(rView))
            {
                pDo->Repeat(rView);
            }
        }
    }
}

void SdrModel::ImpPostUndoAction(std::unique_ptr<SdrUndoAction> pUndo)
{
    DBG_ASSERT( mpImpl->mpUndoManager == nullptr, "svx::SdrModel::ImpPostUndoAction(), method not supported with application undo manager!" );
    if( IsUndoEnabled() )
    {
        if (aUndoLink)
        {
            aUndoLink(std::move(pUndo));
        }
        else
        {
            if (!pUndoStack)
                pUndoStack.reset(new std::deque<std::unique_ptr<SfxUndoAction>>);
            pUndoStack->emplace_front(std::move(pUndo));
            while (pUndoStack->size()>nMaxUndoCount)
            {
                pUndoStack->pop_back();
            }
            if (pRedoStack!=nullptr)
                pRedoStack->clear();
        }
    }
}

void SdrModel::BegUndo()
{
    if( mpImpl->mpUndoManager )
    {
        ViewShellId nViewShellId(-1);
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            nViewShellId = pViewShell->GetViewShellId();
        mpImpl->mpUndoManager->EnterListAction("","",0,nViewShellId);
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        if(!pCurrentUndoGroup)
        {
            pCurrentUndoGroup.reset(new SdrUndoGroup(*this));
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
        ViewShellId nViewShellId(-1);
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            nViewShellId = pViewShell->GetViewShellId();
        mpImpl->mpUndoManager->EnterListAction( rComment, "", 0, nViewShellId );
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (nUndoLevel==1)
        {
            pCurrentUndoGroup->SetComment(rComment);
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
        ViewShellId nViewShellId(-1);
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            nViewShellId = pViewShell->GetViewShellId();
        mpImpl->mpUndoManager->EnterListAction( aComment,"",0,nViewShellId );
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (nUndoLevel==1)
        {
            pCurrentUndoGroup->SetComment(rComment);
            pCurrentUndoGroup->SetObjDescription(rObjDescr);
            pCurrentUndoGroup->SetRepeatFunction(eFunc);
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
        if(pCurrentUndoGroup!=nullptr && IsUndoEnabled())
        {
            nUndoLevel--;
            if(nUndoLevel==0)
            {
                if(pCurrentUndoGroup->GetActionCount()!=0)
                {
                    ImpPostUndoAction(std::move(pCurrentUndoGroup));
                }
                else
                {
                    // was empty
                    pCurrentUndoGroup.reset();
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
    else if( IsUndoEnabled() && nUndoLevel==1)
    {
        pCurrentUndoGroup->SetComment(rComment);
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
            pCurrentUndoGroup->SetComment(rComment);
            pCurrentUndoGroup->SetObjDescription(rObjDescr);
        }
    }
}

void SdrModel::AddUndo(std::unique_ptr<SdrUndoAction> pUndo)
{
    if( mpImpl->mpUndoManager )
    {
        mpImpl->mpUndoManager->AddUndoAction( std::move(pUndo) );
    }
    else if( IsUndoEnabled() )
    {
        if (pCurrentUndoGroup)
        {
            pCurrentUndoGroup->AddAction(std::move(pUndo));
        }
        else
        {
            ImpPostUndoAction(std::move(pUndo));
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

void SdrModel::ImpCreateTables(bool bDisablePropertyFiles)
{
    // use standard path for initial construction
    const OUString aTablePath(!bDisablePropertyFiles ? SvtPathOptions().GetPalettePath() : "");

    for( auto i : o3tl::enumrange<XPropertyListType>() )
    {
        maProperties[i] = XPropertyList::CreatePropertyList(i, aTablePath, ""/*TODO?*/ );
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
        DeletePage( static_cast<sal_uInt16>(i) );
    }
    maPages.clear();
    PageListChanged();

    // delete all Masterpages
    nCount=GetMasterPageCount();
    for(i=nCount-1; i>=0; i--)
    {
        DeleteMasterPage( static_cast<sal_uInt16>(i) );
    }
    maMaPag.clear();
    MasterPageListChanged();

    pLayerAdmin->ClearLayers();
}

SdrModel* SdrModel::AllocModel() const
{
    SdrModel* pModel=new SdrModel();
    pModel->SetScaleUnit(eObjUnit,aObjUnit);
    return pModel;
}

SdrPage* SdrModel::AllocPage(bool bMasterPage)
{
    return new SdrPage(*this,bMasterPage);
}

void SdrModel::SetTextDefaults() const
{
    SetTextDefaults( pItemPool, mnDefTextHgt );
}

void SdrModel::SetTextDefaults( SfxItemPool* pItemPool, sal_Int32 nDefTextHgt )
{
    // set application-language specific dynamic pool language defaults
    SvxFontItem aSvxFontItem( EE_CHAR_FONTINFO) ;
    SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
    SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
    LanguageType nLanguage;
    if (!utl::ConfigManager::IsFuzzing())
        nLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    else
        nLanguage = LANGUAGE_ENGLISH_US;

    // get DEFAULTFONT_LATIN_TEXT and set at pool as dynamic default
    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::LATIN_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItem.SetFamily(aFont.GetFamilyType());
    aSvxFontItem.SetFamilyName(aFont.GetFamilyName());
    aSvxFontItem.SetStyleName(OUString());
    aSvxFontItem.SetPitch( aFont.GetPitch());
    aSvxFontItem.SetCharSet( aFont.GetCharSet() );
    pItemPool->SetPoolDefaultItem(aSvxFontItem);

    // get DEFAULTFONT_CJK_TEXT and set at pool as dynamic default
    vcl::Font aFontCJK(OutputDevice::GetDefaultFont(DefaultFontType::CJK_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCJK.SetFamily( aFontCJK.GetFamilyType());
    aSvxFontItemCJK.SetFamilyName(aFontCJK.GetFamilyName());
    aSvxFontItemCJK.SetStyleName(OUString());
    aSvxFontItemCJK.SetPitch( aFontCJK.GetPitch());
    aSvxFontItemCJK.SetCharSet( aFontCJK.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCJK);

    // get DEFAULTFONT_CTL_TEXT and set at pool as dynamic default
    vcl::Font aFontCTL(OutputDevice::GetDefaultFont(DefaultFontType::CTL_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCTL.SetFamily(aFontCTL.GetFamilyType());
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
    ImpSetOutlinerDefaults( pDrawOutliner.get() );
    ImpSetOutlinerDefaults( pHitTestOutliner.get() );
    RefDeviceChanged();
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
                ::comphelper::LifecycleProxy const & rProxy) const
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
    catch (uno::Exception const&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("svx", "exception: " << exceptionToString(ex));
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
    Broadcast(SdrHint(SdrHintKind::RefDeviceChange));
    ImpReformatAllTextObjects();
}

void SdrModel::SetDefaultFontHeight(sal_Int32 nVal)
{
    if (nVal!=mnDefTextHgt) {
        mnDefTextHgt=nVal;
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetDefaultTabulator(sal_uInt16 nVal)
{
    if (nDefaultTabulator!=nVal) {
        nDefaultTabulator=nVal;
        Outliner& rOutliner=GetDrawOutliner();
        rOutliner.SetDefTab(nVal);
        Broadcast(SdrHint(SdrHintKind::DefaultTabChange));
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
    nUIUnitDecimalMark = 0;
    sal_Int64 nMul(1);
    sal_Int64 nDiv(1);

    // normalize on meters resp. inch
    switch (eObjUnit)
    {
        case MapUnit::Map100thMM   : nUIUnitDecimalMark+=5; break;
        case MapUnit::Map10thMM    : nUIUnitDecimalMark+=4; break;
        case MapUnit::MapMM         : nUIUnitDecimalMark+=3; break;
        case MapUnit::MapCM         : nUIUnitDecimalMark+=2; break;
        case MapUnit::Map1000thInch: nUIUnitDecimalMark+=3; break;
        case MapUnit::Map100thInch : nUIUnitDecimalMark+=2; break;
        case MapUnit::Map10thInch  : nUIUnitDecimalMark+=1; break;
        case MapUnit::MapInch       : nUIUnitDecimalMark+=0; break;
        case MapUnit::MapPoint      : nDiv=72;     break;          // 1Pt   = 1/72"
        case MapUnit::MapTwip       : nDiv=144; nUIUnitDecimalMark++; break; // 1Twip = 1/1440"
        case MapUnit::MapPixel      : break;
        case MapUnit::MapSysFont    : break;
        case MapUnit::MapAppFont    : break;
        case MapUnit::MapRelative   : break;
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
        case FieldUnit::NONE   : break;
        // metric
        case FieldUnit::MM_100TH: nUIUnitDecimalMark-=5; break;
        case FieldUnit::MM     : nUIUnitDecimalMark-=3; break;
        case FieldUnit::CM     : nUIUnitDecimalMark-=2; break;
        case FieldUnit::M      : nUIUnitDecimalMark+=0; break;
        case FieldUnit::KM     : nUIUnitDecimalMark+=3; break;
        // Inch
        case FieldUnit::TWIP   : nMul=144; nUIUnitDecimalMark--;  break;  // 1Twip = 1/1440"
        case FieldUnit::POINT  : nMul=72;     break;            // 1Pt   = 1/72"
        case FieldUnit::PICA   : nMul=6;      break;            // 1Pica = 1/6"
        case FieldUnit::INCH   : break;                         // 1"    = 1"
        case FieldUnit::FOOT   : nDiv*=12;    break;            // 1Ft   = 12"
        case FieldUnit::MILE   : nDiv*=6336; nUIUnitDecimalMark++; break; // 1mile = 63360"
        // other
        case FieldUnit::CUSTOM : break;
        case FieldUnit::PERCENT: nUIUnitDecimalMark+=2; break;
        // TODO: Add code to handle the following if needed (added to remove warning)
        case FieldUnit::CHAR   : break;
        case FieldUnit::LINE   : break;
        case FieldUnit::PIXEL  : break;
        case FieldUnit::DEGREE : break;
        case FieldUnit::SECOND : break;
        case FieldUnit::MILLISECOND : break;
    } // switch

    // check if mapping is from metric to inch and adapt
    const bool bMapInch(IsInch(eObjUnit));
    const bool bUIMetr(IsMetric(eUIUnit));

    if (bMapInch && bUIMetr)
    {
        nUIUnitDecimalMark += 4;
        nMul *= 254;
    }

    // check if mapping is from inch to metric and adapt
    const bool bMapMetr(IsMetric(eObjUnit));
    const bool bUIInch(IsInch(eUIUnit));

    if (bMapMetr && bUIInch)
    {
        nUIUnitDecimalMark -= 4;
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
        nUIUnitDecimalMark--;
        nMul /= 10;
    }

    // shorten trailing zeros for divisor
    while(0 == (nDiv % 10))
    {
        nUIUnitDecimalMark++;
        nDiv /= 10;
    }

    // end preparations, set member values
    aUIUnitFact = Fraction(sal_Int32(nMul), sal_Int32(nDiv));
    aUIUnitStr = GetUnitString(eUIUnit);
}

void SdrModel::SetScaleUnit(MapUnit eMap, const Fraction& rFrac)
{
    if (eObjUnit!=eMap || aObjUnit!=rFrac) {
        eObjUnit=eMap;
        aObjUnit=rFrac;
        pItemPool->SetDefaultMetric(eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetScaleUnit(MapUnit eMap)
{
    if (eObjUnit!=eMap) {
        eObjUnit=eMap;
        pItemPool->SetDefaultMetric(eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetScaleFraction(const Fraction& rFrac)
{
    if (aObjUnit!=rFrac) {
        aObjUnit=rFrac;
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
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

OUString SdrModel::GetUnitString(FieldUnit eUnit)
{
    switch(eUnit)
    {
        default:
        case FieldUnit::NONE   :
        case FieldUnit::CUSTOM :
            return OUString();
        case FieldUnit::MM_100TH:
            return OUString{"/100mm"};
        case FieldUnit::MM     :
            return OUString{"mm"};
        case FieldUnit::CM     :
            return OUString{"cm"};
        case FieldUnit::M      :
            return OUString{"m"};
        case FieldUnit::KM     :
            return OUString{"km"};
        case FieldUnit::TWIP   :
            return OUString{"twip"};
        case FieldUnit::POINT  :
            return OUString{"pt"};
        case FieldUnit::PICA   :
            return OUString{"pica"};
        case FieldUnit::INCH   :
            return OUString{"\""};
        case FieldUnit::FOOT   :
            return OUString{"ft"};
        case FieldUnit::MILE   :
            return OUString{"mile(s)"};
        case FieldUnit::PERCENT:
            return OUString{"%"};
    }
}

OUString SdrModel::GetMetricString(long nVal, bool bNoUnitChars, sal_Int32 nNumDigits) const
{
    // #i22167#
    // change to double precision usage to not lose decimal places
    const bool bNegative(nVal < 0);
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

    sal_Int32 nDecimalMark(nUIUnitDecimalMark);

    if(nDecimalMark > nNumDigits)
    {
        const sal_Int32 nDiff(nDecimalMark - nNumDigits);
        const double fFactor(pow(10.0, static_cast<int>(nDiff)));

        fLocalValue /= fFactor;
        nDecimalMark = nNumDigits;
    }
    else if(nDecimalMark < nNumDigits)
    {
        const sal_Int32 nDiff(nNumDigits - nDecimalMark);
        const double fFactor(pow(10.0, static_cast<int>(nDiff)));

        fLocalValue *= fFactor;
        nDecimalMark = nNumDigits;
    }

    OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(fLocalValue + 0.5));

    if(nDecimalMark < 0)
    {
        // negative nDecimalMark (decimal point) means: add zeros
        sal_Int32 nCount(-nDecimalMark);

        for(sal_Int32 i=0; i<nCount; i++)
            aBuf.append('0');

        nDecimalMark = 0;
    }

    // the second condition needs to be <= since inside this loop
    // also the leading zero is inserted.
    if (nDecimalMark > 0 && aBuf.getLength() <= nDecimalMark)
    {
        // if necessary, add zeros before the decimal point
        sal_Int32 nCount = nDecimalMark - aBuf.getLength();

        if(nCount >= 0 && LocaleDataWrapper::isNumLeadingZero())
            nCount++;

        for(sal_Int32 i=0; i<nCount; i++)
            aBuf.insert(0, '0');
    }

    const sal_Unicode cDec( rLoc.getNumDecimalSep()[0] );

    // insert the decimal mark character
    sal_Int32 nBeforeDecimalMark = aBuf.getLength() - nDecimalMark;

    if(nDecimalMark > 0)
        aBuf.insert(nBeforeDecimalMark, cDec);

    if(!LocaleDataWrapper::isNumTrailingZeros())
    {
        sal_Int32 aPos=aBuf.getLength()-1;

        // Remove all trailing zeros.
        while (aPos>=0 && aBuf[aPos]=='0')
            --aPos;

        // Remove decimal if it's the last character.
        if (aPos>=0 && aBuf[aPos]==cDec)
            --aPos;

        // Adjust aPos to index first char to be truncated, if any
        if (++aPos<aBuf.getLength())
            aBuf.truncate(aPos);
    }

    // if necessary, add separators before every third digit
    if( nBeforeDecimalMark > 3 )
    {
        const OUString& aThoSep( rLoc.getNumThousandSep() );
        if ( !aThoSep.isEmpty() )
        {
            sal_Unicode cTho( aThoSep[0] );
            sal_Int32 i(nBeforeDecimalMark - 3);

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

    return aBuf.makeStringAndClear();
}

OUString SdrModel::GetAngleString(long nAngle)
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

    aBuf.append(DEGREE_CHAR);

    return aBuf.makeStringAndClear();
}

OUString SdrModel::GetPercentString(const Fraction& rVal)
{
    sal_Int32 nMul(rVal.GetNumerator());
    sal_Int32 nDiv(rVal.GetDenominator());
    bool bNeg {false};

    if (nDiv < 0)
    {
        bNeg = !bNeg;
        nDiv = -nDiv;
    }

    if (nMul < 0)
    {
        bNeg = !bNeg;
        nMul = -nMul;
    }

    sal_Int32 nPct = ((nMul*100) + nDiv/2)/nDiv;

    if (bNeg)
        nPct = -nPct;

    return OUString::number(nPct) + "%";
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

    if (nPos<nCount) bPagNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPage);
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
    SdrHint aHint(SdrHintKind::PageOrderChange, pPg);
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

    if (nPos<nCount) {
        bMPgNumsDirty=true;
    }

    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPage);
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
        sal_uInt16 nPageCnt(GetPageCount());

        for(sal_uInt16 np(0); np < nPageCnt; np++)
        {
            GetPage(np)->TRG_ImpMasterPageRemoved(*pRetPg);
        }

        pRetPg->SetInserted(false);
    }

    bMPgNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pRetPg);
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
    SdrHint aHint(SdrHintKind::PageOrderChange, pPg);
    Broadcast(aHint);
}


void SdrModel::CopyPages(sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                         sal_uInt16 nDestPos,
                         bool bUndo, bool bMoveNoCopy)
{
    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

    if( bUndo )
        BegUndo(SvxResId(STR_UndoMergeModel));

    sal_uInt16 nPageCnt=GetPageCount();
    sal_uInt16 nMaxPage=nPageCnt;

    if (nMaxPage!=0)
        nMaxPage--;
    if (nFirstPageNum>nMaxPage)
        nFirstPageNum=nMaxPage;
    if (nLastPageNum>nMaxPage)
        nLastPageNum =nMaxPage;
    bool bReverse=nLastPageNum<nFirstPageNum;
    if (nDestPos>nPageCnt)
        nDestPos=nPageCnt;

    // at first, save the pointers of the affected pages in an array
    sal_uInt16 nPageNum=nFirstPageNum;
    sal_uInt16 nCopyCnt=((!bReverse)?(nLastPageNum-nFirstPageNum):(nFirstPageNum-nLastPageNum))+1;
    std::unique_ptr<SdrPage*[]> pPagePtrs(new SdrPage*[nCopyCnt]);
    sal_uInt16 nCopyNum;
    for(nCopyNum=0; nCopyNum<nCopyCnt; nCopyNum++)
    {
        pPagePtrs[nCopyNum]=GetPage(nPageNum);
        if (bReverse)
            nPageNum--;
        else
            nPageNum++;
    }

    // now copy the pages
    sal_uInt16 nDestNum=nDestPos;
    for (nCopyNum=0; nCopyNum<nCopyCnt; nCopyNum++)
    {
        SdrPage* pPg=pPagePtrs[nCopyNum];
        sal_uInt16 nPageNum2=pPg->GetPageNum();
        if (!bMoveNoCopy)
        {
            const SdrPage* pPg1=GetPage(nPageNum2);

            // Clone to local model
            pPg = pPg1->CloneSdrPage(*this);

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
        BegUndo(SvxResId(STR_UndoMergeModel));

    sal_uInt16 nSrcPageCnt=rSourceModel.GetPageCount();
    sal_uInt16 nSrcMasterPageCnt=rSourceModel.GetMasterPageCount();
    sal_uInt16 nDstMasterPageCnt=GetMasterPageCount();
    bool bInsPages=(nFirstPageNum<nSrcPageCnt || nLastPageNum<nSrcPageCnt);
    sal_uInt16 nMaxSrcPage=nSrcPageCnt; if (nMaxSrcPage!=0) nMaxSrcPage--;
    if (nFirstPageNum>nMaxSrcPage) nFirstPageNum=nMaxSrcPage;
    if (nLastPageNum>nMaxSrcPage)  nLastPageNum =nMaxSrcPage;
    bool bReverse=nLastPageNum<nFirstPageNum;

    std::unique_ptr<sal_uInt16[]> pMasterMap;
    std::unique_ptr<bool[]> pMasterNeed;
    sal_uInt16    nMasterNeed=0;
    if (bMergeMasterPages && nSrcMasterPageCnt!=0) {
        // determine which MasterPages from rSrcModel we need
        pMasterMap.reset(new sal_uInt16[nSrcMasterPageCnt]);
        pMasterNeed.reset(new bool[nSrcMasterPageCnt]);
        memset(pMasterMap.get(),0xFF,nSrcMasterPageCnt*sizeof(sal_uInt16));
        if (bAllMasterPages) {
            memset(pMasterNeed.get(), true, nSrcMasterPageCnt * sizeof(bool));
        } else {
            memset(pMasterNeed.get(), false, nSrcMasterPageCnt * sizeof(bool));
            sal_uInt16 nStart= bReverse ? nLastPageNum : nFirstPageNum;
            sal_uInt16 nEnd= bReverse ? nFirstPageNum : nLastPageNum;
            for (sal_uInt16 i=nStart; i<=nEnd; i++) {
                const SdrPage* pPg=rSourceModel.GetPage(i);
                if(pPg->TRG_HasMasterPage())
                {
                    SdrPage& rMasterPage = pPg->TRG_GetMasterPage();
                    sal_uInt16 nMPgNum(rMasterPage.GetPageNum());

                    if(nMPgNum < nSrcMasterPageCnt)
                    {
                        pMasterNeed[nMPgNum] = true;
                    }
                }
            }
        }
        // now determine the Mapping of the MasterPages
        sal_uInt16 nCurrentMaPagNum=nDstMasterPageCnt;
        for (sal_uInt16 i=0; i<nSrcMasterPageCnt; i++) {
            if (pMasterNeed[i]) {
                pMasterMap[i]=nCurrentMaPagNum;
                nCurrentMaPagNum++;
                nMasterNeed++;
            }
        }
    }

    // get the MasterPages
    if (pMasterMap && pMasterNeed && nMasterNeed!=0) {
        for (sal_uInt16 i=nSrcMasterPageCnt; i>0;) {
            i--;
            if (pMasterNeed[i])
            {
                // Always Clone to new model
                const SdrPage* pPg1(rSourceModel.GetMasterPage(i));
                SdrPage* pPg(pPg1->CloneSdrPage(*this));

                if(!bTreadSourceAsConst)
                {
                    // if requested, delete original/modify original model
                    delete rSourceModel.RemoveMasterPage(i);
                }

                if (pPg!=nullptr) {
                    // Now append all of them to the end of the DstModel.
                    // Don't use InsertMasterPage(), because everything is
                    // inconsistent until all are in.
                    maMaPag.insert(maMaPag.begin()+nDstMasterPageCnt, pPg);
                    MasterPageListChanged();
                    pPg->SetInserted();
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
        sal_uInt16 nMergeCount=sal_uInt16(std::abs(static_cast<long>(static_cast<long>(nFirstPageNum)-nLastPageNum))+1);
        if (nDestPos>GetPageCount()) nDestPos=GetPageCount();
        while (nMergeCount>0)
        {
            // Always Clone to new model
            const SdrPage* pPg1(rSourceModel.GetPage(nSourcePos));
            SdrPage* pPg(pPg1->CloneSdrPage(*this));

            if(!bTreadSourceAsConst)
            {
                // if requested, delete original/modify original model
                delete rSourceModel.RemovePage(nSourcePos);
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
                        sal_uInt16 nNewNum(0xFFFF);

                        if(pMasterMap)
                        {
                            nNewNum = pMasterMap[nMaPgNum];
                        }

                        if(nNewNum != 0xFFFF)
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

                            pPg->TRG_SetMasterPage(*GetMasterPage(nNewNum));
                        }
                        DBG_ASSERT(nNewNum!=0xFFFF,"SdrModel::Merge(): Something is crooked with the mapping of the MasterPages.");
                    } else {
                        if (nMaPgNum>=nDstMasterPageCnt) {
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
        SAL_WARN("svx", "SdrModel::SetStarDrawPreviewMode(): Resetting not allowed, because Model might not be complete.");
    }
    else
    {
        bStarDrawPreviewMode = bPreview;
    }
}

uno::Reference< uno::XInterface > const & SdrModel::getUnoModel()
{
    if( !mxUnoModel.is() )
        mxUnoModel = createUnoModel();

    return mxUnoModel;
}

void SdrModel::setUnoModel( const css::uno::Reference< css::uno::XInterface >& xModel )
{
    mxUnoModel = xModel;
}

void SdrModel::adaptSizeAndBorderForAllPages(
    const Size& /*rNewSize*/,
    long /*nLeft*/,
    long /*nRight*/,
    long /*nUpper*/,
    long /*nLower*/)
{
    // base implementation does currently nothing. It may be added if needed,
    // but we are on SdrModel level here, thus probably have not enough information
    // to do this for higher-level (derived) Models (e.g. Draw/Impress)
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


void SdrModel::MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModelel )
{
    assert(pNewModelel != nullptr);
    if( pSourceSet && pDestSet && (pSourceSet != pDestSet ) )
    {
        SfxWhichIter aWhichIter(*pSourceSet);
        sal_uInt16 nWhich(aWhichIter.FirstWhich());
        const SfxPoolItem *pPoolItem;

        while(nWhich)
        {
            if(SfxItemState::SET == pSourceSet->GetItemState(nWhich, false, &pPoolItem))
            {
                std::unique_ptr<SfxPoolItem> pResultItem;

                switch( nWhich )
                {
                case XATTR_FILLBITMAP:
                    pResultItem = static_cast<const XFillBitmapItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_LINEDASH:
                    pResultItem = static_cast<const XLineDashItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_LINESTART:
                    pResultItem = static_cast<const XLineStartItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_LINEEND:
                    pResultItem = static_cast<const XLineEndItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_FILLGRADIENT:
                    pResultItem = static_cast<const XFillGradientItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_FILLFLOATTRANSPARENCE:
                    // allow all kinds of XFillFloatTransparenceItem to be set
                    pResultItem = static_cast<const XFillFloatTransparenceItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                case XATTR_FILLHATCH:
                    pResultItem = static_cast<const XFillHatchItem*>(pPoolItem)->checkForUniqueItem( pNewModelel );
                    break;
                }

                // set item
                if( pResultItem )
                {
                    pDestSet->Put(*pResultItem);
                    pResultItem.reset();
                }
                else
                    pDestSet->Put(*pPoolItem);
            }
            nWhich = aWhichIter.NextWhich();
        }
    }
}


void SdrModel::SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars)
{
    mpForbiddenCharactersTable = xForbiddenChars;

    ImpSetOutlinerDefaults( pDrawOutliner.get() );
    ImpSetOutlinerDefaults( pHitTestOutliner.get() );
}


void SdrModel::SetCharCompressType( CharCompressType nType )
{
    if( nType != mnCharCompressType )
    {
        mnCharCompressType = nType;
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
    }
}

void SdrModel::SetKernAsianPunctuation( bool bEnabled )
{
    if( mbKernAsianPunctuation != bEnabled )
    {
        mbKernAsianPunctuation = bEnabled;
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
    }
}

void SdrModel::SetAddExtLeading( bool bEnabled )
{
    if( mbAddExtLeading != bEnabled )
    {
        mbAddExtLeading = bEnabled;
        ImpSetOutlinerDefaults( pDrawOutliner.get() );
        ImpSetOutlinerDefaults( pHitTestOutliner.get() );
    }
}

void SdrModel::SetAnchoredTextOverflowLegacy(bool bEnabled)
{
    mpImpl->mbAnchoredTextOverflowLegacy = bEnabled;
}

bool SdrModel::IsAnchoredTextOverflowLegacy() const
{
    return mpImpl->mbAnchoredTextOverflowLegacy;
}

void SdrModel::ReformatAllTextObjects()
{
    ImpReformatAllTextObjects();
}

std::unique_ptr<SdrOutliner> SdrModel::createOutliner( OutlinerMode nOutlinerMode )
{
    if( !mpOutlinerCache )
        mpOutlinerCache.reset(new SdrOutlinerCache(this));

    return mpOutlinerCache->createOutliner( nOutlinerMode );
}

std::vector<SdrOutliner*> SdrModel::GetActiveOutliners() const
{
    std::vector< SdrOutliner* > aRet(mpOutlinerCache ? mpOutlinerCache->GetActiveOutliners() : std::vector< SdrOutliner* >());
    aRet.push_back(pDrawOutliner.get());
    aRet.push_back(pHitTestOutliner.get());

    return aRet;
}

void SdrModel::disposeOutliner( std::unique_ptr<SdrOutliner> pOutliner )
{
    if( mpOutlinerCache )
        mpOutlinerCache->disposeOutliner( std::move(pOutliner) );
}

SvxNumType SdrModel::GetPageNumType() const
{
    return SVX_NUM_ARABIC;
}

void SdrModel::ReadUserDataSequenceValue(const css::beans::PropertyValue* pValue)
{
    bool bBool = false;
    if (pValue->Name == "AnchoredTextOverflowLegacy")
    {
        if (pValue->Value >>= bBool)
        {
            mpImpl->mbAnchoredTextOverflowLegacy = bBool;
        }
    }
}

template <typename T>
static void addPair(std::vector< std::pair< OUString, Any > >& aUserData, const OUString& name, const T val)
{
    aUserData.push_back(std::pair< OUString, Any >(name, css::uno::makeAny(val)));
}

void SdrModel::WriteUserDataSequence(css::uno::Sequence < css::beans::PropertyValue >& rValues)
{
    std::vector< std::pair< OUString, Any > > aUserData;
    addPair(aUserData, "AnchoredTextOverflowLegacy", IsAnchoredTextOverflowLegacy());

    const sal_Int32 nOldLength = rValues.getLength();
    rValues.realloc(nOldLength + aUserData.size());

    css::beans::PropertyValue* pValue = &(rValues.getArray()[nOldLength]);

    for (const auto &aIter : aUserData)
    {
        pValue->Name = aIter.first;
        pValue->Value = aIter.second;
        ++pValue;
    }
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
    return pTextChain.get();
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrModel"));
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
:   meHint(eNewHint),
    mpObj(nullptr),
    mpPage(nullptr)
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj)
:   meHint(eNewHint),
    mpObj(&rNewObj),
    mpPage(rNewObj.getSdrPageFromSdrObject())
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrPage* pPage)
:   meHint(eNewHint),
    mpObj(nullptr),
    mpPage(pPage)
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj, const SdrPage* pPage)
:   meHint(eNewHint),
    mpObj(&rNewObj),
    mpPage(pPage)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
