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
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/forbiddencharacterstable.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/scriptspaceitem.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/useroptions.hxx>
#include <officecfg/Office/Impress.hxx>

#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/dialogs.hrc>
#include "Outliner.hxx"
#include "sdmod.hxx"
#include <editeng/editstat.hxx>
#include <editeng/fontitem.hxx>
#include <svl/flagitem.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdotext.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/numitem.hxx>
#include <svx/svditer.hxx>
#include <editeng/unolingu.hxx>
#include <svl/itempool.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svx/xtable.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <editeng/outlobj.hxx>
#include <unotools/saveopt.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>

#include <editeng/outliner.hxx>
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "sdattr.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "sdresid.hxx"
#include "cusshow.hxx"
#include "customshowlist.hxx"
#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/GraphicDocShell.hxx"
#include "../ui/inc/sdxfer.hxx"
#include "../ui/inc/ViewShell.hxx"
#include "../ui/inc/optsitem.hxx"
#include "../ui/inc/FrameView.hxx"

#include <tools/tenccvt.hxx>
#include <vcl/settings.hxx>
#include <comphelper/lok.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

using namespace com::sun::star::xml::dom;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::beans::PropertyValue;

TYPEINIT1( SdDrawDocument, FmFormModel );

SdDrawDocument* SdDrawDocument::pDocLockedInsertingLinks = NULL;

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
    mnPauseTimeout( 10 ),
    mbShowPauseLogo( false ),
    mbStartWithNavigator(false)
{
}

PresentationSettings::PresentationSettings( const PresentationSettings& r )
:   maPresPage( r.maPresPage ),
    mbAll( r.mbAll ),
    mbEndless( r.mbEndless ),
    mbCustomShow( r.mbCustomShow ),
    mbManual( r.mbManual ),
    mbMouseVisible( r.mbMouseVisible ),
    mbMouseAsPen( r.mbMouseAsPen ),
    mbLockedPages( r.mbLockedPages ),
    mbAlwaysOnTop( r.mbAlwaysOnTop ),
    mbFullScreen( r.mbFullScreen ),
    mbAnimationAllowed( r.mbAnimationAllowed ),
    mnPauseTimeout( r.mnPauseTimeout ),
    mbShowPauseLogo( r.mbShowPauseLogo ),
    mbStartWithNavigator( r.mbStartWithNavigator )
{
}

SdDrawDocument::SdDrawDocument(DocumentType eType, SfxObjectShell* pDrDocSh)
: FmFormModel( !utl::ConfigManager::IsAvoidConfig() ? SvtPathOptions().GetPalettePath() : OUString(), NULL, pDrDocSh )
, bReadOnly(false)
, mpOutliner(NULL)
, mpInternalOutliner(NULL)
, mpWorkStartupTimer(NULL)
, mpOnlineSpellingIdle(NULL)
, mpOnlineSpellingList(NULL)
, mpOnlineSearchItem(NULL)
, mpCustomShowList(NULL)
, mpDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh))
, mpCreatingTransferable( NULL )
, mbHasOnlineSpellErrors(false)
, mbInitialOnlineSpellingEnabled(true)
, mbNewOrLoadCompleted(false)
, mbOnlineSpell(false)
, mbStartWithPresentation( false )
, mbExitAfterPresenting( false )
, meLanguage( LANGUAGE_SYSTEM )
, meLanguageCJK( LANGUAGE_SYSTEM )
, meLanguageCTL( LANGUAGE_SYSTEM )
, mePageNumType(SVX_ARABIC)
, mbAllocDocSh(false)
, meDocType(eType)
, mpCharClass(NULL)
, mpLocale(NULL)
, mbUseEmbedFonts(false)
{
    mpDrawPageListWatcher.reset(new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher.reset(new ImpMasterPageListWatcher(*this));

    InitLayoutVector();
    InitObjectVector();
    SetObjectShell(pDrDocSh);       // for VCDrawModel

    if (mpDocSh)
    {
        SetSwapGraphics();
    }

    // Set measuring unit (of the application) and scale (of SdMod)
    sal_Int32 nX, nY;
    SdOptions* pOptions = SD_MOD()->GetSdOptions(meDocType);
    pOptions->GetScale( nX, nY );

    // Allow UI scale only for draw documents.
    if( eType == DOCUMENT_TYPE_DRAW )
        SetUIUnit( (FieldUnit)pOptions->GetMetric(), Fraction( nX, nY ) );  // user-defined
    else
        SetUIUnit( (FieldUnit)pOptions->GetMetric(), Fraction( 1, 1 ) );    // default

    SetScaleUnit(MAP_100TH_MM);
    SetScaleFraction(Fraction(1, 1));
    SetDefaultFontHeight(847);     // 24p

    pItemPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pItemPool->FreezeIdRanges();
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
    if (!utl::ConfigManager::IsAvoidConfig())
    {
        const SvtLinguConfig    aLinguConfig;
        SvtLinguOptions         aOptions;
        aLinguConfig.GetOptions( aOptions );

        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage,
            ::com::sun::star::i18n::ScriptType::LATIN), EE_CHAR_LANGUAGE );
        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CJK,
            ::com::sun::star::i18n::ScriptType::ASIAN), EE_CHAR_LANGUAGE_CJK );
        SetLanguage( MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CTL,
            ::com::sun::star::i18n::ScriptType::COMPLEX), EE_CHAR_LANGUAGE_CTL );

        if (!comphelper::LibreOfficeKit::isActive())
            mbOnlineSpell = aOptions.bIsSpellAuto;
    }

    LanguageType eRealLanguage = MsLangId::getRealLanguage( meLanguage );
    LanguageTag aLanguageTag( eRealLanguage);
    mpLocale = new ::com::sun::star::lang::Locale( aLanguageTag.getLocale());
    mpCharClass = new CharClass( aLanguageTag );

    // If the current application language is a language that uses right-to-left text...
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    if( MsLangId::isRightToLeft( eRealCTLLanguage ) )
    {
        // ... then we have to set this as a default
        SetDefaultWritingMode( ::com::sun::star::text::WritingMode_RL_TB );
    }

    // for korean and japanese languages we have a different default for apply spacing between asian, latin and ctl text
    if (MsLangId::isKorean(eRealCTLLanguage) || (LANGUAGE_JAPANESE == eRealCTLLanguage))
    {
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
    }

    // Set DefTab and SpellOptions for the SD module
    sal_uInt16 nDefTab = pOptions->GetDefTab();
    SetDefaultTabulator( nDefTab );

    try
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            rOutliner.SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            rOutliner.SetHyphenator( xHyphenator );

        SetForbiddenCharsTable( new SvxForbiddenCharactersTable( ::comphelper::getProcessComponentContext() ) );
    }
    catch(...)
    {
        OSL_FAIL("Can't get SpellChecker");
    }

    rOutliner.SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    if (mpDocSh)
    {
        SetLinkManager( new sfx2::LinkManager(mpDocSh) );
    }

    EEControlBits nCntrl = rOutliner.GetControlWord();
    nCntrl |= EEControlBits::ALLOWBIGOBJS;

    if (mbOnlineSpell)
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;

    nCntrl &= ~ EEControlBits::ULSPACESUMMATION;
    if ( meDocType != DOCUMENT_TYPE_IMPRESS )
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
    SfxItemSet aSet2( pHitTestOutliner->GetEmptyItemSet() );
    pHitTestOutliner->SetStyleSheetPool( static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()) );

    SetCalcFieldValueHdl( pHitTestOutliner );

    try
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            pHitTestOutliner->SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            pHitTestOutliner->SetHyphenator( xHyphenator );
    }
    catch(...)
    {
        OSL_FAIL("Can't get SpellChecker");
    }

    pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    EEControlBits nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EEControlBits::ALLOWBIGOBJS;
    nCntrl2 &= ~EEControlBits::ONLINESPELLING;

    nCntrl2 &= ~ EEControlBits::ULSPACESUMMATION;
    if ( pOptions->IsSummationOfParagraphs() )
        nCntrl2 |= EEControlBits::ULSPACESUMMATION;

    pHitTestOutliner->SetControlWord( nCntrl2 );

    /** Create layers
      *
      * We create the following default layers on all pages and master pages:
      *
      * STR_LAYOUT    : default layer for drawing objects
      *
      * STR_BCKGRND   : background of the master page
      *                 (currently unused within normal pages)
      *
      * STR_BCKGRNDOBJ: objects on the background of master pages
      *                 (currently unused within normal pages)
      *
      * STR_CONTROLS  : default layer for controls
      */

    {
        OUString aControlLayerName( SD_RESSTR(STR_LAYER_CONTROLS) );

        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        rLayerAdmin.NewLayer( SD_RESSTR(STR_LAYER_LAYOUT) );
        rLayerAdmin.NewLayer( SD_RESSTR(STR_LAYER_BCKGRND) );
        rLayerAdmin.NewLayer( SD_RESSTR(STR_LAYER_BCKGRNDOBJ) );
        rLayerAdmin.NewLayer( aControlLayerName );
        rLayerAdmin.NewLayer( SD_RESSTR(STR_LAYER_MEASURELINES) );

        rLayerAdmin.SetControlLayerName(aControlLayerName);
    }

}

// Destructor
SdDrawDocument::~SdDrawDocument()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    if (mpWorkStartupTimer)
    {
        if ( mpWorkStartupTimer->IsActive() )
            mpWorkStartupTimer->Stop();

        delete mpWorkStartupTimer;
        mpWorkStartupTimer = NULL;
    }

    StopOnlineSpelling();
    delete mpOnlineSearchItem;
    mpOnlineSearchItem = NULL;

    CloseBookmarkDoc();
    SetAllocDocSh(false);

    ClearModel(true);

    if (pLinkManager)
    {
        // Release BaseLinks
        if ( !pLinkManager->GetLinks().empty() )
        {
            pLinkManager->Remove( 0, pLinkManager->GetLinks().size() );
        }

        delete pLinkManager;
        pLinkManager = NULL;
    }

    std::vector<sd::FrameView*>::iterator pIter;
    for ( pIter = maFrameViewList.begin(); pIter != maFrameViewList.end(); ++pIter )
        delete *pIter;

    if (mpCustomShowList)
    {
        for (sal_uLong j = 0; j < mpCustomShowList->size(); j++)
        {
            // If necessary, delete CustomShows
            SdCustomShow* pCustomShow = (*mpCustomShowList)[j];
            delete pCustomShow;
        }

        delete mpCustomShowList;
        mpCustomShowList = NULL;
    }

    delete mpOutliner;
    mpOutliner = NULL;

    delete mpInternalOutliner;
    mpInternalOutliner = NULL;

    delete mpLocale;
    mpLocale = NULL;

    delete mpCharClass;
    mpCharClass = NULL;
}

SdrModel* SdDrawDocument::AllocModel() const
{
    return AllocSdDrawDocument();
}

// This method creates a new document (SdDrawDocument) and returns a pointer to
// said document. The drawing engine uses this method to put the document (or
// parts of it) into the clipboard/DragServer.
SdDrawDocument* SdDrawDocument::AllocSdDrawDocument() const
{
    SdDrawDocument* pNewModel = NULL;

    if( mpCreatingTransferable )
    {
        // Document is created for drag & drop/clipboard. To be able to
        // do this, the document has to know a DocShell (SvPersist).
        SfxObjectShell*   pObj = NULL;
        ::sd::DrawDocShell*     pNewDocSh = NULL;

        if( meDocType == DOCUMENT_TYPE_IMPRESS )
            mpCreatingTransferable->SetDocShell( new ::sd::DrawDocShell(
                SfxObjectCreateMode::EMBEDDED, true, meDocType ) );
        else
            mpCreatingTransferable->SetDocShell( new ::sd::GraphicDocShell(
                SfxObjectCreateMode::EMBEDDED, true, meDocType ) );

        pNewDocSh = static_cast< ::sd::DrawDocShell*>( pObj = mpCreatingTransferable->GetDocShell() );
        pNewDocSh->DoInitNew( NULL );
        pNewModel = pNewDocSh->GetDoc();

        // Only necessary for clipboard -
        // for drag & drop this is handled by DragServer
        SdStyleSheetPool* pOldStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        SdStyleSheetPool* pNewStylePool = static_cast<SdStyleSheetPool*>( pNewModel->GetStyleSheetPool() );

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);
        pNewStylePool->CopyCellSheets(*pOldStylePool);
        pNewStylePool->CopyTableStyles(*pOldStylePool);

        for (sal_uInt16 i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            // Move with all of the master page's layouts
            OUString aOldLayoutName(const_cast<SdDrawDocument*>(this)->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aOldLayoutName = aOldLayoutName.copy( 0, aOldLayoutName.indexOf( SD_LT_SEPARATOR ) );
            SdStyleSheetVector aCreatedSheets;
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool, aCreatedSheets );
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  // loaded from source document
    }
    else if( mbAllocDocSh )
    {
        // Create a DocShell which is then returned with GetAllocedDocSh()
        SdDrawDocument* pDoc = const_cast<SdDrawDocument*>(this);
        pDoc->SetAllocDocSh(false);
        pDoc->mxAllocedDocShRef = new ::sd::DrawDocShell(
            SfxObjectCreateMode::EMBEDDED, true, meDocType);
        pDoc->mxAllocedDocShRef->DoInitNew(NULL);
        pNewModel = pDoc->mxAllocedDocShRef->GetDoc();
    }
    else
    {
        pNewModel = new SdDrawDocument(meDocType, NULL);
    }

    return pNewModel;
}

SdPage* SdDrawDocument::AllocSdPage(bool bMasterPage)
{
    return new SdPage(*this, bMasterPage);
}

// This method creates a new page (SdPage) and returns a pointer to said page.
// The drawing engine uses this method to create pages (whose types it does
// not know, as they are _derivatives_ of SdrPage) when loading.
SdrPage* SdDrawDocument::AllocPage(bool bMasterPage)
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
    if (eMode == NEW_DOC)
    {
        // New document:
        // create slideshow and default templates,
        // create pool for virtual controls
        CreateLayoutTemplates();
        CreateDefaultCellStyles();

        static_cast< SdStyleSheetPool* >( mxStyleSheetPool.get() )->CreatePseudosIfNecessary();
    }
    else if (eMode == DOC_LOADED)
    {
            // Document has finished loading

        CheckMasterPages();

        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveUnnecessaryMasterPages( NULL, true, false );

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

        // Create names of the default layers in the user's language
        RestoreLayerNames();

        // Create names of the styles in the user's language
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->UpdateStdNames();

        // Create any missing styles - eg. formerly, there was no Subtitle style
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->CreatePseudosIfNecessary();
    }

    // Set default style of Drawing Engine
    OUString aName( SD_RESSTR(STR_STANDARD_STYLESHEET_NAME));
    SetDefaultStyleSheet(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_GRAPHICS)));

    // #i119287# Set default StyleSheet for SdrGrafObj and SdrOle2Obj
    SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(SD_RESSTR(STR_POOLSHEET_OBJNOLINENOFILL), SD_STYLE_FAMILY_GRAPHICS)));

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
    pHitTestOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));

    if(mpOutliner)
    {
        mpOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }
    if(mpInternalOutliner)
    {
        mpInternalOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    if ( eMode == DOC_LOADED )
    {
        // Make presentation objects listeners of the appropriate styles
        SdStyleSheetPool* pSPool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        sal_uInt16 nPage, nPageCount;

        // create missing layout style sheets for broken documents
        //         that where created with the 5.2
        nPageCount = GetMasterSdPageCount( PK_STANDARD );
        for (nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = GetMasterSdPage(nPage, PK_STANDARD);
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

    // Update all linked pages
    sal_uInt16 nMaxSdPages = GetSdPageCount(PK_STANDARD);

    for (sal_uInt16 nSdPage=0; nSdPage < nMaxSdPages; nSdPage++)
    {
        SdPage* pPage = GetSdPage(nSdPage, PK_STANDARD);

        if (pPage && !pPage->GetFileName().isEmpty() && pPage->GetBookmarkName().getLength())
        {
            pPage->SetModel(this);
        }
    }

    UpdateAllLinks();

    SetChanged( false );
}

/** updates all links, only links in this document should by resolved */
void SdDrawDocument::UpdateAllLinks()
{
    if ( !pDocLockedInsertingLinks && pLinkManager && !pLinkManager->GetLinks().empty() )
    {
        pDocLockedInsertingLinks = this; // lock inserting links. only links in this document should by resolved

        pLinkManager->UpdateAllLinks();  // query box: update all links?

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;  // unlock inserting links
    }
}

/** this loops over the presentation objects of a page and repairs some new settings
    from old binary files and resets all default strings for empty presentation objects.
*/
void SdDrawDocument::NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool )
{
    sd::ShapeList& rPresentationShapes( pPage->GetPresentationShapeList() );
    if(!rPresentationShapes.isEmpty())
    {
        // Create lists of title and outline styles
        OUString aName = pPage->GetLayoutName();
        aName = aName.copy( 0, aName.indexOf( SD_LT_SEPARATOR ) );

        std::vector<SfxStyleSheetBase*> aOutlineList;
        pSPool->CreateOutlineSheetList(aName,aOutlineList);

        SfxStyleSheet* pTitleSheet = static_cast<SfxStyleSheet*>(pSPool->GetTitleSheet(aName));

        SdrObject* pObj = 0;
        rPresentationShapes.seekShape(0);

        // Now look for title and outline text objects, then make those objects
        // listeners.
        while( (pObj = rPresentationShapes.getNextShape()) )
        {
            if (pObj->GetObjInventor() == SdrInventor)
            {
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                sal_uInt16 nId = pObj->GetObjIdentifier();

                if (nId == OBJ_TITLETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );

                    // sal_True: don't delete "hard" attributes when doing this.
                    if (pTitleSheet)
                        pObj->SetStyleSheet(pTitleSheet, true);
                }
                else if (nId == OBJ_OUTLINETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );

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

                if( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr && pObj->IsEmptyPresObj())
                {
                    PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                    OUString aString( pPage->GetPresObjText(ePresObjKind) );

                    if (!aString.isEmpty())
                    {
                        sd::Outliner* pInternalOutl = GetInternalOutliner();
                        pPage->SetObjText( static_cast<SdrTextObj*>(pObj), pInternalOutl, ePresObjKind, aString );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), true );
                        pInternalOutl->Clear();
                    }
                }
            }
        }
    }
}

// Local outliner that is used for outline mode. In this outliner, OutlinerViews
// may be inserted.
::sd::Outliner* SdDrawDocument::GetOutliner(bool bCreateOutliner)
{
    if (!mpOutliner && bCreateOutliner)
    {
        mpOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        if (mpDocSh)
            mpOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpOutliner->SetDefTab( nDefaultTabulator );
        mpOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    return mpOutliner;
}

// Internal outliner that is used to create text objects. We don't insert any
// OutlinerViews into this outliner!
::sd::Outliner* SdDrawDocument::GetInternalOutliner(bool bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        // This outliner is only used to create special text objects. As no
        // information about portions is saved in this outliner, the update mode
        // can/should always remain sal_False.
        mpInternalOutliner->SetUpdateMode( false );
        mpInternalOutliner->EnableUndo( false );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpInternalOutliner->SetDefTab( nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(GetStyleSheetPool()));
    }

    DBG_ASSERT( !mpInternalOutliner || ( ! mpInternalOutliner->GetUpdateMode() ) , "InternalOutliner: UpdateMode = sal_True !" );
    DBG_ASSERT( !mpInternalOutliner || ( ! mpInternalOutliner->IsUndoEnabled() ), "InternalOutliner: Undo = sal_True !" );

    // If you add stuff here, always clear it out.
    // Advantages:
    // a) no unnecessary Clear calls
    // b) no wasted memory
    DBG_ASSERT( !mpInternalOutliner || ( ( mpInternalOutliner->GetParagraphCount() == 1 ) && ( mpInternalOutliner->GetText( mpInternalOutliner->GetParagraph( 0 ) ).isEmpty() ) ), "InternalOutliner: not empty!" );

    return mpInternalOutliner;
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
uno::Reference< uno::XInterface > SdDrawDocument::createUnoModel()
{
    uno::Reference< uno::XInterface > xModel;

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
        case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
        case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
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

void SdDrawDocument::SetStartWithPresentation( bool bStartWithPresentation )
{
    mbStartWithPresentation = bStartWithPresentation;
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
    pOutliner->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
}

sal_uInt16 SdDrawDocument::GetAnnotationAuthorIndex( const OUString& rAuthor )
{
    // force current user to have first color
    if( maAnnotationAuthors.empty() )
    {
        SvtUserOptions aUserOptions;
        maAnnotationAuthors.push_back( aUserOptions.GetFullName() );
    }

    sal_uInt16 idx = 0;
    const std::vector< OUString >::const_iterator aEnd( maAnnotationAuthors.end());
    for( std::vector< OUString >::const_iterator iter( maAnnotationAuthors.begin() ); iter != aEnd; ++iter )
    {
        if( (*iter) == rAuthor )
        {
            break;
        }
        idx++;
    }

    if( idx == maAnnotationAuthors.size() )
    {
        maAnnotationAuthors.push_back( rAuthor );
    }

    return idx;
}

void SdDrawDocument::InitLayoutVector()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return;

    const Reference<css::uno::XComponentContext> xContext(
        ::comphelper::getProcessComponentContext() );

    // get file list from configuration
    Sequence< rtl::OUString > aFiles(
        officecfg::Office::Impress::Misc::LayoutListFiles::get(xContext) );

    rtl::OUString sFilename;
    for( sal_Int32 i=0; i < aFiles.getLength(); ++i )
    {
        sFilename = comphelper::getExpandedUri(xContext, aFiles[i]);

        // load layout file into DOM
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager() , UNO_QUERY_THROW );
        const Reference<XDocumentBuilder> xDocBuilder(
            DocumentBuilder::create( comphelper::getComponentContext (xServiceFactory) ));

        try
        {
            // loop over every layout entry in current file
            const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
            const Reference<XNodeList> layoutlist = xDoc->getElementsByTagName("layout");
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
    if (utl::ConfigManager::IsAvoidConfig())
        return;

    const Reference<css::uno::XComponentContext> xContext(
        ::comphelper::getProcessComponentContext() );

    // get file list from configuration
    Sequence< rtl::OUString > aFiles(
       officecfg::Office::Impress::Misc::PresObjListFiles::get(xContext) );

    rtl::OUString sFilename;
    for( sal_Int32 i=0; i < aFiles.getLength(); ++i )
    {
        sFilename = comphelper::getExpandedUri(xContext, aFiles[i]);

        // load presentation object file into DOM
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager() , UNO_QUERY_THROW );
        const Reference<XDocumentBuilder> xDocBuilder(
            DocumentBuilder::create( comphelper::getComponentContext (xServiceFactory) ));

        try
        {
            // loop over every object entry in current file
            const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
            const Reference<XNodeList> objectlist = xDoc->getElementsByTagName("object");
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
        xmlTextWriterStartDocument(pWriter, NULL, NULL, NULL);
        bOwns = true;
    }
    FmFormModel::dumpAsXml(pWriter);
    if (bOwns)
    {
        xmlTextWriterEndDocument(pWriter);
        xmlFreeTextWriter(pWriter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
