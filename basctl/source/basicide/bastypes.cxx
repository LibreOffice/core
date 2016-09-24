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

#include "basidesh.hrc"
#include "helpid.hrc"

#include "baside2.hxx"
#include "baside3.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"

#include <basic/basmgr.hxx>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/srchdefs.hxx>

namespace basctl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

BaseWindow::BaseWindow( vcl::Window* pParent, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName )
    :Window( pParent, WinBits( WB_3DLOOK ) )
    ,m_aDocument( rDocument )
    ,m_aLibName( aLibName )
    ,m_aName( aName )
{
    pShellHScrollBar = nullptr;
    pShellVScrollBar = nullptr;
    nStatus = 0;
}

BaseWindow::~BaseWindow()
{
    disposeOnce();
}

void BaseWindow::dispose()
{
    if ( pShellVScrollBar )
        pShellVScrollBar->SetScrollHdl( Link<ScrollBar*,void>() );
    if ( pShellHScrollBar )
        pShellHScrollBar->SetScrollHdl( Link<ScrollBar*,void>() );
    pShellVScrollBar.clear();
    pShellHScrollBar.clear();
    vcl::Window::dispose();
}


void BaseWindow::Init()
{
    if ( pShellVScrollBar )
        pShellVScrollBar->SetScrollHdl( LINK( this, BaseWindow, ScrollHdl ) );
    if ( pShellHScrollBar )
        pShellHScrollBar->SetScrollHdl( LINK( this, BaseWindow, ScrollHdl ) );
    DoInit();   // virtual...
}


void BaseWindow::DoInit()
{ }


void BaseWindow::GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll )
{
    pShellHScrollBar = pHScroll;
    pShellVScrollBar = pVScroll;
//  Init(); // does not make sense, leads to flickering and errors...
}


IMPL_LINK_TYPED( BaseWindow, ScrollHdl, ScrollBar *, pCurScrollBar, void )
{
    DoScroll( pCurScrollBar );
}

void BaseWindow::ExecuteCommand (SfxRequest&)
{ }

void BaseWindow::ExecuteGlobal (SfxRequest&)
{ }


bool BaseWindow::Notify( NotifyEvent& rNEvt )
{
    bool bDone = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        KeyEvent aKEvt = *rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = aKEvt.GetKeyCode();
        sal_uInt16 nCode = aCode.GetCode();

        switch ( nCode )
        {
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                if ( aCode.IsMod1() )
                {
                    if (Shell* pShell = GetShell())
                        pShell->NextPage( nCode == KEY_PAGEUP );
                    bDone = true;
                }
            }
            break;
        }
    }

    return bDone || Window::Notify( rNEvt );
}


void BaseWindow::DoScroll( ScrollBar* )
{
}


void BaseWindow::StoreData()
{
}

bool BaseWindow::CanClose()
{
    return true;
}

bool BaseWindow::AllowUndo()
{
    return true;
}


void BaseWindow::UpdateData()
{
}

OUString BaseWindow::GetTitle()
{
    return OUString();
}

OUString BaseWindow::CreateQualifiedName()
{
    OUString aName;
    if ( !m_aLibName.isEmpty() )
    {
        LibraryLocation eLocation = m_aDocument.getLibraryLocation( m_aLibName );
        aName = m_aDocument.getTitle(eLocation) + "." + m_aLibName + "." +
                GetTitle();
    }
    return aName;
}

void BaseWindow::SetReadOnly (bool)
{
}

bool BaseWindow::IsReadOnly ()
{
    return false;
}

void BaseWindow::BasicStarted()
{
}

void BaseWindow::BasicStopped()
{
}

bool BaseWindow::IsModified ()
{
    return true;
}

bool BaseWindow::IsPasteAllowed ()
{
    return false;
}

::svl::IUndoManager* BaseWindow::GetUndoManager()
{
    return nullptr;
}

SearchOptionFlags BaseWindow::GetSearchOptions()
{
    return SearchOptionFlags::NONE;
}

sal_uInt16 BaseWindow::StartSearchAndReplace (SvxSearchItem const&, bool bFromStart)
{
    static_cast<void>(bFromStart);
    return 0;
}

void BaseWindow::OnNewDocument ()
{ }

void BaseWindow::InsertLibInfo () const
{
    if (ExtraData* pData = GetExtraData())
        pData->GetLibInfo().InsertInfo(m_aDocument, m_aLibName, m_aName, GetType());
}

bool BaseWindow::Is (
    ScriptDocument const& rDocument,
    OUString const& rLibName, OUString const& rName,
    ItemType eType, bool bFindSuspended
)
{
    if (bFindSuspended || !IsSuspended())
    {
        // any non-suspended window is ok
        if (rLibName.isEmpty() || rName.isEmpty() || eType == TYPE_UNKNOWN)
            return true;
        // ok if the parameters match
        if (m_aDocument == rDocument && m_aLibName == rLibName && m_aName == rName && GetType() == eType)
            return true;
    }
    return false;
}

bool BaseWindow::HasActiveEditor () const
{
    return false;
}


// DockingWindow


// style bits for DockingWindow
WinBits const DockingWindow::StyleBits =
    WB_BORDER | WB_3DLOOK | WB_CLIPCHILDREN |
    WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_DOCKABLE;

DockingWindow::DockingWindow (vcl::Window* pParent) :
    ::DockingWindow(pParent, StyleBits),
    pLayout(nullptr),
    nShowCount(0)
{ }

DockingWindow::DockingWindow (Layout* pParent) :
    ::DockingWindow(pParent, StyleBits),
    pLayout(pParent),
    nShowCount(0)
{ }

DockingWindow::~DockingWindow()
{
    disposeOnce();
}

void DockingWindow::dispose()
{
    pLayout.clear();
    ::DockingWindow::dispose();
}

// Sets the position and the size of the docking window. This property is saved
// when the window is floating. Called by Layout.
void DockingWindow::ResizeIfDocking (Point const& rPos, Size const& rSize)
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
void DockingWindow::ResizeIfDocking (Size const& rSize)
{
    ResizeIfDocking(aDockingRect.TopLeft(), rSize);
}

// Sets the parent Layout window.
// The physical parent is set only when the window is docking.
void DockingWindow::SetLayoutWindow (Layout* pLayout_)
{
    pLayout = pLayout_;
    if (!IsFloatingMode())
        SetParent(pLayout);

}

// Increases the "show" reference count.
// The window is shown when the reference count is positive.
void DockingWindow::Show (bool bShow) // = true
{
    if (bShow)
    {
        if (++nShowCount == 1)
            ::DockingWindow::Show();
    }
    else
    {
        if (--nShowCount == 0)
            ::DockingWindow::Hide();
    }
}

// Decreases the "show" reference count.
// The window is hidden when the reference count reaches zero.
void DockingWindow::Hide ()
{
    Show(false);
}

bool DockingWindow::Docking( const Point& rPos, Rectangle& rRect )
{
    if (aDockingRect.IsInside(rPos))
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

void DockingWindow::EndDocking( const Rectangle& rRect, bool bFloatMode )
{
    if ( bFloatMode )
        ::DockingWindow::EndDocking( rRect, bFloatMode );
    else
    {
        SetFloatingMode(false);
        DockThis();
    }
}

void DockingWindow::ToggleFloatingMode()
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

bool DockingWindow::PrepareToggleFloatingMode()
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

void DockingWindow::StartDocking()
{
    if (IsFloatingMode())
    {
        aFloatingRect = Rectangle(
            GetParent()->OutputToScreenPixel(GetPosPixel()),
            GetSizePixel()
        );
    }
}

void DockingWindow::DockThis ()
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


// ExtendedEdit


ExtendedEdit::ExtendedEdit( vcl::Window* pParent, IDEResId nRes ) :
    Edit( pParent, nRes )
{
    aAcc.SetSelectHdl( LINK( this, ExtendedEdit, EditAccHdl ) );
    Control::SetGetFocusHdl( LINK( this, ExtendedEdit, ImplGetFocusHdl ) );
    Control::SetLoseFocusHdl( LINK( this, ExtendedEdit, ImplLoseFocusHdl ) );
}

IMPL_LINK_NOARG_TYPED(ExtendedEdit, ImplGetFocusHdl, Control&, void)
{
    Application::InsertAccel( &aAcc );
    aLoseFocusHdl.Call( this );
}


IMPL_LINK_NOARG_TYPED(ExtendedEdit, ImplLoseFocusHdl, Control&, void)
{
    Application::RemoveAccel( &aAcc );
}


IMPL_LINK_TYPED( ExtendedEdit, EditAccHdl, Accelerator&, rAcc, void )
{
    aAccHdl.Call( rAcc );
}

//  TabBar


TabBar::TabBar( vcl::Window* pParent ) :
    ::TabBar( pParent, WinBits( WB_3DLOOK | WB_SCROLL | WB_BORDER | WB_SIZEABLE | WB_DRAG ) )
{
    EnableEditMode();

    SetHelpId( HID_BASICIDE_TABBAR );
}

void TabBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) && !IsInEditMode() )
    {
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute( SID_BASICIDE_MODULEDLG );
    }
    else
    {
        ::TabBar::MouseButtonDown( rMEvt ); // base class version
    }
}

void TabBar::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == CommandEventId::ContextMenu ) && !IsInEditMode() )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        if ( rCEvt.IsMouseEvent() )     // select right tab
        {
            Point aP = PixelToLogic( aPos );
            MouseEvent aMouseEvent( aP, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT );
            ::TabBar::MouseButtonDown( aMouseEvent ); // base class
        }

        ScopedVclPtrInstance<PopupMenu> aPopup( IDEResId( RID_POPUP_TABBAR ) );
        if ( GetPageCount() == 0 )
        {
            aPopup->EnableItem(SID_BASICIDE_DELETECURRENT, false);
            aPopup->EnableItem(SID_BASICIDE_RENAMECURRENT, false);
            aPopup->EnableItem(SID_BASICIDE_HIDECURPAGE, false);
        }

        if ( StarBASIC::IsRunning() )
        {
            aPopup->EnableItem(SID_BASICIDE_DELETECURRENT, false);
            aPopup->EnableItem(SID_BASICIDE_RENAMECURRENT, false);
            aPopup->EnableItem(SID_BASICIDE_MODULEDLG, false);
        }

        if (Shell* pShell = GetShell())
        {
            ScriptDocument aDocument( pShell->GetCurDocument() );
            OUString       aOULibName( pShell->GetCurLibName() );
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
            {
                aPopup->EnableItem(aPopup->GetItemId( 0 ), false);
                aPopup->EnableItem(SID_BASICIDE_DELETECURRENT, false);
                aPopup->EnableItem(SID_BASICIDE_RENAMECURRENT, false);
                aPopup->RemoveDisabledEntries();
            }
             if ( aDocument.isInVBAMode() )
            {
                // disable to delete or remove object modules in IDE
                if (BasicManager* pBasMgr = aDocument.getBasicManager())
                {
                    if (StarBASIC* pBasic = pBasMgr->GetLib(aOULibName))
                    {
                        Shell::WindowTable& aWindowTable = pShell->GetWindowTable();
                        Shell::WindowTableIt it = aWindowTable.find( GetCurPageId() );
                        if (it != aWindowTable.end() && dynamic_cast<ModulWindow*>(it->second.get()))
                        {
                            SbModule* pActiveModule = pBasic->FindModule( it->second->GetName() );
                            if( pActiveModule && ( pActiveModule->GetModuleType() == script::ModuleType::DOCUMENT ) )
                            {
                                aPopup->EnableItem(SID_BASICIDE_DELETECURRENT, false);
                                aPopup->EnableItem(SID_BASICIDE_RENAMECURRENT, false);
                            }
                        }
                    }
                }
            }
        }
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute(aPopup->Execute(this, aPos));
    }
}

TabBarAllowRenamingReturnCode TabBar::AllowRenaming()
{
    bool const bValid = IsValidSbxName(GetEditText());

    if ( !bValid )
        ScopedVclPtrInstance<MessageDialog>(this, IDEResId(RID_STR_BADSBXNAME))->Execute();

    return bValid ? TABBAR_RENAMING_YES : TABBAR_RENAMING_NO;
}


void TabBar::EndRenaming()
{
    if ( !IsEditModeCanceled() )
    {
        SfxUInt16Item aID( SID_BASICIDE_ARG_TABID, GetEditPageId() );
        SfxStringItem aNewName( SID_BASICIDE_ARG_MODULENAME, GetEditText() );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->ExecuteList( SID_BASICIDE_NAMECHANGEDONTAB,
                      SfxCallMode::SYNCHRON, { &aID, &aNewName });
    }
}


namespace
{

// helper class for sorting TabBar
struct TabBarSortHelper
{
    sal_uInt16      nPageId;
    OUString        aPageText;

    bool operator < (TabBarSortHelper const& rComp) const
    {
        return aPageText.compareToIgnoreAsciiCase(rComp.aPageText) < 0;
    }
};

} // namespace

void TabBar::Sort()
{
    if (Shell* pShell = GetShell())
    {
        Shell::WindowTable& aWindowTable = pShell->GetWindowTable();
        TabBarSortHelper aTabBarSortHelper;
        std::vector<TabBarSortHelper> aModuleList;
        std::vector<TabBarSortHelper> aDialogList;
        sal_uInt16 nPageCount = GetPageCount();
        sal_uInt16 i;

        // create module and dialog lists for sorting
        for ( i = 0; i < nPageCount; i++)
        {
            sal_uInt16 nId = GetPageId( i );
            aTabBarSortHelper.nPageId = nId;
            aTabBarSortHelper.aPageText = GetPageText( nId );
            BaseWindow* pWin = aWindowTable[ nId ];

            if (dynamic_cast<ModulWindow*>(pWin))
            {
                aModuleList.push_back( aTabBarSortHelper );
            }
            else if (dynamic_cast<DialogWindow*>(pWin))
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

void CutLines( OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, bool bEraseTrailingEmptyLines )
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

    SAL_WARN_IF( nStartPos == -1, "basctl.basicide", "CutLines: Startzeile nicht gefunden!" );

    if ( nStartPos == -1 )
        return;

    sal_Int32 nEndPos = nStartPos;

    for ( sal_Int32 i = 0; i < nLines; i++ )
        nEndPos = searchEOL( rStr, nEndPos+1 );

    if ( nEndPos == -1 ) // might happen at the last line
        nEndPos = rStr.getLength();
    else
        nEndPos++;

    OUString aEndStr = rStr.copy( nEndPos );
    rStr = rStr.copy( 0, nStartPos );
    rStr += aEndStr;

    if ( bEraseTrailingEmptyLines )
    {
        sal_Int32 n = nStartPos;
        sal_Int32 nLen = rStr.getLength();
        while ( ( n < nLen ) && ( rStr[ n ] == LINE_SEP ||
                                  rStr[ n ] == LINE_SEP_CR ) )
        {
            n++;
        }

        if ( n > nStartPos )
        {
            aEndStr = rStr.copy( n );
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
    rStream.ReadChar( c );
    while ( !rStream.IsEof() )
    {
        if ( c == '\n' )
            nLFs++;
        else if ( c == '\r' )
            nCRs++;
        rStream.ReadChar( c );
    }

    rStream.Seek( 0 );
    if ( nLFs > nCRs )
        return nLFs;
    return nCRs;
}


// LibInfo


LibInfo::LibInfo ()
{ }

LibInfo::~LibInfo ()
{ }

void LibInfo::InsertInfo (
    ScriptDocument const& rDocument,
    OUString const& rLibName,
    OUString const& rCurrentName,
    ItemType eCurrentType
)
{
    Key aKey(rDocument, rLibName);
    m_aMap.erase(aKey);
    m_aMap.insert(Map::value_type(aKey, Item(rCurrentName, eCurrentType)));
}

void LibInfo::RemoveInfoFor (ScriptDocument const& rDocument)
{
    Map::iterator it;
    for (it = m_aMap.begin(); it != m_aMap.end(); ++it)
        if (it->first.GetDocument() == rDocument)
            break;
    if (it != m_aMap.end())
        m_aMap.erase(it);
}

LibInfo::Item const* LibInfo::GetInfo (
    ScriptDocument const& rDocument, OUString const& rLibName
)
{
    Map::iterator it = m_aMap.find(Key(rDocument, rLibName));
    return it != m_aMap.end() ? &it->second : nullptr;
}

LibInfo::Key::Key (ScriptDocument const& rDocument, OUString const& rLibName) :
    m_aDocument(rDocument), m_aLibName(rLibName)
{ }

LibInfo::Key::~Key ()
{ }

bool LibInfo::Key::operator == (Key const& rKey) const
{
    return m_aDocument == rKey.m_aDocument && m_aLibName == rKey.m_aLibName;
}

size_t LibInfo::Key::Hash::operator () (Key const& rKey) const
{
    return rKey.m_aDocument.hashCode() + rKey.m_aLibName.hashCode();
}

LibInfo::Item::Item (
    OUString const& rCurrentName,
    ItemType eCurrentType
) :
    m_aCurrentName(rCurrentName),
    m_eCurrentType(eCurrentType)
{ }

LibInfo::Item::~Item ()
{ }

bool QueryDel( const OUString& rName, const ResId& rId, vcl::Window* pParent )
{
    OUString aQuery(rId.toString());
    OUStringBuffer aNameBuf( rName );
    aNameBuf.append('\'');
    aNameBuf.insert(0, '\'');
    aQuery = aQuery.replaceAll("XX", aNameBuf.makeStringAndClear());
    ScopedVclPtrInstance< MessageDialog > aQueryBox(pParent, aQuery, VclMessageType::Question, VCL_BUTTONS_YES_NO);
    return ( aQueryBox->Execute() == RET_YES );
}

bool QueryDelMacro( const OUString& rName, vcl::Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMACRO ), pParent );
}

bool QueryReplaceMacro( const OUString& rName, vcl::Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYREPLACEMACRO ), pParent );
}

bool QueryDelDialog( const OUString& rName, vcl::Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELDIALOG ), pParent );
}

bool QueryDelLib( const OUString& rName, bool bRef, vcl::Window* pParent )
{
    return QueryDel( rName, IDEResId( bRef ? RID_STR_QUERYDELLIBREF : RID_STR_QUERYDELLIB ), pParent );
}

bool QueryDelModule( const OUString& rName, vcl::Window* pParent )
{
    return QueryDel( rName, IDEResId( RID_STR_QUERYDELMODULE ), pParent );
}

bool QueryPassword( const Reference< script::XLibraryContainer >& xLibContainer, const OUString& rLibName, OUString& rPassword, bool bRepeat, bool bNewTitle )
{
    bool bOK = false;
    sal_uInt16 nRet = 0;

    do
    {
        // password dialog
        ScopedVclPtrInstance< SfxPasswordDialog > aDlg(Application::GetDefDialogParent());
        aDlg->SetMinLen( 1 );

        // set new title
        if ( bNewTitle )
        {
            OUString aTitle(IDE_RESSTR(RID_STR_ENTERPASSWORD));
            aTitle = aTitle.replaceAll("XX", rLibName);
            aDlg->SetText( aTitle );
        }

        // execute dialog
        nRet = aDlg->Execute();

        // verify password
        if ( nRet == RET_OK )
        {
            if ( xLibContainer.is() && xLibContainer->hasByName( rLibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( rLibName ) && !xPasswd->isLibraryPasswordVerified( rLibName ) )
                {
                    rPassword = aDlg->GetPassword();
                    //                    OUString aOUPassword( rPassword );
                    bOK = xPasswd->verifyLibraryPassword( rLibName, rPassword );

                    if ( !bOK )
                    {
                        ScopedVclPtrInstance< MessageDialog > aErrorBox(Application::GetDefDialogParent(), IDE_RESSTR(RID_STR_WRONGPASSWORD));
                        aErrorBox->Execute();
                    }
                }
            }
        }
    }
    while ( bRepeat && !bOK && nRet == RET_OK );

    return bOK;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
