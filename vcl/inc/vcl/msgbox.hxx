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

#ifndef _SV_MSGBOX_HXX
#define _SV_MSGBOX_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/btndlg.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
class FixedText;
class FixedImage;
class CheckBox;

// -----------------
// - MessBox-Types -
// -----------------

// Return-Werte von Execute
//!!! bei Aenderungen \basic\source\runtime\methods.cxx msgbox anpassen

#define RET_OK               sal_True
#define RET_CANCEL           sal_False
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
    FixedText*          mpFixedText;
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

    virtual Size        GetOptimalSize(WindowSizeType eType) const;
};

// -----------
// - InfoBox -
// -----------

class VCL_DLLPUBLIC InfoBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitInfoBoxData();

public:
                        InfoBox( Window* pParent, const XubString& rMessage );
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
