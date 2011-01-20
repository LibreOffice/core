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
#include "precompiled_cui.hxx"
#include "commonlingui.hxx"

#include <cuires.hrc>
#include <dialmgr.hxx>
#include <vcl/decoview.hxx>

#include "hangulhanjadlg.hrc"

//=============================================================================
// SvxClickInfoCtr
//=============================================================================

//-----------------------------------------------------------------------------
SvxClickInfoCtr::SvxClickInfoCtr( Window* pParent, const ResId& rResId ) :
        Control( pParent, rResId ),
        aFixedInfo( this)
{
    aFixedInfo.SetSizePixel(GetOutputSizePixel());
    aFixedInfo.Show();
}

//-----------------------------------------------------------------------------
void SvxClickInfoCtr::MouseButtonDown( const MouseEvent& )
{
    aActivateLink.Call(this);
}

//-----------------------------------------------------------------------------
long SvxClickInfoCtr::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==EVENT_GETFOCUS || rNEvt.GetType()==EVENT_MOUSEBUTTONDOWN)
    {
        aActivateLink.Call(this);
    }

    return Control::PreNotify(rNEvt);
}

//-----------------------------------------------------------------------------
void SvxClickInfoCtr::SetText( const XubString& rStr )
{
    aFixedInfo.SetText(rStr );
}

//-----------------------------------------------------------------------------
XubString SvxClickInfoCtr::GetText() const
{
    return aFixedInfo.GetText();
}

//-----------------------------------------------------------------------------
__EXPORT SvxClickInfoCtr::~SvxClickInfoCtr()
{
}

//=============================================================================
// SvxCommonLinguisticControl
//=============================================================================
//-----------------------------------------------------------------------------
SvxCommonLinguisticControl::SvxCommonLinguisticControl( ModalDialog* _pParent )
    :Window( _pParent, CUI_RES( RID_SVX_WND_COMMON_LINGU ) )
    ,aWordText      ( this, CUI_RES( FT_WORD ) )
    ,aAktWord       ( this, CUI_RES( FT_AKTWORD ) )
    ,aNewWord       ( this, CUI_RES( FT_NEWWORD ) )
    ,aNewWordED     ( this, CUI_RES( ED_NEWWORD ) )
    ,aSuggestionFT  ( this, CUI_RES( FT_SUGGESTION ) )
    ,aIgnoreBtn     ( this, CUI_RES( BTN_IGNORE ) )
    ,aIgnoreAllBtn  ( this, CUI_RES( BTN_IGNOREALL ) )
    ,aChangeBtn     ( this, CUI_RES( BTN_CHANGE ) )
    ,aChangeAllBtn  ( this, CUI_RES( BTN_CHANGEALL ) )
    ,aOptionsBtn    ( this, CUI_RES( BTN_OPTIONS ) )
    ,aStatusText    ( this, CUI_RES( FT_STATUS ) )
    ,aHelpBtn       ( this, CUI_RES( BTN_SPL_HELP ) )
    ,aCancelBtn     ( this, CUI_RES( BTN_SPL_CANCEL ) )
    ,aAuditBox      ( this, CUI_RES( GB_AUDIT ) )
{
    FreeResource();

#ifdef FS_PRIV_DEBUG
    SetType( WINDOW_TABPAGE );
#endif

    aAktWord.SetAccessibleName(aWordText.GetText());
    SetPosSizePixel( Point( 0, 0 ), _pParent->GetOutputSizePixel() );
    Show();
}

// -----------------------------------------------------------------------
PushButton* SvxCommonLinguisticControl::implGetButton( ButtonType _eType  ) const
{
    const PushButton* pButton = NULL;
    switch ( _eType )
    {
        case eClose: pButton = &aCancelBtn; break;
        case eIgnore: pButton = &aIgnoreBtn; break;
        case eIgnoreAll: pButton = &aIgnoreAllBtn; break;
        case eChange: pButton = &aChangeBtn; break;
        case eChangeAll: pButton = &aChangeAllBtn; break;
        case eOptions: pButton = &aOptionsBtn; break;
    }
    return const_cast< PushButton* >( pButton );
}

// -----------------------------------------------------------------------
void SvxCommonLinguisticControl::SetButtonHandler( ButtonType _eType, const Link& _rHandler )
{
    Button* pButton = GetButton( _eType );
    if ( pButton )
        pButton->SetClickHdl( _rHandler );
}

// -----------------------------------------------------------------------
void SvxCommonLinguisticControl::EnableButton( ButtonType _eType, sal_Bool _bEnable )
{
    Button* pButton = GetButton( _eType );
    if ( pButton )
        pButton->Enable( _bEnable );
}

// -----------------------------------------------------------------------
void SvxCommonLinguisticControl::InsertControlGroup( Window& _rFirstGroupWindow, Window& _rLastGroupWindow, ControlGroup _eInsertAfter )
{
    Window* pInsertAfter = NULL;    // will be the last window of our own "undividable" group, after which we insert the foreign group
    switch ( _eInsertAfter )
    {
        case eLeftRightWords    : pInsertAfter = &aNewWordED; break;
        case eSuggestionLabel   : pInsertAfter = &aSuggestionFT; break;
        case eActionButtons     : pInsertAfter = &aChangeAllBtn; break;
        case eDialogButtons     : pInsertAfter = &aCancelBtn; break;
    }

    // now loop through the remaining windows of the foreign group
    Window* pInsertBehind = pInsertAfter;
    Window* pInsert = &_rFirstGroupWindow;

    // some strange thing below: asking a window for WINDOW_NEXT or WINDOW_PREV does not take into
    // account the border window, _but_ SetZOrder _does_!. Thus, when advancing through a chain
    // of windows, we need to work with the border windows (WINDOW_BORDER), instead of simply
    // asking for WINDOW_NEXT.

    Window* pLoopEnd = _rLastGroupWindow.GetWindow( WINDOW_BORDER );
    while ( pInsert && ( pInsertBehind != pLoopEnd ) )
    {
        // we'll destroy the NEXT relation immediately, so remember the next window
        DBG_ASSERT( pInsert->GetWindow( WINDOW_BORDER ), "SvxCommonLinguisticControl::InsertControlGroup: border window is expected to be non NULL!" );
        Window* pNextInsert = pInsert->GetWindow( WINDOW_BORDER )->GetWindow( WINDOW_NEXT );
        // knit
        pInsert->SetZOrder( pInsertBehind, WINDOW_ZORDER_BEHIND );
        // advance
        pInsertBehind = pInsert;
        pInsert = pNextInsert;
    }
    DBG_ASSERT( pInsertBehind == pLoopEnd, "SvxCommonLinguisticControl::InsertControlGroup: controls do not form a group!" );
        // if we did not reach pLoopEnd, then we did not reach _rLastGroupWindow in the loop, then
        // (FirstWindow, LastWindow) was no valid control group
}

//-----------------------------------------------------------------------------
void SvxCommonLinguisticControl::Paint( const Rectangle& rRect )
{
    Window::Paint(rRect );

    DecorationView aDecoView( this );

    Rectangle aRect( aAuditBox.GetPosPixel(), aAuditBox.GetSizePixel() );
    aDecoView.DrawButton( aRect, BUTTON_DRAW_NOFILL );
}

//-----------------------------------------------------------------------------
void SvxCommonLinguisticControl::Enlarge( sal_Int32 _nX, sal_Int32 _nY )
{
    Size aSize;
    Point aPos;

    // the controls which need to be resized
    {
        Window* pResize[] =
        {
            this, &aAuditBox, &aStatusText
        };
        for ( sal_uInt32 i = 0; i < sizeof( pResize ) / sizeof( pResize[0] ); ++i )
        {
            aSize = pResize[i]->GetSizePixel( );
            pResize[i]->SetSizePixel( Size( aSize.Width() + _nX, aSize.Height() + _nY ) );
        }
    }

    // the controls which stick to the bottom of the window
    {
        Window* pMoveDown[] =
        {
            &aStatusText, &aHelpBtn, &aCancelBtn
        };
        for ( sal_uInt32 i = 0; i < sizeof( pMoveDown ) / sizeof( pMoveDown[0] ); ++i )
        {
            aPos = pMoveDown[i]->GetPosPixel();
            aPos.Y() += _nY;
            pMoveDown[i]->SetPosPixel( aPos );
        }
    }

    // the controls which stick to the right
    {
        Window* pMoveRight[] =
        {
            &aIgnoreBtn, &aIgnoreAllBtn, &aChangeBtn, &aChangeAllBtn, &aOptionsBtn, &aHelpBtn, &aCancelBtn
        };
        for ( sal_uInt32 i = 0; i < sizeof( pMoveRight ) / sizeof( pMoveRight[0] ); ++i )
        {
            aPos = pMoveRight[i]->GetPosPixel();
            aPos.X() += _nX;
            pMoveRight[i]->SetPosPixel( aPos );
        }
    }
}
