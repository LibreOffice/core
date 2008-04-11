/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: button.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _SV_BUTTON_HXX
#define _SV_BUTTON_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/image.hxx>
#include <vcl/symbol.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salnativewidgets.hxx>

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
    SAL_DLLPRIVATE USHORT           ImplGetButtonState() const;
    SAL_DLLPRIVATE USHORT&          ImplGetButtonState();
    SAL_DLLPRIVATE USHORT           ImplGetTextStyle( XubString& rText, WinBits nWinStyle, ULONG nDrawFlags );
    SAL_DLLPRIVATE void             ImplDrawAlignedImage( OutputDevice* pDev, Point& rPos, Size& rSize,
                                              BOOL bLayout, ULONG nImageSep, ULONG nDrawFlags,
                                              USHORT nTextStyle, Rectangle *pSymbolRect=NULL );
    SAL_DLLPRIVATE void             ImplSetFocusRect( const Rectangle &rFocusRect );
    SAL_DLLPRIVATE const Rectangle& ImplGetFocusRect() const;
    SAL_DLLPRIVATE void             ImplSetSymbolAlign( SymbolAlign eAlign );
    SAL_DLLPRIVATE SymbolAlign      ImplGetSymbolAlign() const;
    SAL_DLLPRIVATE void             ImplSetSmallSymbol( BOOL bSmall = TRUE );

protected:
                        Button( WindowType nType );

public:
                        Button( Window* pParent, WinBits nStyle = 0 );
                        Button( Window* pParent, const ResId& rResId );
                       ~Button();

    virtual void        Click();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }

    static XubString    GetStandardText( StandardButtonType eButton );
    static XubString    GetStandardHelpText( StandardButtonType eButton );

    BOOL                SetModeImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image         GetModeImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;
    BOOL                HasImage() const;
    void                SetImageAlign( ImageAlign eAlign );
    ImageAlign          GetImageAlign() const;

    BOOL                SetModeBitmap( const BitmapEx& rBitmap, BmpColorMode eMode = BMP_COLOR_NORMAL );
    BitmapEx            GetModeBitmap( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;

    void                EnableImageDisplay( BOOL bEnable );
    BOOL                IsImageDisplayEnabled();
    void                EnableTextDisplay( BOOL bEnable );
    BOOL                IsTextDisplayEnabled();

    void                SetFocusRect( const Rectangle& rFocusRect );
    const Rectangle&    GetFocusRect() const;
};

// --------------------
// - PushButton-Types -
// --------------------

#define PUSHBUTTON_DROPDOWN_TOOLBOX         ((USHORT)0x0001)
#define PUSHBUTTON_DROPDOWN_MENUBUTTON      ((USHORT)0x0002)

// --------------
// - PushButton -
// --------------

class VCL_DLLPUBLIC PushButton : public Button
{
protected:
    SymbolType      meSymbol;
    TriState        meState;
    TriState        meSaveValue;
    USHORT          mnDDStyle;
    BOOL            mbPressed;
    BOOL            mbInUserDraw;
    Link            maToggleHdl;

    SAL_DLLPRIVATE void            ImplInitPushButtonData();
    SAL_DLLPRIVATE WinBits         ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void            ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE void            ImplDrawPushButtonContent( OutputDevice* pDev, ULONG nDrawFlags,
                                               const Rectangle& rRect, bool bLayout );
    SAL_DLLPRIVATE void            ImplDrawPushButton( bool bLayout = false );
    using Button::ImplGetTextStyle;
    SAL_DLLPRIVATE USHORT          ImplGetTextStyle( ULONG nDrawFlags ) const;
    SAL_DLLPRIVATE BOOL            IsSymbol() const { return ( (meSymbol != SYMBOL_NOSYMBOL) && (meSymbol != SYMBOL_IMAGE) ); }
    SAL_DLLPRIVATE BOOL            IsImage() const { return Button::HasImage(); }

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                 PushButton( const PushButton & );
    SAL_DLLPRIVATE                 PushButton& operator=( const PushButton & );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void            ImplInit( Window* pParent, WinBits nStyle );

public:
    SAL_DLLPRIVATE void            ImplSetDefButton( BOOL bSet );
    SAL_DLLPRIVATE static void     ImplDrawPushButtonFrame( Window* pDev, Rectangle& rRect, USHORT nStyle );
    SAL_DLLPRIVATE static BOOL     ImplHitTestPushButton( Window* pDev, const Point& rPos );
    SAL_DLLPRIVATE BOOL            ImplIsDefButton() const;

protected:
                    PushButton( WindowType nType );

    virtual void    FillLayoutData() const;
public:
                    PushButton( Window* pParent, WinBits nStyle = 0 );
                    PushButton( Window* pParent, const ResId& rResId );
                    ~PushButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
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
    SymbolAlign     GetSymbolAlign() const;

    void            SetDropDown( USHORT nStyle );
    USHORT          GetDropDown() const { return mnDDStyle; }

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( BOOL bCheck = TRUE );
    BOOL            IsChecked() const;

    void            SetPressed( BOOL bPressed );
    BOOL            IsPressed() const { return mbPressed; }

    void            EndSelection();

    void            SaveValue() { meSaveValue = GetState(); }
    TriState        GetSavedValue() const { return meSaveValue; }

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }
};

inline void PushButton::Check( BOOL bCheck )
{
    SetState( (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline BOOL PushButton::IsChecked() const
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
                    OKButton( Window* pParent, WinBits nStyle = WB_DEFBUTTON );
                    OKButton( Window* pParent, const ResId& rResId );

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
                    CancelButton( Window* pParent, WinBits nStyle = 0 );
                    CancelButton( Window* pParent, const ResId& rResId );

    virtual void    Click();
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
                    HelpButton( Window* pParent, WinBits nStyle = 0 );
                    HelpButton( Window* pParent, const ResId& rResId );

    virtual void    Click();
};

// ---------------
// - RadioButton -
// ---------------

class VCL_DLLPUBLIC RadioButton : public Button
{
private:
    Rectangle       maStateRect;
    Rectangle       maMouseRect;
    Image           maImage;
    Image           maImageHC;
    BOOL            mbChecked;
    BOOL            mbSaveValue;
    BOOL            mbRadioCheck;
    BOOL            mbStateChanged;
    Link            maToggleHdl;

    SAL_DLLPRIVATE void     ImplInitRadioButtonData();
    SAL_DLLPRIVATE WinBits  ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void     ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE void     ImplDrawRadioButtonState();
    SAL_DLLPRIVATE void     ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                              const Point& rPos, const Size& rSize,
                              const Size& rImageSize, long nImageSep,
                              Rectangle& rStateRect, Rectangle& rMouseRect,
                              bool bLayout = false );
    SAL_DLLPRIVATE void     ImplDrawRadioButton( bool bLayout = false );
    SAL_DLLPRIVATE void     ImplInvalidateOrDrawRadioButtonState();
    SAL_DLLPRIVATE void     ImplUncheckAllOther();
    SAL_DLLPRIVATE Size     ImplGetRadioImageSize() const;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          RadioButton(const RadioButton &);
    SAL_DLLPRIVATE          RadioButton& operator= (const RadioButton &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    SAL_DLLPRIVATE void     ImplCallClick( BOOL bGrabFocus = FALSE, USHORT nFocusFlags = 0 );
    SAL_DLLPRIVATE void     ImplSetMinimumNWFSize();

protected:
    virtual void FillLayoutData() const;

    inline void             SetMouseRect( const Rectangle& _rMouseRect )    { maMouseRect = _rMouseRect; }
    inline const Rectangle& GetMouseRect( ) const                           { return maMouseRect; }
    inline void             SetStateRect( const Rectangle& _rStateRect )    { maStateRect = _rStateRect; }
    inline const Rectangle& GetStateRect( ) const                           { return maStateRect; }

    // draws the radio button (the knob image), in it's current state (pressed/checked)
    // at the usual location, which can be overridden with SetStateRect
    void            DrawRadioButtonState( );

public:
                    RadioButton( Window* pParent, WinBits nWinStyle = 0 );
                    RadioButton( Window* pParent, const ResId& rResId );
                    ~RadioButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    Toggle();

    BOOL            IsStateChanged() const { return mbStateChanged; }

    void            EnableRadioCheck( BOOL bRadioCheck = TRUE ) { mbRadioCheck = bRadioCheck; }
    BOOL            IsRadioCheckEnabled() const { return mbRadioCheck; }

    BOOL            SetModeRadioImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image&    GetModeRadioImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;

    void            SetState( BOOL bCheck );
    void            Check( BOOL bCheck = TRUE );
    BOOL            IsChecked() const { return mbChecked; }

    void            SaveValue() { mbSaveValue = IsChecked(); }
    BOOL            GetSavedValue() const { return mbSaveValue; }

    static Image    GetRadioImage( const AllSettings& rSettings, USHORT nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }

    /** GetRadioButtonGroup returns a list of pointers to <code>RadioButton</code>s in the same group.

    The pointers in the returned list are valid at the time call returns. However rescheduling
    or giving up the SolarMutex may mean events get executed that lead to the pointers getting
    invalid.

    @param io_rGroup
    gets cleared on entering the function. on return contains the <code>RadioButton</code>s
    in the same group as this <code>RadioButton</code>.

    @param bIncludeThis
    defines whether <code>this</code> is contained in the returned list
    */
    void            GetRadioButtonGroup( std::vector<RadioButton*>& io_rGroup, bool bIncludeThis ) const;
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
    BOOL            mbTriState;
    Link            maToggleHdl;

    SAL_DLLPRIVATE void         ImplInitCheckBoxData();
    SAL_DLLPRIVATE WinBits      ImplInitStyle( const Window* pPrevWindow, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE void         ImplDrawCheckBoxState();
    SAL_DLLPRIVATE void         ImplInvalidateOrDrawCheckBoxState();
    SAL_DLLPRIVATE void         ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                                    const Point& rPos, const Size& rSize,
                                    const Size& rImageSize, long nImageSep,
                                    Rectangle& rStateRect,
                                    Rectangle& rMouseRect, bool bLayout );
    SAL_DLLPRIVATE void         ImplDrawCheckBox( bool bLayout = false );
    SAL_DLLPRIVATE Size         ImplGetCheckImageSize() const;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE              CheckBox(const CheckBox &);
    SAL_DLLPRIVATE              CheckBox& operator= (const CheckBox &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE virtual void FillLayoutData() const;

public:
    SAL_DLLPRIVATE void         ImplCheck();
    SAL_DLLPRIVATE void         ImplSetMinimumNWFSize();

public:
                    CheckBox( Window* pParent, WinBits nStyle = 0 );
                    CheckBox( Window* pParent, const ResId& rResId );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    Toggle();

    void            SetState( TriState eState );
    TriState        GetState() const { return meState; }

    void            Check( BOOL bCheck = TRUE );
    BOOL            IsChecked() const;

    void            EnableTriState( BOOL bTriState = TRUE );
    BOOL            IsTriStateEnabled() const { return mbTriState; }

    void            SaveValue() { meSaveValue = GetState(); }
    TriState        GetSavedValue() const { return meSaveValue; }

    static Image    GetCheckImage( const AllSettings& rSettings, USHORT nFlags );

    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
    const Link&     GetToggleHdl() const { return maToggleHdl; }
};

inline void CheckBox::Check( BOOL bCheck )
{
    SetState( (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline BOOL CheckBox::IsChecked() const
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

protected:
                    ImageButton( WindowType nType );

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
                    ImageRadioButton( Window* pParent, WinBits nStyle = 0 );
                    ImageRadioButton( Window* pParent, const ResId& rResId );
                    ~ImageRadioButton();
};

class VCL_DLLPUBLIC TriStateBox : public CheckBox
{
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  TriStateBox( const TriStateBox & );
    SAL_DLLPRIVATE  TriStateBox & operator= ( const TriStateBox & );

public:
            TriStateBox( Window* pParent, WinBits nStyle = 0 );
            TriStateBox( Window* pParent, const ResId& rResId );
            ~TriStateBox();
};

#endif  // _SV_BUTTON_HXX
