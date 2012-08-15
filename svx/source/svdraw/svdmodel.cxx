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


#include <svx/svdmodel.hxx>

#include <math.h>

#include <osl/endian.h>
#include <rtl/logfile.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/oustringostreaminserter.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <unotools/ucbstreamhelper.hxx>

#include <tools/string.hxx>
#include <svl/whiter.hxx>
#include <svx/xit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>

#include "svx/svditext.hxx"
#include <editeng/editeng.hxx>   // for EditEngine::CreatePool()

#include <svx/xtable.hxx>

#include "svx/svditer.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>  // for ReformatAllTextObjects and CalcFieldValue
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdoole2.hxx>
#include "svx/svdglob.hxx"  // StringCache
#include "svx/svdstr.hrc"   // object's name
#include "svdoutlinercache.hxx"

#include "svx/xflclit.hxx"
#include "svx/xflhtit.hxx"
#include "svx/xlnclit.hxx"

#include "officecfg/Office/Common.hxx"
#include "editeng/fontitem.hxx"
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/outlobj.hxx>
#include "editeng/forbiddencharacterstable.hxx"
#include <svl/zforlist.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>

#include <tools/tenccvt.hxx>
#include <unotools/syslocale.hxx>

#include <vcl/svapp.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SdrModelImpl
{
    SfxUndoManager* mpUndoManager;
    SdrUndoFactory* mpUndoFactory;
    bool mbAllowShapePropertyChangeListener;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrModel)
TYPEINIT1(SdrModel,SfxBroadcaster);
void SdrModel::ImpCtor(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* _pEmbeddedHelper,
    bool bUseExtColorTable, bool bLoadRefCounts)
{
    mpImpl = new SdrModelImpl;
    mpImpl->mpUndoManager=0;
    mpImpl->mpUndoFactory=0;
    mpImpl->mbAllowShapePropertyChangeListener = false;
    mbInDestruction = false;
    aObjUnit=SdrEngineDefaults::GetMapFraction();
    eObjUnit=SdrEngineDefaults::GetMapUnit();
    eUIUnit=FUNIT_MM;
    aUIScale=Fraction(1,1);
    nUIUnitKomma=0;
    bUIOnlyKomma=sal_False;
    pLayerAdmin=NULL;
    pItemPool=pPool;
    bMyPool=sal_False;
    m_pEmbeddedHelper=_pEmbeddedHelper;
    pDrawOutliner=NULL;
    pHitTestOutliner=NULL;
    pRefOutDev=NULL;
    nProgressAkt=0;
    nProgressMax=0;
    nProgressOfs=0;
    pDefaultStyleSheet=NULL;
    pLinkManager=NULL;
    pUndoStack=NULL;
    pRedoStack=NULL;
    nMaxUndoCount=16;
    pAktUndoGroup=NULL;
    nUndoLevel=0;
    mbUndoEnabled=true;
    nProgressPercent=0;
    nLoadVersion=0;
    bExtColorTable=sal_False;
    mbChanged = sal_False;
    bInfoChanged=sal_False;
    bPagNumsDirty=sal_False;
    bMPgNumsDirty=sal_False;
    bPageNotValid=sal_False;
    bSavePortable=sal_False;
    bSaveCompressed=sal_False;
    bSaveNative=sal_False;
    bSwapGraphics=sal_False;
    nSwapGraphicsMode=SDR_SWAPGRAPHICSMODE_DEFAULT;
    bSaveOLEPreview=sal_False;
    bPasteResize=sal_False;
    bNoBitmapCaching=sal_False;
    bReadOnly=sal_False;
    nStreamCompressMode=COMPRESSMODE_NONE;
    nStreamNumberFormat=NUMBERFORMAT_INT_BIGENDIAN;
    nDefaultTabulator=0;
    mpNumberFormatter = NULL;
    bTransparentTextFrames=sal_False;
    bStarDrawPreviewMode = sal_False;
    nStarDrawPreviewMasterPageNum = SDRPAGE_NOTFOUND;
    mpForbiddenCharactersTable = NULL;
    mbModelLocked = false;
    mpOutlinerCache = NULL;
    mbKernAsianPunctuation = sal_False;
    mbAddExtLeading = sal_False;
    mnHandoutPageCount = 0;

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
        // Outliner doesn't have its own Pool, so use the EditEngine's
        SfxItemPool* pOutlPool=EditEngine::CreatePool( bLoadRefCounts );
        // OutlinerPool as SecondaryPool of SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
        // remember that I created both pools myself
        bMyPool=sal_True;
    }
    pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);

// using static SdrEngineDefaults only if default SvxFontHeight item is not available
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

    // can't create DrawOutliner OnDemand, because I can't get the Pool,
    // then (only from 302 onwards!)
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
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
#endif

    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool, pPers, false, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages(),
    aTablePath(rPath)
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
#endif

    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool, pPers, false, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages()
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
#endif

    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,bUseExtColorTable, (bool)bLoadRefCounts);
}

SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts):
    aReadDate( DateTime::EMPTY ),
    maMaPag(),
    maPages(),
    aTablePath(rPath)
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
#endif

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
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
#endif

    // not yet implemented
    OSL_FAIL("SdrModel::CopyCtor() is not yet implemented.");
}

SdrModel::~SdrModel()
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::~SdrModel(...)" );
#endif

    DBG_DTOR(SdrModel,NULL);

    mbInDestruction = true;

    Broadcast(SdrHint(HINT_MODELCLEARED));

    delete mpOutlinerCache;

    ClearUndoBuffer();
#ifdef DBG_UTIL
    if(pAktUndoGroup)
    {
        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
            "In the Dtor of the SdrModel there is an open Undo left: \""));
        aStr.append(rtl::OUStringToOString(pAktUndoGroup->GetComment(), osl_getThreadTextEncoding()))
            .append('\"');
        OSL_FAIL(aStr.getStr());
    }
#endif
    if (pAktUndoGroup!=NULL)
        delete pAktUndoGroup;

    ClearModel(sal_True);

    delete pLayerAdmin;

    // Delete DrawOutliner only after deleting ItemPool, because ItemPool
    // references Items of the DrawOutliner!
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

    if(mpNumberFormatter)
        delete mpNumberFormatter;

    delete mpImpl->mpUndoFactory;
    delete mpImpl;
}

bool SdrModel::IsInDestruction() const
{
    return mbInDestruction;
}

// not yet implemented
void SdrModel::operator=(const SdrModel&)
{
    OSL_FAIL("SdrModel::operator=() is not yet implemented.");
}

bool SdrModel::operator==(const SdrModel&) const
{
    OSL_FAIL("SdrModel::operator==() is not yet implemented");
    return sal_False;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

bool SdrModel::Undo()
{
    bool bRet = false;
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::Undo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo=(SfxUndoAction*)GetUndoAction(0);
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
        SfxUndoAction* pDo=(SfxUndoAction*)GetRedoAction(0);
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
        SfxUndoAction* pDo=(SfxUndoAction*)GetUndoAction(0);
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
        const String aEmpty;
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

void SdrModel::BegUndo(const XubString& rComment)
{
    if( mpImpl->mpUndoManager )
    {
        const String aEmpty;
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

void SdrModel::BegUndo(const XubString& rComment, const XubString& rObjDescr, SdrRepeatFunc eFunc)
{
    if( mpImpl->mpUndoManager )
    {
        String aComment(rComment);
        if( aComment.Len() && rObjDescr.Len() )
        {
            String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%1"));
            aComment.SearchAndReplace(aSearchString, rObjDescr);
        }
        const String aEmpty;
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
                    // was empty
                    delete pAktUndoGroup;
                    pAktUndoGroup=NULL;
                }
            }
        }
    }
}

void SdrModel::SetUndoComment(const XubString& rComment)
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

void SdrModel::SetUndoComment(const XubString& rComment, const XubString& rObjDescr)
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
                (XPropertyListType) i, aTablePath, (XOutdevItemPool*)pItemPool );
    }
}

void SdrModel::ClearModel(sal_Bool bCalledFromDestructor)
{
    if(bCalledFromDestructor)
    {
        mbInDestruction = true;
    }

    sal_Int32 i;
    // delete all drawing pages
    sal_Int32 nAnz=GetPageCount();
    for (i=nAnz-1; i>=0; i--)
    {
        DeletePage( (sal_uInt16)i );
    }
    maPages.clear();
    PageListChanged();

    // delete all Masterpages
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
    // set application-language specific dynamic pool language defaults
    SvxFontItem aSvxFontItem( EE_CHAR_FONTINFO) ;
    SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
    SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
    sal_uInt16 nLanguage(Application::GetSettings().GetLanguage());

    // get DEFAULTFONT_LATIN_TEXT and set at pool as dynamic default
    Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItem.SetFamily(aFont.GetFamily());
    aSvxFontItem.SetFamilyName(aFont.GetName());
    aSvxFontItem.SetStyleName(String());
    aSvxFontItem.SetPitch( aFont.GetPitch());
    aSvxFontItem.SetCharSet( aFont.GetCharSet() );
    pItemPool->SetPoolDefaultItem(aSvxFontItem);

    // get DEFAULTFONT_CJK_TEXT and set at pool as dynamic default
    Font aFontCJK(OutputDevice::GetDefaultFont(DEFAULTFONT_CJK_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItemCJK.SetFamily( aFontCJK.GetFamily());
    aSvxFontItemCJK.SetFamilyName(aFontCJK.GetName());
    aSvxFontItemCJK.SetStyleName(String());
    aSvxFontItemCJK.SetPitch( aFontCJK.GetPitch());
    aSvxFontItemCJK.SetCharSet( aFontCJK.GetCharSet());
    pItemPool->SetPoolDefaultItem(aSvxFontItemCJK);

    // get DEFAULTFONT_CTL_TEXT and set at pool as dynamic default
    Font aFontCTL(OutputDevice::GetDefaultFont(DEFAULTFONT_CTL_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
    aSvxFontItemCTL.SetFamily(aFontCTL.GetFamily());
    aSvxFontItemCTL.SetFamilyName(aFontCTL.GetName());
    aSvxFontItemCTL.SetStyleName(String());
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
SdrModel::GetDocumentStream( ::rtl::OUString const& rURL,
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

// convert template attributes from the string into "hard" attributes
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

void SdrModel::TakeUnitStr(FieldUnit eUnit, XubString& rStr)
{
    switch(eUnit)
    {
        default:
        case FUNIT_NONE   :
        case FUNIT_CUSTOM :
        {
            rStr = String();
            break;
        }
        case FUNIT_100TH_MM:
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("/100mm"));
            break;
        }
        case FUNIT_MM     :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("mm"));
            break;
        }
        case FUNIT_CM     :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("cm"));
            break;
        }
        case FUNIT_M      :
        {
            rStr = String();
            rStr += sal_Unicode('m');
            break;
        }
        case FUNIT_KM     :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("km"));
            break;
        }
        case FUNIT_TWIP   :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("twip"));
            break;
        }
        case FUNIT_POINT  :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("pt"));
            break;
        }
        case FUNIT_PICA   :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("pica"));
            break;
        }
        case FUNIT_INCH   :
        {
            rStr = String();
            rStr += sal_Unicode('"');
            break;
        }
        case FUNIT_FOOT   :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("ft"));
            break;
        }
        case FUNIT_MILE   :
        {
            rStr = UniString(RTL_CONSTASCII_USTRINGPARAM("mile(s)"));
            break;
        }
        case FUNIT_PERCENT:
        {
            rStr = String();
            rStr += sal_Unicode('%');
            break;
        }
    }
}

void SdrModel::TakeMetricStr(long nVal, rtl::OUString& rStr, bool bNoUnitChars, sal_Int32 nNumDigits) const
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

    rtl::OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(fLocalValue + 0.5));

    if(nKomma < 0)
    {
        // negative nKomma (decimal point) means: add zeros
        sal_Int32 nAnz(-nKomma);

        for(sal_Int32 i=0; i<nAnz; i++)
            aBuf.append(sal_Unicode('0'));

        nKomma = 0;
    }

    // the second condition needs to be <= since inside this loop
    // also the leading zero is inserted.
    if (nKomma > 0 && aBuf.getLength() <= nKomma)
    {
        // if necessary, add zeros before the decimal point
        sal_Int32 nAnz = nKomma - aBuf.getLength();

        if(nAnz >= 0 && rLoc.isNumLeadingZero())
            nAnz++;

        for(sal_Int32 i=0; i<nAnz; i++)
            aBuf.insert(0, sal_Unicode('0'));
    }

    sal_Unicode cDec( rLoc.getNumDecimalSep()[0] );

    // insert KommaChar (decimal point character)
    sal_Int32 nVorKomma = aBuf.getLength() - nKomma;

    if(nKomma > 0)
        aBuf.insert(nVorKomma, cDec);

    if(!rLoc.isNumTrailingZeros())
    {
        // Remove all trailing zeros.
        while (aBuf.getLength() && aBuf[aBuf.getLength()-1] == sal_Unicode('0'))
            aBuf.remove(aBuf.getLength()-1, 1);

        // Remove decimal if it's the last character.
        if (aBuf.getLength() && aBuf[aBuf.getLength()-1] == cDec)
            aBuf.remove(aBuf.getLength()-1, 1);
    }

    // if necessary, add separators before every third digit
    if( nVorKomma > 3 )
    {
        String aThoSep( rLoc.getNumThousandSep() );
        if ( aThoSep.Len() > 0 )
        {
            sal_Unicode cTho( aThoSep.GetChar(0) );
            sal_Int32 i(nVorKomma - 3);

            while(i > 0)
            {
                aBuf.insert(i, cTho);
                i -= 3;
            }
        }
    }

    if (!aBuf.getLength())
        aBuf.append(sal_Unicode('0'));

    if(bNegative)
    {
        aBuf.insert(0, sal_Unicode('-'));
    }

    if(!bNoUnitChars)
        aBuf.append(aUIUnitStr);

    rStr = aBuf.makeStringAndClear();
}

void SdrModel::TakeWinkStr(long nWink, rtl::OUString& rStr, bool bNoDegChar) const
{
    bool bNeg = nWink < 0;

    if(bNeg)
        nWink = -nWink;

    rtl::OUStringBuffer aBuf;
    aBuf.append(static_cast<sal_Int32>(nWink));

    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();
    sal_Int32 nAnz = 2;

    if(rLoc.isNumLeadingZero())
        nAnz++;

    while(aBuf.getLength() < nAnz)
        aBuf.insert(0, sal_Unicode('0'));

    aBuf.insert(aBuf.getLength()-2, rLoc.getNumDecimalSep()[0]);

    if(bNeg)
        aBuf.insert(0, sal_Unicode('-'));

    if(!bNoDegChar)
        aBuf.append(DEGREE_CHAR);

    rStr = aBuf.makeStringAndClear();
}

void SdrModel::TakePercentStr(const Fraction& rVal, XubString& rStr, bool bNoPercentChar) const
{
    sal_Int32 nMul(rVal.GetNumerator());
    sal_Int32 nDiv(rVal.GetDenominator());
    sal_Bool bNeg(nMul < 0);

    if(nDiv < 0)
        bNeg = !bNeg;

    if(nMul < 0)
        nMul = -nMul;

    if(nDiv < 0)
        nDiv = -nDiv;

    nMul *= 100;
    nMul += nDiv/2;
    nMul /= nDiv;

    rStr = UniString::CreateFromInt32(nMul);

    if(bNeg)
        rStr.Insert(sal_Unicode('-'), 0);

    if(!bNoPercentChar)
        rStr += sal_Unicode('%');
}

void SdrModel::SetChanged(sal_Bool bFlg)
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
        bMPgNumsDirty=sal_False;
    }
    else
    {
        sal_uInt16 nAnz=sal_uInt16(maPages.size());
        sal_uInt16 i;
        for (i=0; i<nAnz; i++) {
            SdrPage* pPg=maPages[i];
            pPg->SetPageNum(i);
        }
        bPagNumsDirty=sal_False;
    }
}

void SdrModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nAnz=GetPageCount();
    if (nPos>nAnz) nPos=nAnz;
    maPages.insert(maPages.begin()+nPos,pPage);
    PageListChanged();
    pPage->SetInserted(sal_True);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) bPagNumsDirty=sal_True;
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
        pPg->SetInserted(sal_False);
    }
    bPagNumsDirty=sal_True;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
    return pPg;
}

void SdrModel::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    SdrPage* pPg=maPages[nPgNum];
    maPages.erase(maPages.begin()+nPgNum);
    PageListChanged();
    if (pPg!=NULL) {
        pPg->SetInserted(sal_False);
        InsertPage(pPg,nNewPos);
    }
}

void SdrModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nAnz=GetMasterPageCount();
    if (nPos>nAnz) nPos=nAnz;
    maMaPag.insert(maMaPag.begin()+nPos,pPage);
    MasterPageListChanged();
    pPage->SetInserted(sal_True);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) {
        bMPgNumsDirty=sal_True;
    }
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPage);
    Broadcast(aHint);
}

void SdrModel::DeleteMasterPage(sal_uInt16 nPgNum)
{
    SdrPage* pPg=RemoveMasterPage(nPgNum);
    if (pPg!=NULL) delete pPg;
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

        pRetPg->SetInserted(sal_False);
    }

    bMPgNumsDirty=sal_True;
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
        pPg->SetInserted(sal_False);
        maMaPag.insert(maMaPag.begin()+nNewPos,pPg);
        MasterPageListChanged();
    }
    bMPgNumsDirty=sal_True;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
        // determine which MasterPages from rSrcModel we need
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
                    // Now append all of them to the end of the DstModel.
                    // Don't use InsertMasterPage(), because everything is
                    // inconsistent until all are in.
                    maMaPag.insert(maMaPag.begin()+nDstMasterPageAnz, pPg);
                    MasterPageListChanged();
                    pPg->SetInserted(sal_True);
                    pPg->SetModel(this);
                    bMPgNumsDirty=sal_True;
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
        sal_uInt16 nMergeCount=sal_uInt16(Abs((long)((long)nFirstPageNum-nLastPageNum))+1);
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

    delete [] pMasterMap;
    delete [] pMasterNeed;

    bMPgNumsDirty=sal_True;
    bPagNumsDirty=sal_True;

    SetChanged();
    // TODO: Missing: merging and mapping of layers
    // at the objects as well as at the MasterPageDescriptors
    if (bUndo) EndUndo();
}

void SdrModel::SetStarDrawPreviewMode(sal_Bool bPreview)
{
    if (!bPreview && bStarDrawPreviewMode && GetPageCount())
    {
        // Resetting is not allowed, because the Model might not be loaded completely
        DBG_ASSERT(sal_False,"SdrModel::SetStarDrawPreviewMode(): Resetting not allowed, because Model might not be complete.");
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
        if( sal_False == bLock )
        {
            ImpReformatAllEdgeObjects();
        }
        mbModelLocked = bLock;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
            if(SFX_ITEM_SET == pSourceSet->GetItemState(nWhich, sal_False, &pPoolItem))
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
                    // allow all kinds of XFillFloatTransparenceItem to be set
                    pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                case XATTR_FILLHATCH:
                    pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pNewModel );
                    break;
                }

                // set item
                if( pItem )
                {
                    pDestSet->Put(*pItem);

                    // delete item if it was a generated one
                    if( pItem != pPoolItem)
                        delete (SfxPoolItem*)pItem;
                }
            }
            nWhich = aWhichIter.NextWhich();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    return maPages[nPgNum];
}

SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum)
{
    DBG_ASSERT(nPgNum < maPages.size(), "SdrModel::GetPage: Access out of range (!)");
    return maPages[nPgNum];
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

/* added for the reporting engine, but does not work
   correctly, so only enable it for this model */
void SdrModel::SetAllowShapePropertyChangeListener( bool bAllow )
{
    if( mpImpl )
    {
        mpImpl->mbAllowShapePropertyChangeListener = bAllow;
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

void SdrModel::SetDrawingLayerPoolDefaults()
{
    const String aNullStr;
    const Color aNullLineCol(COL_DEFAULT_SHAPE_STROKE);
    const Color aNullFillCol(COL_DEFAULT_SHAPE_FILLING);
    const XHatch aNullHatch(aNullLineCol);

    pItemPool->SetPoolDefaultItem( XFillColorItem(aNullStr,aNullFillCol) );
    pItemPool->SetPoolDefaultItem( XFillHatchItem(pItemPool,aNullHatch) );
    pItemPool->SetPoolDefaultItem( XLineColorItem(aNullStr,aNullLineCol) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
