/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdmodel.hxx>
#include <rtl/uuid.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/endian.h>
#include <rtl/logfile.hxx>
#include <math.h>
#include <tools/urlobj.hxx>
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
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <editeng/editeng.hxx>   // Fuer EditEngine::CreatePool()
#include <svx/xtable.hxx>
#include <svx/svditer.hxx>
#include "svx/svditer.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>  // fuer ReformatAllTextObjects und CalcFieldValue
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdoole2.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include "svdoutlinercache.hxx"
#include <svl/asiancfg.hxx>
#include "editeng/fontitem.hxx"
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <tools/bigint.hxx>
#include <editeng/numitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/outlobj.hxx>
#include "editeng/forbiddencharacterstable.hxx"
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/tenccvt.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdview.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow changing page number at SdrPage, but only from SdrModel

void SVX_DLLPRIVATE SetPageNumberAtSdrPageFromSdrModel(SdrPage& rPage, sal_uInt32 nPageNum)
{
    rPage.SetPageNumber(nPageNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow changing SdrModel at SdrPage, but only from SdrModel

void SVX_DLLPRIVATE SetInsertedAtSdrPageFromSdrModel(SdrPage& rPage, bool bInserted)
{
    rPage.SetInserted(bInserted);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable)
:   mxUnoModel(),
    maMasterPageVector(),
    maPageVector(),
    mpModelLayerAdmin(0),
    mpItemPool(pPool),
    maUndoLink(),
    mpUndoStack(0),
    mpRedoStack(0),
    mpCurrentUndoGroup(0),
    mnUndoLevel(0),
    mnMaxUndoCount(16),
    mpUndoManager(0),
    mpUndoFactory(0),
    maMaxObjectScale(0.0, 0.0),
    maExchangeObjectScale(SdrEngineDefaults::GetMapFraction()),
    meExchangeObjectUnit(SdrEngineDefaults::GetMapUnit()),
    meUIUnit(FUNIT_MM),
    maUIScale(1, 1),
    maUIUnitString(),
    maUIUnitScale(),
    mnUIUnitKomma(0),
    mpDrawOutliner(0),
    mpOutlinerCache(0),
    mxStyleSheetPool(),
    mpDefaultStyleSheet(0),
    mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(0),
    m_pEmbeddedHelper(pPers),
    mpReferenceOutputDevice(0),
    mpLinkManager(0),
    mpForbiddenCharactersTable(0),
    mpNumberFormatter(0),
    mnDefaultFontHeight(SdrEngineDefaults::GetFontHeight()),
    mnSwapGraphicsMode(SDR_SWAPGRAPHICSMODE_DEFAULT),
    mnDefaultTabulator(0),
    mnCharCompressType(0),
    maTablePath(rPath),
    mpColorTable(0),
    mpDashList(0),
    mpLineEndList(0),
    mpHatchList(0),
    mpGradientList(0),
    mpBitmapList(0),
    mnHandoutPageCount(0),
    mbDeletePool(false),
    mbUndoEnabled(true),
    mbChanged(false),
    mbExternalColorTable(bUseExtColorTable),
    mbReadOnly(false),
    mbPickThroughTransparentTextFrames(false),
    mbSwapGraphics(false),
    mbStarDrawPreviewMode(false),
    mbModelLocked(false),
    mbKernAsianPunctuation(false),
    mbAddExtLeading(false),
    mbInDestruction(false)
{
    SvxAsianConfig aAsian;
    mnCharCompressType = aAsian.GetCharDistanceCompression();

    if(!mpItemPool)
    {
        mpItemPool = new SdrItemPool(0, false);
        SfxItemPool* pOutlPool = EditEngine::CreatePool(false);
        GetItemPool().SetSecondaryPool(pOutlPool);
        mbDeletePool = true;
    }

    GetItemPool().SetDefaultMetric((SfxMapUnit)GetExchangeObjectUnit());

    // SJ: #95129# using static SdrEngineDefaults only if default SvxFontHeight item is not available
    const SfxPoolItem* pPoolItem = GetItemPool().GetPoolDefaultItem( EE_CHAR_FONTHEIGHT );

    if(pPoolItem)
    {
        mnDefaultFontHeight = ((SvxFontHeightItem*)pPoolItem)->GetHeight();
    }

    SetTextDefaults(&GetItemPool(), GetDefaultFontHeight());
    mpModelLayerAdmin = new SdrLayerAdmin(*this);
    GetItemPool().SetPoolDefaultItem( SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, false) );

    ImpSetUIUnit();

    mpDrawOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, this);
    ImpSetOutlinerDefaults(mpDrawOutliner, true);

    XOutdevItemPool* pXOutdevItemPool = dynamic_cast< XOutdevItemPool* >(&GetItemPool());

    if(!mbExternalColorTable)
    {
        mpColorTable = new XColorTable(maTablePath, pXOutdevItemPool);
    }

    mpDashList = new XDashList(maTablePath, pXOutdevItemPool);
    mpLineEndList = new XLineEndList(maTablePath, pXOutdevItemPool);
    mpHatchList = new XHatchList(maTablePath, pXOutdevItemPool);
    mpGradientList = new XGradientList(maTablePath, pXOutdevItemPool);
    mpBitmapList = new XBitmapList(maTablePath, pXOutdevItemPool);
}

SdrModel::~SdrModel()
{
    mbInDestruction = true;

    Broadcast(SdrBaseHint(HINT_MODELCLEARED));

    delete mpOutlinerCache;

    ClearUndoBuffer();

#ifdef DBG_UTIL
    if(mpCurrentUndoGroup)
    {
        ByteString aStr("Im Dtor des SdrModel steht noch ein offenes Undo rum: \"");

        aStr += ByteString(mpCurrentUndoGroup->GetComment(), gsl_getSystemTextEncoding());
        aStr += '\"';

        DBG_ERROR(aStr.GetBuffer());
    }
#endif

    if(mpCurrentUndoGroup)
    {
        delete mpCurrentUndoGroup;
    }

    // #116168#
    ClearModel(true);

    delete mpModelLayerAdmin;

    // Den DrawOutliner erst nach dem ItemPool loeschen, da
    // der ItemPool Items des DrawOutliners referenziert !!! (<- das war mal)
    // Wg. Problem bei Malte Reihenfolge wieder umgestellt.
    // Loeschen des Outliners vor dem loeschen des ItemPools
    delete mpDrawOutliner;

    // delete StyleSheetPool, derived classes should not do this since
    // the DrawingEngine may need it in its destrctor (SB)
    if(GetStyleSheetPool())
    {
        Reference< XComponent > xComponent( dynamic_cast< cppu::OWeakObject* >(GetStyleSheetPool()), UNO_QUERY );

        if(xComponent.is()) try
        {
            xComponent->dispose();
        }
        catch( RuntimeException& )
        {
        }

        mxStyleSheetPool.clear();
    }

    if(mbDeletePool)
    {
        // Pools loeschen, falls es meine sind
        SfxItemPool* pOutlPool = GetItemPool().GetSecondaryPool();
        SfxItemPool::Free(&GetItemPool());

        // Der OutlinerPool muss nach dem ItemPool plattgemacht werden, da der
        // ItemPool SetItems enthaelt die ihrerseits Items des OutlinerPools
        // referenzieren (Joe)
        SfxItemPool::Free(pOutlPool);
    }

    if(mpForbiddenCharactersTable)
    {
        mpForbiddenCharactersTable->release();
    }

    // Tabellen, Listen und Paletten loeschen
    if(!mbExternalColorTable)
    {
        delete mpColorTable;
    }

    delete mpDashList;
    delete mpLineEndList;
    delete mpHatchList;
    delete mpGradientList;
    delete mpBitmapList;

    if(mpNumberFormatter)
    {
        delete mpNumberFormatter;
    }

    delete mpUndoFactory;
}

const SvNumberFormatter& SdrModel::GetNumberFormatter() const
{
    if(!mpNumberFormatter)
    {
        // use cast here since from outside view this IS a const method
        const_cast< SdrModel* >(this)->mpNumberFormatter = new SvNumberFormatter(::comphelper::getProcessServiceFactory(), LANGUAGE_SYSTEM);
    }

    return *mpNumberFormatter;
}

bool SdrModel::IsReadOnly() const
{
    return mbReadOnly;
}

void SdrModel::SetReadOnly(bool bYes)
{
    if(mbReadOnly != bYes)
    {
        mbReadOnly = bYes;
    }
}

void SdrModel::SetMaxUndoActionCount(sal_uInt32 nAnz)
{
    if(nAnz < 1)
    {
        nAnz = 1;
    }

    if(GetMaxUndoActionCount() != nAnz)
    {
        mnMaxUndoCount = nAnz;

        if(mpUndoStack)
        {
            while(mpUndoStack->size() > GetMaxUndoActionCount())
            {
                delete mpUndoStack->back();
                mpUndoStack->pop_back();
            }
        }
    }
}

void SdrModel::ClearUndoBuffer()
{
    if(mpUndoStack)
    {
        while(mpUndoStack->size())
        {
            delete mpUndoStack->back();
            mpUndoStack->pop_back();
        }

        delete mpUndoStack;
        mpUndoStack = 0;
    }

    if(mpRedoStack)
    {
        while(mpRedoStack->size())
        {
            delete mpRedoStack->back();
            mpRedoStack->pop_back();
        }

        delete mpRedoStack;
        mpRedoStack = 0;
    }
}

const SfxUndoAction* SdrModel::GetUndoAction(sal_uInt32 nNum) const
{
    if(mpUndoStack)
    {
        if(nNum < mpUndoStack->size())
        {
            return *(mpUndoStack->begin() + nNum);
        }
        else
        {
            OSL_ENSURE(false, "SdrModel::GetUndoAction access out of range (!)");
        }
    }

    return 0;
}

const SfxUndoAction* SdrModel::GetRedoAction(sal_uInt32 nNum) const
{
    if(mpRedoStack)
    {
        if(nNum < mpRedoStack->size())
        {
            return *(mpRedoStack->begin() + nNum);
        }
        else
        {
            OSL_ENSURE(false, "SdrModel::GetRedoAction access out of range (!)");
        }
    }

    return 0;
}

bool SdrModel::Undo()
{
    if(mpUndoManager)
    {
        DBG_ERROR("svx::SdrModel::Undo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo = (SfxUndoAction*)GetUndoAction(0);

        if(pDo)
        {
            const bool bWasUndoEnabled(mbUndoEnabled);

            mbUndoEnabled = false;
            pDo->Undo();

            if(!mpRedoStack)
            {
                mpRedoStack = new SfxUndoActionContainerType();
            }

            const SfxUndoActionContainerType::iterator aCandidate(mpUndoStack->begin());
            SfxUndoAction* pCandidate = *aCandidate;
            mpUndoStack->erase(aCandidate);
            mpRedoStack->insert(mpRedoStack->begin(), pCandidate);
            mbUndoEnabled = bWasUndoEnabled;

            return true;
        }
    }

    return false;
}

bool SdrModel::Redo()
{
    if(mpUndoManager)
    {
        DBG_ERROR("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo = (SfxUndoAction*)GetRedoAction(0);

        if(pDo)
        {
            const bool bWasUndoEnabled(mbUndoEnabled);

            mbUndoEnabled = false;
            pDo->Redo();

            if(!mpUndoStack)
            {
                mpUndoStack = new SfxUndoActionContainerType();
            }

            const SfxUndoActionContainerType::iterator aCandidate(mpRedoStack->begin());
            SfxUndoAction* pCandidate = *aCandidate;
            mpRedoStack->erase(aCandidate);
            mpUndoStack->insert(mpUndoStack->begin(), pCandidate);
            mbUndoEnabled = bWasUndoEnabled;

            return true;
        }
    }

    return false;
}

bool SdrModel::Repeat(SfxRepeatTarget& rView)
{
    if(mpUndoManager )
    {
        DBG_ERROR("svx::SdrModel::Redo(), method not supported with application undo manager!");
    }
    else
    {
        SfxUndoAction* pDo=(SfxUndoAction*)GetUndoAction(0);
        if(pDo!=NULL)
        {
            if(pDo->CanRepeat(rView))
            {
                pDo->Repeat(rView);

                return true;
            }
        }
    }

    return false;
}

void SdrModel::ImpPostUndoAction(SdrUndoAction* pUndo)
{
    DBG_ASSERT(mpUndoManager == 0, "svx::SdrModel::ImpPostUndoAction(), method not supported with application undo manager!" );

    if(IsUndoEnabled())
    {
        if(GetNotifyUndoActionHdl().IsSet())
        {
            maUndoLink.Call(pUndo);
        }
        else
        {
            if(!mpUndoStack)
            {
                mpUndoStack = new SfxUndoActionContainerType();
            }

            mpUndoStack->insert(mpUndoStack->begin(), pUndo);

            while(mpUndoStack->size() > GetMaxUndoActionCount())
            {
                delete mpUndoStack->back();
                mpUndoStack->pop_back();
            }

            if(mpRedoStack)
            {
                mpRedoStack->clear();
            }
        }
    }
    else
    {
        delete pUndo;
    }
}

void SdrModel::BegUndo()
{
    if(mpUndoManager)
    {
        const String aEmpty;

        mpUndoManager->EnterListAction(aEmpty,aEmpty);
        mnUndoLevel++;
    }
    else if(IsUndoEnabled())
    {
        if(!mpCurrentUndoGroup)
        {
            mpCurrentUndoGroup = new SdrUndoGroup(*this);
            mnUndoLevel = 1;
        }
        else
        {
            mnUndoLevel++;
        }
    }
}

void SdrModel::BegUndo(const XubString& rComment)
{
    if(mpUndoManager)
    {
        const String aEmpty;
        mpUndoManager->EnterListAction(rComment, aEmpty);
        mnUndoLevel++;
    }
    else if(IsUndoEnabled())
    {
        BegUndo();

        if(IsLastEndUndo())
        {
            mpCurrentUndoGroup->SetComment(rComment);
        }
    }
}

void SdrModel::BegUndo(const XubString& rComment, const XubString& rObjDescr, SdrRepeatFunc eFunc)
{
    if(mpUndoManager )
    {
        String aComment(rComment);

        if(aComment.Len() && rObjDescr.Len())
        {
            String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%1"));
            aComment.SearchAndReplace(aSearchString, rObjDescr);
        }

        const String aEmpty;

        mpUndoManager->EnterListAction(aComment, aEmpty);
        mnUndoLevel++;
    }
    else if(IsUndoEnabled())
    {
        BegUndo();

        if(IsLastEndUndo())
        {
            mpCurrentUndoGroup->SetComment(rComment);
            mpCurrentUndoGroup->SetObjDescription(rObjDescr);
            mpCurrentUndoGroup->SetRepeatFunction(eFunc);
        }
    }
}

void SdrModel::BegUndo(SdrUndoGroup* pUndoGrp)
{
    if(mpUndoManager )
    {
        DBG_ERROR("svx::SdrModel::BegUndo(), method not supported with application undo manager!" );
        mnUndoLevel++;
    }
    else if(IsUndoEnabled())
    {
        if(!mpCurrentUndoGroup)
        {
            mpCurrentUndoGroup = pUndoGrp;
            mnUndoLevel = 1;
        }
        else
        {
            delete pUndoGrp;
            mnUndoLevel++;
        }
    }
    else
    {
        delete pUndoGrp;
    }
}

void SdrModel::EndUndo()
{
    DBG_ASSERT(mnUndoLevel != 0,"SdrModel::EndUndo(): UndoLevel is already 0!");

    if(mpUndoManager)
    {
        if(mnUndoLevel)
        {
            mnUndoLevel--;
            mpUndoManager->LeaveListAction();
        }
    }
    else
    {
        if(mpCurrentUndoGroup && IsUndoEnabled())
        {
            mnUndoLevel--;

            if(!mnUndoLevel)
            {
                if(mpCurrentUndoGroup->GetActionCount())
                {
                    SdrUndoAction* pUndo = mpCurrentUndoGroup;
                    mpCurrentUndoGroup = 0;
                    ImpPostUndoAction(pUndo);
                }
                else
                {
                    // was empty
                    delete mpCurrentUndoGroup;
                    mpCurrentUndoGroup = 0;
                }
            }
        }
    }
}

void SdrModel::SetUndoComment(const XubString& rComment)
{
    DBG_ASSERT(mnUndoLevel != 0,"SdrModel::SetUndoComment(): UndoLevel is on level 0!");

    if(mpUndoManager)
    {
        DBG_ERROR("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else if(IsUndoEnabled())
    {
        if(IsLastEndUndo())
        {
            mpCurrentUndoGroup->SetComment(rComment);
        }
    }
}

void SdrModel::SetUndoComment(const XubString& rComment, const XubString& rObjDescr)
{
    DBG_ASSERT(mnUndoLevel != 0,"SdrModel::SetUndoComment(): UndoLevel is 0!");

    if(mpUndoManager)
    {
        DBG_ERROR("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else
    {
        if(IsLastEndUndo())
        {
            mpCurrentUndoGroup->SetComment(rComment);
            mpCurrentUndoGroup->SetObjDescription(rObjDescr);
        }
    }
}

void SdrModel::AddUndo(SdrUndoAction* pUndo)
{
    if(mpUndoManager)
    {
        mpUndoManager->AddUndoAction(pUndo);
    }
    else if(!IsUndoEnabled())
    {
        delete pUndo;
    }
    else
    {
        if(mpCurrentUndoGroup)
        {
            mpCurrentUndoGroup->AddAction(pUndo);
        }
        else
        {
            ImpPostUndoAction(pUndo);
        }
    }
}

void SdrModel::EnableUndo(bool bEnable)
{
    if(mpUndoManager)
    {
        mpUndoManager->EnableUndo(bEnable);
    }
    else
    {
        mbUndoEnabled = bEnable;
    }
}

bool SdrModel::IsUndoEnabled() const
{
    if(mpUndoManager)
    {
        return mpUndoManager->IsUndoEnabled();
    }
    else
    {
        return mbUndoEnabled;
    }
}

void SdrModel::ClearModel(bool bCalledFromDestructor)
{
    if(bCalledFromDestructor)
    {
        mbInDestruction = true;
    }

    // delete all drawing pages
    while(GetPageCount())
    {
        DeletePage(GetPageCount() - 1);
    }

    // delete all Masterpages
    while(GetMasterPageCount())
    {
        DeleteMasterPage(GetMasterPageCount() - 1);
    }

    mpModelLayerAdmin->ClearLayer();
}

SdrModel* SdrModel::AllocModel() const
{
    SdrModel* pModel = new SdrModel;

    pModel->SetExchangeObjectUnit(GetExchangeObjectUnit());
    pModel->SetExchangeObjectScale(GetExchangeObjectScale());

    return pModel;
}

SdrPage* SdrModel::AllocPage(bool bMasterPage)
{
    return new SdrPage(*this, bMasterPage);
}

void ImpGetDefaultFontsLanguage(SvxFontItem& rLatin, SvxFontItem& rAsian, SvxFontItem& rComplex)
{
    const sal_uInt16 nItemCnt(3);

    static struct
    {
        sal_uInt16 nFntType, nLanguage;
    }  aOutTypeArr[ nItemCnt ] = {
        {  DEFAULTFONT_LATIN_TEXT, LANGUAGE_ENGLISH_US },
        {  DEFAULTFONT_CJK_TEXT, LANGUAGE_ENGLISH_US },
        {  DEFAULTFONT_CTL_TEXT, LANGUAGE_ARABIC_SAUDI_ARABIA }
    };

    SvxFontItem* aItemArr[ nItemCnt ] = { &rLatin, &rAsian, &rComplex };

    for(sal_uInt16 n(0); n < nItemCnt; ++n )
    {
        Font aFnt( OutputDevice::GetDefaultFont(
            aOutTypeArr[ n ].nFntType, aOutTypeArr[ n ].nLanguage,
            DEFAULTFONT_FLAGS_ONLYONE, 0 ));
        SvxFontItem* pI = aItemArr[ n ];
        pI->SetFamily( aFnt.GetFamily());
        pI->SetFamilyName( aFnt.GetName());
        pI->SetStyleName( String() );
        pI->SetPitch( aFnt.GetPitch());
        pI->SetCharSet( aFnt.GetCharSet() );
    }
}

void SdrModel::SetTextDefaults( SfxItemPool* pItemPool, sal_uInt32 nDefaultFontHeight )
{
    // #95114# set application-language specific dynamic pool language defaults
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
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem( nDefaultFontHeight, 100, EE_CHAR_FONTHEIGHT ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem( nDefaultFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem( nDefaultFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );

    // set FontColor defaults
    pItemPool->SetPoolDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor(), EE_CHAR_COLOR) );
}

SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
{
    mpDrawOutliner->SetTextObj(pObj);

    return *mpDrawOutliner;
}

boost::shared_ptr< SdrOutliner > SdrModel::CreateDrawOutliner(const SdrTextObj* pObj)
{
    boost::shared_ptr< SdrOutliner > xDrawOutliner(SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, this));
    ImpSetOutlinerDefaults(xDrawOutliner.get(), true);
    xDrawOutliner->SetTextObj(pObj);

    return xDrawOutliner;
}

const SdrTextObj* SdrModel::GetFormattingTextObj() const
{
    if(mpDrawOutliner)
    {
        return mpDrawOutliner->GetTextObj();
    }

    return 0;
}

void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, bool bInit )
{
    /**************************************************************************
    * Initialisierung der Outliner fuer Textausgabe und HitTest
    **************************************************************************/
    if(bInit)
    {
        pOutliner->EraseVirtualDevice();
        pOutliner->SetUpdateMode(false);
        pOutliner->SetEditTextObjectPool(&GetItemPool());
        pOutliner->SetDefTab(GetDefaultTabulator());
    }

    pOutliner->SetRefDevice(GetReferenceDevice());
    pOutliner->SetForbiddenCharsTable(GetForbiddenCharsTable());
    pOutliner->SetAsianCompressionMode( mnCharCompressType );
    pOutliner->SetKernAsianPunctuation( IsKernAsianPunctuation() );
    pOutliner->SetAddExtLeading( IsAddExtLeading() );

    if(!GetReferenceDevice())
    {
        MapMode aMapMode(GetExchangeObjectUnit(), Point(0, 0), GetExchangeObjectScale(), GetExchangeObjectScale());
        pOutliner->SetRefMapMode(aMapMode);
    }
}

void SdrModel::SetReferenceDevice(OutputDevice* pDev)
{
    if(GetReferenceDevice() != pDev)
    {
        mpReferenceOutputDevice = pDev;

        ImpSetOutlinerDefaults(mpDrawOutliner);
        Broadcast(SdrBaseHint(HINT_REFDEVICECHG));
        ReformatAllTextObjects();
    }
}

/** #103122#
    steps over all available pages and sends notify messages to
    all edge objects that are connected to other objects so that
    they may reposition itselfs
*/
void SdrModel::ImpReformatAllEdgeObjects()
{
    if(isLocked())
    {
        return;
    }

    sal_uInt32 nAnz(GetMasterPageCount());
    sal_uInt32 nNum;

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetMasterPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrEdgeObj* pSdrEdgeObj = dynamic_cast< SdrEdgeObj* >(aIter.Next());

            if(pSdrEdgeObj)
            {
                pSdrEdgeObj->ReformatEdge();
            }
        }
    }

    nAnz = GetPageCount();

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrEdgeObj* pSdrEdgeObj = dynamic_cast< SdrEdgeObj* >(aIter.Next());

            if(pSdrEdgeObj)
            {
                pSdrEdgeObj->ReformatEdge();
            }
        }
    }
}

SvStream* SdrModel::GetDocumentStream(SdrDocumentStreamInfo& /*rStreamInfo*/) const
{
    return 0;
}

void SdrModel::BurnInStyleSheetAttributes()
{
    sal_uInt32 nAnz(GetMasterPageCount());
    sal_uInt32 nNum;

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetMasterPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            aIter.Next()->BurnInStyleSheetAttributes();
        }
    }

    nAnz = GetPageCount();

    for(nNum=0; nNum<nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            aIter.Next()->BurnInStyleSheetAttributes();
        }
    }
}

void SdrModel::SetDefaultFontHeight(sal_uInt32 nVal)
{
    if(nVal != GetDefaultFontHeight())
    {
        mnDefaultFontHeight = nVal;

        Broadcast(SdrBaseHint(HINT_DEFFONTHGTCHG));
        ReformatAllTextObjects();
    }
}

void SdrModel::SetDefaultTabulator(sal_uInt16 nVal)
{
    if(GetDefaultTabulator() != nVal)
    {
        mnDefaultTabulator = nVal;

        Outliner& rOutliner = GetDrawOutliner();
        rOutliner.SetDefTab(nVal);
        Broadcast(SdrBaseHint(HINT_DEFAULTTABCHG));
        ReformatAllTextObjects();
    }
}

void SdrModel::ImpSetUIUnit()
{
    if(0 == GetUIScale().GetNumerator() || 0 == GetUIScale().GetDenominator())
    {
        maUIScale = Fraction(1,1);
    }

    // set start values
    mnUIUnitKomma = 0;
    sal_Int64 nMul(1);
    sal_Int64 nDiv(1);

    // normalize on meters resp. inch
    switch (GetExchangeObjectUnit())
    {
        case MAP_100TH_MM   : mnUIUnitKomma += 5; break;
        case MAP_10TH_MM    : mnUIUnitKomma += 4; break;
        case MAP_MM         : mnUIUnitKomma += 3; break;
        case MAP_CM         : mnUIUnitKomma += 2; break;
        case MAP_1000TH_INCH: mnUIUnitKomma += 3; break;
        case MAP_100TH_INCH : mnUIUnitKomma += 2; break;
        case MAP_10TH_INCH  : mnUIUnitKomma += 1; break;
        case MAP_INCH       : mnUIUnitKomma += 0; break;
        case MAP_POINT      : nDiv=72;     break;          // 1Pt   = 1/72"
        case MAP_TWIP       : nDiv=144; mnUIUnitKomma++; break; // 1Twip = 1/1440"
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
    switch(GetUIUnit())
    {
        case FUNIT_NONE   : break;
        // Metrisch
        case FUNIT_100TH_MM: mnUIUnitKomma -= 5; break;
        case FUNIT_MM     : mnUIUnitKomma -= 3; break;
        case FUNIT_CM     : mnUIUnitKomma -= 2; break;
        case FUNIT_M      : mnUIUnitKomma += 0; break;
        case FUNIT_KM     : mnUIUnitKomma += 3; break;
        // Inch
        case FUNIT_TWIP   : nMul=144; mnUIUnitKomma--;  break;  // 1Twip = 1/1440"
        case FUNIT_POINT  : nMul=72;     break;            // 1Pt   = 1/72"
        case FUNIT_PICA   : nMul=6;      break;            // 1Pica = 1/6"  ?
        case FUNIT_INCH   : break;                         // 1"    = 1"
        case FUNIT_FOOT   : nDiv*=12;    break;            // 1Ft   = 12"
        case FUNIT_MILE   : nDiv*=6336; mnUIUnitKomma++; break; // 1mile = 63360"
        // sonstiges
        case FUNIT_CUSTOM : break;
        case FUNIT_PERCENT: mnUIUnitKomma += 2; break;
    } // switch

    // check if mapping is from metric to inch and adapt
    const bool bMapInch(IsInch(GetExchangeObjectUnit()));
    const bool bUIMetr(IsMetric(GetUIUnit()));

    if (bMapInch && bUIMetr)
    {
        mnUIUnitKomma += 4;
        nMul *= 254;
    }

    // check if mapping is from inch to metric and adapt
    const bool bMapMetr(IsMetric(GetExchangeObjectUnit()));
    const bool bUIInch(IsInch(GetUIUnit()));

    if (bMapMetr && bUIInch)
    {
        mnUIUnitKomma -= 4;
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
    if(1 != GetUIScale().GetDenominator() || 1 != GetUIScale().GetNumerator())
    {
        // divide by UIScale
        nMul *= GetUIScale().GetDenominator();
        nDiv *= GetUIScale().GetNumerator();
    }

    // shorten trailing zeroes for dividend
    while(0 == (nMul % 10))
    {
        mnUIUnitKomma--;
        nMul /= 10;
    }

    // shorten trailing zeroes for divisor
    while(0 == (nDiv % 10))
    {
        mnUIUnitKomma++;
        nDiv /= 10;
    }

    // end preparations, set member values
    maUIUnitScale = Fraction(sal_Int32(nMul), sal_Int32(nDiv));
    TakeUnitStr(GetUIUnit(), maUIUnitString);
}

void SdrModel::SetExchangeObjectUnit(MapUnit eMap)
{
    if(GetExchangeObjectUnit() != eMap)
    {
        meExchangeObjectUnit = eMap;

        GetItemPool().SetDefaultMetric((SfxMapUnit)GetExchangeObjectUnit());
        ImpSetUIUnit();
        ImpSetOutlinerDefaults(mpDrawOutliner);
        ReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetExchangeObjectScale(const Fraction& rFrac)
{
    if(GetExchangeObjectScale() != rFrac)
    {
        maExchangeObjectScale = rFrac;

        ImpSetUIUnit();
        ImpSetOutlinerDefaults(mpDrawOutliner);
        ReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit)
{
    if(GetUIUnit() != eUnit)
    {
        meUIUnit = eUnit;

        ImpSetUIUnit();
        ReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetUIScale(const Fraction& rScale)
{
    if(GetUIScale() != rScale)
    {
        maUIScale = rScale;

        ImpSetUIUnit();
        ReformatAllTextObjects(); // #40424#
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
            sal_Char aText[] = "/100mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MM     :
        {
            sal_Char aText[] = "mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_CM     :
        {
            sal_Char aText[] = "cm";
            rStr = UniString(aText, sizeof(aText-1));
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
            sal_Char aText[] = "km";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_TWIP   :
        {
            sal_Char aText[] = "twip";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_POINT  :
        {
            sal_Char aText[] = "pt";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_PICA   :
        {
            sal_Char aText[] = "pica";
            rStr = UniString(aText, sizeof(aText-1));
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
            sal_Char aText[] = "ft";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MILE   :
        {
            sal_Char aText[] = "mile(s)";
            rStr = UniString(aText, sizeof(aText-1));
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

void SdrModel::TakeMetricStr(long nVal, XubString& rStr, bool bNoUnitChars, sal_Int32 nNumDigits) const
{
    // change to double precision usage to not lose decimal places after comma
    const bool bNegative(nVal < 0);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc(aSysLoc.GetLocaleData());
    double fLocalValue(double(nVal) * double(maUIUnitScale));

    if(bNegative)
    {
        fLocalValue = -fLocalValue;
    }

    if( -1 == nNumDigits )
    {
        nNumDigits = rLoc.getNumDigits();
    }

    sal_Int32 nKomma(mnUIUnitKomma);

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

    rStr = UniString::CreateFromInt32(static_cast<sal_Int32>(fLocalValue + 0.5));

    if(nKomma < 0)
    {
        // Negatives Komma bedeutet: Nullen dran
        sal_Int32 nAnz(-nKomma);

        for(sal_Int32 i=0; i<nAnz; i++)
            rStr += sal_Unicode('0');

        nKomma = 0;
    }

    // #83257# the second condition needs to be <= since inside this loop
    // also the leading zero is inserted.
    if(nKomma > 0 && rStr.Len() <= nKomma)
    {
        // Fuer Komma evtl. vorne Nullen dran
        sal_Int32 nAnz(nKomma - rStr.Len());

        if(nAnz >= 0 && rLoc.isNumLeadingZero())
            nAnz++;

        for(sal_Int32 i=0; i<nAnz; i++)
            rStr.Insert(sal_Unicode('0'), 0);
    }

    sal_Unicode cDec( rLoc.getNumDecimalSep().GetChar(0) );

    // KommaChar einfuegen
    sal_Int32 nVorKomma(rStr.Len() - nKomma);

    if(nKomma > 0)
        rStr.Insert(cDec, (xub_StrLen) nVorKomma);

    if(!rLoc.isNumTrailingZeros())
    {
        while(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == sal_Unicode('0'))
            rStr.Erase(rStr.Len() - 1);

        if(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == cDec)
            rStr.Erase(rStr.Len() - 1);
    }

    // ggf. Trennpunkte bei jedem Tausender einfuegen
    if( nVorKomma > 3 )
    {
        String aThoSep( rLoc.getNumThousandSep() );
        if ( aThoSep.Len() > 0 )
        {
            sal_Unicode cTho( aThoSep.GetChar(0) );
            sal_Int32 i(nVorKomma - 3);

            while(i > 0) // #78311#
            {
                rStr.Insert(cTho, (xub_StrLen)i);
                i -= 3;
            }
        }
    }

    if(!rStr.Len())
    {
        rStr = String();
        rStr += sal_Unicode('0');
    }

    if(bNegative)
    {
        rStr.Insert(sal_Unicode('-'), 0);
    }

    if(!bNoUnitChars)
    {
        rStr += maUIUnitString;
    }
}

void SdrModel::TakeWinkStr(long nWink, XubString& rStr, bool bNoDegChar) const
{
    bool bNeg(nWink < 0);

    if(bNeg)
    {
        nWink = -nWink;
    }

    rStr = UniString::CreateFromInt32(nWink);

    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();
    xub_StrLen nAnz(2);

    if(rLoc.isNumLeadingZero())
    {
        nAnz++;
    }

    while(rStr.Len() < nAnz)
    {
        rStr.Insert(sal_Unicode('0'), 0);
    }

    rStr.Insert(rLoc.getNumDecimalSep().GetChar(0), rStr.Len() - 2);

    if(bNeg)
    {
        rStr.Insert(sal_Unicode('-'), 0);
    }

    if(!bNoDegChar)
    {
        rStr += DEGREE_CHAR;
    }
}

void SdrModel::TakePercentStr(const Fraction& rVal, XubString& rStr, bool bNoPercentChar) const
{
    sal_Int32 nMul(rVal.GetNumerator());
    sal_Int32 nDiv(rVal.GetDenominator());
    bool bNeg(nMul < 0);

    if(nDiv < 0)
    {
        bNeg = !bNeg;
    }

    if(nMul < 0)
    {
        nMul = -nMul;
    }

    if(nDiv < 0)
    {
        nDiv = -nDiv;
    }

    nMul *= 100;
    nMul += nDiv/2;
    nMul /= nDiv;

    rStr = UniString::CreateFromInt32(nMul);

    if(bNeg)
    {
        rStr.Insert(sal_Unicode('-'), 0);
    }

    if(!bNoPercentChar)
    {
        rStr += sal_Unicode('%');
    }
}

void SdrModel::SetChanged(bool bFlg)
{
    mbChanged = bFlg;
}

void SdrModel::EnsureValidPageNumbers(bool bMaster)
{
    sal_uInt32 a(0);
    SdrPageContainerType::const_iterator aCandidate;

    if(bMaster)
    {
        for(aCandidate = maMasterPageVector.begin();
            aCandidate != maMasterPageVector.end(); a++, aCandidate++)
        {
            OSL_ENSURE(*aCandidate, "Error in MasterPageVector (!)");
            SetPageNumberAtSdrPageFromSdrModel(**aCandidate, a);
        }
    }
    else
    {
        for(aCandidate = maPageVector.begin();
            aCandidate != maPageVector.end(); a++, aCandidate++)
        {
            OSL_ENSURE(*aCandidate, "Error in PageVector (!)");
            SetPageNumberAtSdrPageFromSdrModel(**aCandidate, a);
        }
    }
}

void SdrModel::InsertPage(SdrPage* pPage, sal_uInt32 nPos)
{
    if(pPage && &pPage->getSdrModelFromSdrPage() == this)
    {
        const sal_uInt32 nAnz(GetPageCount());

        if(nPos >= nAnz)
        {
            maPageVector.push_back(pPage);
            SetPageNumberAtSdrPageFromSdrModel(*pPage, nAnz);
        }
        else
        {
            maPageVector.insert(maPageVector.begin() + nPos, pPage);
            EnsureValidPageNumbers(false);
        }

        SetInsertedAtSdrPageFromSdrModel(*pPage, true);
        SetChanged();
        Broadcast(SdrBaseHint(*pPage, HINT_PAGEORDERCHG));
    }
    else
    {
        OSL_ENSURE(pPage, "SdrModel::InsertPage without Page (!)");
        OSL_ENSURE(&pPage->getSdrModelFromSdrPage() == this, "SdrModel::InsertPage with SdrPage from alien SdrModel (!)");
    }
}

void SdrModel::DeletePage(sal_uInt32 nPgNum)
{
    SdrPage* pPg = RemovePage(nPgNum);

    if(pPg)
    {
        delete pPg;
    }
}

SdrPage* SdrModel::RemovePage(sal_uInt32 nPgNum)
{
    SdrPage* pPage = 0;

    if(nPgNum < maPageVector.size())
    {
        const SdrPageContainerType::iterator aCandidate(maPageVector.begin() + nPgNum);
        pPage = *aCandidate;

        maPageVector.erase(aCandidate);

        if(nPgNum != maPageVector.size())
        {
            EnsureValidPageNumbers(false);
        }

        OSL_ENSURE(pPage, "SdrModel::RemovePage detected non-existent Page (!)");

        SetInsertedAtSdrPageFromSdrModel(*pPage, false);
        SetChanged();
        Broadcast(SdrBaseHint(*pPage, HINT_PAGEORDERCHG));
    }
    else
    {
        OSL_ENSURE(false, "SdrModel::RemovePage with wrong index (!)");
    }

    return pPage;
}

void SdrModel::MovePage(sal_uInt32 nPgNum, sal_uInt32 nNewPos)
{
    if(nPgNum == nNewPos)
    {
        return;
    }
    else
    {
        const sal_uInt32 nCount(GetPageCount());

        if(nPgNum < nCount && nNewPos < nCount)
        {
            const SdrPageContainerType::iterator aCandidate(maPageVector.begin() + nPgNum);
            SdrPage* pPg = *aCandidate;

            maPageVector.erase(aCandidate);

            OSL_ENSURE(pPg, "SdrModel::MovePage detected non-existent Page (!)");

            if(pPg)
            {
                InsertPage(pPg, nNewPos);
            }

            EnsureValidPageNumbers(false);
        }
        else
        {
            OSL_ENSURE(false, "SdrModel::MovePage with wrong index (!)");
        }
    }
}

void SdrModel::InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos)
{
    if(pPage && &pPage->getSdrModelFromSdrPage() == this)
    {
        const sal_uInt32 nAnz(GetMasterPageCount());

        if(nPos >= nAnz)
        {
            maMasterPageVector.push_back(pPage);
            SetPageNumberAtSdrPageFromSdrModel(*pPage, nAnz);
        }
        else
        {
            maMasterPageVector.insert(maMasterPageVector.begin() + nPos, pPage);
            EnsureValidPageNumbers(true);
        }

        SetInsertedAtSdrPageFromSdrModel(*pPage, true);
        SetChanged();
        Broadcast(SdrBaseHint(*pPage, HINT_PAGEORDERCHG));
    }
    else
    {
        OSL_ENSURE(pPage, "SdrModel::InsertMasterPage without Page (!)");
        OSL_ENSURE(&pPage->getSdrModelFromSdrPage() == this, "SdrModel::InsertMasterPage with SdrPage from alien SdrModel (!)");
    }
}

void SdrModel::DeleteMasterPage(sal_uInt32 nPgNum)
{
    SdrPage* pPg = RemoveMasterPage(nPgNum);

    if(pPg)
    {
        delete pPg;
    }
}

SdrPage* SdrModel::RemoveMasterPage(sal_uInt32 nPgNum)
{
    SdrPage* pPage = 0;

    if(nPgNum < GetMasterPageCount())
    {
        const SdrPageContainerType::iterator aCandidate(maMasterPageVector.begin() + nPgNum);

        pPage = *aCandidate;
        maMasterPageVector.erase(aCandidate);

        if(nPgNum != maMasterPageVector.size())
        {
            EnsureValidPageNumbers(true);
        }

        OSL_ENSURE(pPage, "SdrModel::RemoveMasterPage detected non-existent Page (!)");

        if(pPage)
        {
            // Nun die Verweise der normalen Zeichenseiten auf die entfernte MasterPage loeschen
            const sal_uInt32 nPageAnz(GetPageCount());

            for(sal_uInt32 np(0); np < nPageAnz; np++)
            {
                GetPage(np)->TRG_MasterPageRemoved(*pPage);
            }
        }

        SetInsertedAtSdrPageFromSdrModel(*pPage, false);
        SetChanged();
        Broadcast(SdrBaseHint(*pPage, HINT_PAGEORDERCHG));
    }
    else
    {
        OSL_ENSURE(false, "SdrModel::RemoveMasterPage with wrong index (!)");
    }

    return pPage;
}

void SdrModel::MoveMasterPage(sal_uInt32 nPgNum, sal_uInt32 nNewPos)
{
    if(nPgNum == nNewPos)
    {
        return;
    }
    else
    {
        const sal_uInt32 nCount(GetMasterPageCount());

        if(nPgNum < nCount && nNewPos < nCount)
        {
            const SdrPageContainerType::iterator aCandidate(maMasterPageVector.begin() + nPgNum);
            SdrPage* pPg = *aCandidate;

            maMasterPageVector.erase(aCandidate);

            OSL_ENSURE(pPg, "SdrModel::MoveMasterPage detected non-existent Page (!)");

            if(pPg)
            {
                maMasterPageVector.insert(maMasterPageVector.begin() + nNewPos, pPg);
            }

            EnsureValidPageNumbers(true);
            SetChanged();
            Broadcast(SdrBaseHint(*pPg, HINT_PAGEORDERCHG));
        }
        else
        {
            OSL_ENSURE(false, "SdrModel::MoveMasterPage with wrong index (!)");
        }
    }
}

void SdrModel::CopyPages(
    sal_uInt32 nFirstPageNum,
    sal_uInt32 nLastPageNum,
    sal_uInt32 nDestPos,
    bool bUndo,
    bool bMoveNoCopy)
{
    if(bUndo && !IsUndoEnabled())
    {
        bUndo = false;
    }

    if(bUndo)
    {
        BegUndo(ImpGetResStr(STR_UndoMergeModel));
    }

    const sal_uInt32 nPageAnz(GetPageCount());
    sal_uInt32 nMaxPage(nPageAnz);

    if(nMaxPage)
    {
        nMaxPage--;
    }

    if(nFirstPageNum > nMaxPage)
    {
        nFirstPageNum = nMaxPage;
    }

    if(nLastPageNum > nMaxPage)
    {
        nLastPageNum = nMaxPage;
    }

    const bool bReverse(nLastPageNum < nFirstPageNum);

    if(nDestPos > nPageAnz)
    {
        nDestPos = nPageAnz;
    }

    // Zunaechst die Zeiger der betroffenen Seiten in einem Array sichern
    sal_uInt32 nPageNum(nFirstPageNum);
    sal_uInt32 nCopyAnz(((!bReverse) ? (nLastPageNum - nFirstPageNum) : (nFirstPageNum - nLastPageNum)) + 1);
    SdrPage** pPagePtrs = new SdrPage*[nCopyAnz];
    sal_uInt32 nCopyNum;

    for(nCopyNum = 0; nCopyNum < nCopyAnz; nCopyNum++)
    {
        pPagePtrs[nCopyNum] = GetPage(nPageNum);

        if(bReverse)
        {
            nPageNum--;
        }
        else
        {
            nPageNum++;
        }
    }

    // Jetzt die Seiten kopieren
    sal_uInt32 nDestNum(nDestPos);

    for(nCopyNum = 0; nCopyNum < nCopyAnz; nCopyNum++)
    {
        SdrPage* pPg = pPagePtrs[nCopyNum];
        sal_uInt32 nPageNum2(pPg->GetPageNumber());

        if(!bMoveNoCopy)
        {
            const SdrPage* pPg1 = GetPage(nPageNum2);

            pPg = pPg1->CloneSdrPage(this);
            InsertPage(pPg, nDestNum);

            if(bUndo)
            {
                AddUndo(GetSdrUndoFactory().CreateUndoCopyPage(*pPg));
            }

            nDestNum++;
        }
        else
        {
            if(nDestNum > nPageNum2)
            {
                nDestNum--;
            }

            if(bUndo)
            {
                AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*GetPage(nPageNum2), nPageNum2, nDestNum));
            }

            pPg = RemovePage(nPageNum2);
            InsertPage(pPg, nDestNum);
            nDestNum++;
        }

        if(bReverse)
        {
            nPageNum2--;
        }
        else
        {
            nPageNum2++;
        }
    }

    delete[] pPagePtrs;

    if(bUndo)
    {
        EndUndo();
    }
}

void SdrModel::Merge(
    SdrModel& rSourceModel,
    sal_uInt32 nFirstPageNum,
    sal_uInt32 nLastPageNum,
    sal_uInt32 nDestPos,
    bool bMergeMasterPages,
    bool bAllMasterPages,
    bool bUndo,
    bool bTreadSourceAsConst)
{
    if(&rSourceModel == this)
    {
        CopyPages(nFirstPageNum,nLastPageNum,nDestPos,bUndo,!bTreadSourceAsConst);
        return;
    }

    if(bUndo && !IsUndoEnabled())
    {
        bUndo = false;
    }

    if(bUndo)
    {
        BegUndo(ImpGetResStr(STR_UndoMergeModel));
    }

    const sal_uInt32 nSrcPageAnz(rSourceModel.GetPageCount());
    const sal_uInt32 nSrcMasterPageAnz(rSourceModel.GetMasterPageCount());
    const sal_uInt32 nDstMasterPageAnz(GetMasterPageCount());
    bool bInsPages((nFirstPageNum < nSrcPageAnz || nLastPageNum < nSrcPageAnz));
    sal_uInt32 nMaxSrcPage(nSrcPageAnz);

    if(nMaxSrcPage)
    {
        nMaxSrcPage--;
    }

    if(nFirstPageNum > nMaxSrcPage)
    {
        nFirstPageNum = nMaxSrcPage;
    }

    if(nLastPageNum > nMaxSrcPage)
    {
        nLastPageNum = nMaxSrcPage;
    }

    const bool bReverse(nLastPageNum < nFirstPageNum);
    sal_uInt32* pMasterMap = 0;
    bool* pMasterNeed = 0;
    sal_uInt32 nMasterNeed(0);

    if(bMergeMasterPages && nSrcMasterPageAnz)
    {
        // Feststellen, welche MasterPages aus rSrcModel benoetigt werden
        pMasterMap = new sal_uInt32[nSrcMasterPageAnz];
        pMasterNeed = new bool[nSrcMasterPageAnz];
        memset(pMasterMap, 0xff, nSrcMasterPageAnz * sizeof(sal_uInt32));

        if(bAllMasterPages)
        {
            memset(pMasterNeed, true, nSrcMasterPageAnz * sizeof(bool));
        }
        else
        {
            memset(pMasterNeed, false, nSrcMasterPageAnz * sizeof(bool));
            sal_uInt32 nAnf(bReverse ? nLastPageNum : nFirstPageNum);
            sal_uInt32 nEnd(bReverse ? nFirstPageNum : nLastPageNum);

            for(sal_uInt32 i(nAnf); i <= nEnd; i++)
            {
                const SdrPage* pPg = rSourceModel.GetPage(i);

                if(pPg->TRG_HasMasterPage())
                {
                    SdrPage& rMasterPage = pPg->TRG_GetMasterPage();
                    const sal_uInt32 nMPgNum(rMasterPage.GetPageNumber());

                    if(nMPgNum < nSrcMasterPageAnz)
                    {
                        pMasterNeed[nMPgNum] = true;
                    }
                }
            }
        }

        // Nun das Mapping der MasterPages bestimmen
        sal_uInt32 nAktMaPagNum(nDstMasterPageAnz);

        for(sal_uInt32 i(0); i < nSrcMasterPageAnz; i++)
        {
            if(pMasterNeed[i])
            {
                pMasterMap[i] = nAktMaPagNum;
                nAktMaPagNum++;
                nMasterNeed++;
            }
        }
    }

    // rueberholen der Masterpages
    if(pMasterMap && pMasterNeed && nMasterNeed)
    {
        for(sal_uInt32 i(nSrcMasterPageAnz); i > 0;)
        {
            i--;

            if(pMasterNeed[i])
            {
                SdrPage* pPg = rSourceModel.GetMasterPage(i)->CloneSdrPage(this);;

                if(!bTreadSourceAsConst)
                {
                    delete rSourceModel.RemoveMasterPage(i);
                }

                if(pPg)
                {
                    // und alle ans einstige Ende des DstModel reinschieben.
                    // nicht InsertMasterPage() verwenden da die Sache
                    // inkonsistent ist bis alle drin sind
                    maMasterPageVector.insert(maMasterPageVector.begin() + nDstMasterPageAnz, pPg);
                    SetInsertedAtSdrPageFromSdrModel(*pPg, true);
                    Broadcast(SdrBaseHint(*pPg, HINT_PAGEORDERCHG));

                    if(bUndo)
                    {
                        AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pPg));
                    }
                }
                else
                {
                    DBG_ERROR("SdrModel::Merge(): MasterPage im SourceModel nicht gefunden");
                }
            }
        }

        EnsureValidPageNumbers(true);
    }

    // rueberholen der Zeichenseiten
    if(bInsPages)
    {
        sal_uInt32 nSourcePos(nFirstPageNum);
        sal_uInt32 nMergeCount((nLastPageNum > nFirstPageNum ? (nLastPageNum - nFirstPageNum) : (nFirstPageNum - nLastPageNum)) + 1);

        if(nDestPos > GetPageCount())
        {
            nDestPos = GetPageCount();
        }

        while(nMergeCount)
        {
            SdrPage* pPg = rSourceModel.GetPage(nSourcePos)->CloneSdrPage(this);

            if(!bTreadSourceAsConst)
            {
                delete rSourceModel.RemovePage(nSourcePos);
            }

            if(pPg)
            {
                InsertPage(pPg, nDestPos);

                if(bUndo)
                {
                    AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pPg));
                }

                // und nun zu den MasterPageDescriptoren
                if(pPg->TRG_HasMasterPage())
                {
                    SdrPage& rMasterPage = pPg->TRG_GetMasterPage();
                    sal_uInt32 nMaPgNum(rMasterPage.GetPageNumber());

                    if (bMergeMasterPages)
                    {
                        sal_uInt32 nNeuNum(0xffffffff);

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

                        DBG_ASSERT(nNeuNum!=0xFFFF,"SdrModel::Merge(): Irgendwas ist krumm beim Mappen der MasterPages");
                    }
                    else
                    {
                        if(nMaPgNum >= nDstMasterPageAnz)
                        {
                            // Aha, die ist ausserbalb des urspruenglichen Bereichs der Masterpages des DstModel
                            pPg->TRG_ClearMasterPage();
                        }
                    }
                }
            }
            else
            {
                DBG_ERROR("SdrModel::Merge(): Zeichenseite im SourceModel nicht gefunden");
            }

            nDestPos++;

            if(bReverse)
            {
                nSourcePos--;
            }
            else if(bTreadSourceAsConst)
            {
                nSourcePos++;
            }

            nMergeCount--;
        }
    }

    delete [] pMasterMap;
    delete [] pMasterNeed;

    SetChanged();

    // Fehlt: Mergen und Mapping der Layer
    // an den Objekten sowie an den MasterPageDescriptoren

    if(bUndo)
    {
        EndUndo();
    }
}

void SdrModel::SetStarDrawPreviewMode(bool bPreview)
{
    if(!bPreview && IsStarDrawPreviewMode() && GetPageCount())
    {
        // Das Zuruecksetzen ist nicht erlaubt, da das Model ev. nicht vollstaendig geladen wurde
        DBG_ASSERT(false,"SdrModel::SetStarDrawPreviewMode(): Zuruecksetzen nicht erlaubt, da Model ev. nicht vollstaendig");
    }
    else
    {
        if(mbStarDrawPreviewMode != bPreview)
        {
            mbStarDrawPreviewMode = bPreview;
        }
    }
}

uno::Reference< uno::XInterface > SdrModel::getUnoModel()
{
    if(!mxUnoModel.is())
    {
        mxUnoModel = createUnoModel();
    }

    return mxUnoModel;
}

void SdrModel::setUnoModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xModel )
{
    mxUnoModel = xModel;
}

uno::Reference< uno::XInterface > SdrModel::createUnoModel()
{
    DBG_ERROR( "SdrModel::createUnoModel() - base implementation should not be called!" );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt;

    return xInt;
}

void SdrModel::setLock( bool bLock )
{
    if(mbModelLocked != bLock)
    {
        // #120437# need to set first, else ImpReformatAllEdgeObjects will do nothing
        mbModelLocked = bLock;

        if(!bLock)
        {
            ImpReformatAllEdgeObjects();
        }
    }
}

void SdrModel::SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars )
{
    if(mpForbiddenCharactersTable)
    {
        mpForbiddenCharactersTable->release();
    }

    mpForbiddenCharactersTable = xForbiddenChars.getBodyPtr();

    if(mpForbiddenCharactersTable)
    {
        mpForbiddenCharactersTable->acquire();
    }

    ImpSetOutlinerDefaults(mpDrawOutliner);
}

vos::ORef<SvxForbiddenCharactersTable> SdrModel::GetForbiddenCharsTable() const
{
    return mpForbiddenCharactersTable;
}

void SdrModel::SetCharCompressType(sal_uInt16 nType)
{
    if(nType != mnCharCompressType)
    {
        mnCharCompressType = nType;

        ImpSetOutlinerDefaults(mpDrawOutliner);
    }
}

void SdrModel::SetKernAsianPunctuation(bool bEnabled)
{
    if(mbKernAsianPunctuation != bEnabled)
    {
        mbKernAsianPunctuation = bEnabled;

        ImpSetOutlinerDefaults(mpDrawOutliner);
    }
}

void SdrModel::SetAddExtLeading( bool bEnabled )
{
    if(mbAddExtLeading != bEnabled)
    {
        mbAddExtLeading = bEnabled;

        ImpSetOutlinerDefaults(mpDrawOutliner);
    }
}

void SdrModel::ReformatAllTextObjects()
{
    if(isLocked())
    {
        return;
    }

    sal_uInt32 nAnz(GetMasterPageCount());
    sal_uInt32 nNum;

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetMasterPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            aIter.Next()->ReformatText();
        }
    }

    nAnz = GetPageCount();

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        SdrObjListIter aIter(*GetPage(nNum), IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            aIter.Next()->ReformatText();
        }
    }
}

SdrOutliner* SdrModel::createOutliner(sal_uInt16 nOutlinerMode)
{
    if(!mpOutlinerCache)
    {
        mpOutlinerCache = new SdrOutlinerCache(this);
    }

    return mpOutlinerCache->createOutliner(nOutlinerMode);
}

void SdrModel::disposeOutliner(SdrOutliner* pOutliner)
{
    if(mpOutlinerCache)
    {
        mpOutlinerCache->disposeOutliner(pOutliner);
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

SdrPage* SdrModel::GetPage(sal_uInt32 nPgNum) const
{
    if(nPgNum < maPageVector.size())
    {
        return *(maPageVector.begin() + nPgNum);
    }
    else
    {
        OSL_ENSURE(false, "SdrModel::GetPage access oot of range (!)");
        return 0;
    }
}

SdrPage* SdrModel::GetMasterPage(sal_uInt32 nPgNum) const
{
    if(nPgNum < maMasterPageVector.size())
    {
        return *(maMasterPageVector.begin() + nPgNum);
    }
    else
    {
        OSL_ENSURE(false, "SdrModel::GetMasterPage access out of range (!)");
        return 0;
    }
}

void SdrModel::SetSdrUndoManager(SfxUndoManager* pUndoManager)
{
    mpUndoManager = pUndoManager;
}

SfxUndoManager* SdrModel::GetSdrUndoManager() const
{
    return mpUndoManager;
}

SdrUndoFactory& SdrModel::GetSdrUndoFactory() const
{
    if(!mpUndoFactory)
    {
        const_cast< SdrModel* >(this)->mpUndoFactory = new SdrUndoFactory();
    }

    return *mpUndoFactory;
}

void SdrModel::SetSdrUndoFactory(SdrUndoFactory* pUndoFactory)
{
    if(pUndoFactory && (pUndoFactory != mpUndoFactory))
    {
        delete mpUndoFactory;

        mpUndoFactory = pUndoFactory;
    }
}

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdrModel::getUnoTunnelImplementationId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;

    if(!pSeq)
    {
        ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());

        if(!pSeq)
        {
            static Sequence< sal_Int8 > aSeq( 16 );

            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, true);
            pSeq = &aSeq;
        }
    }

    return *pSeq;
}

void SdrModel::SetDrawingLayerPoolDefaults()
{
    const String aNullStr;
    const Color aNullLineCol(COL_DEFAULT_SHAPE_STROKE);
    const Color aNullFillCol(COL_DEFAULT_SHAPE_FILLING);
    const XHatch aNullHatch(aNullLineCol);

    mpItemPool->SetPoolDefaultItem(XFillColorItem(aNullStr, aNullFillCol));
    mpItemPool->SetPoolDefaultItem(XFillHatchItem(mpItemPool, aNullHatch));
    mpItemPool->SetPoolDefaultItem(XLineColorItem(aNullStr, aNullLineCol));
}

bool SdrModel::IsWriter() const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// i120668, move from the header files, add delete action

void SdrModel::SetColorTable(XColorTable* pTable) { delete mpColorTable; mpColorTable = pTable; }
void SdrModel::SetDashList(XDashList* pList) { delete mpDashList; mpDashList = pList; }
void SdrModel::SetLineEndList(XLineEndList* pList) { delete mpLineEndList; mpLineEndList = pList; }
void SdrModel::SetHatchList(XHatchList* pList) { delete mpHatchList; mpHatchList = pList; }
void SdrModel::SetGradientList(XGradientList* pList) { delete mpGradientList; mpGradientList = pList; }
void SdrModel::SetBitmapList(XBitmapList* pList) { delete mpBitmapList; mpBitmapList = pList; }

::std::set< SdrView* > SdrModel::getSdrViews() const
{
    ::std::set< SdrView* > aRetval;
    const sal_uInt16 nCount(GetListenerCount());

    for(sal_uInt16 a(0); a < nCount; a++)
    {
        SdrView* pCandidate = dynamic_cast< SdrView* >(GetListener(a));

        if(pCandidate)
        {
            aRetval.insert(pCandidate);
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrBaseHint::SdrBaseHint(
    SdrHintKind eSdrHintKind)
:   SfxHint(),
    meSdrHint(eSdrHintKind),
    mpSdrPage(0),
    mpSdrObject(0)
{
}

SdrBaseHint::SdrBaseHint(
    const SdrPage& rSdrPage,
    SdrHintKind eSdrHintKind)
:   SfxHint(),
    meSdrHint(eSdrHintKind),
    mpSdrPage(&rSdrPage),
    mpSdrObject(0)
{
}

SdrBaseHint::SdrBaseHint(
    const SdrObject& rSdrObject,
    SdrHintKind eSdrHintKind)
:   SfxHint(),
    meSdrHint(eSdrHintKind),
    mpSdrPage(rSdrObject.getSdrPageFromSdrObject()),
    mpSdrObject(&rSdrObject)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
