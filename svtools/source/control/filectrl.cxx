/*************************************************************************
 *
 *  $RCSfile: filectrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#define _SV_FIELCTRL_CXX

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_FILEDLG_HXX
#include <vcl/filedlg.hxx>
#endif

#include <filedlg.hxx>
#include <svtdata.hxx>

#include <filectrl.hxx>
#include <filectrl.hrc>

#pragma hdrstop

// =======================================================================

FileControl::FileControl( Window* pParent, WinBits nStyle, FileControlMode nFlags ) :
    Window( pParent, nStyle|WB_DIALOGCONTROL ),
    maEdit( this, (nStyle&(~WB_BORDER))|WB_NOTABSTOP ),
    maButton( this, nStyle&(~WB_BORDER)|WB_NOLIGHTBORDER|WB_NOPOINTERFOCUS|WB_NOTABSTOP ),
    maButtonText( SvtResId( STR_FILECTRL_BUTTONTEXT ) ),
    mnFlags( nFlags ),
    mnInternalFlags( FILECTRL_ORIGINALBUTTONTEXT ),
    mpVclDlg( 0 ),
    mpFDlg( 0 )
{
    maButton.SetClickHdl( LINK( this, FileControl, ButtonHdl ) );
    mbOpenDlg = TRUE;

    maButton.Show();
    maEdit.Show();

    SetCompoundControl( TRUE );

    SetStyle( ImplInitStyle( GetStyle() ) );
}

// -----------------------------------------------------------------------

WinBits FileControl::ImplInitStyle( WinBits nStyle )
{
    if ( !( nStyle & WB_NOTABSTOP ) )
    {
        maEdit.SetStyle( (maEdit.GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
        maButton.SetStyle( (maButton.GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
    }
    else
    {
        maEdit.SetStyle( (maEdit.GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
        maButton.SetStyle( (maButton.GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
    }

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_NOBORDER ) )
        nStyle |= WB_BORDER;

    nStyle &= ~WB_TABSTOP;

    return nStyle;
}

// -----------------------------------------------------------------------

FileControl::~FileControl()
{
}

// -----------------------------------------------------------------------

void FileControl::SetText( const XubString& rStr )
{
    maEdit.SetText( rStr );
    if ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN )
        Resize();
}

// -----------------------------------------------------------------------

XubString FileControl::GetText() const
{
    return maEdit.GetText();
}

// -----------------------------------------------------------------------

void FileControl::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_ENABLE )
    {
        maEdit.Enable( IsEnabled() );
        maButton.Enable( IsEnabled() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        GetEdit().SetZoom( GetZoom() );
        GetButton().SetZoom( GetZoom() );
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        GetEdit().SetControlFont( GetControlFont() );
        // Fuer den Button nur die Hoehe uebernehmen, weil in
        // HTML immer Courier eingestellt wird.
        Font aFont = GetButton().GetControlFont();
        aFont.SetSize( GetControlFont().GetSize() );
        GetButton().SetControlFont( aFont );
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        GetEdit().SetControlForeground( GetControlForeground() );
        GetButton().SetControlForeground( GetControlForeground() );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        GetEdit().SetControlBackground( GetControlBackground() );
        GetButton().SetControlBackground( GetControlBackground() );
    }
    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void FileControl::Resize()
{
    static long ButtonBorder = 10;

    if( mnInternalFlags & FILECTRL_INRESIZE )
        return;
    mnInternalFlags |= FILECTRL_INRESIZE;//InResize = TRUE

    Size aOutSz = GetOutputSizePixel();
    long nButtonTextWidth = maButton.GetTextWidth( maButtonText );
    if ( (mnInternalFlags & FILECTRL_ORIGINALBUTTONTEXT == 0) ||
        nButtonTextWidth < aOutSz.Width()/3 &&
        ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN
        ? ( maEdit.GetTextWidth( maEdit.GetText() )
            <= aOutSz.Width() - nButtonTextWidth - ButtonBorder )
        : TRUE )
       )
    {
        maButton.SetText( maButtonText );
    }
    else
    {
        XubString aSmallText( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
        maButton.SetText( aSmallText );
        nButtonTextWidth = maButton.GetTextWidth( aSmallText );
    }

    long nButtonWidth = nButtonTextWidth+ButtonBorder;
    maEdit.SetPosSizePixel( 0, 0, aOutSz.Width()-nButtonWidth, aOutSz.Height() );
    maButton.SetPosSizePixel( aOutSz.Width()-nButtonWidth, 0, nButtonWidth, aOutSz.Height() );

    mnInternalFlags &= ~FILECTRL_INRESIZE; //InResize = FALSE
}

// -----------------------------------------------------------------------

IMPL_LINK( FileControl, ButtonHdl, PushButton*, EMPTYARG )
{
    XubString aNewText;
    mpVclDlg = GetpApp()->CreateFileDialog( this, mbOpenDlg ? WB_OPEN : WB_SAVEAS );

    if ( mpVclDlg )
    {
        mpVclDlg->SetPath( maEdit.GetText() );

        maDialogCreatedHdl.Call( this );

        if ( mpVclDlg->Execute()  )
            aNewText = mpVclDlg->GetPath();
        DELETEZ( mpVclDlg );
    }
    else
    {
        mpFDlg = new FileDialog( this, mbOpenDlg ? WB_OPEN : WB_SAVEAS );
        mpFDlg->SetPath( maEdit.GetText() );

        maDialogCreatedHdl.Call( this );

        if ( mpFDlg->Execute()  )
            aNewText = mpFDlg->GetPath();
        DELETEZ( mpFDlg );
    }

    if ( aNewText.Len() )
    {
        INetURLObject aObj( aNewText );
        if ( aObj.GetProtocol() == INET_PROT_FILE )
            aNewText = aObj.PathToFileName();
        SetText( aNewText );
        maEdit.GetModifyHdl().Call( &maEdit );
    }

    return 0;
}

// -----------------------------------------------------------------------

void FileControl::GetFocus()
{
    maEdit.GrabFocus();
}

// -----------------------------------------------------------------------

void FileControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    WinBits nOldEditStyle = GetEdit().GetStyle();
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle|WB_BORDER );
    GetEdit().Draw( pDev, rPos, rSize, nFlags );
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle );
}

// -----------------------------------------------------------------------

void FileControl::SetButtonText( const XubString& rStr )
{
    mnInternalFlags &= ~FILECTRL_ORIGINALBUTTONTEXT;
    maButtonText = rStr;
    Resize();
}

// -----------------------------------------------------------------------

void FileControl::ResetButtonText()
{
    mnInternalFlags |= FILECTRL_ORIGINALBUTTONTEXT;
    maButtonText = XubString( SvtResId( STR_FILECTRL_BUTTONTEXT ) );
    Resize();
}


