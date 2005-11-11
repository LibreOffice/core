/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filectrl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 09:16:42 $
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

#ifndef _SV_FILECTRL_HXX
#define _SV_FILECTRL_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif


#define STR_FILECTRL_BUTTONTEXT     333     // ID-Range?!

// Flags for FileControl
typedef USHORT FileControlMode;
#define FILECTRL_RESIZEBUTTONBYPATHLEN  ((USHORT)0x0001)//if this is set, the button will become small as soon as the Text in the Edit Field is to long to be shown completely


// Flags for internal use of FileControl
typedef USHORT FileControlMode_Internal;
#define FILECTRL_INRESIZE               ((USHORT)0x0001)
#define FILECTRL_ORIGINALBUTTONTEXT     ((USHORT)0x0002)


class SVT_DLLPUBLIC FileControl : public Window
{
private:
    Edit            maEdit;
    PushButton      maButton;

    String          maButtonText;
    BOOL            mbOpenDlg;

    Link            maDialogCreatedHdl;

    FileControlMode             mnFlags;
    FileControlMode_Internal    mnInternalFlags;

private:
    SVT_DLLPRIVATE void     ImplBrowseFile( );

protected:
    SVT_DLLPRIVATE void     Resize();
    SVT_DLLPRIVATE void     GetFocus();
    SVT_DLLPRIVATE void     StateChanged( StateChangedType nType );
    SVT_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    DECL_DLLPRIVATE_LINK( ButtonHdl, PushButton* );

public:
                    FileControl( Window* pParent, WinBits nStyle, FileControlMode = 0 );
                    ~FileControl();

    Edit&           GetEdit() { return maEdit; }
    PushButton&     GetButton() { return maButton; }

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );

    void            SetOpenDialog( BOOL bOpen )     { mbOpenDlg = bOpen; }
    BOOL            IsOpenDialog() const            { return mbOpenDlg; }

    void            SetText( const XubString& rStr );
    XubString       GetText() const;
    UniString           GetSelectedText() const         { return maEdit.GetSelected(); }

    void            SetSelection( const Selection& rSelection ) { maEdit.SetSelection( rSelection ); }
    Selection       GetSelection() const                        { return maEdit.GetSelection(); }

    void            SetReadOnly( BOOL bReadOnly = TRUE )    { maEdit.SetReadOnly( bReadOnly ); }
    BOOL            IsReadOnly() const                      { return maEdit.IsReadOnly(); }

    //------
    //manipulate the Button-Text:
    XubString       GetButtonText() const { return maButtonText; }
    void            SetButtonText( const XubString& rStr );
    void            ResetButtonText();

    //------
    //use this to manipulate the dialog bevore executing it:
    void            SetDialogCreatedHdl( const Link& rLink ) { maDialogCreatedHdl = rLink; }
    const Link&     GetDialogCreatedHdl() const { return maDialogCreatedHdl; }
};

#endif

