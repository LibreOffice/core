/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bastypes.cxx,v $
 * $Revision: 1.32 $
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
#include "precompiled_basctl.hxx"


#include <vector>
#include <algorithm>

#include <ide_pch.hxx>


#include <basic/sbx.hxx>
#include <helpid.hrc>
#include <basidesh.hrc>
#include <bastypes.hxx>
#include <bastype2.hxx>
#include <baside2.hxx>  // Leider brauche ich teilweise pModulWindow...
#include <baside3.hxx>
#include <baside2.hrc>
#include <svtools/textview.hxx>
#include <svtools/texteng.hxx>
#include <basobj.hxx>
#include <sbxitem.hxx>
#include <iderdll.hxx>

#ifndef _PASSWD_HXX //autogen
#include <sfx2/passwd.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#include <com/sun/star/script/XLibraryContainerPassword.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


DBG_NAME( IDEBaseWindow )

const char* pRegName = "BasicIDETabBar";

TYPEINIT0( IDEBaseWindow )
TYPEINIT1( SbxItem, SfxPoolItem );

IDEBaseWindow::IDEBaseWindow( Window* pParent, const ScriptDocument& rDocument, String aLibName, String aName )
    :Window( pParent, WinBits( WB_3DLOOK ) )
    ,m_aDocument( rDocument )
    ,m_aLibName( aLibName )
    ,m_aName( aName )
{
    DBG_CTOR( IDEBaseWindow, 0 );
    pShellHScrollBar = 0;
    pShellVScrollBar = 0;
    nStatus = 0;
}



__EXPORT IDEBaseWindow::~IDEBaseWindow()
{
    DBG_DTOR( IDEBaseWindow, 0 );
    if ( pShellVScrollBar )
        pShellVScrollBar->SetScrollHdl( Link() );
    if ( pShellHScrollBar )
        pShellHScrollBar->SetScrollHdl( Link() );
}



void IDEBaseWindow::Init()
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
    if ( pShellVScrollBar )
        pShellVScrollBar->SetScrollHdl( LINK( this, IDEBaseWindow, ScrollHdl ) );
    if ( pShellHScrollBar )
        pShellHScrollBar->SetScrollHdl( LINK( this, IDEBaseWindow, ScrollHdl ) );
    DoInit();   // virtuell...
}



void __EXPORT IDEBaseWindow::DoInit()
{
}



void IDEBaseWindow::GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
    pShellHScrollBar = pHScroll;
    pShellVScrollBar = pVScroll;
//  Init(); // macht kein Sinn, fuehrt zu flackern, fuehr zu Fehlern...
}



IMPL_LINK_INLINE_START( IDEBaseWindow, ScrollHdl, ScrollBar *, pCurScrollBar )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
    DoScroll( pCurScrollBar );
    return 0;
}
IMPL_LINK_INLINE_END( IDEBaseWindow, ScrollHdl, ScrollBar *, pCurScrollBar )



void __EXPORT IDEBaseWindow::ExecuteCommand( SfxRequest& )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
}



void __EXPORT IDEBaseWindow::GetState( SfxItemSet& )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
}


long IDEBaseWindow::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKEvt = *rNEvt.GetKeyEvent();
        KeyCode aCode = aKEvt.GetKeyCode();
        USHORT nCode = aCode.GetCode();

        switch ( nCode )
        {
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                if ( aCode.IsMod1() )
                {
                    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                    if ( pIDEShell )
                        pIDEShell->NextPage( nCode == KEY_PAGEUP );

                    nDone = 1;
                }
            }
            break;
        }
    }

    return nDone ? nDone : Window::Notify( rNEvt );
}


void __EXPORT IDEBaseWindow::DoScroll( ScrollBar* )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
}


void __EXPORT IDEBaseWindow::StoreData()
{
}

BOOL __EXPORT IDEBaseWindow::CanClose()
{
    return TRUE;
}

BOOL __EXPORT IDEBaseWindow::AllowUndo()
{
    return TRUE;
}



void __EXPORT IDEBaseWindow::UpdateData()
{
}



void __EXPORT IDEBaseWindow::PrintData( Printer* )
{
}



String __EXPORT IDEBaseWindow::GetTitle()
{
    return String();
}



String IDEBaseWindow::CreateQualifiedName()
{
    String aName;
    if ( m_aLibName.Len() )
    {
        LibraryLocation eLocation = m_aDocument.getLibraryLocation( m_aLibName );
        aName = m_aDocument.getTitle( eLocation );
        aName += '.';
        aName += m_aLibName;
        aName += '.';
        aName += GetTitle();
    }

    return aName;
}

void IDEBaseWindow::SetReadOnly( BOOL )
{
}

BOOL IDEBaseWindow::IsReadOnly()
{
    return FALSE;
}

void __EXPORT IDEBaseWindow::BasicStarted()
{
}

void __EXPORT IDEBaseWindow::BasicStopped()
{
}

BOOL __EXPORT IDEBaseWindow::IsModified()
{
    return TRUE;
}

BOOL __EXPORT IDEBaseWindow::IsPasteAllowed()
{
    return FALSE;
}

Window* __EXPORT IDEBaseWindow::GetLayoutWindow()
{
    return this;
}

SfxUndoManager* __EXPORT IDEBaseWindow::GetUndoManager()
{
    return NULL;
}

BreakPointList::BreakPointList()
{}

BreakPointList::BreakPointList(BreakPointList const & rList):
    BreakPL( sal::static_int_cast<USHORT>( rList.Count() ))
{
    for (ULONG i = 0; i < rList.Count(); ++i)
        Insert(new BreakPoint(*rList.GetObject(i)), i);
}

BreakPointList::~BreakPointList()
{
    reset();
}

void BreakPointList::reset()
{
    while (Count() > 0)
        delete Remove(Count() - 1);
}

void BreakPointList::transfer(BreakPointList & rList)
{
    reset();
    for (ULONG i = 0; i < rList.Count(); ++i)
        Insert(rList.GetObject(i), i);
    rList.Clear();
}

void BreakPointList::InsertSorted( BreakPoint* pNewBrk )
{
    BreakPoint* pBrk = First();
    while ( pBrk )
    {
        if ( pNewBrk->nLine <= pBrk->nLine )
        {
            DBG_ASSERT( ( pBrk->nLine != pNewBrk->nLine ) || pNewBrk->bTemp, "BreakPoint existiert schon!" );
            Insert( pNewBrk );
            return;
        }
        pBrk = Next();
    }
    // Keine Einfuegeposition gefunden => LIST_APPEND
    Insert( pNewBrk, LIST_APPEND );
}

void BreakPointList::SetBreakPointsInBasic( SbModule* pModule )
{
    pModule->ClearAllBP();

    BreakPoint* pBrk = First();
    while ( pBrk )
    {
        if ( pBrk->bEnabled )
            pModule->SetBP( (USHORT)pBrk->nLine );
        pBrk = Next();
    }
}

BreakPoint* BreakPointList::FindBreakPoint( ULONG nLine )
{
    BreakPoint* pBrk = First();
    while ( pBrk )
    {
        if ( pBrk->nLine == nLine )
            return pBrk;

        pBrk = Next();
    }

    return (BreakPoint*)0;
}



void BreakPointList::AdjustBreakPoints( ULONG nLine, BOOL bInserted )
{
    BreakPoint* pBrk = First();
    while ( pBrk )
    {
        BOOL bDelBrk = FALSE;
        if ( pBrk->nLine == nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                bDelBrk = TRUE;
        }
        else if ( pBrk->nLine > nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                pBrk->nLine--;
        }

        if ( bDelBrk )
        {
            ULONG n = GetCurPos();
            delete Remove( pBrk );
            pBrk = Seek( n );
        }
        else
        {
            pBrk = Next();
        }
    }
}

void BreakPointList::ResetHitCount()
{
    BreakPoint* pBrk = First();
    while ( pBrk )
    {
        pBrk->nHitCount = 0;
        pBrk = Next();
    }
}

void IDEBaseWindow::Deactivating()
{
}

USHORT __EXPORT IDEBaseWindow::GetSearchOptions()
{
    return 0;
}


BasicDockingWindow::BasicDockingWindow( Window* pParent ) :
    DockingWindow( pParent, WB_BORDER | WB_3DLOOK | WB_DOCKABLE | WB_MOVEABLE |
                            WB_SIZEABLE | WB_ROLLABLE |
                            WB_DOCKABLE | WB_CLIPCHILDREN )
{
}



BOOL __EXPORT BasicDockingWindow::Docking( const Point& rPos, Rectangle& rRect )
{
    ModulWindowLayout* pLayout = (ModulWindowLayout*)GetParent();
    Rectangle aTmpRec( rRect );
    BOOL bDock = IsDockingPrevented() ? FALSE : pLayout->IsToBeDocked( this, rPos, aTmpRec );
    if ( bDock )
    {
        rRect.SetSize( aTmpRec.GetSize() );
    }
    else    // Alte Groesse einstellen
    {
        if ( !aFloatingPosAndSize.IsEmpty() )
            rRect.SetSize( aFloatingPosAndSize.GetSize() );
    }
    return !bDock;  // bFloat
}



void __EXPORT BasicDockingWindow::EndDocking( const Rectangle& rRect, BOOL bFloatMode )
{
    if ( bFloatMode )
        DockingWindow::EndDocking( rRect, bFloatMode );
    else
    {
        SetFloatingMode( FALSE );
        ModulWindowLayout* pLayout = (ModulWindowLayout*)GetParent();
        pLayout->DockaWindow( this );
    }
}



void __EXPORT BasicDockingWindow::ToggleFloatingMode()
{
    ModulWindowLayout* pLayout = (ModulWindowLayout*)GetParent();
    if ( IsFloatingMode() )
    {
        if ( !aFloatingPosAndSize.IsEmpty() )
            SetPosSizePixel( GetParent()->ScreenToOutputPixel( aFloatingPosAndSize.TopLeft() ),
                aFloatingPosAndSize.GetSize() );
    }
    pLayout->DockaWindow( this );
}



BOOL __EXPORT BasicDockingWindow::PrepareToggleFloatingMode()
{
    if ( IsFloatingMode() )
    {
        // Position und Groesse auf dem Desktop merken...
        aFloatingPosAndSize.SetPos( GetParent()->OutputToScreenPixel( GetPosPixel() ) );
        aFloatingPosAndSize.SetSize( GetSizePixel() );
    }
    return TRUE;
}



void __EXPORT BasicDockingWindow::StartDocking()
{
    // Position und Groesse auf dem Desktop merken...
    if ( IsFloatingMode() )
    {
        aFloatingPosAndSize.SetPos( GetParent()->OutputToScreenPixel( GetPosPixel() ) );
        aFloatingPosAndSize.SetSize( GetSizePixel() );
    }
}




BasicToolBox::BasicToolBox( Window* pParent, IDEResId nRes ) :
    ToolBox( pParent, nRes )
{
}



void __EXPORT BasicToolBox::MouseButtonDown( const MouseEvent &rEvt )
{
    ToolBox::MouseButtonDown( rEvt );
    if ( !GetCurItemId() )
        ((BasicDockingWindow*)GetParent())->MouseButtonDown( rEvt );
}




ExtendedEdit::ExtendedEdit( Window* pParent, IDEResId nRes ) :
    Edit( pParent, nRes )
{
    aAcc.SetSelectHdl( LINK( this, ExtendedEdit, EditAccHdl ) );
    Control::SetGetFocusHdl( LINK( this, ExtendedEdit, ImplGetFocusHdl ) );
    Control::SetLoseFocusHdl( LINK( this, ExtendedEdit, ImplLoseFocusHdl ) );
}

IMPL_LINK( ExtendedEdit, ImplGetFocusHdl, Control*, EMPTYARG )
{
    Application::InsertAccel( &aAcc );
    aLoseFocusHdl.Call( this );
    return 0;
}


IMPL_LINK( ExtendedEdit, ImplLoseFocusHdl, Control*, EMPTYARG )
{
    Application::RemoveAccel( &aAcc );
    return 0;
}


IMPL_LINK_INLINE_START( ExtendedEdit, EditAccHdl, Accelerator *, pAcc )
{
    aAccHdl.Call( pAcc );
    return 0;
}
IMPL_LINK_INLINE_END( ExtendedEdit, EditAccHdl, Accelerator *, pAcc )



ExtendedMultiLineEdit::ExtendedMultiLineEdit( Window* pParent, IDEResId nRes ) :
    MultiLineEdit( pParent, nRes )
{
    aAcc.SetSelectHdl( LINK( this, ExtendedMultiLineEdit, EditAccHdl ) );
    Control::SetGetFocusHdl( LINK( this, ExtendedMultiLineEdit, ImplGetFocusHdl ) );
    Control::SetLoseFocusHdl( LINK( this, ExtendedMultiLineEdit, ImplLoseFocusHdl ) );
}

IMPL_LINK( ExtendedMultiLineEdit, ImplGetFocusHdl, Control*, EMPTYARG )
{
    Application::InsertAccel( &aAcc );
    return 0;
}


IMPL_LINK( ExtendedMultiLineEdit, ImplLoseFocusHdl, Control*, EMPTYARG )
{
    Application::RemoveAccel( &aAcc );
    return 0;
}

IMPL_LINK_INLINE_START( ExtendedMultiLineEdit, EditAccHdl, Accelerator *, pAcc )
{
    aAccHdl.Call( pAcc );
    return 0;
}
IMPL_LINK_INLINE_END( ExtendedMultiLineEdit, EditAccHdl, Accelerator *, pAcc )

struct TabBarDDInfo
{
    ULONG   npTabBar;
    USHORT  nPage;

    TabBarDDInfo() { npTabBar = 0; nPage = 0; }
    TabBarDDInfo( ULONG _npTabBar, USHORT _nPage ) { npTabBar = _npTabBar; nPage = _nPage; }
};


BasicIDETabBar::BasicIDETabBar( Window* pParent ) :
    TabBar( pParent, WinBits( WB_3DLOOK | WB_SCROLL | WB_BORDER | WB_SIZEABLE | WB_DRAG ) )
{
    EnableEditMode( TRUE );

    SetHelpId( HID_BASICIDE_TABBAR );
}

void __EXPORT BasicIDETabBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) && !IsInEditMode() )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_MODULEDLG );
        }
    }
    else
    {
        TabBar::MouseButtonDown( rMEvt );
    }
}

void __EXPORT BasicIDETabBar::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ) && !IsInEditMode() )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        if ( rCEvt.IsMouseEvent() )     // Richtige Tab selektieren
        {
            Point aP = PixelToLogic( aPos );
            MouseEvent aMouseEvent( aP, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT );
            TabBar::MouseButtonDown( aMouseEvent );
        }

        PopupMenu aPopup( IDEResId( RID_POPUP_TABBAR ) );
        if ( GetPageCount() == 0 )
        {
            aPopup.EnableItem( SID_BASICIDE_DELETECURRENT, FALSE );
            aPopup.EnableItem( SID_BASICIDE_RENAMECURRENT, FALSE );
            aPopup.EnableItem( SID_BASICIDE_HIDECURPAGE, FALSE );
        }

        if ( StarBASIC::IsRunning() )
        {
            aPopup.EnableItem(SID_BASICIDE_DELETECURRENT, false);
            aPopup.EnableItem( SID_BASICIDE_RENAMECURRENT, false);
            aPopup.EnableItem(SID_BASICIDE_MODULEDLG, false);
        }

        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        if ( pIDEShell )
        {
            ScriptDocument aDocument( pIDEShell->GetCurDocument() );
            ::rtl::OUString aOULibName( pIDEShell->GetCurLibName() );
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
            {
                aPopup.EnableItem( aPopup.GetItemId( 0 ), FALSE );
                aPopup.EnableItem( SID_BASICIDE_DELETECURRENT, FALSE );
                aPopup.EnableItem( SID_BASICIDE_RENAMECURRENT, FALSE );
                aPopup.RemoveDisabledEntries();
            }
        }

        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if ( pDispatcher )
            pDispatcher->Execute( aPopup.Execute( this, aPos ) );
    }
}

long BasicIDETabBar::AllowRenaming()
{
    BOOL bValid = BasicIDE::IsValidSbxName( GetEditText() );

    if ( !bValid )
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();

    return bValid ? TAB_RENAMING_YES : TAB_RENAMING_NO;
}


void __EXPORT BasicIDETabBar::EndRenaming()
{
    if ( !IsEditModeCanceled() )
    {
        SfxUInt16Item aID( SID_BASICIDE_ARG_TABID, GetEditPageId() );
        SfxStringItem aNewName( SID_BASICIDE_ARG_MODULENAME, GetEditText() );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_NAMECHANGEDONTAB,
                                  SFX_CALLMODE_SYNCHRON, &aID, &aNewName, 0L );
        }
    }
}


void BasicIDETabBar::Sort()
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        IDEWindowTable& aIDEWindowTable = pIDEShell->GetIDEWindowTable();
        TabBarSortHelper aTabBarSortHelper;
        ::std::vector<TabBarSortHelper> aModuleList;
        ::std::vector<TabBarSortHelper> aDialogList;
        USHORT nPageCount = GetPageCount();
        USHORT i;

        // create module and dialog lists for sorting
        for ( i = 0; i < nPageCount; i++)
        {
            USHORT nId = GetPageId( i );
            aTabBarSortHelper.nPageId = nId;
            aTabBarSortHelper.aPageText = GetPageText( nId );
            IDEBaseWindow* pWin = aIDEWindowTable.Get( nId );

            if ( pWin->IsA( TYPE( ModulWindow ) ) )
            {
                aModuleList.push_back( aTabBarSortHelper );
            }
            else if ( pWin->IsA( TYPE( DialogWindow ) ) )
            {
                aDialogList.push_back( aTabBarSortHelper );
            }
        }

        // sort module and dialog lists by page text
        ::std::sort( aModuleList.begin() , aModuleList.end() );
        ::std::sort( aDialogList.begin() , aDialogList.end() );


        USHORT nModules = sal::static_int_cast<USHORT>( aModuleList.size() );
        USHORT nDialogs = sal::static_int_cast<USHORT>( aDialogList.size() );

        // move module pages to new positions
        for (i = 0; i < nModules; i++)
        {
            MovePage( aModuleList[i].nPageId , i );
        }

        // move dialog pages to new positions
        for (i = 0; i < nDialogs; i++)
        {
            MovePage( aDialogList[i].nPageId , nModules + i );
        }
    }
}

void CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, BOOL bEraseTrailingEmptyLines )
{
    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = 0;
    sal_Int32 nLine = 0;
    while ( nLine < nStartLine )
    {
        nStartPos = searchEOL( rStr, nStartPos );
        if( nStartPos == -1 )
            break;
        nStartPos++;    // nicht das \n.
        nLine++;
    }

    DBG_ASSERTWARNING( nStartPos != -1, "CutLines: Startzeile nicht gefunden!" );

    if ( nStartPos != -1 )
    {
        nEndPos = nStartPos;
        for ( sal_Int32 i = 0; i < nLines; i++ )
            nEndPos = searchEOL( rStr, nEndPos+1 );

        if ( nEndPos == -1 ) // kann bei letzter Zeile passieren
            nEndPos = rStr.getLength();
        else
            nEndPos++;

        ::rtl::OUString aEndStr = rStr.copy( nEndPos );
        rStr = rStr.copy( 0, nStartPos );
        rStr += aEndStr;
    }
    if ( bEraseTrailingEmptyLines )
    {
        sal_Int32 n = nStartPos;
        sal_Int32 nLen = rStr.getLength();
        while ( ( n < nLen ) && ( rStr.getStr()[ n ] == LINE_SEP ||
                                  rStr.getStr()[ n ] == LINE_SEP_CR ) )
        {
            n++;
        }

        if ( n > nStartPos )
        {
            ::rtl::OUString aEndStr = rStr.copy( n );
            rStr = rStr.copy( 0, nStartPos );
            rStr += aEndStr;
        }
    }
}

ULONG CalcLineCount( SvStream& rStream )
{
    ULONG nLFs = 0;
    ULONG nCRs = 0;
    char c;

    rStream.Seek( 0 );
    rStream >> c;
    while ( !rStream.IsEof() )
    {
        if ( c == '\n' )
            nLFs++;
        else if ( c == '\r' )
            nCRs++;
        rStream >> c;
    }

    rStream.Seek( 0 );
    if ( nLFs > nCRs )
        return nLFs;
    return nCRs;
}

LibInfoKey::LibInfoKey()
    :m_aDocument( ScriptDocument::getApplicationScriptDocument() )
{
}

LibInfoKey::LibInfoKey( const ScriptDocument& rDocument, const String& rLibName )
    :m_aDocument( rDocument )
    ,m_aLibName( rLibName )
{
}

LibInfoKey::~LibInfoKey()
{
}

LibInfoKey::LibInfoKey( const LibInfoKey& rKey )
    :m_aDocument( rKey.m_aDocument )
    ,m_aLibName( rKey.m_aLibName )
{
}

LibInfoKey& LibInfoKey::operator=( const LibInfoKey& rKey )
{
    m_aDocument = rKey.m_aDocument;
    m_aLibName = rKey.m_aLibName;
    return *this;
}

bool LibInfoKey::operator==( const LibInfoKey& rKey ) const
{
    bool bRet = false;
    if ( m_aDocument == rKey.m_aDocument && m_aLibName == rKey.m_aLibName )
        bRet = true;
    return bRet;
}

LibInfoItem::LibInfoItem()
    :m_aDocument( ScriptDocument::getApplicationScriptDocument() )
    ,m_nCurrentType( 0 )
{
}

LibInfoItem::LibInfoItem( const ScriptDocument& rDocument, const String& rLibName, const String& rCurrentName, USHORT nCurrentType )
    :m_aDocument( rDocument )
    ,m_aLibName( rLibName )
    ,m_aCurrentName( rCurrentName )
    ,m_nCurrentType( nCurrentType )
{
}

LibInfoItem::~LibInfoItem()
{
}

LibInfoItem::LibInfoItem( const LibInfoItem& rItem )
    :m_aDocument( rItem.m_aDocument )
    ,m_aLibName( rItem.m_aLibName )
    ,m_aCurrentName( rItem.m_aCurrentName )
    ,m_nCurrentType( rItem.m_nCurrentType )
{
}

LibInfoItem& LibInfoItem::operator=( const LibInfoItem& rItem )
{
    m_aDocument = rItem.m_aDocument;
    m_aLibName = rItem.m_aLibName;
    m_aCurrentName = rItem.m_aCurrentName;
    m_nCurrentType = rItem.m_nCurrentType;

    return *this;
}

LibInfos::LibInfos()
{
}

LibInfos::~LibInfos()
{
    LibInfoMap::iterator end = m_aLibInfoMap.end();
    for ( LibInfoMap::iterator it = m_aLibInfoMap.begin(); it != end; ++it )
        delete it->second;
    m_aLibInfoMap.clear();
}

void LibInfos::InsertInfo( LibInfoItem* pItem )
{
    LibInfoKey aKey( pItem->GetDocument(), pItem->GetLibName() );
    LibInfoMap::iterator it = m_aLibInfoMap.find( aKey );
    if ( it != m_aLibInfoMap.end() )
    {
        LibInfoItem* pI = it->second;
        m_aLibInfoMap.erase( it );
        delete pI;
    }
    m_aLibInfoMap.insert( LibInfoMap::value_type( aKey, pItem ) );
}

void LibInfos::RemoveInfo( const LibInfoKey& rKey )
{
    LibInfoMap::iterator it = m_aLibInfoMap.find( rKey );
    if ( it != m_aLibInfoMap.end() )
    {
        LibInfoItem* pItem = it->second;
        m_aLibInfoMap.erase( it );
        delete pItem;
    }
}

LibInfoItem* LibInfos::GetInfo( const LibInfoKey& rKey )
{
    LibInfoItem* pItem = 0;
    LibInfoMap::iterator it = m_aLibInfoMap.find( rKey );
    if ( it != m_aLibInfoMap.end() )
        pItem = it->second;
    return pItem;
}

SbxItem::SbxItem(USHORT nWhich_, const ScriptDocument& rDocument, const String& aLibName, const String& aName, USHORT nType )
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(USHORT nWhich_, const ScriptDocument& rDocument, const String& aLibName, const String& aName, const String& aMethodName, USHORT nType )
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_aMethodName(aMethodName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(const SbxItem& rCopy)
    :SfxPoolItem( rCopy )
    ,m_aDocument( rCopy.m_aDocument )
{
    m_aLibName = rCopy.m_aLibName;
    m_aName = rCopy.m_aName;
    m_aMethodName = rCopy.m_aMethodName;
    m_nType = rCopy.m_nType;
}

int SbxItem::operator==( const SfxPoolItem& rCmp) const
{
    DBG_ASSERT( rCmp.ISA( SbxItem ), "==: Kein SbxItem!" );
    return ( SfxPoolItem::operator==( rCmp ) && ( m_aDocument == ((const SbxItem&)rCmp).m_aDocument )
                                             && ( m_aLibName == ((const SbxItem&)rCmp).m_aLibName )
                                             && ( m_aName == ((const SbxItem&)rCmp).m_aName )
                                             && ( m_aMethodName == ((const SbxItem&)rCmp).m_aMethodName )
                                             && ( m_nType == ((const SbxItem&)rCmp).m_nType ) );
}

SfxPoolItem *SbxItem::Clone( SfxItemPool* ) const
{
    return new SbxItem(*this);
}

BOOL QueryDel( const String& rName, const ResId& rId, Window* pParent )
{
    String aQuery( rId );
    String aName( rName );
    aName += '\'';
    aName.Insert( '\'', 0 );
    aQuery.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aName );
    QueryBox aQueryBox( pParent, WB_YES_NO | WB_DEF_YES, aQuery );
    if ( aQueryBox.Execute() == RET_YES )
        return TRUE;
    return FALSE;
}

BOOL QueryDelMacro( const String& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMACRO ), pParent );
}

BOOL QueryReplaceMacro( const String& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYREPLACEMACRO ), pParent );
}

BOOL QueryDelDialog( const String& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELDIALOG ), pParent );
}

BOOL QueryDelLib( const String& rName, BOOL bRef, Window* pParent )
{
    return QueryDel( rName, IDEResId( bRef ? RID_STR_QUERYDELLIBREF : RID_STR_QUERYDELLIB ), pParent );
}

BOOL QueryDelModule( const String& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMODULE ), pParent );
}

BOOL QueryPassword( const Reference< script::XLibraryContainer >& xLibContainer, const String& rLibName, String& rPassword, BOOL bRepeat, BOOL bNewTitle )
{
    BOOL bOK = FALSE;
    USHORT nRet = 0;

    do
    {
        // password dialog
        SfxPasswordDialog* pDlg = new SfxPasswordDialog( Application::GetDefDialogParent() );
        pDlg->SetMinLen( 1 );

        // set new title
        if ( bNewTitle )
        {
            String aTitle( IDEResId( RID_STR_ENTERPASSWORD ) );
            aTitle.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), rLibName );
            pDlg->SetText( aTitle );
        }

        // execute dialog
        nRet = pDlg->Execute();

        // verify password
        if ( nRet == RET_OK )
        {
            ::rtl::OUString aOULibName( rLibName );
            if ( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                {
                    rPassword = pDlg->GetPassword();
                    ::rtl::OUString aOUPassword( rPassword );
                    bOK = xPasswd->verifyLibraryPassword( aOULibName, aOUPassword );

                    if ( !bOK )
                    {
                        ErrorBox aErrorBox( Application::GetDefDialogParent(), WB_OK, String( IDEResId( RID_STR_WRONGPASSWORD ) ) );
                        aErrorBox.Execute();
                    }
                }
            }
        }

        delete pDlg;
    }
    while ( bRepeat && !bOK && nRet == RET_OK );

    return bOK;
}

