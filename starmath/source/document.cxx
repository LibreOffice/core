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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Any.h>

#include <comphelper/accessibletexthelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <rtl/logfile.hxx>
#include <rtl/ustring.hxx>
#include <unotools/eventcfg.hxx>
#include <sfx2/event.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/clsids.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <svl/eitem.hxx>
#include <svl/fstathelper.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/ptitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/slstitm.hxx>
#include <svl/smplhint.hxx>
#include <svl/stritem.hxx>
#include <svtools/transfer.hxx>
#include <svl/undo.hxx>
#include <svl/urihelper.hxx>
#include <svl/whiter.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/unolingu.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/mapmod.hxx>
#include <tools/mapunit.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/sfx.hrc>
#include <document.hxx>
#include <action.hxx>
#include <config.hxx>
#include <dialog.hxx>
#include <format.hxx>
#include <smdll.hxx>
#include <starmath.hrc>
#include <symbol.hxx>
#include <toolbox.hxx>
#include <unomodel.hxx>
#include <utility.hxx>
#include <view.hxx>
#include "mathtype.hxx"
#include "mathmlimport.hxx"
#include "mathmlexport.hxx"
#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;


#define DOCUMENT_BUFFER_SIZE    (sal_uInt16)32768

static const char __FAR_DATA pStarMathDoc[] = "StarMathDocument";

#define SmDocShell
#include "smslots.hxx"

////////////////////////////////////////////////////////////


TYPEINIT1( SmDocShell, SfxObjectShell );

SFX_IMPL_INTERFACE(SmDocShell, SfxObjectShell, SmResId(0))
{
    SFX_POPUPMENU_REGISTRATION(SmResId(RID_VIEWMENU));
    SFX_POPUPMENU_REGISTRATION(SmResId(RID_COMMANDMENU));
}

SFX_IMPL_OBJECTFACTORY(SmDocShell, SvGlobalName(SO3_SM_CLASSID), SFXOBJECTSHELL_STD_NORMAL, "smath" )

void SmDocShell::SFX_NOTIFY(SfxBroadcaster&, const TypeId&,
                    const SfxHint& rHint, const TypeId&)
{
    switch (((SfxSimpleHint&)rHint).GetId())
    {
        case HINT_FORMATCHANGED:
            SetFormulaArranged(sal_False);

            nModifyCount++;     //! see comment for SID_GAPHIC_SM in SmDocShell::GetState

            Repaint();
            break;
    }
}

void SmDocShell::LoadSymbols()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::LoadSymbols" );

    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Load();
}


const String SmDocShell::GetComment() const
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetComment" );
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        const_cast<SmDocShell*>(this)->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    return xDocProps->getDescription();
}


void SmDocShell::SetText(const String& rBuffer)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetText" );

    if (rBuffer != aText)
    {
        sal_Bool bIsEnabled = IsEnableSetModified();
        if( bIsEnabled )
            EnableSetModified( sal_False );

        aText = rBuffer;
        SetFormulaArranged( sal_False );

        Parse();
        //Repaint();

        SmViewShell *pViewSh = SmGetActiveView();
        if( pViewSh )
        {
            pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_TEXT);
            if ( SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
            {
                // have SwOleClient::FormatChanged() to align the modified formula properly
                // even if the vis area does not change (e.g. when formula text changes from
                // "{a over b + c} over d" to "d over {a over b + c}"
                SFX_APP()->NotifyEvent(SfxEventHint( SFX_EVENT_VISAREACHANGED, GlobalEventConfig::GetEventName(STR_EVENT_VISAREACHANGED), this));

                Repaint();
            }
            else
                pViewSh->GetGraphicWindow().Invalidate();
        }

        if ( bIsEnabled )
            EnableSetModified( bIsEnabled );
        SetModified(sal_True);

        // launch accessible event if necessary
        SmGraphicAccessible *pAcc = pViewSh ? pViewSh->GetGraphicWindow().GetAccessible_Impl() : 0;
        if (pAcc)
        {
            Any aOldValue, aNewValue;
            if ( comphelper::OCommonAccessibleText::implInitTextChangedEvent( aText, rBuffer, aOldValue, aNewValue ) )
            {
                pAcc->LaunchEvent( AccessibleEventId::TEXT_CHANGED,
                        aOldValue, aNewValue );
            }
        }

        if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
            OnDocumentPrinterChanged(0);
    }
}

void SmDocShell::SetFormat(SmFormat& rFormat)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetFormat" );

    aFormat = rFormat;
    SetFormulaArranged( sal_False );
    SetModified( sal_True );

    nModifyCount++;     //! see comment for SID_GAPHIC_SM in SmDocShell::GetState

    // don't use SmGetActiveView since the view shell might not be active (0 pointer)
    // if for example the Basic Macro dialog currently has the focus. Thus:
    SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
    while (pFrm)
    {
        pFrm->GetBindings().Invalidate(SID_GAPHIC_SM);
        pFrm = SfxViewFrame::GetNext( *pFrm, this );
    }
}

String SmDocShell::GetAccessibleText()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetAccessibleText" );

    if (!IsFormulaArranged())
        ArrangeFormula();
    if (0 == aAccText.Len())
    {
        DBG_ASSERT( pTree, "Tree missing" );
        if (pTree)
            pTree->GetAccessibleText( aAccText );
    }
    return aAccText;
}

void SmDocShell::Parse()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Parse" );

    if (pTree)
        delete pTree;
    ReplaceBadChars();
    pTree = aInterpreter.Parse(aText);
    nModifyCount++;     //! see comment for SID_GAPHIC_SM in SmDocShell::GetState
    SetFormulaArranged( sal_False );

    aUsedSymbols = aInterpreter.GetUsedSymbols();
}


void SmDocShell::ArrangeFormula()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::ArrangeFormula" );

    if (IsFormulaArranged())
        return;

    //! Nur f�r die Dauer der Existenz dieses Objekts sind am Drucker die
    //! richtigen Einstellungen garantiert.
    SmPrinterAccess  aPrtAcc(*this);
//  OutputDevice    *pOutDev = aPrtAcc.GetPrinter();
    OutputDevice* pOutDev = aPrtAcc.GetRefDev();

    if (!pOutDev)
    {
#if OSL_DEBUG_LEVEL > 1
        DBG_ERROR("!! SmDocShell::ArrangeFormula: reference device missing !!");
#endif
    }

    // falls n�tig ein anderes OutputDevice holen f�r das formatiert wird
    if (!pOutDev)
    {
        SmViewShell *pView = SmGetActiveView();
        if (pView)
            pOutDev = &pView->GetGraphicWindow();
        else
        {
            pOutDev = &SM_MOD()->GetDefaultVirtualDev();
            pOutDev->SetMapMode( MapMode(MAP_100TH_MM) );
        }
    }
    DBG_ASSERT(pOutDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
               "Sm : falscher MapMode");

    const SmFormat &rFormat = GetFormat();
    pTree->Prepare(rFormat, *this);

    // format/draw formulas always from left to right,
    // and numbers should not be converted
    sal_uLong nLayoutMode = pOutDev->GetLayoutMode();
    pOutDev->SetLayoutMode( TEXT_LAYOUT_BIDI_LTR );
    sal_Int16 nDigitLang = pOutDev->GetDigitLanguage();
    pOutDev->SetDigitLanguage( LANGUAGE_ENGLISH );
    //
    pTree->Arrange(*pOutDev, rFormat);
    //
    pOutDev->SetLayoutMode( nLayoutMode );
    pOutDev->SetDigitLanguage( nDigitLang );

    SetFormulaArranged(sal_True);

    // invalidate accessible text
    aAccText = String();
}


void SetEditEngineDefaultFonts(
        EditEngine &/*rEditEngine*/,
        SfxItemPool &rEditEngineItemPool )
{
        //
        // set fonts to be used
        //
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );
        //
        struct FontDta {
            sal_Int16       nFallbackLang;
            sal_Int16       nLang;
            sal_uInt16      nFontType;
            sal_uInt16      nFontInfoId;
            } aTable[3] =
        {
            // info to get western font to be used
            {   LANGUAGE_ENGLISH_US,    LANGUAGE_NONE,
                DEFAULTFONT_FIXED,      EE_CHAR_FONTINFO },
            // info to get CJK font to be used
            {   LANGUAGE_JAPANESE,      LANGUAGE_NONE,
                DEFAULTFONT_CJK_TEXT,   EE_CHAR_FONTINFO_CJK },
            // info to get CTL font to be used
            {   LANGUAGE_ARABIC_SAUDI_ARABIA,  LANGUAGE_NONE,
                DEFAULTFONT_CTL_TEXT,   EE_CHAR_FONTINFO_CTL }
        };
        aTable[0].nLang = aOpt.nDefaultLanguage;
        aTable[1].nLang = aOpt.nDefaultLanguage_CJK;
        aTable[2].nLang = aOpt.nDefaultLanguage_CTL;
        //
        for (int i = 0;  i < 3;  ++i)
        {
            const FontDta &rFntDta = aTable[i];
            LanguageType nLang = (LANGUAGE_NONE == rFntDta.nLang) ?
                    rFntDta.nFallbackLang : rFntDta.nLang;
            Font aFont = Application::GetDefaultDevice()->GetDefaultFont(
                        rFntDta.nFontType, nLang, DEFAULTFONT_FLAGS_ONLYONE );
#ifdef DEBUG_TL
            ByteString aFntName( aFont.GetName(), 1 );
            int eFntFamily = aFont.GetFamily();
            ByteString aFntStyleName( aFont.GetStyleName(), 1 );
            int ePitch = aFont.GetPitch();
            int eCharSet = aFont.GetCharSet();
            fprintf(stderr, "\nFontName %s \n", aFntName.GetBuffer() );
            fprintf(stderr, "StyleName %s \n", aFntStyleName.GetBuffer() );
            fprintf(stderr, "eFntFamily %d \n", eFntFamily );
            fprintf(stderr, "ePitch %d \n", ePitch );
            fprintf(stderr, "eCharSet %d \n", eCharSet );
#endif
            rEditEngineItemPool.SetPoolDefaultItem(
                    SvxFontItem( aFont.GetFamily(), aFont.GetName(),
                        aFont.GetStyleName(), aFont.GetPitch(), aFont.GetCharSet(),
                        rFntDta.nFontInfoId ) );
        }

        // set font heights
        SvxFontHeightItem aFontHeigt(
                        Application::GetDefaultDevice()->LogicToPixel(
                        Size( 0, 11 ), MapMode( MAP_POINT ) ).Height(), 100,
                        EE_CHAR_FONTHEIGHT );
        rEditEngineItemPool.SetPoolDefaultItem( aFontHeigt );
        aFontHeigt.SetWhich( EE_CHAR_FONTHEIGHT_CJK );
        rEditEngineItemPool.SetPoolDefaultItem( aFontHeigt );
        aFontHeigt.SetWhich( EE_CHAR_FONTHEIGHT_CTL );
        rEditEngineItemPool.SetPoolDefaultItem( aFontHeigt );
}


EditEngine& SmDocShell::GetEditEngine()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetEditEngine" );

    if (!pEditEngine)
    {
        //!
        //! see also SmEditWindow::DataChanged !
        //!

        pEditEngineItemPool = EditEngine::CreatePool();

        SetEditEngineDefaultFonts( *pEditEngine, *pEditEngineItemPool );

        pEditEngine = new EditEngine( pEditEngineItemPool );

        pEditEngine->EnableUndo( sal_True );
        pEditEngine->SetDefTab( sal_uInt16(
            Application::GetDefaultDevice()->GetTextWidth( C2S("XXXX") ) ) );

        pEditEngine->SetControlWord(
                (pEditEngine->GetControlWord() | EE_CNTRL_AUTOINDENTING) &
                (~EE_CNTRL_UNDOATTRIBS) &
                (~EE_CNTRL_PASTESPECIAL) );

        pEditEngine->SetWordDelimiters( C2S(" .=+-*/(){}[];\"" ) );
        pEditEngine->SetRefMapMode( MAP_PIXEL );

        pEditEngine->SetPaperSize( Size( 800, 0 ) );

        pEditEngine->EraseVirtualDevice();

        // set initial text if the document already has some...
        // (may be the case when reloading a doc)
        String aTxt( GetText() );
        if (aTxt.Len())
            pEditEngine->SetText( aTxt );

        pEditEngine->ClearModifyFlag();

        // forces new settings to be used if the itempool was modified
        // after cthe creation of the EditEngine
        //pEditEngine->Clear(); //#77957 incorrect font size
    }
    return *pEditEngine;
}


SfxItemPool& SmDocShell::GetEditEngineItemPool()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetEditEngineItemPool" );

    if (!pEditEngineItemPool)
        GetEditEngine();
    DBG_ASSERT( pEditEngineItemPool, "EditEngineItemPool missing" );
    return *pEditEngineItemPool;
}


void SmDocShell::Draw(OutputDevice &rDev, Point &rPosition)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Draw" );

    if (!pTree)
        Parse();
    DBG_ASSERT(pTree, "Sm : NULL pointer");

    if (!IsFormulaArranged())
        ArrangeFormula();

    //Problem: Was passiert mit dem WYSIWYG? Wir haben waehrend wir inplace aktiv
    //sind kein Referenzdevice und sind auch nicht darauf ausgerichtet. Es kann
    //also jetzt eine Differenz zwischen der VisArea (spricht die Groesse im Client)
    //und der jetzt vorliegenden Groese geben.
    //Idee: Die Differenz koennte, zumindest behelfsmaessig, mit SmNod::SetSize
    //angepasst werden.

    rPosition.X() += aFormat.GetDistance( DIS_LEFTSPACE );
    rPosition.Y() += aFormat.GetDistance( DIS_TOPSPACE  );

    //! in case of high contrast-mode (accessibility option!)
    //! the draw mode needs to be set to default, because when imbedding
    //! Math for example in Calc in "a over b" the fraction bar may not
    //! be visible else. More generally: the FillColor may have been changed.
    sal_uLong nOldDrawMode = DRAWMODE_DEFAULT;
    sal_Bool bRestoreDrawMode = sal_False;
    if (OUTDEV_WINDOW == rDev.GetOutDevType() &&
        ((Window &) rDev).GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        nOldDrawMode = rDev.GetDrawMode();
        rDev.SetDrawMode( DRAWMODE_DEFAULT );
        bRestoreDrawMode = sal_True;
    }

    // format/draw formulas always from left to right
    // and numbers should not be converted
    sal_uLong nLayoutMode = rDev.GetLayoutMode();
    rDev.SetLayoutMode( TEXT_LAYOUT_BIDI_LTR );
    sal_Int16 nDigitLang = rDev.GetDigitLanguage();
    rDev.SetDigitLanguage( LANGUAGE_ENGLISH );
    //
    pTree->Draw(rDev, rPosition);
    //
    rDev.SetLayoutMode( nLayoutMode );
    rDev.SetDigitLanguage( nDigitLang );

    if (bRestoreDrawMode)
        rDev.SetDrawMode( nOldDrawMode );
}



Size SmDocShell::GetSize()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetSize" );

    Size aRet;

    if (!pTree)
        Parse();

    if (pTree)
    {
        if (!IsFormulaArranged())
            ArrangeFormula();
        aRet = pTree->GetSize();

        if ( !aRet.Width() )
            aRet.Width() = 2000;
        else
            aRet.Width()  += aFormat.GetDistance( DIS_LEFTSPACE ) +
                             aFormat.GetDistance( DIS_RIGHTSPACE );
        if ( !aRet.Height() )
            aRet.Height() = 1000;
        else
            aRet.Height() += aFormat.GetDistance( DIS_TOPSPACE ) +
                             aFormat.GetDistance( DIS_BOTTOMSPACE );
    }

    return aRet;
}

////////////////////////////////////////

SmPrinterAccess::SmPrinterAccess( SmDocShell &rDocShell )
{
    if ( 0 != (pPrinter = rDocShell.GetPrt()) )
    {
        pPrinter->Push( PUSH_MAPMODE );
        if ( SFX_CREATE_MODE_EMBEDDED == rDocShell.GetCreateMode() )
        {
            // if it is an embedded object (without it's own printer)
            // we change the MapMode temporarily.
            //!If it is a document with it's own printer the MapMode should
            //!be set correct (once) elsewhere(!), in order to avoid numerous
            //!superfluous pushing and poping of the MapMode when using
            //!this class.

            const MapUnit eOld = pPrinter->GetMapMode().GetMapUnit();
            if ( MAP_100TH_MM != eOld )
            {
                MapMode aMap( pPrinter->GetMapMode() );
                aMap.SetMapUnit( MAP_100TH_MM );
                Point aTmp( aMap.GetOrigin() );
                aTmp.X() = OutputDevice::LogicToLogic( aTmp.X(), eOld, MAP_100TH_MM );
                aTmp.Y() = OutputDevice::LogicToLogic( aTmp.Y(), eOld, MAP_100TH_MM );
                aMap.SetOrigin( aTmp );
                pPrinter->SetMapMode( aMap );
            }
        }
    }
    if ( 0 != (pRefDev = rDocShell.GetRefDev()) && pPrinter != pRefDev )
    {
        pRefDev->Push( PUSH_MAPMODE );
        if ( SFX_CREATE_MODE_EMBEDDED == rDocShell.GetCreateMode() )
        {
            // if it is an embedded object (without it's own printer)
            // we change the MapMode temporarily.
            //!If it is a document with it's own printer the MapMode should
            //!be set correct (once) elsewhere(!), in order to avoid numerous
            //!superfluous pushing and poping of the MapMode when using
            //!this class.

            const MapUnit eOld = pRefDev->GetMapMode().GetMapUnit();
            if ( MAP_100TH_MM != eOld )
            {
                MapMode aMap( pRefDev->GetMapMode() );
                aMap.SetMapUnit( MAP_100TH_MM );
                Point aTmp( aMap.GetOrigin() );
                aTmp.X() = OutputDevice::LogicToLogic( aTmp.X(), eOld, MAP_100TH_MM );
                aTmp.Y() = OutputDevice::LogicToLogic( aTmp.Y(), eOld, MAP_100TH_MM );
                aMap.SetOrigin( aTmp );
                pRefDev->SetMapMode( aMap );
            }
        }
    }
}

SmPrinterAccess::~SmPrinterAccess()
{
    if ( pPrinter )
        pPrinter->Pop();
    if ( pRefDev && pRefDev != pPrinter )
        pRefDev->Pop();
}

////////////////////////////////////////

Printer* SmDocShell::GetPrt()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetPrt" );

    if ( SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
    {
        //Normalerweise wird der Printer vom Server besorgt. Wenn dieser aber
        //keinen liefert (weil etwa noch keine connection da ist), kann es
        //dennoch sein, dass wir den Printer kennen, denn dieser wird in
        //OnDocumentPrinterChanged vom Server durchgereicht und dann temporaer
        //festgehalten.
        Printer *pPrt = GetDocumentPrinter();
        if ( !pPrt && pTmpPrinter )
            pPrt = pTmpPrinter;
        return pPrt;
    }
    else if ( !pPrinter )
    {
        SfxItemSet *pOptions =
            new SfxItemSet(GetPool(),
                           SID_PRINTSIZE,       SID_PRINTSIZE,
                           SID_PRINTZOOM,       SID_PRINTZOOM,
                           SID_PRINTTITLE,      SID_PRINTTITLE,
                           SID_PRINTTEXT,       SID_PRINTTEXT,
                           SID_PRINTFRAME,      SID_PRINTFRAME,
                           SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
                           0);

        SmModule *pp = SM_MOD();
        pp->GetConfig()->ConfigToItemSet(*pOptions);
        pPrinter = new SfxPrinter(pOptions);
        pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
    }
    return pPrinter;
}

OutputDevice* SmDocShell::GetRefDev()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetRefDev" );

    if ( SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
    {
        OutputDevice* pOutDev = GetDocumentRefDev();
        if ( pOutDev )
            return pOutDev;
    }

    return GetPrt();
}


void SmDocShell::SetPrinter( SfxPrinter *pNew )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetPrinter" );

    delete pPrinter;
    pPrinter = pNew;    //Eigentumsuebergang!
    pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
    SetFormulaArranged(sal_False);
    Repaint();
}

void SmDocShell::OnDocumentPrinterChanged( Printer *pPrt )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::OnDocumentPrinterChanged" );

    pTmpPrinter = pPrt;
    SetFormulaArranged(sal_False);
    Size aOldSize = GetVisArea().GetSize();
    Repaint();
    if( aOldSize != GetVisArea().GetSize() && aText.Len() )
        SetModified( sal_True );
    pTmpPrinter = 0;
}

void SmDocShell::Repaint()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Repaint" );

    sal_Bool bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( sal_False );

    SetFormulaArranged( sal_False );

    Size aVisSize = GetSize();
    SetVisAreaSize( aVisSize );
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetGraphicWindow().Invalidate();

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


SmDocShell::SmDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell( i_nSfxCreationFlags ),
    pTree               ( 0 ),
    pEditEngineItemPool ( 0 ),
    pEditEngine         ( 0 ),
    pPrinter            ( 0 ),
    pTmpPrinter         ( 0 ),
    nModifyCount        ( 0 ),
    bIsFormulaArranged  ( sal_False )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SmDocShell" );

    SetPool(&SFX_APP()->GetPool());

    SmModule *pp = SM_MOD();
    aFormat = pp->GetConfig()->GetStandardFormat();

    StartListening(aFormat);
    StartListening(*pp->GetConfig());

    SetBaseModel( new SmModel(this) );
}



SmDocShell::~SmDocShell()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::~SmDocShell" );

    SmModule *pp = SM_MOD();

    EndListening(aFormat);
    EndListening(*pp->GetConfig());

    delete pEditEngine;
    SfxItemPool::Free(pEditEngineItemPool);
    delete pTree;
    delete pPrinter;
}


sal_Bool SmDocShell::SetData( const String& rData )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetData" );

    SetText( rData );
    return sal_True;
}


sal_Bool SmDocShell::ConvertFrom(SfxMedium &rMedium)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::ConvertFrom" );

    sal_Bool     bSuccess = sal_False;
    const String& rFltName = rMedium.GetFilter()->GetFilterName();

    DBG_ASSERT( !rFltName.EqualsAscii( STAROFFICE_XML ), "Wrong filter!");

    if ( rFltName.EqualsAscii( MATHML_XML ) )
    {
        if (pTree)
        {
            delete pTree;
            pTree = 0;
        }
        Reference<com::sun::star::frame::XModel> xModel(GetModel());
        SmXMLImportWrapper aEquation(xModel);
        bSuccess = 0 == aEquation.Import(rMedium);
    }
    else
    {
        SvStream *pStream = rMedium.GetInStream();
        if ( pStream )
        {
            if ( SotStorage::IsStorageFile( pStream ) )
            {
                SvStorageRef aStorage = new SotStorage( pStream, sal_False );
                if ( aStorage->IsStream( C2S( "Equation Native" ) ) )
                {
                    // is this a MathType Storage?
                    MathType aEquation( aText );
                    if ( sal_True == (bSuccess = (1 == aEquation.Parse( aStorage )) ))
                        Parse();
                }
            }
            else
            {
                //bSuccess = ImportSM20File( pStream );
            }
        }
    }

    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        //???OnDocumentPrinterChanged(0);
        SetFormulaArranged( sal_False );
        Repaint();
    }

    FinishedLoading( SFX_LOADED_ALL );
    return bSuccess;
}


sal_Bool SmDocShell::InitNew( const uno::Reference < embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::InitNew" );

    sal_Bool bRet = sal_False;
    if ( SfxObjectShell::InitNew( xStorage ) )
    {
        bRet = sal_True;
        SetVisArea(Rectangle(Point(0, 0), Size(2000, 1000)));
    }
    return bRet;
}


sal_Bool SmDocShell::Load( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Load" );

    sal_Bool bRet = sal_False;
    if( SfxObjectShell::Load( rMedium ))
    {
        uno::Reference < embed::XStorage > xStorage = GetMedium()->GetStorage();
        uno::Reference < container::XNameAccess > xAccess (xStorage, uno::UNO_QUERY);
        if (
            (
             xAccess->hasByName( C2S( "content.xml" ) ) &&
             xStorage->isStreamElement( C2S( "content.xml" ) )
            ) ||
            (
             xAccess->hasByName( C2S( "Content.xml" ) ) &&
             xStorage->isStreamElement( C2S( "Content.xml" ) )
            )
           )
        {
            // is this a fabulous math package ?
            Reference<com::sun::star::frame::XModel> xModel(GetModel());
            SmXMLImportWrapper aEquation(xModel);
            sal_uLong nError = aEquation.Import(rMedium);
            bRet = 0 == nError;
            SetError( nError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        }
    }

    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        //???OnDocumentPrinterChanged(0);
        SetFormulaArranged( sal_False );
        Repaint();
    }

    FinishedLoading( SFX_LOADED_ALL );
    return bRet;
}

//------------------------------------------------------------------

sal_Bool SmDocShell::Save()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Save" );

    //! apply latest changes if necessary
    UpdateText();

    if ( SfxObjectShell::Save() )
    {
        if (!pTree)
            Parse();
        if( pTree && !IsFormulaArranged() )
            ArrangeFormula();

        Reference<com::sun::star::frame::XModel> xModel(GetModel());
        SmXMLExportWrapper aEquation(xModel);
        aEquation.SetFlat(sal_False);
        return aEquation.Export(*GetMedium());
    }

    return sal_False;
}

/*
 * replace bad characters that can not be saved. (#i74144)
 * */
sal_Bool SmDocShell::ReplaceBadChars()
{
    sal_Bool bReplace = sal_False;
    if (pEditEngine)
    {
        String aEngTxt( pEditEngine->GetText( LINEEND_LF ) );
        const sal_Unicode *pEngTxt = aEngTxt.GetBuffer();
        xub_StrLen nLen = aEngTxt.Len();
        for (xub_StrLen i = 0;  i < nLen && !bReplace;  ++i)
        {
            const sal_Unicode c = *pEngTxt++;
            if (c < ' ' && c != '\r' && c != '\n' && c != '\t')
                bReplace = sal_True;
        }
        if (bReplace)
        {
            sal_Unicode *pChgTxt = aEngTxt.GetBufferAccess();
            for (xub_StrLen i = 0;  i < nLen;  ++i)
            {
                sal_Unicode &rc = *pChgTxt++;
                if (rc < ' ' && rc != '\r' && rc != '\n' && rc != '\t')
                    rc = ' ';
            }
            aEngTxt.ReleaseBufferAccess( nLen );

            aText = aEngTxt;
        }
    }
    return bReplace;
}


void SmDocShell::UpdateText()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::UpdateText" );

    if (pEditEngine && pEditEngine->IsModified())
    {
        String aEngTxt( pEditEngine->GetText( LINEEND_LF ) );
        if (GetText() != aEngTxt)
            SetText( aEngTxt );
    }
}


sal_Bool SmDocShell::SaveAs( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SaveAs" );

    sal_Bool bRet = sal_False;

    //! apply latest changes if necessary
    UpdateText();

    if ( SfxObjectShell::SaveAs( rMedium ) )
    {
        if (!pTree)
            Parse();
        if( pTree && !IsFormulaArranged() )
            ArrangeFormula();

        Reference<com::sun::star::frame::XModel> xModel(GetModel());
        SmXMLExportWrapper aEquation(xModel);
        aEquation.SetFlat(sal_False);
        bRet = aEquation.Export(rMedium);
    }
    return bRet;
}

sal_Bool SmDocShell::ConvertTo( SfxMedium &rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::ConvertTo" );

    sal_Bool bRet = sal_False;
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( pFlt )
    {
        if( !pTree )
            Parse();
        if( pTree && !IsFormulaArranged() )
            ArrangeFormula();

        const String& rFltName = pFlt->GetFilterName();
        if(rFltName.EqualsAscii( STAROFFICE_XML ))
        {
            Reference<com::sun::star::frame::XModel> xModel(GetModel());
            SmXMLExportWrapper aEquation(xModel);
            aEquation.SetFlat(sal_False);
            bRet = aEquation.Export(rMedium);
        }
        else if(rFltName.EqualsAscii( MATHML_XML ))
        {
            Reference<com::sun::star::frame::XModel> xModel(GetModel());
            SmXMLExportWrapper aEquation(xModel);
            aEquation.SetFlat(sal_True);
            bRet = aEquation.Export(rMedium);
        }
        else if( pFlt->GetFilterName().EqualsAscii("MathType 3.x"))
            bRet = WriteAsMathType3( rMedium );
    }
    return bRet;
}

sal_Bool SmDocShell::SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SaveCompleted" );

    if( SfxObjectShell::SaveCompleted( xStorage ))
        return sal_True;

    return sal_False;
}


void SmDocShell::Execute(SfxRequest& rReq)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Execute" );

    switch (rReq.GetSlot())
    {
        case SID_TEXTMODE:
        {
            SmFormat aOldFormat  = GetFormat();
            SmFormat aNewFormat( aOldFormat );
            aNewFormat.SetTextmode(!aOldFormat.IsTextmode());

            ::svl::IUndoManager *pTmpUndoMgr = GetUndoManager();
            if (pTmpUndoMgr)
                pTmpUndoMgr->AddUndoAction(
                    new SmFormatAction(this, aOldFormat, aNewFormat));

            SetFormat( aNewFormat );
            Repaint();
        }
        break;

        case SID_AUTO_REDRAW :
        {
            SmModule *pp = SM_MOD();
            sal_Bool bRedraw = pp->GetConfig()->IsAutoRedraw();
            pp->GetConfig()->SetAutoRedraw(!bRedraw);
        }
        break;

        case SID_LOADSYMBOLS:
            LoadSymbols();
        break;

        case SID_SAVESYMBOLS:
            SaveSymbols();
        break;

        case SID_FONT:
        {
            // get device used to retrieve the FontList
            OutputDevice *pDev = GetPrinter();
            if (!pDev || pDev->GetDevFontCount() == 0)
                pDev = &SM_MOD()->GetDefaultVirtualDev();
            DBG_ASSERT (pDev, "device for font list missing" );

            SmFontTypeDialog *pFontTypeDialog = new SmFontTypeDialog( NULL, pDev );

            SmFormat aOldFormat  = GetFormat();
            pFontTypeDialog->ReadFrom( aOldFormat );
            if (pFontTypeDialog->Execute() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                pFontTypeDialog->WriteTo(aNewFormat);
                ::svl::IUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        new SmFormatAction(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
            delete pFontTypeDialog;
        }
        break;

        case SID_FONTSIZE:
        {
            SmFontSizeDialog *pFontSizeDialog = new SmFontSizeDialog(NULL);

            SmFormat aOldFormat  = GetFormat();
            pFontSizeDialog->ReadFrom( aOldFormat );
            if (pFontSizeDialog->Execute() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                pFontSizeDialog->WriteTo(aNewFormat);

                ::svl::IUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        new SmFormatAction(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
            delete pFontSizeDialog;
        }
        break;

        case SID_DISTANCE:
        {
            SmDistanceDialog *pDistanceDialog = new SmDistanceDialog(NULL);

            SmFormat aOldFormat  = GetFormat();
            pDistanceDialog->ReadFrom( aOldFormat );
            if (pDistanceDialog->Execute() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                pDistanceDialog->WriteTo(aNewFormat);

                ::svl::IUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        new SmFormatAction(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
            delete pDistanceDialog;
        }
        break;

        case SID_ALIGN:
        {
            SmAlignDialog *pAlignDialog = new SmAlignDialog(NULL);

            SmFormat aOldFormat  = GetFormat();
            pAlignDialog->ReadFrom( aOldFormat );
            if (pAlignDialog->Execute() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                pAlignDialog->WriteTo(aNewFormat);

                SmModule *pp = SM_MOD();
                SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
                pAlignDialog->WriteTo( aFmt );
                pp->GetConfig()->SetStandardFormat( aFmt );

                ::svl::IUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        new SmFormatAction(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
            delete pAlignDialog;
        }
        break;

        case SID_TEXT:
        {
            const SfxStringItem& rItem = (const SfxStringItem&)rReq.GetArgs()->Get(SID_TEXT);
            if (GetText() != rItem.GetValue())
                SetText(rItem.GetValue());
        }
        break;

        case SID_UNDO:
        case SID_REDO:
        {
            ::svl::IUndoManager* pTmpUndoMgr = GetUndoManager();
            if( pTmpUndoMgr )
            {
                sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
                const SfxItemSet* pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem;
                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, sal_False, &pItem ))
                    nCnt = ((SfxUInt16Item*)pItem)->GetValue();

                sal_Bool (::svl::IUndoManager:: *fnDo)();

                sal_uInt16 nCount;
                if( SID_UNDO == rReq.GetSlot() )
                {
                    nCount = pTmpUndoMgr->GetUndoActionCount();
                    fnDo = &::svl::IUndoManager::Undo;
                }
                else
                {
                    nCount = pTmpUndoMgr->GetRedoActionCount();
                    fnDo = &::svl::IUndoManager::Redo;
                }

                try
                {
                    for( ; nCnt && nCount; --nCnt, --nCount )
                        (pTmpUndoMgr->*fnDo)();
                }
                catch( const Exception& e )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            Repaint();
            SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
            while( pFrm )
            {
                SfxBindings& rBind = pFrm->GetBindings();
                rBind.Invalidate(SID_UNDO);
                rBind.Invalidate(SID_REDO);
                rBind.Invalidate(SID_REPEAT);
                rBind.Invalidate(SID_CLEARHISTORY);
                pFrm = SfxViewFrame::GetNext( *pFrm, this );
            }
        }
        break;
    }

    rReq.Done();
}


void SmDocShell::GetState(SfxItemSet &rSet)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetState" );

    SfxWhichIter aIter(rSet);

    for (sal_uInt16 nWh = aIter.FirstWhich();  0 != nWh;  nWh = aIter.NextWhich())
    {
        switch (nWh)
        {
        case SID_TEXTMODE:
            rSet.Put(SfxBoolItem(SID_TEXTMODE, GetFormat().IsTextmode()));
            break;

        case SID_DOCTEMPLATE :
            rSet.DisableItem(SID_DOCTEMPLATE);
            break;

        case SID_AUTO_REDRAW :
            {
                SmModule  *pp = SM_MOD();
                sal_Bool       bRedraw = pp->GetConfig()->IsAutoRedraw();

                rSet.Put(SfxBoolItem(SID_AUTO_REDRAW, bRedraw));
            }
            break;

        case SID_MODIFYSTATUS:
            {
                sal_Unicode cMod = ' ';
                if (IsModified())
                    cMod = '*';
                rSet.Put(SfxStringItem(SID_MODIFYSTATUS, String(cMod)));
            }
            break;

        case SID_TEXT:
            rSet.Put(SfxStringItem(SID_TEXT, GetText()));
            break;

        case SID_GAPHIC_SM:
            //! very old (pre UNO) and ugly hack to invalidate the SmGraphicWindow.
            //! If nModifyCount gets changed then the call below will implicitly notify
            //! SmGraphicController::StateChanged and there the window gets invalidated.
            //! Thus all the 'nModifyCount++' before invalidating this slot.
            rSet.Put(SfxInt16Item(SID_GAPHIC_SM, nModifyCount));
            break;

        case SID_UNDO:
        case SID_REDO:
            {
                SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
                if( pFrm )
                    pFrm->GetSlotState( nWh, NULL, &rSet );
                else
                    rSet.DisableItem( nWh );
            }
            break;

        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                ::svl::IUndoManager* pTmpUndoMgr = GetUndoManager();
                if( pTmpUndoMgr )
                {
                    UniString(::svl::IUndoManager:: *fnGetComment)( size_t, bool const ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWh )
                    {
                        nCount = pTmpUndoMgr->GetUndoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pTmpUndoMgr->GetRedoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetRedoActionComment;
                    }
                    if( nCount )
                    {
                        String sList;
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            ( sList += (pTmpUndoMgr->*fnGetComment)( n, ::svl::IUndoManager::TopLevel ) )
                                    += '\n';

                        SfxStringListItem aItem( nWh );
                        aItem.SetString( sList );
                        rSet.Put( aItem );
                    }
                }
                else
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
}


::svl::IUndoManager *SmDocShell::GetUndoManager()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::GetUndoManager" );

    if (!pEditEngine)
        GetEditEngine();
    return &pEditEngine->GetUndoManager();
}


void SmDocShell::SaveSymbols()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SaveSymbols" );

    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Save();
}


void SmDocShell::Draw(OutputDevice *pDevice,
                      const JobSetup &,
                      sal_uInt16 /*nAspect*/)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::Draw" );

    pDevice->IntersectClipRegion(GetVisArea());
    Point atmppoint;
    Draw(*pDevice, atmppoint);
}

SfxItemPool& SmDocShell::GetPool() const
{
    return SFX_APP()->GetPool();
}

void SmDocShell::SetVisArea(const Rectangle & rVisArea)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetVisArea" );

    Rectangle aNewRect(rVisArea);

    aNewRect.SetPos(Point());

    if (! aNewRect.Right()) aNewRect.Right() = 2000;
    if (! aNewRect.Bottom()) aNewRect.Bottom() = 1000;

    sal_Bool bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( sal_False );

    //TODO/LATER: it's unclear how this interacts with the SFX code
    // If outplace editing, then dont resize the OutplaceWindow. But the
    // ObjectShell has to resize. Bug 56470
    sal_Bool bUnLockFrame;
    if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !IsInPlaceActive() && GetFrame() )
    {
        GetFrame()->LockAdjustPosSizePixel();
        bUnLockFrame = sal_True;
    }
    else
        bUnLockFrame = sal_False;

    SfxObjectShell::SetVisArea( aNewRect );

    if( bUnLockFrame )
        GetFrame()->UnlockAdjustPosSizePixel();

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


void SmDocShell::FillClass(SvGlobalName* pClassName,
                           sal_uInt32*  pFormat,
                           String* /*pAppName*/,
                           String* pFullTypeName,
                           String* pShortTypeName,
                           sal_Int32 nFileFormat,
                           sal_Bool bTemplate /* = sal_False */) const
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::FillClass" );

    if (nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_60);
        *pFormat        = SOT_FORMATSTR_ID_STARMATH_60;
        *pFullTypeName  = String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_CURRENT));
        *pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_60);
        *pFormat        = bTemplate ? SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE : SOT_FORMATSTR_ID_STARMATH_8;
        *pFullTypeName  = String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_CURRENT));
        *pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    }
}

sal_uLong SmDocShell::GetMiscStatus() const
{
    return SfxObjectShell::GetMiscStatus() | SVOBJ_MISCSTATUS_NOTRESIZEABLE
                                             | SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
}

void SmDocShell::SetModified(sal_Bool bModified)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::SetModified" );

    if( IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
    }
}

sal_Bool SmDocShell::WriteAsMathType3( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmDocShell::WriteAsMathType3" );

    MathType aEquation( aText, pTree );

    sal_Bool bRet = 0 != aEquation.ConvertFromStarMath( rMedium );
    return bRet;
}


