/*************************************************************************
 *
 *  $RCSfile: sfxhelp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:41 $
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

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#include <automation/automation.hxx>
#include <svtools/pathoptions.hxx>
#pragma hdrstop

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifdef WNT
#pragma optimize ( "", off )
#endif

#include <tools/urlobj.hxx>
#include <svtools/helpopt.hxx>

#include <helpid.hrc>
#include <app.hxx>
#include <sfx.hrc>
#include <sfxhelp.hxx>
#include <sfxhelp2.hxx>
#include <docfac.hxx>
#include <openflag.hxx>
#include <bindings.hxx>
#include <dispatch.hxx>
#include <genlink.hxx>
#include <viewsh.hxx>
#include <sfxresid.hxx>
#include <printer.hxx>
#include <request.hxx>
#include <docfile.hxx>
#include <app.hrc>
#include <msgpool.hxx>
#include <appdata.hxx>
#include "objshimp.hxx"
#include "macrconf.hxx"
#include "viewfrm.hxx"
#include "objface.hxx"

#ifdef MAC
#define HELP_APP_NAME "StarHelp 4.0"
#elif defined UNX
#define HELP_APP_NAME "shelp"
#else
#define HELP_APP_NAME "SHELP.EXE"
#endif

static const char __FAR_DATA zsHelpAgentConfig[] = "hlpagent.ini";
static const char __FAR_DATA zsHelpAgentConfig_Tips[] = "Tips";

static const USHORT nBorder = 10;       // So war's im Writer - aber warum ???

SV_IMPL_VARARR_SORT( SortedULONGs, ULONG );
//SV_IMPL_PTRARR( HelpTextCaches, HelpTextCache* );

USHORT ImplSetLanguageGroup( Config& rConfig, const String& rGroupName, BOOL bSearchLanguage )
{
    USHORT nFound = 0;

    // Nach Prioritaeten die richtige Language-Section suchen...
    LanguageType eLanguage = Application::GetSettings().GetInternational().GetLanguage();
    for ( USHORT nPrio = 0; nPrio <= 4; nPrio++ )
    {
        const char* pLang = ResMgr::GetLang( eLanguage, nPrio );
        if ( pLang )
        {
            String aGroup( rGroupName );
            aGroup += '-';
            aGroup += String::CreateFromAscii(pLang);
            if ( !bSearchLanguage || rConfig.HasGroup( U2S(aGroup) ) )
            {
                rConfig.SetGroup( U2S(aGroup) );
                nFound = nPrio+1;
                break;
            }
            // Falls keine gefunden wird, dann die genaueste einstellen
            if ( nPrio == 0 )
                rConfig.SetGroup( U2S(aGroup) );
        }
    }
    return nFound;
}

//=========================================================================
SFX_IMPL_DOCKINGWINDOW(SfxHelpPIWrapper, SID_HELP_PI);
SFX_IMPL_DOCKINGWINDOW(SfxHelpTipsWrapper, SID_TIPWINDOW);

static Point CalcFloaterPos( Window *pPI, Rectangle& rFloaterRect, const Rectangle& rDialogRect )
{
    Point aPos = rFloaterRect.TopLeft();
    if ( rFloaterRect.IsOver( rDialogRect ) )
    {
        Size aScreenSize = pPI->GetDesktopRectPixel().GetSize();
        Size aFloaterSz = rFloaterRect.GetSize();
        long nMinWidth = aScreenSize.Width() / 5;
//      long nMinHeight = aScreenSize.Height() / 5;

        // Minimalgroessen nur, wenn nicht vorher schon kleiner, weil der
        // Anwender es wollte:
        if ( aFloaterSz.Width() < nMinWidth )
            nMinWidth =  aFloaterSz.Width();
//      if ( aFloaterSz.Height() < nMinHeight )
//          nMinHeight=  aFloaterSz.Height();


        long nTopDiff = -(rFloaterRect.Top() - rDialogRect.Top());
        long nBottomDiff = rFloaterRect.Bottom() - rDialogRect.Bottom();
        long nLeftDiff = -(rFloaterRect.Left() - rDialogRect.Left());
        long nRightDiff = rFloaterRect.Right() - rDialogRect.Right();

        BOOL bRight = ( rDialogRect.Right() + rFloaterRect.GetWidth() ) < aScreenSize.Width();
        BOOL bLeft = ( rDialogRect.Left() - rFloaterRect.GetWidth() ) > 0;
        BOOL bTop = ( rDialogRect.Top() - rFloaterRect.GetHeight() ) > 0;
        BOOL bBottom = ( rDialogRect.Bottom() + rFloaterRect.GetHeight() ) < aScreenSize.Height();

        if ( bLeft || bRight )
            aPos.X() = ( bRight && ( !bLeft || ( nLeftDiff < nRightDiff ) ) )
                ? rDialogRect.Right()
                : rDialogRect.Left() - rFloaterRect.GetWidth();
        else if ( bTop || bBottom )
            aPos.Y() = ( bTop && ( !bBottom || ( nBottomDiff < nTopDiff ) ) )
                ? rDialogRect.Top() - rFloaterRect.GetHeight()
                : rDialogRect.Bottom();
        else
                {
            // Rechts oder links daneben, wo mehr Platz ist...
            long nLeftSpace = rDialogRect.Left();
            long nRightSpace = aScreenSize.Width() - rDialogRect.Right();
            if ( nLeftSpace > nRightSpace )
            {
                aPos.X() = 0;
                long nWidth = nLeftSpace;
                if ( nWidth < nMinWidth )
                    nWidth = nMinWidth;
                rFloaterRect.Right() = rFloaterRect.Left() + nWidth;

            }
            else
            {
                long nWidth = nRightSpace;
                if ( nWidth < nMinWidth )
                    nWidth = nMinWidth;
                aPos.X() = aScreenSize.Width() - nWidth;
                rFloaterRect.Right() = rFloaterRect.Left() + nWidth;
            }
        }
    }
    rFloaterRect.SetPos( aPos );
    return aPos;
}


//=========================================================================

SfxHelpPIWrapper::SfxHelpPIWrapper(Window *pParent,
    USHORT nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo) :
    SfxChildWindow(pParent, nId)
{
    // Skalierung des HelpPI initialisieren
    USHORT nScale = 100;
    if ( pInfo->aExtraString.Len() )
    {
        USHORT nPos = pInfo->aExtraString.Search(0x0023); // '#' = 23h
        if ( nPos != STRING_NOTFOUND )
        {
            String aScale = pInfo->aExtraString.Copy(nPos+1);
            pInfo->aExtraString.Erase(nPos+1);
            pInfo->aExtraString.Erase(nPos,1);
            nScale = (USHORT) aScale.ToInt32();
        }
        else
        {
            // Es ist noch ein alter Eintrag (FloatingWindow); das Alignment
            // wird dann in Initialize nicht ver"andert.
            pInfo->aExtraString.Erase();
        }
    }

    // Versuche, das Helferlein zu laden
    SfxHelpPI *pSfxHelpPI = new SfxHelpPI(pBindings, this, pParent, nScale);

    pWindow = pSfxHelpPI;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    if ( !pInfo->aSize.Width() )
    {
        // initial zentriert hochkommen
        Size aSz( 400, 250 );
        pSfxHelpPI->SetSizePixel( aSz );

        if ( SFX_APP()->GetDemoKind() == SFX_DEMOKIND_TRYBUY )
        {
            Rectangle aRect = Rectangle( Point(), pParent->GetOutputSizePixel() );
            Point aPoint = aRect.BottomRight();
            aPoint.X() -= ( aSz.Width() + 10 );
            aPoint.Y() -= ( aSz.Height() + 10 );
            pSfxHelpPI->SetFloatingPos( aPoint );
        }
    }

    pSfxHelpPI->Initialize(pInfo);
    pSfxHelpPI->ResetTopic();
}

BOOL SfxHelpPIWrapper::QueryClose()
{
    BOOL bCanClose = TRUE;
    SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
    {
//        HelpPI* pH = pHelpPI->GetHelpPI();
//        if ( pH && pH->HasDialog() )
//            bCanClose = FALSE;
    }
    return bCanClose;
}

void SfxHelpPI::ResetTopic()
{
    // initial antriggern
    ULONG nHelpId = 0L;
    SfxDispatcher* pDispatcher = SFX_APP()->GetDispatcher_Impl();
    SfxShell *pShell;
    for ( USHORT nShellLevel = 0;
          !nHelpId && 0 != ( pShell = pDispatcher->GetShell(nShellLevel) );
          ++nShellLevel )
        nHelpId = pShell->GetHelpId();
    LoadTopic( nHelpId );
}

//-------------------------------------------------------------------------

void SfxHelpPI::FillInfo(SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.aExtraString += '#';
//    rInfo.aExtraString += String( pHelpPI->GetCurScale() );
}

//-------------------------------------------------------------------------

SfxHelpPI::SfxHelpPI(SfxBindings* pBindinx, SfxChildWindow* pCW, Window* pParent, USHORT nScale )
    : SfxDockingWindow( pBindinx, pCW, pParent,
        WB_CLOSEABLE | WB_DOCKABLE | WB_CLIPCHILDREN | WB_MOVEABLE | WB_SIZEABLE | WB_DIALOGCONTROL | WB_3DLOOK ),
    pInnerWindow( NULL ),
    aTipBox( this, WB_3DLOOK )
{
    AlwaysEnableInput( TRUE, TRUE );
    Enable( TRUE, TRUE );
    bInShowMe = FALSE;
    nTip = 0;

    aTopicJustRequestedTimer.SetTimeout( 200 );

    pInnerWindow = new Window( this, WB_DIALOGCONTROL );

//    pHelpPI = new HelpPI( pInnerWindow, TRUE );
//    pHelpPI->SetSpecialLinkHdl( LINK( this, SfxHelpPI, SpecialLinkHdl ) );
//    pHelpPI->SetToolboxHdl( LINK( this, SfxHelpPI, PIToolboxHdl ) );

    SetText( DEFINE_CONST_UNICODE("Help Agent") );

    SetSizePixel( LogicToPixel( Size( 150, 70 ), MapMode( MAP_SYSFONT ) ) );
    pInnerWindow->SetSizePixel( LogicToPixel ( Size( 147, 67 ), MapMode( MAP_SYSFONT ) ) );
    pInnerWindow->SetPosPixel( LogicToPixel ( Point( 3, 3 ), MapMode( MAP_SYSFONT ) ) );

    pInnerWindow->Show();

    SetUniqueId( SID_HELP_PI );
    SetMinOutputSizePixel( Size( 200, 100 ) );
//    if ( nScale )
//        pHelpPI->ChangeScale( nScale );
//    else
    {
        const long nDPI = LogicToPixel(Point(1, 1), MapMode(MAP_INCH)).X();
//        if(nDPI < 90 && GetDesktopRectPixel().GetWidth() > 1000)
//            pHelpPI->ChangeScale( 150 );
    }

//    pHelpPI->GetTopicChangedHdl() = LINK( this, SfxHelpPI, TopicChangedHdl_Impl );
    LoadTopic( HELP_INDEX );
}

BOOL SfxHelpPI::Close()
{
//    return pHelpPI->HasDialog() ? FALSE : SfxDockingWindow::Close();
    return TRUE;
}

void SfxHelpPI::SetTip( ULONG nTp )
{
    nTip = nTp;
    if ( nTip )
    {
        Resize();       // Falls keine Aenderung von aussen kommt...
        aTipBox.SetHelpId( HID_HELPAGENT_TIP_BOX );
        aTipBox.SetClickHdl( LINK( this, SfxHelpPI, TipBoxHdl ) );
        aTipBox.Show();
    }
    else
    {
        aTipBox.Hide();
        Resize();
    }
}

void SfxHelpPI::SetTipText( const String& rTip )
{
    String aText( SfxResId( RID_STR_NOAUTOSTARTHELPAGENT ) );
    aText.SearchAndReplace( DEFINE_CONST_UNICODE("XX"), rTip );
    aTipBox.SetText( aText );
}

IMPL_LINK( SfxHelpPI, PIToolboxHdl, ToolBox* , pToolBox )
{
    // Wird z.Zt nur fuer 'Volle Hilfe' gerufen
    if ( pToolBox )
    {
        // Link kommt von ToolBox, im Hdl darf diese aber nicht durch das
        // Abschalten des HelpPI zerstoert werden
        // => Diesen Link nochmal als UserEvent rufen...
        Application::PostUserEvent( LINK( this, SfxHelpPI, PIToolboxHdl ) );
    }
    else
    {
        Help* pHelp = Application::GetHelp();
    }

    return 1;
}
#if 0
IMPL_LINK( SfxHelpPI, SpecialLinkHdl, SpecialLinkInfo* , pInfo )
{
    bInShowMe = TRUE;
    if ( pInfo->aURL.Len() )
    {
        SfxStringItem aDoc( SID_FILE_NAME, pInfo->aURL );
        SfxStringItem aRef( SID_REFERER, DEFINE_CONST_UNICODE("private:help") );
        SFX_APP()->GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, &aDoc, &aRef, 0L );
    }
    bInShowMe = FALSE;
    delete pInfo;   // Dieser Link wird asynchron gerufen, deshalb wurden die Daten mit new angelegt...
    return 0L;
}
#endif

IMPL_LINK( SfxHelpPI, TopicChangedHdl_Impl, void* , pVoid )
{
    if ( nTip && !aTipBox.GetText().Len() )
    {
//        String aTitle( pHelpPI->GetTitle( nTip ) );
        // Tip-Text erst hier, da der HelpAgent asynchron starten soll...
//        SetTipText( aTitle );
    }

    return 0L;
}

IMPL_LINK( SfxHelpPI, TipBoxHdl, CheckBox* , pBox )
{
    Help* pHelp = Application::GetHelp();
    if ( pHelp )
        ((SfxHelp_Impl*)pHelp)->EnableTip( nTip, !pBox->IsChecked() );

    return 0L;
}

//-------------------------------------------------------------------------

SfxHelpPI::~SfxHelpPI()
{
//    delete pHelpPI;
    delete pInnerWindow;

    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    if ( pHelp )
        pHelp->HelpAgentClosed();
}

//-------------------------------------------------------------------------

void SfxHelpPI::Paint( const Rectangle& rRect)
{
}

//-------------------------------------------------------------------------

void SfxHelpPI::Resize()
{
    Size aSize( GetOutputSizePixel() );
    Point aPos( pInnerWindow->GetPosPixel() );
    aSize.Width() -= 2 * aPos.X();
    aSize.Height() -= 2 * aPos.Y();

    if ( nTip )
    {
        Size aTextSize( aTipBox.GetTextWidth( String() ), aTipBox.GetTextHeight());
        if ( aSize.Height() > ( aTextSize.Height() + aPos.X() ) )
        {
            aSize.Height() -= aTextSize.Height() + aPos.X();
            aTipBox.SetPosSizePixel( Point( aPos.X(), aPos.Y()+aPos.X()+aSize.Height() ),
                                     Size( aSize.Width(), aTextSize.Height() ) );
            aTipBox.Show();
        }
        else
            aTipBox.Hide();
    }

    pInnerWindow->SetSizePixel( aSize );
//    pHelpPI->Resize();
    SfxDockingWindow::Resize();
    Invalidate();
}

//-------------------------------------------------------------------------

String SfxHelpPI::GetExtraInfo() const
{
    return String();
//    return pHelpPI->GetCurScale();
}

void SfxHelpPI::LoadTopic( const String& rFileName, ULONG nId)
{
//    pHelpPI->LoadTopic( rFileName, nId );
}

void SfxHelpPI::LoadTopic( ULONG nId )
{
    // at the moment no implementation
}

void SfxHelpPI::LoadTopic( const String& rKeyword )
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if ( pShell )
    {
        INetURLObject aAbsName( SfxHelp_Impl::GetHelpPath() );
        aAbsName.insertName( pShell->GetFactory().GetHelpFile() );
        aAbsName.setExtension( DEFINE_CONST_UNICODE("svh") );
        String aFileName = aAbsName.GetMainURL();
//        pHelpPI->LoadTopic( aFileName, rKeyword );
    }
}

SfxHelp_Impl::SfxHelp_Impl()
{
    nLastDialog = 0;
    pHelpInfo = 0;
//    pHelpCache = 0;
    pPIStarterList = 0;
    pHelpFileInfos = 0;

    aDialogDetector.SetTimeout( 10 );
    aDialogDetector.SetTimeoutHdl( LINK( this, SfxHelp_Impl, DialogDetectHdl ) );
}

SfxHelp_Impl::~SfxHelp_Impl()
{
    aDialogDetector.Stop();

//    delete pHelpInfo;
    delete pPIStarterList;

    if ( pHelpFileInfos )
    {
        for ( USHORT n = pHelpFileInfos->Count(); n; )
        {
            HelpFileInfo* pInf = (HelpFileInfo*)pHelpFileInfos->GetObject( --n );
            delete pInf;
        }
        delete pHelpFileInfos;
    }

    // HelpCache wird durch PTRARR_DEL geloescht.
}

USHORT SfxHelp_Impl::GetHelpFileInfoCount()
{
    // at the moment no implementation
    if ( !pHelpFileInfos )
        pHelpFileInfos = new List;
    return pHelpFileInfos->Count();
}

HelpFileInfo* SfxHelp_Impl::GetHelpFileInfo( USHORT n )
{
    // GetHelpFileInfoCount() rufen, weil die Liste ggf. erst erzeugt wird
    if ( n < GetHelpFileInfoCount() )
        return (HelpFileInfo*) pHelpFileInfos->GetObject( n );
    return NULL;

}

void SfxHelp_Impl::HelpAgentClosed()
{
    nLastDialog = 0;
}

String SfxHelp_Impl::GetHelpFileName( ULONG nHelpId )
{
    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    if ( pHelp && pHelp->aCustomHelpFile.Len() )
        return pHelp->aCustomHelpFile;

    String aHelpFileName;
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrame *pViewFrame = pSfxApp->GetViewFrame();
    if ( pViewFrame )
    {
        // Wenn es ein Slot ist, kann es sein, da\s internes InPlace vorliegt
        // und eine Container-SlotId gefragt ist
        if (nHelpId >= (ULONG) SID_SFX_START && nHelpId <= (ULONG) SHRT_MAX)
        {
            if ( pViewFrame->GetParentViewFrame_Impl() )
            {
                // Ist es ein ContainerSlot ?
                const SfxSlot* pSlot =
                    pSfxApp->GetSlotPool(pViewFrame).GetSlot( (USHORT) nHelpId );
                if ( !pSlot || pSlot->IsMode( SFX_SLOT_CONTAINER ) )
                    pViewFrame = pViewFrame->GetParentViewFrame_Impl();
            }
        }

        if( pViewFrame->GetObjectShell() )
            aHelpFileName = pViewFrame->GetHelpFile_Impl();
    }

    if ( !aHelpFileName.Len() )
        aHelpFileName = DEFINE_CONST_UNICODE(SFX_DESKTOP_HELPFILE_NAME);

    return aHelpFileName;
}

void SfxHelp_Impl::SetCurrentHelpFile( ULONG nHelpId )
{
    String aString = GetHelpFileName( nHelpId );
    if ( GetCurrentHelpFile() != aString )
        SetHelpFile( aString );
}

Window* SfxHelp_Impl::SearchFocusWindowParent()
{
    RESOURCE_TYPE nType;
    Window* pFocusParent = Application::GetFocusWindow();
    while ( pFocusParent &&
            ( ( nType = pFocusParent->GetType() ) != WINDOW_MODELESSDIALOG ) &&
            ( nType != WINDOW_MODALDIALOG ) &&
            ( nType != WINDOW_TABDIALOG ) &&
            ( nType != WINDOW_PATHDIALOG ) &&
            ( nType != WINDOW_FILEDIALOG ) &&
            ( nType != WINDOW_PRINTERSETUPDIALOG ) &&
//          ( nType != WINDOW_FLOATINGWINDOW ) &&
//          ( nType != WINDOW_DOCKINGWINDOW ) &&
            ( nType != WINDOW_PRINTDIALOG ) )
    {
        pFocusParent = pFocusParent->GetParent();
    }
    return pFocusParent;
}

void SfxHelp_Impl::CheckPIPosition()
{
    Window* pDialog = SearchFocusWindowParent();
    if( ( nLastDialog != (ULONG)pDialog ) &&
        ( !pDialog || ( ! pDialog->GetText().EqualsAscii("HelpWindow") ) ) )
    {
        BOOL bAccepted = TRUE;
        SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();

        if ( pDialog && pHelpPI )
        {
//            if ( pHelpPI->IsWindowOrChild( pDialog, TRUE ) && !pHelpPI->GetHelpPI()->HasDialogFocus() )
//                return;

            if ( !pDialog->IsVisible() )
            {
                aDialogDetector.Start();
                bAccepted = FALSE;
            }
            else if ( pDialog == pHelpPI->GetFloatingWindow() )
            {
                bAccepted = FALSE;
            }
            else if ( pHelpPI && pHelpPI->IsFloatingMode() )
            {
                // Dialog schon fertig?
                Size aDlgSize = pDialog->GetSizePixel();
                Point aDlgPos = pDialog->GetPosPixel();
                // Sind Position und Groesse schon eingestellt?
                if ( aDlgSize.Width() && ( aDlgPos.X() || aDlgPos.Y() ) )
                {
                    Rectangle aDlgRect( aDlgPos, aDlgSize );
                    Rectangle aPIRect( pHelpPI->GetPosPixel(), pHelpPI->GetSizePixel() );
                    /* Point aPos = */ CalcFloaterPos( pHelpPI, aPIRect, aDlgRect );
                    pHelpPI->SetPosSizePixel( aPIRect.TopLeft(), aPIRect.GetSize() );
                    pHelpPI->GetFloatingWindow()->ToTop();
                }
                else
                {
                    aDialogDetector.Start();
                    bAccepted = FALSE;
                }
            }
        }

        if ( bAccepted )
            nLastDialog = (ULONG)pDialog;
    }
}

IMPL_LINK( SfxHelp_Impl, DialogDetectHdl, Timer* , EMPTYARG )
{
    SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI && pHelpPI->IsFloatingMode() )
        CheckPIPosition();
    return 1;
}

BOOL SfxHelp_Impl::Start( ULONG nHelpId )
{
    return ImplStart( nHelpId, TRUE, TRUE, TRUE );
}

BOOL SfxHelp_Impl::ImplStart( ULONG nHelpId, BOOL bCheckHelpFile, BOOL bChangeHelpFile, BOOL bHelpAgent )
{
/*! (pb) what about help?
    static BOOL bInHelpRequest = FALSE;
    if ( bInHelpRequest || !nHelpId || ( nHelpId == SID_EXTENDEDHELP ) )
    {
        if ( bInHelpRequest )
            Sound::Beep();
        return FALSE;
    }


    if ( Help::IsRightHelp() )
    {
        if( ImplGetHelpMode() & HELPTEXTMODE_NOHELPAGENT )
            return FALSE;

        if ( ( nHelpId == HELP_INDEX ) || !CheckHelpFile( TRUE ) )
            return FALSE;

        bInHelpRequest = TRUE;
        SetCurrentHelpFile( nHelpId );
        StartHelpPI( nHelpId, FALSE, FALSE );
        bInHelpRequest = FALSE;
        return TRUE;
    }

    bInHelpRequest = TRUE;

    if( ImplGetHelpMode() & HELPTEXTMODE_NOCONTEXTHELP )
    {
        if( nHelpId < 20000 || nHelpId > 20006 )
            nHelpId = HELP_INDEX;
    }

    if ( bChangeHelpFile )
        SetCurrentHelpFile( nHelpId );

    BOOL bDone = FALSE;
    if ( !bCheckHelpFile || CheckHelpFile( TRUE ) )
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        if ( !bHelpAgent || ( nHelpId == HELP_INDEX ) )
//              !Application::IsInModalMode() &&
//              Application::GetAppWindow()->IsEnabled() &&
//              !SearchFocusWindowParent() &&   // kein Dialog aktiv
//              ( !pFrame || !pFrame->GetObjectShell()->IsInModalMode() ) )
        {
            SfxHelpViewShell* pViewShell = GetHelpViewShell( TRUE );
            if ( pViewShell )
                bDone = pViewShell->ShowHelp( GetHelpFile(), nHelpId );
        }
        else
        {
            StartHelpPI( nHelpId, TRUE, FALSE );
            SfxHelpPI* pHelpPI= SFX_APP()->GetHelpPI();
            if ( pHelpPI )
            {
                if ( !pHelpPI->IsFloatingMode() )
                {
                    pHelpPI->SetFloatingMode( TRUE );
                    bForcedFloatingPI = TRUE;
                }
                if ( pHelpPI->GetFloatingWindow() )
                    pHelpPI->GetFloatingWindow()->ToTop();
                CheckPIPosition();
                bDone = TRUE;
            }
        }
    }
    bInHelpRequest = FALSE;
    return bDone;
*/
    return FALSE;
}


BOOL SfxHelp_Impl::Start( const XubString& rKeyword )
{
    BOOL bDone = FALSE;
    SetCurrentHelpFile( HELP_INDEX );
    if ( CheckHelpFile( TRUE ) )
    {
    }
    return bDone;
}

void SfxHelp_Impl::SlotExecutedOrFocusChanged( ULONG nId, BOOL bSlot, BOOL bAutoStart )
{
    BOOL bAutoStarted = FALSE;
    SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
    {
//      if ( !bSlot && pHelpPI->IsWindowOrChild( Application::GetFocusWindow(), TRUE ) && !pHelpPI->GetHelpPI()->HasDialogFocus() )
        if ( !bSlot && pHelpPI->IsWindowOrChild( Application::GetFocusWindow(), TRUE ) )
            return; // Wenn Navigation im HelpPI, dann nicht triggern...
        // Nicht auf Focus reagieren, wenn gerade mit F1 angetriggert
        if ( pHelpPI->IsTopicJustRequested() )
            return;

        if ( !bSlot || !pHelpPI->IsInShowMe() )
            pHelpPI->LoadTopic( nId );
    }
    else if ( bAutoStart && GetPIStarterList()->Seek_Entry( nId, 0 ) )
    {
         SetCurrentHelpFile( nId );
        StartHelpPI( nId, bSlot, TRUE );
        bAutoStarted = TRUE;
    }

    if ( !bSlot && ( pHelpPI || bAutoStarted ) )
        CheckPIPosition();
}


XubString SfxHelp_Impl::GetHelpText( ULONG nHelpId )
{
#if defined DBG_UTIL && ( defined WNT || defined OS2 || defined UNX )
    static int bTraceHelpIds = 2;
    if ( bTraceHelpIds == 2 )
    {
        char *pHELP = getenv( "HELP" );
        if( pHELP && !strcmp( pHELP, "MP" ) )
            bTraceHelpIds = 1;
        else
            bTraceHelpIds = 0;
    }
    if ( bTraceHelpIds )
    {
        ByteString aTraceStr( "HELPID: " );
        aTraceStr += ByteString::CreateFromInt32( nHelpId );
        DBG_TRACE( aTraceStr.GetBuffer() );
    }
#endif

    // at the moment no implementation
    return XubString();
}

void SfxHelp_Impl::GetHelpURLs( const String& rRootURL, SvStringsDtor& rLst )
{
    AssertValidHelpDocInfo();
  //  pHelpInfo->GetHelpURLs( rRootURL, rLst );
}

void SfxHelp_Impl::GetBookmarks( SvStringsDtor& rLst )
{
    AssertValidHelpDocInfo();
//    pHelpInfo->GetBookmarks( rLst );
}

void SfxHelp_Impl::AddBookmark( const String& rName, const String& rURL )
{
    AssertValidHelpDocInfo();
//    pHelpInfo->AddBookmark( rName, rURL );
}

void SfxHelp_Impl::RemoveBookmark( const String& rName )
{
    AssertValidHelpDocInfo();
//    pHelpInfo->RemoveBookmark( rName );
}

void SfxHelp_Impl::RenameBookmark( const String& rOldTitle, const String& rNewTitle )
{
    AssertValidHelpDocInfo();
//    pHelpInfo->RenameBookmark( rOldTitle, rNewTitle );
}


BOOL SfxHelp_Impl::CheckHelpFile( BOOL bPrompt ) const
{
    return SfxHelp_Impl::CheckHelpFile( GetHelpFile(), bPrompt );
}

BOOL SfxHelp_Impl::CheckHelpFile( const String& rFileName, BOOL bPrompt )
{
    // at the moment no implementation
    BOOL bOK = FALSE;
    if ( !bOK && bPrompt )
    {
        Window* pParent = Application::GetFocusWindow();
        String aErrorStr( SfxResId( RID_STR_HLPFILENOTEXIST ) );
        InfoBox( pParent, aErrorStr ).Execute();
    }

    return bOK;
}

void SfxHelp_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint *pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if ( pHint && ( pHint->GetId() & SFX_HINT_DYING ) )
    {
        // z.Zt. lausche ich nur am HelpDoc...
    }
}

//-------------------------------------------------------------------------

void SfxHelp_Impl::SetHelpFile( const String &rFileName, BOOL bAdjustExt )
{
    INetURLObject aAbsName( GetHelpPath() );
    aAbsName.insertName( rFileName );

    if ( bAdjustExt )
        aAbsName.setExtension( DEFINE_CONST_UNICODE("svh") );

    String aFileName = aAbsName.PathToFileName();
//      InfoBox( 0, aFileName ).Execute();
    Help::SetHelpFile( aFileName );
    aCurHelpFile = rFileName;
}

void SfxHelp_Impl::AssertValidHelpDocInfo()
{
//    if ( !pHelpInfo || ( pHelpInfo->GetRequestedFileName() != GetHelpFile() ) )
    {
//        delete pHelpInfo;
//        pHelpInfo = new SHelpInfo( GetHelpFile(), FALSE );      // vielleicht doch cachable?
    }
}

String SfxHelp_Impl::GetHelpPath()
{
#if SUPD<613//MUSTINI
    SfxIniManager* pIni = SFX_INIMANAGER()->Find( SFX_KEY_HELP_DIR );
    if ( !pIni )
        pIni = SFX_INIMANAGER();
    String aHelpDir = pIni->Get( SFX_KEY_HELP_DIR );
#else
    String aHelpDir = SvtPathOptions().GetHelpPath();
#endif

    if ( aHelpDir.Len() )
    {
        String aPath;
        INetURLObject aHelpRoot( aHelpDir, INET_PROT_FILE );

        // Nach Prioritaeten das richtige Language-Dir suchen...
        LanguageType eLanguage = Application::GetSettings().GetInternational().GetLanguage();
        for ( USHORT nPrio = 0; nPrio <= 4; nPrio++ )
        {
            const char* pLang = ResMgr::GetLang( eLanguage, nPrio );
            if ( pLang )
            {
                INetURLObject aTemp = aHelpRoot;
                aTemp.insertName( String::CreateFromAscii( pLang ) );
                if ( 0 /*! Exists??? */ )
                {
                    return aTemp.GetMainURL();
                }
            }
        }
        return aPath;
    }
    else
    {
        INetURLObject aObj( Application::GetAppFileName(), INET_PROT_FILE );
        return aObj.GetMainURL();
    }
}

void SfxHelp_Impl::CreatePIStarterList()
{
    // Config:
    // [Tips]
    // Slot=0/1

    DBG_ASSERT( !pPIStarterList, "PIStarterList existiert schon!" );
    pPIStarterList = new SortedULONGs;
    Config aConfig( GetHelpAgentConfig() );
    ImplSetLanguageGroup( aConfig, String::CreateFromAscii(zsHelpAgentConfig_Tips), TRUE );
    USHORT nTips = aConfig.GetKeyCount();
    for ( USHORT nTip = 0; nTip < nTips; nTip++ )
    {
        if ( (BOOL)(USHORT)aConfig.ReadKey( nTip ).ToInt32() )
        {
            ULONG nId = aConfig.GetKeyName( nTip ).ToInt32();
            if ( nId )
                pPIStarterList->Insert( nId );
        }
    }
}

void SfxHelp_Impl::StartHelpPI( ULONG nHelpId, BOOL bSlot, BOOL bTip )
{
/*! (pb) what about help?
    if( ImplGetHelpMode() & HELPTEXTMODE_NOHELPAGENT )
        return;

    BOOL bValidHelpFile = CheckHelpFile( FALSE );
    SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
    if ( !pHelpPI && bValidHelpFile )
    {
        SFX_APP()->SetChildWindow( SID_HELP_PI, TRUE );
        pHelpPI = SFX_APP()->GetHelpPI();
        CheckPIPosition();
    }
    else if ( pHelpPI && !bValidHelpFile )
    {
        SFX_APP()->SetChildWindow( SID_HELP_PI, FALSE );
        pHelpPI = NULL;
    }

    if ( pHelpPI )
    {
        if ( bTip )
        {
            pHelpPI->SetTip( nHelpId );
            if ( !bSlot )
            {
                // Start ueber Focus => Text des Controls...
                Window* pWin = Application::GetFocusWindow();
                if ( pWin && pWin->GetText().Len() )
                {
                    String aText( pWin->GetText() );
                    aText.EraseAllChars( '~' );
                    pHelpPI->SetTipText( aText );
                }
            }
        }
        pHelpPI->LoadTopic( nHelpId );
        pHelpPI->SetTopicJustRequested( TRUE );
    }
*/
}

void SfxHelp_Impl::EnableTip( ULONG nTip, BOOL bEnable )
{
    Config aConfig( GetHelpAgentConfig() );
    ImplSetLanguageGroup( aConfig, String::CreateFromAscii(zsHelpAgentConfig_Tips), TRUE );
    aConfig.WriteKey( ByteString::CreateFromInt32( nTip ),
                      ByteString::CreateFromInt32( (USHORT)bEnable ) );
    if ( !bEnable )
    {
        USHORT nPos;
        if ( GetPIStarterList()->Seek_Entry( nTip, &nPos ) )
            GetPIStarterList()->Remove( nPos );
    }
    else
        GetPIStarterList()->Insert( nTip );
}

void SfxHelp_Impl::ResetPIStarterList()
{
    delete pPIStarterList;
    pPIStarterList = 0;

    Config aConfig( GetHelpAgentConfig() );
    ImplSetLanguageGroup( aConfig, String::CreateFromAscii(zsHelpAgentConfig_Tips), TRUE );
    USHORT nTips = aConfig.GetKeyCount();
    ByteString aOn = ByteString::CreateFromInt32( 1 );
    for ( USHORT nTip = 0; nTip < nTips; nTip++ )
        aConfig.WriteKey( aConfig.GetKeyName( nTip ), aOn );
}

String SfxHelp_Impl::GetConfigDir( BOOL bGetSharedConfig )
{
#if SUPD<613//MUSTINI
    ULONG nKey = bGetSharedConfig ? SFX_KEY_CONFIG_DIR : SFX_KEY_USERCONFIG_PATH;
    SfxIniManager* pIni = SFX_INIMANAGER()->Find( nKey );
    if ( !pIni )
        pIni = SFX_INIMANAGER();
    return pIni->Get( nKey );
#else
    String sConfigDir;
    SvtPathOptions aPathCFG;
    if( bGetSharedConfig == sal_True )
    {
        sConfigDir = aPathCFG.GetConfigPath();
    }
    else
    {
        sConfigDir = aPathCFG.GetUserConfigPath();
    }
    return sConfigDir;
#endif
}

String SfxHelp_Impl::GetHelpAgentConfig()
{
    INetURLObject aObj( GetConfigDir( FALSE ), INET_PROT_FILE );
    aObj.insertName( String::CreateFromAscii( zsHelpAgentConfig ) );
    return aObj.PathToFileName();
}


void SfxHelp::SetCustomHelpFile( const String& rName )
{
    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    pHelp->aCustomHelpFile = rName;
}

void SfxHelp::ShowHint( ULONG nId )
{
    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    if ( pHelp )
    {
        pHelp->SlotExecutedOrFocusChanged( nId, TRUE, TRUE );
        // Falls sofort eine Slot-Id folgt, diese unterdruecken
         SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
        if ( pHelpPI )
            pHelpPI->SetTopicJustRequested( TRUE );
    }
}

BOOL SfxHelp::ShowHelp( ULONG nId, BOOL bShowInHelpAgent, const char* pFileName, BOOL bQuiet )
{
    // at the moment no implementation
    return FALSE;
}

BOOL SfxHelp::ShowHelp( const String& rKeyword, BOOL bShowInHelpAgent, const char* pFileName )
{
    // at the moment no implementation
    BOOL bDone = FALSE;
    return bDone;
}

USHORT SfxHelp::GetHelpFileInfoCount()
{
    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    return pHelp ? pHelp->GetHelpFileInfoCount() : 0;
}

HelpFileInfo* SfxHelp::GetHelpFileInfo( USHORT n )
{
    SfxHelp_Impl* pHelp = (SfxHelp_Impl*) Application::GetHelp();
    return pHelp ? pHelp->GetHelpFileInfo( n ) : NULL;
}

// class SfxHelpTipsWrapper ----------------------------------------------

SfxHelpTipsWrapper::SfxHelpTipsWrapper(Window *pParent,
    USHORT nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo) :
    SfxChildWindow(pParent, nId)
{
    // Skalierung des HelpPI initialisieren
    USHORT nScale = 100;
    if ( pInfo->aExtraString.Len() )
    {
        USHORT nPos = pInfo->aExtraString.Search(0x0023); // '#' = 23h
        if ( nPos != STRING_NOTFOUND )
        {
            String aScale = pInfo->aExtraString.Erase(nPos+1);
            pInfo->aExtraString.Erase(nPos,1);
            nScale = (USHORT) aScale.ToInt32();
        }
        else
        {
            // Es ist noch ein alter Eintrag (FloatingWindow); das Alignment
            // wird dann in Initialize nicht ver"andert.
            pInfo->aExtraString.Erase();
        }
    }

    // Versuche, das TipWindow zu laden
    SfxHelpTipsWindow* pDockWindow = new SfxHelpTipsWindow( pBindings, this, pParent );
    pWindow = pDockWindow;

    eChildAlignment = SFX_ALIGN_BOTTOM;
    if ( !pInfo->aSize.Width() )
    {
        Size aSz( 400, 150 );
        pWindow->SetSizePixel( aSz );
    }

    pDockWindow->Initialize( pInfo );
}

SfxHelpTipsWindow::SfxHelpTipsWindow( SfxBindings* pBindings, SfxChildWindow* pChildWin, Window* pParent )
    : SfxDockingWindow( pBindings, pChildWin, pParent,
        WB_CLOSEABLE | WB_DOCKABLE | WB_CLIPCHILDREN | WB_MOVEABLE | WB_SIZEABLE | WB_DIALOGCONTROL | WB_3DLOOK ),

      maCloseButton( this, 0 ),
      maCheckBox( this, 0 ),
      maTipWindow( this, WB_BORDER )
{
    mpHelpPI = NULL;

    maCloseButton.SetSymbol( SYMBOL_CLOSE );
    maCloseButton.SetClickHdl( LINK( this, SfxHelpTipsWindow, CloseButtonHdl ) );

    maCheckBox.SetClickHdl( LINK( this, SfxHelpTipsWindow, CheckBoxHdl ) );
    maCheckBox.SetHelpId( HID_HELPAGENT_TIP_BOX );
    maCheckBox.SetText( String( SfxResId( RID_STR_NOWELCOMESCREEN ) ) );

    PostUserEvent( LINK( this, SfxHelpTipsWindow, ShowTip ) );

#ifdef DEBUG
    maTipWindow.SetBackground( Wallpaper( COL_GREEN ) );
#endif

    maCloseButton.Show();
    maTipWindow.Show();
    maCheckBox.Show();
}

SfxHelpTipsWindow::~SfxHelpTipsWindow()
{
//    delete mpHelpPI;
}

void SfxHelpTipsWindow::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = FALSE; // Visibility wird ueber Config gesteuert
}

void SfxHelpTipsWindow::Resize()
{
    Size aOutSz( GetOutputSizePixel() );
    const long nButtonSize = 16;
    const long nSpace = 2;
    Size aCheckBoxSz = maCheckBox.CalcMinimumSize();
    aCheckBoxSz.Width() += 4; // FocusRect
    long nWindowPos = IsFloatingMode() ? 0 : (nButtonSize+2*nSpace);
    maCloseButton.Show( !IsFloatingMode() );

    maCloseButton.SetPosSizePixel( nSpace, nSpace, nButtonSize, nButtonSize );
    maCheckBox.SetPosSizePixel(     nWindowPos, aOutSz.Height() - aCheckBoxSz.Height() - nSpace,
                                    aCheckBoxSz.Width(), aCheckBoxSz.Height() );
    maTipWindow.SetPosSizePixel(    nWindowPos, 0,
                                    aOutSz.Width() - nWindowPos, aOutSz.Height() - aCheckBoxSz.Height() - 2*nSpace );

//    if ( mpHelpPI )
//        mpHelpPI->Resize();
}

IMPL_LINK( SfxHelpTipsWindow, ShowTip, void*, EMPTYARG )
{
    ULONG nId = 0;
    Config aConfig( SfxHelp_Impl::GetHelpAgentConfig() );
    ImplSetLanguageGroup( aConfig, DEFINE_CONST_UNICODE("WelcomeScreen"), TRUE );
    USHORT nTips = aConfig.GetKeyCount();
    for ( USHORT nTip = 0; nTip < nTips; nTip++ )
    {
        if ( (BOOL)(USHORT)aConfig.ReadKey( nTip ).ToInt32() )
        {
            nId = aConfig.GetKeyName( nTip ).ToInt32();
            break;
        }
    }

    if ( nId )
    {
        // at the moment no implementation
    }

    return 0;
}

IMPL_LINK( SfxHelpTipsWindow, CloseButtonHdl, Button*, EMPTYARG )
{
    Close();
    return 0;
}

IMPL_LINK( SfxHelpTipsWindow, CheckBoxHdl, CheckBox*, EMPTYARG )
{
    SvtHelpOptions().SetWelcomeScreen( !maCheckBox.IsChecked() );
    return 0;
}


