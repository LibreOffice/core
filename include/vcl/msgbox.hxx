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

#ifndef INCLUDED_VCL_MSGBOX_HXX
#define INCLUDED_VCL_MSGBOX_HXX

#include <vcl/dllapi.h>
#include <vcl/btndlg.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>

class VclMultiLineEdit;
class FixedImage;
class CheckBox;

class VCL_DLLPUBLIC MessBox : public ButtonDialog
{
protected:
    VclPtr<VclMultiLineEdit>   mpVCLMultiLineEdit;
    VclPtr<FixedImage>         mpFixedImage;
    OUString                   maMessText;
    Image                      maImage;
    bool                       mbHelpBtn;
    VclPtr<CheckBox>           mpCheckBox;
    OUString                   maCheckBoxText;
    bool                       mbCheck;

    SAL_DLLPRIVATE void ImplInitMessBoxData();
    SAL_DLLPRIVATE void ImplInitButtons();
    SAL_DLLPRIVATE void ImplPosControls();

public:
                        MessBox( vcl::Window* pParent, WinBits nStyle,
                                 const OUString& rTitle, const OUString& rMessage );
    virtual             ~MessBox();
    virtual void        dispose() override;

    virtual void        StateChanged( StateChangedType nStateChange ) override;

    void                SetMessText( const OUString& rText ) { maMessText = rText; }
    const OUString&     GetMessText() const { return maMessText; }

    void                SetImage( const Image& rImage ) { maImage = rImage; }

    void                SetCheckBoxText( const OUString& rText ) { maCheckBoxText = rText;}
    void                SetCheckBoxState( bool bCheck );
    bool                GetCheckBoxState() const;

    virtual Size        GetOptimalSize() const override;
};

class VCL_DLLPUBLIC InfoBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitInfoBoxData();

public:
                        InfoBox( vcl::Window* pParent, const OUString& rMessage );
                        InfoBox( vcl::Window* pParent, WinBits nStyle,
                                const OUString& rMessage );

    static Image        GetStandardImage();
};

class VCL_DLLPUBLIC WarningBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitWarningBoxData();

public:
                        WarningBox( vcl::Window* pParent, WinBits nStyle,
                                    const OUString& rMessage );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
};

class VCL_DLLPUBLIC ErrorBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitErrorBoxData();

public:
                        ErrorBox( vcl::Window* pParent, WinBits nStyle,
                                  const OUString& rMessage );

    static Image        GetStandardImage();
};

class VCL_DLLPUBLIC QueryBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitQueryBoxData();

public:
                        QueryBox( vcl::Window* pParent, WinBits nStyle,
                                  const OUString& rMessage );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
};

#endif // INCLUDED_VCL_MSGBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
