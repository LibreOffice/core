/*************************************************************************
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dl $ $Date: 2000-10-18 12:07:10 $
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

#include <svx/svxids.hrc>
#include <svx/srchitem.hxx>

#ifndef SVX_LIGHT
#ifndef _OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFA_MISCCFG_HXX
#include <sfx2/misccfg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif
#include <sfx2/app.hxx>
#include <offmgr/app.hxx>
#include <svx/linkmgr.hxx>
#include <svx/dialogs.hrc>
#include "sdoutl.hxx"
#include "app.hxx"
#else  // SVX_LIGHT
#ifndef _SVDOUTL_HXX //autogen wg. Outliner
#include <svx/svdoutl.hxx>
#endif
#define SfxPrinter Printer
#endif // !SVX_LIGHT

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
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
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
#ifndef _SFXITEMPOOL_HXX //autogen wg. SfxItemPool
#include <svtools/itempool.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#include <svx/xtable.hxx>
#ifndef _COM_SUN_STAR_LINGUISTIC_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic/XHyphenator.hpp>
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
#include <cppuhelper/extract.hxx>

#include "eetext.hxx"

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#include <unotools/charclass.hxx>

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
#ifndef SVX_LIGHT
#include "../ui/inc/docshell.hxx"
#include "../ui/inc/grdocsh.hxx"
#include "../ui/inc/dragserv.hxx"
#include "../ui/inc/viewshel.hxx"
#include "../ui/inc/grdocsh.hxx"
#include "../ui/inc/optsitem.hxx"
#endif //!SVX_LIGHT
#include "../ui/inc/frmview.hxx"
#else
#ifndef SVX_LIGHT
#include "docshell.hxx"
#include "grdocsh.hxx"
#include "sdresid.hxx"
#include "dragserv.hxx"
#include "viewshel.hxx"
#include "grdocsh.hxx"
#include "optsitem.hxx"
#endif //!SVX_LIGHT
#include "frmview.hxx"
#endif

using namespace ::com::sun::star;

TYPEINIT1( SdDrawDocument, FmFormModel );

SdDrawDocument* SdDrawDocument::pDocLockedInsertingLinks = NULL;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdDrawDocument::SdDrawDocument(DocumentType eType, SfxObjectShell* pDrDocSh) :
    FmFormModel(
#ifndef SVX_LIGHT
    SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ),
#else
    String(),
#endif
    NULL, (SvPersist*)pDrDocSh ),
    eDocType(eType),
    pDocSh( (SdDrawDocShell*) pDrDocSh ),
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
    bSdDataObj(FALSE),
    bCustomShow(TRUE),
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
#ifndef SVX_LIGHT
    bAllocDocSh(FALSE),
#endif
    pDeletedPresObjList(NULL),
    nFileFormatVersion(SDIOCOMPAT_VERSIONDONTKNOW),
    pCustomShowList(NULL)
{
    SetObjectShell(pDrDocSh);       // fuer das VCDrawModel

    if (pDocSh)
    {
        SetSwapGraphics(TRUE);
    }

#ifndef SVX_LIGHT
    // Masseinheit (von App) und Massstab (von SdMod) setzen
    INT32 nX, nY;
    SdOptions* pOptions = SD_MOD()->GetSdOptions(eDocType);
    pOptions->GetScale( nX, nY );
    SetUIUnit( (FieldUnit)pOptions->GetMetric(), Fraction( nX, nY ) );
#endif

    SetScaleUnit(MAP_100TH_MM);
    SetScaleFraction(Fraction(1, 1));
    SetDefaultFontHeight(847);     // 24p

    pItemPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pItemPool->FreezeIdRanges();
    SetTextDefaults();

    // die DrawingEngine muss auch wissen, wo er ist
    FmFormModel::SetStyleSheetPool( new SdStyleSheetPool( GetPool(), this ) );

#ifndef SVX_LIGHT
    // Language setzen
    LanguageType eLanguage = LANGUAGE_SYSTEM;

    try
    {
        uno::Reference< beans::XPropertySet > xProp( SvxGetLinguPropertySet() );
        if( xProp.is() )
        {
            sal_Int16 nValue;
            uno::Any aAny( xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultLanguage"))));

            if( aAny >>= nValue )
                eLanguage = nValue;
        }
        else
        {
            eLanguage = Application::GetAppInternational().GetLanguage();
        }
    }
    catch(...)
    {
        DBG_ERROR("No 'DefaultLanguage' property");
    }

    if (eLanguage == LANGUAGE_SYSTEM)
        eLanguage = System::GetLanguage();
    if (eLanguage == LANGUAGE_DONTKNOW)
        eLanguage = LANGUAGE_ENGLISH_US;
    SetLanguage( eLanguage );

    mpInternational = new International(eLanguage);
    String aLanguage, aCountry, aEmpty;
    ConvertLanguageToIsoNames( International::GetRealLanguage( eLanguage ), aLanguage, aCountry );
    mpLocale = new ::com::sun::star::lang::Locale( aLanguage, aCountry, aEmpty );
    mpCharClass = new CharClass( *mpLocale );

    // DefTab und SpellOptions setzen
    //OfaMiscCfg* pOfaMiscCfg = SFX_APP()->GetMiscConfig();
    // Jetzt am Modul (SD)
    USHORT nDefTab = pOptions->GetDefTab();
    SetDefaultTabulator( nDefTab );

    bHideSpell   = sal_True;
    bOnlineSpell = sal_False;
    try
    {
        uno::Reference< beans::XPropertySet > xProp( SvxGetLinguPropertySet() );
        if( xProp.is() )
        {
            bHideSpell = ::cppu::any2bool( xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsSpellHide" ))));
            bOnlineSpell = ::cppu::any2bool( xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsSpellAuto" ))));
        }
    }
    catch(...)
    {
        DBG_ERROR( "Ill. Type inside linguistic property" );
    }
#else
    SetLanguage( System::GetLanguage() );
#endif // !SVX_LIGHT

    // Dem DrawOutliner den StyleSheetPool setzen, damit Textobjekte richtig
    // eingelesen werden koennen. Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
    SdrOutliner& rOutliner = GetDrawOutliner();
    rOutliner.SetStyleSheetPool((SfxStyleSheetPool*)GetStyleSheetPool());
#ifndef SVX_LIGHT
    rOutliner.SetCalcFieldValueHdl(LINK(SFX_APP(), SdModule, CalcFieldValueHdl));
#endif // !SVX_LIGHT
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

    rOutliner.SetControlWord(nCntrl);

#ifndef SVX_LIGHT

    try
    {
        uno::Reference< linguistic::XSpellChecker1 > xSpellChecker( SvxGetSpellChecker() );
        if ( xSpellChecker.is() )
            rOutliner.SetSpeller(xSpellChecker);

        uno::Reference< linguistic::XHyphenator > xHyphenator( OFF_APP()->GetHyphenator() );
        if( xHyphenator.is() )
            rOutliner.SetHyphenator( xHyphenator );

    }
    catch(...)
    {
        DBG_ERROR("Can't get SpellChecker");
    }

    rOutliner.SetDefaultLanguage( eLanguage );

    aOldNotifyUndoActionHdl = GetNotifyUndoActionHdl();
    SetNotifyUndoActionHdl(LINK(this, SdDrawDocument, NotifyUndoActionHdl));

    if (pDocSh)
    {
        SetLinkManager( new SvxLinkManager(pDocSh) );
    }
#endif // !SVX_LIGHT


    // Dem HitTestOutliner den StyleSheetPool setzen.
    // Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
    SfxItemSet aSet2( pHitTestOutliner->GetEmptyItemSet() );
    pHitTestOutliner->SetStyleSheetPool( (SfxStyleSheetPool*)GetStyleSheetPool() );

#ifndef SVX_LIGHT
    pHitTestOutliner->SetCalcFieldValueHdl( LINK(SFX_APP(), SdModule, CalcFieldValueHdl) );
    try
    {
        uno::Reference< linguistic::XSpellChecker1 > xSpellChecker( SvxGetSpellChecker() );
        if ( xSpellChecker.is() )
            pHitTestOutliner->SetSpeller( xSpellChecker );

        uno::Reference< linguistic::XHyphenator > xHyphenator( OFF_APP()->GetHyphenator() );
        if( xHyphenator.is() )
            pHitTestOutliner->SetHyphenator( xHyphenator );
    }
    catch(...)
    {
        DBG_ERROR("Can't get SpellChecker");
    }

    pHitTestOutliner->SetDefaultLanguage( eLanguage );
#endif // !SVX_LIGHT

    ULONG nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl2 |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl2 |= EE_CNTRL_NOREDLINES;
    nCntrl2 &= ~EE_CNTRL_ONLINESPELLING;
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

__EXPORT SdDrawDocument::~SdDrawDocument()
{
    if (pWorkStartupTimer)
    {
        if ( pWorkStartupTimer->IsActive() )
            pWorkStartupTimer->Stop();

        delete pWorkStartupTimer;
        pWorkStartupTimer = NULL;
    }

#ifndef SVX_LIGHT
    StopOnlineSpelling();
    delete pOnlineSearchItem;
    pOnlineSearchItem = NULL;

    CloseBookmarkDoc();
    SetAllocDocSh(FALSE);

    SetNotifyUndoActionHdl(aOldNotifyUndoActionHdl);
#endif
    Clear();

#ifndef SVX_LIGHT
    if (pLinkManager)
    {
        // BaseLinks freigeben
        for ( USHORT n = pLinkManager->GetServers().Count(); n; )
        {
            ( (SvPseudoObject*) pLinkManager->GetServers()[ --n ])->Closed();
        }

        if ( pLinkManager->GetLinks().Count() )
        {
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
        }

        delete pLinkManager;
        pLinkManager = NULL;
    }
#endif // !SVX_LIGHT

    FrameView* pFrameView = NULL;

    for (ULONG i = 0; i < pFrameViewList->Count(); i++)
    {
        // Ggf. FrameViews loeschen
        pFrameView = (FrameView*) pFrameViewList->GetObject(i);

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
}

/*************************************************************************
|*
|* Diese Methode erzeugt ein neues Dokument (SdDrawDocument) und gibt einen
|* Zeiger darauf zurueck. Die Drawing Engine benutzt diese Methode um das
|* Dokument oder Teile davon ins Clipboard/DragServer stellen zu koennen.
|*
\************************************************************************/

SdrModel* __EXPORT SdDrawDocument::AllocModel() const
{
    SdDrawDocument* pNewModel = NULL;

#ifndef SVX_LIGHT
    if (bSdDataObj)
    {
        // Dokument wird fuer Drag&Drop erzeugt, dafuer muss dem Dokument
        // eine DocShell (SvPersist) bekannt sein

        SvEmbeddedObject* pObj = NULL;
        SdDrawDocShell* pNewDocSh = NULL;
        SdDataObject* pClipboardData = SD_MOD()->pClipboardData;
        SdDataObject* pDragData = SD_MOD()->pDragData;

        if (pDragData)
        {
            if( eDocType == DOCUMENT_TYPE_IMPRESS)
            {
                pDragData->aDocShellRef = new SdDrawDocShell(SFX_CREATE_MODE_EMBEDDED,
                                                             TRUE, eDocType);
            }
            else
            {
                pDragData->aDocShellRef = new SdGraphicDocShell(SFX_CREATE_MODE_EMBEDDED,
                                                             TRUE, eDocType);
            }

            pObj = pDragData->aDocShellRef;
            pNewDocSh = (SdDrawDocShell*) pObj;
        }
        else if (pClipboardData)
        {
            if( eDocType == DOCUMENT_TYPE_IMPRESS)
            {
                pClipboardData->aDocShellRef = new SdDrawDocShell(SFX_CREATE_MODE_EMBEDDED,
                                                                  TRUE, eDocType);
            }
            else
            {
                pClipboardData->aDocShellRef = new SdGraphicDocShell(SFX_CREATE_MODE_EMBEDDED,
                                                                  TRUE, eDocType);
            }

            pObj = pClipboardData->aDocShellRef;
            pNewDocSh = (SdDrawDocShell*) pObj;
        }

        pNewDocSh->DoInitNew(NULL);
        pNewModel = pNewDocSh->GetDoc();

        // Nur fuer Clipboard notwendig,
        // fuer Drag&Drop erfolgt dieses im DragServer
        SdStyleSheetPool* pOldStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        SdStyleSheetPool* pNewStylePool = (SdStyleSheetPool*) pNewModel->GetStyleSheetPool();

        pNewStylePool->CopyGraphicSheets(*pOldStylePool);

        for (USHORT i = 0; i < GetMasterSdPageCount(PK_STANDARD); i++)
        {
            // Alle Layouts der MasterPage mitnehmen
            String aOldLayoutName(((SdDrawDocument*) this)->GetMasterSdPage(i, PK_STANDARD)->GetName());
            pNewStylePool->CopyLayoutSheets(aOldLayoutName, *pOldStylePool);
        }

        pNewModel->NewOrLoadCompleted( DOC_LOADED );  // loaded from source document
    }
    else if (bAllocDocSh)
    {
        // Es wird eine DocShell erzeugt, welche mit GetAllocedDocSh()
        // zurueckgegeben wird

        // const as const can... (geht z.Z. nicht anders)
        SdDrawDocument* pDoc = (SdDrawDocument*) this;
        pDoc->SetAllocDocSh(FALSE);
        pDoc->xAllocedDocShRef = new SdDrawDocShell(SFX_CREATE_MODE_EMBEDDED,
                                                    TRUE, eDocType);
        pDoc->xAllocedDocShRef->DoInitNew(NULL);
        pNewModel = pDoc->xAllocedDocShRef->GetDoc();
    }
    else
#endif // !SVX_LIGHT
    {
        pNewModel = new SdDrawDocument(eDocType, NULL);
    }

    return(pNewModel);
}

/*************************************************************************
|*
|* Diese Methode erzeugt eine neue Seite (SdPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/

SdrPage* __EXPORT SdDrawDocument::AllocPage(FASTBOOL bMasterPage)
{
    return new SdPage(*this, NULL, bMasterPage);
}

/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/

SvStream& operator << (SvStream& rOut, SdDrawDocument& rDoc)
{
#ifndef SVX_LIGHT
    CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());

    /**************************************************************************
    * Aktuelle FileFormat-Versionsnummer
    * Bei Aenderugen stets inkrementieren und beim Laden beruecksichtigen!
    **************************************************************************/
    rDoc.nFileFormatVersion = 18;

    // AutoLayouts muessen ggf. erzeugt werden
    rDoc.StopWorkStartupDelay();

    // Eindeutige Namen der StandardLayer erzeugen
    rDoc.MakeUniqueLayerNames();

    rOut << (FmFormModel&) rDoc;

    // Sprachabhaengige Namen der StandardLayer wieder herstellen
    rDoc.RestoreLayerNames();

    if ( rDoc.IsStreamingSdrModel() )
    {
        // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
        // Anwendungsfall: svdraw Clipboard-Format
        return(rOut);
    }

    SdIOCompat aIO(rOut, STREAM_WRITE, rDoc.nFileFormatVersion);

    BOOL bDummy = TRUE;
    rOut << bDummy;                    // ehem. bPresentation
    rOut << rDoc.bPresAll;
    rOut << rDoc.bPresEndless;
    rOut << rDoc.bPresManual;
    rOut << rDoc.bPresMouseVisible;
    rOut << rDoc.bPresMouseAsPen;
    rOut << rDoc.nPresFirstPage;

    // Es wird nun eine Liste von FrameViews geschrieben (siehe weiter unten),
    // daher wird an dieser Stelle ein FALSE vermerkt.
    BOOL bSingleFrameView = FALSE;
    rOut << bSingleFrameView;

    /**************************************************************************
    * Frueher (StarDraw Version 3.0, File-Format Version 3) wurde hier das
    * JobSetup geschrieben, nun der Printer (binaer-kompatibel, daher wurde
    * die Version des File-Formats nicht geaendert)
    **************************************************************************/
    if (rDoc.pDocSh)
    {
        SfxPrinter* pPrinter = rDoc.pDocSh->GetPrinter(TRUE);
        pPrinter->Store(rOut);
    }
    else
    {
        // Keine DocShell, daher wird ein JobSetup geschrieben
        JobSetup aJobSetup;
        rOut << aJobSetup;
    }

    rOut << (ULONG) rDoc.GetLanguage();

    /**************************************************************************
    * FrameViews schreiben
    **************************************************************************/
    ULONG nFrameViewCount = 0;
    SdViewShell* pViewSh = NULL;
    SfxViewShell* pSfxViewSh = NULL;
    SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(rDoc.pDocSh,
                                                         TYPE(SfxTopViewFrame));

    while (pSfxViewFrame)
    {
        // Anzahl FrameViews ermitteln
        pSfxViewSh = pSfxViewFrame->GetViewShell();
        pViewSh = PTR_CAST( SdViewShell, pSfxViewSh );

        if ( pViewSh && pViewSh->GetFrameView() )
        {
            nFrameViewCount++;
        }

        pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, rDoc.pDocSh,
                                              TYPE(SfxTopViewFrame));
    }

    // Anzahl FrameViews schreiben
    rOut << nFrameViewCount;

    FrameView* pFrame = NULL;
    pViewSh = NULL;
    pSfxViewSh = NULL;
    pSfxViewFrame = SfxViewFrame::GetFirst(rDoc.pDocSh, TYPE(SfxTopViewFrame));

    while (pSfxViewFrame)
    {
        // FrameViews schreiben
        pSfxViewSh = pSfxViewFrame->GetViewShell();
        pViewSh = PTR_CAST( SdViewShell, pSfxViewSh );

        if ( pViewSh && pViewSh->GetFrameView() )
        {
            pViewSh->WriteFrameViewData();
            rOut << *pViewSh->GetFrameView();
        }

        pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, rDoc.pDocSh,
                                              TYPE(SfxTopViewFrame));
    }

    rOut << rDoc.bStartPresWithNavigator;
    rOut << rDoc.bPresLockedPages;
    rOut << rDoc.bPresAlwaysOnTop;
    rOut << rDoc.bOnlineSpell;
    rOut << rDoc.bHideSpell;
    rOut << rDoc.bPresFullScreen;
    rOut.WriteByteString( rDoc.aPresPage, eSysSet );
    rOut << rDoc.bAnimationAllowed;

    UINT16 nDocType = (UINT16) rDoc.eDocType;
    rOut << nDocType;

    // CustomShow aktiv
    rOut << rDoc.bCustomShow;

    // Anzahl CustomShows schreiben
    ULONG nCustomShowCount = 0;

    if (rDoc.pCustomShowList)
    {
        nCustomShowCount = rDoc.pCustomShowList->Count();
    }

    rOut << nCustomShowCount;

    if (rDoc.pCustomShowList)
    {
        for (ULONG i = 0; i < nCustomShowCount; i++)
        {
            // CustomShows schreiben
            SdCustomShow* pCustomShow = (SdCustomShow*) rDoc.pCustomShowList->GetObject(i);
            rOut << *pCustomShow;
        }

        // Position der aktuellen CustomShow
        ULONG nCurPos = rDoc.pCustomShowList->GetCurPos();
        rOut << nCurPos;
    }

    // ab Version 15
    rOut << (ULONG) rDoc.GetPageNumType();

    // ab Version 17
    rOut << rDoc.GetPresPause() << rDoc.IsPresShowLogo();

    // ab Version 18 (keine Aenderung)

#endif // !SVX_LIGHT
    return rOut;
}

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

SvStream& operator >> (SvStream& rIn, SdDrawDocument& rDoc)
{
    CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());

    rIn >> (FmFormModel&) rDoc;
    rDoc.GetItemPool().LoadCompleted();

    // Fehler ?
    if (rIn.GetError() != 0)
        return (rIn);

    if ( rDoc.IsStreamingSdrModel() )
    {
        // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
        // Anwendungsfall: svdraw Clipboard-Format
        return(rIn);
    }

    SdIOCompat aIO(rIn, STREAM_READ);

    BOOL bDummy;
    rIn >> bDummy;                     // ehem. bPresentation
    rIn >> rDoc.bPresAll;
    rIn >> rDoc.bPresEndless;
    rIn >> rDoc.bPresManual;
    rIn >> rDoc.bPresMouseVisible;
    rIn >> rDoc.bPresMouseAsPen;
    rIn >> rDoc.nPresFirstPage;

    rDoc.nFileFormatVersion = aIO.GetVersion();

    if (rDoc.nFileFormatVersion >= 1)
    {
        // Daten der Versionen >= 1 einlesen

        BOOL bSingleFrameView;
        rIn >> bSingleFrameView;

        if (bSingleFrameView)
        {
            FrameView * pFrameView = new FrameView( &rDoc );
            rIn >> *pFrameView;
            rDoc.pFrameViewList->Insert(pFrameView, LIST_APPEND);

            // Fehler ?
            if (rIn.GetError() != 0)
                return (rIn);
        }
    }

    if (rDoc.nFileFormatVersion >= 2)
    {
        // Daten der Versionen >= 2 einlesen

        /******************************************************************
        * Frueher (StarDraw Version 3.0, File-Format Version 3) wurde hier
        * das JobSetup eingelesen, nun wird der Printer erzeugt
        * (binaer-kompatibel)
        *******************************************************************/
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( rDoc.GetPool(),
                        SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                        ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                        0 );
        // PrintOptionsSet setzen
#ifndef SVX_LIGHT
        SdOptionsPrintItem aPrintItem(ATTR_OPTIONS_PRINT
                                      ,SD_MOD()->GetSdOptions(rDoc.eDocType)
                                      );

        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        USHORT      nFlags = 0;

        nFlags =  (aPrintItem.IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        SfxPrinter* pPrinter = SfxPrinter::Create(rIn, pSet);

        MapMode aMM (pPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMM);
        if (rDoc.pDocSh)            // z. B. nicht bei "Einfuegen-Datei"
            rDoc.pDocSh->SetPrinter(pPrinter);
        else
            delete pPrinter;
#else
        JobSetup aFileJobSetup;
        rIn >> aFileJobSetup;
#endif
    }

    if (rDoc.nFileFormatVersion >= 3)
    {
        ULONG nTmp;
        rIn >> nTmp;
        rDoc.SetLanguage( (LanguageType) nTmp );
    }

    if (rDoc.nFileFormatVersion >= 4)
    {
        /**********************************************************************
        * FrameViews lesen
        **********************************************************************/
        ULONG nCount = 0;
        FrameView* pFrameView = NULL;

        for (nCount=0; nCount<rDoc.pFrameViewList->Count(); nCount++)
        {
            // Ggf. FrameViews loeschen
            pFrameView = (FrameView*) rDoc.pFrameViewList->GetObject(nCount);

            if (pFrameView)
                delete pFrameView;
        }

        rDoc.pFrameViewList->Clear();

        // Anzahl FrameViews lesen
#ifndef SVX_LIGHT
        const SvtSaveOptions aOptions;
        BOOL bIsSaveDocView = aOptions.IsSaveDocView();
#else
        BOOL bIsSaveDocView = FALSE;
#endif // !SVX_LIGHT

        ULONG nFrameViewCount = 0;
        rIn >> nFrameViewCount;

        for (nCount=0; nCount<nFrameViewCount; nCount++)
        {
            // Einzelne FrameViews lesen
            pFrameView = new FrameView( &rDoc );
            rIn >> *pFrameView;

            if (bIsSaveDocView)
            {
                // FrameViews werden fuer die ViewShell gebraucht
                // Die FrameView gehoert nun der Liste
                rDoc.pFrameViewList->Insert(pFrameView, nCount);
            }
            else
            {
                // FrameView kann wieder geloescht werden
                delete pFrameView;
            }

            // Fehler ?
            if (rIn.GetError() != 0)
                return (rIn);
        }
    }

    if (rDoc.nFileFormatVersion >= 5)
    {
        rIn >> rDoc.bStartPresWithNavigator;
    }

    if (rDoc.nFileFormatVersion >= 6)
    {
        rIn >> rDoc.bPresLockedPages;
    }

    if (rDoc.nFileFormatVersion >= 7)
    {
        rIn >> rDoc.bPresAlwaysOnTop;
    }

    if (rDoc.nFileFormatVersion >= 8)
    {
        rIn >> rDoc.bOnlineSpell;
        rIn >> rDoc.bHideSpell;
    }

    if (rDoc.nFileFormatVersion >= 9)
    {
        rIn >> rDoc.bPresFullScreen;
    }

    if (rDoc.nFileFormatVersion >= 10)
    {
        rIn.ReadByteString( rDoc.aPresPage, eSysSet );
    }

    if (rDoc.nFileFormatVersion >= 11)
    {
        rIn >> rDoc.bAnimationAllowed;
    }

    if (rDoc.nFileFormatVersion >= 12)
    {
        UINT16 nDocType;
        rIn >> nDocType;
#ifndef SVX_LIGHT
        rDoc.eDocType = (DocumentType) nDocType;
        // existiert eine DocShell bestimmt diese den DocType
        if(rDoc.pDocSh)
        {
            if(NULL != PTR_CAST(SdGraphicDocShell,rDoc.pDocSh))
                rDoc.eDocType = DOCUMENT_TYPE_DRAW;
            else
                rDoc.eDocType = DOCUMENT_TYPE_IMPRESS;
        }
#else
        rDoc.eDocType = DOCUMENT_TYPE_IMPRESS;
#endif // !SVX_LIGHT
    }

    if (rDoc.nFileFormatVersion >= 13)
    {
        // Keine Aenderung
    }

    if (rDoc.nFileFormatVersion >= 14)
    {
        // CustomShow aktiv
        rIn >> rDoc.bCustomShow;

        ULONG nCustomShowCount = 0;
        rIn >> nCustomShowCount;

        if (nCustomShowCount > 0)
        {
            // Liste erzeugen
            rDoc.GetCustomShowList(TRUE);
            rDoc.pCustomShowList->Clear();

            for (ULONG i = 0; i < nCustomShowCount; i++)
            {
                // Einzelne CustomShows lesen
                SdCustomShow* pCustomShow = new SdCustomShow(&rDoc);
                rIn >> *pCustomShow;

                // Die CustomShows gehoert nun der Liste
                rDoc.pCustomShowList->Insert(pCustomShow, i);

                // Fehler ?
                if (rIn.GetError() != 0)
                    return (rIn);
            }

            // Aktuelle CustomShow selektieren
            ULONG nCurPos;
            rIn >> nCurPos;
            rDoc.pCustomShowList->Seek(nCurPos);
        }
    }

    if (rDoc.nFileFormatVersion >= 15)
    {
        ULONG nTmp;
        rIn >> nTmp;
        rDoc.SetPageNumType( (SvxNumType) nTmp );
    }

    if (rDoc.nFileFormatVersion >= 17)
    {
        ULONG   nPauseSec;
        BOOL    bShowLogo;

        rIn >> nPauseSec >> bShowLogo;
        rDoc.SetPresPause( nPauseSec );
        rDoc.SetPresShowLogo( bShowLogo );
    }
    else
        rDoc.SetPresPause( 0 );

    if (rDoc.nFileFormatVersion >= 18)
    {
        // Keine Aenderung
    }

    /**************************************************************************
    * So machts der Writer, und so muessen es alle machen:
    * Bug 9714: Der CharSet an den Fonts muss geaendert werden, wenn
    * es der globale CharSet ist (MT)
    **************************************************************************/
    SfxItemPool& rPool = rDoc.GetItemPool();
    USHORT nMaxItems = rPool.GetItemCount(EE_CHAR_FONTINFO);
    SvxFontItem* pItem;
    CharSet eSrcSet = ((SdPage*) rDoc.GetPage(0))->GetCharSet();

    for (USHORT n = 0; n < nMaxItems; ++n)
    {
        pItem = (SvxFontItem*) rPool.GetItem(EE_CHAR_FONTINFO, n);
        if (pItem && pItem->GetCharSet() == eSrcSet)
        {
            pItem->GetCharSet() = eSysSet;
        }
    }

    return rIn;
}

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

void __EXPORT SdDrawDocument::SetChanged(FASTBOOL bFlag)
{
#ifndef SVX_LIGHT
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
#endif // !SVX_LIGHT
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
#ifndef SVX_LIGHT
    if (eMode == NEW_DOC)
    {
        // Neues Dokument:
        // Praesentations- und Standardvorlagen erzeugen,
        // Pool fuer virtuelle Controls erzeugen
        CreateLayoutTemplates();

        ((SdStyleSheetPool*)pStyleSheetPool)->CreatePseudosIfNecessary();
    }
    else if (eMode == DOC_LOADED)
#endif // !SVX_LIGHT
    {
        // Dokument wurde geladen:

#ifndef SVX_LIGHT
        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveUnnessesaryMasterPages( NULL, TRUE, FALSE );

        for ( ULONG i = 0; i < GetPageCount(); i++ )
        {
            // Check for correct layout names
            SdPage* pPage = (SdPage*) GetPage( i );
            if( pPage->GetMasterPageCount() > 0 )
            {
                SdPage* pMaster = (SdPage*) pPage->GetMasterPage( 0 );
                if( pMaster && pMaster->GetLayoutName() != pPage->GetLayoutName() )
                    pPage->SetLayoutName( pMaster->GetLayoutName() );
            }
        }

        // check for correct background shape position
        // previous version may moved other shapes behind the background
        // shape. For performance reason this has to be fixed because
        // the background shape should always have position 0
        for ( ULONG nPage = 0; nPage < GetMasterSdPageCount( PK_STANDARD ); nPage++)
        {
            SdPage* pPage = GetMasterSdPage( nPage, PK_STANDARD );
            SdrObject* pPresObj = pPage->GetPresObj( PRESOBJ_BACKGROUND ) ;

            if (pPresObj && pPresObj->GetOrdNum() != 0 )
                pPage->NbcSetObjectOrdNum(pPresObj->GetOrdNum(),0);
        }

        // Sprachabhaengige Namen der StandardLayer erzeugen
        RestoreLayerNames();
#endif

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
    Outliner& rDrawOutliner = GetDrawOutliner();
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

        // Standard- und Notizseiten:
        for (USHORT nPage = 0; nPage < GetPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetPage(nPage);
            SdrObjListIter aIter( *pPage );
            while( aIter.IsMore() )
            {
                OutlinerParaObject* pOPO = aIter.Next()->GetOutlinerParaObject();
                if( pOPO )
                {
                    if( pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );

                    pOPO->FinishLoad( pSPool );
                }
            }

            List* pPresObjList = pPage->GetPresObjList();
            ULONG nObjCount = pPresObjList->Count();
            if (nObjCount)
            {
                // Listen mit Titel- und Gliederungsvorlagen erstellen
                String aName = pPage->GetLayoutName();
                aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));

                List* pOutlineList = pSPool->CreateOutlineSheetList(aName);
                SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)
                                                pSPool->GetTitleSheet(aName);

                // jetzt nach Titel- und Gliederungstextobjekten suchen und
                // Objekte zu Listenern machen
                SdrAttrObj* pObj = (SdrAttrObj*)pPresObjList->First();
                while (pObj)
                {
                    if (pObj->GetObjInventor() == SdrInventor)
                    {
                        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                        SdPage* pPage = (SdPage*) pObj->GetPage();
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
                                pSheet = (SfxStyleSheet*)pOutlineList->GetObject(nSheet);
                                if (pSheet)
                                {
                                    pObj->StartListening(*pSheet);

                                    if( nSheet == 0)
                                        // Textrahmen hoert auf StyleSheet der Ebene1
                                        pObj->NbcSetStyleSheet(pSheet, TRUE);
                                }
                            }
                        }

#ifndef SVX_LIGHT
                        if (pObj->ISA(SdrTextObj) && pObj->IsEmptyPresObj() && pPage)
                        {
                            PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                            String aString = pPage->GetPresObjText(ePresObjKind);

                            if (aString.Len())
                            {
                                SdOutliner* pInternalOutl = GetInternalOutliner(TRUE);
                                pInternalOutl->SetMinDepth(0);
                                pPage->SetObjText( (SdrTextObj*) pObj, pInternalOutl, ePresObjKind, aString );
                                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), TRUE );
                                pInternalOutl->Clear();
                            }
                        }
#endif // !SVX_LIGHT
                    }
                    pObj = (SdrAttrObj*)pPresObjList->Next();
                }

                delete pOutlineList;
            }
        }

        // Masterpages:
        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            SdPage* pPage = (SdPage*)GetMasterPage(nPage);
            SdrObjListIter aIter( *pPage );
            while( aIter.IsMore() )
            {
                OutlinerParaObject* pOPO = aIter.Next()->GetOutlinerParaObject();
                if( pOPO )
                {
                    if( pOPO->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
                        pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );

                    pOPO->FinishLoad( pSPool );
                }
            }

            // BackgroundObjekt vor Selektion schuetzen #62144#
            SdrObject* pBackObj = pPage->GetPresObj(PRESOBJ_BACKGROUND);
            if(pBackObj)
                pBackObj->SetMarkProtect(TRUE);

            List* pPresObjList = pPage->GetPresObjList();
            ULONG nObjCount = pPresObjList->Count();
            if (nObjCount)
            {
                // Listen mit Titel- und Gliederungsvorlagen erstellen
                String aName = pPage->GetLayoutName();
                aName.Erase(aName.SearchAscii( SD_LT_SEPARATOR ));

                List* pOutlineList = pSPool->CreateOutlineSheetList(aName);
                SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)
                                                pSPool->GetTitleSheet(aName);

                // jetzt nach Titel- und Gliederungstextobjekten suchen und
                // Objekte zu Listenern machen
                SdrAttrObj* pObj = (SdrAttrObj*)pPresObjList->First();
                while (pObj)
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
                                pSheet = (SfxStyleSheet*)pOutlineList->GetObject(nSheet);
                                if (pSheet)
                                {
                                    pObj->StartListening(*pSheet);

                                    if( nSheet == 0)
                                        // Textrahmen hoert auf StyleSheet der Ebene1
                                        pObj->NbcSetStyleSheet(pSheet, TRUE);
                                }
                            }
                        }

#ifndef SVX_LIGHT
                        SdPage* pPage = (SdPage*) pObj->GetPage();

                        if (pObj->ISA(SdrTextObj) && pObj->IsEmptyPresObj() && pPage)
                        {
                            PresObjKind ePresObjKind = pPage->GetPresObjKind(pObj);
                            String aString = pPage->GetPresObjText(ePresObjKind);

                            if (aString.Len())
                            {
                                SdOutliner* pInternalOutl = GetInternalOutliner(TRUE);
                                pInternalOutl->SetMinDepth(0);
                                pPage->SetObjText( (SdrTextObj*) pObj, pInternalOutl, ePresObjKind, aString );
                                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), TRUE );
                                pInternalOutl->Clear();
                            }
                        }
#endif // !SVX_LIGHT
                    }
                    pObj = (SdrAttrObj*)pPresObjList->Next();
                }

                delete pOutlineList;
            }
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

#ifndef SVX_LIGHT
    if ( !pDocLockedInsertingLinks && pLinkManager && pLinkManager->GetLinks().Count() )
    {
        pDocLockedInsertingLinks = this; // lock inserting links. only links in this document should by resolved

        pLinkManager->UpdateAllLinks();  // query box: update all links?

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;  // unlock inserting links
    }

#endif // !SVX_LIGHT

    SetChanged( FALSE );
}



/*************************************************************************
|*
|* Lokaler Outliner, welcher fuer den Gliederungsmodus verwendet wird
|* In diesen Outliner werden ggf. OutlinerViews inserted!
|*
\************************************************************************/

SdOutliner* SdDrawDocument::GetOutliner(BOOL bCreateOutliner)
{
    if (!pOutliner && bCreateOutliner)
    {
#ifndef SVX_LIGHT
        pOutliner = new SdOutliner( this, OUTLINERMODE_TEXTOBJECT );
#else
        pOutliner = new Outliner( &GetItemPool(), OUTLINERMODE_TEXTOBJECT );
#endif

        if (pDocSh)
        {
            pOutliner->SetRefDevice( pDocSh->GetPrinter(TRUE) );
        }
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

SdOutliner* SdDrawDocument::GetInternalOutliner(BOOL bCreateOutliner)
{
    if ( !pInternalOutliner && bCreateOutliner )
    {
#ifndef SVX_LIGHT
        pInternalOutliner = new SdOutliner( this, OUTLINERMODE_TEXTOBJECT );
#else
        pInternalOutliner = new Outliner( &GetItemPool(), OUTLINERMODE_TEXTOBJECT );
#endif
        // MT:
        // Dieser Outliner wird nur fuer das Erzeugen spezieller Textobjekte
        // verwendet. Da in diesen Textobjekten keine Portion-Informationen
        // gespeichert werden muessen, kann/soll der Update-Mode immer FALSE bleiben.
        pInternalOutliner->SetUpdateMode( FALSE );
        pInternalOutliner->EnableUndo( FALSE );

        if (pDocSh)
        {
            pInternalOutliner->SetRefDevice( pDocSh->GetPrinter(TRUE) );
        }
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

#ifndef SVX_LIGHT
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

    Outliner& rOutliner = GetDrawOutliner();

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
#endif


/*************************************************************************
|*
|* OnlineSpelling: Markierung ein/aus
|*
\************************************************************************/

#ifndef SVX_LIGHT
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

    Outliner& rOutliner = GetDrawOutliner();

    nCntrl = rOutliner.GetControlWord();

    if (bHideSpell)
       nCntrl |= EE_CNTRL_NOREDLINES;
    else
       nCntrl &= ~EE_CNTRL_NOREDLINES;

    rOutliner.SetControlWord(nCntrl);
}
#endif


