/*************************************************************************
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.61 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-27 13:29:26 $
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

#ifndef _OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFA_MISCCFG_HXX
#include <sfx2/misccfg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
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
#ifndef _XCEPTION_HXX_
#include <vos/xception.hxx>
#endif
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
#include "../ui/inc/docshell.hxx"
#include "../ui/inc/grdocsh.hxx"
#include "../ui/inc/sdxfer.hxx"
#include "../ui/inc/viewshel.hxx"
#include "../ui/inc/grdocsh.hxx"
#include "../ui/inc/optsitem.hxx"
#include "../ui/inc/frmview.hxx"
#else
#include "docshell.hxx"
#include "grdocsh.hxx"
#include "sdresid.hxx"
#include "sdxfer.hxx"
#include "viewshel.hxx"
#include "grdocsh.hxx"
#include "optsitem.hxx"
#include "frmview.hxx"
#endif

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

//////////////////////////////////////////////////////////////////////////////
// #109538#

void ImpPageListWatcher::ImpRecreateSortedPageListOnDemand()
{
    // clear vectors
    maPageVectorStandard.clear();
    maPageVectorNotes.clear();
    mpHandoutPage = 0L;

    // build up vectors again
    const sal_uInt32 nPageCount(ImpGetPageCount());

    for(sal_uInt32 a(0L); a < nPageCount; a++)
    {
        SdPage* pCandidate = ImpGetPage(a);
        DBG_ASSERT(pCandidate, "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Invalid PageList in Model (!)");

        switch(pCandidate->GetPageKind())
        {
            case PK_STANDARD:
            {
                maPageVectorStandard.push_back(pCandidate);
                break;
            }
            case PK_NOTES:
            {
                maPageVectorNotes.push_back(pCandidate);
                break;
            }
            case PK_HANDOUT:
            {
                DBG_ASSERT(!mpHandoutPage, "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Two Handout pages in PageList of Model (!)");
                mpHandoutPage = pCandidate;
                break;
            }
        }
    }

    // set to valid
    mbPageListValid = sal_True;
}

ImpPageListWatcher::ImpPageListWatcher(const SdrModel& rModel)
:   mrModel(rModel),
    mpHandoutPage(0L),
    mbPageListValid(sal_False)
{
}

ImpPageListWatcher::~ImpPageListWatcher()
{
}

SdPage* ImpPageListWatcher::GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum)
{
    SdPage* pRetval(0L);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PK_STANDARD:
        {
            DBG_ASSERT(nPgNum <= maPageVectorStandard.size(), "ImpPageListWatcher::GetSdPage: access out of range (!)");
            pRetval = maPageVectorStandard[nPgNum];
            break;
        }
        case PK_NOTES:
        {
            DBG_ASSERT(nPgNum <= maPageVectorNotes.size(), "ImpPageListWatcher::GetSdPage: access out of range (!)");
            pRetval = maPageVectorNotes[nPgNum];
            break;
        }
        case PK_HANDOUT:
        {
            DBG_ASSERT(mpHandoutPage, "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            DBG_ASSERT(nPgNum == 0L, "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            pRetval = mpHandoutPage;
            break;
        }
    }

    return pRetval;
}

sal_uInt32 ImpPageListWatcher::GetSdPageCount(PageKind ePgKind)
{
    sal_uInt32 nRetval(0L);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PK_STANDARD:
        {
            nRetval = maPageVectorStandard.size();
            break;
        }
        case PK_NOTES:
        {
            nRetval = maPageVectorNotes.size();
            break;
        }
        case PK_HANDOUT:
        {
            if(mpHandoutPage)
            {
                nRetval = 1L;
            }

            break;
        }
    }

    return nRetval;
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt32 ImpDrawPageListWatcher::ImpGetPageCount() const
{
    return (sal_uInt32)mrModel.GetPageCount();
}

SdPage* ImpDrawPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return (SdPage*)mrModel.GetPage((sal_uInt16)nIndex);
}

ImpDrawPageListWatcher::ImpDrawPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpDrawPageListWatcher::~ImpDrawPageListWatcher()
{
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt32 ImpMasterPageListWatcher::ImpGetPageCount() const
{
    return (sal_uInt32)mrModel.GetMasterPageCount();
}

SdPage* ImpMasterPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return (SdPage*)mrModel.GetMasterPage((sal_uInt16)nIndex);
}

ImpMasterPageListWatcher::ImpMasterPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpMasterPageListWatcher::~ImpMasterPageListWatcher()
{
}

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
    NULL, (SvPersist*)pDrDocSh ),
    eDocType(eType),
    pDocSh( (SdDrawDocShell*) pDrDocSh ),
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
    pDocStor(NULL),
    pCustomShowList(NULL),
    eLanguage( LANGUAGE_SYSTEM ),
    eLanguageCJK( LANGUAGE_SYSTEM ),
    eLanguageCTL( LANGUAGE_SYSTEM ),
    mbStartWithPresentation( false ),
    // #109538#
    mpDrawPageListWatcher(0L),
    mpMasterPageListWatcher(0L)
{
    // #109538#
    mpDrawPageListWatcher = new ImpDrawPageListWatcher(*this);
    mpMasterPageListWatcher = new ImpMasterPageListWatcher(*this);

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

    // DefTab und SpellOptions setzen
    //OfaMiscCfg* pOfaMiscCfg = SFX_APP()->GetMiscConfig();
    // Jetzt am Modul (SD)
    USHORT nDefTab = pOptions->GetDefTab();
    SetDefaultTabulator( nDefTab );

    TRY
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            rOutliner.SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            rOutliner.SetHyphenator( xHyphenator );

        SetForbiddenCharsTable( new SvxForbiddenCharactersTable( ::comphelper::getProcessServiceFactory() ) );
    }
    CATCH_ALL()
    {
        DBG_ERROR("Can't get SpellChecker");
    }
    END_CATCH

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

    TRY
    {
        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            pHitTestOutliner->SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            pHitTestOutliner->SetHyphenator( xHyphenator );
    }
    CATCH_ALL()
    {
        DBG_ERROR("Can't get SpellChecker");
    }
    END_CATCH

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
    Clear();

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

    // #109538#
    delete mpDrawPageListWatcher;
    mpDrawPageListWatcher = 0L;

    // #109538#
    delete mpMasterPageListWatcher;
    mpMasterPageListWatcher = 0L;
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
        SvEmbeddedObject*   pObj = NULL;
        SdDrawDocShell*     pNewDocSh = NULL;

        if( eDocType == DOCUMENT_TYPE_IMPRESS )
            pCreatingTransferable->SetDocShell( new SdDrawDocShell( SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType ) );
        else
            pCreatingTransferable->SetDocShell( new SdGraphicDocShell( SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType ) );

        pNewDocSh = (SdDrawDocShell*) ( pObj = pCreatingTransferable->GetDocShell() );
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
        pDoc->xAllocedDocShRef = new SdDrawDocShell(SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType);
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

SvStream& operator << (SvStream& rOut, SdDrawDocument& rDoc)
{
    // #90477# CharSet eSysSet = ::GetStoreCharSet( gsl_getSystemTextEncoding());
    CharSet eSysSet = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());

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

    rOut << (ULONG) rDoc.eLanguage;

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
    rDoc.SetTextDefaults();     // overwrites loaded pool defaults

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
    }

    if (rDoc.nFileFormatVersion >= 3)
    {
        ULONG nTmp;
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
        rDoc.eDocType = (DocumentType) nDocType;
        // existiert eine DocShell bestimmt diese den DocType
        if(rDoc.pDocSh)
        {
            if(NULL != PTR_CAST(SdGraphicDocShell,rDoc.pDocSh))
                rDoc.eDocType = DOCUMENT_TYPE_DRAW;
            else
                rDoc.eDocType = DOCUMENT_TYPE_IMPRESS;
        }
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
            // Dokument wurde geladen:

        CheckMasterPages();

        if ( GetMasterSdPageCount(PK_STANDARD) > 1 )
            RemoveUnnessesaryMasterPages( NULL, TRUE, FALSE );

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
        pDocLockedInsertingLinks = this; // lock inserting links. only links in this document should by resolved

        pLinkManager->UpdateAllLinks();  // query box: update all links?

        if( pDocLockedInsertingLinks == this )
            pDocLockedInsertingLinks = NULL;  // unlock inserting links
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

    Outliner& rOutliner = GetDrawOutliner();

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
