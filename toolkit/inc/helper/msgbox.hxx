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

#include <o3tl/typed_flags_set.hxx>
#include <helper/btndlg.hxx>
#include <vcl/toolkit/fixed.hxx>

// Window-Bits for MessageBoxen
enum class MessBoxStyle
{
    NONE = 0x0000,
    Ok = 0x0001,
    OkCancel = 0x0002,
    YesNo = 0x0004,
    YesNoCancel = 0x0008,
    RetryCancel = 0x0010,
    DefaultOk = 0x0020,
    DefaultCancel = 0x0040,
    DefaultRetry = 0x0080,
    DefaultYes = 0x0100,
    DefaultNo = 0x0200,
    AbortRetryIgnore = 0x1000,
    DefaultIgnore = 0x2000,
};
namespace o3tl
{
template <> struct typed_flags<MessBoxStyle> : is_typed_flags<MessBoxStyle, 0x33ff>
{
};
}

class MessBox : public ButtonDialog
{
    VclPtr<VclMultiLineEdit> mpVCLMultiLineEdit;
    VclPtr<FixedImage> mpFixedImage;
    Image maImage;
    bool mbHelpBtn;
    MessBoxStyle mnMessBoxStyle;

protected:
    OUString maMessText;

    void ImplInitButtons();
    void ImplPosControls();

public:
    MessBox(vcl::Window* pParent, MessBoxStyle nMessBoxStyle, WinBits n, const OUString& rTitle,
            const OUString& rMessage);
    virtual ~MessBox() override;
    virtual void dispose() override;

    virtual void StateChanged(StateChangedType nStateChange) override;

    void SetMessText(const OUString& rText) { maMessText = rText; }
    const OUString& GetMessText() const { return maMessText; }

    void SetImage(const Image& rImage) { maImage = rImage; }

    virtual Size GetOptimalSize() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
