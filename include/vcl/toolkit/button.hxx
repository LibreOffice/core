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
#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <vcl/button.hxx>

class VCL_DLLPUBLIC OKButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void            ImplInit( vcl::Window* pParent, WinBits nStyle );

                                   OKButton (const OKButton &) = delete;
                                   OKButton & operator= (const OKButton &) = delete;

public:
    explicit        OKButton( vcl::Window* pParent, WinBits nStyle = WB_DEFBUTTON );

    virtual void    Click() override;
};

class VCL_DLLPUBLIC CancelButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );

                        CancelButton (const CancelButton &) = delete;
                        CancelButton & operator= (const CancelButton &) = delete;

public:
    explicit        CancelButton( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    Click() override;
};

class CloseButton final : public CancelButton
{
public:
    explicit CloseButton(vcl::Window* pParent, WinBits nStyle = 0);
};

class VCL_DLLPUBLIC HelpButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );

                        HelpButton( const HelpButton & ) = delete;
                        HelpButton & operator= ( const HelpButton & ) = delete;

    virtual void    StateChanged( StateChangedType nStateChange ) override;

public:
    explicit        HelpButton( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    Click() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
