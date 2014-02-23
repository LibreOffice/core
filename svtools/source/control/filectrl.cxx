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

#include <svtools/filectrl.hxx>

#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/file.h>
#include <svtools/svtresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/stdtext.hxx>
#include <filectrl.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;



FileControl::FileControl( Window* pParent, WinBits nStyle, FileControlMode nFlags ) :
    Window( pParent, nStyle|WB_DIALOGCONTROL ),
    maEdit( this, (nStyle&(~WB_BORDER))|WB_NOTABSTOP ),
    maButton( this, (nStyle&(~WB_BORDER))|WB_NOLIGHTBORDER|WB_NOPOINTERFOCUS|WB_NOTABSTOP ),
    maButtonText( SVT_RESSTR(STR_FILECTRL_BUTTONTEXT) ),
    mnFlags( nFlags ),
    mnInternalFlags( FILECTRL_ORIGINALBUTTONTEXT )
{
    maButton.SetClickHdl( LINK( this, FileControl, ButtonHdl ) );
    mbOpenDlg = sal_True;

    maButton.Show();
    maEdit.Show();

    SetCompoundControl( true );

    SetStyle( ImplInitStyle( GetStyle() ) );
}



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

    const WinBits nAlignmentStyle = ( WB_TOP | WB_VCENTER | WB_BOTTOM );
    maEdit.SetStyle( ( maEdit.GetStyle() & ~nAlignmentStyle ) | ( nStyle & nAlignmentStyle ) );

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_NOBORDER ) )
        nStyle |= WB_BORDER;

    nStyle &= ~WB_TABSTOP;

    return nStyle;
}



FileControl::~FileControl()
{
}



void FileControl::SetText( const OUString& rStr )
{
    maEdit.SetText( rStr );
    if ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN )
        Resize();
}



OUString FileControl::GetText() const
{
    return maEdit.GetText();
}



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
        // Only use height of the button, as in HTML
        // always Courier is used
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



void FileControl::Resize()
{
    static long ButtonBorder = 10;

    if( mnInternalFlags & FILECTRL_INRESIZE )
        return;
    mnInternalFlags |= FILECTRL_INRESIZE;//InResize = sal_True

    Size aOutSz = GetOutputSizePixel();
    long nButtonTextWidth = maButton.GetTextWidth( maButtonText );
    if ( ((mnInternalFlags & FILECTRL_ORIGINALBUTTONTEXT) == 0) ||
        ( nButtonTextWidth < aOutSz.Width()/3 &&
        ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN
        ? ( maEdit.GetTextWidth( maEdit.GetText() )
            <= aOutSz.Width() - nButtonTextWidth - ButtonBorder )
        : sal_True ) )
       )
    {
        maButton.SetText( maButtonText );
    }
    else
    {
        OUString aSmallText( "..." );
        maButton.SetText( aSmallText );
        nButtonTextWidth = maButton.GetTextWidth( aSmallText );
    }

    long nButtonWidth = nButtonTextWidth+ButtonBorder;
    maEdit.setPosSizePixel( 0, 0, aOutSz.Width()-nButtonWidth, aOutSz.Height() );
    maButton.setPosSizePixel( aOutSz.Width()-nButtonWidth, 0, nButtonWidth, aOutSz.Height() );

    mnInternalFlags &= ~FILECTRL_INRESIZE; //InResize = sal_False
}



IMPL_LINK_NOARG(FileControl, ButtonHdl)
{
    ImplBrowseFile( );

    return 0;
}



void FileControl::GetFocus()
{
    maEdit.GrabFocus();
}



void FileControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    WinBits nOldEditStyle = GetEdit().GetStyle();
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle|WB_BORDER );
    GetEdit().Draw( pDev, rPos, rSize, nFlags );
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle );
}

void FileControl::ImplBrowseFile( )
{
    try
    {
        Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
        Reference < dialogs::XFilePicker3 > xFilePicker = dialogs::FilePicker::createWithMode( xContext, dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        // transform the system notation text into a file URL
        OUString sSystemNotation = GetText(), sFileURL;
        oslFileError nError = osl_getFileURLFromSystemPath( sSystemNotation.pData, &sFileURL.pData );
        if ( nError == osl_File_E_INVAL )
            sFileURL = GetText();   // #97709# Maybe URL is already a file URL...

        //#90430# Check if URL is really a file URL
        OUString aTmp;
        if ( osl_getSystemPathFromFileURL( sFileURL.pData, &aTmp.pData ) == osl_File_E_None )
        {
            // initially set this directory
            xFilePicker->setDisplayDirectory( sFileURL );
        }

        if ( xFilePicker->execute() )
        {
            Sequence < OUString > aPathSeq = xFilePicker->getFiles();

            if ( aPathSeq.getLength() )
            {
                OUString aNewText = aPathSeq[0];
                INetURLObject aObj( aNewText );
                if ( aObj.GetProtocol() == INET_PROT_FILE )
                    aNewText = aObj.PathToFileName();
                SetText( aNewText );
                maEdit.GetModifyHdl().Call( &maEdit );
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FileControl::ImplBrowseFile: caught an exception while executing the file picker!" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
