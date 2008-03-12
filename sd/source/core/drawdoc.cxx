/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.85 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:25:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "PageListWatcher.hxx"

#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#include <svx/svxids.hrc>
#ifndef _SFX_SRCHITEM_HXX
#include <sfx2/srchitem.hxx>
#endif
#include <svx/eeitem.hxx>
#include <svx/scriptspaceitem.hxx>

#ifndef _OFA_MISCCFG_HXX
#include <svtools/misccfg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif
#include <sfx2/app.hxx>
#include <svx/linkmgr.hxx>
#include <svx/dialogs.hrc>
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#include "app.hxx"

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SVDOATTR_HXX //autogen
#include <svx/svdoattr.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVX_BULITEM_HXX //autogen
#include <svx/bulitem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen wg. SfxItemPool
#include <svtools/itempool.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <svx/xtable.hxx>
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#include <comphelper/extract.hxx>
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#include <unotools/charclass.hxx>
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVTOOLS_PATHOPTIONS_HXX_
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif

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

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../ui/inc/DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "../ui/inc/GraphicDocShell.hxx"
#endif
#include "../ui/inc/sdxfer.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "../ui/inc/ViewShell.hxx"
#endif
#include "../ui/inc/optsitem.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "../ui/inc/FrameView.hxx"
#endif

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

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
, mpFrameViewList( new List() )
, mpCustomShowList(NULL)
, mpDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh))
, mpCreatingTransferable( NULL )
, mbHasOnlineSpellErrors(FALSE)
, mbInitialOnlineSpellingEnabled(TRUE)
, mbNewOrLoadCompleted(FALSE)
, mbStartWithPresentation( false )
, meLanguage( LANGUAGE_SYSTEM )
, meLanguageCJK( LANGUAGE_SYSTEM )
, meLanguageCTL( LANGUAGE_SYSTEM )
, mePageNumType(SVX_ARABIC)
, mbAllocDocSh(FALSE)
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

    SetObjectShell(pDrDocSh);       // fuer das VCDrawModel

    if (mpDocSh)
    {
        SetSwapGraphics(TRUE);
    }

    // Masseinheit (von App) und Massstab (von SdMod) setzen
    INT32 nX, nY;
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

    // die DrawingEngine muss auch wissen, wo er ist
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

    // Dem DrawOutliner den StyleSheetPool setzen, damit Textobjekte richtig
    // eingelesen werden koennen. Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
    SdrOutliner& rOutliner = GetDrawOutliner();
    rOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
    rOutliner.SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));

    // set linguistic options
    {
        const SvtLinguConfig    aLinguConfig;
        SvtLinguOptions         aOptions;
        aLinguConfig.GetOptions( aOptions );

        SetLanguage( aOptions.nDefaultLanguage, EE_CHAR_LANGUAGE );
        SetLanguage( aOptions.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK );
        SetLanguage( aOptions.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL );

        mbOnlineSpell = aOptions.bIsSpellAuto;
        mbHideSpell = aOptions.bIsSpellHideMarkings;
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
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( FALSE, EE_PARA_ASIANCJKSPACING ) );
    }

    // DefTab und SpellOptions setzen
    //OfaMiscCfg* pOfaMiscCfg = SFX_APP()->GetMiscConfig();
    // Jetzt am Modul (SD)
    USHORT nDefTab = pOptions->GetDefTab();
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
        SetLinkManager( new SvxLinkManager(mpDocSh) );
    }

    ULONG nCntrl = rOutliner.GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;

    if (mbHideSpell)
        nCntrl |= EE_CNTRL_NOREDLINES;
    else
        nCntrl &= ~EE_CNTRL_NOREDLINES;

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

    pHitTestOutliner->SetCalcFieldValueHdl( LINK(SD_MOD(), SdModule, CalcFieldValueHdl) );

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

    ULONG nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl2 |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl2 |= EE_CNTRL_NOREDLINES;
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
    * Layer STR_LAYOUT    : Standardlayer fr alle Zeichenobjekte
    *
    * Layer STR_BCKGRND   : Hintergrund der MasterPage
    *                       (auf normalen Pages z.Z. keine Verwendung)
    *
    * Layer STR_BCKGRNDOBJ: Objekte auf dem Hintergrund der MasterPage
    *                       (auf normalen Pages z.Z. keine Verwendung)
    *
    * Layer STR_CONTROLS  : Standardlayer fr Controls
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
    SetAllocDocSh(FALSE);

    // #116168#
    ClearModel(sal_True);

    if (pLinkManager)
    {
        // BaseLinks freigeben
        if ( pLinkManager->GetLinks().Count() )
        {
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
        }

        delete pLinkManager;
        pLinkManager = NULL;
    }

    ::sd::FrameView* pFrameView = NULL;

    for (ULONG i = 0; i < mpFrameViewList->Count(); i++)
    {
        // Ggf. FrameViews loeschen
        pFrameView =
            static_cast< ::sd::FrameView*>(mpFrameViewList->GetObject(i));

        if (pFrameView)
            delete pFrameView;
    }

    delete mpFrameViewList;
    mpFrameViewList = NULL;

    if (mpCustomShowList)
    {
        for (ULONG j = 0; j < mpCustomShowList->Count(); j++)
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
                SFX_CREATE_MODE_EMBEDDED, TRUE, meDocType ) );
        else
            mpCreatingTransferable->SetDocShell( new ::sd::GraphicDocShell(
                SFX_CREATE_MODE_EMBEDDED, TRUE, meDocType ) );

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


        for (USHORT i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
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
        pDoc->SetAllocDocSh(FALSE);
        pDoc->mxAllocedDocShRef = new ::sd::DrawDocShell(
            SFX_CREATE_MODE_EMBEDDED, TRUE, meDocType);
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

SdrPage* SdDrawDocument::AllocPage(FASTBOOL bMasterPage)
{
    return new SdPage(*this, NULL, (BOOL)bMasterPage);
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
    // #100237# forward to baseclass
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
            RemoveUnnessesaryMasterPages( NULL, TRUE, FALSE );

        for ( USHORT i = 0; i < GetPageCount(); i++ )
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

        for ( USHORT nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            // LayoutName and PageName must be the same
            SdPage* pPage = (SdPage*) GetMasterPage( nPage );

            String aName( pPage->GetLayoutName() );
            aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ) );

            if( aName != pPage->GetName() )
                pPage->SetName( aName );

            SdrObject* pPresObj = pPage->GetPresObj( PRESOBJ_BACKGROUND ) ;

            if( pPage->GetPageKind() == PK_STANDARD )
            {
                DBG_ASSERT( pPresObj, "Masterpage without a background object!" );
                if (pPresObj && pPresObj->GetOrdNum() != 0 )
                    pPage->NbcSetObjectOrdNum(pPresObj->GetOrdNum(),0);
            }
            else
            {
                DBG_ASSERT( pPresObj == NULL, "Non Standard Masterpage with a background object!\n(This assertion is ok for old binary files)" );
                if( pPresObj )
                {
                    pPage->RemoveObject( pPresObj->GetOrdNum() );
                    pPage->RemovePresObj(pPresObj);
                    SdrObject::Free( pPresObj );
                }
            }
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
       rDrawOutliner.SetMinDepth(0);
    ULONG nCntrl = rDrawOutliner.GetControlWord();
    if (mbOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    // HitTest-Outliner und  Dokument Outliner initialisieren,
    // aber nicht den globalen Outliner, den der ist ja nicht
    // dokumentspezifisch wie StyleSheetPool und StyleRequestHandler
    pHitTestOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
       pHitTestOutliner->SetMinDepth(0);

    if(mpOutliner)
    {
        mpOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        mpOutliner->SetMinDepth(0);
    }
    if(mpInternalOutliner)
    {
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        mpInternalOutliner->SetMinDepth(0);
    }

    if ( eMode == DOC_LOADED )
    {
        // Praesentationsobjekte muessen wieder Listener der entsprechenden
        // Vorlagen werden
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*) GetStyleSheetPool();
        USHORT nPage, nPageCount;

        // #96323# create missing layout style sheets for broken documents
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

            // BackgroundObjekt vor Selektion schuetzen #62144#
            SdrObject* pBackObj = pPage->GetPresObj(PRESOBJ_BACKGROUND);
            if(pBackObj)
                pBackObj->SetMarkProtect(TRUE);
        }
    }

    mbNewOrLoadCompleted = TRUE;

    /**************************************************************************
    * Alle gelinkten Pages aktualisieren
    **************************************************************************/
    SdPage* pPage = NULL;
    USHORT nMaxSdPages = GetSdPageCount(PK_STANDARD);

    for (USHORT nSdPage=0; nSdPage < nMaxSdPages; nSdPage++)
    {
        pPage = (SdPage*) GetSdPage(nSdPage, PK_STANDARD);

        if (pPage && pPage->GetFileName().Len() && pPage->GetBookmarkName().Len())
        {
            pPage->SetModel(this);
        }
    }

    UpdateAllLinks();

    SetChanged( FALSE );
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
        // Listen mit Titel- und Gliederungsvorlagen erstellen
        String aName = pPage->GetLayoutName();
        aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));

        List* pOutlineList = pSPool->CreateOutlineSheetList(aName);
        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)
                                        pSPool->GetTitleSheet(aName);

        SdrObject* pObj = rPresentationShapes.getNextShape(0);

        // jetzt nach Titel- und Gliederungstextobjekten suchen und
        // Objekte zu Listenern machen
        while(pObj)
        {
            if (pObj->GetObjInventor() == SdrInventor)
            {
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                UINT16 nId = pObj->GetObjIdentifier();

                if (nId == OBJ_TITLETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );

                    // TRUE: harte Attribute dabei nicht loeschen
                    if (pTitleSheet)
                        pObj->SetStyleSheet(pTitleSheet, TRUE);
                }
                else if (nId == OBJ_OUTLINETEXT)
                {
                    if( pOPO && pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );

                    for (USHORT nSheet = 0; nSheet < 10; nSheet++)
                    {
                        SfxStyleSheet* pSheet = (SfxStyleSheet*)pOutlineList->GetObject(nSheet);
                        if (pSheet)
                        {
                            pObj->StartListening(*pSheet);

                            if( nSheet == 0)
                                // Textrahmen hoert auf StyleSheet der Ebene1
                                pObj->NbcSetStyleSheet(pSheet, TRUE);
                        }
                    }
                }

                if (pObj->ISA(SdrTextObj) && pObj->IsEmptyPresObj() && pPage)
                {
                    PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                    String aString( pPage->GetPresObjText(ePresObjKind) );

                    if (aString.Len())
                    {
                        sd::Outliner* pInternalOutl = GetInternalOutliner(TRUE);
                        pInternalOutl->SetMinDepth(0);
                        pPage->SetObjText( (SdrTextObj*) pObj, pInternalOutl, ePresObjKind, aString );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), TRUE );
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
|* Lokaler Outliner, welcher fuer den Gliederungsmodus verwendet wird
|* In diesen Outliner werden ggf. OutlinerViews inserted!
|*
\************************************************************************/

::sd::Outliner* SdDrawDocument::GetOutliner(BOOL bCreateOutliner)
{
    if (!mpOutliner && bCreateOutliner)
    {
        mpOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        if (mpDocSh)
            mpOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpOutliner->SetDefTab( nDefaultTabulator );
        mpOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        mpOutliner->SetMinDepth(0);
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

::sd::Outliner* SdDrawDocument::GetInternalOutliner(BOOL bCreateOutliner)
{
    if ( !mpInternalOutliner && bCreateOutliner )
    {
        mpInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );
        // MT:
        // Dieser Outliner wird nur fuer das Erzeugen spezieller Textobjekte
        // verwendet. Da in diesen Textobjekten keine Portion-Informationen
        // gespeichert werden muessen, kann/soll der Update-Mode immer FALSE bleiben.
        mpInternalOutliner->SetUpdateMode( FALSE );
        mpInternalOutliner->EnableUndo( FALSE );

        if (mpDocSh)
            mpInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        mpInternalOutliner->SetDefTab( nDefaultTabulator );
        mpInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        mpInternalOutliner->SetMinDepth(0);
    }

    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->GetUpdateMode() == FALSE ) , "InternalOutliner: UpdateMode = TRUE !" );
    DBG_ASSERT( !mpInternalOutliner || ( mpInternalOutliner->IsUndoEnabled() == FALSE ), "InternalOutliner: Undo = TRUE !" );

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

void SdDrawDocument::SetOnlineSpell(BOOL bIn)
{
    mbOnlineSpell = bIn;
    ULONG nCntrl = 0;

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

void SdDrawDocument::SetHideSpell(BOOL bIn)
{
    mbHideSpell = bIn;
    ULONG nCntrl = 0;

    if(mpOutliner)
    {
        nCntrl = mpOutliner->GetControlWord();

        if (mbHideSpell)
            nCntrl |= EE_CNTRL_NOREDLINES;
        else
            nCntrl &= ~EE_CNTRL_NOREDLINES;

        mpOutliner->SetControlWord(nCntrl);
    }

    if(mpInternalOutliner)
    {
        nCntrl = mpInternalOutliner->GetControlWord();

        if (mbHideSpell)
            nCntrl |= EE_CNTRL_NOREDLINES;
        else
            nCntrl &= ~EE_CNTRL_NOREDLINES;

        mpInternalOutliner->SetControlWord(nCntrl);
    }

    ::Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (mbHideSpell)
       nCntrl |= EE_CNTRL_NOREDLINES;
    else
       nCntrl &= ~EE_CNTRL_NOREDLINES;

    rOutliner.SetControlWord(nCntrl);
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


// eof
