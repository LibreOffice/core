/*************************************************************************
 *
 *  $RCSfile: inputbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <svtools/sbx.hxx>
#include "runtime.hxx"
#pragma hdrstop
#include "stdobj.hxx"
#include "rtlproto.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )


class SvRTLInputBox : public ModalDialog
{
    Edit aEdit;
    OKButton aOk;
    CancelButton aCancel;
    FixedText aPromptText;
    String aText;

    void PositionDialog( long nXTwips, long nYTwips, const Size& rDlgSize );
    void InitButtons( const Size& rDlgSize );
    void PositionEdit( const Size& rDlgSize );
    void PositionPrompt( const String& rPrompt, const Size& rDlgSize );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( CancelHdl, Button * );

public:
    SvRTLInputBox( Window* pParent, const String& rPrompt, const String& rTitle,
        const String& rDefault, long nXTwips = -1, long nYTwips = -1 );
    String GetText() const { return aText; }
};

SvRTLInputBox::SvRTLInputBox( Window* pParent, const String& rPrompt,
        const String& rTitle, const String& rDefault,
        long nXTwips, long nYTwips ) :
    ModalDialog( pParent,WB_SVLOOK | WB_MOVEABLE | WB_CLOSEABLE ),
    aEdit( this,  WB_LEFT | WB_BORDER ),
    aOk( this ), aCancel( this ), aPromptText( this, WB_WORDBREAK )
{
    SetMapMode( MapMode( MAP_APPFONT ) );
    Size aDlgSizeApp( 280, 80 );
    PositionDialog( nXTwips, nYTwips, aDlgSizeApp );
    InitButtons( aDlgSizeApp );
    PositionEdit( aDlgSizeApp );
    PositionPrompt( rPrompt, aDlgSizeApp );
    aOk.Show();
    aCancel.Show();
    aEdit.Show();
    aPromptText.Show();
    SetText( rTitle );
    Font aFont( GetFont());
    Color aColor( GetBackgroundBrush().GetFillColor());
    aFont.SetFillColor( aColor );
    aEdit.SetFont( aFont );
    aEdit.SetText( rDefault );
    aEdit.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
}

void SvRTLInputBox::InitButtons( const Size& rDlgSize )
{
    aOk.SetSizePixel( LogicToPixel( Size( 45, 15) ));
    aCancel.SetSizePixel( LogicToPixel( Size( 45, 15) ));
    Point aPos( rDlgSize.Width()-45-10, 5 );
    aOk.SetPosPixel( LogicToPixel( Point(aPos) ));
    aPos.Y() += 16;
    aCancel.SetPosPixel( LogicToPixel( Point(aPos) ));
    aOk.SetClickHdl(LINK(this,SvRTLInputBox, OkHdl));
    aCancel.SetClickHdl(LINK(this,SvRTLInputBox,CancelHdl));
}

void SvRTLInputBox::PositionDialog(long nXTwips, long nYTwips, const Size& rDlgSize)
{
    Size aScreenSzApp(Window::GetOutputSizePixel());
    aScreenSzApp = PixelToLogic( aScreenSzApp );

    Point aDlgPosApp( nXTwips, nYTwips );
    aDlgPosApp = LogicToPixel( aDlgPosApp, MAP_TWIP );
    aDlgPosApp = PixelToLogic( aDlgPosApp );
    if ( nXTwips == -1 || nYTwips == -1 ||
         aDlgPosApp.X() >= aScreenSzApp.Width() ||
         aDlgPosApp.Y() >= aScreenSzApp.Height() )
    {
        aDlgPosApp.X() = ( aScreenSzApp.Width() - rDlgSize.Width() ) / 2;
        aDlgPosApp.Y() = ( aScreenSzApp.Height() - rDlgSize.Height() ) / 2;
    }
    SetSizePixel( LogicToPixel(rDlgSize) );
    SetPosPixel( LogicToPixel(aDlgPosApp) );
}

void SvRTLInputBox::PositionEdit( const Size& rDlgSize )
{
    aEdit.SetPosPixel( LogicToPixel( Point( 5,rDlgSize.Height()-35)));
    aEdit.SetSizePixel( LogicToPixel( Size(rDlgSize.Width()-15,12)));
}


void SvRTLInputBox::PositionPrompt(const String& rPrompt,const Size& rDlgSize)
{
    if ( rPrompt.Len() == 0 )
        return;
    String aText( rPrompt );
    aText.ConvertLineEnd( LINEEND_CR );
    aPromptText.SetPosPixel( LogicToPixel(Point(5,5)));
    aPromptText.SetText( aText );
    Size aSize( rDlgSize );
    aSize.Width() -= 70;
    aSize.Height() -= 50;
    aPromptText.SetSizePixel( LogicToPixel(aSize));
}


IMPL_LINK_INLINE_START( SvRTLInputBox, OkHdl, Button *, pButton )
{
    aText = aEdit.GetText();
    EndDialog( 1 );
    return 0;
}
IMPL_LINK_INLINE_END( SvRTLInputBox, OkHdl, Button *, pButton )

IMPL_LINK_INLINE_START( SvRTLInputBox, CancelHdl, Button *, pButton )
{
    aText.Erase();
    EndDialog( 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SvRTLInputBox, CancelHdl, Button *, pButton )


// *********************************************************************
// *********************************************************************
// *********************************************************************

// Syntax: String InputBox( Prompt, [Title], [Default] [, nXpos, nYpos ] )

RTLFUNC(InputBox)
{
    ULONG nArgCount = rPar.Count();
    if ( nArgCount < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aTitle;
        String aDefault;
        INT32 nX = -1, nY = -1;  // zentrieren
        const String& rPrompt = rPar.Get(1)->GetString();
        if ( nArgCount > 2 )
            aTitle = rPar.Get(2)->GetString();
        if ( nArgCount > 3 )
            aDefault = rPar.Get(3)->GetString();
        if ( nArgCount > 4 )
        {
            if ( nArgCount != 6 )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return;
            }
            nX = rPar.Get(4)->GetLong();
            nY = rPar.Get(5)->GetLong();
        }
        SvRTLInputBox *pDlg=new SvRTLInputBox(GetpApp()->GetDefModalDialogParent(),
                    rPrompt,aTitle,aDefault,nX,nY);
        pDlg->Execute();
        rPar.Get(0)->PutString( pDlg->GetText() );
        delete pDlg;
    }
}



