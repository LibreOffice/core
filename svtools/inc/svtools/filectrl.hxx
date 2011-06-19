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

#ifndef _SV_FILECTRL_HXX
#define _SV_FILECTRL_HXX

#include "svtools/svtdllapi.h"
#include <vcl/window.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>


#define STR_FILECTRL_BUTTONTEXT     333     // ID-Range?!

// Flags for FileControl
typedef sal_uInt16 FileControlMode;
#define FILECTRL_RESIZEBUTTONBYPATHLEN  ((sal_uInt16)0x0001)//if this is set, the button will become small as soon as the Text in the Edit Field is to long to be shown completely


// Flags for internal use of FileControl
typedef sal_uInt16 FileControlMode_Internal;
#define FILECTRL_INRESIZE               ((sal_uInt16)0x0001)
#define FILECTRL_ORIGINALBUTTONTEXT     ((sal_uInt16)0x0002)


class SVT_DLLPUBLIC FileControl : public Window
{
private:
    Edit            maEdit;
    PushButton      maButton;

    String          maButtonText;
    sal_Bool            mbOpenDlg;

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

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

    void            SetOpenDialog( sal_Bool bOpen )     { mbOpenDlg = bOpen; }
    sal_Bool            IsOpenDialog() const            { return mbOpenDlg; }

    void            SetText( const XubString& rStr );
    XubString       GetText() const;
    UniString           GetSelectedText() const         { return maEdit.GetSelected(); }

    void            SetSelection( const Selection& rSelection ) { maEdit.SetSelection( rSelection ); }
    Selection       GetSelection() const                        { return maEdit.GetSelection(); }

    void            SetReadOnly( sal_Bool bReadOnly = sal_True )    { maEdit.SetReadOnly( bReadOnly ); }
    sal_Bool            IsReadOnly() const                      { return maEdit.IsReadOnly(); }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
