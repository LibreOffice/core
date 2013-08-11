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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Any.h>

#include <comphelper/accessibletexthelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
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
#include <comphelper/classids.hxx>
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
#include "ooxmlexport.hxx"
#include "ooxmlimport.hxx"
#include "rtfexport.hxx"
#include "mathmlimport.hxx"
#include "mathmlexport.hxx"
#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include "cursor.hxx"
#include <tools/diagnose_ex.h>
#include "visitors.hxx"
#include "accessibility.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;

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
            SetFormulaArranged(false);

            nModifyCount++;     //! see comment for SID_GAPHIC_SM in SmDocShell::GetState

            Repaint();
            break;
    }
}

void SmDocShell::LoadSymbols()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::LoadSymbols" );

    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Load();
}


const String SmDocShell::GetComment() const
{
    SAL_INFO( "starmath", "starmath: SmDocShell::GetComment" );
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        const_cast<SmDocShell*>(this)->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    return xDocProps->getDescription();
}


void SmDocShell::SetText(const OUString& rBuffer)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SetText" );

    if (rBuffer != aText)
    {
        bool bIsEnabled = IsEnableSetModified();
        if( bIsEnabled )
            EnableSetModified( false );

        aText = rBuffer;
        SetFormulaArranged( false );

        Parse();

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
        SetModified(true);

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
    SAL_INFO( "starmath", "starmath: SmDocShell::SetFormat" );

    aFormat = rFormat;
    SetFormulaArranged( false );
    SetModified( true );

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

OUString SmDocShell::GetAccessibleText()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::GetAccessibleText" );

    if (!IsFormulaArranged())
        ArrangeFormula();
    if (aAccText.isEmpty())
    {
        OSL_ENSURE( pTree, "Tree missing" );
        if (pTree)
        {
            OUStringBuffer aBuf;
            pTree->GetAccessibleText(aBuf);
            aAccText = aBuf.makeStringAndClear();
        }
    }
    return aAccText;
}

void SmDocShell::Parse()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Parse" );

    if (pTree)
        delete pTree;
    ReplaceBadChars();
    pTree = aInterpreter.Parse(aText);
    nModifyCount++;     //! see comment for SID_GAPHIC_SM in SmDocShell::GetState
    SetFormulaArranged( false );
    InvalidateCursor();
    aUsedSymbols = aInterpreter.GetUsedSymbols();
}


void SmDocShell::ArrangeFormula()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::ArrangeFormula" );

    if (IsFormulaArranged())
        return;

    // Only for the duration of the existence of this object the correct settings
    // at the printer are guaranteed!
    SmPrinterAccess  aPrtAcc(*this);
    OutputDevice* pOutDev = aPrtAcc.GetRefDev();

    if (!pOutDev)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "starmath", "!! SmDocShell::ArrangeFormula: reference device missing !!");
#endif
    }

    // if necessary get another OutputDevice for which we format
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
    OSL_ENSURE(pOutDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
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

    SetFormulaArranged(true);

    // invalidate accessible text
    aAccText = OUString();
}


void SetEditEngineDefaultFonts(SfxItemPool &rEditEngineItemPool)
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
    SAL_INFO( "starmath", "starmath: SmDocShell::GetEditEngine" );

    if (!pEditEngine)
    {
        //!
        //! see also SmEditWindow::DataChanged !
        //!

        pEditEngineItemPool = EditEngine::CreatePool();

        SetEditEngineDefaultFonts(*pEditEngineItemPool);

        pEditEngine = new EditEngine( pEditEngineItemPool );

        pEditEngine->EnableUndo( true );
        pEditEngine->SetDefTab( sal_uInt16(
            Application::GetDefaultDevice()->GetTextWidth(OUString("XXXX"))) );

        pEditEngine->SetControlWord(
                (pEditEngine->GetControlWord() | EE_CNTRL_AUTOINDENTING) &
                (~EE_CNTRL_UNDOATTRIBS) &
                (~EE_CNTRL_PASTESPECIAL) );

        pEditEngine->SetWordDelimiters(" .=+-*/(){}[];\"");
        pEditEngine->SetRefMapMode( MAP_PIXEL );

        pEditEngine->SetPaperSize( Size( 800, 0 ) );

        pEditEngine->EraseVirtualDevice();

        // set initial text if the document already has some...
        // (may be the case when reloading a doc)
        String aTxt( GetText() );
        if (aTxt.Len())
            pEditEngine->SetText( aTxt );

        pEditEngine->ClearModifyFlag();

    }
    return *pEditEngine;
}


SfxItemPool& SmDocShell::GetEditEngineItemPool()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::GetEditEngineItemPool" );

    if (!pEditEngineItemPool)
        GetEditEngine();
    OSL_ENSURE( pEditEngineItemPool, "EditEngineItemPool missing" );
    return *pEditEngineItemPool;
}

void SmDocShell::DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Draw" );

    if (!pTree)
        Parse();
    OSL_ENSURE(pTree, "Sm : NULL pointer");

    if (!IsFormulaArranged())
        ArrangeFormula();

    // Problem: What happens to WYSIWYG? While we're active inplace, we don't have a reference
    // device and aren't aligned to that either. So now there can be a difference between the
    // VisArea (i.e. the size within the client) and the current size.
    // Idea: The difference could be adapted with SmNod::SetSize (no long-term solution)

    rPosition.X() += aFormat.GetDistance( DIS_LEFTSPACE );
    rPosition.Y() += aFormat.GetDistance( DIS_TOPSPACE  );

    //! in case of high contrast-mode (accessibility option!)
    //! the draw mode needs to be set to default, because when imbedding
    //! Math for example in Calc in "a over b" the fraction bar may not
    //! be visible else. More generally: the FillColor may have been changed.
    sal_uLong nOldDrawMode = DRAWMODE_DEFAULT;
    bool bRestoreDrawMode = false;
    if (OUTDEV_WINDOW == rDev.GetOutDevType() &&
        ((Window &) rDev).GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        nOldDrawMode = rDev.GetDrawMode();
        rDev.SetDrawMode( DRAWMODE_DEFAULT );
        bRestoreDrawMode = true;
    }

    // format/draw formulas always from left to right
    // and numbers should not be converted
    sal_uLong nLayoutMode = rDev.GetLayoutMode();
    rDev.SetLayoutMode( TEXT_LAYOUT_BIDI_LTR );
    sal_Int16 nDigitLang = rDev.GetDigitLanguage();
    rDev.SetDigitLanguage( LANGUAGE_ENGLISH );

    //Set selection if any
    if(pCursor && bDrawSelection){
        pCursor->AnnotateSelection();
        SmSelectionDrawingVisitor(rDev, pTree, rPosition);
    }

    //Drawing using visitor
    SmDrawingVisitor(rDev, rPosition, pTree);

    //
    rDev.SetLayoutMode( nLayoutMode );
    rDev.SetDigitLanguage( nDigitLang );

    if (bRestoreDrawMode)
        rDev.SetDrawMode( nOldDrawMode );
}

Size SmDocShell::GetSize()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::GetSize" );

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

void SmDocShell::InvalidateCursor(){
    delete pCursor;
    pCursor = NULL;
}

SmCursor& SmDocShell::GetCursor(){
    if(!pCursor)
        pCursor = new SmCursor(pTree, this);
    return *pCursor;
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
    SAL_INFO( "starmath", "starmath: SmDocShell::GetPrt" );

    if ( SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
    {
        // Normally the server provides the printer. But if it doesn't provide one (e.g. because
        // there is no connection) it still can be the case that we know the printer because it
        // has been passed on by the server in OnDocumentPrinterChanged and being kept temporarily.
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
                           SID_SAVE_ONLY_USED_SYMBOLS, SID_SAVE_ONLY_USED_SYMBOLS,
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
    SAL_INFO( "starmath", "starmath: SmDocShell::GetRefDev" );

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
    SAL_INFO( "starmath", "starmath: SmDocShell::SetPrinter" );

    delete pPrinter;
    pPrinter = pNew;    //Transfer ownership
    pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
    SetFormulaArranged(false);
    Repaint();
}

void SmDocShell::OnDocumentPrinterChanged( Printer *pPrt )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::OnDocumentPrinterChanged" );

    pTmpPrinter = pPrt;
    SetFormulaArranged(false);
    Size aOldSize = GetVisArea().GetSize();
    Repaint();
    if( aOldSize != GetVisArea().GetSize() && !aText.isEmpty() )
        SetModified( true );
    pTmpPrinter = 0;
}

void SmDocShell::Repaint()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Repaint" );

    bool bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( false );

    SetFormulaArranged( false );

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
    bIsFormulaArranged  ( false )
{
    pCursor = NULL;
    SAL_INFO( "starmath", "starmath: SmDocShell::SmDocShell" );

    SetPool(&SFX_APP()->GetPool());

    SmModule *pp = SM_MOD();
    aFormat = pp->GetConfig()->GetStandardFormat();

    StartListening(aFormat);
    StartListening(*pp->GetConfig());

    SetBaseModel( new SmModel(this) );
}



SmDocShell::~SmDocShell()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::~SmDocShell" );

    SmModule *pp = SM_MOD();

    EndListening(aFormat);
    EndListening(*pp->GetConfig());


    if(pCursor)
        delete pCursor;
    pCursor = NULL;

    delete pEditEngine;
    SfxItemPool::Free(pEditEngineItemPool);
    delete pTree;
    delete pPrinter;
}


sal_Bool SmDocShell::SetData( const String& rData )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SetData" );

    SetText( rData );
    return true;
}


sal_Bool SmDocShell::ConvertFrom(SfxMedium &rMedium)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::ConvertFrom" );

    bool     bSuccess = false;
    const String& rFltName = rMedium.GetFilter()->GetFilterName();

    OSL_ENSURE( !rFltName.EqualsAscii( STAROFFICE_XML ), "Wrong filter!");

    if ( rFltName.EqualsAscii( MATHML_XML ) )
    {
        if (pTree)
        {
            delete pTree;
            pTree = 0;
            InvalidateCursor();
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
                SvStorageRef aStorage = new SotStorage( pStream, false );
                if ( aStorage->IsStream(OUString("Equation Native")) )
                {
                    // is this a MathType Storage?
                    MathType aEquation( aText );
                    if ( true == (bSuccess = (1 == aEquation.Parse( aStorage )) ))
                        Parse();
                }
            }
        }
    }

    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        SetFormulaArranged( false );
        Repaint();
    }

    FinishedLoading( SFX_LOADED_ALL );
    return bSuccess;
}


sal_Bool SmDocShell::InitNew( const uno::Reference < embed::XStorage >& xStorage )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::InitNew" );

    bool bRet = false;
    if ( SfxObjectShell::InitNew( xStorage ) )
    {
        bRet = true;
        SetVisArea(Rectangle(Point(0, 0), Size(2000, 1000)));
    }
    return bRet;
}


sal_Bool SmDocShell::Load( SfxMedium& rMedium )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Load" );

    bool bRet = false;
    if( SfxObjectShell::Load( rMedium ))
    {
        uno::Reference < embed::XStorage > xStorage = GetMedium()->GetStorage();
        uno::Reference < container::XNameAccess > xAccess (xStorage, uno::UNO_QUERY);
        if (
            (
             xAccess->hasByName( OUString("content.xml") ) &&
             xStorage->isStreamElement( OUString("content.xml") )
            ) ||
            (
             xAccess->hasByName( OUString("Content.xml") ) &&
             xStorage->isStreamElement( OUString("Content.xml") )
            )
           )
        {
            // is this a fabulous math package ?
            Reference<com::sun::star::frame::XModel> xModel(GetModel());
            SmXMLImportWrapper aEquation(xModel);
            sal_uLong nError = aEquation.Import(rMedium);
            bRet = 0 == nError;
            SetError( nError, OSL_LOG_PREFIX );
        }
    }

    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        SetFormulaArranged( false );
        Repaint();
    }

    FinishedLoading( SFX_LOADED_ALL );
    return bRet;
}

//------------------------------------------------------------------

sal_Bool SmDocShell::Save()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Save" );

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

    return false;
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
    SAL_INFO( "starmath", "starmath: SmDocShell::UpdateText" );

    if (pEditEngine && pEditEngine->IsModified())
    {
        OUString aEngTxt( pEditEngine->GetText( LINEEND_LF ) );
        if (GetText() != aEngTxt)
            SetText( aEngTxt );
    }
}


sal_Bool SmDocShell::SaveAs( SfxMedium& rMedium )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SaveAs" );

    bool bRet = false;

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
    SAL_INFO( "starmath", "starmath: SmDocShell::ConvertTo" );

    bool bRet = false;
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
        else if (pFlt->GetFilterName().equalsAscii("MathType 3.x"))
            bRet = WriteAsMathType3( rMedium );
    }
    return bRet;
}

bool SmDocShell::writeFormulaOoxml( ::sax_fastparser::FSHelperPtr pSerializer, oox::core::OoxmlVersion version )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::writeFormulaOoxml" );

    if( !pTree )
        Parse();
    if( pTree && !IsFormulaArranged() )
        ArrangeFormula();
    SmOoxmlExport aEquation( pTree, version );
    return aEquation.ConvertFromStarMath( pSerializer );
}

void SmDocShell::writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding)
{
    if (!pTree)
        Parse();
    if (pTree && !IsFormulaArranged())
        ArrangeFormula();
    SmRtfExport aEquation(pTree);
    aEquation.ConvertFromStarMath(rBuffer, nEncoding);
}

void SmDocShell::readFormulaOoxml( oox::formulaimport::XmlStream& stream )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::readFormulaOoxml" );

    SmOoxmlImport aEquation( stream );
    SetText( aEquation.ConvertToStarMath());
}

sal_Bool SmDocShell::SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SaveCompleted" );

    if( SfxObjectShell::SaveCompleted( xStorage ))
        return true;

    return false;
}


void SmDocShell::Execute(SfxRequest& rReq)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Execute" );

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
            bool bRedraw = pp->GetConfig()->IsAutoRedraw();
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
            OSL_ENSURE (pDev, "device for font list missing" );

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
            if (GetText() != OUString(rItem.GetValue()))
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
                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, false, &pItem ))
                    nCnt = ((SfxUInt16Item*)pItem)->GetValue();

                bool (::svl::IUndoManager:: *fnDo)();

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
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            Repaint();
            UpdateText();
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
    SAL_INFO( "starmath", "starmath: SmDocShell::GetState" );

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
                bool       bRedraw = pp->GetConfig()->IsAutoRedraw();

                rSet.Put(SfxBoolItem(SID_AUTO_REDRAW, bRedraw));
            }
            break;

        case SID_MODIFYSTATUS:
            {
                sal_Unicode cMod = ' ';
                if (IsModified())
                    cMod = '*';
                rSet.Put(SfxStringItem(SID_MODIFYSTATUS, OUString(cMod)));
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
                    OUString(::svl::IUndoManager:: *fnGetComment)( size_t, bool const ) const;

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
                        OUString sList;
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            sList += (pTmpUndoMgr->*fnGetComment)( n, ::svl::IUndoManager::TopLevel ) + "\n";

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
    SAL_INFO( "starmath", "starmath: SmDocShell::GetUndoManager" );

    if (!pEditEngine)
        GetEditEngine();
    return &pEditEngine->GetUndoManager();
}


void SmDocShell::SaveSymbols()
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SaveSymbols" );

    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Save();
}


void SmDocShell::Draw(OutputDevice *pDevice,
                      const JobSetup &,
                      sal_uInt16 /*nAspect*/)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::Draw" );

    pDevice->IntersectClipRegion(GetVisArea());
    Point atmppoint;
    DrawFormula(*pDevice, atmppoint);
}

SfxItemPool& SmDocShell::GetPool() const
{
    return SFX_APP()->GetPool();
}

void SmDocShell::SetVisArea(const Rectangle & rVisArea)
{
    SAL_INFO( "starmath", "starmath: SmDocShell::SetVisArea" );

    Rectangle aNewRect(rVisArea);

    aNewRect.SetPos(Point());

    if (! aNewRect.Right()) aNewRect.Right() = 2000;
    if (! aNewRect.Bottom()) aNewRect.Bottom() = 1000;

    bool bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( false );

    //TODO/LATER: it's unclear how this interacts with the SFX code
    // If outplace editing, then dont resize the OutplaceWindow. But the
    // ObjectShell has to resize. Bug 56470
    bool bUnLockFrame;
    if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !IsInPlaceActive() && GetFrame() )
    {
        GetFrame()->LockAdjustPosSizePixel();
        bUnLockFrame = true;
    }
    else
        bUnLockFrame = false;

    SfxObjectShell::SetVisArea( aNewRect );

    if( bUnLockFrame )
        GetFrame()->UnlockAdjustPosSizePixel();

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


void SmDocShell::FillClass(SvGlobalName* pClassName,
                           sal_uInt32*  pFormat,
                           OUString* /*pAppName*/,
                           OUString* pFullTypeName,
                           OUString* pShortTypeName,
                           sal_Int32 nFileFormat,
                           sal_Bool bTemplate /* = sal_False */) const
{
    SAL_INFO( "starmath", "starmath: SmDocShell::FillClass" );

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
    SAL_INFO( "starmath", "starmath: SmDocShell::SetModified" );

    if( IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
    }
}

bool SmDocShell::WriteAsMathType3( SfxMedium& rMedium )
{
    SAL_INFO( "starmath", "starmath: SmDocShell::WriteAsMathType3" );

    MathType aEquation( aText, pTree );

    bool bRet = 0 != aEquation.ConvertFromStarMath( rMedium );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
