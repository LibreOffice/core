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
#include <o3tl/typed_flags_set.hxx>

class VclMultiLineEdit;
class FixedImage;
class CheckBox;

// Window-Bits for MessageBoxen
enum class MessBoxStyle {
    NONE              = 0x0000,
    Ok                = 0x0001,
    OkCancel          = 0x0002,
    YesNo             = 0x0004,
    YesNoCancel       = 0x0008,
    RetryCancel       = 0x0010,
    DefaultOk         = 0x0020,
    DefaultCancel     = 0x0040,
    DefaultRetry      = 0x0080,
    DefaultYes        = 0x0100,
    DefaultNo         = 0x0200,
    AbortRetryIgnore  = 0x1000,
    DefaultIgnore     = 0x2000,
};
namespace o3tl {
    template<> struct typed_flags<MessBoxStyle> : is_typed_flags<MessBoxStyle, 0x3fff> {};
}


class VCL_DLLPUBLIC MessBox : public ButtonDialog
{
    VclPtr<VclMultiLineEdit>   mpVCLMultiLineEdit;
    VclPtr<FixedImage>         mpFixedImage;
    Image                      maImage;
    bool                       mbHelpBtn;
    bool                       mbCheck;
    MessBoxStyle               mnMessBoxStyle;

protected:
    OUString                   maMessText;
    VclPtr<CheckBox>           mpCheckBox;
    OUString                   maCheckBoxText;

    SAL_DLLPRIVATE void ImplInitButtons();
    SAL_DLLPRIVATE void ImplPosControls();

public:
                        MessBox(vcl::Window* pParent, MessBoxStyle nMessBoxStyle, WinBits n,
                                const OUString& rTitle, const OUString& rMessage);
    virtual             ~MessBox() override;
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

VCL_DLLPUBLIC Image const & GetStandardInfoBoxImage();
VCL_DLLPUBLIC OUString GetStandardInfoBoxText();

class VCL_DLLPUBLIC WarningBox : public MessBox
{
public:
                        WarningBox( vcl::Window* pParent, MessBoxStyle nStyle,
                                    const OUString& rMessage );
                        WarningBox( vcl::Window* pParent, MessBoxStyle nStyle, WinBits n,
                                    const OUString& rMessage );

    void                SetDefaultCheckBoxText();

    static Image const & GetStandardImage();
    static OUString     GetStandardText();
};

VCL_DLLPUBLIC Image const & GetStandardErrorBoxImage();
VCL_DLLPUBLIC OUString GetStandardErrorBoxText();

VCL_DLLPUBLIC Image const & GetStandardQueryBoxImage();
VCL_DLLPUBLIC OUString GetStandardQueryBoxText();

#endif // INCLUDED_VCL_MSGBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
