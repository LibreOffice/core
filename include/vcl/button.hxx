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

#ifndef INCLUDED_VCL_BUTTON_HXX
#define INCLUDED_VCL_BUTTON_HXX

#include <tools/solar.h>
#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salnativewidgets.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/vclptr.hxx>
#include <vector>

#include <com/sun/star/frame/FeatureStateEvent.hpp>

class UserDrawEvent;
class ImplCommonButtonData;
enum class DrawButtonFlags;

class VCL_DLLPUBLIC Button : public Control
{
private:
    std::unique_ptr<ImplCommonButtonData> mpButtonData;
    Link<Button*,void> maClickHdl;

    /// Command URL (like .uno:Save) in case the button should handle it.
    OUString maCommand;

                                    Button (const Button &) = delete;
                                    Button & operator= (const Button &) = delete;
public:
    SAL_DLLPRIVATE DrawButtonFlags  ImplGetButtonState() const;
    SAL_DLLPRIVATE DrawButtonFlags& ImplGetButtonState();
    SAL_DLLPRIVATE DrawTextFlags    ImplGetTextStyle( OUString& rText, WinBits nWinStyle, DrawFlags nDrawFlags );
    SAL_DLLPRIVATE void             ImplDrawAlignedImage(OutputDevice* pDev, Point& rPos, Size& rSize,
                                              sal_uLong nImageSep, DrawFlags nDrawFlags,
                                              DrawTextFlags nTextStyle, Rectangle *pSymbolRect=nullptr, bool bAddImageSep = false );
    SAL_DLLPRIVATE void             ImplSetFocusRect( const Rectangle &rFocusRect );
    SAL_DLLPRIVATE const Rectangle& ImplGetFocusRect() const;
    SAL_DLLPRIVATE void             ImplSetSymbolAlign( SymbolAlign eAlign );
    /// The x-coordinate of the vertical separator line, use in MenuButton subclass only.
    SAL_DLLPRIVATE long             ImplGetSeparatorX() const;
    SAL_DLLPRIVATE void             ImplSetSeparatorX( long nX );

protected:
    explicit            Button( WindowType nType );

public:
    virtual            ~Button() override;
    virtual void        dispose() override;

    virtual void        Click();

    void                SetClickHdl( const Link<Button*,void>& rLink ) { maClickHdl = rLink; }
    const Link<Button*,void>& GetClickHdl() const { return maClickHdl; }

    /// Setup handler for UNO commands so that commands like .uno:Something are handled automagically by this button.
    void                SetCommandHandler(const OUString& aCommand);
    const OUString      GetCommand() const { return maCommand; }

    static OUString     GetStandardText( StandardButtonType eButton );

    bool                SetModeImage( const Image& rImage );
    const Image         GetModeImage( ) const;
    bool                HasImage() const;
    void                SetImageAlign( ImageAlign eAlign );
    ImageAlign          GetImageAlign() const;

    void                EnableImageDisplay( bool bEnable );
    void                EnableTextDisplay( bool bEnable );

    void                SetFocusRect( const Rectangle& rFocusRect );
    bool                IsSmallSymbol() const;
    void                SetSmallSymbol();
    virtual bool        set_property(const OString &rKey, const OString &rValue) override;

    /// Sets the button state according to the FeatureStateEvent emitted by an Uno state change.
    virtual void        statusChanged(const css::frame::FeatureStateEvent& rEvent);

    virtual FactoryFunction GetUITestFactory() const override;

protected:

    /// Handler for click, in case we want the button to handle uno commands (.uno:Something).
    DECL_STATIC_LINK(Button, dispatchCommandHandler, Button*, void);
};

enum class PushButtonDropdownStyle
{
    NONE            = 0x0000,
    Toolbox         = 0x0001,
    MenuButton      = 0x0002, //visual down arrow
    SplitMenuButton = 0x0003, //visual down arrow and seperator line
};

class VCL_DLLPUBLIC PushButton : public Button
{
public:
    SAL_DLLPRIVATE void            ImplSetDefButton( bool bSet );
    SAL_DLLPRIVATE void            ImplDrawPushButtonFrame(vcl::RenderContext& rRenderContext, Rectangle& rRect, DrawButtonFlags nStyle);
    SAL_DLLPRIVATE static bool     ImplHitTestPushButton(vcl::Window* pDev, const Point& rPos);
    SAL_DLLPRIVATE bool            ImplIsDefButton() const;

    explicit        PushButton( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    KeyUp( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    void            Toggle();

    void            SetSymbol( SymbolType eSymbol );
    SymbolType      GetSymbol() const { return meSymbol; }
    void            SetSymbolAlign( SymbolAlign eAlign );

    void            SetDropDown( PushButtonDropdownStyle nStyle );

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }
    virtual void    statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    void            Check( bool bCheck = true );
    bool            IsChecked() const;

    void            SetPressed( bool bPressed );
    bool            IsPressed() const { return mbPressed; }

    void            EndSelection();

    void            SaveValue() { meSaveValue = GetState(); }
    bool            IsValueChangedFromSaved() const { return meSaveValue != GetState(); }

    Size            CalcMinimumSize() const;
    virtual Size    GetOptimalSize() const override;

    virtual bool    set_property(const OString &rKey, const OString &rValue) override;
    virtual void    ShowFocus(const Rectangle& rRect) override;

protected:
    PushButtonDropdownStyle mnDDStyle;
    bool            mbIsActive;

    SAL_DLLPRIVATE void            ImplInitPushButtonData();
    SAL_DLLPRIVATE static WinBits  ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void            ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void            ImplDrawPushButtonContent(OutputDevice* pDev, DrawFlags nDrawFlags,
                                                             const Rectangle& rRect, bool bMenuBtnSep);
    SAL_DLLPRIVATE void            ImplDrawPushButton(vcl::RenderContext& rRenderContext);
    using Button::ImplGetTextStyle;
    SAL_DLLPRIVATE DrawTextFlags   ImplGetTextStyle( DrawFlags nDrawFlags ) const;
    SAL_DLLPRIVATE bool            IsSymbol() const { return ( (meSymbol != SymbolType::DONTKNOW) && (meSymbol != SymbolType::IMAGE) ); }

                                   PushButton( const PushButton & ) = delete;
                                   PushButton& operator=( const PushButton & )
                                       = delete;

    SAL_DLLPRIVATE void            ImplInit( vcl::Window* pParent, WinBits nStyle );

    using Control::ImplInitSettings;
    using Window::ImplInit;

    explicit        PushButton( WindowType nType );

    virtual void    FillLayoutData() const override;
    virtual const vcl::Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;

private:
    SymbolType      meSymbol;
    TriState        meState;
    TriState        meSaveValue;
    bool            mbPressed;
};

inline void PushButton::Check( bool bCheck )
{
    SetState( (bCheck) ? TRISTATE_TRUE : TRISTATE_FALSE );
}

inline bool PushButton::IsChecked() const
{
    return (GetState() == TRISTATE_TRUE);
}

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

class VCL_DLLPUBLIC CloseButton : public CancelButton
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

public:
    explicit        HelpButton( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    Click() override;
};

class VCL_DLLPUBLIC RadioButton : public Button
{
private:
    std::shared_ptr< std::vector< VclPtr< RadioButton > > > m_xGroup;
    Rectangle       maStateRect;
    Rectangle       maMouseRect;
    Image           maImage;
    bool            mbChecked;
    bool            mbSaveValue;
    bool            mbRadioCheck;
    bool            mbStateChanged;
    Link<RadioButton&,void> maToggleHdl;
    // when mbLegacyNoTextAlign is set then the old behaviour where
    // the WB_LEFT, WB_RIGHT & WB_CENTER affect the image placement
    // occurs, otherwise the image ( radiobutton circle ) is placed
    // to the left or right ( depending on RTL or LTR settings )
    bool            mbLegacyNoTextAlign;
    SAL_DLLPRIVATE void     ImplInitRadioButtonData();
    SAL_DLLPRIVATE WinBits  ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void     ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void     ImplDrawRadioButtonState(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void     ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                              const Point& rPos, const Size& rSize,
                              const Size& rImageSize, Rectangle& rStateRect,
                              Rectangle& rMouseRect );
    SAL_DLLPRIVATE void     ImplDrawRadioButton(vcl::RenderContext& rRenderContext );
    SAL_DLLPRIVATE void     ImplUncheckAllOther();
    SAL_DLLPRIVATE Size     ImplGetRadioImageSize() const;
    SAL_DLLPRIVATE long     ImplGetImageToTextDistance() const;

                            RadioButton(const RadioButton &) = delete;
                            RadioButton& operator= (const RadioButton &) = delete;

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( vcl::Window* pParent, WinBits nStyle );

public:
    SAL_DLLPRIVATE void     ImplCallClick( bool bGrabFocus = false, GetFocusFlags nFocusFlags = GetFocusFlags::NONE );
    SAL_DLLPRIVATE void     ImplSetMinimumNWFSize();

protected:
    virtual void    FillLayoutData() const override;
    virtual const vcl::Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;

    inline void     SetMouseRect( const Rectangle& _rMouseRect )    { maMouseRect = _rMouseRect; }
    inline void     SetStateRect( const Rectangle& _rStateRect )    { maStateRect = _rStateRect; }

    // draws the radio button (the knob image), in its current state (pressed/checked)
    // at the usual location, which can be overridden with SetStateRect
    void            DrawRadioButtonState(vcl::RenderContext& rRenderContext);

public:
    explicit        RadioButton( vcl::Window* pParent, WinBits nWinStyle = 0 );
    virtual         ~RadioButton() override;
    virtual void    dispose() override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    KeyUp( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    void            Toggle();

    bool            IsStateChanged() const { return mbStateChanged; }

    void            EnableRadioCheck( bool bRadioCheck ) { mbRadioCheck = bRadioCheck; }
    bool            IsRadioCheckEnabled() const { return mbRadioCheck; }

    bool            SetModeRadioImage( const Image& rImage );
    const Image&    GetModeRadioImage( ) const { return maImage;}

    void            SetState( bool bCheck );
    void            Check( bool bCheck = true );
    bool            IsChecked() const { return mbChecked; }

    void            SaveValue() { mbSaveValue = IsChecked(); }
    bool            GetSavedValue() const { return mbSaveValue; }
    bool            IsValueChangedFromSaved() const { return mbSaveValue != IsChecked(); }

    static Image    GetRadioImage( const AllSettings& rSettings, DrawButtonFlags nFlags );

    Size            CalcMinimumSize() const;
    virtual Size    GetOptimalSize() const override;

    void            SetToggleHdl( const Link<RadioButton&,void>& rLink ) { maToggleHdl = rLink; }

    /** GetRadioButtonGroup returns a list of pointers to <code>RadioButton</code>s in the same group.

    The pointers in the returned list are valid at the time call returns. However rescheduling
    or giving up the SolarMutex may mean events get executed that lead to the pointers getting
    invalid.

    @param bIncludeThis
    defines whether <code>this</code> is contained in the returned list

    @return
    on return contains the <code>RadioButton</code>s
    in the same group as this <code>RadioButton</code>.
    */
    std::vector<VclPtr<RadioButton> > GetRadioButtonGroup(bool bIncludeThis = true) const;

    virtual bool set_property(const OString &rKey, const OString &rValue) override;

    /*
     * Group this RadioButton with another
     */
    void group(RadioButton &rOther);
    virtual void ShowFocus(const Rectangle& rRect) override;

    virtual FactoryFunction GetUITestFactory() const override;
};

class VCL_DLLPUBLIC CheckBox : public Button
{
private:
    Rectangle       maStateRect;
    Rectangle       maMouseRect;
    TriState        meState;
    TriState        meSaveValue;
    bool            mbTriState;
    Link<CheckBox&,void> maToggleHdl;
    // when mbLegacyNoTextAlign is set then the old behaviour where
    // the WB_LEFT, WB_RIGHT & WB_CENTER affect the image placement
    // occurs, otherwise the image ( checkbox box ) is placed
    // to the left or right ( depending on RTL or LTR settings )
    bool            mbLegacyNoTextAlign;
    SAL_DLLPRIVATE void         ImplInitCheckBoxData();
    SAL_DLLPRIVATE static WinBits ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void         ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                                    const Point& rPos, const Size& rSize,
                                    const Size& rImageSize, Rectangle& rStateRect,
                                    Rectangle& rMouseRect );
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

    virtual void ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE const Rectangle& GetStateRect() const { return maStateRect; }
    SAL_DLLPRIVATE const Rectangle& GetMouseRect() const { return maMouseRect; }

public:
    SAL_DLLPRIVATE void         ImplCheck();
    SAL_DLLPRIVATE void         ImplSetMinimumNWFSize();
public:
    explicit        CheckBox( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    KeyUp( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    void            Toggle();

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( bool bCheck = true );
    bool            IsChecked() const;

    void            EnableTriState( bool bTriState = true );
    bool            IsTriStateEnabled() const { return mbTriState; }

    void            SaveValue() { meSaveValue = GetState(); }
    TriState        GetSavedValue() const { return meSaveValue; }
    bool            IsValueChangedFromSaved() const { return meSaveValue != GetState(); }

    static Image    GetCheckImage( const AllSettings& rSettings, DrawButtonFlags nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize() const override;

    void            SetToggleHdl( const Link<CheckBox&,void>& rLink ) { maToggleHdl = rLink; }
    void            SetLegacyNoTextAlign( bool bVal ) { mbLegacyNoTextAlign = bVal; }

    virtual bool set_property(const OString &rKey, const OString &rValue) override;
    virtual void ShowFocus(const Rectangle& rRect) override;

    virtual FactoryFunction GetUITestFactory() const override;
};

inline void CheckBox::Check( bool bCheck )
{
    SetState( (bCheck) ? TRISTATE_TRUE : TRISTATE_FALSE );
}

inline bool CheckBox::IsChecked() const
{
    return (GetState() == TRISTATE_TRUE);
}

class VCL_DLLPUBLIC ImageButton : public PushButton
{
protected:
    using PushButton::ImplInitStyle;

private:
    SAL_DLLPRIVATE void     ImplInitStyle();

                            ImageButton( const ImageButton & ) = delete;
                            ImageButton & operator= ( const ImageButton & ) = delete;

public:
                 ImageButton( vcl::Window* pParent, WinBits nStyle = 0 );
};

class VCL_DLLPUBLIC ImageRadioButton : public RadioButton
{
                    ImageRadioButton( const ImageRadioButton & ) = delete;
                    ImageRadioButton & operator= ( const ImageRadioButton & ) = delete;

public:
    explicit        ImageRadioButton( vcl::Window* pParent );
};

class VCL_DLLPUBLIC TriStateBox : public CheckBox
{
                    TriStateBox( const TriStateBox & ) = delete;
                    TriStateBox & operator= ( const TriStateBox & ) = delete;

public:
    explicit        TriStateBox( vcl::Window* pParent, WinBits nStyle );
};

class VCL_DLLPUBLIC DisclosureButton : public CheckBox
{
protected:
    SAL_DLLPRIVATE virtual void ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext) override;

public:
    explicit DisclosureButton( vcl::Window* pParent );

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
};

#endif // INCLUDED_VCL_BUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
