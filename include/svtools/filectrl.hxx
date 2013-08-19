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

    OUString        maButtonText;
    sal_Bool        mbOpenDlg;

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
    DECL_DLLPRIVATE_LINK( ButtonHdl, void* );

public:
                    FileControl( Window* pParent, WinBits nStyle, FileControlMode = 0 );
                    ~FileControl();

    Edit&           GetEdit() { return maEdit; }
    PushButton&     GetButton() { return maButton; }

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

    void            SetOpenDialog( sal_Bool bOpen )     { mbOpenDlg = bOpen; }
    sal_Bool            IsOpenDialog() const            { return mbOpenDlg; }

    void            SetText( const OUString& rStr );
    OUString        GetText() const;
    OUString   GetSelectedText() const         { return maEdit.GetSelected(); }

    void            SetSelection( const Selection& rSelection ) { maEdit.SetSelection( rSelection ); }
    Selection       GetSelection() const                        { return maEdit.GetSelection(); }

    void            SetReadOnly( sal_Bool bReadOnly = sal_True )    { maEdit.SetReadOnly( bReadOnly ); }
    sal_Bool            IsReadOnly() const                      { return maEdit.IsReadOnly(); }

    //------
    //use this to manipulate the dialog bevore executing it:
    void            SetDialogCreatedHdl( const Link& rLink ) { maDialogCreatedHdl = rLink; }
    const Link&     GetDialogCreatedHdl() const { return maDialogCreatedHdl; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
