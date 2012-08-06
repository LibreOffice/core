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
#include <unotools/pathoptions.hxx>
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
, mpOutliner(NULL)
, mpInternalOutliner(NULL)
, mpWorkStartupTimer(NULL)
, mpOnlineSpellingTimer(NULL)
, mpOnlineSpellingList(NULL)
, mpOnlineSearchItem(NULL)
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
    mpDrawPageListWatcher = ::std::auto_ptr<ImpDrawPageListWatcher>(
        new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher = ::std::auto_ptr<ImpMasterPageListWatcher>(
        new ImpMasterPageListWatcher(*this));

    SetObjectShell(pDrDocSh);       // fuer das VCDrawModel

    if (mpDocSh)
    {
        SetSwapGraphics(sal_True);
    }

    // Masseinheit (von App) und Massstab (von SdMod) setzen
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

    // die DrawingEngine muss auch wissen, wo er ist
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

    // Dem DrawOutliner den StyleSheetPool setzen, damit Textobjekte richtig
    // eingelesen werden koennen. Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
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
    if (MsLangId::isKorean(eRealCTLLanguage) || (LANGUAGE_JAPANESE == eRealCTLLanguage))
    {
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( sal_False, EE_PARA_ASIANCJKSPACING ) );
    }

    // DefTab und SpellOptions setzen
    // Jetzt am Modul (SD)
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
        OSL_FAIL("Can't get SpellChecker");
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

    // Dem HitTestOutliner den StyleSheetPool setzen.
    // Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
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
        OSL_FAIL("Can't get SpellChecker");
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
    * Layer anlegen
    *
    * Es werden auf Pages und MasterPages folgende Default-Layer angelegt:
    *
    * Layer STR_LAYOUT    : Standardlayer f�r alle Zeichenobjekte
    *
    * Layer STR_BCKGRND   : Hintergrund der MasterPage
    *                       (auf normalen Pages z.Z. keine Verwendung)
    *
    * Layer STR_BCKGRNDOBJ: Objekte auf dem Hintergrund der MasterPage
    *                       (auf normalen Pages z.Z. keine Verwendung)
    *
    * Layer STR_CONTROLS  : Standardlayer f�r Controls
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
|* Destruktor
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

    ClearModel(sal_True);

    if (pLinkManager)
    {
        // BaseLinks freigeben
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
        for (sal_uLong j = 0; j < mpCustomShowList->Count(); j++)
        {
            // Ggf. CustomShows loeschen
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
|* Diese Methode erzeugt ein neues Dokument (SdDrawDocument) und gibt einen
|* Zeiger darauf zurueck. Die Drawing Engine benutzt diese Methode um das
|* Dokument oder Teile davon ins Clipboard/DragServer stellen zu koennen.
|*
\************************************************************************/

SdrModel* SdDrawDocument::AllocModel() const
{
    SdDrawDocument* pNewModel = NULL;

    if( mpCreatingTransferable )
    {
        // Dokument wird fuer Drag&Drop/Clipboard erzeugt, dafuer muss dem Dokument eine DocShell (SvPersist) bekannt sein
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

        // Nur fuer Clipboard notwendig,
        // fuer Drag&Drop erfolgt dieses im DragServer
        SdStyleSheetPool* pOldStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        SdStyleSheetPool* pNewStylePool = (SdStyleSheetPool*) pNewModel->GetStyleSheetPool();

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);
        pNewStylePool->CopyCellSheets(*pOldStylePool);
        pNewStylePool->CopyTableStyles(*pOldStylePool);


        for (sal_uInt16 i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            // Alle Layouts der MasterPage mitnehmen
            String aOldLayoutName(((SdDrawDocument*) this)->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            SdStyleSheetVector aCreatedSheets;
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool, aCreatedSheets );
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  // loaded from source document
    }
    else if( mbAllocDocSh )
    {
        // Es wird eine DocShell erzeugt, welche mit GetAllocedDocSh() zurueckgegeben wird
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
|* Diese Methode erzeugt eine neue Seite (SdPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/

SdrPage* SdDrawDocument::AllocPage(bool bMasterPage)
{
    return new SdPage(*this, NULL, bMasterPage);
}

/*************************************************************************
|*
|* SetChanged(), das Model wurde geaendert
|*
\************************************************************************/

void SdDrawDocument::SetChanged(sal_Bool bFlag)
{
    if (mpDocSh)
    {
        if (mbNewOrLoadCompleted && mpDocSh->IsEnableSetModified())
        {
            // weitergeben an Basisklasse
            FmFormModel::SetChanged(bFlag);

            // an ObjectShell weiterleiten
            mpDocSh->SetModified(bFlag);
        }
    }
    else
    {
        // weitergeben an Basisklasse
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
    // forward to baseclass
    FmFormModel::SetChanged(bFlag);
}

/*************************************************************************
|*
|* NewOrLoadCompleted
|*
|* Wird gerufen, wenn das Dokument geladen wurde bzw. feststeht, dass es
|* nicht mehr geladen wird.
|*
\************************************************************************/

void SdDrawDocument::NewOrLoadCompleted(DocCreationMode eMode)
{
    if (eMode == NEW_DOC)
    {
        // Neues Dokument:
        // Praesentations- und Standardvorlagen erzeugen,
        // Pool fuer virtuelle Controls erzeugen
        CreateLayoutTemplates();
        CreateDefaultCellStyles();

        static_cast< SdStyleSheetPool* >( mxStyleSheetPool.get() )->CreatePseudosIfNecessary();
    }
    else if (eMode == DOC_LOADED)
    {
            // Dokument wurde geladen:

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

        // Sprachabhaengige Namen der StandardLayer erzeugen
        RestoreLayerNames();

        // Sprachabhaengige Namen der Vorlagen setzen
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->UpdateStdNames();

        // Ggf. fehlende Vorlagen erzeugen (es gab z.B. frueher keinen Subtitle)
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->CreatePseudosIfNecessary();
    }

    // Standardvorlage an der Drawing Engine setzen
    String aName( SdResId(STR_STANDARD_STYLESHEET_NAME));
    SetDefaultStyleSheet(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_GRAPHICS)));

    // Draw-Outliner und  Dokument Outliner initialisieren,
    // aber nicht den globalen Outliner, den der ist ja nicht
    // dokumentspezifisch wie StyleSheetPool und StyleRequestHandler
    ::Outliner& rDrawOutliner = GetDrawOutliner();
    rDrawOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    sal_uLong nCntrl = rDrawOutliner.GetControlWord();
    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    // HitTest-Outliner und  Dokument Outliner initialisieren,
    // aber nicht den globalen Outliner, den der ist ja nicht
    // dokumentspezifisch wie StyleSheetPool und StyleRequestHandler
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
        // Praesentationsobjekte muessen wieder Listener der entsprechenden
        // Vorlagen werden
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*) GetStyleSheetPool();
        sal_uInt16 nPage, nPageCount;

        // create missing layout style sheets for broken documents
        //         that where created with the 5.2
        nPageCount = GetMasterSdPageCount( PK_STANDARD );
        for (nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = GetMasterSdPage(nPage, PK_STANDARD);
            pSPool->CreateLayoutStyleSheets( pPage->GetName(), sal_True );
        }

        // Standard- und Notizseiten:
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
    * Alle gelinkten Pages aktualisieren
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
    if ( !pDocLockedInsertingLinks && pLinkManager && !pLinkManager->GetLinks().empty() )
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
    const sd::ShapeList& rPresentationShapes( pPage->GetPresentationShapeList() );
    if(!rPresentationShapes.isEmpty())
    {
        // Listen mit Titel- und Gliederungsvorlagen erstellen
        String aName = pPage->GetLayoutName();
        aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));

        std::vector<SfxStyleSheetBase*> aOutlineList;
        pSPool->CreateOutlineSheetList(aName,aOutlineList);

        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)pSPool->GetTitleSheet(aName);

        SdrObject* pObj = rPresentationShapes.getNextShape(0);

        // jetzt nach Titel- und Gliederungstextobjekten suchen und
        // Objekte zu Listenern machen
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

                    // sal_True: harte Attribute dabei nicht loeschen
                    if (pTitleSheet)
                        pObj->SetStyleSheet(pTitleSheet, sal_True);
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
                                // Textrahmen hoert auf StyleSheet der Ebene1
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
    }
}

/*************************************************************************
|*
|* Lokaler Outliner, welcher fuer den Gliederungsmodus verwendet wird
|* In diesen Outliner werden ggf. OutlinerViews inserted!
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
|* Interner Outliner, welcher fuer die Erzeugung von Textobjekten
|* verwendet wird.
|* In diesen Outliner werden keine OutlinerViews inserted!
|*
\************************************************************************/

::sd::Outliner* SdDrawDocument::GetInternalOutliner(sal_Bool bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );
        // MT:
        // Dieser Outliner wird nur fuer das Erzeugen spezieller Textobjekte
        // verwendet. Da in diesen Textobjekten keine Portion-Informationen
        // gespeichert werden muessen, kann/soll der Update-Mode immer sal_False bleiben.
        mpInternalOutliner->SetUpdateMode( sal_False );
        mpInternalOutliner->EnableUndo( sal_False );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpInternalOutliner->SetDefTab( nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }

    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->GetUpdateMode() == sal_False ) , "InternalOutliner: UpdateMode = sal_True !" );
    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->IsUndoEnabled() == sal_False ), "InternalOutliner: Undo = sal_True !" );

    // MT: Wer ihn vollmuellt, macht ihn auch gleich wieder leer:
    // Vorteile:
    // a) Keine unnoetigen Clear-Aufrufe
    // b) Kein Muell im Speicher.
    DBG_ASSERT( !mpInternalOutliner || ( ( mpInternalOutliner->GetParagraphCount() == 1 ) && ( mpInternalOutliner->GetText( mpInternalOutliner->GetParagraph( 0 ) ).Len() == 0 ) ), "InternalOutliner: Nicht leer!" );

    return mpInternalOutliner;
}

/*************************************************************************
|*
|* OnlineSpelling ein/aus
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
|* OnlineSpelling: Markierung ein/aus
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
    switch (nMode)
    {
        case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
        case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
            // Just store supported modes and inform the doc shell.
            mnPrinterIndependentLayout = nMode;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
