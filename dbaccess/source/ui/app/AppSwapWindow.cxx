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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "AppSwapWindow.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "dbu_app.hrc"
#include "AppView.hxx"
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/mnemonic.hxx>
#include "IApplicationController.hxx"

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
OApplicationSwapWindow::OApplicationSwapWindow( Window* _pParent, OAppBorderWindow& _rBorderWindow )
    :Window(_pParent,WB_DIALOGCONTROL )
    ,m_aIconControl(this)
    ,m_eLastType(E_NONE)
    ,m_rBorderWin( _rBorderWindow )
{
    DBG_CTOR(OApplicationSwapWindow,NULL);

    ImplInitSettings( sal_True, sal_True, sal_True );

    m_aIconControl.SetClickHdl(LINK(this, OApplicationSwapWindow, OnContainerSelectHdl));
    m_aIconControl.setControlActionListener( &m_rBorderWin.getView()->getAppController() );
    m_aIconControl.SetHelpId(HID_APP_SWAP_ICONCONTROL);
    m_aIconControl.Show();
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
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );
}
// -----------------------------------------------------------------------
void OApplicationSwapWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::clearSelection()
{
    m_aIconControl.SetNoSelection();
    sal_uLong nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl.GetSelectedEntry(nPos);
    if ( pEntry )
        m_aIconControl.InvalidateEntry(pEntry);
    m_aIconControl.GetClickHdl().Call(&m_aIconControl);
}

// -----------------------------------------------------------------------------
void OApplicationSwapWindow::createIconAutoMnemonics( MnemonicGenerator& _rMnemonics )
{
    m_aIconControl.CreateAutoMnemonics( _rMnemonics );
}

// -----------------------------------------------------------------------------
bool OApplicationSwapWindow::interceptKeyInput( const KeyEvent& _rEvent )
{
    const KeyCode& rKeyCode = _rEvent.GetKeyCode();
    if ( rKeyCode.GetModifier() == KEY_MOD2 )
        return m_aIconControl.DoKeyInput( _rEvent );

    // not handled
    return false;
}

// -----------------------------------------------------------------------------
ElementType OApplicationSwapWindow::getElementType() const
{
    sal_uLong nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl.GetSelectedEntry(nPos);
    return ( pEntry ) ? *static_cast<ElementType*>(pEntry->GetUserData()) : E_NONE;
}

// -----------------------------------------------------------------------------
bool OApplicationSwapWindow::onContainerSelected( ElementType _eType )
{
    if ( m_eLastType == _eType )
        return true;

    if ( m_rBorderWin.getView()->getAppController().onContainerSelect( _eType ) )
    {
        if ( _eType != E_NONE )
            m_eLastType = _eType;
        return true;
    }

    PostUserEvent( LINK( this, OApplicationSwapWindow, ChangeToLastSelected ) );
    return false;
}

// -----------------------------------------------------------------------------
IMPL_LINK(OApplicationSwapWindow, OnContainerSelectHdl, SvtIconChoiceCtrl*, _pControl)
{
    sal_uLong nPos = 0;
    SvxIconChoiceCtrlEntry* pEntry = _pControl->GetSelectedEntry( nPos );
    ElementType eType = E_NONE;
    if ( pEntry )
    {
        eType = *static_cast<ElementType*>(pEntry->GetUserData());
        onContainerSelected( eType ); // i87582
    }

    return 1L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OApplicationSwapWindow, ChangeToLastSelected, void*, EMPTYARG)
{
    selectContainer(m_eLastType);
    return 0L;
}
// -----------------------------------------------------------------------------
void OApplicationSwapWindow::selectContainer(ElementType _eType)
{
    sal_uLong nCount = m_aIconControl.GetEntryCount();
    SvxIconChoiceCtrlEntry* pEntry = NULL;
    for (sal_uLong i=0; i < nCount; ++i)
    {
        pEntry = m_aIconControl.GetEntry(i);
        if ( pEntry && *static_cast<ElementType*>(pEntry->GetUserData()) == _eType )
            break;
        pEntry = NULL;
    }

    if ( pEntry )
        m_aIconControl.SetCursor(pEntry); // this call also initiates a onContainerSelected call
    else
        onContainerSelected( _eType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
