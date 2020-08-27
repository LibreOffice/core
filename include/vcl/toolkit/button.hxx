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

class VCL_DLLPUBLIC OKButton final : public PushButton
{
private:
    using PushButton::ImplInit;

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

class VCL_DLLPUBLIC HelpButton final : public PushButton
{
private:
    using PushButton::ImplInit;
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );

                        HelpButton( const HelpButton & ) = delete;
                        HelpButton & operator= ( const HelpButton & ) = delete;

    virtual void    StateChanged( StateChangedType nStateChange ) override;

public:
    explicit        HelpButton( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    Click() override;
};

class VCL_DLLPUBLIC CheckBox : public Button
{
private:
    tools::Rectangle       maStateRect;
    tools::Rectangle       maMouseRect;
    TriState        meState;
    bool            mbTriState;
    Link<CheckBox&,void> maToggleHdl;
    SAL_DLLPRIVATE void         ImplInitCheckBoxData();
    SAL_DLLPRIVATE static WinBits ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitSettings( bool bBackground );
    SAL_DLLPRIVATE void         ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                                    const Point& rPos, const Size& rSize,
                                    const Size& rImageSize, tools::Rectangle& rStateRect,
                                    tools::Rectangle& rMouseRect );
    SAL_DLLPRIVATE void         ImplDrawCheckBox(vcl::RenderContext& rRenderContext );
    SAL_DLLPRIVATE long         ImplGetImageToTextDistance() const;
    SAL_DLLPRIVATE Size         ImplGetCheckImageSize() const;

                                CheckBox(const CheckBox &) = delete;
                                CheckBox& operator= (const CheckBox &) = delete;

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( vcl::Window* pParent, WinBits nStyle );
    virtual void                FillLayoutData() const override;
    virtual const vcl::Font&    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&        GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;
    void                        ImplAdjustNWFSizes() override;

    virtual void ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE const tools::Rectangle& GetStateRect() const { return maStateRect; }
    SAL_DLLPRIVATE const tools::Rectangle& GetMouseRect() const { return maMouseRect; }

public:
    SAL_DLLPRIVATE void         ImplCheck();
public:
    explicit        CheckBox( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    KeyUp( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    void            Toggle();

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( bool bCheck = true )
    {
        SetState( bCheck ? TRISTATE_TRUE : TRISTATE_FALSE );
    }

    bool            IsChecked() const
    {
        return (GetState() == TRISTATE_TRUE);
    }

    void            EnableTriState( bool bTriState = true );
    bool            IsTriStateEnabled() const { return mbTriState; }

    static Image    GetCheckImage( const AllSettings& rSettings, DrawButtonFlags nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize() const override;

    void            SetToggleHdl( const Link<CheckBox&,void>& rLink ) { maToggleHdl = rLink; }

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    virtual void ShowFocus(const tools::Rectangle& rRect) override;

    /// Button hes additional stuff that we need to dump too.
    void DumpAsPropertyTree(tools::JsonWriter&) override;

    virtual FactoryFunction GetUITestFactory() const override;
};

class VCL_DLLPUBLIC ImageButton final : public PushButton
{
private:
    using PushButton::ImplInitStyle;

    SAL_DLLPRIVATE void     ImplInitStyle();

                            ImageButton( const ImageButton & ) = delete;
                            ImageButton & operator= ( const ImageButton & ) = delete;

public:
                 ImageButton( vcl::Window* pParent, WinBits nStyle = 0 );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
