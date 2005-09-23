/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppSwapWindow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:17:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef DBAUI_APPSWAPWINDOW_HXX
#include "AppSwapWindow.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef DBAUI_IAPPELEMENTNOTIFICATION_HXX
#include "IAppElementNotification.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif

#include <memory>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

//==================================================================
// class OApplicationSwapWindow
DBG_NAME(OApplicationSwapWindow)
//==================================================================
OApplicationSwapWindow::OApplicationSwapWindow(Window* _pParent,OAppBorderWindow* _pBorderWindow) : Control(_pParent,WB_DIALOGCONTROL )
    ,m_aIconControl(this)
    ,m_pBorderWin(_pBorderWindow)
    ,m_eLastType(E_NONE)
{
    DBG_CTOR(OApplicationSwapWindow,NULL);

    ImplInitSettings( sal_True, sal_True, sal_True );

    m_aIconControl.SetClickHdl(LINK(this, OApplicationSwapWindow, OnContainerSelectHdl));
    m_aIconControl.setControlActionListener(_pBorderWindow->getView()->getActionListener());
    m_aIconControl.SetHelpId(HID_APP_SWAP_ICONCONTROL);
    m_aIconControl.Show();
    m_aIconControl.Enable(TRUE);
}
// -----------------------------------------------------------------------------
OApplicationSwapWindow::~OApplicationSwapWindow()
{

    DBG_DTOR(OApplicationSwapWindow,NULL);
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::Resize()
{
    Size aFLSize = LogicToPixel( Size( 8, 0 ), MAP_APPFONT );
    long nX = 0;
    if ( m_aIconControl.GetEntryCount() != 0 )
        nX = m_aIconControl.GetBoundingBox( m_aIconControl.GetEntry(0) ).GetWidth() + aFLSize.Width();

    Size aOutputSize = GetOutputSize();

    m_aIconControl.SetPosSizePixel( Point(static_cast<long>((aOutputSize.Width() - nX)*0.5), 0)  ,Size(nX,aOutputSize.Height()));
    m_aIconControl.ArrangeIcons();
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetButtonTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetWindowColor() ) );
}
// -----------------------------------------------------------------------
void OApplicationSwapWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::clearSelection()
{
    m_aIconControl.SetNoSelection();
    ULONG nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl.GetSelectedEntry(nPos);
    if ( pEntry )
        m_aIconControl.InvalidateEntry(pEntry);
    m_aIconControl.GetClickHdl().Call(&m_aIconControl);
}

// -----------------------------------------------------------------------------
void OApplicationSwapWindow::createIconAutoMnemonics()
{
    // we need to share our "mnemonic space" with the menu of the window we live in
    MnemonicGenerator aMnemonicGenerator;
    SystemWindow* pSystemWindow = GetSystemWindow();
    MenuBar* pMenu = pSystemWindow ? pSystemWindow->GetMenuBar() : NULL;
    if ( pMenu )
    {
        USHORT nMenuItems = pMenu->GetItemCount();
        for ( USHORT i = 0; i < nMenuItems; ++i )
            aMnemonicGenerator.RegisterMnemonic( pMenu->GetItemText( pMenu->GetItemId( i ) ) );
    }

    m_aIconControl.CreateAutoMnemonics( aMnemonicGenerator );
}

// -----------------------------------------------------------------------------
bool OApplicationSwapWindow::interceptKeyInput( const KeyEvent& _rEvent )
{
    const KeyCode& rKeyCode = _rEvent.GetKeyCode();
    if ( rKeyCode.GetModifier() == KEY_MOD2 )
        return m_aIconControl.DoKeyInput( _rEvent );

    // not intercepted
    return false;
}

// -----------------------------------------------------------------------------
ElementType OApplicationSwapWindow::getElementType() const
{
    ULONG nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl.GetSelectedEntry(nPos);
    return ( pEntry ) ? *static_cast<ElementType*>(pEntry->GetUserData()) : E_NONE;
}
// -----------------------------------------------------------------------------
IMPL_LINK(OApplicationSwapWindow, OnContainerSelectHdl, SvtIconChoiceCtrl*, _pControl)
{
    ULONG nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = _pControl->GetSelectedEntry( nPos );
    ElementType eType = E_NONE;
    if ( pEntry )
        eType = *static_cast<ElementType*>(pEntry->GetUserData());

    if ( m_eLastType != eType && eType != E_NONE )
    {
        if ( m_pBorderWin->getView()->getElementNotification()->onContainerSelect(eType) )
            m_eLastType = eType;
        else
        {
            PostUserEvent(LINK(this, OApplicationSwapWindow, ChangeToLastSelected));
        }
    }

    return 1L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OApplicationSwapWindow, ChangeToLastSelected, void*, EMPTYARG)
{
    changeContainer(m_eLastType);
    return 0L;
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::changeContainer(ElementType _eType)
{
    ULONG nCount = m_aIconControl.GetEntryCount();
    SvxIconChoiceCtrlEntry* pEntry = NULL;
    for (ULONG i=0; i < nCount; ++i)
    {
        pEntry = m_aIconControl.GetEntry(i);
        if ( pEntry && *static_cast<ElementType*>(pEntry->GetUserData()) == _eType )
            break;
        pEntry = NULL;
    }
    if ( pEntry )
    {
        m_aIconControl.SetCursor(pEntry);
        OnContainerSelectHdl(&m_aIconControl);
    }
    else
        m_eLastType = E_NONE;
}
