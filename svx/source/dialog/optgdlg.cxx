/*************************************************************************
 *
 *  $RCSfile: optgdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 13:07:46 $
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

#pragma hdrstop

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SFXSZITEM_HXX
#include <svtools/szitem.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXIMGMGR_HXX
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_CACHEOPTIONS_HXX
#include <svtools/cacheoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS3D_HXX
#include <svtools/options3d.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_FONTOPTIONS_HXX
#include <svtools/fontoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCPOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PRINTOPTIONS_HXX
#include <svtools/printwarningoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#include <svtools/syslocaleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif

#include <unotools/localfilehelper.hxx>

#ifndef _SVX_CUIOPTGENRL_HXX
#include "cuioptgenrl.hxx"
#endif
#ifndef _SVX_OPTPATH_HXX
#include "optpath.hxx"
#endif
#ifndef _SVX_OPTSAVE_HXX
#include "optsave.hxx"
#endif
#ifndef _SVX_OPTLINGU_HXX
#include "optlingu.hxx"
#endif
#ifndef _XPOOL_HXX
#include "xpool.hxx"
#endif
#ifndef _SVX_DLGUTIL_HXX
#include "dlgutil.hxx"
#endif
#ifndef _SVX_TAB_AREA_HXX
#include "cuitabarea.hxx"
#endif
#ifndef _SVX_LANGTAB_HXX //autogen
#include "langtab.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif
#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif
#define ITEMID_LANGUAGE 0
#ifndef _SVX_LANGITEM_HXX
#include "langitem.hxx"
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include "dialmgr.hxx"
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#include "optgdlg.hrc"
#include "optgdlg.hxx"
#include "ofaitem.hxx"
#ifndef _SVT_APEARCFG_HXX
#include <svtools/apearcfg.hxx>
#endif

#define CONFIG_LANGUAGES "OfficeLanguages"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::utl;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define MAX_PROGRAM_ENTRIES     3

// class OfaMiscTabPage --------------------------------------------------

int OfaMiscTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

OfaMiscTabPage::OfaMiscTabPage(Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, ResId( OFA_TP_MISC, DIALOG_MGR() ), rSet ),

    aHelpFL             ( this, ResId( FL_HELP ) ),
    aToolTipsCB         ( this, ResId( CB_TOOLTIP ) ),
    aExtHelpCB          ( this, ResId( CB_EXTHELP ) ),
    aHelpAgentCB        ( this, ResId( CB_HELPAGENT ) ),
    aHelpAgentResetBtn  ( this, ResId( PB_HELPAGENT_RESET ) ),
    aHelpFormatFT       ( this, ResId( FT_HELPFORMAT ) ),
    aHelpFormatLB       ( this, ResId( LB_HELPFORMAT ) ),
    aFileDlgFL          ( this, ResId( FL_FILEDLG ) ),
    aFileDlgCB          ( this, ResId( CB_FILEDLG ) ),
    aDocStatusFL        ( this, ResId( FL_DOCSTATUS ) ),
    aDocStatusCB        ( this, ResId( CB_DOCSTATUS ) ),
    aTwoFigureFL        ( this, ResId( FL_TWOFIGURE ) ),
    aInterpretFT        ( this, ResId( FT_INTERPRET ) ),
    aYearValueField     ( this, ResId( NF_YEARVALUE ) ),
    aToYearFT           ( this, ResId( FT_TOYEAR ) )

{
    FreeResource();

    //quick launch only available in Win
#ifndef WNT
    aFileDlgFL.Hide();
    aFileDlgCB.Hide();
#endif

    // at least the button is as wide as its text
    long nTxtWidth = aHelpAgentResetBtn.GetTextWidth( aHelpAgentResetBtn.GetText() );
    Size aBtnSz = aHelpAgentResetBtn.GetSizePixel();
    if ( aBtnSz.Width() < nTxtWidth )
    {
        aBtnSz.Width() = nTxtWidth;
        aHelpAgentResetBtn.SetSizePixel( aBtnSz );
    }

    aStrDateInfo = aToYearFT.GetText();
    aYearValueField.SetModifyHdl( LINK( this, OfaMiscTabPage, TwoFigureHdl ) );
    Link aLink = LINK( this, OfaMiscTabPage, TwoFigureConfigHdl );
    aYearValueField.SetDownHdl( aLink );
    aYearValueField.SetUpHdl( aLink );
    aYearValueField.SetLoseFocusHdl( aLink );
    aYearValueField.SetFirstHdl( aLink );
    TwoFigureConfigHdl( &aYearValueField );

    SetExchangeSupport();

    aLink = LINK( this, OfaMiscTabPage, HelpCheckHdl_Impl );
    aToolTipsCB.SetClickHdl( aLink );
    aHelpAgentCB.SetClickHdl( aLink );
    aHelpAgentResetBtn.SetClickHdl( LINK( this, OfaMiscTabPage, HelpAgentResetHdl_Impl ) );

    //fill default names as user data
    static const char* aHelpFormatNames[] =
    {
        "Default",
        "HighContrast1",
        "HighContrast2",
        "HighContrastBlack",
        "HighContrastWhite"
    };

    for ( USHORT i = 0; i < aHelpFormatLB.GetEntryCount(); i++ )
    {
        String* pData = new String( String::CreateFromAscii( aHelpFormatNames[i] ) );
        aHelpFormatLB.SetEntryData( i, pData );
    }
}

// -----------------------------------------------------------------------

OfaMiscTabPage::~OfaMiscTabPage()
{
    for(USHORT i = 0; i < aHelpFormatLB.GetEntryCount(); i++)
    {
        delete static_cast< String* >( aHelpFormatLB.GetEntryData(i) );
    }
}

// -----------------------------------------------------------------------

SfxTabPage* OfaMiscTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaMiscTabPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------

BOOL OfaMiscTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;

    SvtHelpOptions aHelpOptions;
    BOOL bChecked = aToolTipsCB.IsChecked();
    if ( bChecked != aToolTipsCB.GetSavedValue() )
        aHelpOptions.SetHelpTips( bChecked );
    bChecked = ( aExtHelpCB.IsChecked() && aToolTipsCB.IsChecked() );
    if ( bChecked != aExtHelpCB.GetSavedValue() )
        aHelpOptions.SetExtendedHelp( bChecked );
    bChecked = aHelpAgentCB.IsChecked();
    if ( bChecked != aHelpAgentCB.GetSavedValue() )
        aHelpOptions.SetHelpAgentAutoStartMode( bChecked );
    USHORT nHelpFormatPos = aHelpFormatLB.GetSelectEntryPos();
    if ( nHelpFormatPos != LISTBOX_ENTRY_NOTFOUND &&
         nHelpFormatPos != aHelpFormatLB.GetSavedValue() )
    {
        aHelpOptions.SetHelpStyleSheet( *static_cast< String* >( aHelpFormatLB.GetEntryData( nHelpFormatPos ) ) );
    }

    if ( aFileDlgCB.IsChecked() != aFileDlgCB.GetSavedValue() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetUseSystemFileDialog( !aFileDlgCB.IsChecked() );
        bModified = TRUE;
    }

    if ( aDocStatusCB.IsChecked() != aDocStatusCB.GetSavedValue() )
    {
        SvtPrintWarningOptions aPrintOptions;
        aPrintOptions.SetModifyDocumentOnPrintingAllowed( aDocStatusCB.IsChecked() );
        bModified = TRUE;
    }

    const SfxUInt16Item* pUInt16Item =
        PTR_CAST( SfxUInt16Item, GetOldItem( rSet, SID_ATTR_YEAR2000 ) );
    USHORT nNum = (USHORT)aYearValueField.GetText().ToInt32();
    if ( pUInt16Item && pUInt16Item->GetValue() != nNum )
    {
        bModified = TRUE;
        rSet.Put( SfxUInt16Item( SID_ATTR_YEAR2000, nNum ) );
    }

    return bModified;
}

// -----------------------------------------------------------------------

void OfaMiscTabPage::Reset( const SfxItemSet& rSet )
{
    SvtHelpOptions aHelpOptions;
    aToolTipsCB.Check( aHelpOptions.IsHelpTips() );
    aExtHelpCB.Check( aHelpOptions.IsHelpTips() && aHelpOptions.IsExtendedHelp() );
    aHelpAgentCB.Check( aHelpOptions.IsHelpAgentAutoStartMode() );
    String sStyleSheet = aHelpOptions.GetHelpStyleSheet();
    for ( USHORT i = 0; i < aHelpFormatLB.GetEntryCount(); ++i )
    {
        if ( *static_cast< String* >( aHelpFormatLB.GetEntryData(i) ) == sStyleSheet )
        {
            aHelpFormatLB.SelectEntryPos(i);
            break;
        }
    }

    aToolTipsCB.SaveValue();
    aExtHelpCB.SaveValue();
    aHelpAgentCB.SaveValue();
    aHelpFormatLB.SaveValue();
    HelpCheckHdl_Impl( &aHelpAgentCB );

    SvtMiscOptions aMiscOpt;
    aFileDlgCB.Check( !aMiscOpt.UseSystemFileDialog() );
    aFileDlgCB.SaveValue();

    SvtPrintWarningOptions aPrintOptions;
    aDocStatusCB.Check(aPrintOptions.IsModifyDocumentOnPrintingAllowed());
    aDocStatusCB.SaveValue();

    const SfxPoolItem* pItem = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, FALSE, &pItem ) )
    {
        aYearValueField.SetValue( ((SfxUInt16Item*)pItem)->GetValue() );
        TwoFigureConfigHdl( &aYearValueField );
    }
    else
    {
        aYearValueField.Enable(FALSE);
        aTwoFigureFL.Enable(FALSE);
        aInterpretFT.Enable(FALSE);
        aToYearFT.Enable(FALSE);
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, TwoFigureHdl, NumericField*, pEd )
{
    String aOutput( aStrDateInfo );
    String aStr( aYearValueField.GetText() );
    International aInt;
    aStr.EraseAllChars( aInt.GetNumThousandSep() );
    long nNum = aStr.ToInt32();
    if ( aStr.Len() != 4 || nNum < aYearValueField.GetMin() || nNum > aYearValueField.GetMax() )
        aOutput.AppendAscii("????");
    else
    {
        nNum += 99;
        aOutput += String::CreateFromInt32( nNum );
    }
    aToYearFT.SetText( aOutput );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, TwoFigureConfigHdl, NumericField*, pEd )
{
    long nNum = aYearValueField.GetValue();
    String aOutput( String::CreateFromInt32( nNum ) );
    aYearValueField.SetText( aOutput );
    aYearValueField.SetSelection( Selection( 0, aOutput.Len() ) );
    TwoFigureHdl( pEd );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, HelpCheckHdl_Impl, CheckBox*, EMPTYARG )
{
    aExtHelpCB.Enable( aToolTipsCB.IsChecked() );
    aHelpAgentResetBtn.Enable( aHelpAgentCB.IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, HelpAgentResetHdl_Impl, PushButton*, EMPTYARG )
{
    SvtHelpOptions().resetAgentIgnoreURLCounter();
    return 0;
}

// class OfaViewTabPage --------------------------------------------------
// -----------------------------------------------------------------------
IMPL_LINK_INLINE_START( OfaViewTabPage, OpenGLHdl, CheckBox*, EMPTYARG )
{
    a3DOpenGLFasterCB.Enable( a3DOpenGLCB.IsChecked() );
    return 0;
}
IMPL_LINK_INLINE_END( OfaViewTabPage, OpenGLHdl, CheckBox*, EMPTYARG )

OfaViewTabPage::OfaViewTabPage(Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, ResId( OFA_TP_VIEW, DIALOG_MGR() ), rSet ),

    aUserInterfaceFL    ( this, ResId( FL_USERINTERFACE ) ),
    aWindowSizeFT       ( this, ResId( FT_WINDOWSIZE ) ),
    aWindowSizeMF       ( this, ResId( MF_WINDOWSIZE ) ),
    aIconSizeFT              ( this, ResId( FT_ICONSIZE ) ),
    aIconSizeLB              ( this, ResId( LB_ICONSIZE ) ),
    m_aSystemFont               (this, ResId( CB_SYSTEM_FONT ) ),
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    aFontAntiAliasing   ( this, ResId( CB_FONTANTIALIASING )),
    aAAPointLimitLabel  ( this, ResId( FT_POINTLIMIT_LABEL )),
    aAAPointLimit       ( this, ResId( NF_AA_POINTLIMIT )),
    aAAPointLimitUnits  ( this, ResId( FT_POINTLIMIT_UNIT )),
#endif
    aMenuFL             ( this, ResId( FL_MENU ) ),
    aMenuIconsCB        ( this, ResId( CB_MENU_ICONS )),
    aShowInactiveItemsCB( this, ResId( CB_SHOW_INACTIVE ) ),
    aFontListsFL        ( this, ResId( FL_FONTLISTS) ),
    aFontShowCB         ( this, ResId( CB_FONT_SHOW ) ),
    aFontHistoryCB      ( this, ResId( CB_FONT_HISTORY ) ),

    aMouseFL            ( this, ResId( FL_MOUSE ) ),
    aMousePosFT         ( this, ResId( FT_MOUSEPOS ) ),
    aMousePosLB         ( this, ResId( LB_MOUSEPOS ) ),
    aMouseMiddleFT      ( this, ResId( FT_MOUSEMIDDLE ) ),
    aMouseMiddleLB      ( this, ResId( LB_MOUSEMIDDLE ) ),
    a3DGB               ( this, ResId( FL_3D ) ),
    a3DOpenGLCB         ( this, ResId( CB_3D_OPENGL ) ),
    a3DOpenGLFasterCB   ( this, ResId( CB_3D_OPENGL_FASTER ) ),
    a3DDitheringCB      ( this, ResId( CB_3D_DITHERING ) ),
    a3DShowFullCB       ( this, ResId( CB_3D_SHOWFULL ) ),
    aWorkingSetBox      ( this, ResId( FL_WORKINGSET ) ),
    aDocViewBtn         ( this, ResId( BTN_DOCVIEW ) ),
    aOpenWinBtn         ( this, ResId( BTN_OPENWIN ) ),
    pAppearanceCfg(new SvtTabAppearanceCfg)
{
    a3DOpenGLCB.SetClickHdl( LINK( this, OfaViewTabPage, OpenGLHdl ) );

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    aFontAntiAliasing.SetToggleHdl( LINK( this, OfaViewTabPage, OnAntialiasingToggled ) );

    // depending on the size of the text in aAAPointLimitLabel, we slightly re-arrange aAAPointLimit and aAAPointLimitUnits
    //#110391#  if the label has no mnemonic and we are in a CJK version the mnemonic "(X)" will be added which
    //          influences the width calculation
    MnemonicGenerator aMnemonicGenerator;
    String sLabel(aAAPointLimitLabel.GetText());
    aMnemonicGenerator.RegisterMnemonic( sLabel );
    aMnemonicGenerator.CreateMnemonic( sLabel );
    sLabel.EraseAllChars('~');

    sal_Int32 nLabelWidth = aAAPointLimitLabel.GetTextWidth( sLabel );
    nLabelWidth += 3;   // small gap
    // pixels to move both controls to the left
    Size aSize = aAAPointLimitLabel.GetSizePixel();
    sal_Int32 nMoveLeft = aSize.Width() - nLabelWidth;
    // resize the first label
    aSize.Width() = nLabelWidth;
    aAAPointLimitLabel.SetSizePixel( aSize );

    // move the numeric field
    Point aPos( aAAPointLimit.GetPosPixel() );
    aPos.X() -= nMoveLeft;
    aAAPointLimit.SetPosPixel( aPos );

    // move (and resize) the units FixedText
    aPos = ( aAAPointLimitUnits.GetPosPixel() );
    aPos.X() -= nMoveLeft;
    aSize = aAAPointLimitUnits.GetSizePixel();
    aSize.Width() += nMoveLeft;
    aAAPointLimitUnits.SetPosSizePixel( aPos, aSize );
#else
    // on this platform, we do not have the anti aliasing options - move the other checkboxes accordingly
    // (in the resource, the coordinates are calculated for the AA options beeing present)
    Control* pMiscOptions[] =
    {
        &aMenuFL, &aFontShowCB, &aShowInactiveItemsCB,
        &aFontListsFL, &aFontHistoryCB, &aMenuIconsCB
    };

    // temporaryly create the checkbox for the anti aliasing (we need to to determine it's pos)
    CheckBox* pFontAntiAliasing = new CheckBox( this, ResId( CB_FONTANTIALIASING ) );
    sal_Int32 nMoveUp = aMenuFL.GetPosPixel().Y() - pFontAntiAliasing->GetPosPixel().Y();
    DELETEZ( pFontAntiAliasing );

    Point aPos;
    for ( sal_Int32 i = 0; i < sizeof( pMiscOptions ) / sizeof( pMiscOptions[0] ); ++i )
    {
        aPos = pMiscOptions[i]->GetPosPixel( );
        aPos.Y() -= nMoveUp;
        pMiscOptions[i]->SetPosPixel( aPos );
    }

#endif
    FreeResource();
    if( ! Application::ValidateSystemFont() )
    {
        m_aSystemFont.Check( FALSE );
        m_aSystemFont.Enable( FALSE );
    }
}

OfaViewTabPage::~OfaViewTabPage()
{
    delete pAppearanceCfg;
}

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
//--- 20.08.01 10:16:12 ---------------------------------------------------
IMPL_LINK( OfaViewTabPage, OnAntialiasingToggled, void*, NOTINTERESTEDIN )
{
    sal_Bool bAAEnabled = aFontAntiAliasing.IsChecked();

    aAAPointLimitLabel.Enable( bAAEnabled );
    aAAPointLimit.Enable( bAAEnabled );
    aAAPointLimitUnits.Enable( bAAEnabled );

    return 0L;
}
#endif

/*-----------------06.12.96 11.50-------------------

--------------------------------------------------*/

SfxTabPage* OfaViewTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaViewTabPage(pParent, rAttrSet);
}

/*-----------------06.12.96 11.50-------------------

--------------------------------------------------*/

BOOL OfaViewTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvtFontOptions aFontOpt;
    SvtMenuOptions aMenuOpt;
    SvtStartOptions aStartOpt;

    BOOL bModified = FALSE;
    BOOL bMenuOptModified = FALSE;

    SvtMiscOptions aMiscOptions;
    UINT16 nBigLB_NewSelection = aIconSizeLB.GetSelectEntryPos();
    if( nBigLB_InitialSelection != nBigLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        sal_Int16 eSet = SFX_SYMBOLS_AUTO;
        switch( nBigLB_NewSelection )
        {
            case 0: eSet = SFX_SYMBOLS_AUTO;  break;
            case 1: eSet = SFX_SYMBOLS_SMALL; break;
            case 2: eSet = SFX_SYMBOLS_LARGE; break;
            default:
                DBG_ERROR( "OfaViewTabPage::FillItemSet(): This state should not be possible!" );
        }
        aMiscOptions.SetSymbolSet( eSet );
    }

    BOOL bAppearanceChanged = FALSE;


    // Screen Scaling
    UINT16 nOldScale = pAppearanceCfg->GetScaleFactor();
    UINT16 nNewScale = (UINT16)aWindowSizeMF.GetValue();

    if ( nNewScale != nOldScale )
    {
        pAppearanceCfg->SetScaleFactor(nNewScale);
        bAppearanceChanged = TRUE;
    }

    // Mouse Snap Mode
    short eOldSnap = pAppearanceCfg->GetSnapMode();
    short eNewSnap = aMousePosLB.GetSelectEntryPos();
    if(eNewSnap > 2)
        eNewSnap = 2;

    if ( eNewSnap != eOldSnap )
    {
        pAppearanceCfg->SetSnapMode(eNewSnap );
        bAppearanceChanged = TRUE;
    }

    // Middle Mouse Button
    short eOldMiddleMouse = pAppearanceCfg->GetMiddleMouseButton();
    short eNewMiddleMouse = aMouseMiddleLB.GetSelectEntryPos();
    if(eNewMiddleMouse > 2)
        eNewMiddleMouse = 2;

    if ( eNewMiddleMouse != eOldMiddleMouse )
    {
        pAppearanceCfg->SetMiddleMouseButton( eNewMiddleMouse );
        bAppearanceChanged = TRUE;
    }

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    if ( aFontAntiAliasing.IsChecked() != aFontAntiAliasing.GetSavedValue() )
    {
        pAppearanceCfg->SetFontAntiAliasing( aFontAntiAliasing.IsChecked() );
        bAppearanceChanged = TRUE;
    }

    if ( aAAPointLimit.GetValue() != aAAPointLimit.GetSavedValue().ToInt32() )
    {
        pAppearanceCfg->SetFontAntialiasingMinPixelHeight( aAAPointLimit.GetValue() );
        bAppearanceChanged = TRUE;
    }
#endif

    if ( aFontShowCB.IsChecked() != aFontShowCB.GetSavedValue() )
    {
        aFontOpt.EnableFontWYSIWYG( aFontShowCB.IsChecked() );
        bModified = TRUE;
    }

    if ( aShowInactiveItemsCB.IsChecked() != aShowInactiveItemsCB.GetSavedValue() )
    {
        aMenuOpt.SetEntryHidingState( aShowInactiveItemsCB.IsChecked() );
        bModified = TRUE;
    }

    if(aMenuIconsCB.IsChecked() != aMenuIconsCB.GetSavedValue())
    {
        aMenuOpt.SetMenuIconsState( aMenuIconsCB.IsChecked() );
        bModified = TRUE;
        bMenuOptModified = TRUE;
    }

    if ( aFontHistoryCB.IsChecked() != aFontHistoryCB.GetSavedValue() )
    {
        aFontOpt.EnableFontHistory( aFontHistoryCB.IsChecked() );
        bModified = TRUE;
    }
    if ( bAppearanceChanged )
    {
        pAppearanceCfg->Commit();
        pAppearanceCfg->SetApplicationDefaults ( GetpApp() );
    }
    // Workingset
    SvtSaveOptions aSaveOptions;
    if ( aDocViewBtn.IsChecked() != aDocViewBtn.GetSavedValue() )
    {
        aSaveOptions.SetSaveDocView(aDocViewBtn.IsChecked());
    }

    if ( aOpenWinBtn.IsChecked() != aOpenWinBtn.GetSavedValue() )
    {
        aSaveOptions.SetSaveDocWins(aOpenWinBtn.IsChecked() );
    }
    SvtOptions3D a3DOpt;
    BOOL bTemp = a3DOpt.IsOpenGL();

    if ( bTemp != a3DOpenGLCB.IsChecked() )
    {
        a3DOpt.SetOpenGL( a3DOpenGLCB.IsChecked() );
        bModified = TRUE;
    }

    BOOL bCheck = ( a3DOpenGLCB.IsChecked() && a3DOpenGLFasterCB.IsChecked() );
    if ( a3DOpt.IsOpenGL_Faster() != bCheck )
    {
        a3DOpt.SetOpenGL_Faster( bCheck );
        bModified = TRUE;
    }

    if ( a3DOpt.IsDithering() != a3DDitheringCB.IsChecked() )
    {
        a3DOpt.SetDithering( a3DDitheringCB.IsChecked() );
        bModified = TRUE;
    }

    if ( a3DOpt.IsShowFull() != a3DShowFullCB.IsChecked() )
    {
        a3DOpt.SetShowFull( a3DShowFullCB.IsChecked() );
        bModified = TRUE;
    }

    SvtAccessibilityOptions     aAccessibilityOptions;
    if( aAccessibilityOptions.GetIsSystemFont() != m_aSystemFont.IsChecked() &&
        m_aSystemFont.IsEnabled() )
    {
        aAccessibilityOptions.SetIsSystemFont( m_aSystemFont.IsChecked() );
        bModified = TRUE;
        bMenuOptModified = TRUE;
    }

    if( bMenuOptModified )
    {
        // Set changed settings to the application instance
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        aStyleSettings.SetUseImagesInMenus( aMenuIconsCB.IsChecked() );
        if( m_aSystemFont.IsEnabled() )
            aStyleSettings.SetUseSystemUIFonts( m_aSystemFont.IsChecked() );
        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings( aAllSettings );
        Application::SetSettings(aAllSettings);
    }

    return bModified;
}

/*-----------------06.12.96 11.50-------------------

--------------------------------------------------*/

void OfaViewTabPage::Reset( const SfxItemSet& rSet )
{

    SvtOptions3D a3DOpt;
    a3DOpenGLCB.Check( a3DOpt.IsOpenGL() );
    a3DOpenGLFasterCB.Check( a3DOpenGLCB.IsChecked() && a3DOpt.IsOpenGL_Faster() );
    OpenGLHdl( NULL );
    a3DDitheringCB.Check( a3DOpt.IsDithering() );
    a3DShowFullCB.Check( a3DOpt.IsShowFull() );

    SvtMiscOptions aMiscOptions;

    if( aMiscOptions.GetSymbolSet() != SFX_SYMBOLS_AUTO )
        nBigLB_InitialSelection = ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE )? 2 : 1;
    aIconSizeLB.SelectEntryPos( nBigLB_InitialSelection );
    aIconSizeLB.SaveValue();

    if( m_aSystemFont.IsEnabled() )
    {
        SvtAccessibilityOptions aAccessibilityOptions;
        m_aSystemFont.Check( aAccessibilityOptions.GetIsSystemFont() );
    }

    // Screen Scaling
    aWindowSizeMF.SetValue ( pAppearanceCfg->GetScaleFactor() );
    // Mouse Snap
    aMousePosLB.SelectEntryPos(pAppearanceCfg->GetSnapMode());
    aMousePosLB.SaveValue();

    // Mouse Snap
    aMouseMiddleLB.SelectEntryPos(pAppearanceCfg->GetMiddleMouseButton());
    aMouseMiddleLB.SaveValue();

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    aFontAntiAliasing.Check( pAppearanceCfg->IsFontAntiAliasing() );
    aAAPointLimit.SetValue( pAppearanceCfg->GetFontAntialiasingMinPixelHeight() );
#endif

    // WorkingSet
    SvtSaveOptions aSaveOptions;
    aDocViewBtn.Enable(!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEDOCVIEW));
    aDocViewBtn.Check( aSaveOptions.IsSaveDocView() );

    aOpenWinBtn.Enable(!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEDOCWINS));
    aOpenWinBtn.Check( aSaveOptions.IsSaveDocWins() );

    SvtFontOptions aFontOpt;
    aFontShowCB.Check( aFontOpt.IsFontWYSIWYGEnabled() );
    SvtMenuOptions aMenuOpt;
    aShowInactiveItemsCB.Check( aMenuOpt.IsEntryHidingEnabled() );
    aMenuIconsCB.Check(aMenuOpt.IsMenuIconsEnabled());
    aMenuIconsCB.SaveValue();

    aFontHistoryCB.Check( aFontOpt.IsFontHistoryEnabled() );

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    aFontAntiAliasing.SaveValue();
    aAAPointLimit.SaveValue();
#endif
    aFontShowCB.SaveValue();
    aShowInactiveItemsCB.SaveValue();
    aFontHistoryCB.SaveValue();
    aDocViewBtn.SaveValue();
    aOpenWinBtn.SaveValue();

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    LINK( this, OfaViewTabPage, OnAntialiasingToggled ).Call( NULL );
#endif
}
/* -----------------------------23.11.00 14:55--------------------------------

 ---------------------------------------------------------------------------*/
class LangConfigItem_Impl : public ConfigItem
{
    Any         aValue;
    OUString    aPropertyName;
public:
    LangConfigItem_Impl(const OUString& rTree, const OUString& rProperty);
    ~LangConfigItem_Impl();

    virtual void            Commit();

    const Any&  GetValue() const {return aValue;}
    void        SetValue(Any& rValue)  {aValue = rValue; SetModified();}
};
/* -----------------------------23.11.00 15:06--------------------------------

 ---------------------------------------------------------------------------*/
LangConfigItem_Impl::LangConfigItem_Impl(
    const OUString& rTree, const OUString& rProperty) :
    ConfigItem(rTree),
    aPropertyName(rProperty)
{
    Sequence<OUString> aNames(1);
    aNames.getArray()[0] = aPropertyName;
    Sequence<Any> aValues = GetProperties(aNames);
    aValue = aValues.getConstArray()[0];
}
/* -----------------------------23.11.00 15:06--------------------------------

 ---------------------------------------------------------------------------*/
LangConfigItem_Impl::~LangConfigItem_Impl()
{}
/* -----------------------------23.11.00 15:10--------------------------------

 ---------------------------------------------------------------------------*/
void LangConfigItem_Impl::Commit()
{
    Sequence<OUString> aNames(1);
    aNames.getArray()[0] = aPropertyName;
    Sequence<Any> aValues(1);
    aValues.getArray()[0] = aValue;
    PutProperties(aNames, aValues);
}
/* -----------------22.07.2003 10:33-----------------

 --------------------------------------------------*/
struct LanguageConfig_Impl
{
    SvtLanguageOptions aLanguageOptions;
    SvtSysLocaleOptions aSysLocaleOptions;
    SvtLinguConfig aLinguConfig;
};
/* -----------------------------23.11.00 13:06--------------------------------

 ---------------------------------------------------------------------------*/
static sal_Bool bLanguageCurrentDoc_Impl = sal_False;
OfaLanguagesTabPage::OfaLanguagesTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, ResId( OFA_TP_LANGUAGES, DIALOG_MGR() ), rSet ),
    aUILanguageGB(this,         ResId(FL_UI_LANG        )),
    aLocaleSettingFI(this,      ResId(FI_LOCALESETTING)),
    aLocaleSettingFT(this,      ResId(FT_LOCALESETTING)),
    aLocaleSettingLB(this,      ResId(LB_LOCALESETTING)),
    aCurrencyFI( this,          ResId(FI_CURRENCY       )),
    aDecimalSeparatorFT(this,   ResId(FT_DECIMALSEPARATOR)),
    aDecimalSeparatorCB(this,   ResId(CB_DECIMALSEPARATOR)),
    aCurrencyFT( this,          ResId(FT_CURRENCY       )),
    aCurrencyLB( this,          ResId(LB_CURRENCY       )),
    aLinguLanguageGB(this,      ResId(FL_LINGU_LANG     )),
    aWesternLanguageFI(this,    ResId(FI_WEST_LANG      )),
    aWesternLanguageFT(this,    ResId(FT_WEST_LANG      )),
    aWesternLanguageLB(this,    ResId(LB_WEST_LANG      )),
    aAsianLanguageFI(this,      ResId(FI_ASIAN_LANG     )),
    aAsianLanguageFT(this,      ResId(FT_ASIAN_LANG     )),
    aAsianLanguageLB(this,      ResId(LB_ASIAN_LANG     )),
    aComplexLanguageFI(this,    ResId(FI_COMPLEX_LANG   )),
    aComplexLanguageFT(this,    ResId(FT_COMPLEX_LANG   )),
    aComplexLanguageLB(this,    ResId(LB_COMPLEX_LANG   )),
    aCurrentDocCB(this,         ResId(CB_CURRENT_DOC    )),
    aAsianSupportFI(this,       ResId(FI_ASIANSUPPORT   )),
    aAsianSupportFL(this,       ResId(FL_ASIANSUPPORT    )),
    aAsianSupportCB(this,       ResId(CB_ASIANSUPPORT   )),
    aCTLSupportFI(this,         ResId(FI_CTLSUPPORT    )),
    aCTLSupportFL(this,         ResId(FL_CTLSUPPORT    )),
    aCTLSupportCB(this,         ResId(CB_CTLSUPPORT   )),
    pLangConfig(new LanguageConfig_Impl)
{
    FreeResource();

    sal_Bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    aWesternLanguageLB.SetLanguageList( LANG_LIST_WESTERN | LANG_LIST_ONLY_KNOWN, TRUE,  FALSE, TRUE );
    aAsianLanguageLB  .SetLanguageList( LANG_LIST_CJK     | LANG_LIST_ONLY_KNOWN, TRUE,  FALSE, TRUE );
    aComplexLanguageLB.SetLanguageList( LANG_LIST_CTL     | LANG_LIST_ONLY_KNOWN, TRUE,  FALSE, TRUE );
    aLocaleSettingLB  .SetLanguageList( LANG_LIST_ALL     | LANG_LIST_ONLY_KNOWN, FALSE, FALSE, FALSE);
    aLocaleSettingLB.InsertLanguage( LANGUAGE_SYSTEM );

    SvxLanguageTable* pLanguageTable = new SvxLanguageTable;
    // insert SYSTEM entry, no specific currency
    String aStr( pLanguageTable->GetString( LANGUAGE_SYSTEM ) );
    aCurrencyLB.InsertEntry( aStr );
    // all currencies
    String aTwoSpace( RTL_CONSTASCII_USTRINGPARAM( "  " ) );
    const NfCurrencyTable& rCurrTab = SvNumberFormatter::GetTheCurrencyTable();
    USHORT nCurrCount = rCurrTab.Count();
    // first entry is SYSTEM, skip it
    for ( USHORT j=1; j < nCurrCount; ++j )
    {
        const NfCurrencyEntry* pCurr = rCurrTab[j];
        String aStr( pCurr->GetBankSymbol() );
        aStr += aTwoSpace;
        aStr += pCurr->GetSymbol();
        aStr += aTwoSpace;
        aStr += pLanguageTable->GetString( pCurr->GetLanguage() );
        USHORT nPos = aCurrencyLB.InsertEntry( aStr );
        aCurrencyLB.SetEntryData( nPos, (void*) pCurr );
    }
    delete pLanguageTable;

    aLocaleSettingLB.SetSelectHdl( LINK( this, OfaLanguagesTabPage, LocaleSettingHdl ) );
    Link aLink( LINK( this, OfaLanguagesTabPage, SupportHdl ) );
    aAsianSupportCB.SetClickHdl( aLink );
    aCTLSupportCB.SetClickHdl( aLink );

    aAsianSupportCB.Check( m_bOldAsian = pLangConfig->aLanguageOptions.IsAnyEnabled() );
    aAsianSupportCB.SaveValue();
    sal_Bool bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK);
    aAsianSupportCB.Enable(!bReadonly);
    aAsianSupportFI.Show(bReadonly);
    SupportHdl( &aAsianSupportCB );

    aCTLSupportCB.Check( m_bOldCtl = pLangConfig->aLanguageOptions.IsCTLFontEnabled() );
    aCTLSupportCB.SaveValue();
    bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT);
    aCTLSupportCB.Enable(!bReadonly);
    aCTLSupportFI.Show(bReadonly);
    SupportHdl( &aCTLSupportCB );
}
/*-- 23.11.00 13:06:40---------------------------------------------------

  -----------------------------------------------------------------------*/
OfaLanguagesTabPage::~OfaLanguagesTabPage()
{
    delete pLangConfig;
}
/*-- 23.11.00 13:06:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* OfaLanguagesTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaLanguagesTabPage(pParent, rAttrSet);
}
/*-- 23.11.00 13:06:41---------------------------------------------------

  -----------------------------------------------------------------------*/
LanguageType lcl_LangStringToLangType(const OUString& rLang)
{
    Locale aLocale;
    aLocale.Language = rLang.copy(0, 2);
    if(rLang.getLength() >= 5)
        aLocale.Country = rLang.copy(3, 2);
    LanguageType eLangType = SvxLocaleToLanguage( aLocale );
    return eLangType;
}

/*-----------------12.02.01 09:01-------------------
 *
 * --------------------------------------------------*/
void lcl_LanguageToLocale( Locale& rLocale, const LanguageType eLang )
{
    OUString aEmpty;
    SvxLanguageToLocale( rLocale, eLang );
    switch ( eLang )
    {
        case LANGUAGE_FRENCH :
        case LANGUAGE_GERMAN :
        case LANGUAGE_ITALIAN :
        case LANGUAGE_DUTCH :
        case LANGUAGE_SPANISH :
        case LANGUAGE_PORTUGUESE :
        case LANGUAGE_DANISH :
        case LANGUAGE_GREEK :
        case LANGUAGE_JAPANESE :
        case LANGUAGE_KOREAN :
        case LANGUAGE_SWEDISH :
        case LANGUAGE_RUSSIAN :
        case LANGUAGE_POLISH :
        case LANGUAGE_TURKISH :
            rLocale.Country = aEmpty;
            break;
    }

}
/*-- 23.11.00 13:06:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_UpdateAndDelete(SfxVoidItem* pInvalidItems[], SfxBoolItem* pBoolItems[], sal_uInt16 nCount)
{
    SfxViewFrame* pCurrentFrm = SfxViewFrame::Current();
    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while(pViewFrm)
    {
        SfxBindings& rBind = pViewFrm->GetBindings();
        for(sal_Int16 i = 0; i < nCount; i++)
        {
            if(pCurrentFrm == pViewFrm)
                rBind.InvalidateAll(sal_False);
            rBind.SetState( *pInvalidItems[i] );
            rBind.SetState( *pBoolItems[i] );
        }
        pViewFrm = SfxViewFrame::GetNext(*pViewFrm);
    }
    for(sal_Int16 i = 0; i < nCount; i++)
    {
        delete pInvalidItems[i];
        delete pBoolItems[i] ;
    }
}

BOOL OfaLanguagesTabPage::FillItemSet( SfxItemSet& rSet )
{
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( TRUE );

    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    LanguageType eOldLocale = (sLang.getLength() ?
        lcl_LangStringToLangType( sLang ) : LANGUAGE_SYSTEM);
    LanguageType eNewLocale = aLocaleSettingLB.GetSelectLanguage();
    if ( eOldLocale != eNewLocale )
    {
        // an empty string denotes SYSTEM locale
        OUString sNewLang;
        if ( eNewLocale != LANGUAGE_SYSTEM )
        {
            Locale aLocale;
            // Do NOT use lcl_LanguageToLocale() or locale won't match I18N
            SvxLanguageToLocale( aLocale, eNewLocale );
            sNewLang = aLocale.Language;
            if ( aLocale.Country.getLength() > 0 )
            {
                sNewLang += C2U("-");
                sNewLang += aLocale.Country;
            }
        }
        // Set application settings before options, so listeners at the
        // options will access the new locale.
        AllSettings aSettings( Application::GetSettings() );
        aSettings.SetLanguage( eNewLocale );
        Application::SetSettings( aSettings );
        pLangConfig->aSysLocaleOptions.SetLocaleConfigString( sNewLang );
        rSet.Put( SfxBoolItem( SID_OPT_LOCALE_CHANGED, TRUE ) );
    }

    //
    if(aDecimalSeparatorCB.GetSavedValue() != aDecimalSeparatorCB.IsChecked())
        pLangConfig->aSysLocaleOptions.SetDecimalSeparatorAsLocale(aDecimalSeparatorCB.IsChecked());

    // Configured currency, for example, USD-en-US or EUR-de-DE, or empty for
    // locale default. This must be set _after_ the locale above in order to
    // have a valid locale for broadcasting the currency change.
    OUString sOldCurr = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    USHORT nCurrPos = aCurrencyLB.GetSelectEntryPos();
    const NfCurrencyEntry* pCurr = (const NfCurrencyEntry*)
        aCurrencyLB.GetEntryData( nCurrPos );
    OUString sNewCurr;
    if ( pCurr )
        sNewCurr = SvtSysLocaleOptions::CreateCurrencyConfigString(
            pCurr->GetBankSymbol(), pCurr->GetLanguage() );
    if ( sOldCurr != sNewCurr )
        pLangConfig->aSysLocaleOptions.SetCurrencyConfigString( sNewCurr );

    BOOL bRet = FALSE;
    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    Reference< XPropertySet > xLinguProp( LinguMgr::GetLinguPropertySet(), UNO_QUERY );
    BOOL bCurrentDocCBChecked = aCurrentDocCB.IsChecked();
    if(aCurrentDocCB.IsEnabled())
        bLanguageCurrentDoc_Impl = bCurrentDocCBChecked;
    BOOL bCurrentDocCBChanged = bCurrentDocCBChecked != aCurrentDocCB.GetSavedValue();

    BOOL bValChanged = aWesternLanguageLB.GetSavedValue() != aWesternLanguageLB.GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = aWesternLanguageLB.GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = SvxCreateLocale( eSelectLang );
            aValue <<= aLocale;
            OUString aPropName( C2U("DefaultLocale") );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setPropertyValue( aPropName, aValue );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(eSelectLang, SID_ATTR_LANGUAGE));
            bRet = TRUE;
        }
    }
    bValChanged = aAsianLanguageLB.GetSavedValue() != aAsianLanguageLB.GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = aAsianLanguageLB.GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = SvxCreateLocale( eSelectLang );
            aValue <<= aLocale;
            OUString aPropName( C2U("DefaultLocale_CJK") );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setPropertyValue( aPropName, aValue );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(eSelectLang, SID_ATTR_CHAR_CJK_LANGUAGE));
            bRet = TRUE;
        }
    }
    bValChanged = aComplexLanguageLB.GetSavedValue() != aComplexLanguageLB.GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = aComplexLanguageLB.GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = SvxCreateLocale( eSelectLang );
            aValue <<= aLocale;
            OUString aPropName( C2U("DefaultLocale_CTL") );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setPropertyValue( aPropName, aValue );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(eSelectLang, SID_ATTR_CHAR_CTL_LANGUAGE));
            bRet = TRUE;
        }
    }

    if(aAsianSupportCB.GetSavedValue() != aAsianSupportCB.IsChecked() )
    {
        sal_Bool bChecked = aAsianSupportCB.IsChecked();
        pLangConfig->aLanguageOptions.SetAll(bChecked);
        //iterate over all bindings to invalidate vertical text direction

      const sal_uInt16 STATE_COUNT = 2;

      SfxBoolItem* pBoolItems[STATE_COUNT];
      pBoolItems[0] = new SfxBoolItem(SID_VERTICALTEXT_STATE, FALSE);
      pBoolItems[1] = new SfxBoolItem(SID_TEXT_FITTOSIZE_VERTICAL, FALSE);

      SfxVoidItem* pInvalidItems[STATE_COUNT];
      pInvalidItems[0] = new SfxVoidItem(SID_VERTICALTEXT_STATE);
      pInvalidItems[1] = new SfxVoidItem(SID_TEXT_FITTOSIZE_VERTICAL);

    lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( aCTLSupportCB.GetSavedValue() != aCTLSupportCB.IsChecked() )
    {
        pLangConfig->aLanguageOptions.SetCTLFontEnabled( aCTLSupportCB.IsChecked() );

        const sal_uInt16 STATE_COUNT = 1;
        SfxBoolItem* pBoolItems[STATE_COUNT];
        pBoolItems[0] = new SfxBoolItem(SID_CTLFONT_STATE, FALSE);
        SfxVoidItem* pInvalidItems[STATE_COUNT];
        pInvalidItems[0] = new SfxVoidItem(SID_CTLFONT_STATE);
        lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( pLangConfig->aSysLocaleOptions.IsModified() )
        pLangConfig->aSysLocaleOptions.Commit();
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( FALSE );

    return FALSE;
}
//-----------------------------------------------------------------------------
void OfaLanguagesTabPage::Reset( const SfxItemSet& rSet )
{
    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    if ( sLang.getLength() )
        aLocaleSettingLB.SelectLanguage(lcl_LangStringToLangType(sLang));
    else
        aLocaleSettingLB.SelectLanguage( LANGUAGE_SYSTEM );
    sal_Bool bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_LOCALE);
    aLocaleSettingLB.Enable(!bReadonly);
    aLocaleSettingFT.Enable(!bReadonly);
    aLocaleSettingFI.Show(bReadonly);

    //
    aDecimalSeparatorCB.Check( pLangConfig->aSysLocaleOptions.IsDecimalSeparatorAsLocale());
    aDecimalSeparatorCB.SaveValue();

    // let LocaleSettingHdl enable/disable checkboxes for CJK/CTL support
    // #i15812# must be done *before* the configured currency is set
    // and update the decimal separator used for the given locale
    LocaleSettingHdl(&aLocaleSettingLB);

    // configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default
    String aAbbrev;
    LanguageType eLang;
    const NfCurrencyEntry* pCurr = NULL;
    sLang = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    if ( sLang.getLength() )
    {
        SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( aAbbrev, eLang, sLang );
        pCurr = SvNumberFormatter::GetCurrencyEntry( aAbbrev, eLang );
    }
    // if pCurr==NULL the SYSTEM entry is selected
    USHORT nPos = aCurrencyLB.GetEntryPos( (void*) pCurr );
    aCurrencyLB.SelectEntryPos( nPos );
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_CURRENCY);
    aCurrencyLB.Enable(!bReadonly);
    aCurrencyFT.Enable(!bReadonly);
    aCurrencyFI.Show(bReadonly);

    //western/CJK/CLK language
    LanguageType eCurLang = LANGUAGE_NONE;
    LanguageType eCurLangCJK = LANGUAGE_NONE;
    LanguageType eCurLangCTL = LANGUAGE_NONE;
    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    //collect the configuration values first
    aCurrentDocCB.Enable(FALSE);
    //
    Any aWestLang;
    Any aCJKLang;
    Any aCTLLang;
    try
    {
        aWestLang = pLangConfig->aLinguConfig.GetProperty(C2U("DefaultLocale"));
        Locale aLocale;
        aWestLang >>= aLocale;
        eCurLang = SvxLocaleToLanguage( aLocale );
        aCJKLang = pLangConfig->aLinguConfig.GetProperty(C2U("DefaultLocale_CJK"));
        aCJKLang >>= aLocale;
        eCurLangCJK = SvxLocaleToLanguage( aLocale );
        aCTLLang = pLangConfig->aLinguConfig.GetProperty(C2U("DefaultLocale_CTL"));
        aCTLLang >>= aLocale;
        eCurLangCTL = SvxLocaleToLanguage( aLocale );
    }
    catch(Exception&)
    {
    }
    //overwrite them by the values provided by the DocShell
    if(pCurrentDocShell)
    {
        aCurrentDocCB.Enable(TRUE);
        aCurrentDocCB.Check(bLanguageCurrentDoc_Impl);
        const SfxPoolItem* pLang;
        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_LANGUAGE, FALSE, &pLang))
            eCurLang = ((const SvxLanguageItem*)pLang)->GetValue();

        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, FALSE, &pLang))
            eCurLangCJK = ((const SvxLanguageItem*)pLang)->GetValue();

        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, FALSE, &pLang))
            eCurLangCTL = ((const SvxLanguageItem*)pLang)->GetValue();
    }
    if(LANGUAGE_NONE == eCurLang || LANGUAGE_DONTKNOW == eCurLang)
        aWesternLanguageLB.SelectLanguage(LANGUAGE_NONE);
    else
        aWesternLanguageLB.SelectLanguage(eCurLang);

    if(LANGUAGE_NONE == eCurLangCJK || LANGUAGE_DONTKNOW == eCurLangCJK)
        aAsianLanguageLB.SelectLanguage(LANGUAGE_NONE);
    else
        aAsianLanguageLB.SelectLanguage(eCurLangCJK);

    if(LANGUAGE_NONE == eCurLangCTL || LANGUAGE_DONTKNOW == eCurLangCTL)
        aComplexLanguageLB.SelectLanguage(LANGUAGE_NONE);
    else
        aComplexLanguageLB.SelectLanguage(eCurLangCTL);

    aWesternLanguageLB.SaveValue();
    aAsianLanguageLB.SaveValue();
    aComplexLanguageLB.SaveValue();
    aCurrentDocCB.SaveValue();

    sal_Bool bEnable = !pLangConfig->aLinguConfig.IsReadOnly( C2U("DefaultLocale") );
    aWesternLanguageFT.Enable( bEnable );
    aWesternLanguageLB.Enable( bEnable );
    aWesternLanguageFI.Show(!bEnable);

    // #i15812# controls for CJK/CTL already enabled/disabled from LocaleSettingHdl
#if 0
    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( C2U("DefaultLocale_CJK") ) && aAsianSupportCB.IsChecked() );
    aAsianLanguageFT.Enable( bEnable );
    aAsianLanguageLB.Enable( bEnable );

    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( C2U("DefaultLocale_CTL") ) && aCTLSupportCB.IsChecked() );
    aComplexLanguageFT.Enable( bEnable );
    aComplexLanguageLB.Enable( bEnable );
#endif
}
/* -----------------------------20.04.01 15:09--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(  OfaLanguagesTabPage, SupportHdl, CheckBox*, pBox )
{
    DBG_ASSERT( pBox, "OfaLanguagesTabPage::SupportHdl(): pBox invalid" );

    sal_Bool bCheck = pBox->IsChecked();
    if ( &aAsianSupportCB == pBox )
    {
        sal_Bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly( C2U("DefaultLocale_CJK"));
        bCheck = ( bCheck && !bReadonly );
        aAsianLanguageFT.Enable( bCheck );
        aAsianLanguageLB.Enable( bCheck );
        aAsianLanguageFI.Show(bReadonly);
        if( pBox->IsEnabled() )
            m_bOldAsian = bCheck;
    }
    else if ( &aCTLSupportCB == pBox )
    {
        sal_Bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly( C2U("DefaultLocale_CTL"));
        bCheck = ( bCheck && !bReadonly  );
        aComplexLanguageFT.Enable( bCheck );
        aComplexLanguageLB.Enable( bCheck );
        aComplexLanguageFI.Show(bReadonly);
        if( pBox->IsEnabled() )
            m_bOldCtl = bCheck;
    }
    else
    {
        DBG_ERRORFILE( "OfaLanguagesTabPage::SupportHdl(): wrong pBox" );
    }

    return 0;
}

namespace
{
    void lcl_checkLanguageCheckBox(CheckBox& _rCB,FixedLine& _rFL,sal_Bool _bNewValue,sal_Bool _bOldValue)
    {
        if ( _bNewValue )
            _rCB.Check(TRUE);
        else
            _rCB.Check( _bOldValue );
// #i15082# do not call SaveValue() in running dialog...
//      _rCB.SaveValue();
        _rCB.Enable( !_bNewValue );
        _rFL.Enable( !_bNewValue );
    }
}
/* -----------------08.06.01 17:56-------------------

 --------------------------------------------------*/
IMPL_LINK( OfaLanguagesTabPage, LocaleSettingHdl, SvxLanguageBox*, pBox )
{
    LanguageType eLang = pBox->GetSelectLanguage();
    sal_uInt16 nType = SvtLanguageOptions::GetScriptTypeOfLanguage(eLang);
    // first check if CTL must be enabled
    // #103299# - if CTL font setting is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT))
    {
        sal_Bool bIsCTLFixed = nType & SCRIPTTYPE_COMPLEX;
        lcl_checkLanguageCheckBox(aCTLSupportCB,aCTLSupportFL,bIsCTLFixed,m_bOldCtl);
        SupportHdl( &aCTLSupportCB );
    }
    // second check if CJK must be enabled
    // #103299# - if CJK support is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK))
    {
        sal_Bool bIsCJKFixed = nType & SCRIPTTYPE_ASIAN;
        lcl_checkLanguageCheckBox(aAsianSupportCB,aAsianSupportFL,bIsCJKFixed,m_bOldAsian);
        SupportHdl( &aAsianSupportCB );
    }

    USHORT nPos;
    if ( eLang == LANGUAGE_SYSTEM )
        nPos = aCurrencyLB.GetEntryPos( (void*) NULL );
    else
    {
        const NfCurrencyEntry* pCurr = &SvNumberFormatter::GetCurrencyEntry( eLang );
        nPos = aCurrencyLB.GetEntryPos( (void*) pCurr );
    }
    aCurrencyLB.SelectEntryPos( nPos );

    //update the decimal separator key of the related CheckBox
    Locale aTempLocale;
    lcl_LanguageToLocale( aTempLocale, eLang );
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), aTempLocale );
    String sTempLabel(sDecimalSeparatorLabel);
    sTempLabel.SearchAndReplaceAscii("%1", aLocaleWrapper.getNumDecimalSep() );
    aDecimalSeparatorCB.SetText(sTempLabel);

    return 0;
}

