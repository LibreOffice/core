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

#define ITEMID_SEARCH           SID_SEARCH_ITEM

#include <bf_sd/sdmod.hxx>

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <bf_sfx2/objsh.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>

#include <bf_svx/svxids.hrc>
#include <bf_svx/dlgutil.hxx>

#include <bf_sfx2/printer.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_offmgr/app.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svx/dialogs.hrc>
#include "sdoutl.hxx"

#include <bf_svx/eeitem.hxx>
#include <bf_svx/eeitemid.hxx>
#include <bf_svx/editstat.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svtools/flagitem.hxx>
#include <bf_svx/svdoattr.hxx>
#include <bf_svx/svdotext.hxx>
#include <bf_svx/bulitem.hxx>
#include <bf_svx/numitem.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/unolingu.hxx>
#include <bf_svtools/itempool.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <bf_svx/xtable.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <bf_svx/outlobj.hxx>
#include <bf_svtools/saveopt.hxx>
#include <comphelper/extract.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/lingucfg.hxx>
#include <bf_svtools/linguprops.hxx>

#include "bf_sd/frmview.hxx"
#include "eetext.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "sdresid.hxx"
#include "cusshow.hxx"

#include "bf_sd/docshell.hxx"
#include "bf_sd/grdocsh.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002

#include <tools/tenccvt.hxx>

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

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
    bCustomShow(false),
    nPresFirstPage(1),
    pOutliner(NULL),
    pInternalOutliner(NULL),
    ePageNumType(SVX_ARABIC),
    bNewOrLoadCompleted(FALSE),
    pOnlineSpellingTimer(NULL),
    pOnlineSpellingList(NULL),
    bInitialOnlineSpellingEnabled(TRUE),
    bHasOnlineSpellErrors(FALSE),
    mpLocale(NULL),
    mpCharClass(NULL),
    bAllocDocSh(FALSE),
    pDeletedPresObjList(NULL),
    nFileFormatVersion(SDIOCOMPAT_VERSIONDONTKNOW),
    pDocStor(NULL),
    pCustomShowList(NULL),
    eLanguage( LANGUAGE_SYSTEM ),
    eLanguageCJK( LANGUAGE_SYSTEM ),
    eLanguageCTL( LANGUAGE_SYSTEM ),
    mbStartWithPresentation( false )
{
    SetObjectShell(pDrDocSh);		// fuer das VCDrawModel

    if (pDocSh)
    {
        SetSwapGraphics(TRUE);
    }

        SetUIUnit( GetModuleFieldUnit(), Fraction( 1, 1 ) );	// default

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
        SvtLinguOptions			aOptions;
        aLinguConfig.GetOptions( aOptions );

        SetLanguage( aOptions.nDefaultLanguage, EE_CHAR_LANGUAGE );
        SetLanguage( aOptions.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK );
        SetLanguage( aOptions.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL );

        bOnlineSpell = aOptions.bIsSpellAuto;
        bHideSpell = aOptions.bIsSpellHideMarkings;
    }

    LanguageType eRealLanguage = MsLangId::getRealLanguage( eLanguage );

    mpLocale = new ::com::sun::star::lang::Locale( MsLangId::convertLanguageToLocale( eRealLanguage ));
    mpCharClass = new CharClass( *mpLocale );

    // If the current application language is a language that uses right-to-left text...
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguage();
    if( MsLangId::isRightToLeft( eRealCTLLanguage ) )
    {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // ... then we have to set this as a default
    }

    SetDefaultTabulator( 1250 );

    rOutliner.SetDefaultLanguage( Application::GetSettings().GetLanguage() );

    if (pDocSh)
        SetLinkManager( new SvxLinkManager(pDrDocSh) );

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
        SetSummationOfParagraphs( sal_False );
    rOutliner.SetControlWord(nCntrl);

    SetPrinterIndependentLayout(1);

    // Dem HitTestOutliner den StyleSheetPool setzen.
    // Der Link zum StyleRequest-Handler des
    // Dokuments wird erst in NewOrLoadCompleted gesetzt, da erst dann alle
    // Vorlagen existieren.
    SfxItemSet aSet2( pHitTestOutliner->GetEmptyItemSet() );
    pHitTestOutliner->SetStyleSheetPool( (SfxStyleSheetPool*)GetStyleSheetPool() );

    pHitTestOutliner->SetCalcFieldValueHdl( LINK(SD_MOD(), SdModule, CalcFieldValueHdl) );


    pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguage() );

    ULONG nCntrl2 = pHitTestOutliner->GetControlWord();
    nCntrl2 |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl2 |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl2 |= EE_CNTRL_NOREDLINES;
    nCntrl2 &= ~EE_CNTRL_ONLINESPELLING;

    nCntrl2 &= ~ EE_CNTRL_ULSPACESUMMATION;
    pHitTestOutliner->SetControlWord( nCntrl2 );

    {
        String aControlLayerName( RTL_CONSTASCII_USTRINGPARAM( "LAYER_CONTROLS" ) );

        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        rLayerAdmin.NewLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_LAYOUT" )) );
        rLayerAdmin.NewLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )) );
        rLayerAdmin.NewLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )) );
        rLayerAdmin.NewLayer( aControlLayerName );
        rLayerAdmin.NewLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_MEASURELINES" )) );

        rLayerAdmin.SetControlLayerName(aControlLayerName);
    }

    pFrameViewList = new List();
}

SdDrawDocument::~SdDrawDocument()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    CloseBookmarkDoc();
    SetAllocDocSh(FALSE);

    Clear();

    if (pLinkManager)
    {
        if ( pLinkManager->GetLinks().Count() )
        {
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
        }

        delete pLinkManager;
        pLinkManager = NULL;
    }

    FrameView* pFrameView = NULL;

    for (ULONG i = 0; i < pFrameViewList->Count(); i++)
    {
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

    delete mpLocale;
    mpLocale = NULL;

    delete mpCharClass;
    mpCharClass = NULL;
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

SvStream& operator << (SvStream& rOut, SdDrawDocument& rDoc)
{
    // #90477# CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());
    CharSet eSysSet = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());

    /**************************************************************************
    * Aktuelle FileFormat-Versionsnummer
    * Bei Aenderugen stets inkrementieren und beim Laden beruecksichtigen!
    **************************************************************************/
    rDoc.nFileFormatVersion = 18;

    rOut << (FmFormModel&) rDoc;

    if ( rDoc.IsStreamingSdrModel() )
    {
        // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
        // Anwendungsfall: svdraw Clipboard-Format
        return(rOut);
    }

    SdIOCompat aIO(rOut, STREAM_WRITE, rDoc.nFileFormatVersion);

    BOOL bDummy = TRUE;
    rOut << bDummy; 				   // ehem. bPresentation
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

    rOut << (sal_uInt32) rDoc.eLanguage;

    /**************************************************************************
    * FrameViews schreiben
    **************************************************************************/
    sal_uInt32 nFrameViewCount = 0;

    // Anzahl FrameViews schreiben
    rOut << nFrameViewCount;


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
    sal_uInt32 nCustomShowCount = 0;

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
        sal_uInt32 nCurPos = rDoc.pCustomShowList->GetCurPos();
        rOut << nCurPos;
    }

    // ab Version 15
    rOut << (sal_uInt32) rDoc.GetPageNumType();

    // ab Version 17
    rOut << rDoc.GetPresPause() << rDoc.IsPresShowLogo();

    // ab Version 18 (keine Aenderung)

    return rOut;
}

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

SvStream& operator >> (SvStream& rIn, SdDrawDocument& rDoc)
{
    // #90477# CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());
    CharSet eSysSet = GetSOLoadTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rIn.GetVersion());

    rIn >> (FmFormModel&) rDoc;
    rDoc.GetItemPool().LoadCompleted();
    rDoc.SetTextDefaults();		// overwrites loaded pool defaults

    // Turn off printer independent layout (make it printer *dependent*) for
    // pre-6.0 documents.
    rDoc.SetPrinterIndependentLayout (
        ::com::sun::star::document::PrinterIndependentLayout::DISABLED);

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
    rIn >> bDummy;					   // ehem. bPresentation
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
        SfxPrinter* pPrinter = SdDrawDocShell::CreatePrinter(rIn,rDoc);
        MapMode aMM (pPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMM);
        if (rDoc.pDocSh)			// z. B. nicht bei "Einfuegen-Datei"
            rDoc.pDocSh->SetPrinter(pPrinter);
        else
            delete pPrinter;
    }

    if (rDoc.nFileFormatVersion >= 3)
    {
        sal_uInt32 nTmp;
        rIn >> nTmp;
        rDoc.SetLanguage( (LanguageType) nTmp, EE_CHAR_LANGUAGE );
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
        const SvtSaveOptions aOptions;
        BOOL bIsSaveDocView = aOptions.IsSaveDocView();

        sal_uInt32 nFrameViewCount = 0;
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
    }

    if (rDoc.nFileFormatVersion >= 13)
    {
        // Keine Aenderung
    }

    if (rDoc.nFileFormatVersion >= 14)
    {
        // CustomShow aktiv
        rIn >> rDoc.bCustomShow;

        sal_uInt32 nCustomShowCount = 0;
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
            sal_uInt32 nCurPos;
            rIn >> nCurPos;
            rDoc.pCustomShowList->Seek(nCurPos);
        }
    }

    if (rDoc.nFileFormatVersion >= 15)
    {
        sal_uInt32 nTmp;
        rIn >> nTmp;
        rDoc.SetPageNumType( (SvxNumType) nTmp );
    }

    if (rDoc.nFileFormatVersion >= 17)
    {
        sal_uInt32 nPauseSec;
        BOOL	bShowLogo;

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
    bPresAll	 = bNewPresAll;
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


/*************************************************************************
|*
|* SetChanged(), das Model wurde geaendert
|*
\************************************************************************/

void SdDrawDocument::SetChanged(FASTBOOL bFlag)
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

void SdDrawDocument::NbcSetChanged(FASTBOOL bFlag)
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
        RestoreLayerNames();

        CheckMasterPages();

        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveDuplicateMasterPages();

        for ( USHORT i = 0; i < GetPageCount(); i++ )
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
                    pPage->GetPresObjList()->Remove(pPresObj);
                    delete pPresObj;
                }
            }
        }

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
        SfxStyleSheet*	  pSheet = NULL;
        USHORT nPage, nPageCount;

        // #96323# create missing layout style sheets for broken documents
        //		   that where created with the 5.2
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

    UpdateAllLinks();

    SetChanged( FALSE );
}

/** updates all links, only links in this document should by resolved */
void SdDrawDocument::UpdateAllLinks()
{
    if ( !pDocLockedInsertingLinks && pLinkManager && pLinkManager->GetLinks().Count() )
    {
        pDocLockedInsertingLinks = this;            // lock inserting links. only links in this document should by resolved

      pLinkManager->UpdateAllLinks(TRUE, FALSE);

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;        // unlock inserting links
    }
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
        pOutliner = new SdOutliner( this, OUTLINERMODE_TEXTOBJECT );

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

SdOutliner* SdDrawDocument::GetInternalOutliner(BOOL bCreateOutliner)
{
    if ( !pInternalOutliner && bCreateOutliner )
    {
         pInternalOutliner = new SdOutliner( this, OUTLINERMODE_TEXTOBJECT );
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


uno::Reference< uno::XInterface > SdDrawDocument::createUnoModel()
{
    uno::Reference< uno::XInterface > xModel;

    try
    {
        xModel = pDocSh->GetModel();
    }
    catch( uno::RuntimeException& e )
    {
        e;	                            // to avoid a compiler warning...
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
