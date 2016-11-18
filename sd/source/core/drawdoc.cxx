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
#include "precompiled_sd.hxx"

#include "PageListWatcher.hxx"
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/forbiddencharacterstable.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/scriptspaceitem.hxx>

#include <unotools/useroptions.hxx>

#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/dialogs.hrc>
#include "Outliner.hxx"
#include "app.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/fontitem.hxx>
#include <svl/flagitem.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdotext.hxx>
#include <editeng/bulitem.hxx>
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
#include <i18npool/mslangid.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#ifndef _SVTOOLS_PATHOPTIONS_HXX_
#include <unotools/pathoptions.hxx>
#endif
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>

#include "eetext.hxx"
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
#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/GraphicDocShell.hxx"
#include "../ui/inc/sdxfer.hxx"
#include "../ui/inc/ViewShell.hxx"
#include "../ui/inc/optsitem.hxx"
#include "../ui/inc/FrameView.hxx"

// #90477#
#include <tools/tenccvt.hxx>

using ::rtl::OUString;
using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdDrawDocument, FmFormModel );

SdDrawDocument* SdDrawDocument::pDocLockedInsertingLinks = NULL;

//////////////////////////////////////////////////////////////////////////////

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

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

SdDrawDocument::SdDrawDocument(DocumentType eType, SfxObjectShell* pDrDocSh)
: FmFormModel( SvtPathOptions().GetPalettePath(), NULL, pDrDocSh )
, bReadOnly(sal_False)
, mpOutliner(NULL)
, mpInternalOutliner(NULL)
, mpWorkStartupTimer(NULL)
, mpOnlineSpellingTimer(NULL)
, mpOnlineSpellingList(NULL)
, mpOnlineSearchItem(NULL)
, mpFrameViewList( new List() )
, mpCustomShowList(NULL)
, mpDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh))
, mpCreatingTransferable( NULL )
, mbHasOnlineSpellErrors(sal_False)
, mbInitialOnlineSpellingEnabled(sal_True)
, mbNewOrLoadCompleted(sal_False)
, mbStartWithPresentation( false )
, meLanguage( LANGUAGE_SYSTEM )
, meLanguageCJK( LANGUAGE_SYSTEM )
, meLanguageCTL( LANGUAGE_SYSTEM )
, mePageNumType(SVX_ARABIC)
, mbAllocDocSh(sal_False)
, meDocType(eType)
, mpCharClass(NULL)
, mpLocale(NULL)
, mpDrawPageListWatcher(0)
, mpMasterPageListWatcher(0)
{
    // #109538#
    mpDrawPageListWatcher = ::std::auto_ptr<ImpDrawPageListWatcher>(
        new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher = ::std::auto_ptr<ImpMasterPageListWatcher>(
        new ImpMasterPageListWatcher(*this));

    SetObjectShell(pDrDocSh);       // for the VCDrawModel

    if (mpDocSh)
    {
        SetSwapGraphics(sal_True);
    }

    // set measurement unit (of App) and scale (of SdMod)
    sal_Int32 nX, nY;
    SdOptions* pOptions = SD_MOD()->GetSdOptions(meDocType);
    pOptions->GetScale( nX, nY );

    // #92067# Allow UI scale only for draw documents.
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

    // also the DrawingEngine needs to know where it is
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

    // Set the StyleSheetPool to the DrawOutliner, in order to read text objects
    // correctly. The link to the StyleRequest handler of the document will be
    // set first in NewOrLoadCompleted, because only then all templates exist.
    SdrOutliner& rOutliner = GetDrawOutliner();
    rOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    SetCalcFieldValueHdl( &rOutliner );

    // set linguistic options
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

        mbOnlineSpell = aOptions.bIsSpellAuto;
    }

    LanguageType eRealLanguage = MsLangId::getRealLanguage( meLanguage );
    mpLocale = new ::com::sun::star::lang::Locale( MsLangId::convertLanguageToLocale( eRealLanguage ));
    mpCharClass = new CharClass( *mpLocale );

    // If the current application language is a language that uses right-to-left text...
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguage();
    if( MsLangId::isRightToLeft( eRealCTLLanguage ) )
    {
        // ... then we have to set this as a default
        SetDefaultWritingMode( ::com::sun::star::text::WritingMode_RL_TB );
    }

    // for korean and japanese languages we have a different default for apply spacing between asian, latin and ctl text
    if( ( LANGUAGE_KOREAN  == eRealCTLLanguage ) || ( LANGUAGE_KOREAN_JOHAB == eRealCTLLanguage ) || ( LANGUAGE_JAPANESE == eRealCTLLanguage ) )
    {
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( sal_False, EE_PARA_ASIANCJKSPACING ) );
    }

    // set DefTab and SpellOptions
    // now at the module (SD)
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

        SetForbiddenCharsTable( new SvxForbiddenCharactersTable( ::comphelper::getProcessServiceFactory() ) );
    }
    catch(...)
    {
        DBG_ERROR("Can't get SpellChecker");
    }

    rOutliner.SetDefaultLanguage( Application::GetSettings().GetLanguage() );

    if (mpDocSh)
    {
        SetLinkManager( new sfx2::LinkManager(mpDocSh) );
    }

    sal_uLong nCntrl = rOutliner.GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;

    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;

    nCntrl &= ~ EE_CNTRL_ULSPACESUMMATION;
    if ( meDocType != DOCUMENT_TYPE_IMPRESS )
        SetSummationOfParagraphs( sal_False );
    else
    {
        SetSummationOfParagraphs( pOptions->IsSummationOfParagraphs() );
        if ( pOptions->IsSummationOfParagraphs() )
            nCntrl |= EE_CNTRL_ULSPACESUMMATION;
    }
    rOutliner.SetControlWord(nCntrl);

    // Initialize the printer independent layout mode.
    SetPrinterIndependentLayout (pOptions->GetPrinterIndependentLayout());

    // Set the StyleSheetPool to the HitTestOutliner.
    // The link to the StyleRequest handler of the document will be set
    // first in NewOrLoadCompleted, because only then all templates exist.
    SfxItemSet aSet2( pHitTestOutliner->GetEmptyItemSet() );
    pHitTestOutliner->SetStyleSheetPool( (SfxStyleSheetPool*)GetStyleSheetPool() );

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
        DBG_ERROR("Can't get SpellChecker");
    }

    pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguage() );

    sal_uLong nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl2 |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl2 &= ~EE_CNTRL_ONLINESPELLING;

    nCntrl2 &= ~ EE_CNTRL_ULSPACESUMMATION;
    if ( pOptions->IsSummationOfParagraphs() )
        nCntrl2 |= EE_CNTRL_ULSPACESUMMATION;

    pHitTestOutliner->SetControlWord( nCntrl2 );

    /**************************************************************************
    * Create layer
    *
    * The following default layer will created on Pages and MasterPages:
    *
    * Layer STR_LAYOUT  : Default layer for all drawing objects
    *
    * Layer STR_BCKGRND : Background of the MasterPage
    *             (it's currently not used on normal Pages)
    *
    * Layer STR_BCKGRNDOBJ  : Objects on the background of the MasterPage
    *             (it's currently not used on normal Pages)
    *
    * Layer STR_CONTROLS    : Default layer for Controls
    *
    **************************************************************************/
    {
        String aControlLayerName( SdResId(STR_LAYER_CONTROLS) );

        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        rLayerAdmin.NewLayer( String(SdResId(STR_LAYER_LAYOUT)) );
        rLayerAdmin.NewLayer( String(SdResId(STR_LAYER_BCKGRND)) );
        rLayerAdmin.NewLayer( String(SdResId(STR_LAYER_BCKGRNDOBJ)) );
        rLayerAdmin.NewLayer( aControlLayerName );
        rLayerAdmin.NewLayer( String(SdResId(STR_LAYER_MEASURELINES)) );

        rLayerAdmin.SetControlLayerName(aControlLayerName);
    }


}

/*************************************************************************
|*
|* Destructor
|*
\************************************************************************/

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
    SetAllocDocSh(sal_False);

    // #116168#
    ClearModel(sal_True);

    if (pLinkManager)
    {
        // deallocate BaseLinks
        if ( pLinkManager->GetLinks().Count() )
        {
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
        }

        delete pLinkManager;
        pLinkManager = NULL;
    }

    ::sd::FrameView* pFrameView = NULL;

    for (sal_uLong i = 0; i < mpFrameViewList->Count(); i++)
    {
        // if necessary delete FrameViews
        pFrameView =
            static_cast< ::sd::FrameView*>(mpFrameViewList->GetObject(i));

        if (pFrameView)
            delete pFrameView;
    }

    delete mpFrameViewList;
    mpFrameViewList = NULL;

    if (mpCustomShowList)
    {
        for (sal_uLong j = 0; j < mpCustomShowList->Count(); j++)
        {
            // if necessary delete CustomShows
            SdCustomShow* pCustomShow = (SdCustomShow*) mpCustomShowList->GetObject(j);
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

/*************************************************************************
|*
|* This methods creates a new document (SdDrawDocument) and gives back a
|* pointer to it. The Drawing Engine uses this method to be able to put
|* the document or parts of it into Clipboard/DragServer.
|*
\************************************************************************/

SdrModel* SdDrawDocument::AllocModel() const
{
    SdDrawDocument* pNewModel = NULL;

    if( mpCreatingTransferable )
    {
        // the document will created for Drag&Drop/Clipboard,
        // for this the document has to know a DocShell (SvPersist)
        SfxObjectShell*   pObj = NULL;
        ::sd::DrawDocShell*     pNewDocSh = NULL;

        if( meDocType == DOCUMENT_TYPE_IMPRESS )
            mpCreatingTransferable->SetDocShell( new ::sd::DrawDocShell(
                SFX_CREATE_MODE_EMBEDDED, sal_True, meDocType ) );
        else
            mpCreatingTransferable->SetDocShell( new ::sd::GraphicDocShell(
                SFX_CREATE_MODE_EMBEDDED, sal_True, meDocType ) );

        pNewDocSh = static_cast< ::sd::DrawDocShell*>( pObj = mpCreatingTransferable->GetDocShell() );
        pNewDocSh->DoInitNew( NULL );
        pNewModel = pNewDocSh->GetDoc();

        // only necessary for Clipboard,
        // for Drag&Drop this will happen in DragServer
        SdStyleSheetPool* pOldStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        SdStyleSheetPool* pNewStylePool = (SdStyleSheetPool*) pNewModel->GetStyleSheetPool();

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);
        pNewStylePool->CopyCellSheets(*pOldStylePool);
        pNewStylePool->CopyTableStyles(*pOldStylePool);


        for (sal_uInt16 i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            // take all layouts of the MasterPage
            String aOldLayoutName(((SdDrawDocument*) this)->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            SdStyleSheetVector aCreatedSheets;
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool, aCreatedSheets );
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  // loaded from source document
    }
    else if( mbAllocDocSh )
    {
        // a DocShell will be created which will be returned with GetAllocedDocSh()
        SdDrawDocument* pDoc = (SdDrawDocument*) this;
        pDoc->SetAllocDocSh(sal_False);
        pDoc->mxAllocedDocShRef = new ::sd::DrawDocShell(
            SFX_CREATE_MODE_EMBEDDED, sal_True, meDocType);
        pDoc->mxAllocedDocShRef->DoInitNew(NULL);
        pNewModel = pDoc->mxAllocedDocShRef->GetDoc();
    }
    else
    {
        pNewModel = new SdDrawDocument(meDocType, NULL);
    }

    return pNewModel;
}

/*************************************************************************
|*
|* This methods creates a new page (SdPage) and gives back a pointer to it.
|* The Drawing Engine uses this method when loading to create pages (whose
|* types are not knowed for it because these are DERIVATIONS of SdrPage).
|*
\************************************************************************/

SdrPage* SdDrawDocument::AllocPage(FASTBOOL bMasterPage)
{
    return new SdPage(*this, NULL, (sal_Bool)bMasterPage);
}

/*************************************************************************
|*
|* SetChanged(), the modele was changed
|*
\************************************************************************/

void SdDrawDocument::SetChanged(sal_Bool bFlag)
{
    if (mpDocSh)
    {
        if (mbNewOrLoadCompleted && mpDocSh->IsEnableSetModified())
        {
            // forward to baseclass
            FmFormModel::SetChanged(bFlag);

            // pass to ObjectShell
            mpDocSh->SetModified(bFlag);
        }
    }
    else
    {
        // forward to baseclass
        FmFormModel::SetChanged(bFlag);
    }
}

/*************************************************************************
|*
|* NbcSetChanged(), the model changed, don't call anybody else
|*
\************************************************************************/

void SdDrawDocument::NbcSetChanged(sal_Bool bFlag)
{
    // #100237# forward to baseclass
    FmFormModel::SetChanged(bFlag);
}

/*************************************************************************
|*
|* NewOrLoadCompleted
|*
|* Will be called when the document was loaded respectively when it is
|* certain that it will not be loaded any longer.
|*
\************************************************************************/

void SdDrawDocument::NewOrLoadCompleted(DocCreationMode eMode)
{
    if (eMode == NEW_DOC)
    {
        // new document:
        // create Presentation and default templates,
        // create pool for virtual controls
        CreateLayoutTemplates();
        CreateDefaultCellStyles();

        static_cast< SdStyleSheetPool* >( mxStyleSheetPool.get() )->CreatePseudosIfNecessary();
    }
    else if (eMode == DOC_LOADED)
    {
            // document was loaded:

        CheckMasterPages();

        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveUnnecessaryMasterPages( NULL, sal_True, sal_False );

        for ( sal_uInt16 i = 0; i < GetPageCount(); i++ )
        {
            // Check for correct layout names
            SdPage* pPage = (SdPage*) GetPage( i );

            if(pPage->TRG_HasMasterPage())
            {
                SdPage& rMaster = (SdPage&)pPage->TRG_GetMasterPage();

                if(rMaster.GetLayoutName() != pPage->GetLayoutName())
                {
                    pPage->SetLayoutName(rMaster.GetLayoutName());
                }
            }
        }

        for ( sal_uInt16 nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            // LayoutName and PageName must be the same
            SdPage* pPage = (SdPage*) GetMasterPage( nPage );

            String aName( pPage->GetLayoutName() );
            aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ) );

            if( aName != pPage->GetName() )
                pPage->SetName( aName );
        }

        // create language-dependent names of the default layer
        RestoreLayerNames();

        // set language-dependent names of the templates
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->UpdateStdNames();

        // if necessary create missing templates (e.g., formerly there was no Subtitle)
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->CreatePseudosIfNecessary();
    }

    // set default template at the Drawing Engine
    String aName( SdResId(STR_STANDARD_STYLESHEET_NAME));
    SetDefaultStyleSheet(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_GRAPHICS)));

    // #119287# Set default StyleSheet for SdrGrafObj and SdrOle2Obj
    SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(String( SdResId(STR_POOLSHEET_OBJNOLINENOFILL)), SD_STYLE_FAMILY_GRAPHICS)));

    // initialize Draw-Outliner and Document Outliner,
    // but not the global Outliner because it is not document-specific
    // like StyleSheetPool and StyleRequestHandler
    ::Outliner& rDrawOutliner = GetDrawOutliner();
    rDrawOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    sal_uLong nCntrl = rDrawOutliner.GetControlWord();
    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    // initialize HitTest-Outliner and Document Outliner,
    // but not the global Outliner because it is not document-specific
    // like StyleSheetPool and StyleRequestHandler
    pHitTestOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());

    if(mpOutliner)
    {
        mpOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }
    if(mpInternalOutliner)
    {
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }

    if ( eMode == DOC_LOADED )
    {
        // presentation objects have to be Listener again of the respective templates
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*) GetStyleSheetPool();
        sal_uInt16 nPage, nPageCount;

        // #96323# create missing layout style sheets for broken documents
        //         that where created with the 5.2
        nPageCount = GetMasterSdPageCount( PK_STANDARD );
        for (nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = GetMasterSdPage(nPage, PK_STANDARD);
            pSPool->CreateLayoutStyleSheets( pPage->GetName(), sal_True );
        }

        // Default and note pages:
        for (nPage = 0; nPage < GetPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetPage(nPage);
            NewOrLoadCompleted( pPage, pSPool );
        }

        // Masterpages:
        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetMasterPage(nPage);

            NewOrLoadCompleted( pPage, pSPool );
        }
    }

    mbNewOrLoadCompleted = sal_True;

    /**************************************************************************
    * Update all linked Pages
    **************************************************************************/
    SdPage* pPage = NULL;
    sal_uInt16 nMaxSdPages = GetSdPageCount(PK_STANDARD);

    for (sal_uInt16 nSdPage=0; nSdPage < nMaxSdPages; nSdPage++)
    {
        pPage = (SdPage*) GetSdPage(nSdPage, PK_STANDARD);

        if (pPage && pPage->GetFileName().Len() && pPage->GetBookmarkName().Len())
        {
            pPage->SetModel(this);
        }
    }

    UpdateAllLinks();

    SetChanged( sal_False );
}

/** updates all links, only links in this document should by resolved */
void SdDrawDocument::UpdateAllLinks()
{
    if ( !pDocLockedInsertingLinks && pLinkManager && pLinkManager->GetLinks().Count() )
    {
        pDocLockedInsertingLinks = this; // lock inserting links. only links in this document should by resolved

        pLinkManager->UpdateAllLinks();  // query box: update all links?

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;  // unlock inserting links
    }
}

/** this loops over the presentation objectes of a page and repairs some new settings
    from old binary files and resets all default strings for empty presentation objects.
*/
void SdDrawDocument::NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool )
{
/* cl removed because not needed anymore since binfilter
    SdrObjListIter aShapeIter( *pPage );
    while( aShapeIter.IsMore() )
    {
        OutlinerParaObject* pOPO = aShapeIter.Next()->GetOutlinerParaObject();
        if( pOPO )
        {
            if( pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );

            pOPO->FinishLoad( pSPool );
        }
    }
*/

    const sd::ShapeList& rPresentationShapes( pPage->GetPresentationShapeList() );
    if(!rPresentationShapes.isEmpty())
    {
        // create lists with title and outline templates
        String aName = pPage->GetLayoutName();
        aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));

        List* pOutlineList = pSPool->CreateOutlineSheetList(aName);
        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)
                                        pSPool->GetTitleSheet(aName);

        SdrObject* pObj = rPresentationShapes.getNextShape(0);

        // now search for title and outline text objects and
        // change objects into Listener
        while(pObj)
        {
            if (pObj->GetObjInventor() == SdrInventor)
            {
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                sal_uInt16 nId = pObj->GetObjIdentifier();

                if (nId == OBJ_TITLETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );

                    // sal_True: in doing so do not delete hard attributes
                    if (pTitleSheet)
                        pObj->SetStyleSheet(pTitleSheet, sal_True);
                }
                else if (nId == OBJ_OUTLINETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );

                    for (sal_uInt16 nSheet = 0; nSheet < 10; nSheet++)
                    {
                        SfxStyleSheet* pSheet = (SfxStyleSheet*)pOutlineList->GetObject(nSheet);
                        if (pSheet)
                        {
                            pObj->StartListening(*pSheet);

                            if( nSheet == 0)
                                // text frame listen to StyleSheet of Layer1
                                pObj->NbcSetStyleSheet(pSheet, sal_True);
                        }
                    }
                }

                if (pObj->ISA(SdrTextObj) && pObj->IsEmptyPresObj() && pPage)
                {
                    PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                    String aString( pPage->GetPresObjText(ePresObjKind) );

                    if (aString.Len())
                    {
                        sd::Outliner* pInternalOutl = GetInternalOutliner(sal_True);
                        pPage->SetObjText( (SdrTextObj*) pObj, pInternalOutl, ePresObjKind, aString );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), sal_True );
                        pInternalOutl->Clear();
                    }
                }
            }

            pObj = rPresentationShapes.getNextShape(pObj);
        }

        delete pOutlineList;
    }
}

/*************************************************************************
|*
|* Local Outliner that is used for the outline mode.
|* If necessary, insert OutlinerViews into this outliner !
|*
\************************************************************************/

::sd::Outliner* SdDrawDocument::GetOutliner(sal_Bool bCreateOutliner)
{
    if (!mpOutliner && bCreateOutliner)
    {
        mpOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        if (mpDocSh)
            mpOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpOutliner->SetDefTab( nDefaultTabulator );
        mpOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }

    return(mpOutliner);
}


/*************************************************************************
|*
|* Internal Outliner that is used for creating text objects.
|* No OutlinerViews will be inserted into this Outliner!
|*
\************************************************************************/

::sd::Outliner* SdDrawDocument::GetInternalOutliner(sal_Bool bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );
        // MT:
        // This Outliner is used to create special text objects only.
        // As no portion information have to be saved in these text objects,
        // the update mode can/should stay always sal_False.
        mpInternalOutliner->SetUpdateMode( sal_False );
        mpInternalOutliner->EnableUndo( sal_False );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpInternalOutliner->SetDefTab( nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }

    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->GetUpdateMode() == sal_False ) , "InternalOutliner: UpdateMode = sal_True !" );
    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->IsUndoEnabled() == sal_False ), "InternalOutliner: Undo = sal_True !" );

    // MT: Who fills it with spam has to empty it right after:
    // Advantages:
    // a) No uncessary clear calls.
    // b) No spam in memory.
    DBG_ASSERT( !mpInternalOutliner || ( ( mpInternalOutliner->GetParagraphCount() == 1 ) && ( mpInternalOutliner->GetText( mpInternalOutliner->GetParagraph( 0 ) ).Len() == 0 ) ), "InternalOutliner: Nicht leer!" );

    return mpInternalOutliner;
}

/*************************************************************************
|*
|* OnlineSpelling on/off
|*
\************************************************************************/

void SdDrawDocument::SetOnlineSpell(sal_Bool bIn)
{
    mbOnlineSpell = bIn;
    sal_uLong nCntrl = 0;

    if(mpOutliner)
    {
        nCntrl = mpOutliner->GetControlWord();

        if(mbOnlineSpell)
            nCntrl |= EE_CNTRL_ONLINESPELLING;
        else
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;

        mpOutliner->SetControlWord(nCntrl);
    }

    if (mpInternalOutliner)
    {
        nCntrl = mpInternalOutliner->GetControlWord();

        if (mbOnlineSpell)
            nCntrl |= EE_CNTRL_ONLINESPELLING;
        else
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;

        mpInternalOutliner->SetControlWord(nCntrl);
    }

    ::Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;

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


/*************************************************************************
|*
|* OnlineSpelling: Marking on/off
|*
\************************************************************************/

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
    // #108104#
    // DBG_ASSERT (mpDocSh!=NULL, "No available document shell to set ref device at.");

    switch (nMode)
    {
        case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
        case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
            // Just store supported modes and inform the doc shell.
            mnPrinterIndependentLayout = nMode;

            // #108104#
            // Since it is possible that a SdDrawDocument is constructed without a
            // SdDrawDocShell the pointer member mpDocSh needs to be tested
            // before the call is executed. This is e.-g. used for copy/paste.
            if(mpDocSh)
            {
                mpDocSh->UpdateRefDevice ();
            }

            break;

        default:
            // Ignore unknown values.
            break;
    }
}

sal_Int32 SdDrawDocument::GetPrinterIndependentLayout (void)
{
    return mnPrinterIndependentLayout;
}

bool SdDrawDocument::IsStartWithPresentation() const
{
    return mbStartWithPresentation;
}

void SdDrawDocument::SetStartWithPresentation( bool bStartWithPresentation )
{
    mbStartWithPresentation = bStartWithPresentation;
}

// #109538#
void SdDrawDocument::PageListChanged()
{
    mpDrawPageListWatcher->Invalidate();
}

// #109538#
void SdDrawDocument::MasterPageListChanged()
{
    mpMasterPageListWatcher->Invalidate();
}

void SdDrawDocument::SetCalcFieldValueHdl(::Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
}

sal_uInt16 SdDrawDocument::GetAnnotationAuthorIndex( const rtl::OUString& rAuthor )
{
    // force current user to have first color
    if( maAnnotationAuthors.empty() )
    {
        SvtUserOptions aUserOptions;
        maAnnotationAuthors.push_back( aUserOptions.GetFullName() );
    }

    sal_uInt16 idx = 0;
    for( std::vector< OUString >::iterator iter( maAnnotationAuthors.begin() ); iter != maAnnotationAuthors.end(); iter++ )
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

// eof
