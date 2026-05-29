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

#include <libxml/xmlwriter.h>

#include "PageListWatcher.hxx"
#include <ViewShellBase.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <editeng/forbiddencharacterstable.hxx>

#include <svl/srchitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <tools/debug.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/useroptions.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Draw.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <svx/fillbitmaplink.hxx>
#include <svx/sdrpageuser.hxx>
#include <svx/sdrobjectuser.hxx>
#include <xmloff/SoundReference.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/lokhelper.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <unotools/securityoptions.hxx>
#include <Outliner.hxx>
#include <sdmod.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdotext.hxx>
#include <editeng/unolingu.hxx>
#include <svl/itempool.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <editeng/outlobj.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustring.hxx>

#include <editeng/outliner.hxx>
#include <drawdoc.hxx>
#include <SlideSectionManager.hxx>
#include <sdpage.hxx>
#include <anminfo.hxx>
#include <svx/svditer.hxx>
#include <strings.hrc>
#include <glob.hxx>
#include <stlpool.hxx>
#include <sdresid.hxx>
#include <customshowlist.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <sdxfer.hxx>
#include <optsitem.hxx>
#include <FrameView.hxx>
#include <undo/undomanager.hxx>
#include <sdundogr.hxx>
#include <undopage.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <unokywds.hxx>
#include <unomodel.hxx>

namespace com::sun::star::linguistic2 { class XHyphenator; }
namespace com::sun::star::linguistic2 { class XSpellChecker1; }

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

using namespace com::sun::star::xml::dom;
using ::com::sun::star::uno::Reference;


SdDrawDocument* SdDrawDocument::s_pDocLockedInsertingLinks = nullptr;
SdDrawDocument* SdDrawDocument::s_pLast = nullptr;

PresentationSettings::PresentationSettings()
:   mbAll( true ),
    mbEndless( false ),
    mbCustomShow(false),
    mbManual( false ),
    mbMouseVisible( false ),
    mbMouseAsPen( false ),
    mbLockedPages( false ),
    mbAlwaysOnTop( false ),
    mbFullScreen( true ),
    mbAnimationAllowed( true ),
    mnPauseTimeout( 0 ),
    mbShowPauseLogo( false ),
    mbStartCustomShow( false ),
    mbInteractive( true )
{
}

SdDrawDocument::SdDrawDocument(DocumentType eType, SfxObjectShell* pDrDocSh)
:   FmFormModel(
        nullptr,
        pDrDocSh)
, mpDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh))
, mpCreatingTransferable( nullptr )
, mbHasOnlineSpellErrors(false)
, mbInitialOnlineSpellingEnabled(true)
, mbNewOrLoadCompleted(false)
, mbOnlineSpell(false)
, mnStartWithPresentation(0)
, mbExitAfterPresenting( false )
, meLanguage( LANGUAGE_SYSTEM )
, meLanguageCJK( LANGUAGE_SYSTEM )
, meLanguageCTL( LANGUAGE_SYSTEM )
, mePageNumType(SVX_NUM_ARABIC)
, mbAllocDocSh(false)
, meDocType(eType)
, mbEmbedFonts(false)
, mbEmbedUsedFontsOnly(false)
, mbEmbedFontScriptLatin(true)
, mbEmbedFontScriptAsian(true)
, mbEmbedFontScriptComplex(true)
, mnImagePreferredDPI(0)
{
    m_bThemedControls = false;

    mpDrawPageListWatcher.reset(new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher.reset(new ImpMasterPageListWatcher(*this));

    this->SetImpress(true);

    InitLayoutVector();
    InitObjectVector();
    SetObjectShell(pDrDocSh);       // for VCDrawModel

    if (mpDocSh)
    {
        SetSwapGraphics();
    }

    SdOptions* pOptions = SdModule::get()->GetSdOptions(meDocType);
    SvtSysLocale aSysLocale;
    // Set measuring unit (of the application) and scale (of SdMod)
    if (comphelper::IsFuzzing())
        SetUIUnit(FieldUnit::CM, 1.0);
    else
    {
        sal_Int32 nX = officecfg::Office::Draw::Zoom::ScaleX::get();
        sal_Int32 nY = officecfg::Office::Draw::Zoom::ScaleY::get();

        // Allow UI scale only for draw documents.
        if( eType == DocumentType::Draw )
            if (aSysLocale.GetLocaleData().getMeasurementSystemEnum() == MeasurementSystem::Metric)
                SetUIUnit( static_cast<FieldUnit>(officecfg::Office::Draw::Layout::Other::MeasureUnit::Metric::get()), double(nX) / nY );  // user-defined
            else
                SetUIUnit( static_cast<FieldUnit>(officecfg::Office::Draw::Layout::Other::MeasureUnit::NonMetric::get()), double(nX) / nY );  // user-defined
        else
            if (aSysLocale.GetLocaleData().getMeasurementSystemEnum() == MeasurementSystem::Metric)
                SetUIUnit( static_cast<FieldUnit>(officecfg::Office::Impress::Layout::Other::MeasureUnit::Metric::get()), 1.0 );    // default
            else
                SetUIUnit( static_cast<FieldUnit>(officecfg::Office::Impress::Layout::Other::MeasureUnit::NonMetric::get()), 1.0 );    // default
    }

    SetScaleUnit(MapUnit::Map100thMM);
    SetDefaultFontHeight(o3tl::convert(24, o3tl::Length::pt, o3tl::Length::mm100));

    m_pItemPool->SetDefaultMetric(MapUnit::Map100thMM);
    SetTextDefaults();

    // DrawingEngine has to know where it is...
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

    // Set StyleSheetPool for DrawOutliner, so text objects can be read correctly.
    // The link to the StyleRequest handler of the document is set later, in
    // NewOrLoadCompleted, because only then do all the templates exist.
    SdrOutliner& rOutliner = GetDrawOutliner();
    rOutliner.SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    SetCalcFieldValueHdl( &rOutliner );

    // set linguistic options
    if (!comphelper::IsFuzzing())
    {
        const SvtLinguConfig    aLinguConfig;
        SvtLinguOptions         aOptions;
        aLinguConfig.GetOptions( aOptions );

        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage,
            css::i18n::ScriptType::LATIN), EE_CHAR_LANGUAGE );
        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CJK,
            css::i18n::ScriptType::ASIAN), EE_CHAR_LANGUAGE_CJK );
        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CTL,
            css::i18n::ScriptType::COMPLEX), EE_CHAR_LANGUAGE_CTL );

        mbOnlineSpell = aOptions.bIsSpellAuto;
    }

    LanguageType eRealLanguage = MsLangId::getRealLanguage( meLanguage );
    moCharClass.emplace(LanguageTag( eRealLanguage));

    // If the current application language is a language that uses right-to-left text...
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();

    // for korean and japanese languages we have a different default for apply spacing between asian, latin and ctl text
    if (MsLangId::isKorean(eRealCTLLanguage) || (LANGUAGE_JAPANESE == eRealCTLLanguage))
    {
        GetPool().GetSecondaryPool()->SetUserDefaultItem( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
    }

    // Set DefTab and SpellOptions for the SD module
    if (eType == DocumentType::Impress)
        if (aSysLocale.GetLocaleData().getMeasurementSystemEnum() == MeasurementSystem::Metric)
            SetDefaultTabulator( static_cast<sal_uInt16>(officecfg::Office::Impress::Layout::Other::TabStop::Metric::get()) );
        else
            SetDefaultTabulator( static_cast<sal_uInt16>(officecfg::Office::Impress::Layout::Other::TabStop::NonMetric::get()) );
    else
        if (aSysLocale.GetLocaleData().getMeasurementSystemEnum() == MeasurementSystem::Metric)
            SetDefaultTabulator( static_cast<sal_uInt16>(officecfg::Office::Draw::Layout::Other::TabStop::Metric::get()) );
        else
            SetDefaultTabulator( static_cast<sal_uInt16>(officecfg::Office::Draw::Layout::Other::TabStop::NonMetric::get()) );

    try
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            rOutliner.SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            rOutliner.SetHyphenator( xHyphenator );

        SetForbiddenCharsTable(SvxForbiddenCharactersTable::makeForbiddenCharactersTable(::comphelper::getProcessComponentContext()));
    }
    catch(...)
    {
        OSL_FAIL("Can't get SpellChecker");
    }

    rOutliner.SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    if (mpDocSh)
    {
        // Setting the LinkManager also installs the fill bitmap link tracker,
        // so deferred remote fills on shapes, page backgrounds and the
        // Background style are registered per host as they are set.
        SetLinkManager( new sfx2::LinkManager(mpDocSh) );
    }

    EEControlBits nCntrl = rOutliner.GetControlWord();
    nCntrl |= EEControlBits::ALLOWBIGOBJS;

    if (mbOnlineSpell)
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;

    nCntrl &= ~ EEControlBits::ULSPACESUMMATION;
    if ( meDocType != DocumentType::Impress )
        SetSummationOfParagraphs( false );
    else
    {
        SetSummationOfParagraphs( pOptions->IsSummationOfParagraphs() );
        if ( pOptions->IsSummationOfParagraphs() )
            nCntrl |= EEControlBits::ULSPACESUMMATION;
    }
    rOutliner.SetControlWord(nCntrl);

    // Initialize the printer independent layout mode
    SetPrinterIndependentLayout (pOptions->GetPrinterIndependentLayout());

    // Set the StyleSheetPool for HitTestOutliner.
    // The link to the StyleRequest handler of the document is set later, in
    // NewOrLoadCompleted, because only then do all the templates exist.
    m_pHitTestOutliner->SetStyleSheetPool( static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()) );

    SetCalcFieldValueHdl( m_pHitTestOutliner.get() );

    try
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            m_pHitTestOutliner->SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            m_pHitTestOutliner->SetHyphenator( xHyphenator );
    }
    catch(...)
    {
        OSL_FAIL("Can't get SpellChecker");
    }

    m_pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    EEControlBits nCntrl2 = m_pHitTestOutliner->GetControlWord();
    nCntrl2 |= EEControlBits::ALLOWBIGOBJS;
    nCntrl2 &= ~EEControlBits::ONLINESPELLING;

    nCntrl2 &= ~ EEControlBits::ULSPACESUMMATION;
    if ( pOptions->IsSummationOfParagraphs() )
        nCntrl2 |= EEControlBits::ULSPACESUMMATION;

    m_pHitTestOutliner->SetControlWord( nCntrl2 );

    /** Create layers
      *
      * We create the following default layers on all pages and master pages:
      *
      * sUNO_LayerName_layout; "layout": default layer for drawing objects of normal pages
      * localized by SdResId(STR_LAYER_LAYOUT)
      *
      * sUNO_LayerName_background; "background": background of the master page
      * localized by SdResId(STR_LAYER_BCKGRND)
      *           (currently unused within normal pages and not visible to users)
      *
      * sUNO_LayerName_background_objects; "backgroundobjects": objects on the background of master pages
      * localized by SdResId(STR_LAYER_BCKGRNDOBJ)
      *           (currently unused within normal pages)
      *
      * sUNO_LayerName_controls; "controls": default layer for controls
      * localized by SdResId(STR_LAYER_CONTROLS)
      *           (currently special handling in regard to z-order)
      *
      * sUNO_LayerName_measurelines; "measurelines" : default layer for measure lines
      * localized by SdResId(STR_LAYER_MEASURELINES)
      */

    {
        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        rLayerAdmin.NewLayer( sUNO_LayerName_layout );
        rLayerAdmin.NewLayer( sUNO_LayerName_background );
        rLayerAdmin.NewLayer( sUNO_LayerName_background_objects );
        rLayerAdmin.NewLayer( sUNO_LayerName_controls);
        rLayerAdmin.NewLayer( sUNO_LayerName_measurelines );

        rLayerAdmin.SetControlLayerName(sUNO_LayerName_controls);
    }

    s_pLast = this;
}

// Destructor
SdDrawDocument::~SdDrawDocument()
{
    mbDestroying = true;
    s_pLast = nullptr;

    Broadcast(SdrHint(SdrHintKind::ModelCleared));

    if (mpWorkStartupTimer)
    {
        if ( mpWorkStartupTimer->IsActive() )
            mpWorkStartupTimer->Stop();

        mpWorkStartupTimer.reset();
    }

    StopOnlineSpelling();
    mpOnlineSearchItem.reset();

    CloseBookmarkDoc();
    SetAllocDocSh(false);

    ClearModel(true);

    if (m_pLinkManager)
    {
        // Release BaseLinks
        if ( !m_pLinkManager->GetLinks().empty() )
        {
            m_pLinkManager->Remove( 0, m_pLinkManager->GetLinks().size() );
        }

        delete m_pLinkManager;
        m_pLinkManager = nullptr;
    }

    maFrameViewList.clear();
    mpCustomShowList.reset();
    mpOutliner.reset();
    mpInternalOutliner.reset();
    moCharClass.reset();
    mbDestroying = false;
}

void SdDrawDocument::adaptSizeAndBorderForAllPages(
    const Size& rNewSize,
    ::tools::Long nLeft,
    ::tools::Long nRight,
    ::tools::Long nUpper,
    ::tools::Long nLower)
{
    const sal_uInt16 nMasterPageCnt(GetMasterSdPageCount(PageKind::Standard));
    const sal_uInt16 nPageCnt(GetSdPageCount(PageKind::Standard));

    if(0 == nMasterPageCnt && 0 == nPageCnt)
    {
        return;
    }

    SdPage* pPage(0 != nPageCnt ? GetSdPage(0, PageKind::Standard) : GetMasterSdPage(0, PageKind::Standard));

    // call fully implemented local version, including getting
    // some more information from one of the Pages (1st one)
    AdaptPageSizeForAllPages(
        rNewSize,
        PageKind::Standard,
        nullptr,
        nLeft,
        nRight,
        nUpper,
        nLower,
        true,
        pPage->GetOrientation(),
        pPage->GetPaperBin(),
        pPage->IsBackgroundFullSize());

    // adjust handout page to new format of the standard page
    if(0 != nPageCnt)
    {
        GetSdPage(0, PageKind::Handout)->CreateTitleAndLayout(true);
    }
}

void SdDrawDocument::ResizeCurrentPage(
    SdPage* pPage,
    const Size& rNewSize,
    PageKind ePageKind,
    SdUndoGroup* pUndoGroup,
    ::tools::Long nLeft,
    ::tools::Long nRight,
    ::tools::Long nUpper,
    ::tools::Long nLower,
    bool bScaleAll,
    Orientation eOrientation,
    sal_uInt16 nPaperBin,
    bool bBackgroundFullSize)
{
    const sal_uInt16 nMasterPageCnt(GetMasterSdPageCount(ePageKind));
    const sal_uInt16 nPageCnt(GetSdPageCount(ePageKind));

    if(0 == nMasterPageCnt && 0 == nPageCnt)
    {
        return;
    }

    // TODO: handle undo action for new master page creation

    bool bIsMasterPage = pPage->IsMasterPage();

    SdPage* pMasterPage = bIsMasterPage ? pPage : static_cast<SdPage*>(&pPage->TRG_GetMasterPage());

    if (!bIsMasterPage)
    {
        // Count how many pages uses pMasterPage
        sal_uInt16 nCount = 0;
        for (sal_uInt16 i = 0; i < nPageCnt && nCount <= 1; i++)
        {
            SdPage* pDrawPage = GetSdPage(i, ePageKind);
            if (pDrawPage->TRG_HasMasterPage() &&
                static_cast<SdPage*>(&pDrawPage->TRG_GetMasterPage()) == pMasterPage)
            {
                nCount++;
            }
        }

        // If pMasterPage is used by other pages, create a new master page
        if (nCount > 1)
        {
            SdPage* pNewMasterPage = AddNewMasterPageFromExisting(pMasterPage);
            pPage->TRG_SetMasterPage(static_cast<SdrPage&>(*pNewMasterPage));
            pMasterPage = pNewMasterPage;
        }
    }

    AdaptPageSize(pMasterPage,
                  rNewSize,
                  pUndoGroup,
                  nLeft,
                  nRight,
                  nUpper,
                  nLower,
                  bScaleAll,
                  eOrientation,
                  nPaperBin,
                  bBackgroundFullSize);

    //     if ( ePageKind == PageKind::Standard )
    //     {
    //         GetMasterSdPage(i, PageKind::Notes)->CreateTitleAndLayout();
    //     }

    if (!bIsMasterPage)
    {
        AdaptPageSize(pPage,
                      rNewSize,
                      pUndoGroup,
                      nLeft,
                      nRight,
                      nUpper,
                      nLower,
                      bScaleAll,
                      eOrientation,
                      nPaperBin,
                      bBackgroundFullSize);
    }
    else
    {
        for (sal_uInt16 i = 0; i < nPageCnt; i++)
        {
            SdPage* pDrawPage = GetSdPage(i, ePageKind);
            if (pDrawPage->TRG_HasMasterPage() &&
                static_cast<SdPage*>(&pDrawPage->TRG_GetMasterPage()) == pPage)
            {
                AdaptPageSize(pDrawPage,
                              rNewSize,
                              pUndoGroup,
                              nLeft,
                              nRight,
                              nUpper,
                              nLower,
                              bScaleAll,
                              eOrientation,
                              nPaperBin,
                              bBackgroundFullSize);
            }
        }
    }

    // if ( ePageKind == PageKind::Standard )
    // {
    //     SdPage* pNotesPage = GetSdPage(i, PageKind::Notes);
    //     pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
    // }

    // Notify LOK clients of the current page size change.
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        SdXImpressDocument* pDoc = comphelper::getFromUnoTunnel<SdXImpressDocument>(pViewShell->GetCurrentDocument());
        SfxLokHelper::notifyCurrentPageSizeChangedAllViews(pDoc);
    }
}

void SdDrawDocument::AdaptPageSize(
        SdPage* pPage,
        const Size& rNewSize,
        SdUndoGroup* pUndoGroup,
        ::tools::Long nLeft,
        ::tools::Long nRight,
        ::tools::Long nUpper,
        ::tools::Long nLower,
        bool bScaleAll,
        Orientation eOrientation,
        sal_uInt16 nPaperBin,
        bool bBackgroundFullSize)
{
    if(pUndoGroup)
    {
        SdUndoAction* pUndo(
            new SdPageFormatUndoAction(
                *this,
                pPage,
                pPage->GetSize(),
                pPage->GetLeftBorder(), pPage->GetRightBorder(),
                pPage->GetUpperBorder(), pPage->GetLowerBorder(),
                pPage->GetOrientation(),
                pPage->GetPaperBin(),
                pPage->IsBackgroundFullSize(),
                rNewSize,
                nLeft, nRight,
                nUpper, nLower,
                bScaleAll,
                eOrientation,
                nPaperBin,
                bBackgroundFullSize));
        pUndoGroup->AddAction(pUndo);
    }

    if (rNewSize.Width() > 0 || nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0)
    {
        ::tools::Rectangle aNewBorderRect(nLeft, nUpper, nRight, nLower);
        pPage->ScaleObjects(rNewSize, aNewBorderRect, bScaleAll);

        if (rNewSize.Width() > 0)
        {
            pPage->SetSize(rNewSize);
        }
    }

    if( nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0 )
    {
        pPage->SetBorder(nLeft, nUpper, nRight, nLower);
    }

    pPage->SetOrientation(eOrientation);
    pPage->SetPaperBin( nPaperBin );
    pPage->SetBackgroundFullSize( bBackgroundFullSize );

    if (pPage->IsMasterPage())
    {
        pPage->CreateTitleAndLayout();
    }
    else
    {
        pPage->SetAutoLayout( pPage->GetAutoLayout() );
    }
}

void SdDrawDocument::AdaptPageSizeForAllPages(
    const Size& rNewSize,
    PageKind ePageKind,
    SdUndoGroup* pUndoGroup,
    ::tools::Long nLeft,
    ::tools::Long nRight,
    ::tools::Long nUpper,
    ::tools::Long nLower,
    bool bScaleAll,
    Orientation eOrientation,
    sal_uInt16 nPaperBin,
    bool bBackgroundFullSize)
{
    sal_uInt16 i;
    const sal_uInt16 nMasterPageCnt(GetMasterSdPageCount(ePageKind));
    const sal_uInt16 nPageCnt(GetSdPageCount(ePageKind));

    if(0 == nMasterPageCnt && 0 == nPageCnt)
    {
        return;
    }

    for (i = 0; i < nMasterPageCnt; i++)
    {
        // first, handle all master pages
        SdPage* pMasterPage(GetMasterSdPage(i, ePageKind));

        AdaptPageSize(pMasterPage,
                      rNewSize,
                      pUndoGroup,
                      nLeft,
                      nRight,
                      nUpper,
                      nLower,
                      bScaleAll,
                      eOrientation,
                      nPaperBin,
                      bBackgroundFullSize);

        if ( ePageKind == PageKind::Standard )
        {
            GetMasterSdPage(i, PageKind::Notes)->CreateTitleAndLayout();
        }
    }

    for (i = 0; i < nPageCnt; i++)
    {
        // then, handle all pages
        SdPage* pPage(GetSdPage(i, ePageKind));

        AdaptPageSize(pPage,
                      rNewSize,
                      pUndoGroup,
                      nLeft,
                      nRight,
                      nUpper,
                      nLower,
                      bScaleAll,
                      eOrientation,
                      nPaperBin,
                      bBackgroundFullSize);

        if ( ePageKind == PageKind::Standard )
        {
            SdPage* pNotesPage = GetSdPage(i, PageKind::Notes);
            pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
        }
    }

    // Notify LOK clients of the document size change.
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        SdXImpressDocument* pDoc = comphelper::getFromUnoTunnel<SdXImpressDocument>(pViewShell->GetCurrentDocument());
        SfxLokHelper::notifyDocumentSizeChangedAllViews(pDoc);
    }
}

SdrModel* SdDrawDocument::AllocModel() const
{
    return AllocSdDrawDocument();
}

namespace
{

/// Copies all user-defined properties from pSource to pDestination.
void lcl_copyUserDefinedProperties(const SfxObjectShell* pSource, const SfxObjectShell* pDestination)
{
    if (!pSource || !pDestination)
        return;

    uno::Reference<document::XDocumentProperties> xSource = pSource->getDocProperties();
    uno::Reference<document::XDocumentProperties> xDestination = pDestination->getDocProperties();
    uno::Reference<beans::XPropertyContainer> xSourcePropertyContainer = xSource->getUserDefinedProperties();
    uno::Reference<beans::XPropertyContainer> xDestinationPropertyContainer = xDestination->getUserDefinedProperties();
    uno::Reference<beans::XPropertySet> xSourcePropertySet(xSourcePropertyContainer, uno::UNO_QUERY);
    const uno::Sequence<beans::Property> aProperties = xSourcePropertySet->getPropertySetInfo()->getProperties();

    for (const beans::Property& rProperty : aProperties)
    {
        const OUString& rKey = rProperty.Name;
        uno::Any aValue = xSourcePropertySet->getPropertyValue(rKey);
        // We know that pDestination was just created, so has no properties: addProperty() will never throw.
        xDestinationPropertyContainer->addProperty(rKey, beans::PropertyAttribute::REMOVABLE, aValue);
    }
}

}

// This method creates a new document (SdDrawDocument) and returns a pointer to
// said document. The drawing engine uses this method to put the document (or
// parts of it) into the clipboard/DragServer.
SdDrawDocument* SdDrawDocument::AllocSdDrawDocument() const
{
    SdDrawDocument* pNewModel = nullptr;

    if( mpCreatingTransferable )
    {
        // Document is created for drag & drop/clipboard. To be able to
        // do this, the document has to know a DocShell (SvPersist).
        SfxObjectShell*   pObj = nullptr;
        ::sd::DrawDocShell*     pNewDocSh = nullptr;

        if( meDocType == DocumentType::Impress )
            mpCreatingTransferable->SetDocShell( new ::sd::DrawDocShell(
                SfxObjectCreateMode::EMBEDDED, true, meDocType ) );
        else
            mpCreatingTransferable->SetDocShell( new ::sd::GraphicDocShell(
                SfxObjectCreateMode::EMBEDDED ) );

        pObj = mpCreatingTransferable->GetDocShell().get();
        pNewDocSh = static_cast< ::sd::DrawDocShell*>( pObj );
        pNewDocSh->DoInitNew();
        pNewModel = pNewDocSh->GetDoc();

        // Only necessary for clipboard -
        // for drag & drop this is handled by DragServer
        SdStyleSheetPool* pOldStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        SdStyleSheetPool* pNewStylePool = static_cast<SdStyleSheetPool*>( pNewModel->GetStyleSheetPool() );

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);
        pNewStylePool->CopyCellSheets(*pOldStylePool);
        pNewStylePool->CopyTableStyles(*pOldStylePool);

        for (sal_uInt16 i = 0; i < GetMasterSdPageCount(PageKind::Standard); i++)
        {
            // Move with all of the master page's layouts
            OUString aOldLayoutName(const_cast<SdDrawDocument*>(this)->GetMasterSdPage(i, PageKind::Standard)->GetLayoutName());
            aOldLayoutName = aOldLayoutName.copy( 0, aOldLayoutName.indexOf( SD_LT_SEPARATOR ) );
            StyleSheetCopyResultVector aCreatedSheets;
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool, aCreatedSheets );
        }

        lcl_copyUserDefinedProperties(GetDocSh(), pNewDocSh);

        pNewModel->NewOrLoadCompleted( DocCreationMode::Loaded );  // loaded from source document
    }
    else if( mbAllocDocSh )
    {
        // Create a DocShell which is then returned with GetAllocedDocSh()
        SdDrawDocument* pDoc = const_cast<SdDrawDocument*>(this);
        pDoc->SetAllocDocSh(false);
        pDoc->mxAllocedDocShRef = new ::sd::DrawDocShell(
            SfxObjectCreateMode::EMBEDDED, true, meDocType);
        pDoc->mxAllocedDocShRef->DoInitNew();
        pNewModel = pDoc->mxAllocedDocShRef->GetDoc();
    }
    else
    {
        pNewModel = new SdDrawDocument(meDocType, nullptr);
    }

    return pNewModel;
}

rtl::Reference<SdPage> SdDrawDocument::AllocSdPage(bool bMasterPage)
{
    return new SdPage(*this, bMasterPage);
}

// This method creates a new page (SdPage) and returns a pointer to said page.
// The drawing engine uses this method to create pages (whose types it does
// not know, as they are _derivatives_ of SdrPage) when loading.
rtl::Reference<SdrPage> SdDrawDocument::AllocPage(bool bMasterPage)
{
    return AllocSdPage(bMasterPage);
}

// When the model has changed
void SdDrawDocument::SetChanged(bool bFlag)
{
    if (mpDocSh)
    {
        if (mbNewOrLoadCompleted && mpDocSh->IsEnableSetModified())
        {
            // Pass on to base class
            FmFormModel::SetChanged(bFlag);

            // Forward to ObjectShell
            mpDocSh->SetModified(bFlag);
        }
    }
    else
    {
        // Pass on to base class
        FmFormModel::SetChanged(bFlag);
    }
}

// The model changed, don't call anything else
void SdDrawDocument::NbcSetChanged(bool bFlag)
{
    // forward to baseclass
    FmFormModel::SetChanged(bFlag);
}

// NewOrLoadCompleted is called when the document is loaded, or when it is clear
// it won't load any more.
void SdDrawDocument::NewOrLoadCompleted(DocCreationMode eMode)
{
    if (eMode == DocCreationMode::New)
    {
        // New document:
        // create slideshow and default templates,
        // create pool for virtual controls
        CreateLayoutTemplates();
        CreateDefaultCellStyles();

        static_cast< SdStyleSheetPool* >( mxStyleSheetPool.get() )->CreatePseudosIfNecessary();
    }
    else if (eMode == DocCreationMode::Loaded)
    {
            // Document has finished loading

        CheckMasterPages();

        if ( GetMasterSdPageCount(PageKind::Standard) > 1 )
            RemoveUnnecessaryMasterPages( nullptr, true, false );

        for ( sal_uInt16 i = 0; i < GetPageCount(); i++ )
        {
            // Check for correct layout names
            SdPage* pPage = static_cast<SdPage*>( GetPage( i ) );

            if(pPage->TRG_HasMasterPage())
            {
                SdPage& rMaster = static_cast<SdPage&>(pPage->TRG_GetMasterPage() );

                if(rMaster.GetLayoutName() != pPage->GetLayoutName())
                {
                    pPage->SetLayoutName(rMaster.GetLayoutName());
                }
            }
        }

        for ( sal_uInt16 nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            // LayoutName and PageName must be the same
            SdPage* pPage = static_cast<SdPage*>( GetMasterPage( nPage ) );

            OUString aName( pPage->GetLayoutName() );
            aName = aName.copy( 0, aName.indexOf( SD_LT_SEPARATOR ) );

            if( aName != pPage->GetName() )
                pPage->SetName( aName );
        }

        // Create names of the styles in the user's language
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->UpdateStdNames();

        // Create any missing styles - eg. formerly, there was no Subtitle style
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->CreatePseudosIfNecessary();
    }

    // Set default style of Drawing Engine
    OUString aName( SdResId(STR_STANDARD_STYLESHEET_NAME));
    SetDefaultStyleSheet(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(aName, SfxStyleFamily::Para)));

    // #i119287# Set default StyleSheet for SdrGrafObj and SdrOle2Obj
    SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(SdResId(STR_POOLSHEET_OBJNOLINENOFILL), SfxStyleFamily::Para)));

    // Initialize DrawOutliner and DocumentOutliner, but don't initialize the
    // global outliner, as it is not document specific like StyleSheetPool and
    // StyleRequestHandler are.
    ::Outliner& rDrawOutliner = GetDrawOutliner();
    rDrawOutliner.SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    EEControlBits nCntrl = rDrawOutliner.GetControlWord();
    if (mbOnlineSpell)
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    // Initialize HitTestOutliner and DocumentOutliner, but don't initialize the
    // global outliner, as it is not document specific like StyleSheetPool and
    // StyleRequestHandler are.
    m_pHitTestOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));

    if(mpOutliner)
    {
        mpOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }
    if(mpInternalOutliner)
    {
        mpInternalOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    if ( eMode == DocCreationMode::Loaded )
    {
        // Make presentation objects listeners of the appropriate styles
        SdStyleSheetPool* pSPool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        sal_uInt16 nPage, nPageCount;

        // create missing layout style sheets for broken documents
        //         that were created with the 5.2
        nPageCount = GetMasterSdPageCount( PageKind::Standard );
        for (nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = GetMasterSdPage(nPage, PageKind::Standard);
            pSPool->CreateLayoutStyleSheets( pPage->GetName(), true );
        }

        // Default and notes pages:
        for (nPage = 0; nPage < GetPageCount(); nPage++)
        {
            SdPage* pPage = static_cast<SdPage*>(GetPage(nPage));
            NewOrLoadCompleted( pPage, pSPool );
        }

        // Master pages:
        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            SdPage* pPage = static_cast<SdPage*>(GetMasterPage(nPage));

            NewOrLoadCompleted( pPage, pSPool );
        }
    }

    mbNewOrLoadCompleted = true;
    UpdateAllLinks();
    SetChanged( false );
}

namespace
{
// Base for a sound link bound to a page through sdr::PageUser, so the link is
// removed when the page goes away. A derived class implements DataChanged to
// mark its own sound allowed.
class SdPageBoundSoundLink : public sfx2::SvBaseLink, public sdr::PageUser
{
protected:
    SdPage* m_pPage;

    explicit SdPageBoundSoundLink(SdPage& rPage)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SIMPLE_FILE)
        , m_pPage(&rPage)
    {
        m_pPage->AddPageUser(*this);
    }

    virtual ~SdPageBoundSoundLink() override
    {
        if (m_pPage)
            m_pPage->RemovePageUser(*this);
    }

public:
    const SdPage* getPage() const { return m_pPage; }

    virtual void PageInDestruction(const SdrPage&) override
    {
        // the page is tearing down and removes its own users, so just drop the back-pointer here
        m_pPage = nullptr;
        tools::SvRef<SvBaseLink> xSelf(this);
        if (sfx2::LinkManager* pLinkMgr = GetLinkManager())
            pLinkMgr->Remove(this);
    }
};

// The slide-transition sound link, bound to its page the way
// sdr::SdrPageFillBitmapLink binds a fill bitmap to its host. Updating the link
// marks the page's own SdSoundLink allowed, so the page carries that state
// directly.
class SdPageSoundLink final : public SdPageBoundSoundLink
{
public:
    explicit SdPageSoundLink(SdPage& rPage)
        : SdPageBoundSoundLink(rPage)
    {
    }

    virtual UpdateResult DataChanged(const OUString&, const css::uno::Any&) override
    {
        if (m_pPage)
            m_pPage->SetSoundAllowed(true);
        return SUCCESS;
    }
};

// The click-action sound link, bound to its shape the way SdPageSoundLink binds
// to its page. Updating the link marks the shape's SdAnimationInfo allowed, so
// the shape carries that state directly.
// Bound through sdr::ObjectUser so it never outlives the shape.
class SdShapeSoundLink final : public sfx2::SvBaseLink, public sdr::ObjectUser
{
    SdrObject* m_pObject;

public:
    explicit SdShapeSoundLink(SdrObject& rObject)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SIMPLE_FILE)
        , m_pObject(&rObject)
    {
        m_pObject->AddObjectUser(*this);
    }

    virtual ~SdShapeSoundLink() override
    {
        if (m_pObject)
            m_pObject->RemoveObjectUser(*this);
    }

    const SdrObject* getObject() const { return m_pObject; }

    virtual UpdateResult DataChanged(const OUString&, const css::uno::Any&) override
    {
        if (m_pObject)
        {
            if (SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*m_pObject))
                pInfo->mbClickSoundAllowed = true;
        }
        return SUCCESS;
    }

    virtual void ObjectInDestruction(const SdrObject&) override
    {
        // the shape is tearing down and removes its own users, so just drop the back-pointer here
        m_pObject = nullptr;
        tools::SvRef<SvBaseLink> xSelf(this);
        if (sfx2::LinkManager* pLinkMgr = GetLinkManager())
            pLinkMgr->Remove(this);
    }
};

// The animation-effect sound link. It holds the audio node whose source carries
// the sound, and is bound to the node's page like the transition sound link.
// Updating the link marks that source allowed, so the node carries the allowed
// state directly. The URL is kept; only the allowed state changes.
class SdAnimationSoundLink final : public SdPageBoundSoundLink
{
    css::uno::Reference<css::animations::XAudio> m_xAudio;

public:
    SdAnimationSoundLink(SdPage& rPage, css::uno::Reference<css::animations::XAudio> xAudio)
        : SdPageBoundSoundLink(rPage)
        , m_xAudio(std::move(xAudio))
    {
    }

    const css::uno::Reference<css::animations::XAudio>& getAudio() const { return m_xAudio; }

    virtual UpdateResult DataChanged(const OUString&, const css::uno::Any&) override
    {
        if (m_xAudio.is())
            m_xAudio->setSource(
                xmloff::makeSoundSource(xmloff::getSoundURL(m_xAudio->getSource()), true));
        return SUCCESS;
    }
};

// True when rLinkManager already holds a link of LinkType for which rMatch
// returns true, that is, the link's owner is already registered.
template <typename LinkType, typename Match>
bool lcl_hasSoundLink(const sfx2::LinkManager& rLinkManager, Match rMatch)
{
    for (const tools::SvRef<sfx2::SvBaseLink>& rLink : rLinkManager.GetLinks())
    {
        const LinkType* pLink = dynamic_cast<LinkType*>(rLink.get());
        if (pLink && rMatch(*pLink))
            return true;
    }
    return false;
}

// True when the document holds any external sound link: a transition, a
// click-action or an animation-effect sound.
bool lcl_hasAnySoundLink(const sfx2::LinkManager& rLinkManager)
{
    for (const tools::SvRef<sfx2::SvBaseLink>& rLink : rLinkManager.GetLinks())
    {
        sfx2::SvBaseLink* pLink = rLink.get();
        if (dynamic_cast<SdPageSoundLink*>(pLink) || dynamic_cast<SdShapeSoundLink*>(pLink)
            || dynamic_cast<SdAnimationSoundLink*>(pLink))
            return true;
    }
    return false;
}

// Register an animation-sound link for every AUDIO node under rNode whose source
// points outside the document package, binding each to rPage. Sets rbFound when
// rNode holds any such sound.
void lcl_registerAudioSoundLinks(
    const css::uno::Reference<css::animations::XAnimationNode>& rNode,
    SdPage& rPage, sfx2::LinkManager& rLinkManager, bool& rbFound)
{
    if (!rNode.is())
        return;

    if (rNode->getType() == css::animations::AnimationNodeType::AUDIO)
    {
        css::uno::Reference<css::animations::XAudio> xAudio(rNode, css::uno::UNO_QUERY);
        const css::uno::Any aSource = xAudio.is() ? xAudio->getSource() : css::uno::Any();
        const OUString aURL = xmloff::getSoundURL(aSource);
        if (SdSoundLink(aURL).isExternalLink() && !xmloff::getSoundAllowed(aSource))
        {
            rbFound = true;
            if (!lcl_hasSoundLink<SdAnimationSoundLink>(
                    rLinkManager, [&xAudio](const SdAnimationSoundLink& rLink)
                    { return rLink.getAudio() == xAudio; }))
            {
                tools::SvRef<sfx2::SvBaseLink> xLink(new SdAnimationSoundLink(rPage, xAudio));
                rLinkManager.InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientFile, aURL);
            }
        }
    }

    css::uno::Reference<css::container::XEnumerationAccess> xEnumAccess(rNode, css::uno::UNO_QUERY);
    if (xEnumAccess.is())
    {
        css::uno::Reference<css::container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
        while (xEnum.is() && xEnum->hasMoreElements())
        {
            css::uno::Reference<css::animations::XAnimationNode> xChild(xEnum->nextElement(),
                                                                       css::uno::UNO_QUERY);
            lcl_registerAudioSoundLinks(xChild, rPage, rLinkManager, rbFound);
        }
    }
}
}

void SdDrawDocument::RegisterPageSoundLink(SdPage& rPage)
{
    // No link manager on a shell-less model (for example the clipboard), which
    // has no infobar or slideshow, so nothing needs registering there. An
    // already-allowed sound is the document's own content, not a link.
    if (!m_pLinkManager || !rPage.GetSoundLink().isExternalLink()
        || rPage.GetSoundLink().isAllowed())
        return;
    if (lcl_hasSoundLink<SdPageSoundLink>(*m_pLinkManager, [&rPage](const SdPageSoundLink& rLink)
                                          { return rLink.getPage() == &rPage; }))
        return; // already registered
    tools::SvRef<sfx2::SvBaseLink> xLink(new SdPageSoundLink(rPage));
    m_pLinkManager->InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientFile,
                                   rPage.GetSoundFile());
}

bool SdDrawDocument::RegisterAnimationSoundLinks(SdPage& rPage)
{
    if (!m_pLinkManager || !rPage.hasAnimationNode())
        return false;
    bool bFound = false;
    lcl_registerAudioSoundLinks(rPage.getAnimationNode(), rPage, *m_pLinkManager, bFound);
    return bFound;
}

void SdDrawDocument::RegisterShapeSoundLink(SdrObject& rObject)
{
    if (!m_pLinkManager)
        return;
    SdAnimationInfo* pInfo = GetShapeUserData(rObject);
    if (!pInfo || pInfo->meClickAction != css::presentation::ClickAction_SOUND
        || !SdSoundLink(pInfo->GetBookmark()).isExternalLink()
        || pInfo->mbClickSoundAllowed)
        return;
    if (lcl_hasSoundLink<SdShapeSoundLink>(*m_pLinkManager, [&rObject](const SdShapeSoundLink& rLink)
                                           { return rLink.getObject() == &rObject; }))
        return; // already registered
    tools::SvRef<sfx2::SvBaseLink> xLink(new SdShapeSoundLink(rObject));
    m_pLinkManager->InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientFile,
                                   pInfo->GetBookmark());
}


/** updates all links, only links in this document should by resolved */
void SdDrawDocument::UpdateAllLinks()
{
    if (s_pDocLockedInsertingLinks || !m_pLinkManager)
        return;

    // Draw-layer fill bitmaps (shapes and slide/page backgrounds) are tracked
    // per host by the model's sdr::FillBitmapLinkTracker as they are set.

    // Register links for form controls with deferred remote ImageURL
    ::sd::DrawDocShell* pDocShell = GetDocSh();
    if (pDocShell)
    {
        registerDeferredFormImageLinks(pDocShell->GetDeferredFormControlImages(), *m_pLinkManager);
        pDocShell->ClearDeferredFormControlImages();
    }

    // Transition, animation-effect and click-action sounds that point outside
    // the document package are links. Each is bound to what owns its URL - the
    // transition sound to its page, the click-action sound to its shape, the
    // animation-effect sound to its audio node - so allowing one marks that
    // owner allowed. The transition and click-action sounds are registered as
    // their URL is set. The animation-effect sound's source is set on its audio
    // node by the ODF import, with no set-time hook here; the import reports one
    // by setting the document's HasExternalAnimationSoundLink property, and only
    // then is the animation tree of each slide walked to register it. Once any
    // external sound link is present, warn through the link-update infobar until
    // the document is allowed to update its links.
    if (pDocShell)
    {
        if (mbMaybeHasAnimationSoundLinks)
        {
            for (sal_uInt16 nPage = 0, nCount = GetSdPageCount(PageKind::Standard); nPage < nCount;
                 ++nPage)
            {
                SdPage* pPage = GetSdPage(nPage, PageKind::Standard);
                if (pPage)
                    RegisterAnimationSoundLinks(*pPage);
            }
        }
        if (!pDocShell->getEmbeddedObjectContainer().getUserAllowsLinkUpdate()
            && lcl_hasAnySoundLink(*m_pLinkManager))
        {
            pDocShell->SetPendingLinkUpdateInfobar();
        }
    }

    if (m_pLinkManager->GetLinks().empty())
        return;

    if (!pDocShell)
        return;

    comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer
        = pDocShell->getEmbeddedObjectContainer();

    if (rEmbeddedObjectContainer.getUserAllowsLinkUpdate())
    {
        s_pDocLockedInsertingLinks = this;
        m_pLinkManager->UpdateAllLinks(false, nullptr, u""_ustr);
        if (s_pDocLockedInsertingLinks == this)
            s_pDocLockedInsertingLinks = nullptr;
        return;
    }

    SfxMedium* pMedium = pDocShell->GetMedium();
    const SfxUInt16Item* pUpdateDocItem = pMedium
        ? pMedium->GetItemSet().GetItem(SID_UPDATEDOCMODE, false)
        : nullptr;
    sal_uInt16 nUpdateDocMode = pUpdateDocItem
        ? pUpdateDocItem->GetValue()
        : css::document::UpdateDocMode::NO_UPDATE;

    if (nUpdateDocMode == css::document::UpdateDocMode::NO_UPDATE)
        return;

    bool bAskUpdate = true;
    switch (nUpdateDocMode)
    {
        case css::document::UpdateDocMode::QUIET_UPDATE:
            bAskUpdate = false;
            break;
        case css::document::UpdateDocMode::FULL_UPDATE:
            bAskUpdate = false;
            break;
        case css::document::UpdateDocMode::ACCORDING_TO_CONFIG:
            bAskUpdate = !pMedium
                || !SvtSecurityOptions::isTrustedLocationUriForUpdatingLinks(
                       pMedium->GetName());
            break;
    }

    if (bAskUpdate)
    {
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(false);
        pDocShell->SetPendingLinkUpdateInfobar();
    }
    else
    {
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);
        s_pDocLockedInsertingLinks = this;
        m_pLinkManager->UpdateAllLinks(false, nullptr, pMedium->GetName());
        if (s_pDocLockedInsertingLinks == this)
            s_pDocLockedInsertingLinks = nullptr;
    }
}

/** this loops over the presentation objects of a page and repairs some new settings
    from old binary files and resets all default strings for empty presentation objects.
*/
void SdDrawDocument::NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool )
{
    sd::ShapeList& rPresentationShapes( pPage->GetPresentationShapeList() );
    if(rPresentationShapes.isEmpty())
        return;

    // Create lists of title and outline styles
    OUString aName = pPage->GetLayoutName();
    aName = aName.copy( 0, aName.indexOf( SD_LT_SEPARATOR ) );

    std::vector<SfxStyleSheetBase*> aOutlineList;
    pSPool->CreateOutlineSheetList(aName,aOutlineList);

    SfxStyleSheet* pTitleSheet = static_cast<SfxStyleSheet*>(pSPool->GetTitleSheet(aName));

    SdrObject* pObj = nullptr;
    rPresentationShapes.seekShape(0);

    // Now look for title and outline text objects, then make those objects
    // listeners.
    while( (pObj = rPresentationShapes.getNextShape()) )
    {
        if (pObj->GetObjInventor() == SdrInventor::Default)
        {
            OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
            SdrObjKind nId = pObj->GetObjIdentifier();

            if (nId == SdrObjKind::TitleText)
            {
                if( pOPO && pOPO->GetOutlinerMode() == OutlinerMode::DontKnow )
                    pOPO->SetOutlinerMode( OutlinerMode::TitleObject );

                // sal_True: don't delete "hard" attributes when doing this.
                if (pTitleSheet)
                    pObj->SetStyleSheet(pTitleSheet, true);
            }
            else if (nId == SdrObjKind::OutlineText)
            {
                if( pOPO && pOPO->GetOutlinerMode() == OutlinerMode::DontKnow )
                    pOPO->SetOutlinerMode( OutlinerMode::OutlineObject );

                std::vector<SfxStyleSheetBase*>::iterator iter;
                for (iter = aOutlineList.begin(); iter != aOutlineList.end(); ++iter)
                {
                    SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(*iter);

                    if (pSheet)
                    {
                        pObj->StartListening(*pSheet);

                        if( iter == aOutlineList.begin())
                            // text frame listens to stylesheet of layer 1
                            pObj->NbcSetStyleSheet(pSheet, true);
                    }
                }
            }

            if( auto pTextObj = DynCastSdrTextObj( pObj ) )
                if (pTextObj->IsEmptyPresObj())
                {
                    PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                    OUString aString( pPage->GetPresObjText(ePresObjKind) );

                    if (!aString.isEmpty())
                    {
                        SdOutliner* pInternalOutl = GetInternalOutliner();
                        pPage->SetObjText( pTextObj, pInternalOutl, ePresObjKind, aString );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), true );
                        pInternalOutl->Clear();
                    }
                }
        }
    }
}

// Local outliner that is used for outline mode. In this outliner, OutlinerViews
// may be inserted.
SdOutliner* SdDrawDocument::GetOutliner(bool bCreateOutliner)
{
    if (!mpOutliner && bCreateOutliner)
    {
        mpOutliner.reset(new SdOutliner( *this, OutlinerMode::TextObject ));

        if (mpDocSh)
            mpOutliner->SetRefDevice(SdModule::get()->GetVirtualRefDevice());

        mpOutliner->SetDefTab( m_nDefaultTabulator );
        mpOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    return mpOutliner.get();
}

// Internal outliner that is used to create text objects. We don't insert any
// OutlinerViews into this outliner!
SdOutliner* SdDrawDocument::GetInternalOutliner(bool bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner.reset( new SdOutliner( *this, OutlinerMode::TextObject ) );

        // This outliner is only used to create special text objects. As no
        // information about portions is saved in this outliner, the update mode
        // can/should always remain sal_False.
        mpInternalOutliner->SetUpdateLayout( false );
        mpInternalOutliner->EnableUndo( false );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice(SdModule::get()->GetVirtualRefDevice());

        mpInternalOutliner->SetDefTab( m_nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    DBG_ASSERT( !mpInternalOutliner || ( ! mpInternalOutliner->IsUpdateLayout() ) , "InternalOutliner: UpdateMode = sal_True !" );
    DBG_ASSERT( !mpInternalOutliner || ( ! mpInternalOutliner->IsUndoEnabled() ), "InternalOutliner: Undo = sal_True !" );

    // If you add stuff here, always clear it out.
    // Advantages:
    // a) no unnecessary Clear calls
    // b) no wasted memory
    DBG_ASSERT( !mpInternalOutliner || ( ( mpInternalOutliner->GetParagraphCount() == 1 ) && ( mpInternalOutliner->GetText( mpInternalOutliner->GetParagraph( 0 ) ).isEmpty() ) ), "InternalOutliner: not empty!" );

    return mpInternalOutliner.get();
}

// OnlineSpelling on/off
void SdDrawDocument::SetOnlineSpell(bool bIn)
{
    mbOnlineSpell = bIn;
    EEControlBits nCntrl;

    if(mpOutliner)
    {
        nCntrl = mpOutliner->GetControlWord();

        if(mbOnlineSpell)
            nCntrl |= EEControlBits::ONLINESPELLING;
        else
            nCntrl &= ~EEControlBits::ONLINESPELLING;

        mpOutliner->SetControlWord(nCntrl);
    }

    if (mpInternalOutliner)
    {
        nCntrl = mpInternalOutliner->GetControlWord();

        if (mbOnlineSpell)
            nCntrl |= EEControlBits::ONLINESPELLING;
        else
            nCntrl &= ~EEControlBits::ONLINESPELLING;

        mpInternalOutliner->SetControlWord(nCntrl);
    }

    ::Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (mbOnlineSpell)
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;

    rOutliner.SetControlWord(nCntrl);

    if (mbOnlineSpell)
    {
        StartOnlineSpelling();
    }
    else
    {
        StopOnlineSpelling();
    }
}

// OnlineSpelling: highlighting on/off
uno::Reference< frame::XModel > SdDrawDocument::createUnoModel()
{
    uno::Reference< frame::XModel > xModel;

    try
    {
        if ( mpDocSh )
            xModel = mpDocSh->GetModel();
    }
    catch( uno::RuntimeException& )
    {
    }

    return xModel;
}

SvxNumType SdDrawDocument::GetPageNumType() const
{
    return mePageNumType;
}

void SdDrawDocument::SetPrinterIndependentLayout (sal_Int32 nMode)
{
    switch (nMode)
    {
        case css::document::PrinterIndependentLayout::DISABLED:
        case css::document::PrinterIndependentLayout::ENABLED:
            // Just store supported modes and inform the doc shell
            mnPrinterIndependentLayout = nMode;

            // Since it is possible that a SdDrawDocument is constructed without a
            // SdDrawDocShell the pointer member mpDocSh needs to be tested
            // before the call is executed. This is e. g. used for copy/paste.
            if(mpDocSh)
            {
                mpDocSh->UpdateRefDevice ();
            }

            break;

        default:
            // Ignore unknown values
            break;
    }
}

void SdDrawDocument::SetStartWithPresentation(sal_uInt16 nStartingSlide)
{
    mnStartWithPresentation = nStartingSlide;
}

void SdDrawDocument::SetExitAfterPresenting( bool bExitAfterPresenting )
{
    mbExitAfterPresenting = bExitAfterPresenting;
}

void SdDrawDocument::PageListChanged()
{
    mpDrawPageListWatcher->Invalidate();
}

void SdDrawDocument::MasterPageListChanged()
{
    mpMasterPageListWatcher->Invalidate();
}

void SdDrawDocument::SetCalcFieldValueHdl(::Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(SdModule::get(), SdModule, CalcFieldValueHdl));
}

sal_uInt16 SdDrawDocument::GetAnnotationAuthorIndex( const OUString& rAuthor )
{
    // force current user to have first color
    if( maAnnotationAuthors.empty() )
    {
        SvtUserOptions aUserOptions;
        maAnnotationAuthors.push_back( aUserOptions.GetFullName() );
    }

    auto iter = std::find(maAnnotationAuthors.begin(), maAnnotationAuthors.end(), rAuthor);
    sal_uInt16 idx = static_cast<sal_uInt16>(std::distance(maAnnotationAuthors.begin(), iter));

    if( idx == maAnnotationAuthors.size() )
    {
        maAnnotationAuthors.push_back( rAuthor );
    }

    return idx;
}

void SdDrawDocument::InitLayoutVector()
{
    if (comphelper::IsFuzzing())
        return;

    const Reference<css::uno::XComponentContext>& xContext(
        ::comphelper::getProcessComponentContext() );

    // get file list from configuration
    const Sequence< OUString > aFiles(
        officecfg::Office::Impress::Misc::LayoutListFiles::get() );

    if (aFiles.getLength() == 0)
        return;
    const Reference<XDocumentBuilder> xDocBuilder = DocumentBuilder::create( xContext );

    for( const auto& rFile : aFiles )
    {
        OUString sFilename = comphelper::getExpandedUri(xContext, rFile);

        // load layout file into DOM

        try
        {
            // loop over every layout entry in current file
            const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
            const Reference<XNodeList> layoutlist = xDoc->getElementsByTagName(u"layout"_ustr);
            const int nElements = layoutlist->getLength();
            for(int index=0; index < nElements; index++)
                maLayoutInfo.push_back( layoutlist->item(index) );
        }
        catch (const uno::Exception &)
        {
            // skip missing config. files
        }
    }
}

void SdDrawDocument::InitObjectVector()
{
    if (comphelper::IsFuzzing())
        return;

    const Reference<css::uno::XComponentContext>& xContext(
        ::comphelper::getProcessComponentContext() );

    // get file list from configuration
    const Sequence< OUString > aFiles(
       officecfg::Office::Impress::Misc::PresObjListFiles::get() );

    if (aFiles.getLength() == 0)
        return;
    const Reference<XDocumentBuilder> xDocBuilder = DocumentBuilder::create( xContext );
    for( const auto& rFile : aFiles )
    {
        OUString sFilename = comphelper::getExpandedUri(xContext, rFile);

        // load presentation object file into DOM

        try
        {
            // loop over every object entry in current file
            const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
            const Reference<XNodeList> objectlist = xDoc->getElementsByTagName(u"object"_ustr);
            const int nElements = objectlist->getLength();
            for(int index=0; index < nElements; index++)
                maPresObjectInfo.push_back( objectlist->item(index) );
        }
        catch (const uno::Exception &)
        {
            // skip missing config. files
        }
    }
}

void SdDrawDocument::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("model.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        (void)xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdDrawDocument"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    if (mpOutliner)
        mpOutliner->dumpAsXml(pWriter);
    FmFormModel::dumpAsXml(pWriter);
    if (GetUndoManager())
        GetUndoManager()->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
    if (bOwns)
    {
        (void)xmlTextWriterEndDocument(pWriter);
        xmlFreeTextWriter(pWriter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
