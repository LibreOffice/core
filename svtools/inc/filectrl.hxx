/*************************************************************************
 *
 *  $RCSfile: filectrl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:50 $
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

#ifndef _SV_FILECTRL_HXX
#define _SV_FILECTRL_HXX

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


class VclFileDialog;
class FileDialog;
class FileControl : public Window
{
private:
    Edit            maEdit;
    PushButton      maButton;

    String          maButtonText;
    BOOL            mbOpenDlg;

    VclFileDialog*  mpVclDlg;
    FileDialog*     mpFDlg;

    Link            maDialogCreatedHdl;

    FileControlMode             mnFlags;
    FileControlMode_Internal    mnInternalFlags;

protected:
    void            Resize();
    void            GetFocus();
    void            StateChanged( StateChangedType nType );
    WinBits         ImplInitStyle( WinBits nStyle );
    DECL_LINK(      ButtonHdl, PushButton* );

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

        //only use the next two methods in 'DialogCreatedHdl' and don't store the dialog-pointer
    VclFileDialog*  GetVclFileDialog() const { return mpVclDlg; }
    FileDialog*     GetFileDialog() const { return mpFDlg; }
    //------
};

#endif

