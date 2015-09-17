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

#include <tools/lineend.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/svapp.hxx>
#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include <memory>

class SvRTLInputBox : public ModalDialog
{
    VclPtr<Edit> aEdit;
    VclPtr<OKButton> aOk;
    VclPtr<CancelButton> aCancel;
    VclPtr<FixedText> aPromptText;
    OUString aText;

    void PositionDialog( long nXTwips, long nYTwips, const Size& rDlgSize );
    void InitButtons( const Size& rDlgSize );
    void PositionEdit( const Size& rDlgSize );
    void PositionPrompt( const OUString& rPrompt, const Size& rDlgSize );
    DECL_LINK_TYPED( OkHdl, Button *, void );
    DECL_LINK_TYPED( CancelHdl, Button *, void );

public:
    SvRTLInputBox( vcl::Window* pParent, const OUString& rPrompt, const OUString& rTitle,
        const OUString& rDefault, long nXTwips = -1, long nYTwips = -1 );
    virtual ~SvRTLInputBox() { disposeOnce(); }
    virtual void dispose() SAL_OVERRIDE;
    OUString GetText() const SAL_OVERRIDE { return aText; }
};

SvRTLInputBox::SvRTLInputBox( vcl::Window* pParent, const OUString& rPrompt,
        const OUString& rTitle, const OUString& rDefault,
        long nXTwips, long nYTwips ) :
    ModalDialog( pParent,WB_3DLOOK | WB_MOVEABLE | WB_CLOSEABLE ),
    aEdit( VclPtr<Edit>::Create(this,  WB_LEFT | WB_BORDER) ),
    aOk( VclPtr<OKButton>::Create(this) ), aCancel( VclPtr<CancelButton>::Create(this) ), aPromptText( VclPtr<FixedText>::Create(this, WB_WORDBREAK) )
{
    SetMapMode( MapMode( MAP_APPFONT ) );
    Size aDlgSizeApp( 280, 80 );
    PositionDialog( nXTwips, nYTwips, aDlgSizeApp );
    InitButtons( aDlgSizeApp );
    PositionEdit( aDlgSizeApp );
    PositionPrompt( rPrompt, aDlgSizeApp );
    aOk->Show();
    aCancel->Show();
    aEdit->Show();
    aPromptText->Show();
    SetText( rTitle );
    vcl::Font aFont( GetFont());
    Color aColor( GetBackground().GetColor() );
    aFont.SetFillColor( aColor );
    aEdit->SetFont( aFont );
    aEdit->SetText( rDefault );
    aEdit->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
}

void SvRTLInputBox::dispose()
{
    aEdit.disposeAndClear();
    aOk.disposeAndClear();
    aCancel.disposeAndClear();
    aPromptText.disposeAndClear();
    ModalDialog::dispose();
}

void SvRTLInputBox::InitButtons( const Size& rDlgSize )
{
    aOk->SetSizePixel( LogicToPixel( Size( 45, 15) ));
    aCancel->SetSizePixel( LogicToPixel( Size( 45, 15) ));
    Point aPos( rDlgSize.Width()-45-10, 5 );
    aOk->SetPosPixel( LogicToPixel( Point(aPos) ));
    aPos.Y() += 16;
    aCancel->SetPosPixel( LogicToPixel( Point(aPos) ));
    aOk->SetClickHdl(LINK(this,SvRTLInputBox, OkHdl));
    aCancel->SetClickHdl(LINK(this,SvRTLInputBox,CancelHdl));
}

void SvRTLInputBox::PositionDialog(long nXTwips, long nYTwips, const Size& rDlgSize)
{
    SetSizePixel( LogicToPixel(rDlgSize) );
    if( nXTwips != -1 && nYTwips != -1 )
    {
        Point aDlgPosApp( nXTwips, nYTwips );
        SetPosPixel( LogicToPixel( aDlgPosApp, MAP_TWIP ) );
    }
}

void SvRTLInputBox::PositionEdit( const Size& rDlgSize )
{
    aEdit->SetPosPixel( LogicToPixel( Point( 5,rDlgSize.Height()-35)));
    aEdit->SetSizePixel( LogicToPixel( Size(rDlgSize.Width()-15,12)));
}


void SvRTLInputBox::PositionPrompt(const OUString& rPrompt,const Size& rDlgSize)
{
    if ( rPrompt.isEmpty() )
        return;
    OUString aText_(convertLineEnd(rPrompt, LINEEND_CR));
    aPromptText->SetPosPixel( LogicToPixel(Point(5,5)));
    aPromptText->SetText( aText_ );
    Size aSize( rDlgSize );
    aSize.Width() -= 70;
    aSize.Height() -= 50;
    aPromptText->SetSizePixel( LogicToPixel(aSize));
}


IMPL_LINK_NOARG_TYPED( SvRTLInputBox, OkHdl, Button *, void )
{
    aText = aEdit->GetText();
    EndDialog( 1 );
}

IMPL_LINK_NOARG_TYPED( SvRTLInputBox, CancelHdl, Button *, void )
{
    aText.clear();
    EndDialog();
}


// *********************************************************************
// *********************************************************************

// Syntax: String InputBox( Prompt, [Title], [Default] [, nXpos, nYpos ] )

RTLFUNC(InputBox)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt32 nArgCount = rPar.Count();
    if ( nArgCount < 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        OUString aTitle;
        OUString aDefault;
        sal_Int32 nX = -1, nY = -1;  // center
        const OUString& rPrompt = rPar.Get(1)->GetOUString();
        if ( nArgCount > 2 && !rPar.Get(2)->IsErr() )
            aTitle = rPar.Get(2)->GetOUString();
        if ( nArgCount > 3 && !rPar.Get(3)->IsErr() )
            aDefault = rPar.Get(3)->GetOUString();
        if ( nArgCount > 4 )
        {
            if ( nArgCount != 6 )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                return;
            }
            nX = rPar.Get(4)->GetLong();
            nY = rPar.Get(5)->GetLong();
        }
        VclPtrInstance<SvRTLInputBox> pDlg(Application::GetDefDialogParent(),
                                           rPrompt,aTitle,aDefault,nX,nY);
        pDlg->Execute();
        rPar.Get(0)->PutString( pDlg->GetText() );
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
