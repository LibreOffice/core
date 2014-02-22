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
#include <comphelper/expandmacro.hxx>

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
: FmFormModel( SvtPathOptions().GetPalettePath(), NULL, pDrDocSh )
, bReadOnly(sal_False)
, mpOutliner(NULL)
, mpInternalOutliner(NULL)
, mpWorkStartupTimer(NULL)
, mpOnlineSpellingTimer(NULL)
, mpOnlineSpellingList(NULL)
, mpOnlineSearchItem(NULL)
, mpCustomShowList(NULL)
, mpDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh))
, mpCreatingTransferable( NULL )
, mbHasOnlineSpellErrors(false)
, mbInitialOnlineSpellingEnabled(true)
, mbNewOrLoadCompleted(false)
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
, mpDrawPageListWatcher(0)
, mpMasterPageListWatcher(0)
, mbUseEmbedFonts(false)
{
    mpDrawPageListWatcher.reset(new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher.reset(new ImpMasterPageListWatcher(*this));

    InitLayoutVector();
    InitObjectVector();
    SetObjectShell(pDrDocSh);       

    if (mpDocSh)
    {
        SetSwapGraphics(true);
    }

    
    sal_Int32 nX, nY;
    SdOptions* pOptions = SD_MOD()->GetSdOptions(meDocType);
    pOptions->GetScale( nX, nY );

    
    if( eType == DOCUMENT_TYPE_DRAW )
        SetUIUnit( (FieldUnit)pOptions->GetMetric(), Fraction( nX, nY ) );  
    else
        SetUIUnit( (FieldUnit)pOptions->GetMetric(), Fraction( 1, 1 ) );    

    SetScaleUnit(MAP_100TH_MM);
    SetScaleFraction(Fraction(1, 1));
    SetDefaultFontHeight(847);     

    pItemPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pItemPool->FreezeIdRanges();
    SetTextDefaults();

    
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

    
    
    
    SdrOutliner& rOutliner = GetDrawOutliner();
    rOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    SetCalcFieldValueHdl( &rOutliner );

    
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
    LanguageTag aLanguageTag( eRealLanguage);
    mpLocale = new ::com::sun::star::lang::Locale( aLanguageTag.getLocale());
    mpCharClass = new CharClass( aLanguageTag );

    
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    if( MsLangId::isRightToLeft( eRealCTLLanguage ) )
    {
        
        SetDefaultWritingMode( ::com::sun::star::text::WritingMode_RL_TB );
    }

    
    if (MsLangId::isKorean(eRealCTLLanguage) || (LANGUAGE_JAPANESE == eRealCTLLanguage))
    {
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( sal_False, EE_PARA_ASIANCJKSPACING ) );
    }

    
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

    
    SetPrinterIndependentLayout (pOptions->GetPrinterIndependentLayout());

    
    
    
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

    pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    sal_uLong nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl2 |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl2 &= ~EE_CNTRL_ONLINESPELLING;

    nCntrl2 &= ~ EE_CNTRL_ULSPACESUMMATION;
    if ( pOptions->IsSummationOfParagraphs() )
        nCntrl2 |= EE_CNTRL_ULSPACESUMMATION;

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

    ClearModel(sal_True);

    if (pLinkManager)
    {
        
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
    SdDrawDocument* pNewModel = NULL;

    if( mpCreatingTransferable )
    {
        
        
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

        
        
        SdStyleSheetPool* pOldStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        SdStyleSheetPool* pNewStylePool = (SdStyleSheetPool*) pNewModel->GetStyleSheetPool();

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);
        pNewStylePool->CopyCellSheets(*pOldStylePool);
        pNewStylePool->CopyTableStyles(*pOldStylePool);


        for (sal_uInt16 i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            
            OUString aOldLayoutName(((SdDrawDocument*) this)->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aOldLayoutName = aOldLayoutName.copy( 0, aOldLayoutName.indexOf( SD_LT_SEPARATOR ) );
            SdStyleSheetVector aCreatedSheets;
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool, aCreatedSheets );
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  
    }
    else if( mbAllocDocSh )
    {
        
        SdDrawDocument* pDoc = (SdDrawDocument*) this;
        pDoc->SetAllocDocSh(false);
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




SdrPage* SdDrawDocument::AllocPage(bool bMasterPage)
{
    return new SdPage(*this, bMasterPage);
}


void SdDrawDocument::SetChanged(bool bFlag)
{
    if (mpDocSh)
    {
        if (mbNewOrLoadCompleted && mpDocSh->IsEnableSetModified())
        {
            
            FmFormModel::SetChanged(bFlag);

            
            mpDocSh->SetModified(bFlag);
        }
    }
    else
    {
        
        FmFormModel::SetChanged(bFlag);
    }
}


void SdDrawDocument::NbcSetChanged(sal_Bool bFlag)
{
    
    FmFormModel::SetChanged(bFlag);
}



void SdDrawDocument::NewOrLoadCompleted(DocCreationMode eMode)
{
    if (eMode == NEW_DOC)
    {
        
        
        
        CreateLayoutTemplates();
        CreateDefaultCellStyles();

        static_cast< SdStyleSheetPool* >( mxStyleSheetPool.get() )->CreatePseudosIfNecessary();
    }
    else if (eMode == DOC_LOADED)
    {
            

        CheckMasterPages();

        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveUnnecessaryMasterPages( NULL, sal_True, sal_False );

        for ( sal_uInt16 i = 0; i < GetPageCount(); i++ )
        {
            
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
            
            SdPage* pPage = (SdPage*) GetMasterPage( nPage );

            OUString aName( pPage->GetLayoutName() );
            aName = aName.copy( 0, aName.indexOf( SD_LT_SEPARATOR ) );

            if( aName != pPage->GetName() )
                pPage->SetName( aName );
        }

        
        RestoreLayerNames();

        
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->UpdateStdNames();

        
        static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get())->CreatePseudosIfNecessary();
    }

    
    OUString aName( SD_RESSTR(STR_STANDARD_STYLESHEET_NAME));
    SetDefaultStyleSheet(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_GRAPHICS)));

    
    SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(static_cast<SfxStyleSheet*>(mxStyleSheetPool->Find(SD_RESSTR(STR_POOLSHEET_OBJNOLINENOFILL), SD_STYLE_FAMILY_GRAPHICS)));

    
    
    
    ::Outliner& rDrawOutliner = GetDrawOutliner();
    rDrawOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    sal_uLong nCntrl = rDrawOutliner.GetControlWord();
    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    
    
    
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
        
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*) GetStyleSheetPool();
        sal_uInt16 nPage, nPageCount;

        
        
        nPageCount = GetMasterSdPageCount( PK_STANDARD );
        for (nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = GetMasterSdPage(nPage, PK_STANDARD);
            pSPool->CreateLayoutStyleSheets( pPage->GetName(), sal_True );
        }

        
        for (nPage = 0; nPage < GetPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetPage(nPage);
            NewOrLoadCompleted( pPage, pSPool );
        }

        
        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetMasterPage(nPage);

            NewOrLoadCompleted( pPage, pSPool );
        }
    }

    mbNewOrLoadCompleted = true;

    
    SdPage* pPage = NULL;
    sal_uInt16 nMaxSdPages = GetSdPageCount(PK_STANDARD);

    for (sal_uInt16 nSdPage=0; nSdPage < nMaxSdPages; nSdPage++)
    {
        pPage = (SdPage*) GetSdPage(nSdPage, PK_STANDARD);

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
        pDocLockedInsertingLinks = this; 

        pLinkManager->UpdateAllLinks();  

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;  
    }
}

/** this loops over the presentation objectes of a page and repairs some new settings
    from old binary files and resets all default strings for empty presentation objects.
*/
void SdDrawDocument::NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool )
{
    sd::ShapeList& rPresentationShapes( pPage->GetPresentationShapeList() );
    if(!rPresentationShapes.isEmpty())
    {
        
        OUString aName = pPage->GetLayoutName();
        aName = aName.copy( 0, aName.indexOf( SD_LT_SEPARATOR ) );

        std::vector<SfxStyleSheetBase*> aOutlineList;
        pSPool->CreateOutlineSheetList(aName,aOutlineList);

        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)pSPool->GetTitleSheet(aName);

        SdrObject* pObj = 0;
        rPresentationShapes.seekShape(0);

        
        
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
                                
                                pObj->NbcSetStyleSheet(pSheet, sal_True);
                        }
                    }
                }

                if (pObj->ISA(SdrTextObj) && pObj->IsEmptyPresObj() && pPage)
                {
                    PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                    OUString aString( pPage->GetPresObjText(ePresObjKind) );

                    if (!aString.isEmpty())
                    {
                        sd::Outliner* pInternalOutl = GetInternalOutliner(sal_True);
                        pPage->SetObjText( (SdrTextObj*) pObj, pInternalOutl, ePresObjKind, aString );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), sal_True );
                        pInternalOutl->Clear();
                    }
                }
            }
        }
    }
}



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



::sd::Outliner* SdDrawDocument::GetInternalOutliner(sal_Bool bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        
        
        
        mpInternalOutliner->SetUpdateMode( sal_False );
        mpInternalOutliner->EnableUndo( sal_False );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpInternalOutliner->SetDefTab( nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    }

    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->GetUpdateMode() == sal_False ) , "InternalOutliner: UpdateMode = sal_True !" );
    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->IsUndoEnabled() == sal_False ), "InternalOutliner: Undo = sal_True !" );

    
    
    
    
    DBG_ASSERT( !mpInternalOutliner || ( ( mpInternalOutliner->GetParagraphCount() == 1 ) && ( mpInternalOutliner->GetText( mpInternalOutliner->GetParagraph( 0 ) ).isEmpty() ) ), "InternalOutliner: not empty!" );

    return mpInternalOutliner;
}


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
            
            mnPrinterIndependentLayout = nMode;

            
            
            
            if(mpDocSh)
            {
                mpDocSh->UpdateRefDevice ();
            }

            break;

        default:
            
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

bool SdDrawDocument::IsExitAfterPresenting() const
{
    return mbExitAfterPresenting;
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
    const Reference<css::uno::XComponentContext> xContext(
        ::comphelper::getProcessComponentContext() );

    
    Sequence< rtl::OUString > aFiles(
        officecfg::Office::Impress::Misc::LayoutListFiles::get(xContext) );

    rtl::OUString sFilename;
    for( sal_Int32 i=0; i < aFiles.getLength(); ++i )
    {
        sFilename = ::comphelper::getExpandedFilePath(aFiles[i]);

        
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager() , UNO_QUERY_THROW );
        const Reference<XDocumentBuilder> xDocBuilder(
            DocumentBuilder::create( comphelper::getComponentContext (xServiceFactory) ));

        
        const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
        const Reference<XNodeList> layoutlist = xDoc->getElementsByTagName("layout");
        const int nElements = layoutlist->getLength();
        for(int index=0; index < nElements; index++)
            maLayoutInfo.push_back( layoutlist->item(index) );
    }
}

void SdDrawDocument::InitObjectVector()
{
    const Reference<css::uno::XComponentContext> xContext(
        ::comphelper::getProcessComponentContext() );

    
    Sequence< rtl::OUString > aFiles(
       officecfg::Office::Impress::Misc::PresObjListFiles::get(xContext) );

    rtl::OUString sFilename;
    for( sal_Int32 i=0; i < aFiles.getLength(); ++i )
    {
        sFilename = ::comphelper::getExpandedFilePath(aFiles[i]);

        
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager() , UNO_QUERY_THROW );
        const Reference<XDocumentBuilder> xDocBuilder(
            DocumentBuilder::create( comphelper::getComponentContext (xServiceFactory) ));

        
        const Reference<XDocument> xDoc = xDocBuilder->parseURI( sFilename );
        const Reference<XNodeList> objectlist = xDoc->getElementsByTagName("object");
        const int nElements = objectlist->getLength();
        for(int index=0; index < nElements; index++)
            maPresObjectInfo.push_back( objectlist->item(index) );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
