/*************************************************************************
 *
 *  $RCSfile: commonlingui.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:41:21 $
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

#ifndef SVX_COMMON_LINGUI_HXX
#include "commonlingui.hxx"
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif

#include "spldlg.hrc"

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
void SvxClickInfoCtr::MouseButtonDown( const MouseEvent& rMEvt )
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
    :Window( _pParent, SVX_RES( RID_SVX_WND_COMMON_LINGU ) )
    ,aWordText      ( this, ResId( FT_WORD ) )
    ,aAktWord       ( this, ResId( FT_AKTWORD ) )
    ,aNewWord       ( this, ResId( FT_NEWWORD ) )
    ,aNewWordED     ( this, ResId( ED_NEWWORD ) )
    ,aSuggestionFT  ( this, ResId( FT_SUGGESTION ) )
    ,aAuditBox      ( this, ResId( GB_AUDIT ) )
    ,aIgnoreBtn     ( this, ResId( BTN_IGNORE ) )
    ,aIgnoreAllBtn  ( this, ResId( BTN_IGNOREALL ) )
    ,aChangeBtn     ( this, ResId( BTN_CHANGE ) )
    ,aChangeAllBtn  ( this, ResId( BTN_CHANGEALL ) )
    ,aStatusText    ( this, ResId( FT_STATUS ) )
    ,aCancelBtn     ( this, ResId( BTN_SPL_CANCEL ) )
    ,aHelpBtn       ( this, ResId( BTN_SPL_HELP ) )
{
    FreeResource();

#ifdef FS_PRIV_DEBUG
    SetType( WINDOW_TABPAGE );
#endif

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

// -----------------------------------------------------------------------
String SvxCommonLinguisticControl::GetNewEditWord()
{
    return aNewWordED.GetText();
}

// -----------------------------------------------------------------------
void SvxCommonLinguisticControl::SetNewEditWord( const String& _rNew )
{
    aNewWordED.SetText( _rNew );
}

//-----------------------------------------------------------------------------
void SvxCommonLinguisticControl::UpdateIgnoreHelp( )
{

    String aInfoStr( RTL_CONSTASCII_USTRINGPARAM( ": " ) );
    aInfoStr.Append( GetCurrentText() );

    String aString = GetNonMnemonicString( aIgnoreAllBtn.GetText() );
    aString.Append( aInfoStr );
    aIgnoreAllBtn.SetQuickHelpText( aString );

    aString = GetNonMnemonicString( aIgnoreBtn.GetText() );
    aString.Append( aInfoStr );
    aIgnoreBtn.SetQuickHelpText( aString );
}

//-----------------------------------------------------------------------------
void SvxCommonLinguisticControl::UpdateChangesHelp( const String& _rNewText )
{
    String aInfoStr( RTL_CONSTASCII_USTRINGPARAM( ": " ) );
    aInfoStr.Append( GetCurrentText() );
    aInfoStr.Append( String( RTL_CONSTASCII_USTRINGPARAM( " -> " ) ) );
    aInfoStr.Append( _rNewText );
        // TODO: shouldn't this be part of the resources, for proper localization?

    String aString = GetNonMnemonicString( aChangeAllBtn.GetText() );
    aString.Append( aInfoStr );
    aChangeAllBtn.SetQuickHelpText( aString );

    aString = GetNonMnemonicString( aChangeBtn.GetText() );
    aString.Append( aInfoStr );
    aChangeBtn.SetQuickHelpText( aString );
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
        for ( sal_Int32 i = 0; i < sizeof( pResize ) / sizeof( pResize[0] ); ++i )
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
        for ( sal_Int32 i = 0; i < sizeof( pMoveDown ) / sizeof( pMoveDown[0] ); ++i )
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
            &aIgnoreBtn, &aIgnoreAllBtn, &aChangeBtn, &aChangeAllBtn, &aHelpBtn, &aCancelBtn
        };
        for ( sal_Int32 i = 0; i < sizeof( pMoveRight ) / sizeof( pMoveRight[0] ); ++i )
        {
            aPos = pMoveRight[i]->GetPosPixel();
            aPos.X() += _nX;
            pMoveRight[i]->SetPosPixel( aPos );
        }
    }
}
