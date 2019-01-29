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
#include <osl/diagnose.h>
#include <svtools/svtresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/edit.hxx>
#include <vcl/stdtext.hxx>
#include <svtools/strings.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;


FileControl::FileControl( vcl::Window* pParent, WinBits nStyle ) :
    Window( pParent, nStyle|WB_DIALOGCONTROL ),
    maEdit( VclPtr<Edit>::Create(this, (nStyle&(~WB_BORDER))|WB_NOTABSTOP) ),
    maButton( VclPtr<PushButton>::Create( this, (nStyle&(~WB_BORDER))|WB_NOLIGHTBORDER|WB_NOPOINTERFOCUS|WB_NOTABSTOP ) ),
    maButtonText( SvtResId(STR_FILECTRL_BUTTONTEXT) ),
    mnInternalFlags( FileControlMode_Internal::ORIGINALBUTTONTEXT )
{
    maButton->SetClickHdl( LINK( this, FileControl, ButtonHdl ) );

    maButton->Show();
    maEdit->Show();

    SetCompoundControl( true );

    SetStyle( ImplInitStyle( GetStyle() ) );
}


WinBits FileControl::ImplInitStyle( WinBits nStyle )
{
    if ( !( nStyle & WB_NOTABSTOP ) )
    {
        maEdit->SetStyle( (maEdit->GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
        maButton->SetStyle( (maButton->GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
    }
    else
    {
        maEdit->SetStyle( (maEdit->GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
        maButton->SetStyle( (maButton->GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
    }

    const WinBits nAlignmentStyle = ( WB_TOP | WB_VCENTER | WB_BOTTOM );
    maEdit->SetStyle( ( maEdit->GetStyle() & ~nAlignmentStyle ) | ( nStyle & nAlignmentStyle ) );

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_NOBORDER ) )
        nStyle |= WB_BORDER;

    nStyle &= ~WB_TABSTOP;

    return nStyle;
}


FileControl::~FileControl()
{
    disposeOnce();
}

void FileControl::dispose()
{
    maEdit.disposeAndClear();
    maButton.disposeAndClear();
    Window::dispose();
}

void FileControl::SetText( const OUString& rStr )
{
    maEdit->SetText( rStr );
}


OUString FileControl::GetText() const
{
    return maEdit->GetText();
}


void FileControl::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::Enable )
    {
        maEdit->Enable( IsEnabled() );
        maButton->Enable( IsEnabled() );
    }
    else if ( nType == StateChangedType::Zoom )
    {
        GetEdit().SetZoom( GetZoom() );
        GetButton().SetZoom( GetZoom() );
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        GetEdit().SetControlFont( GetControlFont() );
        // Only use height of the button, as in HTML
        // always Courier is used
        vcl::Font aFont = GetButton().GetControlFont();
        aFont.SetFontSize( GetControlFont().GetFontSize() );
        GetButton().SetControlFont( aFont );
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        GetEdit().SetControlForeground( GetControlForeground() );
        GetButton().SetControlForeground( GetControlForeground() );
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        GetEdit().SetControlBackground( GetControlBackground() );
        GetButton().SetControlBackground( GetControlBackground() );
    }
    Window::StateChanged( nType );
}


void FileControl::Resize()
{
    static const long ButtonBorder = 10;

    if( mnInternalFlags & FileControlMode_Internal::INRESIZE )
        return;
    mnInternalFlags |= FileControlMode_Internal::INRESIZE;//InResize = sal_True

    Size aOutSz = GetOutputSizePixel();
    long nButtonTextWidth = maButton->GetTextWidth( maButtonText );
    if ( !(mnInternalFlags & FileControlMode_Internal::ORIGINALBUTTONTEXT) ||
         ( nButtonTextWidth < aOutSz.Width()/3 ) )
    {
        maButton->SetText( maButtonText );
    }
    else
    {
        OUString aSmallText( "..." );
        maButton->SetText( aSmallText );
        nButtonTextWidth = maButton->GetTextWidth( aSmallText );
    }

    long nButtonWidth = nButtonTextWidth+ButtonBorder;
    maEdit->setPosSizePixel( 0, 0, aOutSz.Width()-nButtonWidth, aOutSz.Height() );
    maButton->setPosSizePixel( aOutSz.Width()-nButtonWidth, 0, nButtonWidth, aOutSz.Height() );

    mnInternalFlags &= ~FileControlMode_Internal::INRESIZE; //InResize = sal_False
}


void FileControl::GetFocus()
{
    if (!maEdit || maEdit->IsDisposed())
        return;
    maEdit->GrabFocus();
}

void FileControl::SetEditModifyHdl( const Link<Edit&,void>& rLink )
{
    if (!maEdit || maEdit->IsDisposed())
        return;
    maEdit->SetModifyHdl(rLink);
}

void FileControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags )
{
    WinBits nOldEditStyle = GetEdit().GetStyle();
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle|WB_BORDER );
    GetEdit().Draw( pDev, rPos, rSize, nFlags );
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle );
}

IMPL_LINK_NOARG(FileControl, ButtonHdl, Button*, void)
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
            Sequence < OUString > aPathSeq = xFilePicker->getSelectedFiles();

            if ( aPathSeq.getLength() )
            {
                OUString aNewText = aPathSeq[0];
                INetURLObject aObj( aNewText );
                if ( aObj.GetProtocol() == INetProtocol::File )
                    aNewText = aObj.PathToFileName();
                SetText( aNewText );
                maEdit->GetModifyHdl().Call( *maEdit );
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FileControl::ImplBrowseFile: caught an exception while executing the file picker!" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
