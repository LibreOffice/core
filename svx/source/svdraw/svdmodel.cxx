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
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/whiter.hxx>
#include <svl/asiancfg.hxx>
#include <svx/compatflags.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>
#include <editeng/editeng.hxx>
#include <svx/xtable.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <textchain.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/theme/IThemeColorChanger.hxx>
#include <svdoutlinercache.hxx>
#include <svx/sdasitm.hxx>
#include <officecfg/Office/Common.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <memory>
#include <libxml/xmlwriter.h>
#include <sfx2/viewsh.hxx>
#include <o3tl/enumrange.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/UnitConversion.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/ColorSets.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoashp.hxx>


using namespace ::com::sun::star;

struct SdrModelImpl
{
    SfxUndoManager* mpUndoManager;
    SdrUndoFactory* mpUndoFactory;
    bool mbAnchoredTextOverflowLegacy; // tdf#99729 compatibility flag
    bool mbLegacyFontwork;             // tdf#148000 compatibility flag
    bool mbConnectorUseSnapRect;       // tdf#149756 compatibility flag
    bool mbIgnoreBreakAfterMultilineField; ///< tdf#148966 compatibility flag
    std::shared_ptr<model::Theme> mpTheme;
    std::shared_ptr<svx::IThemeColorChanger> mpThemeColorChanger;

    SdrModelImpl()
        : mpUndoManager(nullptr)
        , mpUndoFactory(nullptr)
        , mbAnchoredTextOverflowLegacy(false)
        , mbLegacyFontwork(false)
        , mbConnectorUseSnapRect(false)
        , mbIgnoreBreakAfterMultilineField(false)
        , mpTheme(new model::Theme(u"Office"_ustr))
    {}

    void initTheme()
    {
        auto const* pColorSet = svx::ColorSets::get().getColorSet(u"LibreOffice");
        if (pColorSet)
        {
            std::shared_ptr<model::ColorSet> pDefaultColorSet(new model::ColorSet(*pColorSet));
            mpTheme->setColorSet(pDefaultColorSet);
        }
    }
};


SdrModel::SdrModel(SfxItemPool* pPool, comphelper::IEmbeddedHelper* pEmbeddedHelper, bool bDisablePropertyFiles)
    : m_eObjUnit(SdrEngineDefaults::GetMapUnit())
    , m_eUIUnit(FieldUnit::MM)
    , m_aUIScale(Fraction(1,1))
    , m_nUIUnitDecimalMark(0)
    , m_pLayerAdmin(new SdrLayerAdmin)
    , m_pItemPool(pPool)
    , m_pEmbeddedHelper(pEmbeddedHelper)
    , mnDefTextHgt(SdrEngineDefaults::GetFontHeight())
    , m_pRefOutDev(nullptr)
    , m_pDefaultStyleSheet(nullptr)
    , mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(nullptr)
    , m_pLinkManager(nullptr)
    , m_nUndoLevel(0)
    , m_bIsWriter(true)
    , m_bThemedControls(true)
    , mbUndoEnabled(true)
    , mbChanged(false)
    , m_bPagNumsDirty(false)
    , m_bMPgNumsDirty(false)
    , m_bTransportContainer(false)
    , m_bReadOnly(false)
    , m_bTransparentTextFrames(false)
    , m_bSwapGraphics(false)
    , m_bPasteResize(false)
    , m_bStarDrawPreviewMode(false)
    , mbDisableTextEditUsesCommonUndoManager(false)
    , mbVOCInvalidationIsReliable(false)
    , m_nDefaultTabulator(0)
    , m_nMaxUndoCount(16)
    , m_pTextChain(new TextChain)
    , mpImpl(new SdrModelImpl)
    , mnCharCompressType(CharCompressType::NONE)
    , mnHandoutPageCount(0)
    , mbModelLocked(false)
    , mbKernAsianPunctuation(false)
    , mbAddExtLeading(false)
    , mbInDestruction(false)
{
    if (!comphelper::IsFuzzing())
    {
        mnCharCompressType = static_cast<CharCompressType>(
            officecfg::Office::Common::AsianLayout::CompressCharacterDistance::get());
    }

    if (m_pItemPool == nullptr)
    {
        m_pItemPool = new SdrItemPool(nullptr);
        // Outliner doesn't have its own Pool, so use the EditEngine's
        rtl::Reference<SfxItemPool> pOutlPool=EditEngine::CreatePool();
        // OutlinerPool as SecondaryPool of SdrPool
        m_pItemPool->SetSecondaryPool(pOutlPool.get());
        // remember that I created both pools myself
        m_bIsWriter = false;
    }
    m_pItemPool->SetDefaultMetric(m_eObjUnit);

// using static SdrEngineDefaults only if default SvxFontHeight item is not available
    const SfxPoolItem* pPoolItem = m_pItemPool->GetUserDefaultItem( EE_CHAR_FONTHEIGHT );
    if (pPoolItem)
        mnDefTextHgt = static_cast<const SvxFontHeightItem*>(pPoolItem)->GetHeight();

    m_pItemPool->SetUserDefaultItem( makeSdrTextWordWrapItem( false ) );

    SetTextDefaults();
    m_pLayerAdmin->SetModel(this);
    ImpSetUIUnit();

    // can't create DrawOutliner OnDemand, because I can't get the Pool,
    // then (only from 302 onwards!)
    m_pDrawOutliner = SdrMakeOutliner(OutlinerMode::TextObject, *this);
    ImpSetOutlinerDefaults(m_pDrawOutliner.get(), true);

    m_pHitTestOutliner = SdrMakeOutliner(OutlinerMode::TextObject, *this);
    ImpSetOutlinerDefaults(m_pHitTestOutliner.get(), true);

    /* Start Text Chaining related code */
    // Initialize Chaining Outliner
    m_pChainingOutliner = SdrMakeOutliner( OutlinerMode::TextObject, *this );
    ImpSetOutlinerDefaults(m_pChainingOutliner.get(), true);

    ImpCreateTables(bDisablePropertyFiles || comphelper::IsFuzzing());

    mpImpl->initTheme();
}

void SdrModel::implDtorClearModel()
{
    mbInDestruction = true;

    Broadcast(SdrHint(SdrHintKind::ModelCleared));

    mpOutlinerCache.reset();

    ClearUndoBuffer();
#ifdef DBG_UTIL
    SAL_WARN_IF(m_pCurrentUndoGroup, "svx", "In the Dtor of the SdrModel there is an open Undo left: \""
                    << m_pCurrentUndoGroup->GetComment() << '\"');
#endif
    m_pCurrentUndoGroup.reset();

    ClearModel(true);
}

SdrModel::~SdrModel()
{
    implDtorClearModel();

#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    if(!maAllIncarnatedObjects.empty())
    {
        SAL_WARN("svx",
            "SdrModel::~SdrModel: Not all incarnations of SdrObjects deleted, possible memory leak");
        for (const auto & pObj : maAllIncarnatedObjects)
            SAL_WARN("svx", "leaked instance of " << typeid(*pObj).name());
    }
#endif

    m_pLayerAdmin.reset();

    m_pTextChain.reset();
    // Delete DrawOutliner only after deleting ItemPool, because ItemPool
    // references Items of the DrawOutliner!
    m_pChainingOutliner.reset();
    m_pHitTestOutliner.reset();
    m_pDrawOutliner.reset();

    // delete StyleSheetPool, derived classes should not do this since
    // the DrawingEngine may need it in its destructor
    if( mxStyleSheetPool.is() )
    {
        uno::Reference<lang::XComponent> xComponent( getXWeak( mxStyleSheetPool.get() ), uno::UNO_QUERY );
        if( xComponent.is() ) try
        {
            xComponent->dispose();
        }
        catch (uno::RuntimeException&)
        {
        }
        mxStyleSheetPool.clear();
    }

    mpForbiddenCharactersTable.reset();

    delete mpImpl->mpUndoFactory;
}

void SdrModel::SetSwapGraphics()
{
    m_bSwapGraphics = true;
}

bool SdrModel::IsReadOnly() const
{
    return m_bReadOnly;
}

void SdrModel::SetReadOnly(bool bYes)
{
    m_bReadOnly=bYes;
}


void SdrModel::SetMaxUndoActionCount(sal_uInt32 nCount)
{
    if (nCount<1) nCount=1;
    m_nMaxUndoCount=nCount;
    while (m_aUndoStack.size()>m_nMaxUndoCount)
        m_aUndoStack.pop_back();
}

void SdrModel::ClearUndoBuffer()
{
    m_aUndoStack.clear();
    m_aRedoStack.clear();
}

bool SdrModel::HasUndoActions() const
{
    return !m_aUndoStack.empty();
}

bool SdrModel::HasRedoActions() const
{
    return !m_aRedoStack.empty();
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
            SfxUndoAction* pDo = m_aUndoStack.front().get();
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Undo();
            std::unique_ptr<SfxUndoAction> p = std::move(m_aUndoStack.front());
            m_aUndoStack.pop_front();
            m_aRedoStack.emplace_front(std::move(p));
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
            SfxUndoAction* pDo = m_aRedoStack.front().get();
            const bool bWasUndoEnabled = mbUndoEnabled;
            mbUndoEnabled = false;
            pDo->Redo();
            std::unique_ptr<SfxUndoAction> p = std::move(m_aRedoStack.front());
            m_aRedoStack.pop_front();
            m_aUndoStack.emplace_front(std::move(p));
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
            SfxUndoAction* pDo =  m_aUndoStack.front().get();
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
    if( !IsUndoEnabled() )
        return;

    if (m_aUndoLink)
    {
        m_aUndoLink(std::move(pUndo));
    }
    else
    {
        m_aUndoStack.emplace_front(std::move(pUndo));
        while (m_aUndoStack.size()>m_nMaxUndoCount)
        {
            m_aUndoStack.pop_back();
        }
        m_aRedoStack.clear();
    }
}

void SdrModel::BegUndo()
{
    if( mpImpl->mpUndoManager )
    {
        ViewShellId nViewShellId(-1);
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            nViewShellId = pViewShell->GetViewShellId();
        mpImpl->mpUndoManager->EnterListAction(u""_ustr,u""_ustr,0,nViewShellId);
        m_nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        if(!m_pCurrentUndoGroup)
        {
            m_pCurrentUndoGroup.reset(new SdrUndoGroup(*this));
            m_nUndoLevel=1;
        }
        else
        {
            m_nUndoLevel++;
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
        mpImpl->mpUndoManager->EnterListAction( rComment, u""_ustr, 0, nViewShellId );
        m_nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (m_nUndoLevel==1)
        {
            m_pCurrentUndoGroup->SetComment(rComment);
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
        mpImpl->mpUndoManager->EnterListAction( aComment,u""_ustr,0,nViewShellId );
        m_nUndoLevel++;
    }
    else if( IsUndoEnabled() )
    {
        BegUndo();
        if (m_nUndoLevel==1)
        {
            m_pCurrentUndoGroup->SetComment(rComment);
            m_pCurrentUndoGroup->SetObjDescription(rObjDescr);
            m_pCurrentUndoGroup->SetRepeatFunction(eFunc);
        }
    }
}

void SdrModel::EndUndo()
{
    DBG_ASSERT(m_nUndoLevel!=0,"SdrModel::EndUndo(): UndoLevel is already 0!");
    if( mpImpl->mpUndoManager )
    {
        if( m_nUndoLevel )
        {
            m_nUndoLevel--;
            mpImpl->mpUndoManager->LeaveListAction();
        }
    }
    else
    {
        if(m_pCurrentUndoGroup!=nullptr && IsUndoEnabled())
        {
            m_nUndoLevel--;
            if(m_nUndoLevel==0)
            {
                if(m_pCurrentUndoGroup->GetActionCount()!=0)
                {
                    ImpPostUndoAction(std::move(m_pCurrentUndoGroup));
                }
                else
                {
                    // was empty
                    m_pCurrentUndoGroup.reset();
                }
            }
        }
    }
}

void SdrModel::SetUndoComment(const OUString& rComment)
{
    DBG_ASSERT(m_nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel is already 0!");

    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else if( IsUndoEnabled() && m_nUndoLevel==1)
    {
        m_pCurrentUndoGroup->SetComment(rComment);
    }
}

void SdrModel::SetUndoComment(const OUString& rComment, const OUString& rObjDescr)
{
    DBG_ASSERT(m_nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel is already 0!");
    if( mpImpl->mpUndoManager )
    {
        OSL_FAIL("svx::SdrModel::SetUndoComment(), method not supported with application undo manager!" );
    }
    else
    {
        if (m_nUndoLevel==1)
        {
            m_pCurrentUndoGroup->SetComment(rComment);
            m_pCurrentUndoGroup->SetObjDescription(rObjDescr);
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
        if (m_pCurrentUndoGroup)
        {
            m_pCurrentUndoGroup->AddAction(std::move(pUndo));
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
    const OUString aTablePath(!bDisablePropertyFiles ? SvtPathOptions().GetPalettePath() : u""_ustr);

    for( auto i : o3tl::enumrange<XPropertyListType>() )
    {
        maProperties[i] = XPropertyList::CreatePropertyList(i, aTablePath, u""_ustr/*TODO?*/ );
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
    maMasterPages.clear();
    MasterPageListChanged();

    m_pLayerAdmin->ClearLayers();
}

SdrModel* SdrModel::AllocModel() const
{
    SdrModel* pModel=new SdrModel();
    pModel->SetScaleUnit(m_eObjUnit);
    return pModel;
}

rtl::Reference<SdrPage> SdrModel::AllocPage(bool bMasterPage)
{
    return new SdrPage(*this,bMasterPage);
}

void SdrModel::SetTextDefaults() const
{
    SetTextDefaults( m_pItemPool.get(), mnDefTextHgt );
}

void SdrModel::SetTextDefaults( SfxItemPool* pItemPool, sal_Int32 nDefTextHgt )
{
    // set application-language specific dynamic pool language defaults
    SvxFontItem aSvxFontItem( EE_CHAR_FONTINFO) ;
    SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
    SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
    LanguageType nLanguage;
    if (!comphelper::IsFuzzing())
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
    pItemPool->SetUserDefaultItem(aSvxFontItem);

    // get DEFAULTFONT_CJK_TEXT and set at pool as dynamic default
    vcl::Font aFontCJK(OutputDevice::GetDefaultFont(DefaultFontType::CJK_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCJK.SetFamily( aFontCJK.GetFamilyType());
    aSvxFontItemCJK.SetFamilyName(aFontCJK.GetFamilyName());
    aSvxFontItemCJK.SetStyleName(OUString());
    aSvxFontItemCJK.SetPitch( aFontCJK.GetPitch());
    aSvxFontItemCJK.SetCharSet( aFontCJK.GetCharSet());
    pItemPool->SetUserDefaultItem(aSvxFontItemCJK);

    // get DEFAULTFONT_CTL_TEXT and set at pool as dynamic default
    vcl::Font aFontCTL(OutputDevice::GetDefaultFont(DefaultFontType::CTL_TEXT, nLanguage, GetDefaultFontFlags::OnlyOne));
    aSvxFontItemCTL.SetFamily(aFontCTL.GetFamilyType());
    aSvxFontItemCTL.SetFamilyName(aFontCTL.GetFamilyName());
    aSvxFontItemCTL.SetStyleName(OUString());
    aSvxFontItemCTL.SetPitch( aFontCTL.GetPitch() );
    aSvxFontItemCTL.SetCharSet( aFontCTL.GetCharSet());
    pItemPool->SetUserDefaultItem(aSvxFontItemCTL);

    // set dynamic FontHeight defaults
    pItemPool->SetUserDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT ) );
    pItemPool->SetUserDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    pItemPool->SetUserDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CTL ) );

    // set FontColor defaults
    pItemPool->SetUserDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor(), EE_CHAR_COLOR) );
}

SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
{
    m_pDrawOutliner->SetTextObj(pObj);
    return *m_pDrawOutliner;
}

SdrOutliner& SdrModel::GetChainingOutliner(const SdrTextObj* pObj) const
{
    m_pChainingOutliner->SetTextObj(pObj);
    return *m_pChainingOutliner;
}

const SdrTextObj* SdrModel::GetFormattingTextObj() const
{
    if (m_pDrawOutliner!=nullptr) {
        return m_pDrawOutliner->GetTextObj();
    }
    return nullptr;
}

void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, bool bInit )
{
    // Initialization of the Outliners for drawing text and HitTest
    if( bInit )
    {
        pOutliner->EraseVirtualDevice();
        pOutliner->SetUpdateLayout(false);
        pOutliner->SetEditTextObjectPool(m_pItemPool.get());
        pOutliner->SetDefTab(m_nDefaultTabulator);
    }

    pOutliner->SetRefDevice(GetRefDevice());
    Outliner::SetForbiddenCharsTable(GetForbiddenCharsTable());
    pOutliner->SetAsianCompressionMode( mnCharCompressType );
    pOutliner->SetKernAsianPunctuation( IsKernAsianPunctuation() );
    pOutliner->SetAddExtLeading( IsAddExtLeading() );

    if ( !GetRefDevice() )
    {
        MapMode aMapMode(m_eObjUnit);
        pOutliner->SetRefMapMode(aMapMode);
    }
}

void SdrModel::SetRefDevice(OutputDevice* pDev)
{
    m_pRefOutDev=pDev;
    ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
    ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
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
        TOOLS_WARN_EXCEPTION("svx", "");
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
    if (m_nDefaultTabulator!=nVal) {
        m_nDefaultTabulator=nVal;
        Outliner& rOutliner=GetDrawOutliner();
        rOutliner.SetDefTab(nVal);
        Broadcast(SdrHint(SdrHintKind::DefaultTabChange));
        ImpReformatAllTextObjects();
    }
}

void SdrModel::ImpSetUIUnit()
{
    if(0 == m_aUIScale.GetNumerator() || 0 == m_aUIScale.GetDenominator())
    {
        m_aUIScale = Fraction(1,1);
    }

    m_nUIUnitDecimalMark = 0;

    o3tl::Length eFrom = MapToO3tlLength(m_eObjUnit, o3tl::Length::invalid);
    o3tl::Length eTo;

    switch (m_eUIUnit)
    {
        case FieldUnit::CHAR:
        case FieldUnit::LINE:
            eTo = o3tl::Length::invalid;
            break;
        case FieldUnit::PERCENT:
            m_nUIUnitDecimalMark += 2;
            [[fallthrough]];
        default:
            eTo = FieldToO3tlLength(m_eUIUnit, o3tl::Length::invalid);
    } // switch

    sal_Int32 nMul = 1, nDiv = 1;
    if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
    {
        const auto& [mul, div] = o3tl::getConversionMulDiv(eFrom, eTo);
        nMul = mul;
        nDiv = div;
    }
    // #i89872# take Unit of Measurement into account
    if(1 != m_aUIScale.GetDenominator() || 1 != m_aUIScale.GetNumerator())
    {
        // divide by UIScale
        nMul *= m_aUIScale.GetDenominator();
        nDiv *= m_aUIScale.GetNumerator();
    }

    // shorten trailing zeros for dividend
    while(0 == (nMul % 10))
    {
        m_nUIUnitDecimalMark--;
        nMul /= 10;
    }

    // shorten trailing zeros for divisor
    while(0 == (nDiv % 10))
    {
        m_nUIUnitDecimalMark++;
        nDiv /= 10;
    }

    // end preparations, set member values
    m_aUIUnitFact = Fraction(sal_Int32(nMul), sal_Int32(nDiv));
    m_aUIUnitStr = GetUnitString(m_eUIUnit);
}

void SdrModel::SetScaleUnit(MapUnit eMap)
{
    if (m_eObjUnit!=eMap) {
        m_eObjUnit=eMap;
        m_pItemPool->SetDefaultMetric(m_eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
        ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit)
{
    if (m_eUIUnit!=eUnit) {
        m_eUIUnit=eUnit;
        ImpSetUIUnit();
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIScale(const Fraction& rScale)
{
    if (m_aUIScale!=rScale) {
        m_aUIScale=rScale;
        ImpSetUIUnit();
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit, const Fraction& rScale)
{
    if (m_eUIUnit!=eUnit || m_aUIScale!=rScale) {
        m_eUIUnit=eUnit;
        m_aUIScale=rScale;
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
            return u"/100mm"_ustr;
        case FieldUnit::MM     :
            return u"mm"_ustr;
        case FieldUnit::CM     :
            return u"cm"_ustr;
        case FieldUnit::M      :
            return u"m"_ustr;
        case FieldUnit::KM     :
            return u"km"_ustr;
        case FieldUnit::TWIP   :
            return u"twip"_ustr;
        case FieldUnit::POINT  :
            return u"pt"_ustr;
        case FieldUnit::PICA   :
            return u"pica"_ustr;
        case FieldUnit::INCH   :
            return u"\""_ustr;
        case FieldUnit::FOOT   :
            return u"ft"_ustr;
        case FieldUnit::MILE   :
            return u"mile(s)"_ustr;
        case FieldUnit::PERCENT:
            return u"%"_ustr;
    }
}

OUString SdrModel::GetMetricString(tools::Long nVal, bool bNoUnitChars, sal_Int32 nNumDigits) const
{
    // #i22167#
    // change to double precision usage to not lose decimal places
    const bool bNegative(nVal < 0);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc(aSysLoc.GetLocaleData());
    double fLocalValue(double(nVal) * double(m_aUIUnitFact));

    if(bNegative)
    {
        fLocalValue = -fLocalValue;
    }

    if( -1 == nNumDigits )
    {
        nNumDigits = LocaleDataWrapper::getNumDigits();
    }

    sal_Int32 nDecimalMark(m_nUIUnitDecimalMark);

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

    OUStringBuffer aBuf = OUString::number(static_cast<sal_Int32>(fLocalValue + 0.5));

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
        aBuf.append(m_aUIUnitStr);

    return aBuf.makeStringAndClear();
}

OUString SdrModel::GetAngleString(Degree100 nAngle)
{
    bool bNeg = nAngle < 0_deg100;

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
        sal_uInt16 nCount=sal_uInt16(maMasterPages.size());
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            SdrPage* pPg = maMasterPages[i].get();
            pPg->SetPageNum(i);
        }
        m_bMPgNumsDirty=false;
    }
    else
    {
        sal_uInt16 nCount=sal_uInt16(maPages.size());
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            SdrPage* pPg = maPages[i].get();
            pPg->SetPageNum(i);
        }
        m_bPagNumsDirty=false;
    }
}

void SdrModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nCount = GetPageCount();
    if (nPos > nCount)
        nPos = nCount;

    maPages.insert(maPages.begin() + nPos, pPage);
    PageListChanged();
    pPage->SetInserted();
    pPage->SetPageNum(nPos);

    if (mbMakePageObjectsNamesUnique)
        pPage->MakePageObjectsNamesUnique();

    if (nPos<nCount) m_bPagNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPage);
    Broadcast(aHint);
}

void SdrModel::DeletePage(sal_uInt16 nPgNum)
{
    RemovePage(nPgNum);
}

rtl::Reference<SdrPage> SdrModel::RemovePage(sal_uInt16 nPgNum)
{
    rtl::Reference<SdrPage> pPg = maPages[nPgNum];
    maPages.erase(maPages.begin()+nPgNum);
    PageListChanged();
    if (pPg) {
        pPg->SetInserted(false);
    }
    m_bPagNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPg.get());
    Broadcast(aHint);
    return pPg;
}

void SdrModel::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    rtl::Reference<SdrPage> pPg = std::move(maPages[nPgNum]);
    if (pPg) {
        maPages.erase(maPages.begin()+nPgNum); // shortcut to avoid two broadcasts
        PageListChanged();
        pPg->SetInserted(false);
        InsertPage(pPg.get(), nNewPos);
    }
    else
        RemovePage(nPgNum);
}

void SdrModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    sal_uInt16 nCount=GetMasterPageCount();
    if (nPos>nCount) nPos=nCount;
    maMasterPages.insert(maMasterPages.begin()+nPos,pPage);
    MasterPageListChanged();
    pPage->SetInserted();
    pPage->SetPageNum(nPos);

    if (nPos<nCount) {
        m_bMPgNumsDirty=true;
    }

    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPage);
    Broadcast(aHint);
}

void SdrModel::DeleteMasterPage(sal_uInt16 nPgNum)
{
    RemoveMasterPage(nPgNum);
}

rtl::Reference<SdrPage> SdrModel::RemoveMasterPage(sal_uInt16 nPgNum)
{
    rtl::Reference<SdrPage> pRetPg = std::move(maMasterPages[nPgNum]);
    maMasterPages.erase(maMasterPages.begin()+nPgNum);
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

    m_bMPgNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pRetPg.get());
    Broadcast(aHint);
    return pRetPg;
}

void SdrModel::MoveMasterPage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    rtl::Reference<SdrPage> pPg = std::move(maMasterPages[nPgNum]);
    maMasterPages.erase(maMasterPages.begin()+nPgNum);
    MasterPageListChanged();
    if (pPg) {
        pPg->SetInserted(false);
        maMasterPages.insert(maMasterPages.begin()+nNewPos,pPg);
        MasterPageListChanged();
    }
    m_bMPgNumsDirty=true;
    SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, pPg.get());
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
        rtl::Reference<SdrPage> pPg = pPagePtrs[nCopyNum];
        sal_uInt16 nPageNum2=pPg->GetPageNum();
        if (!bMoveNoCopy)
        {
            const SdrPage* pPg1=GetPage(nPageNum2);

            // Clone to local model
            pPg = pPg1->CloneSdrPage(*this);

            InsertPage(pPg.get(), nDestNum);
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
            InsertPage(pPg.get(), nDestNum);
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
                rtl::Reference<SdrPage> pPg = pPg1->CloneSdrPage(*this);

                if(!bTreadSourceAsConst)
                {
                    // if requested, delete original/modify original model
                    rSourceModel.RemoveMasterPage(i);
                }

                if (pPg!=nullptr) {
                    // Now append all of them to the end of the DstModel.
                    // Don't use InsertMasterPage(), because everything is
                    // inconsistent until all are in.
                    maMasterPages.insert(maMasterPages.begin()+nDstMasterPageCnt, pPg);
                    MasterPageListChanged();
                    pPg->SetInserted();
                    m_bMPgNumsDirty=true;
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
        sal_uInt16 nMergeCount=sal_uInt16(std::abs(static_cast<tools::Long>(static_cast<tools::Long>(nFirstPageNum)-nLastPageNum))+1);
        if (nDestPos>GetPageCount()) nDestPos=GetPageCount();
        while (nMergeCount>0)
        {
            // Always Clone to new model
            const SdrPage* pPg1(rSourceModel.GetPage(nSourcePos));
            rtl::Reference<SdrPage> pPg = pPg1->CloneSdrPage(*this);

            if(!bTreadSourceAsConst)
            {
                // if requested, delete original/modify original model
                rSourceModel.RemovePage(nSourcePos);
            }

            if (pPg!=nullptr) {
                InsertPage(pPg.get(),nDestPos);
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

    m_bMPgNumsDirty=true;
    m_bPagNumsDirty=true;

    SetChanged();
    // TODO: Missing: merging and mapping of layers
    // at the objects as well as at the MasterPageDescriptors
    if (bUndo) EndUndo();
}

void SdrModel::SetStarDrawPreviewMode(bool bPreview)
{
    if (!bPreview && m_bStarDrawPreviewMode && GetPageCount())
    {
        // Resetting is not allowed, because the Model might not be loaded completely
        SAL_WARN("svx", "SdrModel::SetStarDrawPreviewMode(): Resetting not allowed, because Model might not be complete.");
    }
    else
    {
        m_bStarDrawPreviewMode = bPreview;
    }
}

void SdrModel::setTheme(std::shared_ptr<model::Theme> const& pTheme)
{
    mpImpl->mpTheme = pTheme;
}

std::shared_ptr<model::Theme> const& SdrModel::getTheme() const
{
    return mpImpl->mpTheme;
}

uno::Reference< frame::XModel > const & SdrModel::getUnoModel()
{
    if( !mxUnoModel.is() )
        mxUnoModel = createUnoModel();

    return mxUnoModel;
}

void SdrModel::setUnoModel(const uno::Reference<frame::XModel>& xModel)
{
    mxUnoModel = xModel;
}

void SdrModel::adaptSizeAndBorderForAllPages(
    const Size& /*rNewSize*/,
    tools::Long /*nLeft*/,
    tools::Long /*nRight*/,
    tools::Long /*nUpper*/,
    tools::Long /*nLower*/)
{
    // base implementation does currently nothing. It may be added if needed,
    // but we are on SdrModel level here, thus probably have not enough information
    // to do this for higher-level (derived) Models (e.g. Draw/Impress)
}

uno::Reference< frame::XModel > SdrModel::createUnoModel()
{
    OSL_FAIL( "SdrModel::createUnoModel() - base implementation should not be called!" );
    return nullptr;
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
    if( !(pSourceSet && pDestSet && (pSourceSet != pDestSet )) )
        return;

    SfxWhichIter aWhichIter(*pSourceSet);
    sal_uInt16 nWhich(aWhichIter.FirstWhich());
    const SfxPoolItem *pPoolItem;

    while(nWhich)
    {
        if(SfxItemState::SET == aWhichIter.GetItemState(false, &pPoolItem))
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
                pDestSet->Put(std::move(pResultItem));
            else
                pDestSet->Put(*pPoolItem);
        }
        nWhich = aWhichIter.NextWhich();
    }
}


void SdrModel::SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars)
{
    mpForbiddenCharactersTable = xForbiddenChars;

    ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
    ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
}


void SdrModel::SetCharCompressType( CharCompressType nType )
{
    if( nType != mnCharCompressType )
    {
        mnCharCompressType = nType;
        ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
        ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
    }
}

void SdrModel::SetKernAsianPunctuation( bool bEnabled )
{
    if( mbKernAsianPunctuation != bEnabled )
    {
        mbKernAsianPunctuation = bEnabled;
        ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
        ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
    }
}

void SdrModel::SetAddExtLeading( bool bEnabled )
{
    if( mbAddExtLeading != bEnabled )
    {
        mbAddExtLeading = bEnabled;
        ImpSetOutlinerDefaults( m_pDrawOutliner.get() );
        ImpSetOutlinerDefaults( m_pHitTestOutliner.get() );
    }
}

void SdrModel::SetCompatibilityFlag(SdrCompatibilityFlag eFlag, bool bEnabled)
{
    switch (eFlag)
    {
        case SdrCompatibilityFlag::AnchoredTextOverflowLegacy:
            mpImpl->mbAnchoredTextOverflowLegacy = bEnabled;
            break;
        case SdrCompatibilityFlag::LegacyFontwork:
            mpImpl->mbLegacyFontwork = bEnabled;
            break;
        case SdrCompatibilityFlag::ConnectorUseSnapRect:
            mpImpl->mbConnectorUseSnapRect = bEnabled;
            break;
        case SdrCompatibilityFlag::IgnoreBreakAfterMultilineField:
            mpImpl->mbIgnoreBreakAfterMultilineField = bEnabled;
            break;
    }
}

bool SdrModel::GetCompatibilityFlag(SdrCompatibilityFlag eFlag) const
{
    switch (eFlag)
    {
        case SdrCompatibilityFlag::AnchoredTextOverflowLegacy:
            return mpImpl->mbAnchoredTextOverflowLegacy;
        case SdrCompatibilityFlag::LegacyFontwork:
            return mpImpl->mbLegacyFontwork;
        case SdrCompatibilityFlag::ConnectorUseSnapRect:
            return mpImpl->mbConnectorUseSnapRect;
        case SdrCompatibilityFlag::IgnoreBreakAfterMultilineField:
            return mpImpl->mbIgnoreBreakAfterMultilineField;
        default:
            return false;
    }
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
    aRet.push_back(m_pDrawOutliner.get());
    aRet.push_back(m_pHitTestOutliner.get());

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

void SdrModel::ReadUserDataSequenceValue(const beans::PropertyValue* pValue)
{
    if (pValue->Name == "AnchoredTextOverflowLegacy")
    {
        bool bBool = false;
        if (pValue->Value >>= bBool)
        {
            mpImpl->mbAnchoredTextOverflowLegacy = bBool;
        }
    }
    else if (pValue->Name == "ConnectorUseSnapRect")
    {
        bool bBool = false;
        if (pValue->Value >>= bBool)
        {
            mpImpl->mbConnectorUseSnapRect = bBool;
        }
    }
    else if (pValue->Name == "LegacySingleLineFontwork")
    {
        bool bBool = false;
        if ((pValue->Value >>= bBool) && mpImpl->mbLegacyFontwork != bBool)
        {
            mpImpl->mbLegacyFontwork = bBool;
            // tdf#148000 hack: reset all CustomShape geometry as they may depend on this property
            // Ideally this ReadUserDataSequenceValue should be called before geometry creation
            // Once the calling order will be fixed, this hack will not be needed.
            for (size_t i = 0; i < maPages.size(); ++i)
            {
                if (const SdrPage* pPage = maPages[i].get())
                {
                    SdrObjListIter aIter(pPage, SdrIterMode::DeepWithGroups);
                    while (aIter.IsMore())
                    {
                        SdrObject* pTempObj = aIter.Next();
                        if (SdrObjCustomShape* pShape = dynamic_cast<SdrObjCustomShape*>(pTempObj))
                        {
                            pShape->InvalidateRenderGeometry();
                        }
                    }
                }
            }
        }
    }
    else if (pValue->Name == "IgnoreBreakAfterMultilineField")
    {
        bool bBool = false;
        if (pValue->Value >>= bBool)
        {
            mpImpl->mbIgnoreBreakAfterMultilineField = bBool;
        }
    }
}

void SdrModel::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rValues)
{
    std::vector< std::pair< OUString, uno::Any > > aUserData
    {
        { "AnchoredTextOverflowLegacy", uno::Any(GetCompatibilityFlag(SdrCompatibilityFlag::AnchoredTextOverflowLegacy)) },
        { "LegacySingleLineFontwork", uno::Any(GetCompatibilityFlag(SdrCompatibilityFlag::LegacyFontwork)) },
        { "ConnectorUseSnapRect", uno::Any(GetCompatibilityFlag(SdrCompatibilityFlag::ConnectorUseSnapRect)) },
        { "IgnoreBreakAfterMultilineField", uno::Any(GetCompatibilityFlag(SdrCompatibilityFlag::IgnoreBreakAfterMultilineField)) }
    };

    const sal_Int32 nOldLength = rValues.getLength();
    rValues.realloc(nOldLength + aUserData.size());

    beans::PropertyValue* pValue = &(rValues.getArray()[nOldLength]);

    for (const auto &aIter : aUserData)
    {
        pValue->Name = aIter.first;
        pValue->Value = aIter.second;
        ++pValue;
    }
}

const SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum) const
{
    return nPgNum < maPages.size() ? maPages[nPgNum].get() : nullptr;
}

SdrPage* SdrModel::GetPage(sal_uInt16 nPgNum)
{
    return nPgNum < maPages.size() ? maPages[nPgNum].get() : nullptr;
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
    return m_pTextChain.get();
}

const SdrPage* SdrModel::GetMasterPage(sal_uInt16 nPgNum) const
{
    DBG_ASSERT(nPgNum < maMasterPages.size(), "SdrModel::GetMasterPage: Access out of range (!)");
    return maMasterPages[nPgNum].get();
}

SdrPage* SdrModel::GetMasterPage(sal_uInt16 nPgNum)
{
    DBG_ASSERT(nPgNum < maMasterPages.size(), "SdrModel::GetMasterPage: Access out of range (!)");
    return maMasterPages[nPgNum].get();
}

sal_uInt16 SdrModel::GetMasterPageCount() const
{
    return sal_uInt16(maMasterPages.size());
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
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrModel"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("maMasterPages"));
    for (size_t i = 0; i < maMasterPages.size(); ++i)
    {
        if (const SdrPage* pPage = maMasterPages[i].get())
        {
            pPage->dumpAsXml(pWriter);
        }
    }
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("maPages"));
    for (size_t i = 0; i < maPages.size(); ++i)
    {
        if (const SdrPage* pPage = maPages[i].get())
        {
            pPage->dumpAsXml(pWriter);
        }
    }
    (void)xmlTextWriterEndElement(pWriter);

    if (mpImpl->mpTheme)
    {
        mpImpl->mpTheme->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

const uno::Sequence<sal_Int8>& SdrModel::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSdrModelUnoTunnelImplementationId;
    return theSdrModelUnoTunnelImplementationId.getSeq();
}


SdrHint::SdrHint(SdrHintKind eNewHint)
:   SfxHint(SfxHintId::ThisIsAnSdrHint),
    meHint(eNewHint),
    mpObj(nullptr),
    mpPage(nullptr)
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj)
:   SfxHint(SfxHintId::ThisIsAnSdrHint),
    meHint(eNewHint),
    mpObj(&rNewObj),
    mpPage(rNewObj.getSdrPageFromSdrObject())
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrPage* pPage)
:   SfxHint(SfxHintId::ThisIsAnSdrHint),
    meHint(eNewHint),
    mpObj(nullptr),
    mpPage(pPage)
{
}

SdrHint::SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj, const SdrPage* pPage)
:   SfxHint(SfxHintId::ThisIsAnSdrHint),
    meHint(eNewHint),
    mpObj(&rNewObj),
    mpPage(pPage)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
