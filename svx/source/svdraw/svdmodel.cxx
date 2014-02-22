/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/svdmodel.hxx>

#include <math.h>

#include <osl/endian.h>
#include <rtl/strbuf.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <unotools/ucbstreamhelper.hxx>

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
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdoole2.hxx>
#include "svx/svdglob.hxx"
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;



struct SdrModelImpl
{
    SfxUndoManager* mpUndoManager;
    SdrUndoFactory* mpUndoFactory;
};



DBG_NAME(SdrModel)
TYPEINIT1(SdrModel,SfxBroadcaster);
void SdrModel::ImpCtor(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* _pEmbeddedHelper,
    bool bUseExtColorTable, bool bLoadRefCounts)
{
    mpImpl = new SdrModelImpl;
    mpImpl->mpUndoManager=0;
    mpImpl->mpUndoFactory=0;
    mbInDestruction = false;
    aObjUnit=SdrEngineDefaults::GetMapFraction();
    eObjUnit=SdrEngineDefaults::GetMapUnit();
    eUIUnit=FUNIT_MM;
    aUIScale=Fraction(1,1);
    nUIUnitKomma=0;
    bUIOnlyKomma=false;
    pLayerAdmin=NULL;
    pItemPool=pPool;
    bMyPool=false;
    m_pEmbeddedHelper=_pEmbeddedHelper;
    pDrawOutliner=NULL;
    pHitTestOutliner=NULL;
    pRefOutDev=NULL;
    nProgressAkt=0;
    nProgressMax=0;
    nProgressOfs=0;
    pDefaultStyleSheet=NULL;
    mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = 0;
    pLinkManager=NULL;
    pUndoStack=NULL;
    pRedoStack=NULL;
    nMaxUndoCount=16;
    pAktUndoGroup=NULL;
    nUndoLevel=0;
    mbUndoEnabled=true;
    nProgressPercent=0;
    nLoadVersion=0;
    bExtColorTable=false;
    mbChanged = false;
    bInfoChanged=false;
    bPagNumsDirty=false;
    bMPgNumsDirty=false;
    bPageNotValid=false;
    bSavePortable=false;
    bSaveCompressed=false;
    bSaveNative=false;
    bSwapGraphics=false;
    nSwapGraphicsMode=SDR_SWAPGRAPHICSMODE_DEFAULT;
    bSaveOLEPreview=false;
    bPasteResize=false;
    bNoBitmapCaching=false;
    bReadOnly=false;
    nStreamCompressMode=COMPRESSMODE_NONE;
    nStreamNumberFormat=NUMBERFORMAT_INT_BIGENDIAN;
    nDefaultTabulator=0;
    mpNumberFormatter = NULL;
    bTransparentTextFrames=false;
    bStarDrawPreviewMode = false;
    nStarDrawPreviewMasterPageNum = SDRPAGE_NOTFOUND;
    mpForbiddenCharactersTable = NULL;
    mbModelLocked = false;
    mpOutlinerCache = NULL;
    mbKernAsianPunctuation = false;
    mbAddExtLeading = false;
    mnHandoutPageCount = 0;

    mbDisableTextEditUsesCommonUndoManager = false;

    mnCharCompressType =
        officecfg::Office::Common::AsianLayout::CompressCharacterDistance::
        get();

#ifdef OSL_LITENDIAN
    nStreamNumberFormat=NUMBERFORMAT_INT_LITTLEENDIAN;
#endif
    bExtColorTable=bUseExtColorTable;

    if ( pPool == NULL )
    {
        pItemPool=new SdrItemPool(0L, bLoadRefCounts);
        
        SfxItemPool* pOutlPool=EditEngine::CreatePool( bLoadRefCounts );
        
        pItemPool->SetSecondaryPool(pOutlPool);
        
        bMyPool=true;
    }
    pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);


    const SfxPoolItem* pPoolItem = pItemPool->GetPoolDefaultItem( EE_CHAR_FONTHEIGHT );
    if ( pPoolItem )
        nDefTextHgt = ((SvxFontHeightItem*)pPoolItem)->GetHeight();
    else
        nDefTextHgt = SdrEngineDefaults::GetFontHeight();

    pItemPool->SetPoolDefaultItem( SdrTextWordWrapItem( sal_False ) );

    SetTextDefaults();
    pLayerAdmin=new SdrLayerAdmin;
    pLayerAdmin->SetModel(this);
    ImpSetUIUnit();

    
    
    pDrawOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
    ImpSetOutlinerDefaults(pDrawOutliner, sal_True);

    pHitTestOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
    ImpSetOutlinerDefaults(pHitTestOutliner, sal_True);

    ImpCreateTables();
}

SdrModel::SdrModel(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages()
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool, pPers, false, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(const OUString& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages(),
    aTablePath(rPath)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool, pPers, false, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages()
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,bUseExtColorTable, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(const OUString& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages(),
    aTablePath(rPath)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,bUseExtColorTable, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(const SdrModel& /*rSrcModel*/):
    SfxBroadcaster(),
    tools::WeakBase< SdrModel >(),
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages()
{
    
    OSL_FAIL("SdrModel::CopyCtor() is not yet implemented.");
}

SdrModel::~SdrModel()
{
    DBG_DTOR(SdrModel,NULL);

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

    ClearModel(sal_True);

    delete pLayerAdmin;

    
    
    delete pHitTestOutliner;
    delete pDrawOutliner;

    
    
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
        
        SfxItemPool* pOutlPool=pItemPool->GetSecondaryPool();
        SfxItemPool::Free(pItemPool);
        
        
        SfxItemPool::Free(pOutlPool);
    }

    if( mpForbiddenCharactersTable )
        mpForbiddenCharactersTable->release();

    delete mpNumberFormatter;

    delete mpImpl->mpUndoFactory;
    delete mpImpl;
}

bool SdrModel::IsInDestruction() const
{
    return mbInDestruction;
}


void SdrModel::operator=(const SdrModel&)
{
    OSL_FAIL("SdrModel::operator=() is not yet implemented.");
}

bool SdrModel::operator==(const SdrModel&) const
{
    OSL_FAIL("SdrModel::operator==() is not yet implemented");
    return false;
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



void SdrModel::SetMaxUndoActionCount(sal_uIntPtr nAnz)
{
    if (nAnz<1) nAnz=1;
    nMaxUndoCount=nAnz;
    if (pUndoStack!=NULL) {
        while (pUndoStack->size()>nMaxUndoCount) {
            delete pUndoStack->back();
            pUndoStack->pop_back();
        }
    }
}

void SdrModel::ClearUndoBuffer()
{
    if (pUndoStack!=NULL) {
        while (!pUndoStack->empty()) {
            delete pUndoStack->back();
            pUndoStack->pop_back();
        }
        delete pUndoStack;
        pUndoStack=NULL;
    }
    if (pRedoStack!=NULL) {
        while (!pRedoStack->empty()) {
            delete pRedoStack->back();
            pRedoStack->pop_back();
        }
        delete pRedoStack;
        pRedoStack=NULL;
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
        SfxUndoAction* pDo = HasUndoActions() ? pUndoStack->front() : NULL;
        if(pDo!=NULL)
        {
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Undo();
            if(pRedoStack==NULL)
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
        SfxUndoAction* pDo = HasRedoActions() ? pRedoStack->front() : NULL;
        if(pDo!=NULL)
        {
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Redo();
            if(pUndoStack==NULL)
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
        SfxUndoAction* pDo = HasUndoActions() ? pUndoStack->front() : NULL;
        if(pDo!=NULL)
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
    DBG_ASSERT( mpImpl->mpUndoManager == 0, "svx::SdrModel::ImpPostUndoAction(), method not supported with application undo manager!" );
    if( IsUndoEnabled() )
    {
        if (aUndoLink.IsSet())
        {
            aUndoLink.Call(pUndo);
        }
        else
        {
            if (pUndoStack==NULL)
                pUndoStack=new std::deque<SfxUndoAction*>;
            pUndoStack->push_front(pUndo);
            while (pUndoStack->size()>nMaxUndoCount)
            {
                delete pUndoStack->back();
                pUndoStack->pop_back();
            }
            if (pRedoStack!=NULL)
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
        const OUString aEmpty;
        mpImpl->mpUndoManager->EnterListAction(aEmpty,aEmpty);
        nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        if(pAktUndoGroup==NULL)
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
        const OUString aEmpty;
        mpImpl->mpUndoManager->EnterListAction( rComment, aEmpty );
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
        const OUString aEmpty;
        mpImpl->mpUndoManager->EnterListAction( aComment,aEmpty );
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
        if(pAktUndoGroup!=NULL && IsUndoEnabled())
        {
            nUndoLevel--;
            if(nUndoLevel==0)
            {
                if(pAktUndoGroup->GetActionCount()!=0)
                {
                    SdrUndoAction* pUndo=pAktUndoGroup;
                    pAktUndoGroup=NULL;
                    ImpPostUndoAction(pUndo);
                }
                else
                {
                    
                    delete pAktUndoGroup;
                    pAktUndoGroup=NULL;
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
        if (pAktUndoGroup!=NULL)
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

void SdrModel::ClearModel(sal_Bool bCalledFromDestructor)
{
    if(bCalledFromDestructor)
    {
        mbInDestruction = true;
    }

    sal_Int32 i;
    
    sal_Int32 nAnz=GetPageCount();
    for (i=nAnz-1; i>=0; i--)
    {
        DeletePage( (sal_uInt16)i );
    }
    maPages.clear();
    PageListChanged();

    
    nAnz=GetMasterPageCount();
    for(i=nAnz-1; i>=0; i--)
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
    
    SvxFontItem aSvxFontItem( EE_CHAR_FONTINFO) ;
    SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
    SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
    sal_uInt16 nLanguage(Application::GetSettings().GetLanguageTag().getLanguageType());

    
    Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItem.SetFamily(aFont.GetFamily());
    aSvxFontItem.SetFamilyName(aFont.GetName());
    aSvxFontItem.SetStyleName(OUString());
    aSvxFontItem.SetPitch( aFont.GetPitch());
    aSvxFontItem.SetCharSet( aFont.GetCharSet() );
    pItemPool->SetPoolDefaultItem(aSvxFontItem);

    
    Font aFontCJK(OutputDevice::GetDefaultFont(DEFAULTFONT_CJK_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItemCJK.SetFamily( aFontCJK.GetFamily());
    aSvxFontItemCJK.SetFamilyName(aFontCJK.GetName());
    aSvxFontItemCJK.SetStyleName(OUString());
    aSvxFontItemCJK.SetPitch( aFontCJK.GetPitch());
    aSvxFontItemCJK.SetCharSet( aFontCJK.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCJK);

    
    Font aFontCTL(OutputDevice::GetDefaultFont(DEFAULTFONT_CTL_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItemCTL.SetFamily(aFontCTL.GetFamily());
    aSvxFontItemCTL.SetFamilyName(aFontCTL.GetName());
    aSvxFontItemCTL.SetStyleName(OUString());
    aSvxFontItemCTL.SetPitch( aFontCTL.GetPitch() );
    aSvxFontItemCTL.SetCharSet( aFontCTL.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCTL);

    
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CTL ) );

    
    pItemPool->SetPoolDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor(), EE_CHAR_COLOR) );
}

SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
{
    pDrawOutliner->SetTextObj(pObj);
    return *pDrawOutliner;
}

const SdrTextObj* SdrModel::GetFormattingTextObj() const
{
    if (pDrawOutliner!=NULL) {
        return pDrawOutliner->GetTextObj();
    }
    return NULL;
}

void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, sal_Bool bInit )
{
    /**************************************************************************
    * Initialization of the Outliners for drawing text and HitTest
    **************************************************************************/
    if( bInit )
    {
        pOutliner->EraseVirtualDevice();
        pOutliner->SetUpdateMode(sal_False);
        pOutliner->SetEditTextObjectPool(pItemPool);
        pOutliner->SetDefTab(nDefaultTabulator);
    }

    pOutliner->SetRefDevice(GetRefDevice());
    pOutliner->SetForbiddenCharsTable(GetForbiddenCharsTable());
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

void SdrModel::ImpReformatAllTextObjects()
{
    if( isLocked() )
        return;

    sal_uInt16 nAnz=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nAnz; nNum++) {
        GetMasterPage(nNum)->ReformatAllTextObjects();
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++) {
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

    sal_uInt16 nAnz=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nAnz; nNum++)
    {
        GetMasterPage(nNum)->ReformatAllEdgeObjects();
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++)
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
        return 0;
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
        return 0;
    }
    try {
        uno::Reference<io::XStream> const xStream(
            ::comphelper::OStorageHelper::GetStreamAtPackageURL(
                xStorage, rURL, embed::ElementModes::READ, rProxy));
        return (xStream.is()) ? xStream->getInputStream() : 0;
    }
    catch (container::NoSuchElementException const&)
    {
        SAL_INFO("svx", "not found");
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("svx", "exception: '" << e.Message << "'");
    }
    return 0;
}


void SdrModel::BurnInStyleSheetAttributes()
{
    sal_uInt16 nAnz=GetMasterPageCount();
    sal_uInt16 nNum;
    for (nNum=0; nNum<nAnz; nNum++) {
        GetMasterPage(nNum)->BurnInStyleSheetAttributes();
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++) {
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

    
    nUIUnitKomma = 0;
    sal_Int64 nMul(1);
    sal_Int64 nDiv(1);

    
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
        case MAP_POINT      : nDiv=72;     break;          
        case MAP_TWIP       : nDiv=144; nUIUnitKomma++; break; 
        case MAP_PIXEL      : break;
        case MAP_SYSFONT    : break;
        case MAP_APPFONT    : break;
        case MAP_RELATIVE   : break;
        default: break;
    } 

    
    
    
    
    
    
    switch (eUIUnit)
    {
        case FUNIT_NONE   : break;
        
        case FUNIT_100TH_MM: nUIUnitKomma-=5; break;
        case FUNIT_MM     : nUIUnitKomma-=3; break;
        case FUNIT_CM     : nUIUnitKomma-=2; break;
        case FUNIT_M      : nUIUnitKomma+=0; break;
        case FUNIT_KM     : nUIUnitKomma+=3; break;
        
        case FUNIT_TWIP   : nMul=144; nUIUnitKomma--;  break;  
        case FUNIT_POINT  : nMul=72;     break;            
        case FUNIT_PICA   : nMul=6;      break;            
        case FUNIT_INCH   : break;                         
        case FUNIT_FOOT   : nDiv*=12;    break;            
        case FUNIT_MILE   : nDiv*=6336; nUIUnitKomma++; break; 
        
        case FUNIT_CUSTOM : break;
        case FUNIT_PERCENT: nUIUnitKomma+=2; break;
        
        case FUNIT_CHAR   : break;
        case FUNIT_LINE   : break;
        case FUNIT_PIXEL  : break;
        case FUNIT_DEGREE : break;
        case FUNIT_SECOND : break;
        case FUNIT_MILLISECOND : break;
    } 

    
    const bool bMapInch(IsInch(eObjUnit));
    const bool bUIMetr(IsMetric(eUIUnit));

    if (bMapInch && bUIMetr)
    {
        nUIUnitKomma += 4;
        nMul *= 254;
    }

    
    const bool bMapMetr(IsMetric(eObjUnit));
    const bool bUIInch(IsInch(eUIUnit));

    if (bMapMetr && bUIInch)
    {
        nUIUnitKomma -= 4;
        nDiv *= 254;
    }

    
    
    if(1 != nMul || 1 != nDiv)
    {
        const Fraction aTemp(static_cast< long >(nMul), static_cast< long >(nDiv));
        nMul = aTemp.GetNumerator();
        nDiv = aTemp.GetDenominator();
    }

    
    if(1 != aUIScale.GetDenominator() || 1 != aUIScale.GetNumerator())
    {
        
        nMul *= aUIScale.GetDenominator();
        nDiv *= aUIScale.GetNumerator();
    }

    
    while(0 == (nMul % 10))
    {
        nUIUnitKomma--;
        nMul /= 10;
    }

    
    while(0 == (nDiv % 10))
    {
        nUIUnitKomma++;
        nDiv /= 10;
    }

    
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
            rStr = "";
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
        nNumDigits = rLoc.getNumDigits();
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
        
        sal_Int32 nAnz(-nKomma);

        for(sal_Int32 i=0; i<nAnz; i++)
            aBuf.append('0');

        nKomma = 0;
    }

    
    
    if (nKomma > 0 && aBuf.getLength() <= nKomma)
    {
        
        sal_Int32 nAnz = nKomma - aBuf.getLength();

        if(nAnz >= 0 && rLoc.isNumLeadingZero())
            nAnz++;

        for(sal_Int32 i=0; i<nAnz; i++)
            aBuf.insert(0, '0');
    }

    sal_Unicode cDec( rLoc.getNumDecimalSep()[0] );

    
    sal_Int32 nVorKomma = aBuf.getLength() - nKomma;

    if(nKomma > 0)
        aBuf.insert(nVorKomma, cDec);

    if(!rLoc.isNumTrailingZeros())
    {
        
        while (!aBuf.isEmpty() && aBuf[aBuf.getLength()-1] == '0')
            aBuf.remove(aBuf.getLength()-1, 1);

        
        if (!aBuf.isEmpty() && aBuf[aBuf.getLength()-1] == cDec)
            aBuf.remove(aBuf.getLength()-1, 1);
    }

    
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

void SdrModel::TakeWinkStr(long nWink, OUString& rStr, bool bNoDegChar) const
{
    bool bNeg = nWink < 0;

    if(bNeg)
        nWink = -nWink;

    OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(nWink));

    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();
    sal_Int32 nAnz = 2;

    if(rLoc.isNumLeadingZero())
        nAnz++;

    while(aBuf.getLength() < nAnz)
        aBuf.insert(0, '0');

    aBuf.insert(aBuf.getLength()-2, rLoc.getNumDecimalSep()[0]);

    if(bNeg)
        aBuf.insert(0, '-');

    if(!bNoDegChar)
        aBuf.append(DEGREE_CHAR);

    rStr = aBuf.makeStringAndClear();
}

void SdrModel::TakePercentStr(const Fraction& rVal, OUString& rStr, bool bNoPercentChar) const
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
        sal_uInt16 nAnz=sal_uInt16(maMaPag.size());
        sal_uInt16 i;
        for (i=0; i<nAnz; i++) {
            SdrPage* pPg=maMaPag[i];
            pPg->SetPageNum(i);
        }
        bMPgNumsDirty=false;
    }
    else
    {
        sal_uInt16 nAnz=sal_uInt16(maPages.size());
        sal_uInt16 i;
        for (i=0; i<nAnz; i++) {
            SdrPage* pPg=maPages[i];
            pPg->SetPageNum(i);
        }
        bPagNumsDirty=false;
    }
}

void SdrModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nAnz=GetPageCount();
    if (nPos>nAnz) nPos=nAnz;
    maPages.insert(maPages.begin()+nPos,pPage);
    PageListChanged();
    pPage->SetInserted(true);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) bPagNumsDirty=true;
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
    if (pPg!=NULL) {
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
    if (pPg!=NULL) {
        maPages.erase(maPages.begin()+nPgNum); 
        PageListChanged();
        pPg->SetInserted(false);
        InsertPage(pPg,nNewPos);
    }
    else
        RemovePage(nPgNum);
}

void SdrModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nAnz=GetMasterPageCount();
    if (nPos>nAnz) nPos=nAnz;
    maMaPag.insert(maMaPag.begin()+nPos,pPage);
    MasterPageListChanged();
    pPage->SetInserted(true);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) {
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
    if (pPg!=NULL) {
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

    
    sal_uInt16 nPageNum=nFirstPageNum;
    sal_uInt16 nCopyAnz=((!bReverse)?(nLastPageNum-nFirstPageNum):(nFirstPageNum-nLastPageNum))+1;
    SdrPage** pPagePtrs=new SdrPage*[nCopyAnz];
    sal_uInt16 nCopyNum;
    for(nCopyNum=0; nCopyNum<nCopyAnz; nCopyNum++)
    {
        pPagePtrs[nCopyNum]=GetPage(nPageNum);
        if (bReverse)
            nPageNum--;
        else
            nPageNum++;
    }

    
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

    delete[] pPagePtrs;
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

    sal_uInt16*   pMasterMap=NULL;
    bool* pMasterNeed=NULL;
    sal_uInt16    nMasterNeed=0;
    if (bMergeMasterPages && nSrcMasterPageAnz!=0) {
        
        pMasterMap=new sal_uInt16[nSrcMasterPageAnz];
        pMasterNeed=new bool[nSrcMasterPageAnz];
        memset(pMasterMap,0xFF,nSrcMasterPageAnz*sizeof(sal_uInt16));
        if (bAllMasterPages) {
            memset(pMasterNeed, true, nSrcMasterPageAnz * sizeof(bool));
        } else {
            memset(pMasterNeed, false, nSrcMasterPageAnz * sizeof(bool));
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
        
        sal_uInt16 nAktMaPagNum=nDstMasterPageAnz;
        for (sal_uInt16 i=0; i<nSrcMasterPageAnz; i++) {
            if (pMasterNeed[i]) {
                pMasterMap[i]=nAktMaPagNum;
                nAktMaPagNum++;
                nMasterNeed++;
            }
        }
    }

    
    if (pMasterMap!=NULL && pMasterNeed!=NULL && nMasterNeed!=0) {
        for (sal_uInt16 i=nSrcMasterPageAnz; i>0;) {
            i--;
            if (pMasterNeed[i]) {
                SdrPage* pPg=NULL;
                if (bTreadSourceAsConst) {
                    const SdrPage* pPg1=rSourceModel.GetMasterPage(i);
                    pPg=pPg1->Clone();
                } else {
                    pPg=rSourceModel.RemoveMasterPage(i);
                }
                if (pPg!=NULL) {
                    
                    
                    
                    maMaPag.insert(maMaPag.begin()+nDstMasterPageAnz, pPg);
                    MasterPageListChanged();
                    pPg->SetInserted(true);
                    pPg->SetModel(this);
                    bMPgNumsDirty=true;
                    if (bUndo) AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pPg));
                } else {
                    OSL_FAIL("SdrModel::Merge(): MasterPage not found in SourceModel.");
                }
            }
        }
    }

    
    if (bInsPages) {
        sal_uInt16 nSourcePos=nFirstPageNum;
        sal_uInt16 nMergeCount=sal_uInt16(std::abs((long)((long)nFirstPageNum-nLastPageNum))+1);
        if (nDestPos>GetPageCount()) nDestPos=GetPageCount();
        while (nMergeCount>0) {
            SdrPage* pPg=NULL;
            if (bTreadSourceAsConst) {
                const SdrPage* pPg1=rSourceModel.GetPage(nSourcePos);
                pPg=pPg1->Clone();
            } else {
                pPg=rSourceModel.RemovePage(nSourcePos);
            }
            if (pPg!=NULL) {
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
                            if(bUndo)
                            {
                                AddUndo(GetSdrUndoFactory().CreateUndoPageChangeMasterPage(*pPg));
                            }

                            pPg->TRG_SetMasterPage(*GetMasterPage(nNeuNum));
                        }
                        DBG_ASSERT(nNeuNum!=0xFFFF,"SdrModel::Merge(): Something is crooked with the mapping of the MasterPages.");
                    } else {
                        if (nMaPgNum>=nDstMasterPageAnz) {
                            
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

    delete [] pMasterMap;
    delete [] pMasterNeed;

    bMPgNumsDirty=true;
    bPagNumsDirty=true;

    SetChanged();
    
    
    if (bUndo) EndUndo();
}

void SdrModel::SetStarDrawPreviewMode(sal_Bool bPreview)
{
    if (!bPreview && bStarDrawPreviewMode && GetPageCount())
    {
        
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

void SdrModel::setUnoModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xModel )
{
    mxUnoModel = xModel;
}

uno::Reference< uno::XInterface > SdrModel::createUnoModel()
{
    OSL_FAIL( "SdrModel::createUnoModel() - base implementation should not be called!" );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt;
    return xInt;
}

void SdrModel::setLock( bool bLock )
{
    if( mbModelLocked != bLock )
    {
        
        mbModelLocked = bLock;

        if( !bLock )
        {
            ImpReformatAllEdgeObjects();
        }
    }
}



void SdrModel::MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel )
{
    if( pSourceSet && pDestSet && (pSourceSet != pDestSet ) )
    {
        if( pNewModel == NULL )
            pNewModel = this;

        SfxWhichIter aWhichIter(*pSourceSet);
        sal_uInt16 nWhich(aWhichIter.FirstWhich());
        const SfxPoolItem *pPoolItem;

        while(nWhich)
        {
            if(SFX_ITEM_SET == pSourceSet->GetItemState(nWhich, false, &pPoolItem))
            {
                const SfxPoolItem* pItem = pPoolItem;

                switch( nWhich )
                {
                case XATTR_FILLBITMAP:
                    pItem = ((XFillBitmapItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINEDASH:
                    pItem = ((XLineDashItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINESTART:
                    pItem = ((XLineStartItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_LINEEND:
                    pItem = ((XLineEndItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLGRADIENT:
                    pItem = ((XFillGradientItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLFLOATTRANSPARENCE:
                    
                    pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLHATCH:
                    pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                }

                
                if( pItem )
                {
                    pDestSet->Put(*pItem);

                    
                    if( pItem != pPoolItem)
                        delete (SfxPoolItem*)pItem;
                }
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

rtl::Reference<SvxForbiddenCharactersTable> SdrModel::GetForbiddenCharsTable() const
{
    return mpForbiddenCharactersTable;
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

void SdrModel::SetKernAsianPunctuation( sal_Bool bEnabled )
{
    if( mbKernAsianPunctuation != (bool) bEnabled )
    {
        mbKernAsianPunctuation = bEnabled;
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
    }
}

void SdrModel::SetAddExtLeading( sal_Bool bEnabled )
{
    if( mbAddExtLeading != (bool) bEnabled )
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
    if( NULL == mpOutlinerCache )
        mpOutlinerCache = new SdrOutlinerCache(this);

    return mpOutlinerCache->createOutliner( nOutlinerMode );
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
    return nPgNum < maPages.size() ? maPages[nPgNum] : NULL;
}

SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum)
{
    DBG_ASSERT(nPgNum < maPages.size(), "SdrModel::GetPage: Access out of range (!)");
    return nPgNum < maPages.size() ? maPages[nPgNum] : NULL;
}

sal_uInt16 SdrModel::GetPageCount() const
{
    return sal_uInt16(maPages.size());
}

void SdrModel::PageListChanged()
{
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

namespace
{
    class theSdrModelUnoTunnelImplementationId : public rtl::Static< UnoTunnelIdInit, theSdrModelUnoTunnelImplementationId > {};
}

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdrModel::getUnoTunnelImplementationId()
{
    return theSdrModelUnoTunnelImplementationId::get().getSeq();
}



TYPEINIT1(SdrHint,SfxHint);

SdrHint::SdrHint(SdrHintKind eNewHint)
:   mpPage(0L),
    mpObj(0L),
    mpObjList(0L),
    meHint(eNewHint)
{
}

SdrHint::SdrHint(const SdrObject& rNewObj)
:   mpPage(rNewObj.GetPage()),
    mpObj(&rNewObj),
    mpObjList(rNewObj.GetObjList()),
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

const SdrPage* SdrHint::GetPage() const
{
    return mpPage;
}

const SdrObject* SdrHint::GetObject() const
{
    return mpObj;
}

SdrHintKind SdrHint::GetKind() const
{
    return meHint;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
