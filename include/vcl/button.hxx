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

#ifndef _SV_BUTTON_HXX
#define _SV_BUTTON_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salnativewidgets.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>

#include <vector>

class UserDrawEvent;
class ImplCommonButtonData;

// ----------
// - Button -
// ----------

class VCL_DLLPUBLIC Button : public Control
{
private:
    ImplCommonButtonData *mpButtonData;
    Link                  maClickHdl;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                  Button (const Button &);
    SAL_DLLPRIVATE                  Button & operator= (const Button &);
public:
    SAL_DLLPRIVATE sal_uInt16           ImplGetButtonState() const;
    SAL_DLLPRIVATE sal_uInt16&          ImplGetButtonState();
    SAL_DLLPRIVATE sal_uInt16           ImplGetTextStyle( OUString& rText, WinBits nWinStyle, sal_uLong nDrawFlags );
    SAL_DLLPRIVATE void             ImplDrawAlignedImage( OutputDevice* pDev, Point& rPos, Size& rSize,
                                              sal_Bool bLayout, sal_uLong nImageSep, sal_uLong nDrawFlags,
                                              sal_uInt16 nTextStyle, Rectangle *pSymbolRect=NULL, bool bAddImageSep = false );
    SAL_DLLPRIVATE void             ImplSetFocusRect( const Rectangle &rFocusRect );
    SAL_DLLPRIVATE const Rectangle& ImplGetFocusRect() const;
    SAL_DLLPRIVATE void             ImplSetSymbolAlign( SymbolAlign eAlign );
    /// The x-coordinate of the vertical separator line, use in MenuButton subclass only.
    SAL_DLLPRIVATE long             ImplGetSeparatorX() const;
    SAL_DLLPRIVATE void             ImplSetSeparatorX( long nX );

protected:
    explicit            Button( WindowType nType );

public:
    virtual            ~Button();

    virtual void        Click();

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }

    static OUString     GetStandardText( StandardButtonType eButton );

    sal_Bool            SetModeImage( const Image& rImage );
    const Image         GetModeImage( ) const;
    sal_Bool            HasImage() const;
    void                SetImageAlign( ImageAlign eAlign );
    ImageAlign          GetImageAlign() const;

    void                EnableImageDisplay( sal_Bool bEnable );
    void                EnableTextDisplay( sal_Bool bEnable );

    void                SetFocusRect( const Rectangle& rFocusRect );
    bool IsSmallSymbol() const;
    void SetSmallSymbol(bool bSmall = true);
    virtual bool set_property(const OString &rKey, const OString &rValue);
};

// --------------------
// - PushButton-Types -
// --------------------

#define PUSHBUTTON_DROPDOWN_TOOLBOX         ((sal_uInt16)0x0001)
#define PUSHBUTTON_DROPDOWN_MENUBUTTON      ((sal_uInt16)0x0002)

// --------------
// - PushButton -
// --------------

class VCL_DLLPUBLIC PushButton : public Button
{
protected:
    SymbolType      meSymbol;
    TriState        meState;
    TriState        meSaveValue;
    sal_uInt16          mnDDStyle;
    sal_Bool            mbPressed;
    sal_Bool            mbInUserDraw;
    Link            maToggleHdl;

    SAL_DLLPRIVATE void            ImplInitPushButtonData();
    SAL_DLLPRIVATE WinBits         ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void            ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void            ImplDrawPushButtonContent( OutputDevice* pDev, sal_uLong nDrawFlags,
                                               const Rectangle& rRect, bool bLayout, bool bMenuBtnSep );
    SAL_DLLPRIVATE void            ImplDrawPushButton( bool bLayout = false );
    using Button::ImplGetTextStyle;
    SAL_DLLPRIVATE sal_uInt16          ImplGetTextStyle( sal_uLong nDrawFlags ) const;
    SAL_DLLPRIVATE sal_Bool            IsSymbol() const { return ( (meSymbol != SYMBOL_NOSYMBOL) && (meSymbol != SYMBOL_IMAGE) ); }
    SAL_DLLPRIVATE sal_Bool            IsImage() const { return Button::HasImage(); }

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                 PushButton( const PushButton & );
    SAL_DLLPRIVATE                 PushButton& operator=( const PushButton & );

    SAL_DLLPRIVATE void            ImplInit( Window* pParent, WinBits nStyle );

    using Control::ImplInitSettings;
    using Window::ImplInit;
public:
    SAL_DLLPRIVATE void            ImplSetDefButton( sal_Bool bSet );
    SAL_DLLPRIVATE static void     ImplDrawPushButtonFrame( Window* pDev, Rectangle& rRect, sal_uInt16 nStyle );
    SAL_DLLPRIVATE static sal_Bool     ImplHitTestPushButton( Window* pDev, const Point& rPos );
    SAL_DLLPRIVATE sal_Bool            ImplIsDefButton() const;

protected:
    explicit        PushButton( WindowType nType );

    virtual void    FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;
public:
    explicit        PushButton( Window* pParent, WinBits nStyle = 0 );
    explicit        PushButton( Window* pParent, const ResId& );
    virtual         ~PushButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    virtual void    Toggle();

    void            SetSymbol( SymbolType eSymbol );
    SymbolType      GetSymbol() const { return meSymbol; }
    void            SetSymbolAlign( SymbolAlign eAlign );

    void            SetDropDown( sal_uInt16 nStyle );
    sal_uInt16          GetDropDown() const { return mnDDStyle; }

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked() const;

    void            SetPressed( sal_Bool bPressed );
    sal_Bool            IsPressed() const { return mbPressed; }

    void            EndSelection();

    void            SaveValue() { meSaveValue = GetState(); }
    TriState        GetSavedValue() const { return meSaveValue; }

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize() const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }
    virtual bool set_property(const OString &rKey, const OString &rValue);
};

inline void PushButton::Check( sal_Bool bCheck )
{
    SetState( (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline sal_Bool PushButton::IsChecked() const
{
    return (GetState() == STATE_CHECK);
}

// ------------
// - OKButton -
// ------------

class VCL_DLLPUBLIC OKButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void            ImplInit( Window* pParent, WinBits nStyle );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                 OKButton (const OKButton &);
    SAL_DLLPRIVATE                 OKButton & operator= (const OKButton &);

public:
    explicit        OKButton( Window* pParent, WinBits nStyle = WB_DEFBUTTON );
    explicit        OKButton( Window* pParent, const ResId& );

    virtual void    Click();
};

// ----------------
// - CancelButton -
// ----------------

class VCL_DLLPUBLIC CancelButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      CancelButton (const CancelButton &);
    SAL_DLLPRIVATE      CancelButton & operator= (const CancelButton &);

public:
    explicit        CancelButton( Window* pParent, WinBits nStyle = 0 );
    explicit        CancelButton( Window* pParent, const ResId& );

    virtual void    Click();
};

class VCL_DLLPUBLIC CloseButton : public CancelButton
{
public:
    explicit CloseButton(Window* pParent, WinBits nStyle = 0);
};


// --------------
// - HelpButton -
// --------------

class VCL_DLLPUBLIC HelpButton : public PushButton
{
protected:
    using PushButton::ImplInit;
private:
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      HelpButton( const HelpButton & );
    SAL_DLLPRIVATE      HelpButton & operator= ( const HelpButton & );

public:
    explicit        HelpButton( Window* pParent, WinBits nStyle = 0 );
    explicit        HelpButton( Window* pParent, const ResId& );

    virtual void    Click();
};

// ---------------
// - RadioButton -
// ---------------

class VCL_DLLPUBLIC RadioButton : public Button
{
private:
    boost::shared_ptr< std::vector<RadioButton*> > m_xGroup;
    Rectangle       maStateRect;
    Rectangle       maMouseRect;
    Image           maImage;
    sal_Bool        mbChecked;
    sal_Bool        mbSaveValue;
    sal_Bool        mbRadioCheck;
    sal_Bool        mbStateChanged;
    Link            maToggleHdl;
    // when mbLegacyNoTextAlign is set then the old behaviour where
    // the WB_LEFT, WB_RIGHT & WB_CENTER affect the image placement
    // occurs, otherwise the image ( radiobutton circle ) is placed
    // to the left or right ( depending on RTL or LTR settings )
    bool            mbLegacyNoTextAlign;
    SAL_DLLPRIVATE void     ImplInitRadioButtonData();
    SAL_DLLPRIVATE WinBits  ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void     ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void     ImplDrawRadioButtonState();
    SAL_DLLPRIVATE void     ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize,
                              const Size& rImageSize, Rectangle& rStateRect,
                              Rectangle& rMouseRect, bool bLayout = false );
    SAL_DLLPRIVATE void     ImplDrawRadioButton( bool bLayout = false );
    SAL_DLLPRIVATE void     ImplInvalidateOrDrawRadioButtonState();
    SAL_DLLPRIVATE void     ImplUncheckAllOther();
    SAL_DLLPRIVATE Size     ImplGetRadioImageSize() const;
    SAL_DLLPRIVATE long     ImplGetImageToTextDistance() const;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          RadioButton(const RadioButton &);
    SAL_DLLPRIVATE          RadioButton& operator= (const RadioButton &);

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    SAL_DLLPRIVATE void     ImplCallClick( sal_Bool bGrabFocus = sal_False, sal_uInt16 nFocusFlags = 0 );
    SAL_DLLPRIVATE void     ImplSetMinimumNWFSize();

protected:
    virtual void FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

    inline void             SetMouseRect( const Rectangle& _rMouseRect )    { maMouseRect = _rMouseRect; }
    inline const Rectangle& GetMouseRect( ) const                           { return maMouseRect; }
    inline void             SetStateRect( const Rectangle& _rStateRect )    { maStateRect = _rStateRect; }
    inline const Rectangle& GetStateRect( ) const                           { return maStateRect; }

    // draws the radio button (the knob image), in it's current state (pressed/checked)
    // at the usual location, which can be overridden with SetStateRect
    void            DrawRadioButtonState( );

public:
    explicit        RadioButton( Window* pParent, WinBits nWinStyle = 0 );
    explicit        RadioButton( Window* pParent, const ResId& );
    virtual         ~RadioButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    Toggle();

    sal_Bool            IsStateChanged() const { return mbStateChanged; }

    void            EnableRadioCheck( sal_Bool bRadioCheck = sal_True ) { mbRadioCheck = bRadioCheck; }
    sal_Bool            IsRadioCheckEnabled() const { return mbRadioCheck; }

    sal_Bool        SetModeRadioImage( const Image& rImage );
    const Image&    GetModeRadioImage( ) const;

    void            SetState( sal_Bool bCheck );
    void            Check( sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked() const { return mbChecked; }

    void            SaveValue() { mbSaveValue = IsChecked(); }
    sal_Bool            GetSavedValue() const { return mbSaveValue; }

    static Image    GetRadioImage( const AllSettings& rSettings, sal_uInt16 nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize() const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }

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
    std::vector<RadioButton*> GetRadioButtonGroup(bool bIncludeThis = true) const;

    virtual bool set_property(const OString &rKey, const OString &rValue);

    /*
     * Group this RadioButton with another
     */
    void group(RadioButton &rOther);
};

// ------------
// - CheckBox -
// ------------

class VCL_DLLPUBLIC CheckBox : public Button
{
private:
    Rectangle       maStateRect;
    Rectangle       maMouseRect;
    TriState        meState;
    TriState        meSaveValue;
    sal_Bool            mbTriState;
    Link            maToggleHdl;
    // when mbLegacyNoTextAlign is set then the old behaviour where
    // the WB_LEFT, WB_RIGHT & WB_CENTER affect the image placement
    // occurs, otherwise the image ( checkbox box ) is placed
    // to the left or right ( depending on RTL or LTR settings )
    bool            mbLegacyNoTextAlign;
    SAL_DLLPRIVATE void         ImplInitCheckBoxData();
    SAL_DLLPRIVATE WinBits      ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void         ImplInvalidateOrDrawCheckBoxState();
    SAL_DLLPRIVATE void         ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                                    const Point& rPos, const Size& rSize,
                                    const Size& rImageSize, Rectangle& rStateRect,
                                    Rectangle& rMouseRect, bool bLayout );
    SAL_DLLPRIVATE void         ImplDrawCheckBox( bool bLayout = false );
    SAL_DLLPRIVATE long         ImplGetImageToTextDistance() const;
    SAL_DLLPRIVATE Size         ImplGetCheckImageSize() const;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE              CheckBox(const CheckBox &);
    SAL_DLLPRIVATE              CheckBox& operator= (const CheckBox &);

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );
    virtual void FillLayoutData() const;
    virtual const Font&         GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&        GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

    virtual void ImplDrawCheckBoxState();
    SAL_DLLPRIVATE const Rectangle& GetStateRect() const { return maStateRect; }
    SAL_DLLPRIVATE const Rectangle& GetMouseRect() const { return maMouseRect; }

public:
    SAL_DLLPRIVATE void         ImplCheck();
    SAL_DLLPRIVATE void         ImplSetMinimumNWFSize();
public:
    explicit        CheckBox( Window* pParent, WinBits nStyle = 0 );
    explicit        CheckBox( Window* pParent, const ResId& );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    Toggle();

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked() const;

    void            EnableTriState( sal_Bool bTriState = sal_True );
    sal_Bool            IsTriStateEnabled() const { return mbTriState; }

    void            SaveValue() { meSaveValue = GetState(); }
    TriState        GetSavedValue() const { return meSaveValue; }

    static Image    GetCheckImage( const AllSettings& rSettings, sal_uInt16 nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize() const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }
    bool            IsLegacyNoTextAlign() { return mbLegacyNoTextAlign; }
    void            SetLegacyNoTextAlign( bool bVal ) { mbLegacyNoTextAlign = bVal; }

    virtual bool set_property(const OString &rKey, const OString &rValue);
};

inline void CheckBox::Check( sal_Bool bCheck )
{
    SetState( (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline sal_Bool CheckBox::IsChecked() const
{
    return (GetState() == STATE_CHECK);
}

// ---------------------------------
// - Control-Layer fuer alten Code -
// ---------------------------------

class VCL_DLLPUBLIC ImageButton : public PushButton
{
protected:
    using PushButton::ImplInitStyle;
private:
    SAL_DLLPRIVATE void     ImplInitStyle();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          ImageButton( const ImageButton & );
    SAL_DLLPRIVATE          ImageButton & operator= ( const ImageButton & );

public:
                    ImageButton( Window* pParent, WinBits nStyle = 0 );
                    ImageButton( Window* pParent, const ResId& rResId );
                    ~ImageButton();
};

class VCL_DLLPUBLIC ImageRadioButton : public RadioButton
{
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  ImageRadioButton( const ImageRadioButton & );
    SAL_DLLPRIVATE  ImageRadioButton & operator= ( const ImageRadioButton & );

public:
    explicit        ImageRadioButton( Window* pParent, WinBits nStyle = 0 );
    explicit        ImageRadioButton( Window* pParent, const ResId& );
    virtual         ~ImageRadioButton();
};

class VCL_DLLPUBLIC TriStateBox : public CheckBox
{
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  TriStateBox( const TriStateBox & );
    SAL_DLLPRIVATE  TriStateBox & operator= ( const TriStateBox & );

public:
    explicit        TriStateBox( Window* pParent, WinBits nStyle = 0 );
    explicit        TriStateBox( Window* pParent, const ResId& );
    virtual         ~TriStateBox();
};

class VCL_DLLPUBLIC DisclosureButton : public CheckBox
{
protected:
    SAL_DLLPRIVATE virtual void ImplDrawCheckBoxState();
public:
    explicit DisclosureButton( Window* pParent, WinBits nStyle = 0 );

    virtual void    KeyInput( const KeyEvent& rKEvt );
};

#endif  // _SV_BUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
