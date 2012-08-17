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

#include "baside2.hrc"
#include "basidesh.hrc"
#include "helpid.hrc"

#include "baside2.hxx" // unfortunately pModulWindow is needed partly...
#include "baside3.hxx"
#include "basobj.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"

#include <basic/basmgr.hxx>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

using ::std::vector;
using basctl::Layout;

DBG_NAME( IDEBaseWindow )

TYPEINIT0( IDEBaseWindow )
TYPEINIT1( SbxItem, SfxPoolItem );

IDEBaseWindow::IDEBaseWindow( Window* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName )
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

IDEBaseWindow::~IDEBaseWindow()
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
    DoInit();   // virtual...
}



void IDEBaseWindow::DoInit()
{
}



void IDEBaseWindow::GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
    pShellHScrollBar = pHScroll;
    pShellVScrollBar = pVScroll;
//  Init(); // does not make sense, leads to flickering and errors...
}



IMPL_LINK_INLINE_START( IDEBaseWindow, ScrollHdl, ScrollBar *, pCurScrollBar )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
    DoScroll( pCurScrollBar );
    return 0;
}
IMPL_LINK_INLINE_END( IDEBaseWindow, ScrollHdl, ScrollBar *, pCurScrollBar )


void IDEBaseWindow::ExecuteCommand (SfxRequest&)
{ }

void IDEBaseWindow::ExecuteGlobal (SfxRequest&)
{ }


long IDEBaseWindow::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKEvt = *rNEvt.GetKeyEvent();
        KeyCode aCode = aKEvt.GetKeyCode();
        sal_uInt16 nCode = aCode.GetCode();

        switch ( nCode )
        {
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                if ( aCode.IsMod1() )
                {
                    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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


void IDEBaseWindow::DoScroll( ScrollBar* )
{
    DBG_CHKTHIS( IDEBaseWindow, 0 );
}


void IDEBaseWindow::StoreData()
{
}

bool IDEBaseWindow::CanClose()
{
    return true;
}

bool IDEBaseWindow::AllowUndo()
{
    return true;
}



void IDEBaseWindow::UpdateData()
{
}

::rtl::OUString IDEBaseWindow::GetTitle()
{
    return ::rtl::OUString();
}

::rtl::OUString IDEBaseWindow::CreateQualifiedName()
{
    ::rtl::OUStringBuffer aName;
    if ( !m_aLibName.isEmpty() )
    {
        LibraryLocation eLocation = m_aDocument.getLibraryLocation( m_aLibName );
        aName.append(m_aDocument.getTitle(eLocation));
        aName.append('.');
        aName.append(m_aLibName);
        aName.append('.');
        aName.append(GetTitle());
    }
    return aName.makeStringAndClear();
}

void IDEBaseWindow::SetReadOnly (bool)
{
}

bool IDEBaseWindow::IsReadOnly ()
{
    return false;
}

void IDEBaseWindow::BasicStarted()
{
}

void IDEBaseWindow::BasicStopped()
{
}

bool IDEBaseWindow::IsModified ()
{
    return true;
}

bool IDEBaseWindow::IsPasteAllowed ()
{
    return false;
}

::svl::IUndoManager* IDEBaseWindow::GetUndoManager()
{
    return NULL;
}

sal_uInt16 IDEBaseWindow::GetSearchOptions()
{
    return 0;
}

sal_uInt16 IDEBaseWindow::StartSearchAndReplace (SvxSearchItem const&, bool bFromStart)
{
    static_cast<void>(bFromStart);
    return 0;
}

void IDEBaseWindow::OnNewDocument ()
{ }

void IDEBaseWindow::InsertLibInfo () const
{
    if (BasicIDEData* pData = BasicIDEGlobals::GetExtraData())
    {
        pData->GetLibInfos().InsertInfo(
            new LibInfoItem(m_aDocument, m_aLibName, m_aName, GetType())
        );
    }
}

bool IDEBaseWindow::Is (
    ScriptDocument const& rDocument,
    rtl::OUString const& rLibName, rtl::OUString const& rName,
    BasicIDEType eType, bool bFindSuspended
)
{
    if (bFindSuspended || !IsSuspended())
    {
        // any non-suspended window is ok
        if (rLibName.isEmpty() || rName.isEmpty() || eType == BASICIDE_TYPE_UNKNOWN)
            return true;
        // ok if the parameters match
        if (m_aDocument == rDocument && m_aLibName == rLibName && m_aName == rName && GetType() == eType)
            return true;
    }
    return false;
}

bool IDEBaseWindow::HasActiveEditor () const
{
    return false;
}


//
// BasicDockingWindow
// ==================
//

// style bits for BasicDockingWindow
WinBits const BasicDockingWindow::StyleBits =
    WB_BORDER | WB_3DLOOK | WB_CLIPCHILDREN |
    WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_DOCKABLE;

BasicDockingWindow::BasicDockingWindow (Window* pParent) :
    DockingWindow(pParent, StyleBits),
    pLayout(0),
    nShowCount(0)
{ }

BasicDockingWindow::BasicDockingWindow (Layout* pParent) :
    DockingWindow(pParent, StyleBits),
    pLayout(pParent),
    nShowCount(0)
{ }

// Sets the position and the size of the docking window. This property is saved
// when the window is floating. Called by Layout.
void BasicDockingWindow::ResizeIfDocking (Point const& rPos, Size const& rSize)
{
    Rectangle const rRect(rPos, rSize);
    if (rRect != aDockingRect)
    {
        // saving the position and the size
        aDockingRect = rRect;
        // resizing if actually docking
        if (!IsFloatingMode())
            SetPosSizePixel(rPos, rSize);
    }
}

// Sets the parent Layout window.
// The physical parent is set only when the window is docking.
void BasicDockingWindow::SetLayoutWindow (Layout* pLayout_)
{
    pLayout = pLayout_;
    if (!IsFloatingMode())
        SetParent(pLayout);

}

// Increases the "show" reference count.
// The window is shown when the reference count is positive.
void BasicDockingWindow::Show (bool bShow) // = true
{
    if (bShow)
    {
        if (++nShowCount == 1)
            DockingWindow::Show();
    }
    else
    {
        if (--nShowCount == 0)
            DockingWindow::Hide();
    }
}

// Decreases the "show" reference count.
// The window is hidden when the reference count reaches zero.
void BasicDockingWindow::Hide ()
{
    Show(false);
}

sal_Bool BasicDockingWindow::Docking( const Point& rPos, Rectangle& rRect )
{
    if (!IsDockingPrevented() && aDockingRect.IsInside(rPos))
    {
        rRect.SetSize(aDockingRect.GetSize());
        return false; // dock
    }
    else // adjust old size
    {
        if (!aFloatingRect.IsEmpty())
            rRect.SetSize(aFloatingRect.GetSize());
        return true; // float
    }
}

void BasicDockingWindow::EndDocking( const Rectangle& rRect, sal_Bool bFloatMode )
{
    if ( bFloatMode )
        DockingWindow::EndDocking( rRect, bFloatMode );
    else
    {
        SetFloatingMode(false);
        DockThis();
    }
}

void BasicDockingWindow::ToggleFloatingMode()
{
    if (IsFloatingMode())
    {
        if (!aFloatingRect.IsEmpty())
            SetPosSizePixel(
                GetParent()->ScreenToOutputPixel(aFloatingRect.TopLeft()),
                aFloatingRect.GetSize()
            );
    }
    DockThis();
}

sal_Bool BasicDockingWindow::PrepareToggleFloatingMode()
{
    if (IsFloatingMode())
    {
        // memorize position and size on the desktop...
        aFloatingRect = Rectangle(
            GetParent()->OutputToScreenPixel(GetPosPixel()),
            GetSizePixel()
        );
    }
    return true;
}

void BasicDockingWindow::StartDocking()
{
    if (IsFloatingMode())
    {
        aFloatingRect = Rectangle(
            GetParent()->OutputToScreenPixel(GetPosPixel()),
            GetSizePixel()
        );
    }
}

void BasicDockingWindow::DockThis ()
{
    // resizing when floating -> docking
    if (!IsFloatingMode())
    {
        Point const aPos = aDockingRect.TopLeft();
        Size const aSize = aDockingRect.GetSize();
        if (aSize != GetSizePixel() || aPos != GetPosPixel())
            SetPosSizePixel(aPos, aSize);
    }

    if (pLayout)
    {
        if (!IsFloatingMode() && GetParent() != pLayout)
            SetParent(pLayout);
        pLayout->DockaWindow(this);
    }
}


//
// ExtendedEdit
// ============
//

ExtendedEdit::ExtendedEdit( Window* pParent, IDEResId nRes ) :
    Edit( pParent, nRes )
{
    aAcc.SetSelectHdl( LINK( this, ExtendedEdit, EditAccHdl ) );
    Control::SetGetFocusHdl( LINK( this, ExtendedEdit, ImplGetFocusHdl ) );
    Control::SetLoseFocusHdl( LINK( this, ExtendedEdit, ImplLoseFocusHdl ) );
}

IMPL_LINK_NOARG(ExtendedEdit, ImplGetFocusHdl)
{
    Application::InsertAccel( &aAcc );
    aLoseFocusHdl.Call( this );
    return 0;
}


IMPL_LINK_NOARG(ExtendedEdit, ImplLoseFocusHdl)
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



struct TabBarDDInfo
{
    sal_uLong   npTabBar;
    sal_uInt16  nPage;

    TabBarDDInfo() { npTabBar = 0; nPage = 0; }
    TabBarDDInfo( sal_uLong _npTabBar, sal_uInt16 _nPage ) { npTabBar = _npTabBar; nPage = _nPage; }
};


BasicIDETabBar::BasicIDETabBar( Window* pParent ) :
    TabBar( pParent, WinBits( WB_3DLOOK | WB_SCROLL | WB_BORDER | WB_SIZEABLE | WB_DRAG ) )
{
    EnableEditMode(true);

    SetHelpId( HID_BASICIDE_TABBAR );
}

void BasicIDETabBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) && !IsInEditMode() )
    {
        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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

void BasicIDETabBar::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ) && !IsInEditMode() )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        if ( rCEvt.IsMouseEvent() )     // select right tab
        {
            Point aP = PixelToLogic( aPos );
            MouseEvent aMouseEvent( aP, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT );
            TabBar::MouseButtonDown( aMouseEvent );
        }

        PopupMenu aPopup( IDEResId( RID_POPUP_TABBAR ) );
        if ( GetPageCount() == 0 )
        {
            aPopup.EnableItem(SID_BASICIDE_DELETECURRENT, false);
            aPopup.EnableItem(SID_BASICIDE_RENAMECURRENT, false);
            aPopup.EnableItem(SID_BASICIDE_HIDECURPAGE, false);
        }

        if ( StarBASIC::IsRunning() )
        {
            aPopup.EnableItem(SID_BASICIDE_DELETECURRENT, false);
            aPopup.EnableItem(SID_BASICIDE_RENAMECURRENT, false);
            aPopup.EnableItem(SID_BASICIDE_MODULEDLG, false);
        }

        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
        if ( pIDEShell )
        {
            ScriptDocument aDocument( pIDEShell->GetCurDocument() );
            ::rtl::OUString aOULibName( pIDEShell->GetCurLibName() );
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
            {
                aPopup.EnableItem(aPopup.GetItemId( 0 ), false);
                aPopup.EnableItem(SID_BASICIDE_DELETECURRENT, false);
                aPopup.EnableItem(SID_BASICIDE_RENAMECURRENT, false);
                aPopup.RemoveDisabledEntries();
            }
             if ( aDocument.isInVBAMode() )
            {
                // disable to delete or remove object modules in IDE
                if (BasicManager* pBasMgr = aDocument.getBasicManager())
                {
                    if (StarBASIC* pBasic = pBasMgr->GetLib(aOULibName))
                    {
                        IDEWindowTable& aIDEWindowTable = pIDEShell->GetIDEWindowTable();
                        IDEWindowTable::const_iterator it = aIDEWindowTable.find( GetCurPageId() );
                        if (it != aIDEWindowTable.end() && dynamic_cast<basctl::ModulWindow*>(it->second))
                        {
                            SbModule* pActiveModule = (SbModule*)pBasic->FindModule( it->second->GetName() );
                            if( pActiveModule && ( pActiveModule->GetModuleType() == script::ModuleType::DOCUMENT ) )
                            {
                                aPopup.EnableItem(SID_BASICIDE_DELETECURRENT, false);
                                aPopup.EnableItem(SID_BASICIDE_RENAMECURRENT, false);
                            }
                        }
                    }
                }
            }
        }


        if (pIDEShell)
            if (SfxViewFrame* pViewFrame = pIDEShell->GetViewFrame())
                if (SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher())
                    pDispatcher->Execute(aPopup.Execute(this, aPos));
    }
}

long BasicIDETabBar::AllowRenaming()
{
    bool const bValid = BasicIDE::IsValidSbxName( GetEditText() );

    if ( !bValid )
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();

    return bValid ? TABBAR_RENAMING_YES : TABBAR_RENAMING_NO;
}


void BasicIDETabBar::EndRenaming()
{
    if ( !IsEditModeCanceled() )
    {
        SfxUInt16Item aID( SID_BASICIDE_ARG_TABID, GetEditPageId() );
        SfxStringItem aNewName( SID_BASICIDE_ARG_MODULENAME, GetEditText() );
        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_NAMECHANGEDONTAB,
                                  SFX_CALLMODE_SYNCHRON, &aID, &aNewName, 0L );
        }
    }
}


namespace
{

// helper class for sorting TabBar
struct TabBarSortHelper
{
    sal_uInt16      nPageId;
    String          aPageText;

    bool operator < (TabBarSortHelper const& rComp) const
    {
        return aPageText.CompareIgnoreCaseToAscii(rComp.aPageText) == COMPARE_LESS;
    }
};

} // namespace

void BasicIDETabBar::Sort()
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    if ( pIDEShell )
    {
        IDEWindowTable& aIDEWindowTable = pIDEShell->GetIDEWindowTable();
        TabBarSortHelper aTabBarSortHelper;
        ::std::vector<TabBarSortHelper> aModuleList;
        ::std::vector<TabBarSortHelper> aDialogList;
        sal_uInt16 nPageCount = GetPageCount();
        sal_uInt16 i;

        // create module and dialog lists for sorting
        for ( i = 0; i < nPageCount; i++)
        {
            sal_uInt16 nId = GetPageId( i );
            aTabBarSortHelper.nPageId = nId;
            aTabBarSortHelper.aPageText = GetPageText( nId );
            IDEBaseWindow* pWin = aIDEWindowTable[ nId ];

            if (dynamic_cast<basctl::ModulWindow*>(pWin))
            {
                aModuleList.push_back( aTabBarSortHelper );
            }
            else if (dynamic_cast<basctl::DialogWindow*>(pWin))
            {
                aDialogList.push_back( aTabBarSortHelper );
            }
        }

        // sort module and dialog lists by page text
        ::std::sort( aModuleList.begin() , aModuleList.end() );
        ::std::sort( aDialogList.begin() , aDialogList.end() );


        sal_uInt16 nModules = sal::static_int_cast<sal_uInt16>( aModuleList.size() );
        sal_uInt16 nDialogs = sal::static_int_cast<sal_uInt16>( aDialogList.size() );

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

void CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, bool bEraseTrailingEmptyLines )
{
    sal_Int32 nStartPos = 0;
    sal_Int32 nLine = 0;
    while ( nLine < nStartLine )
    {
        nStartPos = searchEOL( rStr, nStartPos );
        if( nStartPos == -1 )
            break;
        nStartPos++;    // not the \n.
        nLine++;
    }

    DBG_ASSERTWARNING( nStartPos != -1, "CutLines: Startzeile nicht gefunden!" );

    if ( nStartPos != -1 )
    {
        sal_Int32 nEndPos = nStartPos;

        for ( sal_Int32 i = 0; i < nLines; i++ )
            nEndPos = searchEOL( rStr, nEndPos+1 );

        if ( nEndPos == -1 ) // might happen at the last line
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

sal_uLong CalcLineCount( SvStream& rStream )
{
    sal_uLong nLFs = 0;
    sal_uLong nCRs = 0;
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

LibInfoKey::LibInfoKey( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName )
    :m_aDocument( rDocument )
    ,m_aLibName( rLibName )
{
}

LibInfoKey::~LibInfoKey()
{
}

bool LibInfoKey::operator==( const LibInfoKey& rKey ) const
{
    bool bRet = false;
    if ( m_aDocument == rKey.m_aDocument && m_aLibName == rKey.m_aLibName )
        bRet = true;
    return bRet;
}

LibInfoItem::LibInfoItem( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rCurrentName, sal_uInt16 nCurrentType )
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

void LibInfos::RemoveInfoFor( const ScriptDocument& _rDocument )
{
    for (   LibInfoMap::iterator it = m_aLibInfoMap.begin();
            it != m_aLibInfoMap.end();
        )
    {
        if ( it->first.GetDocument() != _rDocument )
        {
            ++it;
            continue;
        }

        LibInfoItem* pItem = it->second;

        LibInfoMap::iterator next_it = it; ++next_it;
        m_aLibInfoMap.erase( it );
        it = next_it;

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

bool QueryDel( const ::rtl::OUString& rName, const ResId& rId, Window* pParent )
{
    ::rtl::OUString aQuery(rId.toString());
    ::rtl::OUStringBuffer aNameBuf( rName );
    aNameBuf.append('\'');
    aNameBuf.insert(sal_Int32(0), sal_Unicode('\''));
    aQuery = aQuery.replaceAll("XX", aNameBuf.makeStringAndClear());
    QueryBox aQueryBox( pParent, WB_YES_NO | WB_DEF_YES, aQuery );
    return ( aQueryBox.Execute() == RET_YES );
}

bool QueryDelMacro( const ::rtl::OUString& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMACRO ), pParent );
}

bool QueryReplaceMacro( const ::rtl::OUString& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYREPLACEMACRO ), pParent );
}

bool QueryDelDialog( const ::rtl::OUString& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELDIALOG ), pParent );
}

bool QueryDelLib( const ::rtl::OUString& rName, bool bRef, Window* pParent )
{
    return QueryDel( rName, IDEResId( bRef ? RID_STR_QUERYDELLIBREF : RID_STR_QUERYDELLIB ), pParent );
}

bool QueryDelModule( const ::rtl::OUString& rName, Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMODULE ), pParent );
}

bool QueryPassword( const Reference< script::XLibraryContainer >& xLibContainer, const ::rtl::OUString& rLibName, ::rtl::OUString& rPassword, bool bRepeat, bool bNewTitle )
{
    bool bOK = false;
    sal_uInt16 nRet = 0;

    do
    {
        // password dialog
        SfxPasswordDialog* pDlg = new SfxPasswordDialog( Application::GetDefDialogParent() );
        pDlg->SetMinLen( 1 );

        // set new title
        if ( bNewTitle )
        {
            ::rtl::OUString aTitle(IDE_RESSTR(RID_STR_ENTERPASSWORD));
            aTitle = aTitle.replaceAll("XX", rLibName);
            pDlg->SetText( aTitle );
        }

        // execute dialog
        nRet = pDlg->Execute();

        // verify password
        if ( nRet == RET_OK )
        {
            if ( xLibContainer.is() && xLibContainer->hasByName( rLibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( rLibName ) && !xPasswd->isLibraryPasswordVerified( rLibName ) )
                {
                    rPassword = pDlg->GetPassword();
                    //                    ::rtl::OUString aOUPassword( rPassword );
                    bOK = xPasswd->verifyLibraryPassword( rLibName, rPassword );

                    if ( !bOK )
                    {
                        ErrorBox aErrorBox( Application::GetDefDialogParent(), WB_OK, IDE_RESSTR(RID_STR_WRONGPASSWORD) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
