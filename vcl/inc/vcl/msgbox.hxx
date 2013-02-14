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

#ifndef _SV_MSGBOX_HXX
#define _SV_MSGBOX_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/btndlg.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
class VclMultiLineEdit;
class FixedImage;
class CheckBox;

// -----------------
// - MessBox-Types -
// -----------------

// Return-Werte von Execute
//!!! bei Aenderungen \basic\source\runtime\methods.cxx msgbox anpassen

#define RET_CANCEL           0
#define RET_OK               1
#define RET_YES              2
#define RET_NO               3
#define RET_RETRY            4
#define RET_IGNORE           5

#define BUTTONID_OK          RET_OK
#define BUTTONID_CANCEL      RET_CANCEL
#define BUTTONID_YES         RET_YES
#define BUTTONID_NO          RET_NO
#define BUTTONID_RETRY       RET_RETRY
#define BUTTONID_IGNORE      RET_IGNORE
#define BUTTONID_HELP        10

// -----------
// - MessBox -
// -----------

class VCL_DLLPUBLIC MessBox : public ButtonDialog
{
protected:
    VclMultiLineEdit*   mpVCLMultiLineEdit;
    FixedImage*         mpFixedImage;
    XubString           maMessText;
    Image               maImage;
    sal_Bool                mbHelpBtn;
    CheckBox*           mpCheckBox;
    XubString           maCheckBoxText;
    sal_Bool                mbCheck;

    SAL_DLLPRIVATE void ImplInitMessBoxData();
    SAL_DLLPRIVATE void ImplInitButtons();
    SAL_DLLPRIVATE void ImplPosControls();

protected:
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );

public:
                        MessBox( Window* pParent, WinBits nStyle,
                                 const XubString& rTitle, const XubString& rMessage );
                        MessBox( Window* pParent, const ResId& rResId );
                        ~MessBox();

    virtual void        StateChanged( StateChangedType nStateChange );

    void                SetMessText( const XubString& rText ) { maMessText = rText; }
    const XubString&    GetMessText() const { return maMessText; }

    void                SetImage( const Image& rImage ) { maImage = rImage; }
    const Image&        GetImage() const { return maImage; }

    void                SetCheckBoxText( const XubString& rText ) { maCheckBoxText = rText;}
    const XubString&    GetCheckBoxText() const { return maCheckBoxText;}
    void                SetCheckBoxState( sal_Bool bCheck );
    sal_Bool                GetCheckBoxState() const;

    virtual Size        GetOptimalSize() const;
};

// -----------
// - InfoBox -
// -----------

class VCL_DLLPUBLIC InfoBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitInfoBoxData();

public:
                        InfoBox( Window* pParent, const OUString& rMessage );
                        InfoBox( Window* pParent, const ResId & rResId );

    static Image        GetStandardImage();
};

// --------------
// - WarningBox -
// --------------

class VCL_DLLPUBLIC WarningBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitWarningBoxData();

public:
                        WarningBox( Window* pParent, WinBits nStyle,
                                    const XubString& rMessage );
                        WarningBox( Window* pParent, const ResId& rResId );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
};

// ------------
// - ErrorBox -
// ------------

class VCL_DLLPUBLIC ErrorBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitErrorBoxData();

public:
                        ErrorBox( Window* pParent, WinBits nStyle,
                                  const XubString& rMessage );
                        ErrorBox( Window* pParent, const ResId& rResId );

    static Image        GetStandardImage();
};

// ------------
// - QueryBox -
// ------------

class VCL_DLLPUBLIC QueryBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitQueryBoxData();

public:
                        QueryBox( Window* pParent, WinBits nStyle,
                                  const XubString& rMessage );
                        QueryBox( Window* pParent, const ResId& rResId );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
};

#endif  // _SV_MSGBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
