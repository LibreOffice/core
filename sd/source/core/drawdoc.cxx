/*************************************************************************
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:53:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define ITEMID_SEARCH           SID_SEARCH_ITEM

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
#include <svx/srchitem.hxx>
#include <svx/eeitem.hxx>
#define ITEMID_SCRIPTSPACE      EE_PARA_ASIANCJKSPACING
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
#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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

#ifndef MAC
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
#else
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#include "sdxfer.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "optsitem.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#endif

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

using namespace ::rtl;
using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdDrawDocument, FmFormModel );

SdDrawDocument* SdDrawDocument::pDocLockedInsertingLinks = NULL;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdDrawDocument::SdDrawDocument(DocumentType eType, SfxObjectShell* pDrDocSh) :
    FmFormModel(
    SvtPathOptions().GetPalettePath(),
    NULL, pDrDocSh ),
    eDocType(eType),
    pDocSh(static_cast< ::sd::DrawDocShell*>(pDrDocSh)),
    pCreatingTransferable( NULL ),
    bPresAll(TRUE),
    bPresEndless(FALSE),
    bPresManual(FALSE),
    bPresMouseVisible(TRUE),
    bPresMouseAsPen(FALSE),
    bPresLockedPages(FALSE),
    bStartPresWithNavigator(FALSE),
    bAnimationAllowed(TRUE),
    bPresAlwaysOnTop(FALSE),
    bPresFullScreen(TRUE),
    nPresPause(10),
    bPresShowLogo(FALSE),
    bCustomShow(false),
    nPresFirstPage(1),
    pOutliner(NULL),
    pInternalOutliner(NULL),
    ePageNumType(SVX_ARABIC),
    bNewOrLoadCompleted(FALSE),
    pWorkStartupTimer(NULL),
    pOnlineSpellingTimer(NULL),
    pOnlineSpellingList(NULL),
    bInitialOnlineSpellingEnabled(TRUE),
    bHasOnlineSpellErrors(FALSE),
    pOnlineSearchItem(NULL),
    mpInternational(NULL),
    mpLocale(NULL),
    mpCharClass(NULL),
    bAllocDocSh(FALSE),
    pDeletedPresObjList(NULL),
    nFileFormatVersion(SDIOCOMPAT_VERSIONDONTKNOW),
    pCustomShowList(NULL),
    eLanguage( LANGUAGE_SYSTEM ),
    eLanguageCJK( LANGUAGE_SYSTEM ),
    eLanguageCTL( LANGUAGE_SYSTEM ),
    mbStartWithPresentation( false ),
    // #109538#
    mpDrawPageListWatcher(0),
    mpMasterPageListWatcher(0)
{
    // #109538#
    mpDrawPageListWatcher = ::std::auto_ptr<ImpDrawPageListWatcher>(
        new ImpDrawPageListWatcher(*this));
    mpMasterPageListWatcher = ::std::auto_ptr<ImpMasterPageListWatcher>(
        new ImpMasterPageListWatcher(*this));

    SetObjectShell(pDrDocSh);       // fuer das VCDrawModel

    if (pDocSh)
    {
        SetSwapGraphics(TRUE);
    }

    // Masseinheit (von App) und Massstab (von SdMod) setzen
    INT32 nX, nY;
    SdOptions* pOptions = SD_MOD()->GetSdOptions(eDocType);
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

        bOnlineSpell = aOptions.bIsSpellAuto;
        bHideSpell = aOptions.bIsSpellHideMarkings;
    }

    LanguageType eRealLanguage = International::GetRealLanguage( eLanguage );

    mpInternational = new International(eLanguage);
    String aLanguage, aCountry, aEmpty;
    ConvertLanguageToIsoNames( eRealLanguage, aLanguage, aCountry );
    mpLocale = new ::com::sun::star::lang::Locale( aLanguage, aCountry, aEmpty );
    mpCharClass = new CharClass( *mpLocale );

    // If the current application language is a language that uses right-to-left text...
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguage();
    if( (LANGUAGE_ARABIC == (eRealCTLLanguage & 0x00ff)) ||
        (LANGUAGE_URDU == (eRealCTLLanguage & 0x00ff)) ||
        (LANGUAGE_HEBREW == eRealCTLLanguage) )
    {
        // ... then we have to set this as a default
        SetDefaultWritingMode( ::com::sun::star::text::WritingMode_RL_TB );
    }

    // for korean and japanese languages we have a different default for apply spacing between asian, latin and ctl text
    if( ( LANGUAGE_KOREAN  == eRealCTLLanguage ) || ( LANGUAGE_KOREAN_JOHAB == eRealCTLLanguage ) || ( LANGUAGE_JAPANESE == eRealCTLLanguage ) )
    {
        GetPool().GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( FALSE ) );
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

    aOldNotifyUndoActionHdl = GetNotifyUndoActionHdl();
    SetNotifyUndoActionHdl(LINK(this, SdDrawDocument, NotifyUndoActionHdl));

    if (pDocSh)
    {
        SetLinkManager( new SvxLinkManager(pDocSh) );
    }

    ULONG nCntrl = rOutliner.GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;

    if (bHideSpell)
        nCntrl |= EE_CNTRL_NOREDLINES;
    else
        nCntrl &= ~EE_CNTRL_NOREDLINES;

    if (bOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;

    nCntrl &= ~ EE_CNTRL_ULSPACESUMMATION;
    if ( eDocType != DOCUMENT_TYPE_IMPRESS )
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

    pFrameViewList = new List();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdDrawDocument::~SdDrawDocument()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    if (pWorkStartupTimer)
    {
        if ( pWorkStartupTimer->IsActive() )
            pWorkStartupTimer->Stop();

        delete pWorkStartupTimer;
        pWorkStartupTimer = NULL;
    }

    StopOnlineSpelling();
    delete pOnlineSearchItem;
    pOnlineSearchItem = NULL;

    CloseBookmarkDoc();
    SetAllocDocSh(FALSE);

    SetNotifyUndoActionHdl(aOldNotifyUndoActionHdl);

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

    for (ULONG i = 0; i < pFrameViewList->Count(); i++)
    {
        // Ggf. FrameViews loeschen
        pFrameView =
            static_cast< ::sd::FrameView*>(pFrameViewList->GetObject(i));

        if (pFrameView)
            delete pFrameView;
    }

    delete pFrameViewList;
    pFrameViewList = NULL;

    if (pCustomShowList)
    {
        for (ULONG j = 0; j < pCustomShowList->Count(); j++)
        {
            // Ggf. CustomShows loeschen
            SdCustomShow* pCustomShow = (SdCustomShow*) pCustomShowList->GetObject(j);
            delete pCustomShow;
        }

        delete pCustomShowList;
        pCustomShowList = NULL;
    }

    delete pOutliner;
    pOutliner = NULL;

    delete pInternalOutliner;
    pInternalOutliner = NULL;

    delete pDeletedPresObjList;
    pDeletedPresObjList = NULL;

    delete mpInternational;
    mpInternational = NULL;

    delete mpLocale;
    mpLocale = NULL;

    delete mpCharClass;
    mpCharClass = NULL;
    /*af deletion is done by auto ptr s.
    // #109538#
    delete mpDrawPageListWatcher;
    mpDrawPageListWatcher = 0L;

    // #109538#
    delete mpMasterPageListWatcher;
    mpMasterPageListWatcher = 0L;
        */
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

    if( pCreatingTransferable )
    {
        // Dokument wird fuer Drag&Drop/Clipboard erzeugt, dafuer muss dem Dokument eine DocShell (SvPersist) bekannt sein
        SfxObjectShell*   pObj = NULL;
        ::sd::DrawDocShell*     pNewDocSh = NULL;

        if( eDocType == DOCUMENT_TYPE_IMPRESS )
            pCreatingTransferable->SetDocShell( new ::sd::DrawDocShell(
                SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType ) );
        else
            pCreatingTransferable->SetDocShell( new ::sd::GraphicDocShell(
                SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType ) );

        pNewDocSh = static_cast< ::sd::DrawDocShell*>( pObj = pCreatingTransferable->GetDocShell() );
        pNewDocSh->DoInitNew( NULL );
        pNewModel = pNewDocSh->GetDoc();

        // Nur fuer Clipboard notwendig,
        // fuer Drag&Drop erfolgt dieses im DragServer
        SdStyleSheetPool* pOldStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        SdStyleSheetPool* pNewStylePool = (SdStyleSheetPool*) pNewModel->GetStyleSheetPool();

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);

        for (USHORT i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            // Alle Layouts der MasterPage mitnehmen
            String aOldLayoutName(((SdDrawDocument*) this)->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool);
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  // loaded from source document
    }
    else if( bAllocDocSh )
    {
        // Es wird eine DocShell erzeugt, welche mit GetAllocedDocSh() zurueckgegeben wird
        SdDrawDocument* pDoc = (SdDrawDocument*) this;
        pDoc->SetAllocDocSh(FALSE);
        pDoc->xAllocedDocShRef = new ::sd::DrawDocShell(
            SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType);
        pDoc->xAllocedDocShRef->DoInitNew(NULL);
        pNewModel = pDoc->xAllocedDocShRef->GetDoc();
    }
    else
    {
        pNewModel = new SdDrawDocument(eDocType, NULL);
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
    return new SdPage(*this, NULL, bMasterPage);
}

/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/

//BFS02SvStream& operator << (SvStream& rOut, SdDrawDocument& rDoc)
//BFS02{
//BFS02 // #90477# CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());
//BFS02 CharSet eSysSet = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());
//BFS02
//BFS02 /**************************************************************************
//BFS02 * Aktuelle FileFormat-Versionsnummer
//BFS02 * Bei Aenderugen stets inkrementieren und beim Laden beruecksichtigen!
//BFS02 **************************************************************************/
//BFS02 rDoc.nFileFormatVersion = 18;
//BFS02
//BFS02 // AutoLayouts muessen ggf. erzeugt werden
//BFS02 rDoc.StopWorkStartupDelay();
//BFS02
//BFS02 // Eindeutige Namen der StandardLayer erzeugen
//BFS02 rDoc.MakeUniqueLayerNames();
//BFS02
//BFS02 rOut << (FmFormModel&) rDoc;
//BFS02
//BFS02 // Sprachabhaengige Namen der StandardLayer wieder herstellen
//BFS02 rDoc.RestoreLayerNames();
//BFS02
//BFS02 if ( rDoc.IsStreamingSdrModel() )
//BFS02 {
//BFS02     // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
//BFS02     // Anwendungsfall: svdraw Clipboard-Format
//BFS02     return(rOut);
//BFS02 }
//BFS02
//BFS02 SdIOCompat aIO(rOut, STREAM_WRITE, rDoc.nFileFormatVersion);
//BFS02
//BFS02 BOOL bDummy = TRUE;
//BFS02 rOut << bDummy;                    // ehem. bPresentation
//BFS02 rOut << rDoc.bPresAll;
//BFS02 rOut << rDoc.bPresEndless;
//BFS02 rOut << rDoc.bPresManual;
//BFS02 rOut << rDoc.bPresMouseVisible;
//BFS02 rOut << rDoc.bPresMouseAsPen;
//BFS02 rOut << rDoc.nPresFirstPage;
//BFS02
//BFS02 // Es wird nun eine Liste von FrameViews geschrieben (siehe weiter unten),
//BFS02 // daher wird an dieser Stelle ein FALSE vermerkt.
//BFS02 BOOL bSingleFrameView = FALSE;
//BFS02 rOut << bSingleFrameView;
//BFS02
//BFS02 /**************************************************************************
//BFS02 * Frueher (StarDraw Version 3.0, File-Format Version 3) wurde hier das
//BFS02 * JobSetup geschrieben, nun der Printer (binaer-kompatibel, daher wurde
//BFS02 * die Version des File-Formats nicht geaendert)
//BFS02 **************************************************************************/
//BFS02 if (rDoc.pDocSh)
//BFS02 {
//BFS02     SfxPrinter* pPrinter = rDoc.pDocSh->GetPrinter(TRUE);
//BFS02     pPrinter->Store(rOut);
//BFS02 }
//BFS02 else
//BFS02 {
//BFS02     // Keine DocShell, daher wird ein JobSetup geschrieben
//BFS02     JobSetup aJobSetup;
//BFS02     rOut << aJobSetup;
//BFS02 }
//BFS02
//BFS02 rOut << (ULONG) rDoc.eLanguage;
//BFS02
//BFS02 /**************************************************************************
//BFS02 * FrameViews schreiben
//BFS02 **************************************************************************/
//BFS02 ULONG nFrameViewCount = 0;
//BFS02 ::sd::ViewShell* pViewSh = NULL;
//BFS02 SfxViewShell* pSfxViewSh = NULL;
//BFS02 SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(rDoc.pDocSh,
//BFS02                                                      TYPE(SfxTopViewFrame));
//BFS02
//BFS02 while (pSfxViewFrame)
//BFS02 {
//BFS02     // Anzahl FrameViews ermitteln
//BFS02     pSfxViewSh = pSfxViewFrame->GetViewShell();
//BFS02     pViewSh = PTR_CAST(::sd::ViewShell, pSfxViewSh );
//BFS02
//BFS02     if ( pViewSh && pViewSh->GetFrameView() )
//BFS02     {
//BFS02         nFrameViewCount++;
//BFS02     }
//BFS02
//BFS02     pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, rDoc.pDocSh,
//BFS02                                           TYPE(SfxTopViewFrame));
//BFS02 }
//BFS02
//BFS02 // Anzahl FrameViews schreiben
//BFS02 rOut << nFrameViewCount;
//BFS02
//BFS02 ::sd::FrameView* pFrame = NULL;
//BFS02 pViewSh = NULL;
//BFS02 pSfxViewSh = NULL;
//BFS02 pSfxViewFrame = SfxViewFrame::GetFirst(rDoc.pDocSh, TYPE(SfxTopViewFrame));
//BFS02
//BFS02 while (pSfxViewFrame)
//BFS02 {
//BFS02     // FrameViews schreiben
//BFS02     pSfxViewSh = pSfxViewFrame->GetViewShell();
//BFS02     pViewSh = PTR_CAST(::sd::ViewShell, pSfxViewSh );
//BFS02
//BFS02     if ( pViewSh && pViewSh->GetFrameView() )
//BFS02     {
//BFS02         pViewSh->WriteFrameViewData();
//BFS02         rOut << *pViewSh->GetFrameView();
//BFS02     }
//BFS02
//BFS02     pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, rDoc.pDocSh,
//BFS02                                           TYPE(SfxTopViewFrame));
//BFS02 }
//BFS02
//BFS02 rOut << rDoc.bStartPresWithNavigator;
//BFS02 rOut << rDoc.bPresLockedPages;
//BFS02 rOut << rDoc.bPresAlwaysOnTop;
//BFS02 rOut << rDoc.bOnlineSpell;
//BFS02 rOut << rDoc.bHideSpell;
//BFS02 rOut << rDoc.bPresFullScreen;
//BFS02 rOut.WriteByteString( rDoc.aPresPage, eSysSet );
//BFS02 rOut << rDoc.bAnimationAllowed;
//BFS02
//BFS02 UINT16 nDocType = (UINT16) rDoc.eDocType;
//BFS02 rOut << nDocType;
//BFS02
//BFS02 // CustomShow aktiv
//BFS02 rOut << rDoc.bCustomShow;
//BFS02
//BFS02 // Anzahl CustomShows schreiben
//BFS02 ULONG nCustomShowCount = 0;
//BFS02
//BFS02 if (rDoc.pCustomShowList)
//BFS02 {
//BFS02     nCustomShowCount = rDoc.pCustomShowList->Count();
//BFS02 }
//BFS02
//BFS02 rOut << nCustomShowCount;
//BFS02
//BFS02 if (rDoc.pCustomShowList)
//BFS02 {
//BFS02     for (ULONG i = 0; i < nCustomShowCount; i++)
//BFS02     {
//BFS02         // CustomShows schreiben
//BFS02         SdCustomShow* pCustomShow = (SdCustomShow*) rDoc.pCustomShowList->GetObject(i);
//BFS02         rOut << *pCustomShow;
//BFS02     }
//BFS02
//BFS02     // Position der aktuellen CustomShow
//BFS02     ULONG nCurPos = rDoc.pCustomShowList->GetCurPos();
//BFS02     rOut << nCurPos;
//BFS02 }
//BFS02
//BFS02 // ab Version 15
//BFS02 rOut << (ULONG) rDoc.GetPageNumType();
//BFS02
//BFS02 // ab Version 17
//BFS02 rOut << rDoc.GetPresPause() << rDoc.IsPresShowLogo();
//BFS02
//BFS02 // ab Version 18 (keine Aenderung)
//BFS02
//BFS02 return rOut;
//BFS02}

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

//BFS02SvStream& operator >> (SvStream& rIn, SdDrawDocument& rDoc)
//BFS02{
//BFS02 // #90477# CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());
//BFS02 CharSet eSysSet = GetSOLoadTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rIn.GetVersion());
//BFS02
//BFS02 rIn >> (FmFormModel&) rDoc;
//BFS02 rDoc.GetItemPool().LoadCompleted();
//BFS02 rDoc.SetTextDefaults();     // overwrites loaded pool defaults
//BFS02
//BFS02    // Turn off printer independent layout (make it printer *dependent*) for
//BFS02    // pre-6.0 documents.
//BFS02    rDoc.SetPrinterIndependentLayout (
//BFS02        ::com::sun::star::document::PrinterIndependentLayout::DISABLED);
//BFS02
//BFS02 // Fehler ?
//BFS02 if (rIn.GetError() != 0)
//BFS02     return (rIn);
//BFS02
//BFS02 if ( rDoc.IsStreamingSdrModel() )
//BFS02 {
//BFS02     // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
//BFS02     // Anwendungsfall: svdraw Clipboard-Format
//BFS02     return(rIn);
//BFS02 }
//BFS02
//BFS02 SdIOCompat aIO(rIn, STREAM_READ);
//BFS02
//BFS02 BOOL bDummy;
//BFS02 rIn >> bDummy;                     // ehem. bPresentation
//BFS02 rIn >> rDoc.bPresAll;
//BFS02 rIn >> rDoc.bPresEndless;
//BFS02 rIn >> rDoc.bPresManual;
//BFS02 rIn >> rDoc.bPresMouseVisible;
//BFS02 rIn >> rDoc.bPresMouseAsPen;
//BFS02 rIn >> rDoc.nPresFirstPage;
//BFS02
//BFS02 rDoc.nFileFormatVersion = aIO.GetVersion();
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 1)
//BFS02 {
//BFS02     // Daten der Versionen >= 1 einlesen
//BFS02
//BFS02     BOOL bSingleFrameView;
//BFS02     rIn >> bSingleFrameView;
//BFS02
//BFS02     if (bSingleFrameView)
//BFS02     {
//BFS02         ::sd::FrameView * pFrameView = new ::sd::FrameView( &rDoc );
//BFS02         rIn >> *pFrameView;
//BFS02         rDoc.pFrameViewList->Insert(pFrameView, LIST_APPEND);
//BFS02
//BFS02         // Fehler ?
//BFS02         if (rIn.GetError() != 0)
//BFS02             return (rIn);
//BFS02     }
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 2)
//BFS02 {
//BFS02     // Daten der Versionen >= 2 einlesen
//BFS02
//BFS02     /******************************************************************
//BFS02     * Frueher (StarDraw Version 3.0, File-Format Version 3) wurde hier
//BFS02     * das JobSetup eingelesen, nun wird der Printer erzeugt
//BFS02     * (binaer-kompatibel)
//BFS02     *******************************************************************/
//BFS02     // ItemSet mit speziellem Poolbereich anlegen
//BFS02     SfxItemSet* pSet = new SfxItemSet( rDoc.GetPool(),
//BFS02                     SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
//BFS02                     SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
//BFS02                     ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
//BFS02                     0 );
//BFS02     // PrintOptionsSet setzen
//BFS02     SdOptionsPrintItem aPrintItem(ATTR_OPTIONS_PRINT
//BFS02                                   ,SD_MOD()->GetSdOptions(rDoc.eDocType)
//BFS02                                   );
//BFS02
//BFS02     SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
//BFS02     USHORT      nFlags = 0;
//BFS02
//BFS02     nFlags =  (aPrintItem.IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
//BFS02             (aPrintItem.IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
//BFS02     aFlagItem.SetValue( nFlags );
//BFS02
//BFS02     pSet->Put( aPrintItem );
//BFS02     pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.IsWarningPrinter() ) );
//BFS02     pSet->Put( aFlagItem );
//BFS02
//BFS02     SfxPrinter* pPrinter = SfxPrinter::Create(rIn, pSet);
//BFS02
//BFS02     MapMode aMM (pPrinter->GetMapMode());
//BFS02     aMM.SetMapUnit(MAP_100TH_MM);
//BFS02     pPrinter->SetMapMode(aMM);
//BFS02     if (rDoc.pDocSh)            // z. B. nicht bei "Einfuegen-Datei"
//BFS02         rDoc.pDocSh->SetPrinter(pPrinter);
//BFS02     else
//BFS02         delete pPrinter;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 3)
//BFS02 {
//BFS02     ULONG nTmp;
//BFS02     rIn >> nTmp;
//BFS02     rDoc.SetLanguage( (LanguageType) nTmp, EE_CHAR_LANGUAGE );
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 4)
//BFS02 {
//BFS02     /**********************************************************************
//BFS02     * FrameViews lesen
//BFS02     **********************************************************************/
//BFS02     ULONG nCount = 0;
//BFS02     ::sd::FrameView* pFrameView = NULL;
//BFS02
//BFS02     for (nCount=0; nCount<rDoc.pFrameViewList->Count(); nCount++)
//BFS02     {
//BFS02         // Ggf. FrameViews loeschen
//BFS02         pFrameView = static_cast< ::sd::FrameView*>(
//BFS02                rDoc.pFrameViewList->GetObject(nCount));
//BFS02
//BFS02         if (pFrameView)
//BFS02             delete pFrameView;
//BFS02     }
//BFS02
//BFS02     rDoc.pFrameViewList->Clear();
//BFS02
//BFS02     // Anzahl FrameViews lesen
//BFS02     const SvtSaveOptions aOptions;
//BFS02     BOOL bIsSaveDocView = aOptions.IsSaveDocView();
//BFS02
//BFS02     ULONG nFrameViewCount = 0;
//BFS02     rIn >> nFrameViewCount;
//BFS02
//BFS02     for (nCount=0; nCount<nFrameViewCount; nCount++)
//BFS02     {
//BFS02         // Einzelne FrameViews lesen
//BFS02         pFrameView = new ::sd::FrameView( &rDoc );
//BFS02         rIn >> *pFrameView;
//BFS02
//BFS02         if (bIsSaveDocView)
//BFS02         {
//BFS02             // FrameViews werden fuer die ViewShell gebraucht
//BFS02             // Die FrameView gehoert nun der Liste
//BFS02             rDoc.pFrameViewList->Insert(pFrameView, nCount);
//BFS02         }
//BFS02         else
//BFS02         {
//BFS02             // FrameView kann wieder geloescht werden
//BFS02             delete pFrameView;
//BFS02         }
//BFS02
//BFS02         // Fehler ?
//BFS02         if (rIn.GetError() != 0)
//BFS02             return (rIn);
//BFS02     }
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 5)
//BFS02 {
//BFS02     rIn >> rDoc.bStartPresWithNavigator;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 6)
//BFS02 {
//BFS02     rIn >> rDoc.bPresLockedPages;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 7)
//BFS02 {
//BFS02     rIn >> rDoc.bPresAlwaysOnTop;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 8)
//BFS02 {
//BFS02     rIn >> rDoc.bOnlineSpell;
//BFS02     rIn >> rDoc.bHideSpell;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 9)
//BFS02 {
//BFS02     rIn >> rDoc.bPresFullScreen;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 10)
//BFS02 {
//BFS02     rIn.ReadByteString( rDoc.aPresPage, eSysSet );
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 11)
//BFS02 {
//BFS02     rIn >> rDoc.bAnimationAllowed;
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 12)
//BFS02 {
//BFS02     UINT16 nDocType;
//BFS02     rIn >> nDocType;
//BFS02     rDoc.eDocType = (DocumentType) nDocType;
//BFS02     // existiert eine DocShell bestimmt diese den DocType
//BFS02     if(rDoc.pDocSh)
//BFS02     {
//BFS02         if(NULL != PTR_CAST(::sd::GraphicDocShell,rDoc.pDocSh))
//BFS02             rDoc.eDocType = DOCUMENT_TYPE_DRAW;
//BFS02         else
//BFS02             rDoc.eDocType = DOCUMENT_TYPE_IMPRESS;
//BFS02     }
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 13)
//BFS02 {
//BFS02     // Keine Aenderung
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 14)
//BFS02 {
//BFS02     // CustomShow aktiv
//BFS02     rIn >> rDoc.bCustomShow;
//BFS02
//BFS02     ULONG nCustomShowCount = 0;
//BFS02     rIn >> nCustomShowCount;
//BFS02
//BFS02     if (nCustomShowCount > 0)
//BFS02     {
//BFS02         // Liste erzeugen
//BFS02         rDoc.GetCustomShowList(TRUE);
//BFS02         rDoc.pCustomShowList->Clear();
//BFS02
//BFS02         for (ULONG i = 0; i < nCustomShowCount; i++)
//BFS02         {
//BFS02             // Einzelne CustomShows lesen
//BFS02             SdCustomShow* pCustomShow = new SdCustomShow(&rDoc);
//BFS02             rIn >> *pCustomShow;
//BFS02
//BFS02             // Die CustomShows gehoert nun der Liste
//BFS02             rDoc.pCustomShowList->Insert(pCustomShow, i);
//BFS02
//BFS02             // Fehler ?
//BFS02             if (rIn.GetError() != 0)
//BFS02                 return (rIn);
//BFS02         }
//BFS02
//BFS02         // Aktuelle CustomShow selektieren
//BFS02         ULONG nCurPos;
//BFS02         rIn >> nCurPos;
//BFS02         rDoc.pCustomShowList->Seek(nCurPos);
//BFS02     }
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 15)
//BFS02 {
//BFS02     ULONG nTmp;
//BFS02     rIn >> nTmp;
//BFS02     rDoc.SetPageNumType( (SvxNumType) nTmp );
//BFS02 }
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 17)
//BFS02 {
//BFS02     ULONG   nPauseSec;
//BFS02     BOOL    bShowLogo;
//BFS02
//BFS02     rIn >> nPauseSec >> bShowLogo;
//BFS02     rDoc.SetPresPause( nPauseSec );
//BFS02     rDoc.SetPresShowLogo( bShowLogo );
//BFS02 }
//BFS02 else
//BFS02     rDoc.SetPresPause( 0 );
//BFS02
//BFS02 if (rDoc.nFileFormatVersion >= 18)
//BFS02 {
//BFS02     // Keine Aenderung
//BFS02 }
//BFS02
//BFS02 /**************************************************************************
//BFS02 * So machts der Writer, und so muessen es alle machen:
//BFS02 * Bug 9714: Der CharSet an den Fonts muss geaendert werden, wenn
//BFS02 * es der globale CharSet ist (MT)
//BFS02 **************************************************************************/
//BFS02 SfxItemPool& rPool = rDoc.GetItemPool();
//BFS02 USHORT nMaxItems = rPool.GetItemCount(EE_CHAR_FONTINFO);
//BFS02 SvxFontItem* pItem;
//BFS02 CharSet eSrcSet = ((SdPage*) rDoc.GetPage(0))->GetCharSet();
//BFS02
//BFS02 for (USHORT n = 0; n < nMaxItems; ++n)
//BFS02 {
//BFS02     pItem = (SvxFontItem*) rPool.GetItem(EE_CHAR_FONTINFO, n);
//BFS02     if (pItem && pItem->GetCharSet() == eSrcSet)
//BFS02     {
//BFS02         pItem->GetCharSet() = eSysSet;
//BFS02     }
//BFS02 }
//BFS02
//BFS02 return rIn;
//BFS02}

/*************************************************************************
|*
|* "Alle Dias"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresAll(BOOL bNewPresAll)
{
    bPresAll     = bNewPresAll;
}

/*************************************************************************
|*
|* "Endlos"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresEndless(BOOL bNewPresEndless)
{
    bPresEndless = bNewPresEndless;
}

/*************************************************************************
|*
|* "Manuell"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresManual(BOOL bNewPresManual)
{
    bPresManual  = bNewPresManual;
}

/*************************************************************************
|*
|* "Maus sichtbar"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresMouseVisible(BOOL bNewPresMouseVisible)
{
    bPresMouseVisible = bNewPresMouseVisible;
}

/*************************************************************************
|*
|* "Maus als Stift"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresMouseAsPen(BOOL bNewPresMouseAsPen)
{
    bPresMouseAsPen = bNewPresMouseAsPen;
}

/*************************************************************************
|*
|* "Ab Dia"-Parameter der Praesentation aendern
|*
\************************************************************************/

void SdDrawDocument::SetPresFirstPage(ULONG nNewPresFirstPage)
{
    nPresFirstPage = nNewPresFirstPage;
}

/*************************************************************************
|*
|* SetChanged(), das Model wurde geaendert
|*
\************************************************************************/

void SdDrawDocument::SetChanged(sal_Bool bFlag)
{
    if (pDocSh)
    {
        if (bNewOrLoadCompleted && pDocSh->IsEnableSetModified())
        {
            // weitergeben an Basisklasse
            FmFormModel::SetChanged(bFlag);

            // an ObjectShell weiterleiten
            pDocSh->SetModified(bFlag);
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

        ((SdStyleSheetPool*)pStyleSheetPool)->CreatePseudosIfNecessary();
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
                    delete pPresObj;
                }
            }
        }

        // Sprachabhaengige Namen der StandardLayer erzeugen
        RestoreLayerNames();

        // Sprachabhaengige Namen der Vorlagen setzen
        ((SdStyleSheetPool*)pStyleSheetPool)->UpdateStdNames();

        // Ggf. fehlende Vorlagen erzeugen (es gab z.B. frueher keinen Subtitle)
        ((SdStyleSheetPool*)pStyleSheetPool)->CreatePseudosIfNecessary();
    }

    // Standardvorlage an der Drawing Engine setzen
    String aName( SdResId(STR_STANDARD_STYLESHEET_NAME));
    SfxStyleSheet* pSheet = (SfxStyleSheet*)pStyleSheetPool->
                                    Find(aName, SFX_STYLE_FAMILY_PARA);
    SetDefaultStyleSheet(pSheet);

    // Draw-Outliner und  Dokument Outliner initialisieren,
    // aber nicht den globalen Outliner, den der ist ja nicht
    // dokumentspezifisch wie StyleSheetPool und StyleRequestHandler
    ::Outliner& rDrawOutliner = GetDrawOutliner();
    rDrawOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
       rDrawOutliner.SetMinDepth(0);
    ULONG nCntrl = rDrawOutliner.GetControlWord();
    if (bOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rDrawOutliner.SetControlWord(nCntrl);

    // HitTest-Outliner und  Dokument Outliner initialisieren,
    // aber nicht den globalen Outliner, den der ist ja nicht
    // dokumentspezifisch wie StyleSheetPool und StyleRequestHandler
    pHitTestOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
       pHitTestOutliner->SetMinDepth(0);

    if (pOutliner)
    {
        pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        pOutliner->SetMinDepth(0);
    }
    if (pInternalOutliner)
    {
        pInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        pInternalOutliner->SetMinDepth(0);
    }

    if ( eMode == DOC_LOADED )
    {
        if (nFileFormatVersion <= 17 && nFileFormatVersion != SDIOCOMPAT_VERSIONDONTKNOW)
        {
            // Da sich die Bedeutung der SvxLRSpaceItems ab nFileFormatVersion = 16
            // geaendert hat (Bullet-Handling des Outliners), muss fuer aeltere Versionen
            // eine Korrektur vorgenommen werden.
            // Bis Version 17 gab es noch diverse Probleme
            ((SdStyleSheetPool*) pStyleSheetPool)->AdjustLRSpaceItems();
        }

        // Praesentationsobjekte muessen wieder Listener der entsprechenden
        // Vorlagen werden
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*) GetStyleSheetPool();
        SfxStyleSheet*    pSheet = NULL;
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

            if( nFileFormatVersion <= 4 )
                pPage->CreateTitleAndLayout();

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

    bNewOrLoadCompleted = TRUE;

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

    PresentationObjectList::iterator aIter( pPage->GetPresObjList().begin() );
    const PresentationObjectList::iterator aEnd( pPage->GetPresObjList().end() );
    if(aIter != aEnd)
    {
        // Listen mit Titel- und Gliederungsvorlagen erstellen
        String aName = pPage->GetLayoutName();
        aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));

        List* pOutlineList = pSPool->CreateOutlineSheetList(aName);
        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)
                                        pSPool->GetTitleSheet(aName);

        // jetzt nach Titel- und Gliederungstextobjekten suchen und
        // Objekte zu Listenern machen
        while (aIter != aEnd)
        {
            SdrObject* pObj = (*aIter).mpObject;

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
                    PresObjKind ePresObjKind = (*aIter).meKind;
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
            aIter++;
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
    if (!pOutliner && bCreateOutliner)
    {
        pOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );

        if (pDocSh)
            pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *pDocSh ) );

        pOutliner->SetDefTab( nDefaultTabulator );
        pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        pOutliner->SetMinDepth(0);
    }

    return(pOutliner);
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
    if ( !pInternalOutliner && bCreateOutliner )
    {
        pInternalOutliner = new ::sd::Outliner( this, OUTLINERMODE_TEXTOBJECT );
        // MT:
        // Dieser Outliner wird nur fuer das Erzeugen spezieller Textobjekte
        // verwendet. Da in diesen Textobjekten keine Portion-Informationen
        // gespeichert werden muessen, kann/soll der Update-Mode immer FALSE bleiben.
        pInternalOutliner->SetUpdateMode( FALSE );
        pInternalOutliner->EnableUndo( FALSE );

        if (pDocSh)
            pInternalOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *pDocSh ) );

        pInternalOutliner->SetDefTab( nDefaultTabulator );
        pInternalOutliner->SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
        pInternalOutliner->SetMinDepth(0);
    }

    DBG_ASSERT( !pInternalOutliner || ( pInternalOutliner->GetUpdateMode() == FALSE ) , "InternalOutliner: UpdateMode = TRUE !" );
    DBG_ASSERT( !pInternalOutliner || ( pInternalOutliner->IsUndoEnabled() == FALSE ), "InternalOutliner: Undo = TRUE !" );

    // MT: Wer ihn vollmuellt, macht ihn auch gleich wieder leer:
    // Vorteile:
    // a) Keine unnoetigen Clear-Aufrufe
    // b) Kein Muell im Speicher.
    DBG_ASSERT( !pInternalOutliner || ( ( pInternalOutliner->GetParagraphCount() == 1 ) && ( pInternalOutliner->GetText( pInternalOutliner->GetParagraph( 0 ) ).Len() == 0 ) ), "InternalOutliner: Nicht leer!" );

    return pInternalOutliner;
}

/*************************************************************************
|*
|* beim Starten der Praesentation den Navigator hochfahren oder nicht
|*
\************************************************************************/

void SdDrawDocument::SetStartPresWithNavigator(BOOL bStart)
{
    bStartPresWithNavigator = bStart;
}

/*************************************************************************
|*
|* in der Praesentation bei einem Klick in die Flaeche nicht die Seite wechslen
|*
\************************************************************************/

void SdDrawDocument::SetPresLockedPages(BOOL bLock)
{
    bPresLockedPages = bLock;
}

/*************************************************************************
|*
|* Praesentation immer oberstes Fenster oder nicht
|*
\************************************************************************/

void SdDrawDocument::SetPresAlwaysOnTop(BOOL bOnTop)
{
    bPresAlwaysOnTop = bOnTop;
}

/*************************************************************************
|*
|* Praesentation im Vollbild-Modus oder im Fenster
|*
\************************************************************************/

void SdDrawDocument::SetPresFullScreen(BOOL bNewFullScreen)
{
    bPresFullScreen = bNewFullScreen;
}


/*************************************************************************
|*
|* OnlineSpelling ein/aus
|*
\************************************************************************/

void SdDrawDocument::SetOnlineSpell(BOOL bIn)
{
    bOnlineSpell = bIn;
    ULONG nCntrl = 0;

    if (pOutliner)
    {
        nCntrl = pOutliner->GetControlWord();

        if (bOnlineSpell)
            nCntrl |= EE_CNTRL_ONLINESPELLING;
        else
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;

        pOutliner->SetControlWord(nCntrl);
    }

    if (pInternalOutliner)
    {
        nCntrl = pInternalOutliner->GetControlWord();

        if (bOnlineSpell)
            nCntrl |= EE_CNTRL_ONLINESPELLING;
        else
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;

        pInternalOutliner->SetControlWord(nCntrl);
    }

    ::Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (bOnlineSpell)
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;

    rOutliner.SetControlWord(nCntrl);

    if (bOnlineSpell)
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
    bHideSpell = bIn;
    ULONG nCntrl = 0;

    if (pOutliner)
    {
        nCntrl = pOutliner->GetControlWord();

        if (bHideSpell)
            nCntrl |= EE_CNTRL_NOREDLINES;
        else
            nCntrl &= ~EE_CNTRL_NOREDLINES;

        pOutliner->SetControlWord(nCntrl);
    }

    if (pInternalOutliner)
    {
        nCntrl = pInternalOutliner->GetControlWord();

        if (bHideSpell)
            nCntrl |= EE_CNTRL_NOREDLINES;
        else
            nCntrl &= ~EE_CNTRL_NOREDLINES;

        pInternalOutliner->SetControlWord(nCntrl);
    }

    ::Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (bHideSpell)
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
        if ( pDocSh )
            xModel = pDocSh->GetModel();
    }
    catch( uno::RuntimeException& e )
    {
        e;                              // to avoid a compiler warning...
    }

    return xModel;
}

SvxNumType SdDrawDocument::GetPageNumType() const
{
    return ePageNumType;
}




void SdDrawDocument::SetPrinterIndependentLayout (sal_Int32 nMode)
{
    // #108104#
    // DBG_ASSERT (pDocSh!=NULL, "No available document shell to set ref device at.");

    switch (nMode)
    {
        case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
        case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
            // Just store supported modes and inform the doc shell.
            mnPrinterIndependentLayout = nMode;

            // #108104#
            // Since it is possible that a SdDrawDocument is constructed without a
            // SdDrawDocShell the pointer member pDocSh needs to be tested
            // before the call is executed. This is e.-g. used for copy/paste.
            if(pDocSh)
            {
                pDocSh->UpdateRefDevice ();
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
